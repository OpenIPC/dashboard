// ExButton.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ExButton.h"
#include "netsdkdemodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExButton

CExButton::CExButton()
{
	m_dwPTZCommand = -1;
}

CExButton::~CExButton()
{
}


BEGIN_MESSAGE_MAP(CExButton, CButton)
	//{{AFX_MSG_MAP(CExButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExButton message handlers
void CExButton::SetButtonCommand(DWORD dwPTZCommand)
{
	m_dwPTZCommand = dwPTZCommand;
}

void CExButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CButton::OnLButtonDown(nFlags, point);

	if((int)m_dwPTZCommand < 0)
	{
		return;
	}
	((CPTZPannel*)GetParent())->SendPtzControl(m_dwPTZCommand,FALSE);
}

void CExButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CButton::OnLButtonUp(nFlags, point);

	if((int)m_dwPTZCommand < 0)
	{
		return;		
	}
	((CPTZPannel*)GetParent())->SendPtzControl(m_dwPTZCommand,TRUE);
}
