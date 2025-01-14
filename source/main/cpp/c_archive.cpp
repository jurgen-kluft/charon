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
        // ------- Read a File into Memory ----------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
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

        // ------------------------------------------------------------------------------------------------
        // ------- Patching Pointers in a Datafile --------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        u8* g_patch(dataunit_header_t* data)
        {
            s32 count = data->m_patch_count;
            s32* head = (s32*)((u8*)data + data->m_patch_offset);
            if (head[0] > 0)
            {
                s32* pointer = (s32*)((uptr_t)data + head[0]);
                while (true)
                {
                    s32 const nextOffset = pointer[0];
                    s32 const dataOffset = pointer[1];

                    void** pointerToPatch = (void**)pointer;
                    *pointerToPatch       = (void*)((uptr_t)pointer + dataOffset);

                    if (nextOffset == 0)
                        break;
                    pointer = (s32*)((uptr_t)pointer + nextOffset);
                }
            }
            return (u8*)(data + 1);
        }

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
        // ------- Data Archive, implementation -----------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        class archive_imp_t : public archive_loader_t
        {
        public:
            void                     setup(alloc_t* allocator, s32 maxNumDataUnits, s32 maxNumDataFileArchives);
            void                     teardown();
            bool                     exists(fileid_t id) const;
            archive_t::file_t const* fileitem(fileid_t id) const;
            string_t                 filename(fileid_t id) const;

            void* v_get_datafile_ptr(fileid_t fileid) override;
            void* v_get_dataunit_ptr(u32 dataunit_index) override;
            void* v_load_datafile(fileid_t fileid) override;
            void* v_load_dataunit(u32 dataunit_index) override;
            void  v_unload_datafile(fileid_t fileid, void*& data) override;
            void  v_unload_dataunit(u32 dataunit_index, void*& data) override;

            alloc_t*               mAllocator;
            s32                    mNumDataUnits;
            s32                    mNumArchives;
            archivefile_t**        mArchives;
            archive_t::section_t** mArchiveSections;
            void***                mDataFilePtrs;
            dataunit_header_t**    mDataUnitPtrs;
        };

        void archive_imp_t::setup(alloc_t* allocator, s32 maxNumDataUnits, s32 maxNumDataFileArchives)
        {
            mAllocator       = allocator;
            mNumDataUnits    = maxNumDataUnits;
            mNumArchives     = maxNumDataFileArchives;
            mArchives        = g_allocate_array_and_clear<archivefile_t*>(allocator, maxNumDataFileArchives);
            mArchiveSections = g_allocate_array_and_clear<archive_t::section_t*>(allocator, maxNumDataFileArchives);
            mDataFilePtrs    = g_allocate_array_and_clear<void**>(allocator, maxNumDataFileArchives);
            mDataUnitPtrs    = g_allocate_array_and_clear<dataunit_header_t*>(allocator, maxNumDataUnits);
        }

        void archive_imp_t::teardown()
        {
            for (s32 i = 0; i < mNumDataUnits; ++i)
            {
                if (mDataUnitPtrs[i] != nullptr)
                {
                    g_deallocate(mAllocator, mDataUnitPtrs[i]);
                }
            }

            for (s32 i = 0; i < mNumArchives; ++i)
            {
                if (mArchives[i] != nullptr)
                {
                    mArchives[i]->close(mAllocator);
                    g_deallocate(mAllocator, mArchives[i]);
                }

                archive_t::section_t* section = mArchiveSections[i];
                if (section != nullptr)
                {
                    for (s32 j = 0; j < section->m_ItemArrayCount; ++j)
                    {
                        if (mDataFilePtrs[i][j] != nullptr)
                        {
                            g_deallocate(mAllocator, mDataFilePtrs[i][j]);
                        }
                    }
                    mArchiveSections[i] = nullptr;
                }
            }

            g_deallocate(mAllocator, mArchives);
            g_deallocate(mAllocator, mArchiveSections);
            g_deallocate(mAllocator, mDataFilePtrs);
            g_deallocate(mAllocator, mDataUnitPtrs);
        }

        bool archive_imp_t::exists(fileid_t id) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr && datafile->exists(id);
            }
            return false;
        }

        archive_t::file_t const* archive_imp_t::fileitem(fileid_t id) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr ? datafile->file(id) : nullptr;
            }
            return nullptr;
        }

        string_t archive_imp_t::filename(fileid_t id) const
        {
            if (id.getArchiveIndex() < mNumArchives)
            {
                archivefile_t* datafile = mArchives[id.getArchiveIndex()];
                return datafile != nullptr ? datafile->filename(id) : string_t();
            }
            return string_t();
        }

        void* archive_imp_t::v_get_datafile_ptr(fileid_t fileid)
        {
            if (fileid.getArchiveIndex() < mNumArchives)
            {
                archive_t::section_t const* section = mArchiveSections[fileid.getArchiveIndex()];
                if (section != nullptr && fileid.getFileIndex() < section->m_ItemArrayCount)
                {
                    void** archiveDataPtrs = mDataFilePtrs[fileid.getArchiveIndex()];
                    if (archiveDataPtrs != nullptr)
                    {
                        return archiveDataPtrs[fileid.getArchiveIndex()];
                    }
                }
            }
            return nullptr;
        }

        void* archive_imp_t::v_get_dataunit_ptr(u32 dataunit_index)
        {
            if (dataunit_index < mNumDataUnits)
            {
                return mDataUnitPtrs[dataunit_index] + 1;
            }
            return nullptr;
        }

        void* archive_imp_t::v_load_datafile(fileid_t fileid)
        {
            if (fileid.getArchiveIndex() < mNumArchives)
            {
                archive_t::section_t const* section = mArchiveSections[fileid.getArchiveIndex()];
                if (section != nullptr && fileid.getFileIndex() < section->m_ItemArrayCount)
                {
                    void** archiveDataPtrs = mDataFilePtrs[fileid.getArchiveIndex()];
                    // TODO: the array of pointers might not exist yet

                    void*& archiveDataFilePtr = archiveDataPtrs[fileid.getFileIndex()];
                    if (archiveDataFilePtr == nullptr)
                    {
                        archive_t::file_t const* files = section->getItemArray<archive_t::file_t>();
                        archive_t::file_t const* entry = &files[fileid.getFileIndex()];
                        u8*                      data  = g_allocate_array<byte>(mAllocator, entry->getFileSize());

                        archivefile_t* dataArchive = mArchives[fileid.getArchiveIndex()];
                        dataArchive->fileRead(fileid, 0, entry->getFileSize(), data);

                        archiveDataFilePtr = data;
                    }
                    return archiveDataFilePtr;
                }
            }
            return nullptr;
        }

        void* archive_imp_t::v_load_dataunit(u32 dataunit_index)
        {
            fileid_t fileid(0, dataunit_index);
            if (fileid.getArchiveIndex() < mNumArchives)
            {
                archive_t::section_t const* section = mArchiveSections[fileid.getArchiveIndex()];
                if (section != nullptr && fileid.getFileIndex() < section->m_ItemArrayCount)
                {
                    dataunit_header_t*& dataUnitPtr = mDataUnitPtrs[fileid.getFileIndex()];
                    if (dataUnitPtr == nullptr)
                    {
                        archive_t::file_t const* files = (archive_t::file_t const*)section->getItemArray<archive_t::file_t>();
                        archive_t::file_t const* entry = &files[fileid.getFileIndex()];
                        u8*                      data  = g_allocate_array<byte>(mAllocator, entry->getFileSize());

                        archivefile_t* dataArchive = mArchives[fileid.getArchiveIndex()];
                        dataArchive->fileRead(fileid, 0, entry->getFileSize(), data);

                        dataUnitPtr = (dataunit_header_t*)data;
                        return g_patch(dataUnitPtr);
                    }
                    return dataUnitPtr + 1;
                }
            }
            return nullptr;
        }

        void archive_imp_t::v_unload_datafile(fileid_t fileid, void*& data)
        {
            if (fileid.getArchiveIndex() < mNumArchives)
            {
                archive_t::section_t const* section = mArchiveSections[fileid.getArchiveIndex()];
                if (section != nullptr && fileid.getFileIndex() < section->m_ItemArrayCount)
                {
                    void** archiveDataPtrs = mDataFilePtrs[fileid.getArchiveIndex()];
                    if (archiveDataPtrs != nullptr)
                    {
                        void*& archiveDataFilePtr = archiveDataPtrs[fileid.getFileIndex()];
                        if (archiveDataFilePtr != nullptr)
                        {
                            ASSERT(archiveDataFilePtr == data);
                            g_deallocate(mAllocator, archiveDataFilePtr);
                            data = nullptr;
                        }
                    }
                }
            }
        }

        void archive_imp_t::v_unload_dataunit(u32 dataunit_index, void*& data)
        {
            fileid_t fileid(0, dataunit_index);
            if (fileid.getArchiveIndex() < mNumArchives)
            {
                archive_t::section_t const* section = mArchiveSections[fileid.getArchiveIndex()];
                if (section != nullptr && fileid.getFileIndex() < section->m_ItemArrayCount)
                {
                    dataunit_header_t*& dataUnitPtr = mDataUnitPtrs[fileid.getFileIndex()];
                    if (dataUnitPtr != nullptr)
                    {
                        ASSERT(dataUnitPtr == (dataunit_header_t*)data);
                        g_deallocate(mAllocator, dataUnitPtr);
                        data = nullptr;
                    }
                }
            }
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
        static archive_t::section_t const s_invalidSection   = {0, 0, 0, 0};

        archive_t::section_t const* toc_t::getSection(u32 archiveIndex) const
        {
            u32 const i = (mNumSections == 1) ? 0 : archiveIndex;
            if (i >= mNumSections)
                return &s_invalidSection;
            archive_t::section_t const* sections = (archive_t::section_t const*)((byte*)this + sizeof(u32));
            return &sections[i];
        }

        archive_t::file_t const* toc_t::getFileItem(fileid_t id) const
        {
            archive_t::section_t const* section   = getSection(id.getArchiveIndex());
            archive_t::file_t const*    itemArray = section->getItemArray<archive_t::file_t>();
            return (id.getFileIndex() < section->m_ItemArrayCount) ? &itemArray[id.getFileIndex()] : &s_invalidFileEntry;
        }

        struct gda_t
        {
            bool                 isValid() const { return fd.isValid(); }
            nfile::file_handle_t fd;
        };

        // FDB is a file/db containing all the filenames of the files in the datafile
        //   Int32: NumSections
        //   Array: Section[NumSections]
        //   Array: {NumBytes, Count, byte[]}
        // End
        struct fdb_t
        {
            string_t                    getFilename(fileid_t id) const;
            archive_t::section_t const* getSection(u32 index) const;
            u32                         mNumSections;
        };

        archive_t::section_t const* fdb_t::getSection(u32 archiveIndex) const
        {
            u32 const i = (mNumSections == 1) ? 0 : archiveIndex;
            if (i >= mNumSections)
                return &s_invalidSection;
            archive_t::section_t const* sections = (archive_t::section_t const*)((byte*)this + sizeof(u32));
            return &sections[i];
        }

        string_t fdb_t::getFilename(fileid_t id) const
        {
            archive_t::section_t const* section = getSection(id.getArchiveIndex());
            if (id.getFileIndex() >= section->m_ItemArrayCount)
                return string_t();

            u32 const* filenameOffsetArray = section->getItemArray<u32>();
            u32 const  filenameOffset      = filenameOffsetArray[id.getFileIndex()];
            u32 const* filenameItem        = (u32*)((byte*)this + filenameOffset);
            return string_t(filenameItem[0], filenameItem[1], (const char*)&filenameItem[2]);
        }

        // HDB is a file containing all the hash values of the files in the datafile
        //   Int32: NumSections
        //   Array: Sections[NumSections]
        //   Array:
        //     Array: u64[NumFiles]
        //   End
        // End
        struct hdb_t
        {
            u64                         getHash(fileid_t id) const;
            archive_t::section_t const* getSection(u32 index) const;
            u32                         mNumSections;
        };

        archive_t::section_t const* hdb_t::getSection(u32 archiveIndex) const
        {
            u32 const i = (mNumSections == 1) ? 0 : archiveIndex;
            if (i >= mNumSections)
                return &s_invalidSection;
            archive_t::section_t const* sections = (archive_t::section_t const*)((byte*)this + sizeof(u32));
            return &sections[i];
        }

        u64 hdb_t::getHash(fileid_t id) const
        {
            archive_t::section_t const* section = getSection(id.getArchiveIndex());
            if (id.getFileIndex() >= section->m_ItemArrayCount)
                return 0;

            u64 const* hashArray = section->getItemArray<u64>();
            return hashArray[id.getFileIndex()];
        }

        archivefile_t::archivefile_t()
        {
            mTOC = nullptr;
            mFDB = nullptr;
            mGDA = nullptr;
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
                return 0;
            }
            return -1;
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
        archive_t*            archive_t::s_instance = nullptr;
        static archive_imp_t* s_imp                 = nullptr;
        archive_loader_t*     g_loader              = nullptr;

        bool                     archive_t::exists(fileid_t const& id) const { return s_imp->exists(id); }
        archive_t::file_t const* archive_t::fileitem(fileid_t const& id) const { return s_imp->fileitem(id); }
        string_t                 archive_t::filename(fileid_t const& id) const { return s_imp->filename(id); }
        archive_loader_t*        archive_t::loader() const { return s_imp; }

        void archive_t::s_setup(alloc_t* allocator, s32 maxNumDataUnits, s32 maxNumDataArchives)
        {
            if (s_instance == nullptr)
            {
                s_instance = g_allocate<archive_t>(allocator);

                s_imp = g_allocate<archive_imp_t>(allocator);
                s_imp->setup(allocator, maxNumDataUnits, maxNumDataArchives);

                g_loader = s_imp;
            }
        }

        void archive_t::s_teardown()
        {
            if (s_instance != nullptr)
            {
                alloc_t* allocator = s_imp->mAllocator;
                s_imp->teardown();

                g_deallocate(allocator, s_imp);
                s_imp    = nullptr;
                g_loader = nullptr;

                g_deallocate(allocator, s_instance);
                s_instance = nullptr;
            }
        }

    }  // namespace charon
}  // namespace ncore
