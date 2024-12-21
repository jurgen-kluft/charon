#include "cbase/c_allocator.h"
#include "cbase/c_log.h"
#include "ccore/c_math.h"
#include "cfile/c_file.h"

#include "charon/c_bigfile.h"
#include "charon/c_object.h"

namespace ncore
{
    namespace charon
    {
        struct bigfile_t
        {
            bigfile_t();

            s32  open(alloc_t* allocator, const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename);
            void close(alloc_t* allocator);

            bool                exists(fileid_t id) const;                                             // Return True if file exists in Archive
            bool                isEqual(fileid_t firstId, fileid_t secondId) const;                    // Return True if both fileIds reference the same physical file
            file_entry_t const* file(fileid_t id) const;                                               // Return FileEntry associated with file id
            string_t            filename(fileid_t id) const;                                           // Return Filename associated with file id
            s64                 fileRead(fileid_t id, void* destination) const;                        // Read whole file in destination
            s64                 fileRead(fileid_t id, s32 size, void* destination) const;              // Read part of file header in destination
            s64                 fileRead(fileid_t id, s32 offset, s32 size, void* destination) const;  // Read part of file in destination

            void*  mBasePtr;  // The TOC of the bigfile in memory
            s32    mIndex;    // Index of the bigfile in the bigfile manager
            gda_t* mGDA;      // The .gda file
            toc_t* mTOC;      // The TOC of the bigfile
            fdb_t* mFDB;      // In DEBUG mode if you want to know the filename of a fileid_t
            hdb_t* mHDB;      // In DEBUG mode if you want to know the hash of a fileid_t
        };

        // Constraints:
        //     Maximum file size = 4GB
        //     Maximum file offset = 4GB * 64 = 256GB
        //     Compression = FileChildrenOffset & 0x1
        //     HasChildren = FileChildrenOffset & 0x2

        static const file_entry_t s_invalidFileEntry;

        // TOC
        //     Int32: Toc Offset
        //     Int32: Toc Count
        //     MFT[]: Array

        // MFT
        //     Int32: Toc Offset
        //     Int32: Toc Count
        //     Int32: GDA Offset
        //     Array: FileId's
        //       {Int32:FileOffset in Archive, Int32:FileSize, Int32:FileChildrenOffset}
        //     End
        //     Block (Containing dynamic array's)
        //       Many {Int32:Count, Index[]}
        //     End
        // End

        struct mft_t
        {
            file_entry_t const* getFileInfo(fileid_t index) const;
            u32                 getGDAOffset() const { return mGDAOffset; }

        private:
            u32 mTocOffset;  // Where this TOC starts (relative to itself)
            u32 mTocCount;   // How many entries this TOC has
            u32 mGDAOffset;  // The base offset in the .gda file
        };

        file_entry_t const* mft_t::getFileInfo(fileid_t id) const
        {
            const file_entry_t* table = (const file_entry_t*)((const byte*)this + mTocOffset);
            u32 const           index = id.getFileIndex();
            return (index < mTocCount) ? &table[index] : &s_invalidFileEntry;
        }

        struct toc_t
        {
            file_entry_t const* getFileInfo(fileid_t index) const;

        private:
            const mft_t* sectionArray() const { return (const mft_t*)((const byte*)this + sizeof(u32)); }
            u32          mNumSections;
            s32          mBigfileIndex;
        };

        file_entry_t const* toc_t::getFileInfo(fileid_t id) const
        {
            const u32 sectionIndex = id.getBigfileIndex();
            return (sectionIndex < mNumSections) ? sectionArray()[sectionIndex].getFileInfo(id) : &s_invalidFileEntry;
        }

        struct gda_t
        {
            bool isValid() const { return fd.isValid(); }

            nfile::file_handle_t fd;
        };

        // FDB is a file containing all the filenames of the files in the bigfile
        // Int32: NumSections
        // Array: SectionOffset[NumSections]
        // Section:
        //   Array: FilenameOffset[NumFiles]
        // End
        // Array: {NumBytes, NumRunes, }[]
        struct fdb_t
        {
            string_t getFilename(fileid_t id) const;
            u32      mNumSections;
        };

        string_t fdb_t::getFilename(fileid_t id) const
        {
            u32 const* sectionOffsetArray = (u32*)((byte*)this + sizeof(u32));
            u32 const  sectionOffset      = sectionOffsetArray[id.getBigfileIndex()];
            u32 const* section            = (u32*)((byte*)this + sectionOffset);
            u32 const  filenameOffset     = section[id.getFileIndex()];
            u32 const* filename           = (u32*)((byte*)section + filenameOffset);
            u32 const  numBytes           = filename[0];
            u32 const  numRunes           = filename[1];
            return charon::string_t(numBytes, numRunes, (const char*)&filename[2]);
        }

        // The .bfa file containing all the files, uses the .mft file to obtain the
        // offset in the .bfa file for a file.

        bigfile_t::bigfile_t()
        {
            mTOC = nullptr;
            mFDB = nullptr;
            mGDA = nullptr;
        }

        s32 bigfile_t::open(alloc_t* allocator, const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename)
        {
            close(allocator);

            mGDA = g_allocate<gda_t>(allocator);

            mGDA->fd = file_open(bigfileFilename, nfile::file_mode_t::FILE_MODE_READ);
            if (mGDA->isValid())
            {
                nfile::file_handle_t gdt = nfile::file_open(bigTocFilename, nfile::file_mode_t::FILE_MODE_READ);
                if (gdt.isValid())
                {
                    const s64 fileSize = nfile::file_size(gdt);
                    mTOC               = (toc_t*)allocator->allocate((u32)math::g_align(fileSize, 4));
                    nfile::file_read(gdt, (u8*)mTOC, (u32)fileSize);
                    nfile::file_close(gdt);
                }

#if !defined(_SUBMISSION)
                nfile::file_handle_t namesFile = nfile::file_open(bigDatabaseFilename, nfile::file_mode_t::FILE_MODE_READ);
                if (namesFile.isValid())
                {
                    const s64 fileSize = nfile::file_size(namesFile);
                    mFDB               = (fdb_t*)allocator->allocate((u32)math::g_align(fileSize, 4));
                    nfile::file_read(namesFile, (u8*)mFDB, (u32)fileSize);
                    nfile::file_close(namesFile);
                }
                else
                {
                    log_t::writeLine(log_t::ERROR, "Error : Loading file name database failed!");
                }
#endif
            }
            else
            {
                return -1;
            }

            return mIndex;
        }

        void bigfile_t::close(alloc_t* allocator)
        {
            if (mGDA->isValid())
            {
                nfile::file_close(mGDA->fd);
                g_deallocate(allocator, mGDA);
                mGDA = nullptr;
            }
            if (mTOC != nullptr)
            {
                allocator->deallocate(mTOC);
                mTOC = nullptr;
            }
            if (mFDB != nullptr)
            {
                allocator->deallocate(mFDB);
                mFDB = nullptr;
            }
        }

        bool bigfile_t::exists(fileid_t id) const
        {
            file_entry_t const* f = mTOC->getFileInfo(id);
            return f != nullptr && f->isValid();
        }

        file_entry_t const* bigfile_t::file(fileid_t id) const { return mTOC->getFileInfo(id); }

        charon::string_t bigfile_t::filename(fileid_t id) const { return mFDB != nullptr ? mFDB->getFilename(id) : charon::string_t(); }

        bool bigfile_t::isEqual(fileid_t firstId, fileid_t secondId) const
        {
            file_entry_t const* f1 = mTOC->getFileInfo(firstId);
            file_entry_t const* f2 = mTOC->getFileInfo(secondId);
            if (f1->isValid() && f2->isValid())
                return (f1->getFileOffset() == f2->getFileOffset());
            return false;
        }

        s64 bigfile_t::fileRead(fileid_t id, void* destination) const
        {
            file_entry_t const* f = mTOC->getFileInfo(id);
            if (!f->isValid())
                return -1;
            return fileRead(id, 0, f->getFileSize(), destination);
        }

        s64 bigfile_t::fileRead(fileid_t id, s32 size, void* destination) const { return fileRead(id, 0, size, destination); }
        s64 bigfile_t::fileRead(fileid_t id, s32 offset, s32 size, void* destination) const
        {
            file_entry_t const* f = mTOC->getFileInfo(id);
            if (!f->isValid())
                return -1;

            const s64 seekpos = f->getFileOffset() + offset;
            nfile::file_seek(mGDA->fd, seekpos, nfile::seek_mode_t::SEEK_MODE_BEG);
            return nfile::file_read(mGDA->fd, (u8*)destination, size);
        }
    }  // namespace charon
}  // namespace ncore
