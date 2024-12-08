#ifndef __CHARON_BIG_FILE_MANAGER_H__
#define __CHARON_BIG_FILE_MANAGER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cfile/c_file.h"
#include "charon/c_object.h"

namespace ncore
{
    class alloc_t;

    namespace charon
    {
        typedef s64 fileid_t;

        struct gda_t;
        struct toc_t;
        struct fdb_t;
        struct hdb_t;

        struct bigfile_t;

        struct file_entry_t
        {
            file_entry_t()
                : mFileOffset(0)
                , mFileSize(0)
                , mFileChildrenOffset(0)
            {
            }

            inline u64 getFileSize() const { return (u64)(mFileSize << 6); }
            inline u64 getFileOffset() const { return (u64)(mFileOffset << 6); }

            inline bool isValid() const { return (mFileSize != 0 && mFileOffset != 0); }
            inline bool isCompressed() const { return (mFileChildrenOffset & 0x1) != 0 ? true : false; }
            inline bool hasChildren() const { return (mFileChildrenOffset & 0x2) != 0 ? true : false; }
            inline s32  numChildren() const
            {
                if (hasChildren())
                {
                    s32 const* data = (s32 const*)((s8 const*)this + (s32)(mFileChildrenOffset & 0xffffffffc));
                    return data[0];
                }
                return 0;
            }
            inline file_entry_t const* getChild(u32 index) const
            {
                s32 const* data   = (s32 const*)((s8 const*)this + (s32)(mFileChildrenOffset & 0xfffffffc));
                s32 const  offset = data[index + 1];
            }

        private:
            u32 const mFileOffset;          // FileOffset = mFileOffset * 64
            u32 const mFileSize;            //
            u32 const mFileChildrenOffset;  //
        };

        struct bigfile_reader_t
        {
            virtual ~bigfile_reader_t() {}
            virtual s64   fileRead(fileid_t id, void* destination) const                       = 0;  // Read whole file in destination
            virtual s64   fileRead(fileid_t id, s32 size, void* destination) const             = 0;  // Read part of file header in destination
            virtual s64   fileRead(fileid_t id, s32 offset, s32 size, void* destination) const = 0;  // Read part of file in destination
            virtual void* fileRead(fileid_t id, alloc_t* allocator) const                      = 0;  // Read whole file in memory allocated using allocator
        };

        template <typename T>
        bool g_LoadObject(fileid_t id, T*& object, bigfile_reader_t const* bigfile, alloc_t* allocator)
        {
            if (object != nullptr)
                return true;

            void* mem = bigfile->fileRead(id, allocator);
            if (mem == nullptr)
                return false;

            object = new (mem) T;
            return true;
        }

        template <typename T>
        void g_UnloadObject(T*& object, alloc_t* allocator)
        {
            if (object != nullptr)
            {
                g_deallocate(allocator, object);
                object = nullptr;
            }
        }

        struct bigfile_manager_t
        {
            void                init(alloc_t* allocator, const char* dirpath);       // Initialize the bigfile manager
            void                teardown();                                          // Shutdown the big
            bool                exists(fileid_t id) const;                           // Return True if file exists in Archive
            bool                isEqual(fileid_t firstId, fileid_t secondId) const;  // Return True if both fileIds reference the same physical file
            file_entry_t const* file(fileid_t id) const;                             // Return FileEntry associated with file id
            string_t            filename(fileid_t id) const;                         // Return Filename associated with file id
            bigfile_reader_t*   reader() const;                                      // Get the reader for the bigfile

        private:
            alloc_t*          mAllocator;
            s32               mNumBigfiles;
            bigfile_t**       mBigfiles;
            bigfile_reader_t* mReader;
        };

    }  // namespace charon
}  // namespace ncore

#endif
