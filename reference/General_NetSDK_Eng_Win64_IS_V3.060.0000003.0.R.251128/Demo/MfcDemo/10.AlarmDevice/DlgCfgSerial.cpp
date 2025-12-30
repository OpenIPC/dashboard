// DlgCfgSerial.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgSerial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSerial dialog

CDlgCfgSerial::CDlgCfgSerial(CWnd* pParent /*=NULL*/, LLONG lLoginID /*=NULL*/)
	: CDialog(CDlgCfgSerial::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgSerial)
	m_csChannel = _T("");
	m_csProtocol = _T("");
	m_lLoginId = lLoginID;
	//}}AFX_DATA_INIT
	memset(&m_stuComm, 0, sizeof(m_stuComm));
}


void CDlgCfgSerial::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgSerial)
	DDX_Control(pDX, IDC_SERIAL_CMB_PROTOCOL, m_cbProtocol);
	DDX_Control(pDX, IDC_SERIAL_CMB_CHN, m_cbChannel);
	DDX_Control(pDX, IDC_SERIAL_CMB_STOPBIT, m_cbStopBit);
	DDX_Control(pDX, IDC_SERIAL_CMB_VERIFY, m_cbVerify);
	DDX_Control(pDX, IDC_SERIAL_CMB_DATABIT, m_cbDataBit);
	DDX_Control(pDX, IDC_SERIAL_CMB_BAUDRATE, m_cbBaudRate);
	DDX_Text(pDX, IDC_SERIAL_CMB_CHN, m_csChannel);
	DDX_Text(pDX, IDC_SERIAL_EDT_PROTOCOL, m_csProtocol);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgSerial, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgSerial)
	ON_BN_CLICKED(IDC_SERIAL_BTN_SET, OnButtonSet)
	ON_CBN_SELCHANGE(IDC_SERIAL_CMB_CHN, OnSelchangeCfgSerialCmbChn)
	ON_CBN_SELENDCANCEL(IDC_SERIAL_CMB_PROTOCOL, OnSelendcancelSerialCmbProtocol)
	ON_CBN_SELENDCANCEL(IDC_SERIAL_CMB_DATABIT, OnSelendcancelComboDatabit)
	ON_CBN_SELENDCANCEL(IDC_SERIAL_CMB_STOPBIT, OnSelendcancelComboStopbit)
	ON_CBN_SELENDCANCEL(IDC_SERIAL_CMB_VERIFY, OnSelendcancelComboVerify)
	ON_CBN_SELENDCANCEL(IDC_SERIAL_CMB_BAUDRATE, OnSelendcancelComboBaudrate)
	ON_BN_CLICKED(IDC_SERIAL_BTN_GET, OnButtonGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSerial private method

void CDlgCfgSerial::InitDlg()
{
	int i = 0;

	for (i = 0; i < MAX_COMM_NUM; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Channel", DLG_CFG_SERIAL), i + 1);
		m_cbChannel.InsertString(-1, csTemp);
	}
	m_cbChannel.SetCurSel(-1);

	for (i = 0; i < sizeof(DemoCommProtocol) / sizeof(DemoCommProtocol[0]); i++)
	{
		m_cbProtocol.InsertString(-1, DemoCommProtocol[i].pszName);
	}
	m_cbProtocol.SetCurSel(-1);

	m_cbDataBit.InsertString(0, "5");
	m_cbDataBit.InsertString(1, "6");
	m_cbDataBit.InsertString(2, "7");
	m_cbDataBit.InsertString(3, "8");
	m_cbDataBit.SetCurSel(-1);
	
	m_cbVerify.InsertString(0, ConvertString("None", DLG_CFG_SERIAL));
	m_cbVerify.InsertString(1, ConvertString("Odd", DLG_CFG_SERIAL));
	m_cbVerify.InsertString(2, ConvertString("Even", DLG_CFG_SERIAL));
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

void CDlgCfgSerial::StuToDlg()
{
	int nSel = m_cbChannel.GetCurSel();
	if (-1 == nSel || nSel >= MAX_COMM_NUM)
	{
		nSel = 0;
	}

	// channel
	m_cbChannel.SetCurSel(nSel);

	// protocol
	if (0 == strcmp(m_stuComm.stuComms[nSel].szProtocolName, "DH-ALARM"))
	{
		m_cbProtocol.SetCurSel((int)em_CommProtocol_DHAlarm);
	}
	else
	{
		m_cbProtocol.SetCurSel((int)em_CommProtocol_Unknown);
	}

	// mode

	// bus

	// databit
	m_cbDataBit.SetCurSel((int)m_stuComm.stuComms[nSel].stuAttribute.byDataBit);

	// stopbit
	m_cbStopBit.SetCurSel((int)m_stuComm.stuComms[nSel].stuAttribute.byStopBit);

	// parity
	m_cbVerify.SetCurSel((int)m_stuComm.stuComms[nSel].stuAttribute.byParity);

	// baudrate
	m_cbBaudRate.SetCurSel((int)m_stuComm.stuComms[nSel].stuAttribute.byBaudRate);

	// address=> ignore by alarm-device
}

void CDlgCfgSerial::DlgToStu()
{
	int nSel = m_cbChannel.GetCurSel();
	if (-1 == nSel || nSel >= MAX_COMM_NUM)
	{
		return;
	}

	// protocol
	memset(m_stuComm.stuComms[nSel].szProtocolName, 0, sizeof(MAX_PROTOCOL_NAME_LEN));

	if (em_CommProtocol_Unknown == m_cbProtocol.GetCurSel())
	{
		// nothing to be filled with
	}
	else if (em_CommProtocol_DHAlarm == m_cbProtocol.GetCurSel())
	{
		CString csProtocol;
		m_cbProtocol.GetWindowText(csProtocol);
		strncpy(m_stuComm.stuComms[nSel].szProtocolName,
			csProtocol,
			__min(MAX_PROTOCOL_NAME_LEN - 1, csProtocol.GetLength()));
	}

	// mode, read-only

	// bus, read-only

	// databit
	m_stuComm.stuComms[nSel].stuAttribute.byDataBit = (BYTE)m_cbDataBit.GetCurSel();

	// stopbit
	m_stuComm.stuComms[nSel].stuAttribute.byStopBit = (BYTE)m_cbStopBit.GetCurSel();

	// parity
	m_stuComm.stuComms[nSel].stuAttribute.byParity = (BYTE)m_cbVerify.GetCurSel();

	// baudrate
	m_stuComm.stuComms[nSel].stuAttribute.byBaudRate = (BYTE)m_cbBaudRate.GetCurSel();
}

BOOL CDlgCfgSerial::GetCommConfig()
{

	char szJsonBuf[1024] = {0};
	int nErr = 0, nRestart = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_COMM,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("GetDevConfig COMM error:", DLG_CFG_SERIAL),CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	
	CFG_COMMGROUP_INFO stuComPara = {0};
	DWORD dwRetLen = 0;
	
	bRet = CLIENT_ParseData(CFG_CMD_COMM, szJsonBuf, &stuComPara, sizeof(CFG_COMMGROUP_INFO), &dwRetLen);
	if (!bRet)
	{
		MessageBox(ConvertString("Parse data for serial para failed.", DLG_CFG_SERIAL), ConvertString("Prompt"));
		return FALSE;
	}
	
	memcpy(&m_stuComm, &stuComPara, sizeof(CFG_COMMGROUP_INFO));
	return TRUE;
}

BOOL CDlgCfgSerial::SetCommConfig()
{
	char szJsonBuf[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_COMM, &m_stuComm, sizeof(CFG_COMMGROUP_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet data for serial failed.", DLG_CFG_SERIAL), ConvertString("Prompt"));
		return FALSE;
	}

	int nErr = 0, nRestart = 0;
	bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_COMM,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);

	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("SetupConfig Comm failed:", DLG_CFG_SERIAL), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	
	MessageBox(ConvertString("Set serial para success.", DLG_CFG_SERIAL), ConvertString("Prompt"));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSerial message handlers

void CDlgCfgSerial::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	//CDialog::OnCancel();
	if (GetCommConfig())
	{
		StuToDlg();
	}
}

void CDlgCfgSerial::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_SERIAL), ConvertString("Prompt"));
		return;
	}

	SetCommConfig();
}

BOOL CDlgCfgSerial::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_SERIAL);
	
	// TODO: Add extra initialization here
	if (!m_lLoginId)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_SERIAL), ConvertString("Prompt"));
		//return TRUE;
	}

	InitDlg();
	GetCommConfig();
	StuToDlg();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgSerial::OnSelchangeCfgSerialCmbChn() 
{
	// TODO: Add your control notification handler code here
	StuToDlg();
}

void CDlgCfgSerial::OnSelendcancelSerialCmbProtocol() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CDlgCfgSerial::OnSelendcancelComboDatabit() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CDlgCfgSerial::OnSelendcancelComboStopbit() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CDlgCfgSerial::OnSelendcancelComboVerify() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CDlgCfgSerial::OnSelendcancelComboBaudrate() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}
