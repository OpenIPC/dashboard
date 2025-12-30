// PictureWnd.cpp : implementation file
//

#include "stdafx.h"
#include "PictureWnd.h"
//#include "ParkingSpaceDetector.h"
#include "ParkingSpaceUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPictureWnd

CPictureWnd::CPictureWnd()
{
	m_hImageMemory = NULL;
	m_pIPicture = NULL;
	m_nScale = 100;

	memset(&m_siVert, 0, sizeof(m_siVert));
	memset(&m_siHorz, 0, sizeof(m_siHorz));
	m_hImageMemory = NULL;	
}

CPictureWnd::~CPictureWnd()
{
}


BEGIN_MESSAGE_MAP(CPictureWnd, CWnd)
	//{{AFX_MSG_MAP(CPictureWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPictureWnd message handlers

BOOL CPictureWnd::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rt;
	GetClientRect(&rt);
	CBrush br;
	br.CreateSolidBrush(PICTURE_BACK_COLOR);
	pDC->FillRect(&rt,&br);

	return TRUE;
//	return CWnd::OnEraseBkgnd(pDC);
}


int CPictureWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	::CoInitialize(NULL);
	
	return 0;
}

void CPictureWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_hImageMemory != NULL)
	{
		GlobalFree(m_hImageMemory);
		m_hImageMemory = NULL;
	}

	BOOL bLock = m_csIPictrue.Lock();
	if (bLock && m_pIPicture != NULL)
	{
		m_pIPicture->Release();
		m_pIPicture = NULL;
	}
	m_csIPictrue.Unlock();
	
	::CoUninitialize();
}

void CPictureWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	BOOL bLock = m_csIPictrue.Lock();
	if (bLock && m_pIPicture != NULL)
	{
		long hmWidth;
		long hmHeight;
		m_pIPicture->get_Width(&hmWidth); // 用接口方法获得图片的宽和高
		m_pIPicture->get_Height(&hmHeight);

		SIZE sz;
		sz.cx = hmWidth;
		sz.cy = hmHeight;
		dc.HIMETRICtoDP (&sz);
		int nWidth = (sz.cx * m_nScale) / 100;
		int nHeight = (sz.cy * m_nScale) / 100;
		RECT rc;
		GetClientRect(&rc);

		if ((rc.right - rc.left) > nWidth)
		{
			ShowScrollBar(SB_HORZ, FALSE);
			EnableScrollBar (SB_HORZ, ESB_DISABLE_BOTH);
			SetScrollRange (SB_HORZ, 0, nWidth, TRUE);
		}
		else
		{
			ShowScrollBar(SB_HORZ, TRUE);
			m_siHorz.cbSize = sizeof (SCROLLINFO);
			m_siHorz.nMin = 0;
			m_siHorz.fMask = SIF_ALL;
			m_siHorz.nMax = nWidth;
			m_siHorz.nPage = rc.right - rc.left;
			SetScrollInfo (SB_HORZ, &m_siHorz, true);
			EnableScrollBar (SB_HORZ, ESB_ENABLE_BOTH);
			SetScrollRange (SB_HORZ, 0, nWidth, TRUE);
//			SetScrollRange (SB_VERT, 0, nHeight, TRUE);
		}
		if ((rc.bottom - rc.top) > nHeight)
		{
			ShowScrollBar(SB_VERT, FALSE);
			EnableScrollBar (SB_VERT, ESB_DISABLE_BOTH);
		}
		else
		{
			ShowScrollBar(SB_VERT, TRUE);
			m_siVert.cbSize = sizeof (SCROLLINFO);
			m_siVert.nMin = 0;
			m_siVert.fMask = SIF_ALL;
			m_siVert.nMax = nHeight;
			m_siVert.nPage = rc.bottom - rc.top;
			SetScrollInfo (SB_VERT, &m_siVert, true);
			EnableScrollBar (SB_VERT, ESB_ENABLE_BOTH);
			SetScrollRange (SB_VERT, 0, nHeight, TRUE);
		}

		sz.cx = m_siHorz.nPos;
		sz.cy = m_siVert.nPos;
		dc.DPtoHIMETRIC (&sz);
		sz.cx = (sz.cx * 100) / m_nScale;
		sz.cy = (sz.cy * 100) / m_nScale;

		// 在指定的DC上绘出图片
		m_pIPicture->Render(dc.m_hDC,
						  0,			//	Horizontal position of image in the device context
						  0,			//	Vertical position of image in the device context
						  nWidth,		//	Horizontal dimension of destination rectangle
						  nHeight,		//	Vertical dimension of destination rectangle
						  sz.cx,		//	Horizontal offset in source picture
						  hmHeight - sz.cy,	//	Vertical offset in source picture
						  hmWidth,		//	Amount to copy horizontally in source picture
						  -hmHeight,	//	Amount to copy vertically in source picture
						  &rc);			//	Pointer to position of destination for a metafile hdc)
	}
	m_csIPictrue.Unlock();
	// Do not call CWnd::OnPaint() for painting messages
}

void CPictureWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	RECT rectClip;
	GetClientRect (&rectClip);

	int	nPosition = 0, nScroll = 0;
	switch (nSBCode)
	{	
		case SB_BOTTOM:			//	Scrolls to the lower right.
		case SB_ENDSCROLL:		//	Ends scroll.
			return;
		case SB_LINEDOWN:		//	Scrolls one line down.
			if ((m_siHorz.nPos + (int) m_siHorz.nPage) >= m_siHorz.nMax)
				return;
			++m_siHorz.nPos;
			ScrollWindowEx (-1, 0, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			break;
		case SB_LINEUP:			//	Scrolls one line right.
			--m_siHorz.nPos;
			if (m_siHorz.nPos < 0)
				m_siHorz.nPos = 0;
			else
				ScrollWindowEx (1, 0, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			break;
		case SB_PAGEDOWN:		//	Scrolls one page right.
			if (m_siHorz.nPos >= m_siHorz.nMax)
				return;
			nPosition = m_siHorz.nPos;
			m_siHorz.nPos += m_siHorz.nPage / 2;
			if ((m_siHorz.nPos + (int) m_siHorz.nPage) > m_siHorz.nMax)
			{
				m_siHorz.nPos = m_siHorz.nMax - m_siHorz.nPage;
				nScroll = m_siHorz.nMax - m_siHorz.nPage - nPosition;
			}
			else
				nScroll = m_siHorz.nPage / 2;
			ScrollWindowEx (-nScroll, 0, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			break;
		case SB_PAGEUP:			//	Scrolls one page left.
			if (m_siHorz.nPos == 0)
				return;
			nPosition = m_siHorz.nPos;
			m_siHorz.nPos -= m_siHorz.nPage / 2;
			if (m_siHorz.nPos < 0)
			{
				m_siHorz.nPos = 0;
				nScroll = nPosition;
			}
			else
				nScroll = m_siHorz.nPage / 2;
			ScrollWindowEx (nScroll, 0, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			break;
		case SB_THUMBPOSITION:	//	The user has dragged the scroll box (thumb) and released the mouse button. The nPos parameter indicates the position of the scroll box at the end of the drag operation.
		case SB_THUMBTRACK:		//	The user is dragging the scroll box. This message is sent repeatedly until the user releases the mouse button. The nPos parameter indicates the position that the scroll box has been dragged to.
			ScrollWindowEx (m_siHorz.nPos - nPos, 0, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			m_siHorz.nPos = nPos;// HIWORD(wParam);
			break;
		case SB_TOP:				//	Scrolls to the upper left.
			return;
			break;
	}
	SetScrollInfo (SB_HORZ, &m_siHorz, true);
	
//	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPictureWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	RECT rectClip;
	GetClientRect (&rectClip);

	int	nPosition = 0, nScroll = 0;
	switch (nSBCode)
	{	
		case SB_BOTTOM:			//	Scrolls to the lower right.
		case SB_ENDSCROLL:		//	Ends scroll.
			return;
		case SB_LINEDOWN:		//	Scrolls one line down.
			if ((m_siVert.nPos + (int) m_siVert.nPage) >= m_siVert.nMax)
				return;
			ScrollWindowEx (0, -1, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			m_siVert.nPos += 1;
			break;
		case SB_LINEUP:			//	Scrolls one line up.
			if (m_siVert.nPos == 0)
				return;
			ScrollWindowEx (0, 1, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			m_siVert.nPos -= 1;
			break;
		case SB_PAGEDOWN:		//	Scrolls one page down.
			if (m_siVert.nPos >= m_siVert.nMax)
				return;
			nPosition = m_siVert.nPos;
			m_siVert.nPos += (int) m_siVert.nPage / 2;
			if ((m_siVert.nPos + (int) m_siVert.nPage) > m_siVert.nMax)
			{
				m_siVert.nPos = m_siVert.nMax - m_siVert.nPage;
				nScroll = nPosition;
			}
			else
				nScroll = m_siVert.nPage / 2;
			ScrollWindowEx (0, -nScroll, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
//			ScrollWindowEx (0, -((int) m_siVert.nPage / 2), NULL, &rClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			break;
		case SB_PAGEUP:			//	Scrolls one page up.
			if (m_siVert.nPos == 0)
				return;
			nPosition = m_siVert.nPos;
			m_siVert.nPos -= m_siVert.nPage / 2;
			if (m_siVert.nPos < 0)
			{
//				Scroll = 0;
				m_siVert.nPos = 0;
				nScroll = nPosition;
			}
			else
				nScroll = m_siVert.nPage / 2;
			ScrollWindowEx (0, nScroll, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
//			ScrollWindowEx (0, m_siVert.nPage / 2, NULL, &rClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			break;
		case SB_THUMBPOSITION:	//	The user has dragged the scroll box (thumb) and released the mouse button. The nPos parameter indicates the position of the scroll box at the end of the drag operation.
		case SB_THUMBTRACK:		//	The user is dragging the scroll box. This message is sent repeatedly until the user releases the mouse button. The nPos parameter indicates the position that the scroll box has been dragged to.
			ScrollWindowEx (0, m_siVert.nPos - nPos, NULL, &rectClip, NULL, NULL, SW_ERASE | SW_INVALIDATE);
			m_siVert.nPos = nPos;
			break;
		case SB_TOP:				//	Scrolls to the upper left.
			return;
	}
	SetScrollInfo (SB_VERT, &m_siVert, true);
	
//	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}