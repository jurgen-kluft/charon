#include "ccore/c_target.h"
#include "cbase/c_hash.h"
#include "cbase/c_log.h"
#include "cbase/c_va_list.h"

#include "charon/c_datafile_system.h"
#include "charon/c_object.h"

namespace ncore
{
    namespace charon
    {
        struct datafile_section_t
        {
            u32           m_data_file_count;
            file_entry_t* m_data_file_array;
            void**        m_data_ptr_array;
        };

        class gamedata_imp_t : public gamedata_t
        {
        public:
            void* get_datafile_ptr(fileid_t fileid) override;
            void* get_dataunit_ptr(u32 dataunit_index) override;
            void  load_datafile(loader_t& loader, fileid_t fileid) override;
            void  load_dataunit(loader_t& loader, u32 dataunit_index) override;

            void**              m_dataunit_ptrs;
            fileid_t            m_dataunit_fileids;
            u32                 m_datafile_section_count;
            datafile_section_t* m_datafile_section_array;
        };

        void g_initialize_gamedata() {}

    }  // namespace charon
}  // namespace ncore
