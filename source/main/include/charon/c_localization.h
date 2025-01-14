#ifndef __CHARON_LANGUAGE_H__
#define __CHARON_LANGUAGE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "charon/c_archive.h"
#include "charon/c_gamedata.h"

namespace ncore
{
    class alloc_t;

    class localization_t
    {
    public:
        localization_t(alloc_t* allocator);

        void setup(charon::languages_t* languages, charon::archive_t* ar);
        void teardown(charon::archive_t* ar);

        charon::enums::ELanguage getCurrentLanguage() const;
        void                     setupLanguage(charon::enums::ELanguage language, charon::archive_t* ar);
        void                     teardownLanguage(charon::enums::ELanguage language, charon::archive_t* ar);

        // UTF-8
        charon::string_t getText(charon::locstr_t lstr) const;

    private:
        void loadLanguage(charon::strtable_t*& language, charon::enums::ELanguage language_id, charon::archive_t* ar);
        void unloadLanguages(charon::archive_t* ar);

        alloc_t*                   mAllocator;
        charon::languages_t const* mLanguages;
        charon::strtable_t*        mLanguageStrTables[charon::enums::LanguageCount];
        charon::enums::ELanguage   mCurrentLanguage;
    };

}  // namespace ncore

#endif  // __CHARON_LANGUAGE_H__
