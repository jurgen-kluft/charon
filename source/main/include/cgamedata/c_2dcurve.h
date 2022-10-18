//////////////////////////////////////////////////////////////////////////////////////////
//  Fichier :       Maths/2dCurve.h                                                     //
//  Creation :      201099 par Francois                                                 //
//  Role :          Definition de la classe C2dFCurve qui represente un echantillonage  //
//                  de courbe                                                           //
//////////////////////////////////////////////////////////////////////////////////////////             

#ifndef _2DCURVE_H_
#define _2DCURVE_H_

#ifdef M_PRAGMA_ONCE
#  pragma once
# endif // M_PRAGMA_ONCE

#include "IO/Resources.h" //use the FileId class defined in this header file

enum ECurveState
{
	ECurveStateSTANDALONE,
	ECurveStateSHAREDVALUES,
	ECurveStateSHARED
};

class SResource;

struct CurveInfo
{
	ncore::f32 fMinX, fMaxX;
	ncore::s32 iSize;
};

class C2dFCurve
{
public:
	C2dFCurve();
	~C2dFCurve();
	void		vClean();
	ncore::xbool		load(FileId file, ncore::xbool copyData = false);
#ifdef ENABLE_CURVE_FILE_NAME
	ncore::xbool        load(const char* file, ncore::xbool copyData = false);
	const char*			getFileName() const               { return mFileName; }
#endif
	ncore::u32		iGetMagicNb() const;
	void		vSetValue(ncore::s32 _iIndex,ncore::f32 _fValue);
	void		vMultiplyX(ncore::f32 _fXCoeff);
	void		vMultiplyY(ncore::f32 _fXCoeff);
	void		vYDegToRad();
	void		vXDegToRad();

	ncore::f32		fReadValue(ncore::f32 _fX) const;
	ncore::f32		fReadAnyValue(ncore::f32 _fX) const;
	ncore::f32		fGetAbscissa(ncore::s32 _iIndex) const;
	ncore::f32		fGetMaxY() const;

	ncore::f32		fGetAbscissaMaxY() const;
	ncore::f32		fGetMinY() const;
	ncore::f32		fGetMaxX() const {ASSERT(mCurveInfo); return mMaxX;}
	ncore::f32		fGetMinX() const {ASSERT(mCurveInfo); return mMinX;}

	ncore::f32		fGetLastNegativeXFromEnd() const;

	FileId			getFileId() const				{ return mFileId; }
protected:
	friend class CurveManager;
	void		setCurve(FileId file, CurveInfo* curveInfo, ncore::xbool copiedData = false);

	ncore::f32		mRatio;
	ncore::f32		mMinX;
	ncore::f32		mMaxX;
	ncore::xbool	mCopiedData;

	CurveInfo*		mCurveInfo;

	ncore::f32*		mValues;

	FileId			mFileId;
#ifdef ENABLE_CURVE_FILE_NAME
	char			mFileName[256];
	void			setCurve(const char* file, CurveInfo* curveInfo, ncore::xbool copiedData = false);
#endif
};

inline ncore::f32 C2dFCurve::fGetAbscissa(ncore::s32 _iIndex) const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ASSERT(mValues != NULL);
	ASSERT(_iIndex >= 0);
	ASSERT (_iIndex < mCurveInfo->iSize);
	return mMinX + ((ncore::f32) _iIndex) / mRatio;
}

inline ncore::f32 C2dFCurve::fGetLastNegativeXFromEnd() const
{
	if( mCurveInfo == NULL )
		return 0.0f;

	ncore::s32 i = mCurveInfo->iSize - 1; 
	while (i > 0)
	{
		if (mValues[i - 1] > 0)
			return fGetAbscissa(i);
		-- i;
	}
	return fGetAbscissa(0);  
}

inline ncore::u32 C2dFCurve::iGetMagicNb() const
{
	if( mCurveInfo == NULL )
		return 0;

	ASSERT(sizeof(ncore::u32) == sizeof(ncore::f32));
	ASSERT(mCurveInfo->iSize == 0 || mValues != NULL);
	ncore::u32 iMagicNb = 0;
	ncore::s32 i;
	for(i= 0; i < mCurveInfo->iSize; i++) 
		iMagicNb += * (ncore::u32*) &mValues[i];

	return iMagicNb;
}



#endif // _2DCURVE_H_
