#ifndef __CGAMEDATA_LOCALIZATION_MANAGER_H__
#define __CGAMEDATA_LOCALIZATION_MANAGER_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cbase/c_debug.h"
#include "cgamedata/c_object.h"

enum ELanguage
{
    LANGUAGE_INVALID = -1,
    // -----support language-----
    LANGUAGE_ENGLISH = 0,
    LANGUAGE_CHINESE = 1,
    //-----------------------------

    // -----not support language-----
    //	LANGUAGE_ITALIAN		= 2,
    //	LANGUAGE_GERMAN			= 3,
    //	LANGUAGE_DUTCH			= 4,
    // 	LANGUAGE_ENGLISH_US		= 5,
    // 	LANGUAGE_SPANISH		= 6,
    // 	LANGUAGE_FRENCH_US		= 7,
    // 	LANGUAGE_PORTUGUESE		= 8,
    // 	LANGUAGE_BRAZILIAN		= 9,												///< Brazilian Portuguese
    // 	LANGUAGE_JAPANESE		= 10,
    // 	LANGUAGE_KOREAN         = 11,												///< Korean
    // 	LANGUAGE_RUSSIAN        = 12,												///< Russian
    // 	LANGUAGE_GREEK			= 13,
    // 	LANGUAGE_CHINESE_T		= 14,
    // 	LANGUAGE_CHINESE_S		= 15,
    // 	LANGUAGE_FINNISH		= 16,
    // 	LANGUAGE_SWEDISH		= 17,
    // 	LANGUAGE_DANISH			= 18,
    // 	LANGUAGE_NORWEGIAN		= 19,
    // 	LANGUAGE_POLISH			= 20,
    //---------------------------------

    LANGUAGE_COUNT,

    LANGUAGE_DEFAULT = LANGUAGE_ENGLISH,
    LANGUAGE_MAIN    = LANGUAGE_DEFAULT
};

struct lstring_t
{
	ncore::u8 hash[20];
};

class LocalizationManager
{
public:
    static void sInit();
    static void sExit();

    static const char*      sGetText(ncore::s32 id);
    static const ncore::s32 sHashToId(lstring_t localizationHash);
    static const ncore::u64 sGetStringHash64(const char* str);
    static const char*      sGetTextByLocId(const char* locId);

    static const char* sGetTextByLString(lstring_t lstr);

public:
    LocalizationManager();
    LocalizationManager(const LocalizationManager& other);

    static ELanguage sGetCurrentLanguage();
    static void      sLoad();

    static ncore::s32  sLanguageFileSize;
    static ncore::s32* sOffsets;
    static char*       sLanguageData;
    static ncore::s32  sNumStrings;
    static const char* sStrings;
    static ELanguage   sCurrentLanguage;
    static ncore::u64* sHashOffsets;
};

#endif // __CGAMEDATA_LOCALIZATION_MANAGER_H__