// DlgCfgAlarmBell.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgAlarmBell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmBell dialog


CDlgCfgAlarmBell::CDlgCfgAlarmBell(CWnd* pParent /* = NULL */, LLONG lLoginID /* = NULL */)
	: CDialog(CDlgCfgAlarmBell::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmBell)
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginID;

    memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgAlarmBell::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmBell)
	DDX_Control(pDX, IDC_CFG_ALARMBELL_CMB_CHN, m_cmbChn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmBell, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmBell)
	ON_BN_CLICKED(IDC_CFG_ALARMBELL_GET, OnCfgAlarmbellGet)
	ON_BN_CLICKED(IDC_CFG_ALARMBELL_SET, OnCfgAlarmbellSet)
	ON_CBN_SELCHANGE(IDC_CFG_ALARMBELL_CMB_CHN, OnSelchangeCfgAlarmbellCmbChn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmBell private method


void CDlgCfgAlarmBell::InitDlg()
{
    // channel
    m_cmbChn.ResetContent();
    for (int i = 0; i < 1; i++)
    {
        CString csChn;
        csChn.Format("%s %d", ConvertString("Channel", DLG_CFG_ALARMBELL), i + 1);
        m_cmbChn.InsertString(-1, csChn);
    }
    m_cmbChn.SetCurSel(0);

    // persistent time
    SetDlgItemInt(IDC_CFG_ALARMBELL_EDT_PERSISTENTTIME, 0);
}

BOOL CDlgCfgAlarmBell::SetConfigToDevice()
{	
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARMBELL, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet AlarmBell error..."), DLG_CFG_ALARMBELL), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{		
		int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
		int nerror = 0;
		int nrestart = 0;
		
		bRet = CLIENT_SetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ALARMBELL, nSelChn, szJsonBuf, 1024*40, &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig AlarmBell failed:", DLG_CFG_ALARMBELL), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig AlarmBell ok!"), DLG_CFG_ALARMBELL), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgAlarmBell::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_lLoginID, CFG_CMD_ALARMBELL, nSelChn, szJsonBuf, 1024*40, &nerror, SDK_API_WAIT);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ALARMBELL, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse AlarmBell error..."), DLG_CFG_ALARMBELL), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig AlarmBell error:", DLG_CFG_ALARMBELL),CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgAlarmBell::DlgToStu()
{
    m_stuInfo.nPersistentTime = GetDlgItemInt(IDC_CFG_ALARMBELL_EDT_PERSISTENTTIME);
}

void CDlgCfgAlarmBell::StuToDlg()
{
    SetDlgItemInt(IDC_CFG_ALARMBELL_EDT_PERSISTENTTIME, m_stuInfo.nPersistentTime);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmBell message handlers

BOOL CDlgCfgAlarmBell::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARMBELL);
	// TODO: Add extra initialization here
    InitDlg();
    if (GetConfigFromDevice())
    {
        StuToDlg();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarmBell::OnCfgAlarmbellGet() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        StuToDlg();
	}
}

void CDlgCfgAlarmBell::OnCfgAlarmbellSet() 
{
	// TODO: Add your control notification handler code here
    DlgToStu();
	SetConfigToDevice();
}

void CDlgCfgAlarmBell::OnSelchangeCfgAlarmbellCmbChn() 
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
