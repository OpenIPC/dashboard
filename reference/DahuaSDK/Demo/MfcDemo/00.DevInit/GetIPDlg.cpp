// GetIP.cpp : implementation file
//

#include "stdafx.h"
#include "SearchDevice.h"
#include "DevInitDlg.h"
#include "GetIPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GetIP dialog


GetIPDlg::GetIPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(GetIPDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(GetIP)
	//}}AFX_DATA_INIT
}


void GetIPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GetIP)
	DDX_Control(pDX, IDC_IPADDRESS_END, m_ctlEndIP);
	DDX_Control(pDX, IDC_IPADDRESS_START, m_ctlStartIP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GetIPDlg, CDialog)
	//{{AFX_MSG_MAP(GetIP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GetIP message handlers

void GetIPDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);
	DWORD dwStartIP,dwEndIP;
	m_ctlStartIP.GetAddress(dwStartIP);
	m_ctlEndIP.GetAddress(dwEndIP);
	(this->GetParent())->PostMessage(WM_SEARCHDEVICEBYIPS, WPARAM(dwStartIP), LPARAM(dwEndIP));
	CDialog::OnOK();
}

BOOL GetIPDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
