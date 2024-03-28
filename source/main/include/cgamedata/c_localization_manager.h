#ifndef __CGAMEDATA_LANGUAGE_H__
#define __CGAMEDATA_LANGUAGE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cgamedata/c_object.h"
#include "cgamedata/c_bigfile_manager.h"

namespace ncore
{
    class alloc_t;

    class language_t
    {
    public:
        static language_t* instance;

        enum
        {
            LANGUAGE_INVALID    = -1,
            LANGUAGE_ENGLISH    = 0,
            LANGUAGE_CHINESE    = 1,
            LANGUAGE_ITALIAN    = 2,
            LANGUAGE_GERMAN     = 3,
            LANGUAGE_DUTCH      = 4,
            LANGUAGE_ENGLISH_US = 5,
            LANGUAGE_SPANISH    = 6,
            LANGUAGE_FRENCH_US  = 7,
            LANGUAGE_PORTUGUESE = 8,
            LANGUAGE_BRAZILIAN  = 9,  // Brazilian Portuguese
            LANGUAGE_JAPANESE   = 10, //
            LANGUAGE_KOREAN     = 11, // Korean
            LANGUAGE_RUSSIAN    = 12, // Russian
            LANGUAGE_GREEK      = 13,
            LANGUAGE_CHINESE_T  = 14,
            LANGUAGE_CHINESE_S  = 15,
            LANGUAGE_FINNISH    = 16,
            LANGUAGE_SWEDISH    = 17,
            LANGUAGE_DANISH     = 18,
            LANGUAGE_NORWEGIAN  = 19,
            LANGUAGE_POLISH     = 20,

            LANGUAGE_COUNT,

            LANGUAGE_DEFAULT = LANGUAGE_ENGLISH,
            LANGUAGE_MAIN    = LANGUAGE_DEFAULT
        };

        language_t(alloc_t* allocator);

        void init(ngd::fileid_t const* languageFileIds);
        void exit();

        s8   getCurrentLanguage() const;
        void loadCurrentLanguage(ngd::bigfile_t* bf, s8 language);

        // UTF-8
        const char* getText(ngd::locstr_t lstr) const;

    private:
        alloc_t*      mAllocator;
        s8            mCurrentLanguage;
        ngd::fileid_t mLanguageFiles[LANGUAGE_COUNT];

        struct data_t // Exact file-format of a language file
        {
            u64 const mMagic; // '_LANGUAGE_'
            u32 const mNumStrings;
            u32 const mOffsetToOffsets;
            u32 const mOffsetToStrings;
            // u32[]    offsets
            // char[][] strings
        };
        data_t*     mData;
        u32 const*  mOffsets;
        char const* mStrings;
    };

} // namespace ncore

#endif // __CGAMEDATA_LANGUAGE_H__