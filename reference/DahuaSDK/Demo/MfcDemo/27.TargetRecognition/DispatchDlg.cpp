// DispatchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FaceRecognition.h"
#include "DispatchDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDispatchDlg dialog


CDispatchDlg::CDispatchDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDispatchDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDispatchDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDispatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDispatchDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDispatchDlg, CDialog)
	//{{AFX_MSG_MAP(CDispatchDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDispatchDlg message handlers
