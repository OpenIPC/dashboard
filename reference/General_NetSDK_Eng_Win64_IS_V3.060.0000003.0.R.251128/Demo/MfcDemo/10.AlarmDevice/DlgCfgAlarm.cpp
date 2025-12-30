// DlgCfgAlarm.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarm.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarm dialog


CDlgCfgAlarm::CDlgCfgAlarm(CWnd* pParent /* = NULL */, LLONG hLoginID /* = NULL */, UINT32 uiAlarmIn /* = 0 */)
	: CDialog(CDlgCfgAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarm)
		// NOTE: the ClassWizard will add member initialization here
	m_uiAlarmIn = uiAlarmIn;
	//}}AFX_DATA_INIT

	m_hLoginID = hLoginID;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_bNeedResetWaittime = FALSE;
}


void CDlgCfgAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarm)
	DDX_Control(pDX, IDC_CFG_ALARM_CHK_LEVEL2, m_chkLevel2);
	DDX_Control(pDX, IDC_CFG_ALARM_EDT_NAME, m_edtName);
	DDX_Control(pDX, IDC_CFG_ALARM_EDT_ENA_DELAY, m_edtEnableDelay);
	DDX_Control(pDX, IDC_CFG_ALARM_EDT_DIS_DELAY, m_edtDisableDelay);
	DDX_Control(pDX, IDC_CFG_ALARM_CMB_SENSORTYPE, m_cmbSensorType);
	DDX_Control(pDX, IDC_CFG_ALARM_CMB_SENSORMETHOD, m_cmbSensorMethod);
	DDX_Control(pDX, IDC_CFG_ALARM_CMB_CHN, m_cmbChn);
	DDX_Control(pDX, IDC_CFG_ALARM_CMB_AREATYPE, m_cmbAreaType);
	DDX_Control(pDX, IDC_CFG_ALARM_CHK_BYPASS, m_chkByPass);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarm, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarm)
	ON_BN_CLICKED(IDC_CFG_ALARM_BTN_EVENTHANDLER, OnCfgAlarmBtnEventhandler)
	ON_BN_CLICKED(IDC_CFG_ALARM_BTN_GET, OnCfgAlarmBtnGet)
	ON_BN_CLICKED(IDC_CFG_ALARM_BTN_SET, OnCfgAlarmBtnSet)
	ON_CBN_SELCHANGE(IDC_CFG_ALARM_CMB_CHN, OnSelchangeCfgAlarmCmbChn)
	ON_BN_CLICKED(IDC_CFG_ALARM_CHK_LEVEL2, OnCfgAlarmChkLevel2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarm message handlers

BOOL CDlgCfgAlarm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARM);
	// TODO: Add extra initialization here
	if (m_hLoginID == 0)
	{
		MessageBox(ConvertString("We haven't login yet!", DLG_CFG_ALARM), ConvertString("Prompt"));
		//EndDialog(0);
	}
	
	InitDlg();
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarm::OnCfgAlarmBtnEventhandler() 
{
	// TODO: Add your control notification handler code here
	CSubDlgCfgEventHandler* pdlgEventHandler = new CSubDlgCfgEventHandler;
	if (pdlgEventHandler != NULL)
	{
		pdlgEventHandler->SetEventHandler(m_stuInfo.stuEventHandler);
		pdlgEventHandler->SetTimeSection(&m_stuInfo.stuTimeSection[0][0]);
		
		if (IDOK == pdlgEventHandler->DoModal())
		{
			m_bNeedResetWaittime = pdlgEventHandler->GetTiemChangeFlag();
			const CFG_ALARM_MSG_HANDLE& stuEventHandler = pdlgEventHandler->GetEventHandler();
			memcpy(&m_stuInfo.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
			
			for (int i = 0; i < WEEK_DAY_NUM; i++)
			{
				for (int j = 0; j < MAX_REC_TSECT; j++)
				{
					const CFG_TIME_SECTION* pstuTimeSection = pdlgEventHandler->GetTimeSection(i, j);
					if (pstuTimeSection)
					{
						memcpy(&m_stuInfo.stuTimeSection[i][j], pstuTimeSection, sizeof(CFG_TIME_SECTION));
					}
				}
			}
		}
		delete pdlgEventHandler;
		pdlgEventHandler = NULL;
	}
	else
	{
		MessageBox(ConvertString("new handler dialog error!", DLG_CFG_ALARM), ConvertString("Prompt"));
	}
	
}

void CDlgCfgAlarm::OnCfgAlarmBtnGet() 
{
	// TODO: Add your control notification handler code here	
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
}

void CDlgCfgAlarm::OnCfgAlarmBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfigToDevice();		
}

//////////////////////////////////////////////////////////////////////////
// Private method

void CDlgCfgAlarm::InitDlg(BOOL bShow /* = TRUE */)
{
	if (!bShow)
	{
		memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	}

	int i = 0;
	int nChannelNum = m_uiAlarmIn == 0 ? 16 : m_uiAlarmIn;

	// channel
	m_cmbChn.ResetContent();
	for (i = 0; i < nChannelNum; i++)
	{
		CString csItem;
		csItem.Format("%s %02d", ConvertString("Channel", DLG_CFG_ALARM), i + 1);
		m_cmbChn.InsertString(-1, csItem);
	}
	m_cmbChn.SetCurSel(m_stuInfo.nChannelID);

	// bypass
	m_chkByPass.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);

	// defence_area_type
	m_cmbAreaType.ResetContent();
	for (i = 0; i < sizeof(stuDemoAreaType) / sizeof(stuDemoAreaType[0]); i++)
	{
		m_cmbAreaType.InsertString(-1, ConvertString(stuDemoAreaType[i].pszName, DLG_CFG_ALARM));
	}
	m_cmbAreaType.SetCurSel((int)m_stuInfo.emDefenceAreaType);

	// disable delay
	SetDlgItemInt(IDC_CFG_ALARM_EDT_DIS_DELAY, m_stuInfo.nDisDelay, FALSE);

	// enable delay
	SetDlgItemInt(IDC_CFG_ALARM_EDT_ENA_DELAY, m_stuInfo.nEnableDelay, FALSE);

	// name
	SetDlgItemText(IDC_CFG_ALARM_EDT_NAME, m_stuInfo.szChnName);

	// sensor type: nc or no
	m_cmbSensorType.ResetContent();
	m_cmbSensorType.InsertString(-1, ConvertString("NC", DLG_CFG_ALARM));
	m_cmbSensorType.InsertString(-1, ConvertString("NO", DLG_CFG_ALARM));
	m_cmbSensorType.SetCurSel(m_stuInfo.nAlarmType);

	// sensor method...to be
	m_cmbSensorMethod.ResetContent();
	for (i = 0; i < sizeof(stuDemoSenseMethod) / sizeof(stuDemoSenseMethod[0]); i++)
	{
		m_cmbSensorMethod.InsertString(-1, ConvertString(stuDemoSenseMethod[i].pszName, DLG_CFG_ALARM));
	}
	m_cmbSensorMethod.SetCurSel(int(m_stuInfo.emSense));
	
	// slot

	// level1

	// level2
}

BOOL CDlgCfgAlarm::SetConfigToDevice()
{	
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARMINPUT, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet Alarm error..."), DLG_CFG_ALARM), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{		
		int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
		int nerror = 0;
		int nrestart = 0;
		
		bRet = CLIENT_SetNewDevConfig((LLONG)m_hLoginID, CFG_CMD_ALARMINPUT, nSelChn, szJsonBuf, 1024*40, &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig Alarm failed:", DLG_CFG_ALARM), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig Alarm ok!"), DLG_CFG_ALARM), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgAlarm::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_hLoginID, CFG_CMD_ALARMINPUT, nSelChn, szJsonBuf, 1024*40, &nerror, SDK_API_WAIT);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ALARMINPUT, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse Alarm error..."), DLG_CFG_ALARM), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig Alarm error:", DLG_CFG_ALARM), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgAlarm::DlgToStu()
{
	m_stuInfo.bEnable = m_chkByPass.GetCheck() ? TRUE : FALSE;
	m_stuInfo.emDefenceAreaType = (EM_CFG_DEFENCEAREATYPE)m_cmbAreaType.GetCurSel();
	m_stuInfo.nDisDelay = GetDlgItemInt(IDC_CFG_ALARM_EDT_DIS_DELAY);
	m_stuInfo.nEnableDelay = GetDlgItemInt(IDC_CFG_ALARM_EDT_ENA_DELAY);
	GetDlgItemText(IDC_CFG_ALARM_EDT_NAME, m_stuInfo.szChnName, sizeof(m_stuInfo.szChnName));
	m_stuInfo.nAlarmType = m_cmbSensorType.GetCurSel();
	m_stuInfo.emSense = (EM_SENSE_METHOD)m_cmbSensorMethod.GetCurSel();

	// slot
	m_stuInfo.nSlot = GetDlgItemInt(IDC_CFG_ALARM_EDT_SLOT);

	// level1
	m_stuInfo.nLevel1 = GetDlgItemInt(IDC_CFG_ALARM_EDT_LEVEL1);

	// level2
	if (m_chkLevel2.GetCheck())
	{
		m_stuInfo.abLevel2 = true;
		m_stuInfo.nLevel2 = GetDlgItemInt(IDC_CFG_ALARM_EDT_LEVEL2);
	}
	else
	{
		m_stuInfo.abLevel2 = false;
	}
}

void CDlgCfgAlarm::StuToDlg()
{
	m_chkByPass.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
	m_cmbAreaType.SetCurSel((int)m_stuInfo.emDefenceAreaType);
	SetDlgItemInt(IDC_CFG_ALARM_EDT_DIS_DELAY, (UINT)m_stuInfo.nDisDelay, FALSE);
	SetDlgItemInt(IDC_CFG_ALARM_EDT_ENA_DELAY, (UINT)m_stuInfo.nEnableDelay, FALSE);
	SetDlgItemText(IDC_CFG_ALARM_EDT_NAME, m_stuInfo.szChnName);
	m_cmbSensorType.SetCurSel((int)m_stuInfo.nAlarmType);
	m_cmbSensorMethod.SetCurSel((int)m_stuInfo.emSense);

	// slot
	SetDlgItemInt(IDC_CFG_ALARM_EDT_SLOT, m_stuInfo.nSlot);

	// level1
	SetDlgItemInt(IDC_CFG_ALARM_EDT_LEVEL1, m_stuInfo.nLevel1);

	// level2
	if (m_stuInfo.abLevel2)
	{
		m_chkLevel2.SetCheck(BST_CHECKED);
		SetDlgItemInt(IDC_CFG_ALARM_EDT_LEVEL2, m_stuInfo.nLevel2);
		GetDlgItem(IDC_CFG_ALARM_EDT_LEVEL2)->EnableWindow();
	}
	else
	{
		m_chkLevel2.SetCheck(BST_UNCHECKED);
		GetDlgItem(IDC_CFG_ALARM_EDT_LEVEL2)->EnableWindow(FALSE);
	}
}

void CDlgCfgAlarm::OnSelchangeCfgAlarmCmbChn() 
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

void CDlgCfgAlarm::OnCfgAlarmChkLevel2() 
{
	// TODO: Add your control notification handler code here
	if (m_chkLevel2.GetCheck())
	{
		GetDlgItem(IDC_CFG_ALARM_EDT_LEVEL2)->EnableWindow();
	}
	else
	{
		GetDlgItem(IDC_CFG_ALARM_EDT_LEVEL2)->EnableWindow(FALSE);
	}
}
