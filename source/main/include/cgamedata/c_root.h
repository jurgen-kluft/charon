#ifndef __CGAMEDATA_ROOT_H__
#define __CGAMEDATA_ROOT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    namespace ngd
    {
        class object_t;
        class stringtable_t;

        class root_t
        {
        public:
            root_t();

            const object_t*      root() const { return m_data->mRoot; }
            const stringtable_t* stringtable() const { return m_data->m_stringtable; }

            void load(const char* dataFilename, const char* relocFilename);
            void reload(const char* dataFilename, const char* relocFilename);
            void unload();

        protected:
            u32            m_allocated_datasize;
            stringtable_t* m_typetable;
            struct dataheader_t
            {
                stringtable_t* m_stringtable;
                object_t*      m_root;
            };
            dataheader_t* m_data;
        };
    } // namespace ngd
} // namespace ncore

#endif /// __CGAMEDATA_ROOT_H__