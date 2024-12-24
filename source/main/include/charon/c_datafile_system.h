#ifndef __CHARON_BIG_FILE_MANAGER_H__
#define __CHARON_BIG_FILE_MANAGER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "charon/c_object.h"

namespace ncore
{
    class alloc_t;

    namespace charon
    {
        struct gda_t;
        struct toc_t;
        struct fdb_t;
        struct hdb_t;

        class datafile_archive_t;
        class datafile_system_imp_t;

        struct file_entry_t
        {
            file_entry_t()
                : mFileOffset(0)
                , mFileSize(0)
            {
            }

            inline u64 getFileSize() const { return (u64)(mFileSize << 6); }
            inline u64 getFileOffset() const { return (u64)(mFileOffset << 6); }

            inline bool isValid() const { return (mFileSize != 0 && mFileOffset != 0); }
            inline bool isCompressed() const { return (mFileSize & 0x1) != 0 ? true : false; }

        private:
            u32 const mFileOffset;  // FileOffset = mFileOffset * 64
            u32 const mFileSize;    //
        };

        static const file_entry_t s_invalidFileEntry;

        class datafile_reader_t
        {
        public:
            virtual ~datafile_reader_t() {}
            virtual s64   fileRead(fileid_t id, void* destination) const                       = 0;  // Read whole file in destination
            virtual s64   fileRead(fileid_t id, s32 size, void* destination) const             = 0;  // Read part of file header in destination
            virtual s64   fileRead(fileid_t id, s32 offset, s32 size, void* destination) const = 0;  // Read part of file in destination
            virtual void* fileRead(fileid_t id, alloc_t* allocator) const                      = 0;  // Read whole file in memory allocated using allocator
        };

        u8* g_PatchPointers(u8* data)
        {
            s32* head = (s32*)data;
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
            return data + 16;
        }

        template <typename T>
        bool g_LoadObject(fileid_t id, T*& object, datafile_reader_t const* bigfile, alloc_t* allocator)
        {
            if (object != nullptr)
                return true;

            void* mem = bigfile->fileRead(id, allocator);
            if (mem == nullptr)
            {
                object = nullptr;
                return false;
            }

            object = (T*)g_PatchPointers((u8*)mem);
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

        template <typename T>
        bool g_LoadDataFile(datafile_t<T>& data_file, datafile_reader_t const* bigfile, alloc_t* allocator)
        {
            return g_LoadObject(data_file.m_fileid, data_file.m_ptr, bigfile, allocator);
        }

        class datafile_system_t
        {
        public:
            void                init(alloc_t* allocator, s32 maxNumBigfiles);        // Initialize the bigfile manager
            void                teardown();                                          // Shutdown the big
            bool                exists(fileid_t id) const;                           // Return True if file-id exists
            bool                isEqual(fileid_t firstId, fileid_t secondId) const;  // Return True if both fileIds reference the same physical file
            file_entry_t const* file(fileid_t id) const;                             // Return FileEntry associated with file id
            string_t            filename(fileid_t id) const;                         // Return Filename associated with file id
            datafile_reader_t*  reader() const;                                      // Get the reader for the bigfile

        private:
            datafile_system_imp_t* mImp;
        };

    }  // namespace charon
}  // namespace ncore

#endif
