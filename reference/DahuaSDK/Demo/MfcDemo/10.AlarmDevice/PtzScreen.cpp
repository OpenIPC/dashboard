// PtzScreen.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "PtzScreen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPtzScreen

CPtzScreen::CPtzScreen()
{
	m_pMessageProc = NULL;
	m_dwMessageUser = 0;
	m_pGetParams = NULL;
	m_dwGetParams = 0;
	m_pSetParams = NULL;
	m_dwSetParams = 0;
	m_pRectEventFunc = NULL;
	m_dwRectEvent = 0;
}

CPtzScreen::~CPtzScreen()
{
	
}
void  CPtzScreen::SetActiveWnd(int nIndex,BOOL bRepaint)
{
	if(nIndex >= 0 && nIndex < DH_MAX_CHANNUM)
	{
		SetActivePage(&m_wndVideo[nIndex],bRepaint);
	}
}
CWnd* CPtzScreen::GetPage(int nIndex)
{
	if (nIndex >= 0 && nIndex < DH_MAX_CHANNUM)
	{
		return &m_wndVideo[nIndex];
	}
	
	return NULL;
}
int CPtzScreen::SetShowPlayWin(int nMain, int nSub)
{
	if (nSub < 0 || nSub > DH_MAX_CHANNUM)
	{
		nSub = 0;
	}

	int nNum = 16;
	int nBegin = 0;
	switch(nMain) 
	{
	case SPLIT1:
		nNum = 1;
		nBegin = nSub;
		break;
	case SPLIT4:
		nNum = 4;
		if (nSub >= 12)
		{
			nBegin = 12;
		}
		else if (nSub >= 8)
		{
			nBegin = 8;
		}
		else if (nSub >= 4)
		{
			nBegin = 4;
		}
		else
		{
			nBegin = 0;
		}

		break;
	case SPLIT9:
		nNum = 9;
		if (nSub >= 10)
		{
			nBegin = 7;
		}
		else
		{
			nBegin = 0;
		}
		break;
	case SPLIT16:
		nNum = 16;
		nBegin = 0;
		break;
	default:
		break;
	}

	m_bMultiScreen = TRUE;
	CList<CWnd*, CWnd*> templist;

	POSITION pos = m_PageList.GetHeadPosition();
	while(pos != NULL)
	{
		CWnd* pWnd = m_PageList.GetNext(pos);
		if (pWnd)
		{
			templist.AddTail(pWnd);
		}
	}

	m_PageList.RemoveAll();

	/*
	while(m_PageList.GetCount())
	{
		DelPage(GetTailPage());
	}
	*/
	
	for(int i=nBegin; i < (nBegin+nNum); i++)
	{
		AddPage(&m_wndVideo[i], TRUE);
	}

	SetActivePage(&m_wndVideo[nSub], TRUE);

	pos = templist.GetHeadPosition();
	while(pos != NULL)
	{
		CWnd* pWnd = templist.GetNext(pos);
		if (pWnd)
		{
			pWnd->ShowWindow(SW_HIDE);
		}
	}

	templist.RemoveAll();

	return m_PageList.GetCount();
}
void CPtzScreen::SetCallBack(OnMessageProcFunc cbMessageProc, LDWORD dwMessageUser,
							 OnGetParamsFunc cbGetParams, LDWORD dwGetParams, 
							 OnSetParamsFunc cbSetParams, LDWORD dwSetParmas,
							 OnRectEventFunc cbEventParams,LDWORD dwRectEventParams)
{
	m_pMessageProc = cbMessageProc;
	m_dwMessageUser = dwMessageUser;
	m_pGetParams = cbGetParams;
	m_dwGetParams = dwGetParams;
	m_pSetParams = cbSetParams;
	m_dwSetParams = dwSetParmas;
	m_pRectEventFunc = cbEventParams;
	m_dwRectEvent = dwRectEventParams;
}
BEGIN_MESSAGE_MAP(CPtzScreen, CWnd)
	//{{AFX_MSG_MAP(CPtzScreen)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ACTIVATE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
//	ON_WM_ACTIVATEAPP()
	ON_WM_ICONERASEBKGND()
	ON_WM_ASKCBFORMATNAME()
	ON_WM_ERASEBKGND()
	ON_WM_CANCELMODE()
	ON_WM_PAINT()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPtzScreen message handlers

void CPtzScreen::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CWnd::OnLButtonDown(nFlags, point);
}

void CPtzScreen::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CPtzScreen::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	
}

int CPtzScreen::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	for(int i = 0; i < DH_MAX_CHANNUM; i++)
	{
		m_wndVideo[i].Create(
			NULL, 
			NULL, 
			WS_VISIBLE | WS_CHILD,
			CRect(0, 0, 0, 0),
			this,
			1979,
			NULL);
		
		m_wndVideo[i].SetWinID(i);
		
		AddPage(&m_wndVideo[i]);
	}
	
	SetActivePage(&m_wndVideo[0], TRUE);
	SetDrawActivePage(TRUE, RGB(248,5,182), RGB(248,5,182));
	
	return 0;	
	return 0;
}

void CPtzScreen::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	for(int i=0; i < DH_MAX_CHANNUM; i++)
	{
		m_wndVideo[0].DestroyWindow();
	}
}

void CPtzScreen::OnActivateApp(BOOL bActive, HTASK hTask) 
{
//	CWnd::OnActivateApp(bActive, hTask);
	
	// TODO: Add your message handler code here
	
}

void CPtzScreen::OnIconEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnIconEraseBkgnd(pDC);
}

void CPtzScreen::OnAskCbFormatName(UINT nMaxCount, LPTSTR lpszString) 
{
	// TODO: Add your message handler code here and/or call default

	CWnd::OnAskCbFormatName(nMaxCount, lpszString);
}

BOOL CPtzScreen::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rt;
	GetClientRect(&rt);
	CBrush br;
	br.CreateSolidBrush(RGB(192,192,192));
	pDC->FillRect(&rt,&br);
	
	return TRUE;
//	return CWnd::OnEraseBkgnd(pDC);
}

void CPtzScreen::OnCancelMode() 
{
	CWnd::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void CPtzScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateWnd();
	// Do not call CWnd::OnPaint() for painting messages
}

void CPtzScreen::OnCaptureChanged(CWnd *pWnd) 
{
	// TODO: Add your message handler code here
	
	CWnd::OnCaptureChanged(pWnd);
}




















































































































































































































































































































































