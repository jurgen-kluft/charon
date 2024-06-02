#ifndef __CHARON_ROOT_H__
#define __CHARON_ROOT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    class alloc_t;

    namespace ngd
    {
        class object_t;
        class strtable_t;

        class root_t
        {
        public:
            root_t(alloc_t* allocator);

            const object_t*   root() const { return m_data->m_root; }
            const strtable_t* stringtable() const { return m_data->m_stringTable; }

            void load(const char* dataFilename, const char* relocFilename);
            void reload(const char* dataFilename, const char* relocFilename);
            void unload();

        protected:
            alloc_t*    m_Allocator;
            strtable_t* m_typeTable;
            struct dataheader_t
            {
                strtable_t* m_stringTable;
                object_t*   m_root;
            };
            u32           m_dataSize;
            dataheader_t* m_data;
        };
    }  // namespace ngd
}  // namespace ncore

#endif  /// __CHARON_ROOT_H__
