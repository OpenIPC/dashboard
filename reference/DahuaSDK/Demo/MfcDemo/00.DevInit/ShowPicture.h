// ShowPicture.h: interface for the CShowPicture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHOWPICTURE_H__853A4490_20BE_4E41_9091_1C9B7212A51C__INCLUDED_)
#define AFX_SHOWPICTURE_H__853A4490_20BE_4E41_9091_1C9B7212A51C__INCLUDED_

#if (_MSC_VER == 1200)	
#include <olestd.h>
#endif

#if (_MSC_VER == 1400)
#include <atlbase.h>
#include <atlwin.h>
#endif

#include <sys/stat.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct PICTURES
{
	void *Picture;
	long PictureWidth;
	long PictureHeight;
	int PositionX;
	int PositionY;
	HWND hWnd;
}Pictures, *pPictures;

class CShowPicture  
{
public:
	CShowPicture();
	virtual ~CShowPicture();
	void RepaintPictures();
	void AddPicture(HWND hWnd, char *FileName, int PositionX, int PositionY);
	IPicture *LoadPicture(HWND hWnd, const unsigned char *data, size_t len, long *ret_w,long *ret_h);

private:
	void RemovePicture();
	void RanderPicture(HDC dc, const RECT &bounds, void *pic);
	bool m_pictureLoaded;
	Pictures m_picture;
};

#endif // !defined(AFX_SHOWPICTURE_H__853A4490_20BE_4E41_9091_1C9B7212A51C__INCLUDED_)
