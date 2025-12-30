// ShowPicture.cpp: implementation of the CShowPicture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "searchdevice.h"
#include "ShowPicture.h"
#include <iostream>

#pragma warning(disable:4291)
#ifndef NEW
#define NEW new(std::nothrow)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



CShowPicture::CShowPicture()
{
	m_pictureLoaded = false;
	memset(&m_picture, 0, sizeof(Pictures));
}

CShowPicture::~CShowPicture()
{
	RemovePicture();
}


void CShowPicture::AddPicture(HWND hWnd, char *FileName, int PositionX, int PositionY)
{
	if(m_pictureLoaded)
		return;
	FILE *fp = NULL;
	fp = fopen(FileName, "rb");
	if( NULL == fp)
	{
		m_pictureLoaded = false;
		return;
	}
	
	struct stat file_stat;
	if( -1 ==fstat(fileno(fp), &file_stat))
	{
		m_pictureLoaded = false;
		fclose(fp);
		return;
	}
	unsigned char *f_buf = NEW unsigned char[4*file_stat.st_size];

	if (NULL == f_buf)
	{		
		m_pictureLoaded = false;
		fclose(fp);
		return;
	}
	long file_size = fread(f_buf, 1, file_stat.st_size, fp);
	fclose(fp);

	if(file_size != file_stat.st_size)
	{
		m_pictureLoaded = false;
		delete[] f_buf;
        f_buf = NULL;
		return;
	}

	m_picture.Picture = LoadPicture(hWnd, f_buf, file_size, &m_picture.PictureWidth,&m_picture.PictureHeight);
	if(m_picture.Picture != NULL)
	{
		m_picture.PositionX = PositionX;
		m_picture.PositionY = PositionY;
		m_picture.hWnd = hWnd;
		m_pictureLoaded = true;
	}

    delete[] f_buf;
    f_buf = NULL;
}
IPicture *CShowPicture::LoadPicture(HWND hWnd, const unsigned char *data, size_t len, long *ret_w,long *ret_h)
{
	IPicture *pic =NULL;
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, len);
	LPVOID pvData = GlobalLock(hGlobal);
	memcpy(pvData, data, len);
	GlobalUnlock(hGlobal);
	LPSTREAM pStream = NULL;
	HRESULT hr = CreateStreamOnHGlobal(hGlobal, TRUE, &pStream);
	OleLoadPicture(pStream, 0, FALSE, IID_IPicture, (void **)&pic);
	pStream->Release();
	GlobalFree(hGlobal);

	OLE_XSIZE_HIMETRIC cx;
	OLE_XSIZE_HIMETRIC cy;
	pic->get_Width(&cx);
	pic->get_Height(&cy);
	*ret_w = MAP_LOGHIM_TO_PIX(cx, GetDeviceCaps(GetDC(hWnd), LOGPIXELSX));
	*ret_h = MAP_LOGHIM_TO_PIX(cy, GetDeviceCaps(GetDC(hWnd), LOGPIXELSX));
	return pic;
}
void CShowPicture::RanderPicture(HDC dc, const RECT &bounds, void *pic)
{
	IPicture *picture = (IPicture*)pic;
	OLE_XSIZE_HIMETRIC cx;
	OLE_XSIZE_HIMETRIC cy;
	picture->get_Width(&cx);
	picture->get_Height(&cy);
	picture->Render(dc, bounds.left, bounds.bottom, bounds.right-bounds.left, bounds.top-bounds.bottom, 0,0, cx,cy, NULL);

}

void CShowPicture::RepaintPictures()
{
	if(!m_pictureLoaded)
		return;
	RECT bounds;
	RECT temp_rect;
	::GetWindowRect(m_picture.hWnd, &temp_rect);
	bounds.top = m_picture.PositionY;
	bounds.bottom = m_picture.PositionY + m_picture.PictureHeight;
	bounds.left = m_picture.PositionX;
	bounds.right = m_picture.PositionX + m_picture.PictureWidth;
	RanderPicture(GetDC(m_picture.hWnd), bounds, m_picture.Picture);
}

void CShowPicture::RemovePicture()
{
	if(!m_pictureLoaded)
		return;
	IPicture *freepic = (IPicture*)m_picture.Picture;
	freepic->Release();
	memset(&m_picture, 0, sizeof(m_picture));
	m_pictureLoaded = FALSE;
}