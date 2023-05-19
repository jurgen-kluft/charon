#include "ccore/c_allocator.h"
#include "cbase/c_log.h"
#include "cbase/c_integer.h"
#include "cfile/c_file.h"

#include "cgamedata/c_bigfile_manager.h"
#include "cgamedata/c_object.h"

namespace ncore
{
    namespace ngd
    {
        struct fileinfo_t
        {
            inline u64 getFileOffset() const { return (u64)fileOffset(); }
            inline u32 getFileSize() const { return fileSize(); }

            inline bool isValid() const { return (mFileSize != 0xffffffff); }
            inline bool isCompressed() const { return (mFileSize & 0x80000000) != 0 ? true : false; }

            inline bool               hasFileIdArray() const { return (mFileSize & 0x40000000) != 0 ? true : false; }
            inline const array_t<u32> getFileIdArray(const void* base)
            {
                u32 const* data = ((u32 const*)base + mFileOffset);
                return array_t<u32>(data[0], data + 1);
            }

        private:
            inline u32 fileSize() const { return mFileSize & ~0xC0000000; }
            inline u64 fileOffset() const { return ((u64)mFileOffset << 5); }

            u32 mFileOffset;
            u32 mFileSize;
        };

        // Header
        //     Int32: Total number of FileId's
        //     Int32: Number of mft_t's
        //     Array
        //       {Int32:Number of entries, Int32:Offset to TOC, Int32:Base offset in Bigfile Archive}
        //     End

        // Layout of one mft_t
        //     Array: FileId's
        //       {Int32:FileOffset in Archive, Int32:Size of file}
        //     End
        //     Block (Containing dynamic array's)
        //       Many {Int32:Length, FileId[]}
        //     End
        // End

        struct mft_t
        {
            fileinfo_t   getFileInfo(const void* base, fileid_t index) const;
            s32          getFileIdCount(s32 index) const;
            array_t<u32> getFileIdArray(s32 index) const;

            u32 mNumEntries;         // How many entries this TOC has
            u32 mTocOffset;          // Where this TOC starts
            u32 mFileDataBaseOffset; //
        };

        fileinfo_t mft_t::getFileInfo(const void* base, fileid_t id) const
        {
            const fileinfo_t* table = (const fileinfo_t*)((ptr_t)base + mTocOffset);
            return table[id & 0xffffffff];
        }

        // Table - Filename offsets
        // Table - Filenames
        //     Char[] - Filename
        //
        struct fdb_t
        {
            const char* filename(fileid_t index) const;
            s32         mNumEntries;
        };

        const char* fdb_t::filename(fileid_t id) const
        {
            const s32* offset = (const s32*)((ptr_t)this + sizeof(fdb_t));
            return (const char*)((ptr_t)this + sizeof(fdb_t) + sizeof(s32) * mNumEntries + offset[id & 0xffffffff]);
        }

        //         The .bfa file containing all the files, uses the .mft file to obtain the
        //         offset in the .bfa file for a file.

        bigfile_t* bigfile_t::instance = nullptr;

        bigfile_t::bigfile_t(alloc_t* allocator)
        {
            mAlloc   = allocator;
            mMFT     = nullptr;
            mFDB     = nullptr;
            mBigfile = file_handle_t();
        }

        bool bigfile_t::open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename)
        {
            close();

            mBigfile = file_open(bigfileFilename, file_mode_t::FILE_MODE_READ);
            if (mBigfile.isValid())
            {
                file_handle_t gdt = file_open(bigTocFilename, file_mode_t::FILE_MODE_READ);
                if (gdt.isValid())
                {
                    const s32 fileSize = (s32)file_size(gdt);
                    mMFT               = (mft_t*)mAlloc->allocate(math::align(fileSize, 4));
                    file_read(gdt, (u8*)mMFT, (u32)fileSize);
                    file_close(gdt);
                }

#if !defined(_SUBMISSION)
                file_handle_t namesFile = file_open(bigDatabaseFilename, file_mode_t::FILE_MODE_READ);
                if (namesFile.isValid())
                {
                    const s32 size = (s32)file_size(namesFile);
                    mFDB           = (fdb_t*)mAlloc->allocate(math::align(size, 4));
                    file_read(namesFile, (u8*)mFDB, (u32)size);
                    file_close(namesFile);
                }
                else
                {
                    log_t::writeLine(log_t::ERROR, "Error : Loading file name database failed!");
                }
#endif
            }
            else
            {
                return false;
            }

            return true;
        }

        void bigfile_t::close()
        {
            if (mBigfile.isValid())
            {
                file_close(mBigfile);
                mBigfile = nullptr;
            }
            if (mMFT != nullptr)
            {
                mAlloc->deallocate(mMFT);
                mMFT = nullptr;
            }
            if (mFDB != nullptr)
            {
                mAlloc->deallocate(mFDB);
                mFDB = nullptr;
            }
        }

        bool bigfile_t::exists(fileid_t id) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.isValid();
        }

        bool bigfile_t::isCompressed(fileid_t id) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.isCompressed();
        }

        const char* bigfile_t::filename(fileid_t id) const
        {
#if defined(_SUBMISSION)
            return "Null";
#else
            const char* f = mFDB->filename(id);
            return f;
#endif
        }

        s32 bigfile_t::size(fileid_t id) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.getFileSize();
        }

        bool bigfile_t::isEqual(fileid_t firstId, fileid_t secondId) const
        {
            fileinfo_t f1 = mMFT->getFileInfo(mBasePtr, firstId);
            fileinfo_t f2 = mMFT->getFileInfo(mBasePtr, secondId);
            if (f1.isValid() && f2.isValid())
                return (f1.getFileOffset() == f2.getFileOffset());
            return false;
        }

        s32 bigfile_t::read(fileid_t id, void* destination) const 
        { 
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            if (!f.isValid())
                return -1;
            return read(id, 0, f.getFileSize(), destination); 
        }
        s32 bigfile_t::read(fileid_t id, s32 size, void* destination) const { return read(id, 0, size, destination); }
        s32 bigfile_t::read(fileid_t id, s32 offset, s32 size, void* destination) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            if (!f.isValid())
                return -1;

            const s64 seekpos = f.getFileOffset() + offset;
            file_seek(mBigfile, seekpos, seek_mode_t::SEEK_MODE_BEG);

            const s32 numBytesRead = (s32)(file_read(mBigfile, (u8*)destination, size));
            return numBytesRead;
        }
    } // namespace ngd
} // namespace ncore