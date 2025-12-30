// DiskControl.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "DiskControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiskControl dialog


CDiskControl::CDiskControl(CWnd* pParent /*=NULL*/)
	: CDialog(CDiskControl::IDD, pParent)
{
	memset(&m_devWorkState, 0, sizeof(NET_DEV_WORKSTATE));
	//{{AFX_DATA_INIT(CDiskControl)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_dev = NULL;
}


void CDiskControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiskControl)
	DDX_Control(pDX, IDC_DISKSEL_COMBO, m_disksel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiskControl, CDialog)
	//{{AFX_MSG_MAP(CDiskControl)
	ON_CBN_SELCHANGE(IDC_DISKSEL_COMBO, OnSelchangeDiskselCombo)	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiskControl message handlers

BOOL CDiskControl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	BOOL bRet = CLIENT_GetDEVWorkState(m_dev->LoginID, &m_devWorkState);
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDiskControl::OnSelchangeDiskselCombo() 
{
}

void CDiskControl::SetDeviceId(DeviceNode *dev)
{
	m_dev = dev;
}