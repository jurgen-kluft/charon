#include "ccore/c_target.h"
#include "cbase/c_hash.h"
#include "cbase/c_log.h"
#include "cbase/c_va_list.h"

#include "charon/c_archive.h"
#include "charon/c_gamedata.h"

namespace ncore
{
    namespace charon
    {
        namespace ngamedata
        {
            class filesystem_imp_t : public filesystem_t
            {
            public:
                void* v_get_datafile_ptr(fileid_t fileid) override;
                void* v_get_dataunit_ptr(u32 dataunit_index) override;
                void  v_load_datafile(archive_loader_t& loader, fileid_t fileid) override;
                void  v_load_dataunit(archive_loader_t& loader, u32 dataunit_index) override;

                alloc_t*               m_allocator;
                u32                    m_dataunit_count;
                u32                    m_datafile_section_count;
                void**                 m_dataunit_ptrs;
                fileid_t*              m_dataunit_fileids;
                archive_t::section_t** m_datafile_section_array;
                void***                m_datafile_section_datafile_ptr_array;
            };

            void* filesystem_imp_t::v_get_datafile_ptr(fileid_t fileid)
            {
                u32 const section_index              = fileid.getArchiveIndex();
                void**    section_datafile_ptr_array = m_datafile_section_datafile_ptr_array[section_index];
                return section_datafile_ptr_array[fileid.getFileIndex()];
            }

            void* filesystem_imp_t::v_get_dataunit_ptr(u32 dataunit_index) { return m_dataunit_ptrs[dataunit_index]; }

            void filesystem_imp_t::v_load_datafile(archive_loader_t& loader, fileid_t fileid)
            {
                u32 const section_index              = fileid.getArchiveIndex();
                void**    section_datafile_ptr_array = m_datafile_section_datafile_ptr_array[section_index];
                void*     data                       = section_datafile_ptr_array[fileid.getFileIndex()];
                if (data == nullptr)
                {
                    if (loader.load(fileid, data, m_allocator))
                    {
                        section_datafile_ptr_array[fileid.getFileIndex()] = data;
                    }
                }
            }

            void filesystem_imp_t::v_load_dataunit(archive_loader_t& loader, u32 dataunit_index)
            {
                void* data = m_dataunit_ptrs[dataunit_index];
                if (data == nullptr)
                {
                    if (loader.load(m_dataunit_fileids[dataunit_index], data, m_allocator))
                    {
                        m_dataunit_ptrs[dataunit_index] = data;
                    }
                }
            }

            void g_initialize_gamedata() {}

        }  // namespace ngamedata
    }  // namespace charon
}  // namespace ncore
