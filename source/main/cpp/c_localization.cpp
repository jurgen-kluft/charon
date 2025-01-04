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

    void localization_t::init(charon::languages_t* languages, charon::archive_t* ar)
    {
        mLanguages = languages;

        for (s32 i = 0; i < charon::enums::LanguageCount; ++i)
        {
            mLanguageStrTables[i] = nullptr;
        }
    }

    void localization_t::loadDefaultLanguage(charon::archive_t* ar)
    {
        mCurrentLanguage = mLanguages->getDefaultLanguage();
        loadLanguage(mLanguageStrTables[mCurrentLanguage], mCurrentLanguage, ar);
    }

    charon::enums::ELanguage localization_t::getCurrentLanguage() const { return mCurrentLanguage; }

    void localization_t::loadLanguage(charon::strtable_t*& language, charon::enums::ELanguage language_id, charon::archive_t* bf)
    {
        charon::datafile_t<charon::strtable_t> lan = mLanguages->getLanguageArray()[language_id];
        charon::archive_loader_t* loader = bf->loader();
        void* language_mem = loader->load_datafile(lan.m_fileid);
        language = new (language_mem) charon::strtable_t();
    }

    void localization_t::exit()
    {
        // For every language that was loaded, unload it
        for (s32 i = 0; i < charon::enums::LanguageCount; ++i)
        {
            if (mLanguageStrTables[i] != nullptr)
            {
                mAllocator->deallocate(mLanguageStrTables[i]);
                mLanguageStrTables[i] = nullptr;
            }
        }
    }

    const char* localization_t::getText(charon::locstr_t lstr) const
    {
        if (lstr.getId() == charon::INVALID_LOCSTR.getId())
            return "INVALID LOC ID";

        charon::strtable_t const* language = mLanguageStrTables[mCurrentLanguage];
        charon::string_t          str  = language->str(lstr.getId());
        return str.c_str();
    }

}  // namespace ncore
