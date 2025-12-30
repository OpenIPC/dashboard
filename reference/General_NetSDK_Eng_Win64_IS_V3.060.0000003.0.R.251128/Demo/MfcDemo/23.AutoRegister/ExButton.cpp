// ExButton.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveLoginDemo.h"
#include "PTZControlDlg.h"
#include "ExButton.h"

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
	m_bIsMouseDown = FALSE;
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

void CExButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	//Check mous has been clicked or not 
	if(!m_bIsMouseDown)
	{
		m_bIsMouseDown = TRUE;
		if((int)m_dwPTZCommand < 0)
		{
			return;
		}
		((CPTZControlDlg*)GetParent())->PtzControl(m_dwPTZCommand,FALSE);
	}
	
	CButton::OnLButtonDown(nFlags, point);
}

void CExButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_bIsMouseDown)
	{
		m_bIsMouseDown = FALSE;
		if((int)m_dwPTZCommand < 0)
		{
			return;		
		}
		((CPTZControlDlg*)GetParent())->PtzControl(m_dwPTZCommand,TRUE);
	}
	
	CButton::OnLButtonUp(nFlags, point);
}

void CExButton::SetButtonCommand(DWORD dwPTZCommand)
{
	m_dwPTZCommand = dwPTZCommand;
}




















































































































































































































































































































































