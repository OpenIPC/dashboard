#include "stdafx.h"
#include "RealPlayAndPTZControl.h"
#include "PlayWnd.h"
#include "PtzScreen.h"
#include "RealPlayAndPTZControlDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayWnd dialog


CPlayWnd::CPlayWnd():m_nWndID(0), m_FlagRect(FALSE),bIsPlaying(false)
{	
	//{{AFX_DATA_INIT(CPlayWnd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPlayWnd, CWnd)
	//{{AFX_MSG_MAP(CPlayWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ACTIVATE()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(VIDEO_MENU_BASE, VIDEO_MENU_END, OnVideoMenu)
	ON_MESSAGE(VIDEO_REPAINT, OnRepaintWnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayWnd message handlers
BOOL CPlayWnd::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rt;
	GetClientRect(&rt);
	CBrush br;
	br.CreateSolidBrush(VIDEO_BACK_COLOR);
	pDC->FillRect(&rt,&br);

	return TRUE;	
}

LRESULT CPlayWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	CPtzScreen *pContainer = (CPtzScreen *)GetParent();
// 	CRealPlayAndPTZControlDlg *pMainWnd = (CRealPlayAndPTZControlDlg *)(AfxGetApp()->GetMainWnd());
// 	CDialogTransmit * pTransWnd = (CDialogTransmit *)pMainWnd->m_pTabTransmit;

	if(pContainer)
	{
		switch(message)
		{	
		case WM_LBUTTONUP:
			{				
				pointEnd.x = LOWORD(lParam);
				pointEnd.y = HIWORD(lParam);
				if(m_FlagRect == TRUE)
				{
					m_FlagRect =FALSE;
					KillTimer(2);
					RECT rt;
					GetClientRect(&rt);
					pContainer->m_pRectEventFunc(rt,pointStart,pointEnd,pContainer->m_dwRectEvent);
				}
			}
			break;
		case WM_MOUSEMOVE:
			{				
				RECT rt;
				GetClientRect(&rt);
				long x = LOWORD(lParam);
				long y = HIWORD(lParam);
				x = x>rt.right?rt.right:x;
				x = x<rt.left?rt.left:x;
				y = y>rt.bottom?rt.bottom:y;
				y = y<rt.top?rt.top:y;
				pointEnd.x = x;
				pointEnd.y = y;				
			}
			break;
		case WM_LBUTTONDOWN:
			{	
                pContainer->SetActivePage(this);        // 设备窗口选择框
                if (pContainer->m_pMessageProc)         // 选择对应播放画面
                {
                    pContainer->m_pMessageProc(m_nWndID, WM_RBUTTONDOWN, pContainer->m_dwMessageUser);
                }	
                if (bIsPlaying == false)
                {
                    break;
                }
                pointStart.x = LOWORD(lParam);
                pointStart.y = HIWORD(lParam);
                m_FlagRect =TRUE;
                SetTimer(2,16,NULL);
                break;
            }
		case WM_LBUTTONDBLCLK:
			{
				BOOL bMulti = pContainer->GetMultiScreen();
				pContainer->SetMultiScreen(!bMulti);
			}
			break;
		default: 
			break;
		}
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}

void CPlayWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	CPtzScreen *pContainer = (CPtzScreen *)GetParent();
	pContainer->SetActivePage(this);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING | pContainer->GetFullScreen()    ? MF_CHECKED : MF_UNCHECKED, VIDEO_MENU_FULLSCREEN, ConvertString(NAME_MENU_FULLSCREEN));
	menu.AppendMenu(MF_STRING | pContainer->GetMultiScreen()    ? MF_CHECKED : MF_UNCHECKED, 	VIDEO_MENU_MULTISCREEN, ConvertString(NAME_MENU_MULTISCREEN));
	menu.AppendMenu(MF_STRING | pContainer->GetAutoAdjustPos() ? MF_CHECKED : MF_UNCHECKED, VIDEO_MENU_AUTOADJUST, ConvertString(NAME_MENU_AUTOADJUST));
	menu.AppendMenu(MF_STRING | pContainer->m_pGetParams(m_nWndID, 0, pContainer->m_dwGetParams) ? MF_CHECKED : MF_UNCHECKED, VIDEO_MENU_EXITDECODE, ConvertString(NAME_MENU_EXITDECODE));
	menu.AppendMenu(MF_STRING | pContainer->m_pGetParams(m_nWndID, 1, pContainer->m_dwGetParams) ? MF_CHECKED : MF_UNCHECKED, VIDEO_MENU_EXITCYCLE, ConvertString(NAME_MENU_EXITCYCLE));

	TrackPopupMenu( 
		menu.m_hMenu, 
		TPM_LEFTALIGN, 
		point.x, 
		point.y,
		0,
		m_hWnd,
		NULL);
}

void CPlayWnd::OnVideoMenu(UINT nID)
{
	CPtzScreen *pContainer = (CPtzScreen *)GetParent();

	switch(nID)
	{
	case VIDEO_MENU_FULLSCREEN:
		pContainer->SetFullScreen(!pContainer->GetFullScreen());
		break;
	case VIDEO_MENU_MULTISCREEN:
		pContainer->SetMultiScreen(!pContainer->GetMultiScreen());
		break;
	case VIDEO_MENU_AUTOADJUST:
		pContainer->SetAutoAdjustPos(!pContainer->GetAutoAdjustPos());
		break;
	case VIDEO_MENU_PRINTSCREEN:
		break;
	case VIDEO_MENU_RECORDVIDEO:
		break;
	case VIDEO_MENU_EXITDECODE:
		pContainer->m_pSetParams(m_nWndID, 0, pContainer->m_dwSetParams);
		break;
	case VIDEO_MENU_EXITCYCLE:
		pContainer->m_pSetParams(m_nWndID, 1, pContainer->m_dwSetParams);
		break;
	default:
		break;
	}
}

LRESULT CPlayWnd::OnRepaintWnd(WPARAM wParam, LPARAM lParam)
{
	Invalidate();
	
	return 0;
}

BOOL CPlayWnd::DestroyWindow() 
{
	return CWnd::DestroyWindow();
}

void CPlayWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnLButtonDown(nFlags, point);
}

void CPlayWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnMButtonUp(nFlags, point);
}

void CPlayWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnMouseMove(nFlags, point);
}

void CPlayWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}

#if _MSC_VER >= 1300
void CPlayWnd::OnTimer(UINT_PTR nIDEvent) 
#else
void CPlayWnd::OnTimer(UINT nIDEvent) 
#endif
{
    if (bIsPlaying == false)
    {
        return;
    }
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent ==2)
	{
		if(m_FlagRect == TRUE)
		{
			CDC *pdc = GetDC();
			RECT  rt;
			rt.left =  pointStart.x;
			rt.top =   pointStart.y;		
			rt.right = 	pointEnd.x;
			rt.bottom = pointEnd.y;
			pdc->Draw3dRect(&rt,RGB(255,0,0),RGB(255,0,0));
			ReleaseDC(pdc);
		}
	}
	CWnd::OnTimer(nIDEvent);
}

#if _MSC_VER >= 1300
void CPlayWnd::OnActivateApp(BOOL bActive, DWORD hTask) 
#else
void CPlayWnd::OnActivateApp(BOOL bActive, HTASK hTask) 
#endif
{
	CWnd::OnActivateApp(bActive, hTask);
	
	// TODO: Add your message handler code here
	
}




















































































































































































































































































































































