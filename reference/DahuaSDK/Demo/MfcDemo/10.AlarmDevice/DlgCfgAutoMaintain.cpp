// DlgCfgAutoMaintain.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAutoMaintain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAutoMaintain dialog


CDlgCfgAutoMaintain::CDlgCfgAutoMaintain(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgCfgAutoMaintain::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAutoMaintain)
	m_timeRebootTime = COleDateTime::GetCurrentTime();
	m_timeShutDownTime = COleDateTime::GetCurrentTime();
	m_timeShutUpTime = COleDateTime::GetCurrentTime();
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(AV_CFG_AutoMaintain));
	m_stuInfo.nStructSize = sizeof(AV_CFG_AutoMaintain);
}


void CDlgCfgAutoMaintain::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAutoMaintain)
	DDX_Control(pDX, IDC_AUTOMAINTAIN_CMB_STARTUPDAY, m_cmbStartUpDay);
	DDX_Control(pDX, IDC_AUTOMAINTAIN_CMB_SHUTDOWNDAY, m_cmbShutDownDay);
	DDX_Control(pDX, IDC_AUTOMAINTAIN_CMB_REBOOTDAY, m_cmbRebootDay);
	DDX_Control(pDX, IDC_AUTOMAINTAIN_CK_REBOOTENABLE, m_ckRebootEnable);
	DDX_DateTimeCtrl(pDX, IDC_AUTOMAINTAIN_DATETIMEPICKER_REBOORTIME, m_timeRebootTime);
	DDX_DateTimeCtrl(pDX, IDC_AUTOMAINTAIN_DATETIMEPICKER_SHUTDOWNTIME, m_timeShutDownTime);
	DDX_DateTimeCtrl(pDX, IDC_AUTOMAINTAIN_DATETIMEPICKER_STARTUPTIME, m_timeShutUpTime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAutoMaintain, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAutoMaintain)
	ON_BN_CLICKED(IDC_AUTOMAINTAIN_BTN_GET, OnAutomaintainBtnGet)
	ON_BN_CLICKED(IDC_AUTOMAINTAIN_BTN_SET, OnAutomaintainBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAutoMaintain message handlers

BOOL CDlgCfgAutoMaintain::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_AUTOMAINTAIN);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox("Please login first!", ConvertString("Prompt"));
		return FALSE;
	}
	int i = 0;
	for (i = 0; i < sizeof(stuAutoMianTainDay)/sizeof(stuAutoMianTainDay[0]); ++i)
	{
		m_cmbRebootDay.InsertString(-1, ConvertString(stuAutoMianTainDay[i].pszName, DLG_AUTOMAINTAIN));
		m_cmbStartUpDay.InsertString(-1, ConvertString(stuAutoMianTainDay[i].pszName, DLG_AUTOMAINTAIN));
		m_cmbShutDownDay.InsertString(-1, ConvertString(stuAutoMianTainDay[i].pszName, DLG_AUTOMAINTAIN));
	}
	m_cmbRebootDay.SetCurSel(0);
	m_cmbShutDownDay.SetCurSel(0);
	m_cmbStartUpDay.SetCurSel(0);
	m_ckRebootEnable.SetCheck(BST_UNCHECKED);
	m_timeRebootTime.SetTime(0, 0, 0);
	m_timeShutDownTime.SetTime(0, 0, 0);
	m_timeShutUpTime.SetTime(0, 0, 0);
	UpdateData(FALSE);
	OnAutomaintainBtnGet();

// 	// 设置回调接口
// 	CLIENT_SetDVRMessCallBack(AutoMainTainMessCallBack, (LDWORD)this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAutoMaintain::OnAutomaintainBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_AUTOMAINTAIN), ConvertString("Prompt"));
		return;
	}
	char szJsonBuf[1024] = {0};
	int nErr = 0, nRestart = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_AUTOMAINTAIN,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_AUTOMAINTAIN, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse AutoMaintain error..."), DLG_AUTOMAINTAIN), ConvertString("Prompt"));
			return ;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("get config AutoMaintain error:", DLG_AUTOMAINTAIN),CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	}
	StuToDlg();
}

void CDlgCfgAutoMaintain::OnAutomaintainBtnSet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox("Please login first!", ConvertString("Prompt"));
		return;
	}

	DlgToStu();
	char szJsonBuf[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_AUTOMAINTAIN, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet AutoMaintain error..."), DLG_AUTOMAINTAIN), ConvertString("Prompt"));
		return ;
	} 
	else
	{
		int nErr = 0, nRestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_AUTOMAINTAIN,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig AutoMainTain failed:", DLG_AUTOMAINTAIN), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return ;
		}
		else
		{
			MessageBox(ConvertString("set AutoMainTain config param OK!", DLG_AUTOMAINTAIN), ConvertString("Prompt"));
		}
	}
}

void CDlgCfgAutoMaintain::StuToDlg()
{
	m_ckRebootEnable.SetCheck(m_stuInfo.bAutoRebootEnable ? BST_CHECKED : BST_UNCHECKED);
	m_cmbRebootDay.SetCurSel(m_stuInfo.nAutoRebootDay + 1);
	m_timeRebootTime.SetTime(m_stuInfo.nAutoRebootHour, m_stuInfo.nAutoRebootMinute, 0);
	m_cmbShutDownDay.SetCurSel(m_stuInfo.nAutoShutdownDay + 1);
	m_timeShutDownTime.SetTime(m_stuInfo.nAutoShutdownHour, m_stuInfo.nAutoShutdownMinute, 0);
	m_cmbStartUpDay.SetCurSel(m_stuInfo.nAutoStartupDay + 1);
	m_timeShutUpTime.SetTime(m_stuInfo.nAutoStartupHour, m_stuInfo.nAutoStartupMinute, 0);
	UpdateData(FALSE);
}

void CDlgCfgAutoMaintain::DlgToStu()
{
	UpdateData(TRUE);
	m_stuInfo.bAutoRebootEnable = m_ckRebootEnable.GetCheck() ? TRUE : FALSE;
	m_stuInfo.nAutoRebootDay = m_cmbRebootDay.GetCurSel() - 1;
	m_stuInfo.nAutoRebootHour = m_timeRebootTime.GetHour();
	m_stuInfo.nAutoRebootMinute = m_timeRebootTime.GetMinute();
	m_stuInfo.nAutoShutdownDay = m_cmbShutDownDay.GetCurSel() - 1;
	m_stuInfo.nAutoShutdownHour = m_timeShutDownTime.GetHour();
	m_stuInfo.nAutoShutdownMinute = m_timeShutDownTime.GetMinute();
	m_stuInfo.nAutoStartupDay = m_cmbStartUpDay.GetCurSel() - 1;
	m_stuInfo.nAutoStartupHour = m_timeShutUpTime.GetHour();
	m_stuInfo.nAutoStartupMinute = m_timeShutUpTime.GetMinute();
}
