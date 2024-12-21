#ifndef __CHARON_OBJECT_H__
#define __CHARON_OBJECT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_allocator.h"
#include "ccore/c_debug.h"

namespace ncore
{
    namespace charon
    {
        // Forward declares
        struct staticmesh_t;
        struct audio_t;
        struct texture_t;
        struct font_t;
        struct curve_t;

        namespace enums
        {
            enum ELanguage
            {
                LanguageDefault    = 0,
                LanguageChinese    = 1,
                LanguageItalian    = 2,
                LanguageGerman     = 3,
                LanguageDutch      = 4,
                LanguageEnglishUs  = 5,
                LanguageSpanish    = 6,
                LanguageFrenchUs   = 7,
                LanguagePortuguese = 8,
                LanguageBrazilian  = 9,
                LanguageJapanese   = 10,
                LanguageKorean     = 11,
                LanguageRussian    = 12,
                LanguageGreek      = 13,
                LanguageChineseT   = 14,
                LanguageChineseS   = 15,
                LanguageFinnish    = 16,
                LanguageSwedish    = 17,
                LanguageDanish     = 18,
                LanguageNorwegian  = 19,
                LanguagePolish     = 20,
                LanguageCount      = 21,
                LanguageInvalid    = -1,
            };
        }

        namespace enums
        {
            enum EnemyType
            {
                Soldier = 0,
                Archer  = 1,
                Knight  = 2,
            };
        }

        // Forward declares
        struct gameroot_t;
        struct ai_t;
        struct fonts_t;
        struct menu_t;
        struct localization_t;
        struct cars_t;
        struct tracks_t;
        struct languages_t;
        struct track_t;
        struct enemy_t;
        struct car_t;
        struct carconfiguration_t;
        struct modeldata_t;

        template <class T>
        struct array_t
        {
            inline array_t()
                : m_array(nullptr)
                , m_bytes(0)
                , m_count(0)
            {
            }

            inline array_t(const u32 count, T const* data)
                : m_array(data)
                , m_bytes(count * sizeof(T))
                , m_count(count)
            {
            }

            inline array_t(const u32 count, const u32 bytes, T const* data)
                : m_array(data)
                , m_bytes(bytes)
                , m_count(count)
            {
            }

            inline u32 size() const { return m_count; }
            inline u32 bytes() const { return m_bytes; }

            inline T& operator[](s32 index)
            {
                ASSERT(index < m_count);
                return m_array[index];
            }

            inline const T& operator[](s32 index) const
            {
                ASSERT(index < m_count);
                return m_array[index];
            }

        private:
            T*  m_array;
            u32 m_bytes;
            u32 m_count;
        };

        struct fileid_t
        {
            explicit fileid_t(u32 bigfileIndex, u32 fileIndex)
                : bigfileIndex(bigfileIndex)
                , fileIndex(fileIndex)
            {
            }

            inline u32 getBigfileIndex() const { return bigfileIndex; }
            inline u32 getFileIndex() const { return fileIndex; }

        private:
            u32 bigfileIndex;
            u32 fileIndex;
        };

        const fileid_t INVALID_FILEID((u32)-1, (u32)-1);

        struct locstr_t
        {
            explicit locstr_t(u64 id)
                : id(id)
            {
            }
            inline u64 getId() const { return id; }

        private:
            u64 id;
        };

        const locstr_t INVALID_LOCSTR((u64)-1);

        // A standard string (ASCII, UTF-8)
        struct string_t
        {
            inline string_t()
                : m_bytes(0)
                , m_count(0)
                , m_array("")
            {
            }

            inline string_t(u32 byteLength, u32 charLength, const char* data)
                : m_bytes(byteLength)
                , m_count(charLength)
                , m_array(data)
            {
            }

            inline u32         size() const { return m_count; }
            inline u32         bytes() const { return m_bytes; }
            inline const char* c_str() const { return m_array; }

        private:
            const u32   m_bytes;
            const u32   m_count;
            char const* m_array;
        };

        struct strtable_t
        {
            inline strtable_t(u32 numStrings, u32 const* byteLengths, u32 const* charLengths, u32 const* offsets, const char* strings)
                : mMagic(0x36DF5DE5)
                , mNumStrings(numStrings)
                , mByteLengths(byteLengths)
                , mCharLengths(charLengths)
                , mOffsets(offsets)
                , mStrings(strings)
            {
            }
            inline s32      size() const { return mNumStrings; }
            inline string_t str(u32 index) const { return string_t(mByteLengths[index], mCharLengths[index], mStrings + mOffsets[index]); }

            DCORE_CLASS_PLACEMENT_NEW_DELETE

        protected:
            u32         mMagic;  // 'STRT'
            u32         mNumStrings;
            u32 const*  mHashes;
            u32 const*  mOffsets;
            u32 const*  mCharLengths;
            u32 const*  mByteLengths;
            const char* mStrings;
        };

        template <typename T>
        struct datafile_t
        {
            T*             m_ptr;
            const fileid_t m_fileid;
        };

        struct modeldata_t
        {
            inline datafile_t<staticmesh_t> const&  getStaticMesh() const { return m_StaticMesh; }
            inline array_t<datafile_t<void>> const& getTextures() const { return m_Textures; }

        private:
            datafile_t<staticmesh_t>  m_StaticMesh;
            array_t<datafile_t<void>> m_Textures;
        };

        struct carconfiguration_t
        {
            inline string_t const& getName() const { return m_Name; }
            inline f32             getWeight() const { return m_Weight; }
            inline f32             getMaxSpeed() const { return m_MaxSpeed; }
            inline f32             getAcceleration() const { return m_Acceleration; }
            inline f32             getBraking() const { return m_Braking; }
            inline f32             getCornering() const { return m_Cornering; }
            inline f32             getStability() const { return m_Stability; }
            inline f32             getTraction() const { return m_Traction; }

        private:
            string_t m_Name;
            f32      m_Weight;
            f32      m_MaxSpeed;
            f32      m_Acceleration;
            f32      m_Braking;
            f32      m_Cornering;
            f32      m_Stability;
            f32      m_Traction;
        };

        struct track_t
        {
            inline datafile_t<texture_t> const& getRoad() const { return m_Road; }
            inline modeldata_t const*           getModel() const { return m_Model; }

        private:
            datafile_t<texture_t> m_Road;
            modeldata_t const*    m_Model;
        };

        struct car_t
        {
            inline carconfiguration_t const* getConfiguration() const { return m_Configuration; }
            inline modeldata_t const*        getModelPath() const { return m_ModelPath; }

        private:
            carconfiguration_t const* m_Configuration;
            modeldata_t const*        m_ModelPath;
        };

        struct enemy_t
        {
            inline f32              getSpeed() const { return m_Speed; }
            inline f32              getAggresiveness() const { return m_Aggresiveness; }
            inline s16              getHealth() const { return m_Health; }
            inline bool             getIsAggressive() const { return (m_Booleans0 & (1 << 0)) != 0; }
            inline bool             getWillFollowPlayer() const { return (m_Booleans0 & (1 << 1)) != 0; }
            inline bool             getWillCallReinforcements() const { return (m_Booleans0 & (1 << 2)) != 0; }
            inline enums::EnemyType getEnemyType() const { return (enums::EnemyType)m_EnemyType; }

        private:
            f32 m_Speed;
            f32 m_Aggresiveness;
            s16 m_Health;
            u8  m_Booleans0;
            u8  m_EnemyType;
        };

        struct languages_t
        {
            inline array_t<datafile_t<strtable_t>> const& getLanguageArray() const { return m_LanguageArray; }
            inline enums::ELanguage                       getDefaultLanguage() const { return (enums::ELanguage)m_DefaultLanguage; }

        private:
            array_t<datafile_t<strtable_t>> m_LanguageArray;
            s16                             m_DefaultLanguage;
        };

        struct cars_t
        {
            inline array_t<car_t const*> const& getcars() const { return m_cars; }

        private:
            array_t<car_t const*> m_cars;
        };

        struct localization_t
        {
            inline languages_t const* getLanguages() const { return m_Languages; }

        private:
            languages_t const* m_Languages;
        };

        struct menu_t
        {
            inline string_t const& getdescr() const { return m_descr; }

        private:
            string_t m_descr;
        };

        struct fonts_t
        {
            inline string_t const&           getDescription() const { return m_Description; }
            inline datafile_t<font_t> const& getFont() const { return m_Font; }

        private:
            string_t           m_Description;
            datafile_t<font_t> m_Font;
        };

        struct ai_t
        {
            inline string_t const&                getDescription() const { return m_Description; }
            inline datafile_t<curve_t> const&     getReactionCurve() const { return m_ReactionCurve; }
            inline array_t<enemy_t const*> const& getBlueprintsAsArray() const { return m_BlueprintsAsArray; }
            inline array_t<enemy_t const*> const& getBlueprintsAsList() const { return m_BlueprintsAsList; }

        private:
            string_t                m_Description;
            datafile_t<curve_t>     m_ReactionCurve;
            array_t<enemy_t const*> m_BlueprintsAsArray;
            array_t<enemy_t const*> m_BlueprintsAsList;
        };

        struct tracks_t
        {
            inline array_t<datafile_t<track_t>>& gettracks() { return m_tracks; }

        private:
            array_t<datafile_t<track_t>> m_tracks;
        };

        struct gameroot_t
        {
            inline datafile_t<audio_t> const&        getBootSound() const { return m_BootSound; }
            inline datafile_t<ai_t> const&           getAI() const { return m_AI; }
            inline datafile_t<fonts_t> const&        getFonts() const { return m_Fonts; }
            inline datafile_t<menu_t> const&         getMenu() const { return m_Menu; }
            inline datafile_t<localization_t> const& getLocalization() const { return m_Localization; }
            inline datafile_t<cars_t> const&         getCars() const { return m_Cars; }
            inline tracks_t const*                   getTracks() const { return m_Tracks; }

            datafile_t<audio_t>        m_BootSound;
            datafile_t<ai_t>           m_AI;
            datafile_t<fonts_t>        m_Fonts;
            datafile_t<menu_t>         m_Menu;
            datafile_t<localization_t> m_Localization;
            datafile_t<cars_t>         m_Cars;
            tracks_t *            m_Tracks;
        };

    }  // namespace charon
}  // namespace ncore

#endif  /// __CHARON_OBJECT_H__
