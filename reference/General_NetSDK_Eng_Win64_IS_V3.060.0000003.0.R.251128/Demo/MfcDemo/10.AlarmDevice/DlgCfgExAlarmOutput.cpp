// DlgCfgExAlarmOutput.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgExAlarmOutput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgExAlarmOutput dialog


CDlgCfgExAlarmOutput::CDlgCfgExAlarmOutput(CWnd* pParent /*=NULL*/, LLONG lLoginID /*= NULL*/)
	: CDialog(CDlgCfgExAlarmOutput::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgExAlarmOutput)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(CFG_EXALARMOUTPUT_INFO));
}


void CDlgCfgExAlarmOutput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgExAlarmOutput)
	DDX_Control(pDX, IDC_EXALARMOUTPUT_COMBO_CHANNEL, m_cbChannel);
	DDX_Control(pDX, IDC_EXALARMOUTPUT_COMBO_MODE, m_cmbMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgExAlarmOutput, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgExAlarmOutput)
	ON_BN_CLICKED(IDC_EXALARMOUTPUT_BTN_GET, OnExalarmoutputBtnGet)
	ON_BN_CLICKED(IDC_EXALARMOUTPUT_BTN_SET, OnExalarmoutputBtnSet)
	ON_CBN_SELCHANGE(IDC_EXALARMOUTPUT_COMBO_CHANNEL, OnSelchangeExalarmoutputComboChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgExAlarmOutput message handlers

BOOL CDlgCfgExAlarmOutput::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_EXALARMOUTPUT);
	
	// TODO: Add extra initialization here
	InitDlg();
	if (m_lLoginId != 0)
	{
// 		m_cbChannel.SetCurSel(0);
// 		if (GetAlarmOutConfig())
// 		{
// 			StuToDlg();
// 		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgExAlarmOutput::InitDlg()
{
	int i = 0;
	
	// channel
	for (i = 0; i < MAX_EXALARM_CHANNEL_NUM; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Channel", DLG_CFG_EXALARMOUTPUT), i + 1);
		m_cbChannel.InsertString(-1, csTemp);
	}
	m_cbChannel.SetCurSel(-1);
	
	// name
	SetDlgItemText(IDC_ALARMOUT_EDT_NAME, "");
	
	// output type
	SetDlgItemText(IDC_ALARMOUT_EDT_OUTTYPE, "");
	
	// output mode
	for (i = 0; i < sizeof(DemoExOutputMode) / sizeof(DemoExOutputMode[0]); i++)
	{
		m_cmbMode.InsertString(-1, ConvertString(DemoExOutputMode[i].pszName, DLG_CFG_EXALARMOUTPUT));
	}
	m_cmbMode.SetCurSel(-1);
}

void CDlgCfgExAlarmOutput::StuToDlg()
{
	SetDlgItemText(IDC_EXALARMOUTPUT_EDIT_NAME, m_stuInfo.szChnName);
	m_cmbMode.SetCurSel(m_stuInfo.nOutputMode);
}

void CDlgCfgExAlarmOutput::DlgToStu()
{
	GetDlgItemText(IDC_EXALARMOUTPUT_EDIT_NAME, m_stuInfo.szChnName, sizeof(m_stuInfo.szChnName));
	m_stuInfo.nOutputMode = m_cmbMode.GetCurSel();
}

BOOL CDlgCfgExAlarmOutput::GetAlarmOutConfig()
{
	int nerror = 0;
	BOOL bSuccess = FALSE;
	BOOL bRet = FALSE;
	char *szOutBuffer = new char[1024];
	if (szOutBuffer == NULL)
	{
		return FALSE;	
	}
	memset(szOutBuffer, 0, 1024);
	
	int nSelChn = m_cbChannel.GetCurSel() == -1 ? 0 : m_cbChannel.GetCurSel();
	bSuccess = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_EXALARMOUTPUT, nSelChn, szOutBuffer, 1024, &nerror, SDK_API_WAIT);
	if (bSuccess)
	{
		CFG_EXALARMOUTPUT_INFO stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CMD_EXALARMOUTPUT, szOutBuffer, &stuInfo, sizeof(stuInfo), &dwRet);
		if (bRet && sizeof(CFG_EXALARMOUTPUT_INFO) == dwRet)
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_EXALARMOUTPUT_INFO));
			//MessageBox("Get config of AlarmOut ok!", ConvertString("Prompt"));
		} 
		else
		{
			MessageBox(ConvertString("Parse AlarmOut error!", DLG_CFG_EXALARMOUTPUT), ConvertString("Prompt"));
			//return FALSE;
		}
	}
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query config of ExAlarmOut error:", DLG_CFG_EXALARMOUTPUT), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		//return FALSE;
	}

	if (szOutBuffer)
	{
		delete []szOutBuffer;
		szOutBuffer = NULL;
	}

	return bRet;
}

BOOL CDlgCfgExAlarmOutput::SetAlarmOutConfig()
{
	int nSelChn = m_cbChannel.GetCurSel();
	int nSelMode = m_cmbMode.GetCurSel();
	if (-1 == nSelChn || -1 == nSelMode)
	{
		return FALSE;
	}
	
	CFG_EXALARMOUTPUT_INFO stuInfo = {0};
	GetDlgItemText(IDC_EXALARMOUTPUT_EDIT_NAME, stuInfo.szChnName, sizeof(stuInfo.szChnName));
	stuInfo.nOutputMode = nSelMode;
	
	char szJson[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_EXALARMOUTPUT, &stuInfo, sizeof(stuInfo), szJson, sizeof(szJson));
	if (bRet)
	{
		int nerror = 0;
		int nrestart = 0;
		bRet  = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_EXALARMOUTPUT, nSelChn, szJson, 1024, &nerror, &nrestart, SDK_API_WAIT);	
		if (bRet)
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_EXALARMOUTPUT_INFO));
			MessageBox(ConvertString("Set config of AlarmOut successfully!", DLG_CFG_EXALARMOUTPUT), ConvertString("Prompt"));
		
		} 
		else
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("Set config of AlarmOut error:", DLG_CFG_EXALARMOUTPUT), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
	} 
	else
	{
		MessageBox(ConvertString("Packet AlarmOut error!", DLG_CFG_EXALARMOUTPUT), ConvertString("Prompt"));
		return FALSE;
	}

	return TRUE;
}

void CDlgCfgExAlarmOutput::OnSelchangeExalarmoutputComboChannel() 
{
	// TODO: Add your control notification handler code here
	GetAlarmOutConfig();
	StuToDlg();
}

void CDlgCfgExAlarmOutput::OnExalarmoutputBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetAlarmOutConfig())
	{
		StuToDlg();
	}
}

void CDlgCfgExAlarmOutput::OnExalarmoutputBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetAlarmOutConfig();
}
