// DlgCfgRCEmergencyCall.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgRCEmergencyCall.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgRCEmergencyCall dialog


CDlgCfgRCEmergencyCall::CDlgCfgRCEmergencyCall(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgRCEmergencyCall::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgRCEmergencyCall)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_lLoginId = lLoginID;
    memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgRCEmergencyCall::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgRCEmergencyCall)
	DDX_Control(pDX, IDC_CFGRCEMERGENCYCALL_CMB_CHN, m_cmbChn);
	DDX_Control(pDX, IDC_CFGRCEMERGENCYCALL_CMB_MODE, m_cmbMode);
	DDX_Control(pDX, IDC_CFGRCEMERGENCYCALL_CMB_ALARMTYPE, m_cmbAlarmType);
	DDX_Control(pDX, IDC_CFGRCEMERGENCYCALL_CHK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgRCEmergencyCall, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgRCEmergencyCall)
	ON_BN_CLICKED(IDC_CFGRCEMERGENCYCALL_BTN_EVENTHANDLER, OnCfgRCEmergencyCallBtnEventhandler)
	ON_BN_CLICKED(IDC_CFGRCEMERGENCYCALL_BTN_GET, OnCfgRCEmergencyCallBtnGet)
	ON_BN_CLICKED(IDC_CFGRCEMERGENCYCALL_BTN_SET, OnCfgRCEmergencyCallBtnSet)
	ON_CBN_SELCHANGE(IDC_CFGRCEMERGENCYCALL_CMB_CHN, OnSelchangeCfgRCEmergencyCallCmbChn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgRCEmergencyCall private method

void CDlgCfgRCEmergencyCall::InitDlg()
{
    int i = 0;
    m_cmbChn.ResetContent();
    for (i = 0; i < 8; i++)
    {
        CString csItem;
        csItem.Format("%s %03d", ConvertString("Channel:", DLG_CFG_ALARM), i + 1);
        m_cmbChn.InsertString(-1, csItem);
    }
	m_cmbChn.SetCurSel(0);

    m_ckEnable.SetCheck(BST_UNCHECKED);

    m_cmbMode.ResetContent();
    for (i = 0; i < sizeof(stuDemoRCEMode)/sizeof(stuDemoRCEMode[0]); i++)
    {
        m_cmbMode.InsertString(-1, ConvertString(stuDemoRCEMode[i].szInfo, DLG_CFG_RCEMERGENCYCALL));
    }
    m_cmbMode.SetCurSel(0);

    m_cmbAlarmType.ResetContent();
    for (i = 0; i < sizeof(stuDemoRCEAlarmType)/sizeof(stuDemoRCEAlarmType[0]); i++)
    {
        m_cmbAlarmType.InsertString(-1, ConvertString(stuDemoRCEAlarmType[i].szInfo, DLG_CFG_RCEMERGENCYCALL));
    }
    m_cmbAlarmType.SetCurSel(0);
}

BOOL CDlgCfgRCEmergencyCall::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_RCEMERGENCY_CALL, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_RCEMERGENCY_CALL, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse RCEmergencyCall error..."), DLG_CFG_RCEMERGENCYCALL), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig RCEmergencyCall error:", DLG_CFG_RCEMERGENCYCALL), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgRCEmergencyCall::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};    
    BOOL bRet = CLIENT_PacketData(CFG_CMD_RCEMERGENCY_CALL, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet RCEmergencyCall error..."), DLG_CFG_RCEMERGENCYCALL), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
		int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_RCEMERGENCY_CALL, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig RCEmergencyCall failed:", DLG_CFG_RCEMERGENCYCALL), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig RCEmergencyCall ok!"), DLG_CFG_RCEMERGENCYCALL), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgRCEmergencyCall::showInfo()
{
    m_ckEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
    m_cmbMode.SetCurSel((int)m_stuInfo.emMode);
    m_cmbAlarmType.SetCurSel((int)m_stuInfo.emAlarmType);
    return TRUE;
}

BOOL CDlgCfgRCEmergencyCall::getInfo()
{
    m_stuInfo.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
    
    if (m_cmbMode.GetCurSel() == -1)
    {
        m_stuInfo.emMode = EM_CFG_RCEMERGENCY_MODE_UNKNOWN;
    }
    else
    {
        m_stuInfo.emMode = (EM_CFG_RCEMERGENCY_MODE_TYPE)m_cmbMode.GetCurSel();
    }
    
    if (m_cmbAlarmType.GetCurSel() == -1)
    {
        m_stuInfo.emAlarmType = EM_CFG_RCEMERGENCY_CALL_UNKNOWN;
    }
    else
    {
        m_stuInfo.emAlarmType = (EM_CFG_RCEMERGENCY_CALL_TYPE)m_cmbAlarmType.GetCurSel();
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgRCEmergencyCall message handlers

BOOL CDlgCfgRCEmergencyCall::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_RCEMERGENCYCALL);
	// TODO: Add extra initialization here
    InitDlg();
    if (GetConfigFromDevice())
    {
        showInfo();
	}	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgRCEmergencyCall::OnCfgRCEmergencyCallBtnEventhandler() 
{
	// TODO: Add your control notification handler code here
    CSubDlgCfgEventHandler* pdlgEventHandler = new CSubDlgCfgEventHandler;
    if (pdlgEventHandler != NULL)
    {
        pdlgEventHandler->SetEventHandler(m_stuInfo.stuEventHandler);
        
        if (IDOK == pdlgEventHandler->DoModal())
        {
            const CFG_ALARM_MSG_HANDLE& stuEventHandler = pdlgEventHandler->GetEventHandler();
            memcpy(&m_stuInfo.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
        }
        
        delete pdlgEventHandler;
        pdlgEventHandler = NULL;
    }
    else
    {
        MessageBox(ConvertString("new handler dialog error!", DLG_CFG_NETABORT), ConvertString("Prompt"));
	}
}

void CDlgCfgRCEmergencyCall::OnCfgRCEmergencyCallBtnGet() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgRCEmergencyCall::OnCfgRCEmergencyCallBtnSet() 
{
	// TODO: Add your control notification handler code here
    getInfo();
    SetConfigToDevice();
}

void CDlgCfgRCEmergencyCall::OnSelchangeCfgRCEmergencyCallCmbChn() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        showInfo();
	}
}
