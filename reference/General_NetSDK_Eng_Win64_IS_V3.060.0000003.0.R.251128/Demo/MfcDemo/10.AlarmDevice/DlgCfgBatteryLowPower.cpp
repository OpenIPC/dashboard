// DlgCfgBatteryLowPower.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgBatteryLowPower.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgBatteryLowPower dialog


CDlgCfgBatteryLowPower::CDlgCfgBatteryLowPower(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgBatteryLowPower::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgBatteryLowPower)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgBatteryLowPower::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgBatteryLowPower)
	DDX_Control(pDX, IDC_BATTERYLOWPOWER_CHK_ENABLE, m_chkEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgBatteryLowPower, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgBatteryLowPower)
	ON_BN_CLICKED(IDC_BATTERYLOWPOWER_BTN_HANDLER, OnBatterylowpowerBtnHandler)
	ON_BN_CLICKED(IDC_BATTERYLOWPOWER_BTN_GET, OnBatterylowpowerBtnGet)
	ON_BN_CLICKED(IDC_BATTERYLOWPOWER_BTN_SET, OnBatterylowpowerBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgBatteryLowPower private method

BOOL CDlgCfgBatteryLowPower::getInfo()
{
	// enable
	m_stuInfo.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;

	// event handler

	return TRUE;
}

BOOL CDlgCfgBatteryLowPower::showInfo()
{
	// enable
	m_chkEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
	
	// event handler

	return TRUE;
}

BOOL CDlgCfgBatteryLowPower::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = -1;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_BATTERY_LOW_POWER, nSelChn, 
		szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
	
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_BATTERY_LOW_POWER, szJsonBuf, (void*)&m_stuInfo, sizeof(CFG_BATTERY_LOW_POWER_INFO), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse BatterLowPower error..."), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig BatterLowPower error:", DLG_ALARM_POWERFAULT), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgBatteryLowPower::SetConfigToDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	
	BOOL bRet = CLIENT_PacketData(CFG_CMD_BATTERY_LOW_POWER, (void*)&m_stuInfo, 
		sizeof(CFG_BATTERY_LOW_POWER_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet BatterLowPower error..."), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_BATTERY_LOW_POWER, -1, szJsonBuf, 
			strlen(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig BatterLowPower failed:", DLG_ALARM_POWERFAULT), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig BatterLowPower ok!"), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgBatteryLowPower message handlers

BOOL CDlgCfgBatteryLowPower::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARM_POWERFAULT);
	// TODO: Add extra initialization here
	if (GetConfigFromDevice())
	{
		showInfo();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgBatteryLowPower::OnBatterylowpowerBtnHandler() 
{
	// TODO: Add your control notification handler code here	
    CSubDlgCfgEventHandler* dlgEventHandler = new CSubDlgCfgEventHandler;
    if (dlgEventHandler != NULL)
    {
        dlgEventHandler->SetEventHandler(m_stuInfo.stuEventHandler);
        
        if (IDOK == dlgEventHandler->DoModal())
        {
            const CFG_ALARM_MSG_HANDLE& stuEventHandler = dlgEventHandler->GetEventHandler();
            memcpy(&m_stuInfo.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
        }
	}
}

void CDlgCfgBatteryLowPower::OnBatterylowpowerBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		showInfo();
	}	
}

void CDlgCfgBatteryLowPower::OnBatterylowpowerBtnSet() 
{
	// TODO: Add your control notification handler code here
	getInfo();
	SetConfigToDevice();	
}
