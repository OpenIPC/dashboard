// DlgCfgNTP.cpp : implementation file
//
#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgNTP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNTP dialog


CDlgCfgNTP::CDlgCfgNTP(CWnd* pParent /* = NULL */, LLONG lLoginId /* = NULL */)
	: CDialog(CDlgCfgNTP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgNTP)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgNTP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgNTP)
	DDX_Control(pDX, IDC_CFG_NTP_CMB_TIMEZONE, m_cmbTimeZone);
	DDX_Control(pDX, IDC_CFG_NTP_CHK_ENABLE, m_chkEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgNTP, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgNTP)
	ON_BN_CLICKED(IDC_CFG_NTP_BTN_GET, OnCfgNtpBtnGet)
	ON_BN_CLICKED(IDC_CFG_NTP_BTN_SET, OnCfgNtpBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNTP private method

void CDlgCfgNTP::InitDlg()
{
	// enable
	m_chkEnable.SetCheck(BST_UNCHECKED);

	// name
	SetDlgItemText(IDC_CFG_NTP_EDT_NAME, "");

	// port
	SetDlgItemInt(IDC_CFG_NTP_EDT_PORT, 0, FALSE);

	// update period
	SetDlgItemInt(IDC_CFG_NTP_EDT_UPDATEPERIOD, 0, FALSE);

	// time zone
	m_cmbTimeZone.ResetContent();
	for (int i = 0; i < sizeof(DemoTimeZone) / sizeof(DemoTimeZone[0]); i++)
	{
		m_cmbTimeZone.InsertString(-1, DemoTimeZone[i].pszName);
	}
	m_cmbTimeZone.SetCurSel(-1);

	// description
	SetDlgItemText(IDC_CFG_NTP_EDT_DESCRIPTION, "");
}

void CDlgCfgNTP::StuToDlg()
{
	m_chkEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(IDC_CFG_NTP_EDT_NAME, m_stuInfo.szAddress);
	SetDlgItemInt(IDC_CFG_NTP_EDT_PORT, m_stuInfo.nPort, FALSE);
	SetDlgItemInt(IDC_CFG_NTP_EDT_UPDATEPERIOD, m_stuInfo.nUpdatePeriod, FALSE);
	m_cmbTimeZone.SetCurSel((int)m_stuInfo.emTimeZoneType);
	SetDlgItemText(IDC_CFG_NTP_EDT_DESCRIPTION, m_stuInfo.szTimeZoneDesc);
}

void CDlgCfgNTP::DlgToStu()
{
	m_stuInfo.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;
	GetDlgItemText(IDC_CFG_NTP_EDT_NAME, m_stuInfo.szAddress, sizeof(m_stuInfo.szAddress));
	m_stuInfo.nPort = GetDlgItemInt(IDC_CFG_NTP_EDT_PORT, NULL, FALSE);
	m_stuInfo.nUpdatePeriod = GetDlgItemInt(IDC_CFG_NTP_EDT_UPDATEPERIOD, NULL, FALSE);
	m_stuInfo.emTimeZoneType = (EM_CFG_TIME_ZONE_TYPE)m_cmbTimeZone.GetCurSel();
	GetDlgItemText(IDC_CFG_NTP_EDT_DESCRIPTION, m_stuInfo.szTimeZoneDesc, sizeof(m_stuInfo.szTimeZoneDesc));
}

BOOL CDlgCfgNTP::GetConfig()
{
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0, nRestart = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_NTP,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_NTP_INFO stuInfo = {sizeof(CFG_NTP_INFO)};  
		
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_NTP, szJsonBuf, (void*)&stuInfo, sizeof(stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse NTP error..."), DLG_CFG_NTP), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_NTP_INFO));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("get Config NTP error:", DLG_CFG_NTP),CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgNTP::SetConfig()
{
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_NTP, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet NTP error..."), DLG_CFG_NTP));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif	
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_NTP,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRetSetupCfg)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig NTP failed:", DLG_CFG_NTP), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig NTP ok!"), DLG_CFG_NTP), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNTP message handlers

BOOL CDlgCfgNTP::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_NTP);
	
	// TODO: Add extra initialization here
	InitDlg();
	if (GetConfig())
	{
		StuToDlg();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgNTP::OnCfgNtpBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfig())
	{
		StuToDlg();
	}	
}

void CDlgCfgNTP::OnCfgNtpBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfig();
}
