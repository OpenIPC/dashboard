// PlayWnd.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "PlayWnd.h"
#include "netsdkdemodlg.h"
#include "ScreenPannel.h"
#include "NetSDKDemo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPlayWnd dialog


CPlayWnd::CPlayWnd()
	:m_nWndID(0)
{
	InitializeCriticalSection(&m_cs);
	memset(&m_splitInfo, 0, sizeof(m_splitInfo));
	m_nPreSplit = 0;
	m_bFullScreen = FALSE;
	
	//{{AFX_DATA_INIT(CPlayWnd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPlayWnd, CWnd)
	//{{AFX_MSG_MAP(CPlayWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
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
	CScreenPannel *pContainer = (CScreenPannel *)GetParent();
	if(pContainer)
	{
		switch(message)
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			pContainer->SetActivePage(this);
			{
				CNetSDKDemoDlg * pMainWnd = (CNetSDKDemoDlg *)(AfxGetMainWnd());
				if (pMainWnd)
				{
					pMainWnd->SetCurWindId(m_nWndID);
					pMainWnd->UpdateCurScreenInfo();
				}
			}
			break;
		case WM_LBUTTONDBLCLK:
			{
				CNetSDKDemoDlg * pMainWnd = (CNetSDKDemoDlg *)(AfxGetMainWnd());
				BOOL bMulti = pContainer->GetMultiScreen();
				pContainer->SetMultiScreen(!bMulti);

				if (TRUE == bMulti)
				{
					m_nPreSplit = pMainWnd->GetSplit();
					pMainWnd->SetSplit(SPLIT1);
				}
				else
				{
					pMainWnd->SetSplit(m_nPreSplit);
				}
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
	CScreenPannel *pContainer = (CScreenPannel *)GetParent();
	pContainer->SetActivePage(this);

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING | pContainer->GetFullScreen()    ? MF_CHECKED : MF_UNCHECKED, VIDEO_MENU_FULLSCREEN, ConvertString(NAME_MENU_FULLSCREEN));
	menu.AppendMenu(MF_STRING | pContainer->GetMultiScreen()    ? MF_CHECKED : MF_UNCHECKED, 	VIDEO_MENU_MULTISCREEN, ConvertString(NAME_MENU_MULTISCREEN));
	menu.AppendMenu(MF_STRING | pContainer->GetAutoAdjustPos() ? MF_CHECKED : MF_UNCHECKED, VIDEO_MENU_AUTOADJUST, ConvertString(NAME_MENU_AUTOADJUST));

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
	CScreenPannel *pContainer = (CScreenPannel *)GetParent();
	switch(nID)
	{
	case VIDEO_MENU_FULLSCREEN:
		pContainer->SetFullScreen(!pContainer->GetFullScreen());
		break;
	case VIDEO_MENU_MULTISCREEN:
		{
			CNetSDKDemoDlg * pMainWnd = (CNetSDKDemoDlg *)(AfxGetMainWnd());
			BOOL bMulti = pContainer->GetMultiScreen();
			//	int nCnt = pContainer->GetCount();
			pContainer->SetMultiScreen(!bMulti);
			
			if (TRUE == bMulti)
			{
				m_nPreSplit = pMainWnd->GetSplit();
				pMainWnd->SetSplit(SPLIT1);
			}
			else
			{
				pMainWnd->SetSplit(m_nPreSplit);
			}
			break;
		}
		
	case VIDEO_MENU_AUTOADJUST:
		pContainer->SetAutoAdjustPos(!pContainer->GetAutoAdjustPos());
		break;
	case VIDEO_MENU_PRINTSCREEN:
		break;
	case VIDEO_MENU_RECORDVIDEO:
		break;
	case VIDEO_MENU_EXITDECODE:
		break;
	default:
		break;
	}
}

BOOL CPlayWnd::GetSplitInfo(SplitInfoNode* info)
{
	if (!info)
	{
		return FALSE;
	}

	EnterCriticalSection(&m_cs);
	memcpy(info, &m_splitInfo, sizeof(SplitInfoNode));
	LeaveCriticalSection(&m_cs);

	return TRUE;
}

BOOL CPlayWnd::SetSplitInfo(SplitInfoNode* info)
{
	if (!info)
	{
		return FALSE;
	}

	EnterCriticalSection(&m_cs);
	memcpy(&m_splitInfo, info, sizeof(SplitInfoNode));
	LeaveCriticalSection(&m_cs);

	return TRUE;
}

void CPlayWnd::SetSplitType(SplitType type)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.Type = type;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitHandle(LLONG dwHandle)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.iHandle = dwHandle;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitParam(void *Param)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.Param = Param;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitCBFlag_Real(int nFlag)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.isSaveData = nFlag;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitCBFile_Raw(FILE *file)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.SavecbFileRaw = file;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitCBFile_Std(FILE *file)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.SavecbFileStd = file;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitCBFile_Pcm(FILE *file)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.SavecbFilePcm = file;
	LeaveCriticalSection(&m_cs);
}

void CPlayWnd::SetSplitCBFile_Yuv(FILE *file)
{
	EnterCriticalSection(&m_cs);
	m_splitInfo.SavecbFileYUV = file;
	LeaveCriticalSection(&m_cs);
}

LRESULT CPlayWnd::OnRepaintWnd(WPARAM wParam, LPARAM lParam)
{
	Invalidate();
	
	return 0;
}

BOOL CPlayWnd::DestroyWindow() 
{
	DeleteCriticalSection(&m_cs);
	return CWnd::DestroyWindow();
}
