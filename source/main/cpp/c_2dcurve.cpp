#if 0

#include "stdafx.h"
#include "Maths/2dCurve.h"

#include "IO/ArcFileManager.h"
#include "Maths/CurveManager.h"

#ifdef COMPILATION
#include <math.h>
#endif

using namespace ncore;

C2dFCurve::C2dFCurve()
:mRatio(0.0f)
,mMinX(0.0f)
,mMaxX(0.0f)
,mCopiedData(false)
,mCurveInfo(NULL)
,mValues(NULL)
{  
}

C2dFCurve::~C2dFCurve()
{
	vClean();
}

void C2dFCurve::vClean()
{
	if ( mCopiedData )
	{
		x_delete( (char*)mCurveInfo );
		mCurveInfo = NULL;
	}
	mCurveInfo	= NULL;
#ifndef ENABLE_CURVE_FILE_NAME
	CurveManager::sGetInstance()->remove(mFileId);
#else
	if( mFileId == FileId::INVALID_ID )
		CurveManager::sGetInstance()->remove(mFileName);
	else
		CurveManager::sGetInstance()->remove(mFileId);
#endif
}

void C2dFCurve::vSetValue(s32 _iIndex,f32 _fValue)
{
	if( mCurveInfo == NULL )
		return;

	ASSERT((_iIndex>=0) && (_iIndex<mCurveInfo->iSize));
	mValues[_iIndex] = _fValue;
}

f32 C2dFCurve::fReadValue(f32 _fX) const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ASSERT(mValues != NULL);

	f32 fAbsciss = (_fX - mMinX) * mRatio;
	f32 fIndex = x_Floor(fAbsciss);
	s32 iIndex = (s32) fIndex;

	ASSERT(iIndex >= 0);
	ASSERT (iIndex < mCurveInfo->iSize);

	if (iIndex < 0)
		return mValues[0];
	if (iIndex >= mCurveInfo->iSize-1)
		return mValues[mCurveInfo->iSize-1];
	else 
	{
		f32 fDistToLastIndex = fAbsciss - fIndex;
		return (mValues[iIndex] * (1.0f - fDistToLastIndex) + mValues[iIndex + 1] * fDistToLastIndex);
	}
}

f32 C2dFCurve::fReadAnyValue(f32 _fX) const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ASSERT(mValues != NULL);

	f32 fAbsciss = (_fX - mMinX) * mRatio;
	f32 fIndex = x_Floor(fAbsciss);
	s32 iIndex = (s32) fIndex;

	if (iIndex < 0)
		return mValues[0];
	if (iIndex >= mCurveInfo->iSize-1)
		return mValues[mCurveInfo->iSize-1];
	else 
	{
		f32 fDistToLastIndex = fAbsciss - fIndex;
		return (mValues[iIndex] * (1.0f - fDistToLastIndex) + mValues[iIndex + 1] * fDistToLastIndex);
	}
}

f32 C2dFCurve::fGetMaxY() const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ASSERT(mValues != NULL);
	f32 fMaxY = - FLT_MAX;

	for(s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++) 
		fMaxY = x_Max(fMaxY, mValues[iVal]);

	return fMaxY;
}


// retourne l'abscisse correspondante ?maxy
f32 C2dFCurve::fGetAbscissaMaxY() const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ASSERT(mValues != NULL);
	f32 fMaxY = - FLT_MAX;
	s32 iAbscissa = 0;

	for(s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++)
	{  
		if ( mValues[iVal] > fMaxY )
		{
			fMaxY = mValues[iVal];
			iAbscissa = iVal;
		}
	}

	return fGetAbscissa( iAbscissa );
}



f32 C2dFCurve::fGetMinY() const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ASSERT(mValues != NULL);
	f32 fMinY = + FLT_MAX;

	for(s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++) 
		fMinY = x_Min(fMinY, mValues[iVal]);

	return fMinY;
}

void C2dFCurve::vYDegToRad()
{
	if( mCurveInfo == NULL )
		return;

	for(s32 iVal = 0; iVal < mCurveInfo->iSize; iVal++) 
		mValues[iVal] = F_DEG_TO_RAD(mValues[iVal]);
}

void C2dFCurve::vXDegToRad()
{
	mMinX = F_DEG_TO_RAD(mMinX);
	mMaxX = F_DEG_TO_RAD(mMaxX);
	mRatio = F_RAD_TO_DEG(mRatio);
}


// Multiplie les abscisses par un coefficient > 0 : homothetie de direction OX, d'axe OY et de rapport _fXCoeff
void C2dFCurve::vMultiplyX(f32 _fXCoeff)
{
	ASSERT(_fXCoeff > 0);
	mRatio /= _fXCoeff;
	// Abscisses de debut & fin de la courbe
	mMinX *= _fXCoeff;
	mMaxX *= _fXCoeff;
}
// Multiplie les ordonnees par un coefficient > 0 : homothetie de direction OY, d'axe OX et de rapport _fXCoeff
void C2dFCurve::vMultiplyY(f32 _fXCoeff)
{
	if( mCurveInfo == NULL )
		return;

	ASSERT(_fXCoeff > 0);
	s32 i;
	for(i= 0; i < mCurveInfo->iSize; i++) 
		mValues[i] *= _fXCoeff;
}


#ifdef ENABLE_CURVE_FILE_NAME

xbool C2dFCurve::load(const char* file, xbool copyData)
{
	mFileId = FileId::INVALID_ID;
	x_strcpy(mFileName, 256, file);
	return CurveManager::sGetInstance()->load(*this, file, copyData);
}

void C2dFCurve::setCurve(const char* file, CurveInfo* curveInfo, xbool copiedData)
{
	ASSERT(curveInfo);

	mFileId = FileId::INVALID_ID;

	x_strcpy(mFileName, 256, file);

	if ( copiedData )
	{
		if ( mCopiedData && mCurveInfo )
		{
			x_delete( (char*)mCurveInfo );
		}
		CurveInfo*	newCurveInfo	= (CurveInfo*)( x_new( char, curveInfo->iSize * sizeof(f32) + sizeof(CurveInfo), XMEM_FLAGS_DEFAULT ) );

		x_memcpy(newCurveInfo,curveInfo,curveInfo->iSize * sizeof(f32) + sizeof(CurveInfo));

		mCurveInfo	= newCurveInfo;
	}
	else
	{
		mCurveInfo = curveInfo;
	}

	mCopiedData	= copiedData;

	mMaxX	= mCurveInfo->fMaxX;
	mMinX	= mCurveInfo->fMinX;
	mRatio	= ((f32) (mCurveInfo->iSize - 1)) / (mMaxX - mMinX);
	mValues	= (f32*)((s32)mCurveInfo + sizeof(CurveInfo));
}

#endif

xbool C2dFCurve::load(FileId file, xbool copyData)
{
	if( file == FileId::INVALID_ID )
		return xFALSE;

	if ( mCurveInfo != NULL )
	{
		if ( mFileId == file )
		{
			return xTRUE;
		}
		else
		{
			vClean();
		}
	}

	mFileId = file;
	return CurveManager::sGetInstance()->load(*this, file, copyData);
}

void C2dFCurve::setCurve(FileId file, CurveInfo* curveInfo, xbool copiedData)
{
	ASSERT(curveInfo);

	mFileId = file;

	if ( copiedData )
	{
		if ( mCopiedData && mCurveInfo )
		{
			x_delete( (char*)mCurveInfo );
		}
		
		CurveInfo*	newCurveInfo	= (CurveInfo*)( x_new( char, curveInfo->iSize * sizeof(f32) + sizeof(CurveInfo), XMEM_FLAGS_DEFAULT ) );

		x_memcpy(newCurveInfo,curveInfo,curveInfo->iSize * sizeof(f32) + sizeof(CurveInfo));

		mCurveInfo	= newCurveInfo;
	}
	else
	{
		mCurveInfo = curveInfo;
	}

	mCopiedData	= copiedData;

	mMaxX	= mCurveInfo->fMaxX;
	mMinX	= mCurveInfo->fMinX;
	mRatio	= ((f32) (mCurveInfo->iSize - 1)) / (mMaxX - mMinX);
	mValues	= (f32*)((s32)mCurveInfo + sizeof(CurveInfo));
}

#endif