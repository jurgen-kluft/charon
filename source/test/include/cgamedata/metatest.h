#ifndef __CGAMEDATA_TEST_H__
#define __CGAMEDATA_TEST_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "cgamedata/c_object.h"

namespace ncore
{
    namespace ngd
    {
        enum ETestEnum
        {
            EnumerationA = 4294901760,
            EnumerationB = 4294901761,
            EnumerationC = 4294901762,
            EnumerationD = 4294901763,
        };

        class TestData
        {
        public:
            string_t     getName() const { return m_Name.str(); }
            fileid_t     getFile() const { return m_File; }
            array_t<f32> getFloats() const { return m_Floats.array(); }
            array_t<s64> getIntegerList() const { return m_IntegerList.array(); }

        private:
            rawstr_t const      m_Name;
            fileid_t const      m_File;
            rawarr_t<f32> const m_Floats;
            rawarr_t<s64> const m_IntegerList;
        };

        class TestRoot
        {
        public:
            const TestData* getData() const { return m_Data.ptr(); }
            bool            getBool() const { return m_Bool; }
            s32             getInt32() const { return m_Int32; }
            f32             getFloat() const { return m_Float; }
            ETestEnum       getEnum() const
            {
                ETestEnum e;
                m_Enum.get(e);
                return e;
            }
            color_t getColor() const { return m_Color; }
            s8      getInt8() const { return m_Int8; }

        private:
            rawobj_t<TestData> const        m_Data;
            bool const                      m_Bool;
            s32 const                       m_Int32;
            f32 const                       m_Float;
            rawenum_t<ETestEnum, u32> const m_Enum;
            color_t const                   m_Color;
            s8 const                        m_Int8;
        };
    } // namespace ngd
} // namespace ncore

#endif