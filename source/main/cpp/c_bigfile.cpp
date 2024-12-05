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
        // Constraints:
        //     Maximum file size = 2GB * 32 = 64GB
        //     Maximum file offset = 2GB * 32 = 64GB
        //     Compression = High Bit in Offset
        //     IsArray = High Bit in Size

        struct file_children_t
        {
            file_children_t()
                : mNumEntries(0)
                , mChildren(nullptr)
            {
            }
            file_children_t(u32 numEntries, u32 const* children)
                : mNumEntries(numEntries)
                , mChildren(children)
            {
            }
            inline u32                size() const { return mNumEntries; }
            inline u32                operator[](u32 index) const { return mChildren[index]; }
            inline const array_t<u32> array() const { return array_t<u32>(mNumEntries, mChildren); }

        private:
            const u32  mNumEntries;
            const u32* mChildren;
        };

        struct file_entry_t
        {
            inline u64 getFileSize() const { return (u64)(mFileSize & ~0x80000000) << 5; }
            inline u64 getFileOffset() const { return (u64)(mFileOffset & ~0x80000000) << 5; }

            inline bool isValid() const { return (mFileSize != 0xffffffff && mFileOffset != 0xffffffff); }
            inline bool isCompressed() const { return (mFileChildrenOffset & 0x40000000) != 0 ? true : false; }

            inline bool            hasChildren() const { return (mFileChildrenOffset & 0x80000000) != 0 ? true : false; }
            inline file_children_t getChildrenArray() const
            {
                // Offsets are relative to its own position
                u32 const* data = (u32 const*)((u32 const*)this + (mFileChildrenOffset & 0x3fffffff));
                return file_children_t(data[0], data + 1);
            }

        private:
            u32 mFileOffset;          // FileOffset = mFileOffset * 64
            u32 mFileSize;            // FileSize
            u32 mFileChildrenOffset;  // 0x80000000 = Has children, 0x40000000 = Is compressed
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
            file_entry_t    getFileInfo(fileid_t index) const;
            s32             getFileCount(s32 index) const;
            file_children_t getFileChildren(s32 index) const;

        private:
            const u32 mTocOffset;  // Where this TOC starts (relative to itself)
            const u32 mTocCount;   // How many entries this TOC has
        };

        file_entry_t mft_t::getFileInfo(fileid_t id) const
        {
            const file_entry_t* table = (const file_entry_t*)((const byte*)this + mTocOffset);
            return table[id & 0xffffffff];
        }

        s32 mft_t::getFileCount(s32 index) const
        {
            const file_entry_t* table = (const file_entry_t*)((const byte*)this + mTocOffset);
            const file_entry_t& entry = table[index & 0xffffffff];
            return entry.hasChildren() ? entry.getChildrenArray().size() : 0;
        }

        file_children_t mft_t::getFileChildren(s32 index) const
        {
            const file_entry_t* table = (const file_entry_t*)((const byte*)this + mTocOffset);
            const file_entry_t& entry = table[index & 0xffffffff];
            return entry.hasChildren() ? entry.getChildrenArray() : file_children_t();
        }

        struct toc_t
        {
            s32          numSections() const { return mNumSections; }
            const mft_t* getSection(s32 index) const { return &sectionArray()[index & 0xffffffff]; }

        private:
            const mft_t* sectionArray() const { return (const mft_t*)((const byte*)this + sizeof(u32)); }
            const u32    mNumSections;
        };

        // Table - Filename offsets
        // Table - Filenames
        //     Char[] - Filename
        //
        // Note: This looks very much like a string table, merge ?
        struct fdb_t
        {
            string_t    filename(fileid_t index) const;
            s32         mNumEntries;
            u32*        mOffsets;
            u32*        mCharLengths;
            u32*        mByteLengths;
            const char* mStrings;
        };

        string_t fdb_t::filename(fileid_t id) const
        {
            // const s32* offset = (const s32*)((ptr_t)this + sizeof(fdb_t));
            // return (const char*)((ptr_t)this + sizeof(fdb_t) + sizeof(s32) * mNumEntries + offset[id & 0xffffffff]);
            return string_t(mByteLengths[id & 0xffffffff], mCharLengths[id & 0xffffffff], mStrings + mOffsets[id & 0xffffffff]);
        }

        // The .bfa file containing all the files, uses the .mft file to obtain the
        // offset in the .bfa file for a file.

        bigfile_t::bigfile_t(alloc_t* allocator)
        {
            mAlloc = allocator;
            mMFT   = nullptr;
            mFDB   = nullptr;
            mGDA   = nullptr;
        }

        bool bigfile_t::open(const char* bigfileFilename, const char* bigTocFilename, const char* bigDatabaseFilename)
        {
            close();

            mBigfile = file_open(bigfileFilename, nfile::file_mode_t::FILE_MODE_READ);
            if (mBigfile.isValid())
            {
                nfile::file_handle_t gdt = nfile::file_open(bigTocFilename, nfile::file_mode_t::FILE_MODE_READ);
                if (gdt.isValid())
                {
                    const s32 fileSize = (s32)nfile::file_size(gdt);
                    mMFT               = (mft_t*)mAlloc->allocate(math::align(fileSize, 4));
                    nfile::file_read(gdt, (u8*)mMFT, (u32)fileSize);
                    nfile::file_close(gdt);
                }

#if !defined(_SUBMISSION)
                nfile::file_handle_t namesFile = nfile::file_open(bigDatabaseFilename, nfile::file_mode_t::FILE_MODE_READ);
                if (namesFile.isValid())
                {
                    const s32 size = (s32)nfile::file_size(namesFile);
                    mFDB           = (fdb_t*)mAlloc->allocate(math::align(size, 4));
                    nfile::file_read(namesFile, (u8*)mFDB, (u32)size);
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
            file_entry_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.isValid();
        }

        bool bigfile_t::isCompressed(fileid_t id) const
        {
            file_entry_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.isCompressed();
        }

        charon::string_t bigfile_t::filename(fileid_t id) const
        {
#if defined(_SUBMISSION)
            return string_t();
#else
            return mFDB->filename(id);
#endif
        }

        s64 bigfile_t::size(fileid_t id) const
        {
            file_entry_t f = mMFT->getFileInfo(mBasePtr, id);
            return f.getFileSize();
        }

        bool bigfile_t::isEqual(fileid_t firstId, fileid_t secondId) const
        {
            file_entry_t f1 = mMFT->getFileInfo(mBasePtr, firstId);
            file_entry_t f2 = mMFT->getFileInfo(mBasePtr, secondId);
            if (f1.isValid() && f2.isValid())
                return (f1.getFileOffset() == f2.getFileOffset());
            return false;
        }

        s64 bigfile_t::read(fileid_t id, void* destination) const
        {
            file_entry_t f = mMFT->getFileInfo(mBasePtr, id);
            if (!f.isValid())
                return -1;
            return read(id, 0, f.getFileSize(), destination);
        }
        s64 bigfile_t::read(fileid_t id, s32 size, void* destination) const { return read(id, 0, size, destination); }
        s64 bigfile_t::read(fileid_t id, s32 offset, s32 size, void* destination) const
        {
            file_entry_t f = mMFT->getFileInfo(mBasePtr, id);
            if (!f.isValid())
                return -1;

            const s64 seekpos = f.getFileOffset() + offset;
            nfile::file_seek(mBigfile, seekpos, nfile::seek_mode_t::SEEK_MODE_BEG);

            const s32 numBytesRead = (s32)(file_read(mBigfile, (u8*)destination, size));
            return numBytesRead;
        }
    }  // namespace charon
}  // namespace ncore
