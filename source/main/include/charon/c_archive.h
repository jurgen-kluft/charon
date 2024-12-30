#ifndef __CHARON_ARCHIVE_H__
#define __CHARON_ARCHIVE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "charon/c_gamedata.h"

namespace ncore
{
    class alloc_t;

    namespace charon
    {
        struct archive_info_t
        {
            inline string_t const& getArchiveData() const { return m_Data; }
            inline string_t const& getArchiveToc() const { return m_Toc; }
            inline string_t const& getArchiveFdb() const { return m_Fdb; }
            inline string_t const& getArchiveHdb() const { return m_Hdb; }

        private:
            string_t m_Data;
            string_t m_Toc;
            string_t m_Fdb;
            string_t m_Hdb;
        };

        class archive_loader_t
        {
        public:
            virtual ~archive_loader_t() {}

            template <typename T>
            bool load(fileid_t id, T*& object, alloc_t* allocator)
            {
                if (object != nullptr)
                    return true;

                void* mem = v_fileRead(id, allocator);
                if (mem == nullptr)
                {
                    object = nullptr;
                    return false;
                }

                object = (T*)patch((u8*)mem);
                return true;
            }

            template <typename T>
            void unload(T*& object, alloc_t* allocator)
            {
                if (object != nullptr)
                {
                    g_deallocate(allocator, object);
                    object = nullptr;
                }
            }

            static u8* patch(u8* data)
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

        protected:
            virtual s64   v_fileRead(fileid_t id, s32 offset, s32 size, void* destination) const = 0;  // Read part of file in destination
            virtual void* v_fileRead(fileid_t id, alloc_t* allocator) const                      = 0;  // Read whole file in memory allocated using allocator
        };

        class archive_t
        {
        public:
            struct file_t
            {
                inline u64 getFileSize() const { return (u64)(mFileSize); }
                inline u64 getFileOffset() const { return (u64)((mFileOffset & 0x7FFFFFFF) << 6); }

                inline bool isValid() const { return mFileSize > 0; }
                inline bool isCompressed() const { return (mFileOffset & 0x80000000) != 0 ? true : false; }

                u32 mFileOffset;  // FileOffset = mFileOffset * 64
                u32 mFileSize;    //
            };

            struct section_t
            {
                u32         m_ArchiveIndex;
                u32         m_ArchiveOffset;
                u32         m_Dummy;
                u32         m_ItemCount;
                void const* m_ItemArray;
            };

            void              init(alloc_t* allocator, s32 maxNumArchives);  // Initialize
            void              teardown();                                    // Shutdown
            bool              exists(fileid_t id) const;                     // Return True if file-id exists
            file_t const*     fileitem(fileid_t id) const;                   // Return Item associated with file id
            string_t          filename(fileid_t id) const;                   // Return Filename associated with file id
            archive_loader_t* loader() const;                                // Get the loader interface

            class imp_t;
            imp_t* mImp;
        };
    }  // namespace charon
}  // namespace ncore

#endif
