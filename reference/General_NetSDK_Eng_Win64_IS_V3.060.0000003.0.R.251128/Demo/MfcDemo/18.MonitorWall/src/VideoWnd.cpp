// VideoWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DemoMonitorWall.h"
#include "VideoWnd.h"
#include "memdc.h"
#include "VideoScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define GET_PARENT_SCREEN			((CVideoScreen *)(GetParent()))

SIZE CVideoWnd::MinSize = { 50, 50 };

/////////////////////////////////////////////////////////////////////////////
// CVideoWnd

IMPLEMENT_DYNCREATE(CVideoWnd, CWnd)

CVideoWnd::CVideoWnd() : m_nWndID(-1), m_bActive(FALSE), m_bEnableResize(FALSE), m_bShowCloseBtn(FALSE)
{
}

CVideoWnd::~CVideoWnd()
{
}

BEGIN_MESSAGE_MAP(CVideoWnd, CWnd)
	//{{AFX_MSG_MAP(CVideoWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVideoWnd message handlers


LRESULT CVideoWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	switch(message)
	{
	case WM_LBUTTONDOWN:
		{
			GET_PARENT_SCREEN->SetActiveWindow(this);
			GET_PARENT_SCREEN->OnVideoLButtonDown(m_nWndID, wParam, lParam);
			
			POINT point = { LOWORD(lParam), HIWORD(lParam) };

			if (PtInRect(m_rcBtnClose, point))
			{
				GET_PARENT_SCREEN->OnCloseButtonClicked(m_nWndID, wParam, lParam);
			}
			else if (PtInRect(m_rcBtnClear, point))
			{
				GET_PARENT_SCREEN->OnClearButtonClicked(m_nWndID, wParam, lParam);
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			GET_PARENT_SCREEN->OnVideoLButtonUp(m_nWndID, wParam, lParam);
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			GET_PARENT_SCREEN->OnVideoLButtonDblClk(m_nWndID, wParam, lParam);
		}
		break;
	case WM_RBUTTONDOWN:
		{
			GET_PARENT_SCREEN->OnVideoRButtonDown(m_nWndID, wParam, lParam);
		}
		break;
	case WM_RBUTTONUP:
		{
			GET_PARENT_SCREEN->OnVideoRButtonUp(m_nWndID, wParam, lParam);
		}
		break;
	case WM_MOVE:
		{
			GET_PARENT_SCREEN->OnVideoMove(m_nWndID, wParam, lParam);
		}
		break;
	case WM_MOVING:
		{		
			GET_PARENT_SCREEN->OnVideoMoving(m_nWndID, wParam, lParam);
		}
		break;
	case WM_MOUSEMOVE:
		{
			GET_PARENT_SCREEN->OnVideoMouseMove(m_nWndID, wParam, lParam);
		}
	default:
		return CWnd::DefWindowProc(message, wParam, lParam);
	}
 	return TRUE;
 }

void CVideoWnd::OnPaint() 
{
	CPaintDC dcPaint(this); // device context for painting

	static CFont s_textFont;
	if (s_textFont.m_hObject == NULL)
	{
		LOGFONT lf = {0};
		CFont* cf = GetFont();
		if(cf)
		{
			cf->GetObject(sizeof(LOGFONT), &lf);
		}
		else
		{
			GetObject(GetStockObject(SYSTEM_FIXED_FONT), sizeof(LOGFONT), &lf);
		}
		s_textFont.DeleteObject();
		s_textFont.CreateFontIndirect(&lf);
	}

	CRect rcClient;
	GetClientRect(rcClient);	
	CMemDC dc(&dcPaint, rcClient);
//	CDC* dc = &dcPaint;

	dc->SetBkMode(TRANSPARENT);
	dc->FillSolidRect(rcClient, RGB(128, 128, 128));

	CRect rcHeader(rcClient);
	rcHeader.bottom = rcClient.top + 15;
	dc->FillSolidRect(rcHeader, HEADER_COLOR);

	CFont* pOldFont = dc->SelectObject(&s_textFont);

	CString strText;
	GetWindowText(strText);

	TEXTMETRIC tm;
	dc->GetTextMetrics(&tm);
	int nHeightLine = tm.tmHeight + tm.tmExternalLeading;

	// context
	CStringArray arrText;
	int nPos = -1;
	LPCTSTR pszSeparator = "\r\n";
	do 
	{
		nPos = strText.Find(pszSeparator);
		if (nPos >= 0)
		{
			arrText.Add(strText.Left(nPos));
			strText.Delete(0, nPos + strlen(pszSeparator));
		}
		else if (!strText.IsEmpty())
		{
			arrText.Add(strText);
		}
	} while (nPos >= 0 && !strText.IsEmpty());
	
	int y = (rcClient.Height() - arrText.GetSize() * nHeightLine) / 2;
	for (int i = 0; i < arrText.GetSize(); ++i)
	{
		dc->DrawText(arrText.GetAt(i), CRect(0, y, rcClient.right, y + nHeightLine), DT_SINGLELINE | DT_CENTER);
		y += nHeightLine;
	}

	// window id
	strText.Format("%d", m_nWndID);
	dc->DrawText(strText, rcHeader, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	// 关闭按钮
	const COLORREF colorBtn = RGB(222, 222, 222);
	CBrush brush(colorBtn);	
	if (m_bShowCloseBtn)
	{		
		CPen penBtn(PS_SOLID, 1, colorBtn);
		CPen* pOldPen = dc->SelectObject(&penBtn);
		dc->FrameRect(m_rcBtnClose, &brush);
		dc->MoveTo(m_rcBtnClose.TopLeft());
		dc->LineTo(m_rcBtnClose.BottomRight());
		dc->MoveTo(m_rcBtnClose.left, m_rcBtnClose.bottom - 1);
		dc->LineTo(m_rcBtnClose.right - 1, m_rcBtnClose.top);
		dc->SelectObject(pOldPen);		
		penBtn.DeleteObject();
	}

	dc->SelectObject(pOldFont);

	// 清除按钮
	dc->Ellipse(m_rcBtnClear);
	brush.DeleteObject();

	if (m_bActive)
	{
		dc->Draw3dRect(rcClient, ACTIVE_COLOR_TOP_LEFT, ACTIVE_COLOR_RIGHT_BOTTOM);
	}
	else
	{
		dc->Draw3dRect(rcClient, INACTIVE_COLOR_TOP_LEFT, INACTIVE_COLOR_RIGHT_BOTTOM);
	}
}

void CVideoWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	const int nSpace = 1;
	const int nBtnWidth = 12;
	const int nHeaderHeight = 15;
	m_rcBtnClose.SetRect(cx - nBtnWidth, (nHeaderHeight - nBtnWidth)/2, cx, (nHeaderHeight + nBtnWidth)/2);
	m_rcHeader.SetRect(0, 0, cx, nHeaderHeight);
	m_rcContext.SetRect(0, nHeaderHeight, cx, cy);
	m_rcBtnClear.SetRect(m_rcBtnClose.left - 1 - nBtnWidth, m_rcBtnClose.top, m_rcBtnClose.left - 1, m_rcBtnClose.bottom);
	m_rcResize.SetRect(cx - nBtnWidth, cy - nBtnWidth, cx, cy);
}

BOOL CVideoWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	HCURSOR hCursor = NULL;
	if ((m_bShowCloseBtn && PtInRect(m_rcBtnClose, point)) || PtInRect(m_rcBtnClear, point))
	{
		hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(32649));
	}
	else if (m_bEnableResize && PtInRect(m_rcResize, point))
	{
		hCursor = ::LoadCursor(NULL, IDC_SIZENWSE);
	}
	else
	{
		hCursor = ::LoadCursor(NULL, IDC_ARROW);
	}
	::SetCursor(hCursor);
	return TRUE;
}

void CVideoWnd::SetActive( BOOL bActive /*= TRUE*/ )
{
	m_bActive = bActive;
	RedrawWindow();
}

int CVideoWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

void CVideoWnd::ShowCloseBtn( BOOL bVisible /*= TRUE*/ )
{
	m_bShowCloseBtn = bVisible;
	RedrawWindow();
}
