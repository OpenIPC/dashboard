// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog


CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
	, m_lLoginID(0)
{
	//{{AFX_DATA_INIT(CLoginDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	memset(&m_stuDevInfo, 0, sizeof(m_stuDevInfo));
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Control(pDX, IDC_COMBO_ADDRESS, m_cbAddr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers

BOOL CLoginDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this, DLG_ADD_DEVICE);
	
	SetDlgItemText(IDC_COMBO_ADDRESS, "172.5.2.103");
	SetDlgItemInt(IDC_EDIT_PORT, 37777);
	SetDlgItemText(IDC_EDIT_USER, "admin");
	SetDlgItemText(IDC_EDIT_PWD, "admin");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoginDlg::OnOK() 
{
	CString strAddr, strUser, strPwd;
	UINT nPort = GetDlgItemInt(IDC_EDIT_PORT);
	m_cbAddr.GetWindowText(strAddr);
	GetDlgItemText(IDC_EDIT_USER, strUser);
	GetDlgItemText(IDC_EDIT_PWD, strPwd);
	
	if (strAddr.IsEmpty())
	{
		MessageBox(ConvertString("Invalid Address.", DLG_ADD_DEVICE), ConvertString("Prompt"));
		return;
	}
	if (nPort <= 0)
	{
		MessageBox(ConvertString("Invalid Port.", DLG_ADD_DEVICE), ConvertString("Prompt"));
		return;
	}
	if (strAddr.IsEmpty())
	{
		MessageBox(ConvertString("Invalid User Name.", DLG_ADD_DEVICE), ConvertString("Prompt"));
		return;
	}

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, (LPTSTR)(LPCTSTR)strAddr, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, (LPTSTR)(LPCTSTR)strPwd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, (LPTSTR)(LPCTSTR)strUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	CString strTemp;
		
	if (lRet == 0)
	{
		CString str;
		str.Format("%s %d.", ConvertString("Login error. Code:", DLG_ADD_DEVICE), stOutparam.nError);
		MessageBox(str, ConvertString("Prompt"));
		return;
	}
	
	m_lLoginID = lRet;
	m_strAddr = strAddr;
	m_stuDevInfo = stOutparam.stuDeviceInfo;
	CDialog::OnOK();
}
