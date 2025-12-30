// DlgCfgExAlarmInput.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgExAlarmInput.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgExAlarmInput dialog


CDlgCfgExAlarmInput::CDlgCfgExAlarmInput(CWnd* pParent /* = NULL */, LLONG lLoginID /* = NULL */)
	: CDialog(CDlgCfgExAlarmInput::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgExAlarmInput)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginID = lLoginID;
	memset(&m_stuInfo.stuAlarmIn, 0, sizeof(CFG_EXALARMINPUT_INFO));
}


void CDlgCfgExAlarmInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgExAlarmInput)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_CFG_EXALARMINPUT_EDT_NAME, m_edtName);
	DDX_Control(pDX, IDC_CFG_EXALARMINPUT_CMB_SENSORMETHOD, m_cmbSensorMethod);
	DDX_Control(pDX, IDC_CFG_EXALARMINPUT_CMB_CHN, m_cmbChn);
	DDX_Control(pDX, IDC_CFG_EXALARMINPUT_CHK_BYPASS, m_chkByPass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgExAlarmInput, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgExAlarmInput)
		// NOTE: the ClassWizard will add message map macros here
	ON_BN_CLICKED(IDC_CFG_EXALARMINPUT_BTN_EVENTHANDLER, OnCfgAlarmBtnEventhandler)
	ON_BN_CLICKED(IDC_CFG_EXALARMINPUT_BTN_GET, OnCfgAlarmBtnGet)
	ON_BN_CLICKED(IDC_CFG_EXALARMINPUT_BTN_SET, OnCfgAlarmBtnSet)
	ON_CBN_SELCHANGE(IDC_CFG_EXALARMINPUT_CMB_CHN, OnSelchangeCfgAlarmCmbChn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgExAlarmInput message handlers

BOOL CDlgCfgExAlarmInput::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_EXALARMINPUT);
	// TODO: Add extra initialization here
	if (!m_lLoginID)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_CFG_EXALARMINPUT), ConvertString("Prompt"));
		//EndDialog(0);
	}
	
	GetConfigFromDevice();
	InitDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgExAlarmInput::OnCfgAlarmBtnEventhandler() 
{
	// TODO: Add your control notification handler code here
	CSubDlgCfgEventHandler* pdlgEventHandler = new CSubDlgCfgEventHandler;
	if (pdlgEventHandler != NULL)
	{
		pdlgEventHandler->SetEventHandler(m_stuInfo.stuAlarmIn.stuEventHandler);
		pdlgEventHandler->SetConfigType(emConfigType_AlarmExAlarmInput);
		
		if (IDOK == pdlgEventHandler->DoModal())
		{
			const CFG_ALARM_MSG_HANDLE& stuEventHandler = pdlgEventHandler->GetEventHandler();
			memcpy(&m_stuInfo.stuAlarmIn.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
		}
		delete pdlgEventHandler;
		pdlgEventHandler = NULL;
	}
	else
	{
		MessageBox(ConvertString("new handler dialog error!", DLG_CFG_EXALARMINPUT), ConvertString("Prompt"));
	}
	
}

void CDlgCfgExAlarmInput::OnCfgAlarmBtnGet() 
{
	// TODO: Add your control notification handler code here	
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
}

void CDlgCfgExAlarmInput::OnCfgAlarmBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfigToDevice();		
}

//////////////////////////////////////////////////////////////////////////
// Private method

void CDlgCfgExAlarmInput::InitDlg(BOOL bShow /* = TRUE */)
{
	if (!bShow)
	{
		memset(&m_stuInfo.stuAlarmIn, 0, sizeof(m_stuInfo.stuAlarmIn));
	}

	int i = 0;

	// channel
	m_cmbChn.ResetContent();
	for (i = 0; i < MAX_EXALARM_CHANNEL_NUM; i++)
	{
		CString csItem;
		csItem.Format("%s %03d", ConvertString("Channel", DLG_CFG_EXALARMINPUT), i + 1);
		m_cmbChn.InsertString(-1, csItem);
	}
	m_cmbChn.SetCurSel(m_stuInfo.stuAlarmIn.nChannelID);

	// bypass
	m_chkByPass.SetCheck(m_stuInfo.stuAlarmIn.bEnable ? BST_UNCHECKED : BST_CHECKED);

	// name
	SetDlgItemText(IDC_CFG_EXALARMINPUT_EDT_NAME, m_stuInfo.stuAlarmIn.szChnName);

	// sensor method...to be
	m_cmbSensorMethod.ResetContent();
	for (i = 0; i < sizeof(stuDemoExAlarmSenseMethod) / sizeof(stuDemoExAlarmSenseMethod[0]); i++)
	{
		m_cmbSensorMethod.InsertString(-1, ConvertString(stuDemoExAlarmSenseMethod[i].pszName, DLG_CFG_EXALARMINPUT));
	}
	m_cmbSensorMethod.SetCurSel(int(m_stuInfo.stuAlarmIn.emSense));
}

BOOL CDlgCfgExAlarmInput::SetConfigToDevice()
{	
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_EXALARMINPUT, &m_stuInfo.stuAlarmIn, sizeof(m_stuInfo.stuAlarmIn), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet ExAlarmInput error..."), DLG_CFG_EXALARMINPUT), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif	// 0

		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_EXALARMINPUT, m_stuInfo.stuAlarmIn.nChannelID, szJsonBuf, 1024*40, &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig ExAlarmInput failed:", DLG_CFG_EXALARMINPUT), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig ExAlarmInput ok!"), DLG_CFG_EXALARMINPUT), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgExAlarmInput::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_EXALARMINPUT, nSelChn, szJsonBuf, 1024*40, &nerror, SDK_API_WAIT);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_EXALARMINPUT, szJsonBuf, (void*)&m_stuInfo.stuAlarmIn, sizeof(m_stuInfo.stuAlarmIn), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse ExAlarmInput error..."), DLG_CFG_EXALARMINPUT), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			// MessageBox(CString("GetConfig ExAlarmInput ok!"), ConvertString("Prompt"));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig ExAlarmInput error:", DLG_CFG_EXALARMINPUT), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgExAlarmInput::DlgToStu()
{
	m_stuInfo.stuAlarmIn.nChannelID = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
	m_stuInfo.stuAlarmIn.bEnable = m_chkByPass.GetCheck() ? FALSE : TRUE;
	GetDlgItemText(IDC_CFG_EXALARMINPUT_EDT_NAME, m_stuInfo.stuAlarmIn.szChnName, sizeof(m_stuInfo.stuAlarmIn.szChnName));
	m_stuInfo.stuAlarmIn.emSense = (EM_SENSE_METHOD)m_cmbSensorMethod.GetCurSel();
}

void CDlgCfgExAlarmInput::StuToDlg()
{
	m_cmbChn.SetCurSel(m_stuInfo.stuAlarmIn.nChannelID);
	m_chkByPass.SetCheck(m_stuInfo.stuAlarmIn.bEnable ? BST_UNCHECKED : BST_CHECKED);
	SetDlgItemText(IDC_CFG_EXALARMINPUT_EDT_NAME, m_stuInfo.stuAlarmIn.szChnName);
	m_cmbSensorMethod.SetCurSel((int)m_stuInfo.stuAlarmIn.emSense);
}

void CDlgCfgExAlarmInput::OnSelchangeCfgAlarmCmbChn() 
{
	// TODO: Add your control notification handler code here
	int nSel = m_cmbChn.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}

	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
}
