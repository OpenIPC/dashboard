// ConfigAlarmDisk.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigAlarmDisk.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmDisk dialog


CConfigAlarmDisk::CConfigAlarmDisk(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigAlarmDisk::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigAlarmDisk)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigAlarmDisk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigAlarmDisk)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigAlarmDisk, CDialog)
	//{{AFX_MSG_MAP(CConfigAlarmDisk)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmDisk message handlers
