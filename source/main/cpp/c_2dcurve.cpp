#include "charon/c_2dcurve.h"
#include "cbase/c_float.h"
#include "cbase/c_integer.h"
#include "cbase/c_limits.h"

namespace ncore
{
    curve2d_t::curve2d_t()
        : mRatio(0.0f)
        , mMinX(0.0f)
        , mMaxX(0.0f)
        , mCopiedData(false)
        , mCurveInfo(nullptr)
        , mValues(nullptr)
    {
    }

    void curve2d_t::vSetValue(s32 _iIndex, f32 _fValue)
    {
        if (mCurveInfo == nullptr)
            return;

        ASSERT((_iIndex >= 0) && (_iIndex < mCurveInfo->iSize));
        mValues[_iIndex] = _fValue;
    }

    f32 curve2d_t::fReadValue(f32 _fX) const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ASSERT(mValues != nullptr);

        f32 fAbsciss = (_fX - mMinX) * mRatio;
        f32 fIndex   = math::floor(fAbsciss);
        s32 iIndex   = (s32)fIndex;

        ASSERT(iIndex >= 0);
        ASSERT(iIndex < mCurveInfo->iSize);

        if (iIndex < 0)
            return mValues[0];
        if (iIndex >= mCurveInfo->iSize - 1)
            return mValues[mCurveInfo->iSize - 1];
        else
        {
            f32 fDistToLastIndex = fAbsciss - fIndex;
            return (mValues[iIndex] * (1.0f - fDistToLastIndex) + mValues[iIndex + 1] * fDistToLastIndex);
        }
    }

    f32 curve2d_t::fReadAnyValue(f32 _fX) const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ASSERT(mValues != nullptr);

        f32 fAbsciss = (_fX - mMinX) * mRatio;
        f32 fIndex   = math::floor(fAbsciss);
        s32 iIndex   = (s32)fIndex;

        if (iIndex < 0)
            return mValues[0];
        if (iIndex >= mCurveInfo->iSize - 1)
            return mValues[mCurveInfo->iSize - 1];
        else
        {
            f32 fDistToLastIndex = fAbsciss - fIndex;
            return (mValues[iIndex] * (1.0f - fDistToLastIndex) + mValues[iIndex + 1] * fDistToLastIndex);
        }
    }

    f32 curve2d_t::fGetMaxY() const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ASSERT(mValues != nullptr);
        f32 fMaxY = type_t<f32>::min();

        for (s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++)
            fMaxY = math::g_max(fMaxY, mValues[iVal]);

        return fMaxY;
    }

    // retourne l'abscisse correspondante ?maxy
    f32 curve2d_t::fGetAbscissaMaxY() const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ASSERT(mValues != nullptr);
        f32 fMaxY     = type_t<f32>::min();
        s32 iAbscissa = 0;

        for (s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++)
        {
            if (mValues[iVal] > fMaxY)
            {
                fMaxY     = mValues[iVal];
                iAbscissa = iVal;
            }
        }

        return fGetAbscissa(iAbscissa);
    }

    f32 curve2d_t::fGetMinY() const
    {
        if (mCurveInfo == nullptr)
            return 0.0f;

        ASSERT(mValues != nullptr);
        f32 fMinY = type_t<f32>::max();

        for (s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++)
            fMinY = math::g_min(fMinY, mValues[iVal]);

        return fMinY;
    }

    void curve2d_t::vYDegToRad()
    {
        if (mCurveInfo == nullptr)
            return;

        for (s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++)
            mValues[iVal] = math::deg_to_rad(mValues[iVal]);
    }

    void curve2d_t::vXDegToRad()
    {
        mMinX  = math::deg_to_rad(mMinX);
        mMaxX  = math::deg_to_rad(mMaxX);
        mRatio = math::rad_to_deg(mRatio);
    }

    // Multiplie les abscisses par un coefficient > 0 : homothetie de direction OX, d'axe OY et de rapport _fXCoeff
    void curve2d_t::vMultiplyX(f32 _fXCoeff)
    {
        ASSERT(_fXCoeff > 0);
        mRatio /= _fXCoeff;
        // Abscisses de debut & fin de la courbe
        mMinX *= _fXCoeff;
        mMaxX *= _fXCoeff;
    }
    // Multiplie les ordonnees par un coefficient > 0 : homothetie de direction OY, d'axe OX et de rapport _fXCoeff
    void curve2d_t::vMultiplyY(f32 _fXCoeff)
    {
        if (mCurveInfo == nullptr)
            return;

        ASSERT(_fXCoeff > 0);
        s32 i;
        for (i = 0; i < mCurveInfo->iSize; i++)
            mValues[i] *= _fXCoeff;
    }

#ifdef ENABLE_CURVE_FILE_NAME

    xbool curve2d_t::load(const char* file, xbool copyData)
    {
        mFileId = FileId::INVALID_ID;
        x_strcpy(mFileName, 256, file);
        return CurveManager::sGetInstance()->load(*this, file, copyData);
    }

    void curve2d_t::setCurve(const char* file, CurveInfo* curveInfo, xbool copiedData)
    {
        ASSERT(curveInfo);

        mFileId = FileId::INVALID_ID;

        x_strcpy(mFileName, 256, file);

        if (copiedData)
        {
            if (mCopiedData && mCurveInfo)
            {
                x_delete((char*)mCurveInfo);
            }
            CurveInfo* newCurveInfo = (CurveInfo*)(x_new(char, curveInfo->iSize * sizeof(f32) + sizeof(CurveInfo), XMEM_FLAGS_DEFAULT));

            x_memcpy(newCurveInfo, curveInfo, curveInfo->iSize * sizeof(f32) + sizeof(CurveInfo));

            mCurveInfo = newCurveInfo;
        }
        else
        {
            mCurveInfo = curveInfo;
        }

        mCopiedData = copiedData;

        mMaxX   = mCurveInfo->fMaxX;
        mMinX   = mCurveInfo->fMinX;
        mRatio  = ((f32)(mCurveInfo->iSize - 1)) / (mMaxX - mMinX);
        mValues = (f32*)((s32)mCurveInfo + sizeof(CurveInfo));
    }

#endif

}  // namespace ncore
