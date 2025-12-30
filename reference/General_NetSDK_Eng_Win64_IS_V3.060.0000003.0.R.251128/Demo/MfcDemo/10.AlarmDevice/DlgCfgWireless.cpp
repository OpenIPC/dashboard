// DlgCfgWireless.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgWireless.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgWireless dialog


CDlgCfgWireless::CDlgCfgWireless(CWnd* pParent /*=NULL*/, LLONG lLoginId /*= 0*/)
	: CDialog(CDlgCfgWireless::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgWireless)
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgWireless::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgWireless)
	DDX_Control(pDX, IDC_WIRELESS_CHK_ENABLE, m_chkEnable);
	DDX_Control(pDX, IDC_WIRELESS_CMB_DAY3GFLUXTATIC, m_cmbFluxTactic);
	DDX_Control(pDX, IDC_WIRELESS_CMB_DAY3GFLUXACTION, m_cmbFluxAction);
	DDX_Control(pDX, IDC_WIRELESS_CMB_APN, m_cmbAPN);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgWireless, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgWireless)
	ON_BN_CLICKED(IDC_WIRELESS_BTN_SET, OnSet)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgWireless private method

void CDlgCfgWireless::InitDlg()
{
	int i = 0;

	// Enable
	m_chkEnable.SetCheck(BST_UNCHECKED);

	// KeepAlive
	SetDlgItemInt(IDC_WIRELESS_EDT_KEEPALIVE, 0, FALSE);

	// APN
	m_cmbAPN.ResetContent();
	for (i = 0; i < sizeof(stuDemoAPN) /  sizeof(stuDemoAPN[0]); i++)
	{
		m_cmbAPN.InsertString(-1, stuDemoAPN[i].pszName);
	}
	m_cmbAPN.SetCurSel(-1);

	// UserName
	SetDlgItemText(IDC_WIRELESS_EDT_USERNAME, "");

	// Password
	SetDlgItemText(IDC_WIRELESS_EDT_PASSWORD, "");

	// Day3GFluxTactic
	m_cmbFluxTactic.ResetContent();
	for (i = 0; i < sizeof(stuDemoFluxTactic) / sizeof(stuDemoFluxTactic[0]); i++)
	{
		m_cmbFluxTactic.InsertString(-1, ConvertString(stuDemoFluxTactic[i].pszName, DLG_CFG_WIRELESS));
	}
	m_cmbFluxTactic.SetCurSel(-1);

	// Day3GFluxUp
	SetDlgItemInt(IDC_WIRELESS_EDT_DAY3GFLUXUP, 0, FALSE);

	// Day3GFluxUse
	SetDlgItemInt(IDC_WIRELESS_EDT_DAY3GFLUXUSE, 0, FALSE);

	// Day3GFluxAction
	m_cmbFluxAction.ResetContent();
	for (i = 0; i < sizeof(stuDemoFluxAction) / sizeof(stuDemoFluxAction[0]); i++)
	{
		m_cmbFluxAction.InsertString(-1, ConvertString(stuDemoFluxAction[i].pszName, DLG_CFG_WIRELESS));
	}
	m_cmbFluxAction.SetCurSel(-1);
}

void CDlgCfgWireless::StuToDlg()
{	
	// Enable
	m_chkEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);

	// KeepAlive
	SetDlgItemInt(IDC_WIRELESS_EDT_KEEPALIVE, m_stuInfo.nKeepAlive, FALSE);

	// APN
	m_cmbAPN.SetCurSel((int)m_stuInfo.emAPN);

	// UserName
	SetDlgItemText(IDC_WIRELESS_EDT_USERNAME, m_stuInfo.szUseName);

	// Password
	SetDlgItemText(IDC_WIRELESS_EDT_PASSWORD, m_stuInfo.szPassword);

	// Day3GFluxTactic
	m_cmbFluxTactic.SetCurSel((int)m_stuInfo.emDay3GFluxTactic);
	
	// Day3GFluxUp
	SetDlgItemInt(IDC_WIRELESS_EDT_DAY3GFLUXUP, m_stuInfo.dwDay3GFluxUp, FALSE);
	
	// Day3GFluxUse
	SetDlgItemInt(IDC_WIRELESS_EDT_DAY3GFLUXUSE, m_stuInfo.dwDay3GFluxUse, FALSE);
	
	// Day3GFluxAction
	m_cmbFluxAction.SetCurSel((int)m_stuInfo.emDay3GFluxAction);
}

void CDlgCfgWireless::DlgToStu()
{	
	// Enable
	m_stuInfo.bEnable = m_chkEnable.GetCheck();
	
	// KeepAlive
	m_stuInfo.nKeepAlive = GetDlgItemInt(IDC_WIRELESS_EDT_KEEPALIVE, NULL, FALSE);
	
	// APN
	m_stuInfo.emAPN = (EM_CFG_APN)m_cmbAPN.GetCurSel();
	
	// UserName
	GetDlgItemText(IDC_WIRELESS_EDT_USERNAME, m_stuInfo.szUseName, sizeof(m_stuInfo.szUseName));
	
	// Password
	GetDlgItemText(IDC_WIRELESS_EDT_PASSWORD, m_stuInfo.szPassword, sizeof(m_stuInfo.szPassword));
	
	// Day3GFluxTactic
	m_stuInfo.emDay3GFluxTactic = (EM_CFG_DAY3GFLUXTACTIC)m_cmbFluxTactic.GetCurSel();
	
	// Day3GFluxUp
	m_stuInfo.dwDay3GFluxUp = GetDlgItemInt(IDC_WIRELESS_EDT_DAY3GFLUXUP, NULL, FALSE);
	
	// Day3GFluxUse
	m_stuInfo.dwDay3GFluxUse = GetDlgItemInt(IDC_WIRELESS_EDT_DAY3GFLUXUSE, NULL, FALSE);
	
	// Day3GFluxAction
	m_stuInfo.emDay3GFluxAction = (EM_CFG_DAY3GFLUXACTION)m_cmbFluxAction.GetCurSel();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgWireless message handlers

BOOL CDlgCfgWireless::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_WIRELESS);
	
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_CFG_WIRELESS), ConvertString("Prompt"));
		return TRUE;
	}
	
	InitDlg();
	if (GetCurrentStrategy())
	{
		StuToDlg();
	}

// 	UpdateData(FALSE);	
	return TRUE; 
}

BOOL CDlgCfgWireless::GetCurrentStrategy()
{
	char szJsonBuf[1024*32] = {0};	
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_WIRELESS,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query wireless failed:", DLG_CFG_WIRELESS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}

	CFG_WIRELESS_INFO stuWireless = {0};
	DWORD dwRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_WIRELESS, szJsonBuf, (void*)&stuWireless, sizeof(stuWireless), &dwRetLen);
	if (!bRet || dwRetLen != sizeof(stuWireless))
	{
		MessageBox(ConvertString("Parse wireless failed.", DLG_CFG_WIRELESS), ConvertString("Prompt"));
		return FALSE;
	}
	else
	{
		memcpy(&m_stuInfo, &stuWireless, sizeof(CFG_WIRELESS_INFO));
	}

	return TRUE;
}

void CDlgCfgWireless::OnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();


	char szJsonBuf[1024*32] = {0};

	BOOL bRet = CLIENT_PacketData(CFG_CMD_WIRELESS, (void*)&m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet wireless failed.", DLG_CFG_WIRELESS), ConvertString("Prompt"));
		return;
	}

	int nErr = 0, nRestart = 0;
	bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_WIRELESS,
		-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Setup wireless failed :", DLG_CFG_WIRELESS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	}
	
	MessageBox(ConvertString("Setup wireless successfully.", DLG_CFG_WIRELESS), ConvertString("Prompt"));
	return;
}

void CDlgCfgWireless::OnCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
