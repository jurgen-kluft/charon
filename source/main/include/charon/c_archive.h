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
            void* load_datafile(fileid_t fileid) { return v_load_datafile(fileid); }
            void* load_dataunit(u32 dataunit_index) { return v_load_dataunit(dataunit_index); }

            template <typename T>
            void* get_datafile_ptr(fileid_t fileid)
            {
                return (T*)v_get_datafile_ptr(fileid);
            }

            template <typename T>
            void* get_dataunit_ptr(u32 dataunit_index)
            {
                return (T*)v_get_dataunit_ptr(dataunit_index);
            }

            template <typename T>
            void unload_datafile(T*& object)
            {
                v_unload_datafile(object);
            }

            template <typename T>
            void unload_dataunit(T*& object)
            {
                v_unload_dataunit(object);
            }

        protected:
            virtual void* v_get_datafile_ptr(fileid_t fileid)    = 0;
            virtual void* v_get_dataunit_ptr(u32 dataunit_index) = 0;
            virtual void* v_load_datafile(fileid_t fileid)       = 0;
            virtual void* v_load_dataunit(u32 dataunit_index)    = 0;
            virtual void  v_unload_datafile(fileid_t fileid)     = 0;
            virtual void  v_unload_dataunit(u32 dataunit_index)  = 0;
        };

        class archive_t
        {
        public:
            static archive_t* s_instance;
            static void       s_setup(alloc_t* allocator, s32 maxNumArchives);
            static void       s_teardown();

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
                u32 m_ArchiveIndex;
                u32 m_ArchiveOffset;
                u32 m_ItemArrayCount;
                u32 m_ItemArrayOffset;

                template <typename T>
                inline T const* getItemArray() const
                {
                    return (T const*)((byte*)this + m_ItemArrayOffset);
                }
            };

            void              init(alloc_t* allocator, s32 maxNumArchives);  // Initialize
            void              teardown();                                    // Shutdown
            bool              exists(fileid_t id) const;                     // Return True if file-id exists
            file_t const*     fileitem(fileid_t id) const;                   // Return Item associated with file id
            string_t          filename(fileid_t id) const;                   // Return Filename associated with file id
            archive_loader_t* loader() const;                                // Get the loader interface
        };

    }  // namespace charon
}  // namespace ncore

#endif
