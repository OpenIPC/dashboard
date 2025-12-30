#ifndef _PICTURE_CTRL_
#define _PICTURE_CTRL_

#pragma once
#include "afxwin.h"

class CPictureCtrl : public CStatic
{
public:	
	CPictureCtrl(void);	
	~CPictureCtrl(void);

public:		
	//Loads an image from a byte stream;
	BOOL LoadFromStream(BYTE* pData, size_t nSize);	
	BOOL Load(BYTE* pData, size_t nSize);
	//Frees the image data
	void FreeData();

protected:
	virtual void PreSubclassWindow();
	//Draws the Control
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnEraseBkgnd(CDC* pDC);

private:
	//Internal image stream buffer
	IStream* m_pStream;
	//Control flag if a picture is loaded
	BOOL m_bIsPicLoaded;
	//GDI Plus Token
	ULONG_PTR m_gdiplusToken;

	// 
	static unsigned int m_nTag;
};

#endif