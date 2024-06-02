#ifndef __CHARON_OBJECT_H__
#define __CHARON_OBJECT_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"

namespace ncore
{
    namespace charon
    {
        // Forward declares
        class member_t;
        class object_t;
        class strtable_t;

        template <class T>
        class array_t
        {
        public:
            inline array_t()
                : m_length(0)
                , m_array(nullptr)
            {
            }
            inline array_t(const s32 size, T const* data)
                : m_length(size)
                , m_array(data)
            {
            }

            inline s32      size() const { return m_length; }
            inline const T& operator[](s32 index) const
            {
                ASSERT(index < m_length);
                return m_array[index];
            }

        private:
            T const*  m_array;
            s32 const m_length;
        };

        class string_t
        {
        public:
            inline string_t()
                : m_bytes(0)
                , m_len(0)
                , m_str("null")
            {
            }
            inline string_t(s32 byteLen, s32 strLen, const char* str)
                : m_bytes(byteLen)
                , m_len(strLen)
                , m_str(str)
            {
            }
            inline s32         len() const { return (s32)m_len; }
            inline const char* str() const { return m_str; }

        private:
            const u32   m_bytes;
            const u32   m_len;
            const char* m_str;
        };

        class strtable_t
        {
        public:
            inline strtable_t(u32 numStrings, u32 const* byteLengths, u32 const* charLengths, u32 const* offsets, const char* strings)
                : mMagic(0x36DF5DE5)
                , mNumStrings(numStrings)
                , mByteLengths(byteLengths)
                , mCharLengths(charLengths)
                , mOffsets(offsets)
                , mStrings(strings)
            {
            }
            inline s32       size() const { return mNumStrings; }
            inline string_t  str(u32 index) const { return string_t(mByteLengths[index], mCharLengths[index], mStrings + mOffsets[index]); }

        protected:
            u32         mMagic;  // 'STRT'
            u32         mNumStrings;
            u32 const*  mHashes;
            u32 const*  mOffsets;
            u32 const*  mCharLengths;
            u32 const*  mByteLengths;
            const char* mStrings;
        };

        // e.g. rawenum_t<EConfig, u16>
        template <class T, class E>
        class enum_t
        {
        public:
            void get(T& e) const { e = (T)mEnum; }
            T    get() const { return (T)mEnum; }

        protected:
            inline enum_t(E e)
                : mEnum(e)
            {
            }
            E const mEnum;
        };

        class membername_t
        {
        public:
            explicit membername_t(const char* name)
                : m_name(name)
            {
            }

            const char* getName() const { return m_name; }

        private:
            const char* m_name;
        };

        typedef u32 color_t;

        const color_t sBlackColor = 0x00000000;
        const color_t sWhiteColor = 0xFFFFFFFF;

        typedef s64 fileid_t;
        typedef s32 locstr_t;

        const fileid_t INVALID_FILEID = -1;
        const locstr_t INVALID_LOCSTR = -1;

        class vec3f_t
        {
        public:
            vec3f_t() {}

            inline float x() const { return mX; }
            inline float y() const { return mY; }
            inline float z() const { return mZ; }

            static const vec3f_t sZero;

        private:
            float mX, mY, mZ;
        };

        typedef s16 fx16_t;
        typedef s32 fx32_t;

        class vec3fx_t
        {
        public:
            inline fx32_t x() const { return mX; }
            inline fx32_t y() const { return mY; }
            inline fx32_t z() const { return mZ; }

            static const vec3fx_t sZero;

        private:
            fx32_t mX, mY, mZ;
        };

        class vec3i_t
        {
        public:
            inline s32 x() const { return mX; }
            inline s32 y() const { return mY; }
            inline s32 z() const { return mZ; }

            static const vec3i_t sZero;

        private:
            s32 mX, mY, mZ;
        };

        // Examples:
        //		const object_t* track   = root->get_object(membername_t("main"))->get_object(membername_t("tracks"))->get_object(membername_t("track1"));
        //		const object_t* texture = track->get_object(membername_t("textures"))->get_object(membername_t("texture1"));
        //      const fileid_t fileId   = texture->get_fileid(membername_t("fileid"));
        class object_t
        {
        public:
            ///@name For checking if a member is present
            bool hasMember(membername_t _tname) const;
            s32  getNumMembers() const { return m_member_count; }

            void print(const strtable_t& strTable, const strtable_t& typeTable) const;

            bool is_s8(membername_t _tname) const;
            bool is_u8(membername_t _tname) const;
            bool is_s16(membername_t _tname) const;
            bool is_u16(membername_t _tname) const;
            bool is_s32(membername_t _tname) const;
            bool is_u32(membername_t _tname) const;
            bool is_fx16(membername_t _tname) const;
            bool is_fx32(membername_t _tname) const;
            bool is_f32(membername_t _tname) const;
            bool is_locstr(membername_t _tname) const;
            bool is_fileid(membername_t _tname) const;
            bool is_color(membername_t _tname) const;
            bool is_object(membername_t _tname) const;

            bool            get_bool(membername_t _tname) const;
            s8              get_s8(membername_t _tname) const;
            u8              get_u8(membername_t _tname) const;
            s16             get_s16(membername_t _tname) const;
            u16             get_u16(membername_t _tname) const;
            s32             get_s32(membername_t _tname) const;
            u32             get_u32(membername_t _tname) const;
            fx16_t          get_fx16(membername_t _tname) const;
            fx32_t          get_fx32(membername_t _tname) const;
            f32             get_f32(membername_t _tname) const;
            const char*     get_string(membername_t _tname) const;
            locstr_t        get_locstr(membername_t _tname) const;
            fileid_t        get_fileid(membername_t _tname) const;
            const object_t* get_object(membername_t _tname) const;
            color_t         get_color(membername_t _tname) const;
            const vec3fx_t& get_vec3fx(membername_t _tname) const;

            bool is_s8_array(membername_t _tname) const;
            bool is_u8_array(membername_t _tname) const;
            bool is_s16_array(membername_t _tname) const;
            bool is_u16_array(membername_t _tname) const;
            bool is_s32_array(membername_t _tname) const;
            bool is_u32_array(membername_t _tname) const;
            bool is_fx16_array(membername_t _tname) const;
            bool is_fx32_array(membername_t _tname) const;
            bool is_f32_array(membername_t _tname) const;
            bool is_locstr_array(membername_t _tname) const;
            bool is_fileid_array(membername_t _tname) const;
            bool is_color_array(membername_t _tname) const;
            bool is_object_array(membername_t _tname) const;

            array_t<bool>            get_bool_array(membername_t _tname) const;
            array_t<s8>              get_s8_array(membername_t _tname) const;
            array_t<u8>              get_u8_array(membername_t _tname) const;
            array_t<s16>             get_s16_array(membername_t _tname) const;
            array_t<u16>             get_u16_array(membername_t _tname) const;
            array_t<s32>             get_s32_array(membername_t _tname) const;
            array_t<u32>             get_u32_array(membername_t _tname) const;
            array_t<fx16_t>          get_fx16_array(membername_t _tname) const;
            array_t<fx32_t>          get_fx32_array(membername_t _tname) const;
            array_t<f32>             get_f32_array(membername_t _tname) const;
            array_t<const char*>     get_string_array(membername_t _tname) const;
            array_t<locstr_t>        get_locstr_array(membername_t _tname) const;
            array_t<fileid_t>        get_fileid_array(membername_t _tname) const;
            array_t<const object_t*> get_object_array(membername_t _tname) const;
            array_t<color_t>         get_color_array(membername_t _tname) const;
            array_t<vec3fx_t>        get_vec3fx_array(membername_t _tname) const;

            template <class T>
            const T* get_compound(membername_t _tname) const;
            template <class T>
            array_t<const T*> get_compoundarray(membername_t _tname) const;

        private:
            s32 m_member_count;

            const member_t* get_member(membername_t _tname) const;
            const void*     get_compound(membername_t _tname) const;
        };

        template <class T>
        const T* object_t::get_compound(membername_t _tname) const
        {
            return (const T*)get_compound(_tname);
        }

        template <class T>
        array_t<const T*> object_t::get_compoundarray(membername_t _tname) const
        {
            array_t<const object_t*> objArray = get_object_array(_tname);
            return array_t<const T*>(objArray.size(), (const T*)&objArray[0]);
        }
    }  // namespace ngd
}  // namespace ncore

#endif  /// __CHARON_OBJECT_H__
