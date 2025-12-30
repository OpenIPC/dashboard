// DlgCfgAlarmOut.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmOut.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DlgCfgAlarmOut dialog


DlgCfgAlarmOut::DlgCfgAlarmOut(CWnd* pParent /* = NULL */, LLONG hLoginId /* = NULL */, UINT32 uiAlarmOut /* = 0 */)
	: CDialog(DlgCfgAlarmOut::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgCfgAlarmOut)
		// NOTE: the ClassWizard will add member initialization here
	m_hLoginId = hLoginId;
	m_uiAlarmOut = uiAlarmOut;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void DlgCfgAlarmOut::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgCfgAlarmOut)
	DDX_Control(pDX, IDC_ALARMOUT_CMB_OUTMODE, m_cbOutMode);
	DDX_Control(pDX, IDC_ALARMOUT_CMB_CHN, m_cbChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgCfgAlarmOut, CDialog)
	//{{AFX_MSG_MAP(DlgCfgAlarmOut)
	ON_BN_CLICKED(IDC_ALARMOUT_GET, OnAlarmoutGet)
	ON_BN_CLICKED(IDC_ALARMOUT_SET, OnAlarmoutSet)
	ON_CBN_SELCHANGE(IDC_ALARMOUT_CMB_CHN, OnSelchangeAlarmoutCmbChn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgCfgAlarmOut private method

void DlgCfgAlarmOut::InitDlg()
{
	int i = 0;
	
	// channel
	int nChannelNum = m_uiAlarmOut == 0 ? 16 : m_uiAlarmOut;
	for (i = 0; i < nChannelNum; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Channel", DLG_CFG_ALARMOUT), i + 1);
		m_cbChannel.InsertString(-1, csTemp);
	}
	m_cbChannel.SetCurSel(-1);

	// name
	SetDlgItemText(IDC_ALARMOUT_EDT_NAME, "");

	// output type
	SetDlgItemText(IDC_ALARMOUT_EDT_OUTTYPE, "");

	// output mode
	for (i = 0; i < sizeof(DemoOutputMode) / sizeof(DemoOutputMode[0]); i++)
	{
		m_cbOutMode.InsertString(-1, ConvertString(DemoOutputMode[i].pszName, DLG_CFG_ALARMOUT));
	}
	m_cbOutMode.SetCurSel(-1);
}

void DlgCfgAlarmOut::StuToDlg()
{
	int nSel = m_cbChannel.GetCurSel();
	if (-1 == nSel)
	{
		nSel = 0;
	}

	m_cbChannel.SetCurSel(nSel);
	SetDlgItemText(IDC_ALARMOUT_EDT_NAME, m_stuInfo.szChnName);
	SetDlgItemText(IDC_ALARMOUT_EDT_OUTTYPE, m_stuInfo.szOutputType);
	m_cbOutMode.SetCurSel(m_stuInfo.nOutputMode);
    SetDlgItemInt(IDC_ALARMOUT_EDT_PULSEDELAY, m_stuInfo.nPulseDelay);
}

void DlgCfgAlarmOut::DlgToStu()
{
	GetDlgItemText(IDC_ALARMOUT_EDT_NAME, m_stuInfo.szChnName, sizeof(m_stuInfo.szChnName));
	GetDlgItemText(IDC_ALARMOUT_EDT_OUTTYPE, m_stuInfo.szOutputType, sizeof(m_stuInfo.szOutputType));
	m_stuInfo.nOutputMode = m_cbOutMode.GetCurSel();
    m_stuInfo.nPulseDelay = GetDlgItemInt(IDC_ALARMOUT_EDT_PULSEDELAY);
}

BOOL DlgCfgAlarmOut::GetAlarmOutConfig()
{
// 	int nSelChn = m_cbChannel.GetCurSel();
// 	if (-1 == nSelChn)
// 	{
// 		return FALSE;
// 	}


	int nerror = 0;
	BOOL bSuccess = FALSE;
	BOOL bRet = FALSE;
	int nBufLen = 1024;
	char *szOutBuffer = new char[nBufLen];
	if (szOutBuffer == NULL)
	{
		return FALSE;	
	}
	memset(szOutBuffer, 0, sizeof(char) * nBufLen);
	
	int nSelChn = m_cbChannel.GetCurSel() == -1 ? 0 : m_cbChannel.GetCurSel();
	bSuccess = CLIENT_GetNewDevConfig((LLONG)m_hLoginId, CFG_CMD_ALARMOUT, nSelChn, szOutBuffer, 1024, &nerror, SDK_API_WAIT);
	if (bSuccess)
	{
		CFG_ALARMOUT_INFO stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ALARMOUT, szOutBuffer, &stuInfo, sizeof(stuInfo), &dwRet);
		if (bRet && sizeof(CFG_ALARMOUT_INFO) == dwRet)
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_ALARMOUT_INFO));
			//MessageBox("Get config of AlarmOut ok!", ConvertString("Prompt"));
		} 
		else
		{
			MessageBox(ConvertString("Parse AlarmOut error!",DLG_CFG_ALARMOUT), ConvertString("Prompt"));
			//return FALSE;
		}
	}
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query config of AlarmOut error:", DLG_CFG_ALARMOUT),CLIENT_GetLastError());
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

BOOL DlgCfgAlarmOut::SetAlarmOutConfig()
{
	int nSelChn = m_cbChannel.GetCurSel();
	int nSelMode = m_cbOutMode.GetCurSel();
	if (-1 == nSelChn || -1 == nSelMode)
	{
		return FALSE;
	}
		
	char szJson[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARMOUT, &m_stuInfo, sizeof(m_stuInfo), szJson, sizeof(szJson));
	if (bRet)
	{
		int nerror = 0;
		int nrestart = 0;
		bRet  = CLIENT_SetNewDevConfig((LLONG)m_hLoginId, CFG_CMD_ALARMOUT, nSelChn, szJson, 1024, &nerror, &nrestart, SDK_API_WAIT);	
		if (bRet)
		{
			if ( 1 == nrestart)
			{
				MessageBox(ConvertString("Set config of AlarmOut successfully! device need restart!", DLG_CFG_ALARMOUT), ConvertString("Prompt"));
			}
			else
			{
				MessageBox(ConvertString("Set config of AlarmOut successfully!", DLG_CFG_ALARMOUT), ConvertString("Prompt"));
			}
		} 
		else
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("Set config of AlarmOut error:", DLG_CFG_ALARMOUT), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
	} 
	else
	{
		MessageBox(ConvertString("Packet AlarmOut error!", DLG_CFG_ALARMOUT), ConvertString("Prompt"));
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// DlgCfgAlarmOut message handlers

BOOL DlgCfgAlarmOut::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARMOUT);

	// TODO: Add extra initialization here
	InitDlg();
	if (m_hLoginId)
	{
		m_cbChannel.SetCurSel(0);
		if (GetAlarmOutConfig())
		{
			StuToDlg();
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgCfgAlarmOut::OnAlarmoutGet() 
{
	// TODO: Add your control notification handler code here
	if (GetAlarmOutConfig())
	{
		StuToDlg();
	}
}

void DlgCfgAlarmOut::OnAlarmoutSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetAlarmOutConfig();
}

void DlgCfgAlarmOut::OnSelchangeAlarmoutCmbChn() 
{
	// TODO: Add your control notification handler code here	
	int nSelChn = m_cbChannel.GetCurSel();
	if (-1 == nSelChn)
	{
		return;
	}

	if (GetAlarmOutConfig())
	{
		StuToDlg();
	}
}
