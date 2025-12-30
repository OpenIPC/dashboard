// VideoScreen.cpp : implementation file
//

#include "stdafx.h"
#include <cmath>
#include "DemoMonitorWall.h"
#include "VideoScreen.h"
#include "VideoWnd.h"
#include "memdc.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVideoScreen

CVideoScreen::CVideoScreen()
: m_nCurSel(0)
, m_emWndCtrlType(FIXED_WINDOW)
, m_nAction(NONE)
{
}

CVideoScreen::~CVideoScreen()
{
	Clear();
}

BEGIN_MESSAGE_MAP(CVideoScreen, CWnd)
	//{{AFX_MSG_MAP(CVideoScreen)
	//ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoScreen message handlers

BOOL CVideoScreen::OnEraseBkgnd(CDC* pDC) 
{
	CRect rcClient;
	GetClientRect(rcClient);

	CMemDC dc(pDC, rcClient);
	dc->FillSolidRect(rcClient, SCREEN_BACK_COLOR);
 	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}

void CVideoScreen::Clear()
{
	vector<CVideoWnd*>::iterator it = m_vecWnd.begin();
	for (; it != m_vecWnd.end(); ++it)
	{
		CVideoWnd* pWnd = *it;
		if (pWnd)
		{
			pWnd->DestroyWindow();
			delete pWnd;
			pWnd = NULL;
		}
	}
	m_vecWnd.clear();
}

void CVideoScreen::SetSplitMode( UINT nSplit )
{
	UINT nCurWndCount = m_vecWnd.size();

	if (nCurWndCount == nSplit)
	{
		return;
	}

	//DrawActiveWindow(FALSE);

	// 窗口数量减少
	if (nCurWndCount > nSplit)
	{
		for (int i = nSplit; i < nCurWndCount; ++i)
		{
			CVideoWnd* pWnd = m_vecWnd[i];
			if (pWnd)
			{
				pWnd->DestroyWindow();
				delete pWnd;
				pWnd = NULL;
			}
		}
		m_vecWnd.resize(nSplit);

		if (m_nCurSel >= nSplit)
		{
			m_nCurSel = 0;
		}
	}
	// 窗口增加
	else if (nCurWndCount < nSplit)
	{
		for (int i = nCurWndCount; i < nSplit; ++i)
		{
			CVideoWnd* pWnd = new CVideoWnd();
			VERIFY(pWnd->Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 13579, NULL));
			m_vecWnd.push_back(pWnd);
			pWnd->SetWndID(i);
		}
	}

	Dispose();
	SetActiveWindow(0);
	//DrawActiveWindow(FALSE);
}

void CVideoScreen::Dispose()
{
	if (m_vecWnd.size() <= 0)
	{
		return;
	}

	CRect rcClient;
	GetClientRect(rcClient);

	int nWndCount = m_vecWnd.size();
	int nCol = 1;
	int nRow = 1;
	nRow = nCol = sqrt((float)nWndCount);

	const int nSpace = 0;
	int nWndWidth = (rcClient.Width() - nSpace) / nCol - nSpace;
	int nWndHeight = (rcClient.Height() - nSpace) / nRow - nSpace;
	
	if (nRow * nCol == nWndCount)
	{
		for (int y = 0; y < nRow; ++y)
		{
			for (int x = 0; x < nCol; ++x)
			{
				int nIndex = y * nCol + x;
				if (nIndex < nWndCount)
				{
					CVideoWnd* pWnd = m_vecWnd[nIndex];
					if (pWnd)
					{
						pWnd->MoveWindow(x * (nWndWidth + nSpace) + nSpace, y * (nWndHeight + nSpace) + nSpace, nWndWidth, nWndHeight);
						pWnd->ShowCloseBtn(FALSE);
					}
				}
			}
		}
	}
	else if (nWndCount == 8)
	{
		int i = 0;
		nRow = nCol = 4;
		nWndWidth = (rcClient.Width() - nSpace) / nCol - nSpace;
		nWndHeight = (rcClient.Height() - nSpace) / nRow - nSpace;
		// window 0
		CRect rcWnd(nSpace, nSpace, nSpace + (nCol - 1) * (nSpace + nWndWidth), (nRow - 1) * (nSpace + nWndHeight));
		m_vecWnd[i]->ShowCloseBtn(FALSE);
		m_vecWnd[i++]->MoveWindow(rcWnd);
		// window 1-3
		rcWnd.left = rcWnd.right + nSpace;
		rcWnd.right += nSpace + nWndWidth;
		rcWnd.bottom = nSpace + nWndHeight;
		for (; i <= 3; ++i)
		{
			m_vecWnd[i]->MoveWindow(rcWnd);
			m_vecWnd[i]->ShowCloseBtn(FALSE);
			rcWnd.OffsetRect(0, nSpace + nWndHeight);
		}
		// window 4-7
		for (i = 4; i <= 7; ++i)
		{
			m_vecWnd[i]->MoveWindow(rcWnd);
			m_vecWnd[i]->ShowCloseBtn(FALSE);
			rcWnd.OffsetRect(-(nSpace + nWndWidth), 0);
		}
	}
}

BOOL CVideoScreen::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{	
	BOOL bResult = CWnd::CreateEx(WS_EX_CLIENTEDGE, lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	SetSplitMode(9);

	return bResult;
}

void CVideoScreen::SetActiveWindow( const CVideoWnd* pWnd )
{
	if(pWnd == NULL || !IsWindow(pWnd->m_hWnd))
	{
		return;
	}
	
	for (int i = 0; i < m_vecWnd.size(); ++i)
	{
		DrawActiveWindow(FALSE);
		if (m_vecWnd[i] == pWnd)
		{
			m_nCurSel = pWnd->GetWndID();
			DrawActiveWindow();
			break;
		}
	}
}

void CVideoScreen::SetActiveWindow( int nWndID )
{
	SetActiveWindow(GetWindow(nWndID));
// 	if (GetWindow(nWndID))
// 	{
// 		DrawActiveWindow(FALSE);
// 		m_nCurSel = nWndID;
// 		DrawActiveWindow();
// 	}
}

void CVideoScreen::DrawActiveWindow( BOOL bActive/* = TRUE*/ )
{
	CVideoWnd* pWnd = GetWindow(m_nCurSel);
	if (pWnd && pWnd->IsActive() != bActive)
	{
		pWnd->SetActive(bActive);
// 		CRect rcWnd;
// 		pWnd->GetWindowRect(rcWnd);
// 		ScreenToClient(rcWnd);
// 		rcWnd.InflateRect(1, 1);
// 		
// 		CClientDC dc(this);
// 		
// 		if (bActive)
// 		{
// 			dc.Draw3dRect(rcWnd, ACTIVE_COLOR_TOP_LEFT, ACTIVE_COLOR_RIGHT_BOTTOM);
// 		}
// 		else
// 		{
// 			dc.Draw3dRect(rcWnd, SCREEN_BACK_COLOR, SCREEN_BACK_COLOR);
// 		}
	}
}

void CVideoScreen::OnVideoLButtonDown( int nWndID, WPARAM wParam, LPARAM lParam )
{
	if (m_emWndCtrlType == SLIP_WINDOW)
	{
		CVideoWnd* pWnd = GetWindow(nWndID);
		if (pWnd)
		{
			CPoint pt(lParam);
			pWnd->ClientToScreen(&pt);

			ScreenToClient(&pt);

			if (PtInRect(&pWnd->GetHeaderRect(), CPoint(lParam)))
			{
				m_nAction = MOVE;
				m_ptStart = pt;
				pWnd->GetWindowRect(m_rcStart);
				ScreenToClient(m_rcStart);
				pWnd->SetCapture();
			}
			else if (PtInRect(&pWnd->GetResizeRect(), CPoint(lParam)))
			{
				m_nAction = ZOOM;
				m_ptStart = pt;
				pWnd->GetWindowRect(m_rcStart);	
				ScreenToClient(m_rcStart);
				pWnd->SetCapture();
			}
			pWnd->BringWindowToTop();
		}
	}
}

void CVideoScreen::OnVideoMouseMove( int nWndID, WPARAM wParam, LPARAM lParam )
{
	if (m_emWndCtrlType == SLIP_WINDOW)
	{
		CPoint ptNow(lParam);
		CRect rcWndNow;
		CRect rcSreen;
		CVideoWnd* pWnd = GetWindow(nWndID);
		GetClientRect(rcSreen);
		
		if (pWnd && pWnd == GetCapture())
		{			
			pWnd->ClientToScreen(&ptNow);
			ScreenToClient(&ptNow);

			if (m_nAction == MOVE)
			{
				CPoint ptOffset(ptNow.x - m_ptStart.x, ptNow.y - m_ptStart.y);
				rcWndNow = m_rcStart;
				rcWndNow.OffsetRect(ptOffset);

				ptOffset.x = ptOffset.y = 0;
				if (rcWndNow.left < rcSreen.left)
				{
					ptOffset.x = rcSreen.left - rcWndNow.left;
				}
				if (rcWndNow.top < rcSreen.top)
				{
					ptOffset.y = rcSreen.top - rcWndNow.top;
				}
				if (ptOffset.x != 0 || ptOffset.y != 0)
				{
					rcWndNow.OffsetRect(ptOffset);
				}

				ptOffset.x = ptOffset.y = 0;
				if (rcWndNow.right > rcSreen.right)
				{
					ptOffset.x = rcSreen.right - rcWndNow.right;
				}
				if (rcWndNow.bottom > rcSreen.bottom)
				{
					ptOffset.y = rcSreen.bottom - rcWndNow.bottom;
				}
				if (ptOffset.x != 0 || ptOffset.y != 0)
				{
					rcWndNow.OffsetRect(ptOffset);
				}
		
				pWnd->MoveWindow(rcWndNow);
			}
			else if (m_nAction == ZOOM)
			{
				CPoint ptOffset(ptNow.x - m_ptStart.x, ptNow.y - m_ptStart.y);
				rcWndNow = m_rcStart;
				rcWndNow.InflateRect(0, 0, ptOffset.x, ptOffset.y);

				if (rcWndNow.Width() < CVideoWnd::MinSize.cx)
				{
					rcWndNow.right = rcWndNow.left + CVideoWnd::MinSize.cx;
				}
				if (rcWndNow.Height() < CVideoWnd::MinSize.cy)
				{
					rcWndNow.bottom = rcWndNow.top + CVideoWnd::MinSize.cy;
				}

				if (rcWndNow.right > rcSreen.right)
				{
					rcWndNow.right = rcSreen.right;
				}
				if (rcWndNow.bottom > rcSreen.bottom)
				{
					rcWndNow.bottom = rcSreen.bottom;
				}
				pWnd->MoveWindow(rcWndNow);
			}			
		}
	}
}

void CVideoScreen::OnVideoLButtonUp( int nWndID, WPARAM wParam, LPARAM lParam )
{
	if (m_emWndCtrlType == SLIP_WINDOW)
	{
		CRect rcWnd;
		CVideoWnd* pWnd = GetWindow(nWndID);
		
		if (pWnd == GetCapture() || m_nAction == MOVE || m_nAction == ZOOM)
		{
			pWnd->GetWindowRect(rcWnd);
			ScreenToClient(&rcWnd);			
			GetParent()->SendMessage(WM_SET_VIDEOWINDOW_RECT, nWndID, (LPARAM)&rcWnd);
		}
		m_nAction = NONE;
		ReleaseCapture();
	}
}

void CVideoScreen::OnVideoLButtonDblClk( int nWndID, WPARAM wParam, LPARAM lParam )
{
	
}

void CVideoScreen::OnVideoRButtonDown( int nWndID, WPARAM wParam, LPARAM lParam )
{
	
}

void CVideoScreen::OnVideoRButtonUp( int nWndID, WPARAM wParam, LPARAM lParam )
{
	
}

void CVideoScreen::OnVideoMove( int nWndID, WPARAM wParam, LPARAM lParam )
{
	
}

void CVideoScreen::OnVideoMoving( int nWndID, WPARAM wParam, LPARAM lParam )
{
	
}

void CVideoScreen::OnCloseButtonClicked( int nWndID, WPARAM wParam, LPARAM lParam )
{
	// 自由风格模式下, 关闭窗口才有效
	if (m_emWndCtrlType == SLIP_WINDOW)
	{
		GetParent()->SendMessage(WM_CLOSE_VIDEOWINDOW, nWndID, 0);
	}
}

void CVideoScreen::OnClearButtonClicked( int nWndID, WPARAM wParam, LPARAM lParam )
{
	GetParent()->SendMessage(WM_CLEAR_VIDEOWINDOW, nWndID, 0);
}

void CVideoScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CRect rcClient;
	GetClientRect(rcClient);
	dc.FillSolidRect(rcClient, SCREEN_BACK_COLOR);
	
// 	CRect rcClient;
// 	GetClientRect(rcClient);
// 	dc.Draw3dRect(rcClient, RGB(113,111,100), RGB(255,255,255));
//	DrawActiveWindow(TRUE);
}

void CVideoScreen::SetWindowText( UINT nWndID, LPCTSTR lpszText )
{
	CVideoWnd* pWnd = GetWindow(nWndID);
	if (pWnd)
	{
		pWnd->SetWindowText(lpszText);
		pWnd->Invalidate();
	}
}

void CVideoScreen::ClearWindowText()
{
	for (int i = 0; i < m_vecWnd.size(); ++i)
	{
		m_vecWnd[i]->SetWindowText(NULL);
	}
}

CString CVideoScreen::GetWindowText( UINT nWndID )
{
	CString str;
	CVideoWnd* pWnd = GetWindow(nWndID);
	if (pWnd)
	{
		pWnd->GetWindowText(str);
	}
	return str;
}

CVideoWnd* CVideoScreen::GetWindow( UINT nWndID )
{
	for (int i = 0; i < m_vecWnd.size(); ++i)
	{
		if (m_vecWnd[i]->GetWndID() == nWndID)
		{
			return m_vecWnd[i];
		}
	}
	return NULL;
}

void CVideoScreen::SetWindowControlType( WindowControlType emType )
{
	m_emWndCtrlType = emType;
	Clear();
	Invalidate();
}

BOOL CVideoScreen::AddWindow( const CRect& rect, UINT nWndID, LPCTSTR lpszText /*= NULL*/ )
{
	if (m_emWndCtrlType == SLIP_WINDOW)
	{
		CVideoWnd* pWnd = GetWindow(nWndID);
		if (pWnd == NULL)
		{
			pWnd = new CVideoWnd();
			VERIFY(pWnd->Create(NULL, NULL, WS_VISIBLE | WS_CHILD, rect, this, 13579, NULL));
			m_vecWnd.push_back(pWnd);
			pWnd->SetWndID(nWndID);
			pWnd->SetWindowText(lpszText);
			pWnd->EnableResize(TRUE);
			pWnd->ShowCloseBtn(TRUE);
		}
	}
	return FALSE;
}

void CVideoScreen::RemoveWindow( UINT nWndID )
{
	if (m_emWndCtrlType == SLIP_WINDOW)
	{
		if (nWndID == -1)
		{
			Clear();
		}
		else
		{
			for (std::vector<CVideoWnd*>::iterator it = m_vecWnd.begin(); it != m_vecWnd.end(); ++it)
			{
				CVideoWnd* pWnd = *it;
				if (pWnd && pWnd->GetWndID() == nWndID)
				{
					m_vecWnd.erase(it);
					pWnd->DestroyWindow();
					delete pWnd;
					pWnd = NULL;
					Invalidate();
					break;
				}
			}
		}
	}
}