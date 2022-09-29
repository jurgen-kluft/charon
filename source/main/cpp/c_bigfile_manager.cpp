#include "cgamedata/c_bigfile_manager.h"
#include "cbase/c_log.h"
#include "cbase/c_integer.h"

namespace ncore
{
    namespace ngd
    {
        struct fileinfo_t
        {
            inline u64 getfileOffset() const { return (u64)fileOffset(); }
            inline u32 getfileSize() const { return fileSize(); }

            inline bool isValid() const { return (mFileOffset != 0xffffffff); }
            inline bool isCompressed() const { return (mFileSize & 0x80000000) != 0 ? true : false; }

            inline u32 fileSize() const { return mFileSize & 0x7FFFFFFF; }
            inline u32 fileOffset() const { return mFileOffset; }

        private:
            u32 mFileOffset;
            u32 mFileSize;
        };

        ///         Int32 - Flags
        ///         Int32 - Number of entries
        ///         Table - Entries
        ///             Int32-OffsetInArchive, Int32-SizeOfFile
        struct MFT
        {
            const fileinfo_t* at(int index) const;
            int               mNumEntries;
        };

        const fileinfo_t* MFT::at(int index) const
        {
            const fileinfo_t* table = (const fileinfo_t*)((int)this + sizeof(MFT));
            return table + index;
        }

        ///         Table - Filename offsets
        ///         Table - Filenames
        ///             Char[] - Filename
        ///
        struct FDB
        {
            const char* filename(int index) const;
            int         mNumEntries;
        };

        const char* FDB::filename(int index) const
        {
            const int* offset = (const int*)((int)this + sizeof(FDB));
            return (const char*)((int)this + sizeof(FDB) + sizeof(int) * mNumEntries + offset[index]);
        }

        //         The .arc file containing all the files, uses the .mft file to obtain the
        //         offset in the .arc file for a file.

        BigFileManager::BigFileManager()
        {
            mMFT     = nullptr;
            mFDB     = nullptr;
            mBigfile = filehandle_t();
        }

        bool BigFileManager::open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename)
        {
            close();

            mBigfile = file_open(bigfileFilename);

            if (mBigfile != nullptr)
            {
                filehandle_t gdt = file_open(bigTocFilename);
                if (gdt != nullptr)
                {
                    const int fileSize = (int)file_size(gdt);
                    mMFT               = (MFT*)Memory::alloc(math::align(fileSize, 4));
                    file_read(gdt, (u8*)mMFT, (u32)fileSize);
                    file_close(gdt);
                    delete gdt;
                }

#if !defined(_SUBMISSION)
                filehandle_t namesFile = file_open(bigDatabaseFilename);
                if (namesFile != nullptr)
                {
                    const int size = (int)file_size(namesFile);
                    mFDB           = (FDB*)new char[math::align(size, 4)];
                    file_read(namesFile, (u8*)mFDB, (u32)size);
                    file_close(namesFile);
                    delete namesFile;
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
            if (mBigfile != nullptr)
            {
                mBigfile->close();
                UMemory::SafeDelete(mBigfile);
                mBigfile = nullptr;
            }

            Memory::free(mMFT);
            mMFT = nullptr;

#if !defined(_SUBMISSION)
            Memory::free(mFDB);
            mFDB = nullptr;
#endif
        }

        bool BigFileManager::exists(fileid_t id) const { return mMFT->at(id) != nullptr ? true : false; }

        bool BigFileManager::isCompressed(fileid_t id) const
        {
            const fileinfo_t* f = mMFT->at(id);
            return f->isCompressed();
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

        int BigFileManager::size(fileid_t id) const
        {
            const fileinfo_t* f = mMFT->at(id);
            return f->fileSize();
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
                const fileinfo_t* f1 = mMFT->at(firstId);
                const fileinfo_t* f2 = mMFT->at(secondId);
                if (f1 != nullptr && f2 != nullptr)
                {
                    res = f1->fileOffset() == f2->fileOffset();
                }
            }
            return res;
        }

        int BigFileManager::read(fileid_t id, void* destination)
        {
            const fileinfo_t* f = mMFT->at(id);
            if (f == nullptr)
                return -1;

            return read(id, 0, f->fileSize(), destination);
        }

        int BigFileManager::read(fileid_t id, int size, void* destination) { return read(id, 0, size, destination); }

        int BigFileManager::read(fileid_t id, int offset, int size, void* destination)
        {
            const fileinfo_t* f = mMFT->at(id);
            if (f == nullptr)
                return -1;

            u64 const seekpos = f->getfileOffset() + offset;
            mBigfile->seek(seekpos);

            const int numBytesRead = (int)(mBigfile->read(destination, size));

            return numBytesRead;
        }
    } // namespace ngd
} // namespace ncore