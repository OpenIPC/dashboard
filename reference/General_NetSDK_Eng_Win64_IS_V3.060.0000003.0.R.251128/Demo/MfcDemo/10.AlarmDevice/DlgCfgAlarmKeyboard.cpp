// DlgCfgAlarmKeyboard.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmKeyboard.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmKeyboard dialog


CDlgCfgAlarmKeyboard::CDlgCfgAlarmKeyboard(CWnd* pParent /* = NULL */, LLONG lLoginId /* = NULL */, int nMaxKeyboard /* = 10 */)
	: CDialog(CDlgCfgAlarmKeyboard::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmKeyboard)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	m_pstuInfo = NULL;
	m_nMaxKeyboard = nMaxKeyboard;
	assert(m_nMaxKeyboard > 0);
	m_nCurKeyboardCount = 0;
}


void CDlgCfgAlarmKeyboard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmKeyboard)
	DDX_Control(pDX, IDC_KEYBOARD_CHK_ENABLE, m_chkEnable);
	DDX_Control(pDX, IDC_KEYBOARD_CMB_ADDRESS, m_cbAddress);
	DDX_Control(pDX, IDC_KEYBOARD_CMB_BAUDRATE, m_cbBaudRate);
	DDX_Control(pDX, IDC_KEYBOARD_CMB_STOPBIT, m_cbStopBit);
	DDX_Control(pDX, IDC_KEYBOARD_CMB_PARITY, m_cbVerify);
	DDX_Control(pDX, IDC_KEYBOARD_CMB_DATABIT, m_cbDataBit);
	DDX_Control(pDX, IDC_KEYBOARD_CMB_SERIALPORT, m_cbSerialPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmKeyboard, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmKeyboard)
	ON_CBN_SELCHANGE(IDC_KEYBOARD_CMB_SERIALPORT, OnSelchangeKeyboardCmbSerialport)
	ON_CBN_SELCHANGE(IDC_KEYBOARD_CMB_ADDRESS, OnSelchangeKeyboardCmbAddress)
	ON_BN_CLICKED(IDC_KEYBOARD_BTN_GET, OnKeyboardBtnGet)
	ON_BN_CLICKED(IDC_KEYBOARD_BTN_SET, OnKeyboardBtnSet)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmKeyboard private method

void CDlgCfgAlarmKeyboard::InitDlg()
{	
	if (m_nMaxKeyboard > 0)
	{
		m_pstuInfo = new CFG_ALARMKEYBOARD_INFO[m_nMaxKeyboard];
		assert(m_pstuInfo != NULL);
		memset(m_pstuInfo, 0, sizeof(CFG_ALARMKEYBOARD_INFO) * m_nMaxKeyboard);
	}

	int i = 0;
	
	for (i = 0; i < MAX_COMM_NUM; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Channel", DLG_CFG_SERIAL), i + 1);
		m_cbSerialPort.InsertString(-1, csTemp);
	}
	m_cbSerialPort.SetCurSel(-1);


	for (i = 0; i < m_nMaxKeyboard; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Address", DLG_CFG_ALARMKEYBOARD), i + 1);
		m_cbAddress.InsertString(-1, csTemp);
	}
	m_cbAddress.SetCurSel(-1);
	
	m_cbDataBit.InsertString(0, "5");
	m_cbDataBit.InsertString(1, "6");
	m_cbDataBit.InsertString(2, "7");
	m_cbDataBit.InsertString(3, "8");
	m_cbDataBit.SetCurSel(-1);
	
	m_cbVerify.InsertString(0, ConvertString("None", DLG_CFG_EXALARMBOX));
	m_cbVerify.InsertString(1, ConvertString("Odd", DLG_CFG_EXALARMBOX));
	m_cbVerify.InsertString(2, ConvertString("Even", DLG_CFG_EXALARMBOX));
	m_cbVerify.SetCurSel(-1);
	
	m_cbStopBit.InsertString(0, "1");
	m_cbStopBit.InsertString(1, "1.5");
	m_cbStopBit.InsertString(2, "2");
	m_cbStopBit.SetCurSel(-1);
	
	m_cbBaudRate.InsertString(-1, "300");
	m_cbBaudRate.InsertString(-1, "600");
	m_cbBaudRate.InsertString(-1, "1200");
	m_cbBaudRate.InsertString(-1, "2400");
	m_cbBaudRate.InsertString(-1, "4800");
	m_cbBaudRate.InsertString(-1, "9600");
	m_cbBaudRate.InsertString(-1, "19200");
	m_cbBaudRate.InsertString(-1, "38400");
	m_cbBaudRate.InsertString(-1, "57600");
	m_cbBaudRate.InsertString(-1, "115200");
	m_cbBaudRate.SetCurSel(-1);
}

BOOL CDlgCfgAlarmKeyboard::GetConfigFromDevice()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbSerialPort.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxKeyboard)
	{
		return FALSE;
	}
	
	char szJsonBuf[1024 * 2] = {0};
	int nErr = 0;
	AV_int32 nRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_ALARMKEYBOARD,
		nSel, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!nRet)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Query AlarmKeyboard Configuration failed:", DLG_CFG_ALARMKEYBOARD), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return FALSE;
	}
	
	CFG_ALARMKEYBOARD_INFO* pstuInfo = new CFG_ALARMKEYBOARD_INFO[m_nMaxKeyboard];
	assert(pstuInfo != NULL);
	memset(pstuInfo, 0, sizeof(CFG_ALARMKEYBOARD_INFO) * m_nMaxKeyboard);
	DWORD dwRetLen = 0;
	
	nRet = CLIENT_ParseData(CFG_CMD_ALARMKEYBOARD, szJsonBuf, pstuInfo, sizeof(CFG_ALARMKEYBOARD_INFO) * m_nMaxKeyboard, &dwRetLen);
	if (!nRet)
	{
		MessageBox(ConvertString("Parse data for AlarmKeyboard Configuration failed!", DLG_CFG_ALARMKEYBOARD), ConvertString("Prompt"));
		delete []pstuInfo;
		return FALSE;
	}

	int nCount = __min(m_nMaxKeyboard, dwRetLen / sizeof(CFG_ALARMKEYBOARD_INFO));
	if (nCount > 0)
	{
		m_nCurKeyboardCount = nCount;
	}
	memcpy(m_pstuInfo, pstuInfo, sizeof(CFG_ALARMKEYBOARD_INFO) * nCount);
	delete []pstuInfo;
	return TRUE;
}

BOOL CDlgCfgAlarmKeyboard::SetConfigToDevice()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbSerialPort.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxKeyboard)
	{
		return FALSE;
	}

	char szJsonBuf[1024 * 2] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARMKEYBOARD, m_pstuInfo, 
		sizeof(CFG_ALARMKEYBOARD_INFO) * m_nCurKeyboardCount, szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet data for AlarmKeyboard Configuration failed.", DLG_CFG_ALARMKEYBOARD), ConvertString("Prompt"));
		return FALSE;
	}
	
	int nErr = 0, nRestart = 0;
	bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_ALARMKEYBOARD,
		nSel, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!bRet)
	{
		CString csOut;
		csOut.Format("%s %08x!", ConvertString("Set AlarmKeyboard Configuration failed:", DLG_CFG_ALARMKEYBOARD), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return FALSE;
	}
	
	MessageBox(ConvertString("Set AlarmKeyboard Configuration Successfully.", DLG_CFG_ALARMKEYBOARD), ConvertString("Prompt"));
	return TRUE;
}

void CDlgCfgAlarmKeyboard::StuToDlg()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbAddress.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxKeyboard)
	{
		return;
	}

	m_chkEnable.SetCheck(m_pstuInfo[nSel].bEnable ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemText(IDC_KEYBOARD_EDT_PROTOCOLNAME, m_pstuInfo[nSel].szProtocolName);

	m_cbSerialPort.SetCurSel(m_pstuInfo[nSel].nPort);

	m_cbAddress.SetCurSel(m_pstuInfo[nSel].nAddress);

	m_cbBaudRate.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byBaudRate);
	m_cbDataBit.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byDataBit);
	m_cbStopBit.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byStopBit);
	m_cbVerify.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byParity);
}

void CDlgCfgAlarmKeyboard::DlgToStu()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbAddress.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxKeyboard)
	{
		return;
	}
	
	BOOL bFound = FALSE;
	int nKeyboardSeq = 0;
	for (; nKeyboardSeq < m_nCurKeyboardCount; nKeyboardSeq++)
	{
		if (m_pstuInfo[nKeyboardSeq].nAddress == nSel)
		{
			bFound = TRUE;
			break;
		}
	}

	if (bFound)
	{
		m_chkEnable.SetCheck(m_pstuInfo[nKeyboardSeq].bEnable ? BST_CHECKED : BST_UNCHECKED);
		
		GetDlgItemText(IDC_KEYBOARD_EDT_PROTOCOLNAME, m_pstuInfo[nKeyboardSeq].szProtocolName, sizeof(m_pstuInfo[nKeyboardSeq].szProtocolName));
		
		m_pstuInfo[nKeyboardSeq].nPort = 1;
		
		m_pstuInfo[nKeyboardSeq].nAddress = m_cbAddress.GetCurSel();
		
		m_pstuInfo[nKeyboardSeq].stuCommAttr.byBaudRate = m_cbBaudRate.GetCurSel();
		m_pstuInfo[nKeyboardSeq].stuCommAttr.byDataBit = m_cbDataBit.GetCurSel();
		m_pstuInfo[nKeyboardSeq].stuCommAttr.byParity = m_cbVerify.GetCurSel();
		m_pstuInfo[nKeyboardSeq].stuCommAttr.byStopBit = m_cbStopBit.GetCurSel();
	}
}

void CDlgCfgAlarmKeyboard::CleanDlg()
{
	m_chkEnable.SetCheck(BST_UNCHECKED);
	
	SetDlgItemText(IDC_KEYBOARD_EDT_PROTOCOLNAME, "");
	
	m_cbSerialPort.SetCurSel(-1);
	
	m_cbAddress.SetCurSel(-1);
	
	m_cbBaudRate.SetCurSel(-1);
	m_cbDataBit.SetCurSel(-1);
	m_cbStopBit.SetCurSel(-1);
	m_cbVerify.SetCurSel(-1);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmKeyboard message handlers

BOOL CDlgCfgAlarmKeyboard::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARMKEYBOARD);
	
	// TODO: Add extra initialization here
	if (!m_lLoginId)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_ALARMKEYBOARD), ConvertString("Prompt"));
		//return TRUE;
	}
	
	InitDlg();
	m_cbSerialPort.SetCurSel(0);
	OnSelchangeKeyboardCmbSerialport();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarmKeyboard::OnSelchangeKeyboardCmbSerialport() 
{
	// TODO: Add your control notification handler code here
	assert(m_pstuInfo != NULL);

	if (!GetConfigFromDevice())
	{
		CleanDlg();
		return;
	}

	// 默认显示第一个配置信息的键盘			
	m_chkEnable.SetCheck(m_pstuInfo[0].bEnable ? BST_CHECKED : BST_UNCHECKED);
	
	SetDlgItemText(IDC_KEYBOARD_EDT_PROTOCOLNAME, m_pstuInfo[0].szProtocolName);
	
	//m_cbSerialPort.SetCurSel(m_pstuInfo[0].nPort);
	
	m_cbAddress.SetCurSel(m_pstuInfo[0].nAddress);
	
	m_cbBaudRate.SetCurSel(m_pstuInfo[0].stuCommAttr.byBaudRate);
	m_cbDataBit.SetCurSel(m_pstuInfo[0].stuCommAttr.byDataBit);
	m_cbStopBit.SetCurSel(m_pstuInfo[0].stuCommAttr.byStopBit);
	m_cbVerify.SetCurSel(m_pstuInfo[0].stuCommAttr.byParity);	
}

void CDlgCfgAlarmKeyboard::OnSelchangeKeyboardCmbAddress() 
{
	// TODO: Add your control notification handler code here
	assert(m_pstuInfo != NULL);
	
	int nCurSerialPort = m_cbSerialPort.GetCurSel();
	if (nCurSerialPort == -1 || nCurSerialPort >= MAX_COMM_NUM)
	{
		CleanDlg();
		return;
	}

	int nSel = m_cbAddress.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxKeyboard)
	{
		CleanDlg();
		return;
	}

	BOOL bFound = FALSE;
	int nKeyboardSeq = 0;
	for (; nKeyboardSeq < m_nCurKeyboardCount; nKeyboardSeq++)
	{
		if (m_pstuInfo[nKeyboardSeq].nAddress == nSel)
		{
			bFound = TRUE;
			break;
		}
	}
	
	if (bFound)
	{
		m_chkEnable.SetCheck(m_pstuInfo[nKeyboardSeq].bEnable ? BST_CHECKED : BST_UNCHECKED);
		
		SetDlgItemText(IDC_KEYBOARD_EDT_PROTOCOLNAME, m_pstuInfo[nKeyboardSeq].szProtocolName);
		
		//m_cbSerialPort.SetCurSel(m_pstuInfo[nKeyboardSeq].nPort);
		
		//m_cbAddress.SetCurSel(m_pstuInfo[nKeyboardSeq].nAddress);
		
		m_cbBaudRate.SetCurSel(m_pstuInfo[nKeyboardSeq].stuCommAttr.byBaudRate);
		m_cbDataBit.SetCurSel(m_pstuInfo[nKeyboardSeq].stuCommAttr.byDataBit);
		m_cbStopBit.SetCurSel(m_pstuInfo[nKeyboardSeq].stuCommAttr.byStopBit);
		m_cbVerify.SetCurSel(m_pstuInfo[nKeyboardSeq].stuCommAttr.byParity);
	}
	else
	{
		m_chkEnable.SetCheck(BST_UNCHECKED);
		SetDlgItemText(IDC_KEYBOARD_EDT_PROTOCOLNAME, "");
		m_cbBaudRate.SetCurSel(-1);
		m_cbDataBit.SetCurSel(-1);
		m_cbStopBit.SetCurSel(-1);
		m_cbVerify.SetCurSel(-1);
	}
}

void CDlgCfgAlarmKeyboard::OnKeyboardBtnGet() 
{
	// TODO: Add your control notification handler code here
// 	if (GetConfigFromDevice())
// 	{
// 		StuToDlg();
// 	}
	OnSelchangeKeyboardCmbSerialport();
}

void CDlgCfgAlarmKeyboard::OnKeyboardBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfigToDevice();
}

void CDlgCfgAlarmKeyboard::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_pstuInfo)
	{
		delete []m_pstuInfo;
		m_pstuInfo = NULL;
	}
}
