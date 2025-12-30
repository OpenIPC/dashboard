// hangeIP.cpp : implementation file
//

#include "stdafx.h"
#include "SearchDevice.h"
#include "DevInitDlg.h"
#include "ModifyIPDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ChangeIP dialog


ModifyIPDlg::ModifyIPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ModifyIPDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ChangeIP)
	m_IpVersion = 0;
	m_IP = _T("");
	m_strPassWord = _T("");
	m_strUserName = _T("");
	m_strSubnetMask = _T("");
	m_strGateWay = _T("");
	//}}AFX_DATA_INIT
}


void ModifyIPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ChangeIP)
	DDX_Text(pDX, IDC_EDIT_IPVERSION, m_IpVersion);
	DDX_Text(pDX, IDC_EDIT_IP, m_IP);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassWord);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_EDIT_SUBNETMASK, m_strSubnetMask);
	DDX_Text(pDX, IDC_EDIT_GATEWAY, m_strGateWay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ModifyIPDlg, CDialog)
	//{{AFX_MSG_MAP(ChangeIP)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &ModifyIPDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ChangeIP message handlers

void ModifyIPDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);

	CDevInitDlg* pInitDlg = (CDevInitDlg *)this->GetParent();
	pInitDlg->ChangeDevNetIp(m_IP,m_IpVersion,m_strUserName,m_strPassWord,m_strSubnetMask,m_strGateWay);

    CDialog::OnOK();
}

BOOL ModifyIPDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ModifyIPDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}
