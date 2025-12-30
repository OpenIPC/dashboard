// DlgCfgAlarmPowerFault.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmPowerFault.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmPowerFault dialog


CDlgCfgAlarmPowerFault::CDlgCfgAlarmPowerFault(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgCfgAlarmPowerFault::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmPowerFault)
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	m_pstuPowerFault = NULL;
	m_nPowerIndex = 0;
}


void CDlgCfgAlarmPowerFault::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmPowerFault)
	DDX_Control(pDX, IDC_POWERFAULT_COMBO_POWER, m_cmbPower);
	DDX_Control(pDX, IDC_POWERFAULT_CHECK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmPowerFault, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmPowerFault)
	ON_BN_CLICKED(IDC_POWERFAULT_BUTTON_GET, OnPowerfaultButtonGet)
	ON_BN_CLICKED(IDC_POWERFAULT_BUTTON_SET, OnPowerfaultButtonSet)
	ON_BN_CLICKED(IDC_POWERFAULT_BUTTON_EVENTHANDLER, OnPowerfaultButtonEventhandler)
	ON_CBN_SELCHANGE(IDC_POWERFAULT_COMBO_POWER, OnSelchangePowerfaultComboPower)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmPowerFault message handlers

BOOL CDlgCfgAlarmPowerFault::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARM_POWERFAULT);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
	}

	m_pstuPowerFault= new CFG_POWERFAULT_INFO;
	if (m_pstuPowerFault == NULL)
	{
		return FALSE;
	}

	memset(m_pstuPowerFault, 0, sizeof(CFG_POWERFAULT_INFO));
	if (GetConfigFromDevice())
	{
		for (int i = 0; i < m_pstuPowerFault->nPowerCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %d", ConvertString("Power", DLG_ALARM_POWERFAULT), i + 1);
			m_cmbPower.InsertString(-1, csTemp);
		}
		m_cmbPower.SetCurSel(0);
		showInfo();
		return TRUE;
	}
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarmPowerFault::OnPowerfaultButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		showInfo();
	}
}

void CDlgCfgAlarmPowerFault::OnPowerfaultButtonSet() 
{
	// TODO: Add your control notification handler code here
	getInfo();
	SetConfigToDevice();
}

void CDlgCfgAlarmPowerFault::OnPowerfaultButtonEventhandler() 
{
	// TODO: Add your control notification handler code here
	// 先设置，后获取
	CSubDlgCfgEventHandler dlgEventHandler;
	dlgEventHandler.SetEventHandler(m_pstuPowerFault->stuPowerFault[m_nPowerIndex].stuEventHandler);
	/*dlgEventHandler.SetConfigType(emConfigType_AlarmPowerFault);*/
	if (IDOK == dlgEventHandler.DoModal())
	{
		const CFG_ALARM_MSG_HANDLE& stuEventHandler = dlgEventHandler.GetEventHandler();
		memcpy(&m_pstuPowerFault->stuPowerFault[m_nPowerIndex].stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
	}
}

BOOL CDlgCfgAlarmPowerFault::getInfo()
{
	m_pstuPowerFault->stuPowerFault[m_nPowerIndex].bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
	return TRUE;
}

BOOL CDlgCfgAlarmPowerFault::showInfo()
{
	m_ckEnable.SetCheck(m_pstuPowerFault->stuPowerFault[m_nPowerIndex].bEnable ? BST_CHECKED : BST_UNCHECKED);
	return TRUE;
}

BOOL CDlgCfgAlarmPowerFault::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = -1;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_POWERFAULT, nSelChn, 
		szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
	
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_POWERFAULT, szJsonBuf, (void*)m_pstuPowerFault, sizeof(CFG_POWERFAULT_INFO), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse PowerFault error..."), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig PowerFault error:", DLG_ALARM_POWERFAULT), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgAlarmPowerFault::SetConfigToDevice()
{
	char szJsonBuf[1024 * 40] = {0};

	BOOL bRet = CLIENT_PacketData(CFG_CMD_POWERFAULT, m_pstuPowerFault, 
		sizeof(CFG_POWERFAULT_INFO), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet PowerFault error..."), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_POWERFAULT, -1, szJsonBuf, 
			strlen(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig PowerFault failed:", DLG_ALARM_POWERFAULT), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig PowerFault ok!"), DLG_ALARM_POWERFAULT), ConvertString("Prompt"));
		}
	}
	return TRUE;
}


void CDlgCfgAlarmPowerFault::OnSelchangePowerfaultComboPower() 
{
	// TODO: Add your control notification handler code here
	m_nPowerIndex = m_cmbPower.GetCurSel();
}

void CDlgCfgAlarmPowerFault::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_pstuPowerFault != NULL)
	{
		delete m_pstuPowerFault;
	}
}
