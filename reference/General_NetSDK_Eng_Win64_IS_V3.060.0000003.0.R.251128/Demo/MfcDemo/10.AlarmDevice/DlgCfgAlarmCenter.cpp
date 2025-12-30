// DlgCfgAlarmCenter.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmCenter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmCenter dialog


CDlgCfgAlarmCenter::CDlgCfgAlarmCenter(CWnd* pParent /*=NULL*/, LLONG hLoginID /*= NULL*/)
	: CDialog(CDlgCfgAlarmCenter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmCenter)
	m_hLogin = hLoginID;
	//}}AFX_DATA_INIT
	memset(&m_stuAlarmServerInfo, 0, sizeof(m_stuAlarmServerInfo));
}

void CDlgCfgAlarmCenter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmCenter)
	DDX_Control(pDX, IDC_ALARMCENTER_IPADDR_IP, m_ctrlIP);
	DDX_Control(pDX, IDC_ALARMCENTER_COMBO_SECOND, m_cmbSecond);
	DDX_Control(pDX, IDC_ALARMCENTER_COMBO_MINUTE, m_cmbMin);
	DDX_Control(pDX, IDC_ALARMCENTER_COMBO_HOUR, m_cmbHour);
	DDX_Control(pDX, IDC_ALARMCENTER_COMBO_ONTIMEUPDAY, m_cbUpday);
	DDX_Control(pDX, IDC_ALARMCENTER_CHECK_ENABLE, m_ckUpEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmCenter, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmCenter)
	ON_BN_CLICKED(IDC_ALARMCENTER_BUTTON_GET_A3, OnAlarmcenterButtonGetA3)
	ON_BN_CLICKED(IDC_ALARMCENTER_BUTTON_SET_A3, OnAlarmcenterButtonSetA3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmCenter message handlers
BOOL CDlgCfgAlarmCenter::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARMCENTER);

	InitDlg();

	if (!m_hLogin)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_ALARMCENTER), ConvertString("Prompt"));
		return TRUE;
	}
	
	OnAlarmcenterButtonGetA3();	

	return TRUE; 
}

void CDlgCfgAlarmCenter::OnAlarmcenterButtonGetA3() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		showInfo();
	}
}

void CDlgCfgAlarmCenter::OnAlarmcenterButtonSetA3() 
{
	// TODO: Add your control notification handler code here
	if (getInfo())
	{
		SetConfigToDevice();
	}
}

void CDlgCfgAlarmCenter::InitDlg()
{
	m_ckUpEnable.SetCheck(BST_UNCHECKED);
	
	int i = 0;

	m_cbUpday.ResetContent();
	for (i = 0; i < sizeof(stuDemoReportWeekDay) / sizeof(stuDemoReportWeekDay[0]); i++)
	{
		m_cbUpday.InsertString(-1, ConvertString(stuDemoReportWeekDay[i].szName, DLG_CFG_ALARMCENTER));
	}
	m_cbUpday.SetCurSel(-1);
	
	m_cmbHour.ResetContent();
	for (i = 0; i < 24; ++i)
	{
		CString csTemp;
		csTemp.Format("%02d", i);
		m_cmbHour.InsertString(-1, csTemp);
	}
	m_cmbHour.SetCurSel(-1);
	
	m_cmbMin.ResetContent();
	m_cmbSecond.ResetContent();
	for (i = 0; i < 60; i++)
	{
		CString csTemp;
		csTemp.Format("%02d", i);
		m_cmbMin.InsertString(-1, csTemp);
		m_cmbSecond.InsertString(-1, csTemp);
	}
	m_cmbMin.SetCurSel(-1);
	m_cmbSecond.SetCurSel(-1);
}

void CDlgCfgAlarmCenter::showInfo()
{
	// Enable
	if (m_stuAlarmServerInfo.stuMainServer.bEnable)
	{
		m_ckUpEnable.SetCheck(BST_CHECKED);
	}
	else
	{
		m_ckUpEnable.SetCheck(BST_UNCHECKED);
	}

	// ReportWeekDay
	if (m_stuAlarmServerInfo.stuMainServer.emReportWeekDay >= emReportWeekDay_DoNotReport
		&& m_stuAlarmServerInfo.stuMainServer.emReportWeekDay <= emReportWeekDay_Everyday)
	{
		m_cbUpday.SetCurSel(int(m_stuAlarmServerInfo.stuMainServer.emReportWeekDay + 1));
	}
	else
	{
		m_cbUpday.SetCurSel(-1);
	}
	
	// ReportHour
	if (m_stuAlarmServerInfo.stuMainServer.nHour >= 0
		&& m_stuAlarmServerInfo.stuMainServer.nHour <= 23)
	{
		m_cmbHour.SetCurSel(m_stuAlarmServerInfo.stuMainServer.nHour);
	}
	else
	{
		m_cmbHour.SetCurSel(-1);
	}

	// ReportMinute
	if (m_stuAlarmServerInfo.stuMainServer.nMinute >= 0
		&& m_stuAlarmServerInfo.stuMainServer.nMinute <= 59)
	{
		m_cmbMin.SetCurSel(m_stuAlarmServerInfo.stuMainServer.nMinute);
	}
	else
	{
		m_cmbMin.SetCurSel(-1);
	}

	// ReportSecond
	if (m_stuAlarmServerInfo.stuMainServer.nSecond >= 0
		&& m_stuAlarmServerInfo.stuMainServer.nSecond <= 59)
	{
		m_cmbSecond.SetCurSel(m_stuAlarmServerInfo.stuMainServer.nSecond);
	}
	else
	{
		m_cmbSecond.SetCurSel(-1);
	}

	// IP address
	m_ctrlIP.SetWindowText(m_stuAlarmServerInfo.stuMainServer.szAddress);

	// Port
	SetDlgItemInt(IDC_ALARMCENTER_EDIT_PORT, m_stuAlarmServerInfo.stuMainServer.nPort);
}

BOOL CDlgCfgAlarmCenter::getInfo()
{
	// Enable
	m_stuAlarmServerInfo.stuMainServer.bEnable = m_ckUpEnable.GetCheck() ? TRUE : FALSE;
	
	// ReportWeekDay
	m_stuAlarmServerInfo.stuMainServer.emReportWeekDay = emCFG_REPORTWEEKDAY(m_cbUpday.GetCurSel() - 1);

	// ReportHour
	m_stuAlarmServerInfo.stuMainServer.nHour = m_cmbHour.GetCurSel();

	// ReportMinute
	m_stuAlarmServerInfo.stuMainServer.nMinute = m_cmbMin.GetCurSel();

	// ReportSecond
	m_stuAlarmServerInfo.stuMainServer.nSecond = m_cmbSecond.GetCurSel();

	// IP address
	CString csIp;
	m_ctrlIP.GetWindowText(csIp);
	strncpy(m_stuAlarmServerInfo.stuMainServer.szAddress, csIp.GetBuffer(0), sizeof(m_stuAlarmServerInfo.stuMainServer.szAddress) - 1);

	// Port
	int nPort = GetDlgItemInt(IDC_ALARMCENTER_EDIT_PORT);
	if (nPort > 0 && nPort <= 65535)
	{
		m_stuAlarmServerInfo.stuMainServer.nPort = nPort;
	}
	else
	{
		MessageBox(ConvertString("Port error!", DLG_CFG_ALARMCENTER), ConvertString("Prompt"));
		return FALSE;
	}

	return TRUE;
}

BOOL CDlgCfgAlarmCenter::SetConfigToDevice()
{
	char szJsonBuf[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARMSERVER, &m_stuAlarmServerInfo, sizeof(m_stuAlarmServerInfo), 
		szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet AlarmServer error..."), DLG_CFG_ALARMCENTER), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_hLogin, CFG_CMD_ALARMSERVER, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig AlarmServer failed:", DLG_CFG_ALARMCENTER), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString("SetConfig AlarmServer ok!", DLG_CFG_ALARMCENTER), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgAlarmCenter::GetConfigFromDevice()
{
	char szJsonBuf[1024] = {0};
	int nerror = 0;
	int nSelChn = -1;
	BOOL bRet = CLIENT_GetNewDevConfig(m_hLogin, CFG_CMD_ALARMSERVER, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
	
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ALARMSERVER, szJsonBuf, (void*)&m_stuAlarmServerInfo, sizeof(m_stuAlarmServerInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse AlarmServer error..."), DLG_CFG_ALARMCENTER), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig AlarmServer error:" ,DLG_CFG_ALARMCENTER),CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}
