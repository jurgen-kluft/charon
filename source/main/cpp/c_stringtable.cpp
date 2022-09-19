#include "cgamedata/c_stringtable.h"
#include "cbase/c_hash.h"
#include "cbase/c_binary_search.h"

namespace ncore
{
    namespace ngd
    {
        // The string table, containing a hash array, offset array and the actual string data
        static s8 scompare_predicate_u32(const void* inItem, const void* inData, s64 inIndex)
        {
            u32 const* item = (u32 const*)inItem;
            u32 const* data = (u32 const*)inData;
            if (*item < data[inIndex])
                return -1;
            else if (*item > data[inIndex])
                return 1;
            return 0;
        }

        u32 stringtable_t::computeHash(const char* name) const { return strhash(name); }

        s32 stringtable_t::getIndexOf(const char* name) const
        {
            u32 hash  = computeHash(name);
            s64 index = g_BinarySearch(&hash, mStringHashes, mNumStrings, scompare_predicate_u32);
            if (index >= 0)
                return (s32)index;
            return -1;
        }

        const char* stringtable_t::getStrByIndex(s32 index) const { return mStringPtrs[index]; }

        const char* stringtable_t::getStrByHash(u32 hash) const
        {
            s64 index = g_BinarySearch(&hash, mStringHashes, mNumStrings, scompare_predicate_u32);
            if (index >= 0)
            {
                return mStringPtrs[index];
            }
            return nullptr;
        }
    } // namespace ngd
} // namespace ncore
