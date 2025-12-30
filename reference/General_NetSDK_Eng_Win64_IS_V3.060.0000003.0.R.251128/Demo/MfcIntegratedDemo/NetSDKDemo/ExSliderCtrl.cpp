// ExSliderCtrl.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ExSliderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExSliderCtrl

CExSliderCtrl::CExSliderCtrl()
{
	m_bLButtonDown = FALSE;
}

CExSliderCtrl::~CExSliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CExSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CExSliderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExSliderCtrl message handlers

void CExSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_bLButtonDown = TRUE;
	
	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CExSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_bLButtonDown = FALSE;

	CSliderCtrl::OnLButtonUp(nFlags, point);
}
