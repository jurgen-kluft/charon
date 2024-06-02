#include "ccore/c_target.h"
#include "cbase/c_allocator.h"
#include "cbase/c_integer.h"
#include "cbase/c_printf.h"

#include "charon/c_localization.h"
#include "charon/c_object.h"

namespace ncore
{
#define XPRINT(msg)

    localization_t::localization_t(alloc_t* allocator)
        : mAllocator(allocator)
        , mCurrentLanguage(LANGUAGE_INVALID)
        , mData(nullptr)
        , mStrings(nullptr)
        , mOffsets(nullptr)
    {
        for (s32 i = 0; i < LANGUAGE_COUNT; ++i)
        {
            mLanguageFiles[LANGUAGE_COUNT] = charon::INVALID_FILEID;
        }
    }

    void localization_t::init(charon::object_t* root, charon::fileid_t const* languageFileIds, charon::bigfile_t* bf)
    {
        mData            = nullptr;
        mStrings         = nullptr;
        mOffsets         = nullptr;
        mCurrentLanguage = LANGUAGE_ENGLISH;

        XPRINT("Start localization_t init...\n");
        if (mData == nullptr)
        {
            XPRINT("Start get file id...\n");
            charon::array_t<charon::fileid_t> locFiles = root->get_object(charon::membername_t("Localization"))->get_fileid_array(charon::membername_t("FileId"));
            XPRINT("The file id list is init success.\n");
            mLanguageFileSize = 0;

            const s32 n = locFiles.size();
            printf("file id list : %d\n", va_t(n));
            for (s32 i = 0; i < n; i++)
            {
                charon::fileid_t fileId   = locFiles[i];
                const s32     fileSize = (fileId != charon::INVALID_FILEID) ? bf->size(fileId) : 0;
                if (fileSize > mLanguageFileSize)
                    mLanguageFileSize = fileSize;
            }
            XPRINT("Language file size init success.\n");
            mLanguageFileSize = math::alignUp(mLanguageFileSize, 4);
            XPRINT("Language file size alloc success.\n");

            mOffsets         = nullptr;
            mStrings         = nullptr;
            mCurrentLanguage = LANGUAGE_ENGLISH;

            mData = (data_t*)mAllocator->allocate(mLanguageFileSize, sizeof(void*));  // new char[sLanguageFileSize];
            XPRINT("Language data alloc success.\n");
        }

        XPRINT("End localization_t init...\n");
    }

    s8   localization_t::getCurrentLanguage() const { return mCurrentLanguage; }
    void localization_t::loadCurrentLanguage(charon::object_t* root, charon::bigfile_t* bf, s8 language)
    {
        charon::array_t<charon::fileid_t> locFiles = root->get_object(charon::membername_t("Localization"))->get_fileid_array(charon::membername_t("FileId"));
        charon::fileid_t               fileId   = locFiles[(s32)language];
        const s32                   fileSize = (fileId != charon::INVALID_FILEID) ? bf->size(fileId) : 0;
        charon::string_t               fileName = bf->filename(fileId);

        mStrings = nullptr;
        mOffsets = nullptr;

        bool is_ok = fileSize > 0;
        if (is_ok)
        {
            is_ok = bf->read(fileId, (void*)mData);
            if (is_ok)
            {
                mStrings = mData->Strings();
                mOffsets = mData->Offsets();
            }
        }

        if (!is_ok)
        {
            // Tracer::Trace("Could not load localization data for language");
        }
    }

    void localization_t::exit()
    {
        mLanguageFileSize = 0;
        mAllocator->deallocate(mData);
        mData    = nullptr;
        mStrings = nullptr;
        mOffsets = nullptr;
    }

    const char* localization_t::getText(charon::locstr_t lstr) const
    {
        if (lstr < 0 || lstr >= mData->mNumStrings)
        {
            return "WRONG LOC ID";
        }
        return mStrings + mOffsets[lstr];
    }

}  // namespace ncore
