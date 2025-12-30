#include "StdAfx.h"
#include "PictureCtrl.h"
#include <GdiPlus.h>
using namespace Gdiplus;

//Macro to release COM Components

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif
#define SAFE_RELEASE(x) do{\
							if((x) != NULL)\
							{\
								while((x)->Release() != 0);\
								(x) = NULL;\
							}\
						}while(0)

unsigned int CPictureCtrl::m_nTag = 1;
 
CPictureCtrl::CPictureCtrl(void):CStatic()
{
    m_pStream = NULL;
    m_bIsPicLoaded = FALSE;
    m_gdiplusToken = 0;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CPictureCtrl::~CPictureCtrl(void)
{
	//Tidy up
	FreeData();
	GdiplusShutdown(m_gdiplusToken);
}

BOOL CPictureCtrl::LoadFromStream(BYTE* pData, size_t nSize)
{
	if (1 == m_nTag)
	{
		ModifyStyle(0, SS_OWNERDRAW);
		++m_nTag;
	}
	
	//Set success error state
	SetLastError(ERROR_SUCCESS);
	FreeData();

	//Allocate stream
	DWORD dwResult = CreateStreamOnHGlobal(NULL, TRUE, &m_pStream);
	if(dwResult != S_OK)
	{
		SetLastError(dwResult);
		return FALSE;
	}

	//Copy argument data to the stream
	dwResult = m_pStream->Write(pData, (ULONG)nSize, NULL);
	if(dwResult != S_OK)
	{
		SetLastError(dwResult);
		SAFE_RELEASE(m_pStream);
		return FALSE;
	}

	//Mark as loaded
	m_bIsPicLoaded = TRUE;
	RedrawWindow();
	return TRUE;
}


BOOL CPictureCtrl::Load(BYTE* pData, size_t nSize)
{
	return LoadFromStream(pData, nSize);
}

void CPictureCtrl::FreeData()
{
	m_bIsPicLoaded = FALSE;
	SAFE_RELEASE(m_pStream);
}

void CPictureCtrl::PreSubclassWindow()
{	
	CStatic::PreSubclassWindow();	
//	ModifyStyle(0, SS_OWNERDRAW);
}

void CPictureCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	//Check if pic data is loaded
	if(m_bIsPicLoaded)
	{
		if (m_pStream == NULL)
		{
			CStatic::DrawItem(lpDrawItemStruct);
			return;
		}

		//Get control measures
		RECT rc;
		this->GetClientRect(&rc);

		Graphics graphics(lpDrawItemStruct->hDC);
		Image image(m_pStream);
		graphics.DrawImage(&image, (INT)rc.left, (INT)rc.top, (INT)(rc.right-rc.left), (INT)(rc.bottom-rc.top));		
	}
}

BOOL CPictureCtrl::OnEraseBkgnd(CDC *pDC)
{
	if(m_bIsPicLoaded)
	{
		//Get control measures
		RECT rc;
		this->GetClientRect(&rc);

		Graphics graphics(pDC->GetSafeHdc());
		LARGE_INTEGER liSeekPos;
		liSeekPos.QuadPart = 0;
		m_pStream->Seek(liSeekPos, STREAM_SEEK_SET, NULL);
		Image image(m_pStream);
		graphics.DrawImage(&image, (INT)rc.left, (INT)rc.top, (INT)(rc.right-rc.left), (INT)(rc.bottom-rc.top));
		return TRUE;
	}
	else
	{
		return CStatic::OnEraseBkgnd(pDC);
	}
}