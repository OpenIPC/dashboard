// DlgCfgAlarmExAlarmBox.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmExAlarmBox.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmExAlarmBox dialog


CDlgCfgAlarmExAlarmBox::CDlgCfgAlarmExAlarmBox(CWnd* pParent /* = NULL */, LLONG lLoginId /* = NULL */, int nMaxExAlarmBox /* = MAX_EXALARMBOX_NUM */)
	: CDialog(CDlgCfgAlarmExAlarmBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmExAlarmBox)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	m_pstuInfo = NULL;
	m_nMaxExAlarmBox = nMaxExAlarmBox;
	assert(m_nMaxExAlarmBox > 0);
	m_nCurExAlarmBoxCount = 0;
}


void CDlgCfgAlarmExAlarmBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmExAlarmBox)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EXALARMBOX_CHK_ENABLE, m_chkEnable);
	DDX_Control(pDX, IDC_EXALARMBOX_CMB_BAUDRATE, m_cbBaudRate);
	DDX_Control(pDX, IDC_EXALARMBOX_CMB_STOPBIT, m_cbStopBit);
	DDX_Control(pDX, IDC_EXALARMBOX_CMB_PARITY, m_cbVerify);
	DDX_Control(pDX, IDC_EXALARMBOX_CMB_DATABIT, m_cbDataBit);
	DDX_Control(pDX, IDC_EXALARMBOX_CMB_SERIALPORT, m_cbSerialPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmExAlarmBox, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmExAlarmBox)
		// NOTE: the ClassWizard will add message map macros here
	ON_CBN_SELCHANGE(IDC_EXALARMBOX_CMB_SERIALPORT, OnSelchangeExAlarmBoxCmbSerialport)
	ON_BN_CLICKED(IDC_EXALARMBOX_BTN_GET, OnExAlarmBoxBtnGet)
	ON_BN_CLICKED(IDC_EXALARMBOX_BTN_SET, OnExAlarmBoxBtnSet)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmExAlarmBox private method

void CDlgCfgAlarmExAlarmBox::InitDlg()
{	
	if (m_nMaxExAlarmBox > 0)
	{
		m_pstuInfo = new CFG_EXALARMBOX_INFO[m_nMaxExAlarmBox];
		assert(m_pstuInfo != NULL);
		memset(m_pstuInfo, 0, sizeof(CFG_EXALARMBOX_INFO) * m_nMaxExAlarmBox);
	}

	int i = 0;
	
	for (i = 0; i < m_nMaxExAlarmBox; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Channel", DLG_CFG_EXALARMBOX), i + 1);
		m_cbSerialPort.InsertString(-1, csTemp);
	}
	m_cbSerialPort.SetCurSel(0);
	
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

BOOL CDlgCfgAlarmExAlarmBox::GetConfigFromDevice()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbSerialPort.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxExAlarmBox)
	{
		return FALSE;
	}
	
	char szJsonBuf[1024 * 2] = {0};
	int nErr = 0;
	AV_int32 nRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_EXALARMBOX,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!nRet)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Query ExAlarmBox Configuration failed: ", DLG_CFG_EXALARMBOX), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return FALSE;
	}
	
	CFG_EXALARMBOX_INFO* pstuInfo = new CFG_EXALARMBOX_INFO[m_nMaxExAlarmBox];
	assert(pstuInfo != NULL);
	memset(pstuInfo, 0, sizeof(CFG_EXALARMBOX_INFO) * m_nMaxExAlarmBox);
	DWORD dwRetLen = 0;
	
	nRet = CLIENT_ParseData(CFG_CMD_EXALARMBOX, szJsonBuf, pstuInfo, sizeof(CFG_EXALARMBOX_INFO) * m_nMaxExAlarmBox, &dwRetLen);
	if (!nRet)
	{
		MessageBox(ConvertString("Parse data for ExAlarmBox Configuration failed!", DLG_CFG_EXALARMBOX), ConvertString("Prompt"));
		delete []pstuInfo;
		return FALSE;
	}

	int nCount = __min(m_nMaxExAlarmBox, dwRetLen / sizeof(CFG_EXALARMBOX_INFO));
	if (nCount > 0)
	{
		m_nCurExAlarmBoxCount = nCount;
		m_cbSerialPort.ResetContent();
		int i = 0;
		
		for (i = 0; i < m_nCurExAlarmBoxCount; i++)
		{
			CString csTemp;
			csTemp.Format("%s %02d", ConvertString("Channel", DLG_CFG_SERIAL), i + 1);
			m_cbSerialPort.InsertString(-1, csTemp);
		}
		m_cbSerialPort.SetCurSel(0);
	}
	memcpy(m_pstuInfo, pstuInfo, sizeof(CFG_EXALARMBOX_INFO) * nCount);
	delete []pstuInfo;
	return TRUE;
}

BOOL CDlgCfgAlarmExAlarmBox::SetConfigToDevice()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbSerialPort.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxExAlarmBox)
	{
		return FALSE;
	}

	char szJsonBuf[1024 * 2] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_EXALARMBOX, m_pstuInfo, 
		sizeof(CFG_EXALARMBOX_INFO) * m_nCurExAlarmBoxCount, szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet data for ExAlarmBox Configuration failed.", DLG_CFG_EXALARMBOX), ConvertString("Prompt"));
		return FALSE;
	}
	
	int nErr = 0, nRestart = 0;
	bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_EXALARMBOX,
		-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!bRet)
	{
		CString csOut;
		csOut.Format("%s %08x!", ConvertString("Set ExAlarmBox Configuration failed:", DLG_CFG_EXALARMBOX), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return FALSE;
	}
	
	MessageBox(ConvertString("Set ExAlarmBox Configuration Successfully.", DLG_CFG_EXALARMBOX), ConvertString("Prompt"));
	return TRUE;
}

void CDlgCfgAlarmExAlarmBox::StuToDlg()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbSerialPort.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxExAlarmBox)
	{
		return;
	}

	m_chkEnable.SetCheck(m_pstuInfo[nSel].bEnable ? BST_CHECKED : BST_UNCHECKED);

	SetDlgItemText(IDC_EXALARMBOX_EDT_PROTOCOLNAME, m_pstuInfo[nSel].szProtocolName);

	m_cbBaudRate.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byBaudRate);
	m_cbDataBit.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byDataBit);
	m_cbStopBit.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byStopBit);
	m_cbVerify.SetCurSel(m_pstuInfo[nSel].stuCommAttr.byParity);
}

void CDlgCfgAlarmExAlarmBox::DlgToStu()
{
	assert(m_pstuInfo != NULL);
	int nSel = m_cbSerialPort.GetCurSel();
	if (nSel == -1 || nSel >= m_nMaxExAlarmBox)
	{
		return;
	}
	
	m_pstuInfo[nSel].bEnable = m_chkEnable.GetCheck();
	
	GetDlgItemText(IDC_EXALARMBOX_EDT_PROTOCOLNAME, m_pstuInfo[nSel].szProtocolName, sizeof(m_pstuInfo[nSel].szProtocolName));
				
	m_pstuInfo[nSel].stuCommAttr.byBaudRate = m_cbBaudRate.GetCurSel();
	m_pstuInfo[nSel].stuCommAttr.byDataBit = m_cbDataBit.GetCurSel();
	m_pstuInfo[nSel].stuCommAttr.byParity = m_cbVerify.GetCurSel();
	m_pstuInfo[nSel].stuCommAttr.byStopBit = m_cbStopBit.GetCurSel();
}

void CDlgCfgAlarmExAlarmBox::CleanDlg()
{
	m_chkEnable.SetCheck(BST_UNCHECKED);
	
	SetDlgItemText(IDC_EXALARMBOX_EDT_PROTOCOLNAME, "");
	
	m_cbSerialPort.SetCurSel(-1);
		
	m_cbBaudRate.SetCurSel(-1);
	m_cbDataBit.SetCurSel(-1);
	m_cbStopBit.SetCurSel(-1);
	m_cbVerify.SetCurSel(-1);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmExAlarmBox message handlers

BOOL CDlgCfgAlarmExAlarmBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_EXALARMBOX);
	
	// TODO: Add extra initialization here
	if (!m_lLoginId)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_CFG_EXALARMBOX), ConvertString("Prompt"));
		return FALSE;
	}
	
	InitDlg();
	// 默认显示第0个盒子
	m_cbSerialPort.SetCurSel(0);

	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarmExAlarmBox::OnSelchangeExAlarmBoxCmbSerialport() 
{
	// TODO: Add your control notification handler code here
	assert(m_pstuInfo != NULL);

	StuToDlg();
}

void CDlgCfgAlarmExAlarmBox::OnExAlarmBoxBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{	
		StuToDlg();
	}
}

void CDlgCfgAlarmExAlarmBox::OnExAlarmBoxBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfigToDevice();
}

void CDlgCfgAlarmExAlarmBox::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_pstuInfo)
	{
		delete []m_pstuInfo;
		m_pstuInfo = NULL;
	}
}
