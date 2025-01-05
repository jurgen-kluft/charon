#include "ccore/c_target.h"
#include "cbase/c_allocator.h"
#include "cbase/c_integer.h"
#include "cbase/c_printf.h"

#include "charon/c_localization.h"
#include "charon/c_archive.h"

namespace ncore
{
    localization_t::localization_t(alloc_t* allocator)
        : mAllocator(allocator)
        , mCurrentLanguage(charon::enums::LanguageInvalid)
        , mLanguages(nullptr)
    {
    }

    void localization_t::setup(charon::languages_t* languages, charon::archive_t* ar)
    {
        mLanguages = languages;
        for (s32 i = 0; i < charon::enums::LanguageCount; ++i)
        {
            mLanguageStrTables[i] = nullptr;
        }
    }

    void localization_t::teardown(charon::archive_t* ar) { unloadLanguages(ar); }

    charon::enums::ELanguage localization_t::getCurrentLanguage() const { return mCurrentLanguage; }

    void localization_t::setupLanguage(charon::enums::ELanguage language, charon::archive_t* ar)
    {
        s32 const i = language;
        if (mLanguageStrTables[i] == nullptr)
        {
            loadLanguage(mLanguageStrTables[i], mCurrentLanguage, ar);
        }
        mCurrentLanguage = language;
    }

    void localization_t::teardownLanguage(charon::enums::ELanguage language, charon::archive_t* ar)
    {
        s32 const i = language;
        if (mLanguageStrTables[i] != nullptr)
        {
            charon::datafile_t<charon::strtable_t> const& lan = mLanguages->getLanguageArray()[language];
            lan.unload(mLanguageStrTables[i]);
            mCurrentLanguage = charon::enums::LanguageEnglish;
        }
    }

    void localization_t::loadLanguage(charon::strtable_t*& language, charon::enums::ELanguage language_id, charon::archive_t* bf)
    {
        charon::array_t<charon::datafile_t<charon::strtable_t>> const& languages    = mLanguages->getLanguageArray();
        void*                                                          language_mem = languages[language_id].load();
        language                                                                    = new (language_mem) charon::strtable_t();
    }

    void localization_t::unloadLanguages(charon::archive_t* ar)
    {
        // For every language that was loaded, unload it
        charon::array_t<charon::datafile_t<charon::strtable_t>> const& languages = mLanguages->getLanguageArray();
        for (s32 i = 0; i < charon::enums::LanguageCount; ++i)
        {
            if (mLanguageStrTables[i] != nullptr)
            {
                languages[i].unload(mLanguageStrTables[i]);
            }
        }
    }

    charon::string_t localization_t::getText(charon::locstr_t lstr) const
    {
        if (lstr.getId() == charon::INVALID_LOCSTR.getId())
            return charon::string_t();

        charon::strtable_t const* language = mLanguageStrTables[mCurrentLanguage];
        return language->str(lstr.getId());
    }

}  // namespace ncore
