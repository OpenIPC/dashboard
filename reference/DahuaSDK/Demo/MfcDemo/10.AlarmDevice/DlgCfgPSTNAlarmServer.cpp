// CDlgCfgPSTNAlarmServer.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgPSTNAlarmServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNAlarmServer dialog


CDlgCfgPSTNAlarmServer::CDlgCfgPSTNAlarmServer(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgCfgPSTNAlarmServer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgPSTNAlarmServer)
	m_lLoginId = lLoginId;
	m_nDelay = 0;
	m_nTryCount = 0;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgPSTNAlarmServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgPSTNAlarmServer)
	DDX_Control(pDX, IDC_PSTN_CMB_SERVER, m_cmbServer);
	DDX_Control(pDX, IDC_PSTN_CMB_PROTOCOL, m_cmbProtocol);
	DDX_Control(pDX, IDC_PSTN_EDT_NUMBER, m_edNumber);
	DDX_Control(pDX, IDC_PSTN_EDT_NAME, m_edName);
	DDX_Control(pDX, IDC_PSTN_EDT_ACCOUNT, m_edAccount);
	DDX_Control(pDX, IDC_PSTN_CMB_MODE, m_cmbMode);
	DDX_Control(pDX, IDC_PSTN_CHK_ENABLE, m_chkEnable);
	DDX_Text(pDX, IDC_PSTN_EDT_DELAY, m_nDelay);
	DDX_Text(pDX, IDC_PSTN_EDT_TRYCOUNT, m_nTryCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgPSTNAlarmServer, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgPSTNAlarmServer)
	ON_BN_CLICKED(IDC_PSTN_BTN_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_PSTN_BTN_SET, OnButtonSet)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_PSTN_CMB_SERVER, OnSelchangeComboServer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNAlarmServer message handlers

BOOL CDlgCfgPSTNAlarmServer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_PSTN_ALARM_SERVER);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"),DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
		GetDlgItem(IDC_PSTN_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_PSTN_BTN_SET)->EnableWindow(FALSE);
	}
	
	InitDlg();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgCfgPSTNAlarmServer::getCfgFromDevice()
{
	char szOutBuffer[1024 * 3] = {0};
	int nerror = 0;
	int nSelChn = -1; // 获取全部配置内容
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_PSTN_ALARM_SERVER, nSelChn, szOutBuffer, 1024 * 3, &nerror, SDK_API_WAIT);
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_PSTN_ALARM_SERVER, szOutBuffer, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			m_stuInfo.nServerCount = 0;
			MessageBox(ConvertString(CString("parse PSTNAlarmServer error..."),DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("get PSTNAlarmServer config error:", DLG_PSTN_ALARM_SERVER), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgPSTNAlarmServer::setCfgToDevice()
{
	char szJsonBuf[1024 * 3] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_PSTN_ALARM_SERVER, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet PSTNAlarmServer error..."), DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_PSTN_ALARM_SERVER, -1, szJsonBuf, sizeof(m_stuInfo), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig PSTNAlarmServer failed:",DLG_PSTN_ALARM_SERVER), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("PSTNAlarmServer SetConfig ok!"),DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

void CDlgCfgPSTNAlarmServer::InitDlg()
{
	GetDlgItem(IDC_PSTN_BTN_SET)->EnableWindow(TRUE);
	GetDlgItem(IDC_PSTN_BTN_GET)->EnableWindow(TRUE);
	int i = 0;
	for (i = 0; i < 2; ++i)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Server", DLG_PSTN_ALARM_SERVER), i + 1);
		m_cmbServer.InsertString(-1, csTemp);	
	}
	// 统一显示服务器1的内容
	m_cmbServer.SetCurSel(0);
	// 先取默认值
	m_stuInfo.nServerCount = 2;
	for (i = 0; i < sizeof(DemoMode) / sizeof(DemoMode[0]); ++i)
	{
		m_cmbMode.InsertString(-1, ConvertString(DemoMode[i].pszMode, DLG_PSTN_ALARM_SERVER));
	}
	for (i = 0; i < sizeof(DemoType) / sizeof(DemoType[0]); ++i)
	{
		m_cmbProtocol.InsertString(-1, ConvertString(DemoType[i].pszType, DLG_PSTN_ALARM_SERVER));
	}

// 	if (!m_stuInfo.bEnable)
// 	{
// 		//GetDlgItem(IDC_BUTTON_GET)->EnableWindow(FALSE);
// 		m_edName.SetWindowText("");
// 		m_edNumber.SetWindowText("");
// 		m_edAccount.SetWindowText("");
// 		m_edDelay.SetWindowText("");
// 		m_edTryCount.SetWindowText("");
// 		m_cmbMode.SetCurSel(0);
// 		m_cmbProtocol.SetCurSel(0);
// 	}
// 	else
	{
		//GetDlgItem(IDC_BUTTON_GET)->EnableWindow(TRUE);
		ShowDlgInfo(0);
	}
	getPSTNAbility();
}

void CDlgCfgPSTNAlarmServer::ShowDlgInfo(int nSerNum)
{
	m_chkEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
	m_cmbServer.SetCurSel(nSerNum);
	m_cmbMode.SetCurSel(m_stuInfo.stuPSTNAlarmServer[nSerNum].emMode);
	m_cmbProtocol.SetCurSel(m_stuInfo.stuPSTNAlarmServer[nSerNum].emProtocol);
	m_edName.SetWindowText(m_stuInfo.stuPSTNAlarmServer[nSerNum].szName);
	m_edNumber.SetWindowText(m_stuInfo.stuPSTNAlarmServer[nSerNum].szNumber);
	m_edAccount.SetWindowText(m_stuInfo.stuPSTNAlarmServer[nSerNum].szAccount);
// 	char szDelay[10], szTryCount[10];
// 	itoa(m_stuInfo.stuPSTNAlarmServer[nSerNum].nDelay, szDelay, 10);
// 	itoa(m_stuInfo.stuPSTNAlarmServer[nSerNum].nTryCount, szTryCount, 10);
// 	m_edDelay.SetWindowText(szDelay);
// 	m_edTryCount.SetWindowText(szTryCount);
	m_nDelay = m_stuInfo.stuPSTNAlarmServer[nSerNum].nDelay;
	m_nTryCount = m_stuInfo.stuPSTNAlarmServer[nSerNum].nTryCount;
	UpdateData(FALSE);
}

BOOL CDlgCfgPSTNAlarmServer::getDlgInfo()
{
	m_stuInfo.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;
	int nSerNum = m_cmbServer.GetCurSel();
	m_stuInfo.stuPSTNAlarmServer[nSerNum].emMode = (CFG_EM_SIGNALTRANSMIT_MODE)m_cmbMode.GetCurSel();
	m_stuInfo.stuPSTNAlarmServer[nSerNum].emProtocol = (CFG_EM_PSTN_PROTOCOL_TYPE)m_cmbProtocol.GetCurSel();
	m_edName.GetWindowText(m_stuInfo.stuPSTNAlarmServer[nSerNum].szName, MAX_NAME_LEN);
	m_edNumber.GetWindowText(m_stuInfo.stuPSTNAlarmServer[nSerNum].szNumber, MAX_PHONE_NUMBER_LEN);
	m_edAccount.GetWindowText(m_stuInfo.stuPSTNAlarmServer[nSerNum].szAccount, MAX_PHONE_NUMBER_LEN);
// 	char szDelay[10] = {0}, szTryCount[10] = {0};
// 	m_edDelay.GetWindowText(szDelay, 10);
// 	m_edTryCount.GetWindowText(szTryCount, 10);
// 	m_stuInfo.stuPSTNAlarmServer[nSerNum].nDelay = atoi(szDelay);
// 	m_stuInfo.stuPSTNAlarmServer[nSerNum].nTryCount = atoi(szTryCount);
	BOOL bRet = UpdateData(TRUE);
 	m_stuInfo.stuPSTNAlarmServer[nSerNum].nDelay = m_nDelay;
 	m_stuInfo.stuPSTNAlarmServer[nSerNum].nTryCount = m_nTryCount;
	return bRet;
}

void CDlgCfgPSTNAlarmServer::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (m_stuInfo.nServerCount <= 0)
	{
		MessageBox(ConvertString("the device do not support PSTN or get ability error!",DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
		return;
	}
	if (getCfgFromDevice())
	{
		// 重新获取后默认显示1号服务器
		ShowDlgInfo(0);
	}
}

void CDlgCfgPSTNAlarmServer::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (m_stuInfo.nServerCount <= 0)
	{
		MessageBox(ConvertString("the device do not support PSTN or get ability error!",DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
		return;
	}
	if (getDlgInfo())
	{
		setCfgToDevice();
	}
}

void CDlgCfgPSTNAlarmServer::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CDlgCfgPSTNAlarmServer::OnSelchangeComboServer() 
{
	// TODO: Add your control notification handler code here
	int nSerNum = m_cmbServer.GetCurSel(); // 从0开始
	// 用户有权增加服务器数量,是否有效需要设备来控制
	m_stuInfo.nServerCount = __max(m_stuInfo.nServerCount, nSerNum + 1);
	ShowDlgInfo(nSerNum);
}

void CDlgCfgPSTNAlarmServer::getPSTNAbility()
{
	CFG_CAP_ALARM_INFO stuAlarmInfo = {0};
	char szAlarmBuff[1024 * 3] = {0};
	int nError = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_lLoginId, CFG_CAP_ALARM, 0, szAlarmBuff, sizeof(szAlarmBuff), &nError, SDK_API_WAIT);
	if (bRet)
	{
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_ALARM, szAlarmBuff, &stuAlarmInfo, sizeof(CFG_CAP_ALARM_INFO), &dwRet);
		if (bRet)
		{
			m_stuInfo.nServerCount = stuAlarmInfo.nMaxPSTNAlarmServer;
		}
		else
		{
			// 失败时为0
			m_stuInfo.nServerCount = 0;
			MessageBox(ConvertString("get PSTN ability error!",DLG_PSTN_ALARM_SERVER), ConvertString("Prompt"));
		}
	}
	else
	{
		// 失败时为0
		m_stuInfo.nServerCount = 0;

		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryAbility PSTNAlarm error:",DLG_PSTN_ALARM_SERVER),
			CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
}
