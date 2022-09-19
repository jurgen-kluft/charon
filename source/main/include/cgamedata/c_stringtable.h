#ifndef __CGAMEDATA_HASHED_STRINGTABLE_H__
#define __CGAMEDATA_HASHED_STRINGTABLE_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    namespace ngd
    {
        // The string table, containing a hash array, offset array and the actual string data
        class stringtable_t
        {
        public:
            stringtable_t() : mNumStrings(0), mStringHashes(nullptr), mStringPtrs(nullptr) {}

            s32 getNumStrings() const { return mNumStrings; }

            u32 computeHash(const char* name) const;
            s32 getIndexOf(const char* name) const;

            const char* getStrByIndex(s32 index) const;
            const char* getStrByHash(u32 hash) const;

        private:
            s32          mNumStrings;
            const u32*   mStringHashes;
            const char** mStringPtrs;
        };
    } // namespace ngd
} // namespace ncore

#endif ///< __CGAMEDATA_HASHED_STRINGTABLE_H__