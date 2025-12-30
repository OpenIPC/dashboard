// ConfigAlarmMotionAreaBlock.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigAlarmMotionAreaBlock.h"
#include "ConfigAlarmMotionArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotionAreaBlock dialog


CConfigAlarmMotionAreaBlock::CConfigAlarmMotionAreaBlock()
{
	//{{AFX_DATA_INIT(CConfigAlarmMotionAreaBlock)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_whiteBrush.CreateSolidBrush(RGB(255,255,255));
	m_blueBrush.CreateSolidBrush(RGB(125,125,200));
	m_brush = &m_whiteBrush;
	m_bSelected = FALSE;
	m_byX = 0;
	m_byY = 0;
	ZeroMemory(m_rect, sizeof(CRect));
	m_thisDC = 0;
}


BEGIN_MESSAGE_MAP(CConfigAlarmMotionAreaBlock, CWnd)
		//{{AFX_MSG_MAP(CPlayWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(MSG_SELETED, OnSeleted)
	ON_MESSAGE(MSG_SHOW, OnShow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotionAreaBlock message handlers

BOOL CConfigAlarmMotionAreaBlock::DestroyWindow()
{
	return CWnd::DestroyWindow();
}


LRESULT CConfigAlarmMotionAreaBlock::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	// TODO: Add your specialized code here and/or call the base class
	CConfigAlarmMotionArea *pMap = (CConfigAlarmMotionArea *)GetParent();
	if(pMap)
	{
		switch(message)
		{
		case WM_LBUTTONDOWN:
			pMap->RecordPoint(m_byX, m_byY);
			break;
		case WM_LBUTTONUP:
			pMap->ReleasePoint();
			break;
		case WM_MOUSEMOVE:
			pMap->MovePoint(wParam, m_byX, m_byY);
		default:
			break;
		}
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}

BOOL CConfigAlarmMotionAreaBlock::OnEraseBkgnd(CDC* pDC)
{
	CRect rt;
	GetClientRect(&rt);
	pDC->FillRect(&rt, m_brush);
	return TRUE;
}

LRESULT CConfigAlarmMotionAreaBlock::OnSeleted(WPARAM wParam, LPARAM lParam)
{
	m_bSelected = !m_bSelected;
	if (m_bSelected)
	{
		m_brush = &m_blueBrush;
	}
	else
	{
		m_brush = &m_whiteBrush;
	}
	
	if (m_thisDC)
	{
		m_thisDC->FillRect(&m_rect, m_brush);
	}

	return 0;
}

void CConfigAlarmMotionAreaBlock::SetPos(BYTE x, BYTE y)
{
	m_byX = x;
	m_byY = y;
	GetClientRect(&m_rect);
	m_thisDC = GetDC();
}

LRESULT CConfigAlarmMotionAreaBlock::OnShow(WPARAM wParam, LPARAM lParam)
{
	if (m_bSelected)
	{
		m_brush = &m_blueBrush;
	}
	else
	{
		m_brush = &m_whiteBrush;
	}

	if (m_thisDC)
	{
		m_thisDC->FillRect(&m_rect, m_brush);
	}

	return 0;
}
