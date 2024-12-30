#include "cbase/c_allocator.h"
#include "cbase/c_log.h"
#include "ccore/c_math.h"
#include "cfile/c_file.h"

#include "charon/c_gamedata.h"
#include "charon/c_archive.h"

namespace ncore
{
    namespace charon
    {
        // Constraints:
        //     Maximum file size = 4GB
        //     Maximum file offset = 2GB * 64 = 128GB
        //     Compression = FileOffset & 0x80000000

        struct gda_t;
        struct toc_t;
        struct fdb_t;
        struct hdb_t;

        // ------------------------------------------------------------------------------------------------
        // ------- A Single Datafile Archive --------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        class archivefile_t
        {
        public:
            archivefile_t();

            s32  open(alloc_t* allocator, const char* archiveFilename, const char* tocFilename, const char* filenameDbFilename, const char* hashDbFilename);
            void close(alloc_t* allocator);

            bool                     exists(fileid_t id) const;                                             // Return True if file exists in Archive
            archive_t::file_t const* file(fileid_t id) const;                                               // Return FileEntry associated with file id
            string_t                 filename(fileid_t id) const;                                           // Return Filename associated with file id
            s64                      fileRead(fileid_t id, s32 offset, s32 size, void* destination) const;  // Read part of file in destination

            void*  mBasePtr;  // The TOC of the datafile in memory
            s32    mIndex;    // Index of the datafile in the datafile manager
            gda_t* mGDA;      // The .gda file
            toc_t* mTOC;      // The TOC of the datafile
            fdb_t* mFDB;      // In DEBUG mode if you want to know the filename of a fileid_t
            hdb_t* mHDB;      // In DEBUG mode if you want to know the hash of a fileid_t
        };

        // ------------------------------------------------------------------------------------------------
        // ------- DataFiles, the actual implementation ----------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        class archive_t::imp_t : public archive_loader_t
        {
        public:
            void                     setup(alloc_t* allocator, s32 maxNumDataFileArchives);
            void                     teardown();
            bool                     exists(fileid_t id) const;
            archive_t::file_t const* fileitem(fileid_t id) const;
            string_t                 filename(fileid_t id) const;
            s64                      v_fileRead(fileid_t id, s32 offset, s32 size, void* destination) const override;
            void*                    v_fileRead(fileid_t id, alloc_t* allocator) const override;

            alloc_t*        mAllocator;
            s32             mNumArchives;
            s32             mMaxNumArchives;
            archivefile_t** mArchives;
        };

        void archive_t::imp_t::setup(alloc_t* allocator, s32 maxNumDataFileArchives)
        {
            mAllocator      = allocator;
            mNumArchives    = 0;
            mMaxNumArchives = maxNumDataFileArchives;
            mArchives       = g_allocate_array_and_clear<archivefile_t*>(allocator, maxNumDataFileArchives);
        }

        void archive_t::imp_t::teardown()
        {
            for (s32 i = 0; i < mNumArchives; ++i)
            {
                if (mArchives[i] != nullptr)
                {
                    mArchives[i]->close(mAllocator);
                    g_deallocate(mAllocator, mArchives[i]);
                    mArchives[i] = nullptr;
                }
            }
            g_deallocate(mAllocator, mArchives);
            mAllocator = nullptr;
        }

        bool archive_t::imp_t::exists(fileid_t id) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr && datafile->exists(id);
            }
            return false;
        }

        archive_t::file_t const* archive_t::imp_t::fileitem(fileid_t id) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr ? datafile->file(id) : nullptr;
            }
            return nullptr;
        }

        string_t archive_t::imp_t::filename(fileid_t id) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr ? datafile->filename(id) : string_t();
            }
            return string_t();
        }

        s64 archive_t::imp_t::v_fileRead(fileid_t id, s32 offset, s32 size, void* destination) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr ? datafile->fileRead(id, offset, size, destination) : 0;
            }
            return 0;
        }

        void* archive_t::imp_t::v_fileRead(fileid_t id, alloc_t* allocator) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t*           datafile = mArchives[id.getArchiveIndex()];
                archive_t::file_t const* entry    = datafile != nullptr ? datafile->file(id) : nullptr;
                if (entry != nullptr)
                {
                    u8* data = g_allocate_array<byte>(allocator, entry->getFileSize());
                    datafile->fileRead(id, 0, entry->getFileSize(), data);
                    return data;
                }
            }
            return nullptr;
        }

        // TOC
        //     Int32:                  Section Count
        //     u32[]:                  Array of Offset to Section
        //     archive_t::section_t[]: Array
        // End
        struct toc_t
        {
            archive_t::section_t const* getSection(u32 index) const;
            archive_t::file_t const*    getFileItem(fileid_t index) const;

        private:
            u32 mNumSections;
        };

        static archive_t::file_t const    s_invalidFileEntry = {0, 0};
        static archive_t::section_t const s_invalidSection   = {0, 0, 0, 0, &s_invalidFileEntry};

        archive_t::section_t const* toc_t::getSection(u32 index) const
        {
            if (index < mNumSections)
            {
                archive_t::section_t const* sections = (archive_t::section_t const*)((byte*)this + sizeof(u32));
                return &sections[index];
            }
            return &s_invalidSection;
        }

        archive_t::file_t const* toc_t::getFileItem(fileid_t id) const
        {
            const u32                   sectionIndex = id.getArchiveIndex();
            archive_t::section_t const* section      = getSection(sectionIndex);
            archive_t::file_t const*    itemArray    = (archive_t::file_t const*)section->m_ItemArray;
            return (id.getFileIndex() < section->m_ItemCount) ? &itemArray[id.getFileIndex()] : &s_invalidFileEntry;
        }

        struct gda_t
        {
            bool                 isValid() const { return fd.isValid(); }
            nfile::file_handle_t fd;
        };

        // FDB is a file/db containing all the filenames of the files in the datafile
        //   Int32: NumSections
        //   Array: Section[NumSections]
        //   Array: {void*, NumBytes, Count}
        // End
        struct fdb_t
        {
            string_t getFilename(fileid_t id) const;
            u32      mNumSections;
        };

        string_t fdb_t::getFilename(fileid_t id) const
        {
            u32 const             archiveIndex       = (mNumSections == 1) ? 0 : id.getArchiveIndex();
            u32 const*            sectionOffsetArray = (u32*)((byte*)this + sizeof(u32));
            u32 const             sectionOffset      = sectionOffsetArray[archiveIndex];
            archive_t::section_t* section            = (archive_t::section_t*)((byte*)this + sectionOffset);
            if (id.getFileIndex() < section->m_ItemCount)
            {
                u32 const* filenameOffsetArray = (u32 const*)section->m_ItemArray;
                u32 const  filenameOffset      = filenameOffsetArray[id.getFileIndex()];
                u32 const* filename            = (u32*)((byte*)this + filenameOffset);
                u32 const  numBytes            = filename[0];
                u32 const  numRunes            = filename[1];
                return string_t(numBytes, numRunes, (const char*)&filename[2]);
            }
            return string_t();
        }

        // HDB is a file containing all the hash values of the files in the datafile
        //   Int32: NumSections
        //   Array: SectionOffset[NumSections]
        //   Section:
        //     Array: u64[NumFiles]
        //   End
        // End
        struct hdb_t
        {
            u64 getHash(fileid_t id) const;
            u32 mNumSections;
        };

        u64 hdb_t::getHash(fileid_t id) const
        {
            u32 const                   archiveIndex = (mNumSections == 1) ? 0 : id.getArchiveIndex();
            archive_t::section_t const* sections     = (archive_t::section_t*)((byte*)this + sizeof(u32));
            archive_t::section_t const* section      = &sections[archiveIndex];
            if (id.getFileIndex() < section->m_ItemCount)
            {
                u64 const* hashArray = (u64 const*)section->m_ItemArray;
                return hashArray[id.getFileIndex()];
            }
        }

        // The .bfa file containing all the files, uses the .mft file to obtain the
        // offset in the .bfa file for a file.

        archivefile_t::archivefile_t()
        {
            mTOC = nullptr;
            mFDB = nullptr;
            mGDA = nullptr;
        }

        static void* s_read_file(const char* filename, alloc_t* allocator, s64* fileSize)
        {
            nfile::file_handle_t fd = nfile::file_open(filename, nfile::file_mode_t::FILE_MODE_READ);
            if (fd.isValid())
            {
                s64   size = nfile::file_size(fd);
                void* data = g_allocate_array<byte>(allocator, size);
                nfile::file_read(fd, (u8*)data, (u32)size);
                nfile::file_close(fd);
                if (fileSize != nullptr)
                    *fileSize = size;
                return data;
            }
            return nullptr;
        }

        s32 archivefile_t::open(alloc_t* allocator, const char* archiveFilename, const char* tocFilename, const char* filenameDbFilename, const char* hashDbFilename)
        {
            close(allocator);

            mGDA     = g_allocate<gda_t>(allocator);
            mGDA->fd = file_open(archiveFilename, nfile::file_mode_t::FILE_MODE_READ);
            if (mGDA->isValid())
            {
                mTOC = (toc_t*)s_read_file(tocFilename, allocator, nullptr);
#if !defined(_SUBMISSION)
                mFDB = (fdb_t*)s_read_file(filenameDbFilename, allocator, nullptr);
                mHDB = (hdb_t*)s_read_file(hashDbFilename, allocator, nullptr);
#endif
            }
            else
            {
                return -1;
            }

            return 0;
        }

        void archivefile_t::close(alloc_t* allocator)
        {
            if (mGDA->isValid())
            {
                nfile::file_close(mGDA->fd);
            }

            g_deallocate(allocator, mGDA);
            g_deallocate(allocator, mTOC);
            g_deallocate(allocator, mFDB);
            g_deallocate(allocator, mHDB);
        }

        bool archivefile_t::exists(fileid_t id) const
        {
            archive_t::file_t const* f = mTOC->getFileItem(id);
            return f != nullptr && f->isValid();
        }

        archive_t::file_t const* archivefile_t::file(fileid_t id) const { return mTOC->getFileItem(id); }
        string_t                 archivefile_t::filename(fileid_t id) const { return mFDB != nullptr ? mFDB->getFilename(id) : string_t(); }

        s64 archivefile_t::fileRead(fileid_t id, s32 offset, s32 size, void* destination) const
        {
            archive_t::file_t const* f = mTOC->getFileItem(id);
            if (!f->isValid())
                return -1;

            const s64 seekpos = f->getFileOffset() + offset;
            nfile::file_seek(mGDA->fd, seekpos, nfile::seek_mode_t::SEEK_MODE_BEG);
            return nfile::file_read(mGDA->fd, (u8*)destination, size);
        }

        // ------------------------------------------------------------------------------------------------
        // ------- Archive Implementation -----------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------

        void archive_t::init(alloc_t* allocator, s32 maxNumDataFileArchives)
        {
            mImp = g_allocate<archive_t::imp_t>(allocator);
            mImp->setup(allocator, maxNumDataFileArchives);
        }

        void archive_t::teardown()
        {
            alloc_t* allocator = mImp->mAllocator;
            mImp->teardown();
            g_deallocate(allocator, mImp);
            mImp = nullptr;
        }

        bool                     archive_t::exists(fileid_t id) const { return mImp->exists(id); }
        archive_t::file_t const* archive_t::fileitem(fileid_t id) const { return mImp->fileitem(id); }
        string_t                 archive_t::filename(fileid_t id) const { return mImp->filename(id); }
        archive_loader_t*        archive_t::loader() const { return mImp; }

    }  // namespace charon
}  // namespace ncore
