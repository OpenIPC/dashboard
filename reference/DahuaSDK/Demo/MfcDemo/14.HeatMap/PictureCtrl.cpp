// PictureCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "PictureCtrl.h"

#define	PICTURE_BACK_COLOR	RGB(105,105,105)
const static COLORREF RESERVED_COLOR = RGB(255, 255, 255);

// PictureCtrl
IMPLEMENT_DYNAMIC(PictureCtrl, CStatic)

PictureCtrl::PictureCtrl() : m_pPicture(NULL)
{

}

PictureCtrl::~PictureCtrl()
{
	if (m_pPicture != NULL)
	{
		m_pPicture->Release();
		m_pPicture = NULL;
	}
}

void PictureCtrl::SetImageFile(LPCSTR szImagePath)
{
	USES_CONVERSION;

	// 获取展示窗口
	RECT rect;
	this->GetWindowRect(&rect);

	CDC* pDC = NULL;
	pDC = this->GetWindowDC();
	if (NULL == pDC)
	{
		return;
	}

	if (m_pPicture != NULL)
	{
		m_pPicture->Release();
		m_pPicture = NULL;
	}

	// 加载图片
	HRESULT hr = S_FALSE;
	hr = OleLoadPicturePath(T2OLE(szImagePath), NULL, 0, RESERVED_COLOR, IID_IPicture, (LPVOID*)&m_pPicture);
	if (NULL == m_pPicture)
	{
		this->ReleaseDC(pDC);
		return;
	}

	//显示全图
	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	m_pPicture->get_Width(&hmWidth); 
	m_pPicture->get_Height(&hmHeight); 

	hr = m_pPicture->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	this->ReleaseDC(pDC);
}

BEGIN_MESSAGE_MAP(PictureCtrl, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void PictureCtrl::OnPaint()
{
	if (NULL != m_pPicture)
	{
		CPaintDC dc(this);

		RECT rect;
		this->GetWindowRect(&rect);
		OLE_XSIZE_HIMETRIC hmWidth; 
		OLE_YSIZE_HIMETRIC hmHeight; 
		m_pPicture->get_Width(&hmWidth); 
		m_pPicture->get_Height(&hmHeight); 
		m_pPicture->Render(dc.m_hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);	
	}
	else
	{
		CDC* cdc = this->GetDC();
		if (NULL == cdc)
		{
			return;
		}

		RECT rect;
		this->GetClientRect(&rect);
		CBrush brush(PICTURE_BACK_COLOR);
		cdc->FillRect(&rect, &brush);
		this->ReleaseDC(cdc);
	}

	CWnd::OnPaint();
}

