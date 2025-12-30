// FlashLabel.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "FlashLabel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlashLabel

CFlashLabel::CFlashLabel()
: m_nTimer(0)
, m_nColorIndex(0)
, m_dwDrawTick(0)
, m_nFlashTime(0)
{
    m_arColor[0] = RGB(0, 255, 0);
    m_arColor[1] = RGB(255, 0, 0);
}

CFlashLabel::~CFlashLabel()
{
}


BEGIN_MESSAGE_MAP(CFlashLabel, CStatic)
	//{{AFX_MSG_MAP(CFlashLabel)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFlashLabel message handlers

void CFlashLabel::PreSubclassWindow() 
{
    LONG lStyle = GetWindowLong(m_hWnd, GWL_STYLE);
    lStyle &= ~WS_VISIBLE;
    SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
    
    CStatic::PreSubclassWindow();
}

void CFlashLabel::OnDestroy() 
{
    if (m_nTimer)
    {
        KillTimer(m_nTimer);
    }
	CStatic::OnDestroy();
}

void CFlashLabel::OnPaint() 
{
    CPaintDC dc(this); 

    CRect rc, rcTarget;
    GetClientRect(rc);

    int nRadius = min(rc.Width(), rc.Height()) / 2;
    int nCenterX = rc.left + rc.Width() / 2;
    int nCenterY = rc.top + rc.Height() / 2;
    rcTarget.left = nCenterX - nRadius;
    rcTarget.right = nCenterX + nRadius;
    rcTarget.top = nCenterY - nRadius;
    rcTarget.bottom = nCenterY + nRadius;
    
    //Ìî³ä±³¾°É«
    dc.SetBkMode(TRANSPARENT);
    CBrush backBrush(m_arColor[m_nColorIndex]);
    CBrush* pOldBrush = dc.SelectObject(&backBrush);
    dc.Ellipse(&rcTarget);
    dc.SelectObject(pOldBrush);    
}

#ifdef WIN64
void CFlashLabel::OnTimer(UINT_PTR nIDEvent)
#elif defined(_WIN32)
void CFlashLabel::OnTimer(UINT nIDEvent) 
#endif
{
	if (nIDEvent == m_nTimer)
    {
        DWORD dwTickNow = GetTickCount();
        if (dwTickNow - m_dwDrawTick <= m_nFlashTime)
        {
            ++m_nColorIndex;
            if (m_nColorIndex >= COLOR_NUM)
            {
                m_nColorIndex = 0;
            }
            Invalidate(FALSE);
        }
        else
        {
            StopFlash();
        }
    }
	
	CStatic::OnTimer(nIDEvent);
}

void CFlashLabel::StartFlash( UINT nTime )
{
    StopFlash();

    m_nFlashTime = nTime;
    m_dwDrawTick = GetTickCount();
    m_nTimer = SetTimer(TIME_ID_FLASH, 500, NULL);
    ShowWindow(SW_SHOWNORMAL);
}

void CFlashLabel::StopFlash()
{
    if (m_nTimer)
    {
        KillTimer(m_nTimer);
        m_nTimer = 0;
    }
    ShowWindow(SW_HIDE);
}