#include "cbase/c_allocator.h"
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
        //     Int32: Number of MFT's
        //     Array
        //       {Int32:Number of entries, Int32:Offset to TOC, Int32:Base offset in Bigfile Archive}
        //     End

        // Layout of one MFT
        //     Array: FileId's
        //       {Int32:FileOffset in Archive, Int32:Size of file}
        //     End
        //     Block (Containing dynamic array's)
        //       Many {Int32:Length, FileId[]}
        //     End
        // End

        struct MFT
        {
            fileinfo_t   getFileInfo(const void* base, s32 index) const;
            s32          getFileIdCount(s32 index) const;
            array_t<u32> getFileIdArray(s32 index) const;

            u32 mNumEntries;         // How many entries this TOC has
            u32 mTocOffset;          // Where this TOC starts
            u32 mFileDataBaseOffset; // 
        };

        fileinfo_t MFT::getFileInfo(const void* base, s32 index) const
        {
            const fileinfo_t* table = (const fileinfo_t*)((ptr_t)base + mTocOffset);
            return table[index];
        }

        // Table - Filename offsets
        // Table - Filenames
        //     Char[] - Filename
        //
        struct FDB
        {
            const char* filename(s32 index) const;
            s32         mNumEntries;
        };

        const char* FDB::filename(s32 index) const
        {
            const s32* offset = (const s32*)((s32)this + sizeof(FDB));
            return (const char*)((s32)this + sizeof(FDB) + sizeof(s32) * mNumEntries + offset[index]);
        }

        //         The .arc file containing all the files, uses the .mft file to obtain the
        //         offset in the .arc file for a file.

        BigFileManager::BigFileManager(alloc_t* allocator)
        {
            mAlloc   = allocator;
            mMFT     = nullptr;
            mFDB     = nullptr;
            mBigfile = file_handle_t();
        }

        bool BigFileManager::open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename)
        {
            close();

            mBigfile = file_open(bigfileFilename, file_mode_t::FILE_MODE_READ);
            if (mBigfile.isValid())
            {
                file_handle_t gdt = file_open(bigTocFilename, file_mode_t::FILE_MODE_READ);
                if (gdt.isValid())
                {
                    const s32 fileSize = (s32)file_size(gdt);
                    mMFT               = (MFT*)mAlloc->allocate(math::align(fileSize, 4));
                    file_read(gdt, (u8*)mMFT, (u32)fileSize);
                    file_close(gdt);
                }

#if !defined(_SUBMISSION)
                file_handle_t namesFile = file_open(bigDatabaseFilename, file_mode_t::FILE_MODE_READ);
                if (namesFile.isValid())
                {
                    const s32 size = (s32)file_size(namesFile);
                    mFDB           = (FDB*)mAlloc->allocate(math::align(size, 4));
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

        void BigFileManager::close()
        {
            if (mBigfile.isValid())
            {
                file_close(mBigfile);
                mBigfile = nullptr;
            }

            mAlloc->deallocate(mMFT);
            mMFT = nullptr;

#if !defined(_SUBMISSION)
            mAlloc->deallocate(mFDB);
            mFDB = nullptr;
#endif
        }

        bool BigFileManager::exists(fileid_t id) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.isValid();
        }

        bool BigFileManager::isCompressed(fileid_t id) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.isCompressed();
        }

        const char* BigFileManager::filename(fileid_t id) const
        {
#if defined(_SUBMISSION)
            return "Null";
#else
            const char* f = mFDB->filename(id);
            return f;
#endif
        }

        s32 BigFileManager::size(fileid_t id) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.getFileSize();
        }

        bool BigFileManager::isEqual(fileid_t firstId, fileid_t secondId) const
        {
            bool res = false;
            if (firstId == secondId)
            {
                res = true;
            }
            else
            {
                fileinfo_t f1 = mMFT->getFileInfo(mBasePtr, firstId);
                fileinfo_t f2 = mMFT->getFileInfo(mBasePtr, secondId);
                if (f1.isValid() && f2.isValid())
                {
                    res = f1.getFileOffset() == f2.getFileOffset();
                }
            }
            return res;
        }

        s32 BigFileManager::read(fileid_t id, void* destination) const
        {
            fileinfo_t f = mMFT->getFileInfo(mBasePtr, id);
            if (!f.isValid())
                return -1;

            return read(id, 0, f.getFileSize(), destination);
        }

        s32 BigFileManager::read(fileid_t id, s32 size, void* destination) const { return read(id, 0, size, destination); }

        s32 BigFileManager::read(fileid_t id, s32 offset, s32 size, void* destination) const
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