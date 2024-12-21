#ifndef __CHARON_2DCURVE_H__
#define __CHARON_2DCURVE_H__
#include "ccore/c_target.h"
#ifdef M_PRAGMA_ONCE
#    pragma once
#endif  // M_PRAGMA_ONCE

#include "ccore/c_debug.h"

namespace ncore
{
    class curve2d_t
    {
    public:
        curve2d_t();

        void vSetValue(ncore::s32 _iIndex, ncore::f32 _fValue);
        void vMultiplyX(ncore::f32 _fXCoeff);
        void vMultiplyY(ncore::f32 _fXCoeff);
        void vYDegToRad();
        void vXDegToRad();

        ncore::f32 fReadValue(ncore::f32 _fX) const;
        ncore::f32 fReadAnyValue(ncore::f32 _fX) const;
        ncore::f32 fGetAbscissa(ncore::s32 _iIndex) const;
        ncore::f32 fGetMaxY() const;

        ncore::f32 fGetAbscissaMaxY() const;
        ncore::f32 fGetMinY() const;
        ncore::f32 fGetMaxX() const
        {
            ASSERT(mCurveInfo);
            return mMaxX;
        }
        ncore::f32 fGetMinX() const
        {
            ASSERT(mCurveInfo);
            return mMinX;
        }

        ncore::f32 fGetLastNegativeXFromEnd() const;

        enum ECurveState
        {
            ECurveStateSTANDALONE,
            ECurveStateSHAREDVALUES,
            ECurveStateSHARED
        };

        struct curve_info_t
        {
            ncore::f32 fMinX, fMaxX;
            ncore::s32 iSize;
        };

    protected:
        curve_info_t* mCurveInfo;
        ncore::f32*   mValues;
        ncore::f32    mRatio;
        ncore::f32    mMinX;
        ncore::f32    mMaxX;
        bool          mCopiedData;
    };

    inline ncore::f32 curve2d_t::fGetAbscissa(ncore::s32 _iIndex) const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ASSERT(mValues != nullptr);
        ASSERT(_iIndex >= 0);
        ASSERT(_iIndex < mCurveInfo->iSize);
        return mMinX + ((ncore::f32)_iIndex) / mRatio;
    }

    inline ncore::f32 curve2d_t::fGetLastNegativeXFromEnd() const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ncore::s32 i = mCurveInfo->iSize - 1;
        while (i > 0)
        {
            if (mValues[i - 1] > 0)
                return fGetAbscissa(i);
            --i;
        }
        return fGetAbscissa(0);
    }

}  // namespace ncore
#endif  // __CHARON_2DCURVE_H__
