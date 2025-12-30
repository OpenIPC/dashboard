// AlarmConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "AlarmConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlarmConfigDlg dialog


CAlarmConfigDlg::CAlarmConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAlarmConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAlarmConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlarmConfigDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlarmConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CAlarmConfigDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlarmConfigDlg message handlers
