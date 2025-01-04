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
        u8* g_patch(dataunit_header_t* data);

        class archive_t
        {
        public:
            static archive_t* s_instance;
            static void       s_setup(alloc_t* allocator, s32 maxNumDataUnits, s32 maxNumDataArchives);
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

            bool              exists(fileid_t const& id) const;    // Return True if file-id exists
            file_t const*     fileitem(fileid_t const& id) const;  // Return Item associated with file id
            string_t          filename(fileid_t const& id) const;  // Return Filename associated with file id
            archive_loader_t* loader() const;                      // Get the loader interface
        };

    }  // namespace charon
}  // namespace ncore

#endif
