// DlgChn.cpp : implementation file
//

#include "stdafx.h"
#include "Alarm.h"
#include "DlgChn.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgChn dialog


CDlgChn::CDlgChn(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgChn::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgChn)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgChn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgChn)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgChn, CDialog)
	//{{AFX_MSG_MAP(CDlgChn)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgChn message handlers
BOOL CDlgChn::SetChnState(BYTE* bState, int nLen)
{
	if(nLen > 32 || bState == NULL)
	{
		return FALSE;
	}
	
	int i = 0;
	for(i = 0; i <= 15; i++)
	{
		GetDlgItem(IDC_ARRAY_00 + i)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ARRAY_90 + i)->ShowWindow(SW_SHOW);
	}
	
	UINT uIDBase = IDC_ARRAY_00;
	BOOL bCheck = FALSE;
	if (nLen <= 16)
	{
		for(UINT uID = uIDBase; uID < uIDBase+nLen; uID++)
		{
	    	bCheck = bState[uID-uIDBase] > 0 ? TRUE : FALSE;
	    	((CButton *)GetDlgItem(uID))->SetCheck(bCheck);
		}
	}
	else
	{
		for(UINT uID = uIDBase; uID <= uIDBase+15; uID++)
		{
			bCheck = bState[uID-uIDBase] > 0 ? TRUE : FALSE;
			((CButton *)GetDlgItem(uID))->SetCheck(bCheck);
		}

		UINT uIDBase1 = IDC_ARRAY_90;
		for (UINT uID1 = uIDBase1; uID1 <= uIDBase1+15; uID1++)
		{
			bCheck = bState[uID1-uIDBase1+16] > 0 ? TRUE : FALSE;
			((CButton *)GetDlgItem(uID1))->SetCheck(bCheck);
		}
	}
	
	UpdateData(FALSE);
	return TRUE;
}

BOOL CDlgChn::GetChnState(BYTE* bState, int nLen)
{
	if(nLen > 16 || bState == NULL)
	{
		return FALSE;
	}
	
	UINT uIDBase = IDC_ARRAY_00;
	BOOL bCheck = FALSE;
	for(UINT uID = uIDBase; uID < uIDBase+nLen; uID++)
	{
		bState[uID-uIDBase] = ((CButton *)GetDlgItem(uID))->GetCheck();
	}
	
	return TRUE;
}

BOOL CDlgChn::Attach(UINT uID, CDialog* pAttachDlg)
{
	//get attached window rect
	RECT rect = {0};
	pAttachDlg->GetDlgItem(uID)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	//get self window rect
	RECT rectSelf = {0};
	this->GetClientRect(&rectSelf);
	ScreenToClient(&rectSelf);
	//calculte the rect
	rect.right = rect.left+rectSelf.right-rectSelf.left;
	rect.bottom = rect.top+rectSelf.bottom-rectSelf.top;
	
	//hide the attach window
	pAttachDlg->GetDlgItem(uID)->ShowWindow(SW_HIDE);
	
	this->MoveWindow(&rect, TRUE);
	this->ShowWindow(SW_SHOW);
	
	return TRUE;
}