// DlgCfgAlarmSlotBond.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgAlarmSlotBond.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSlotBond dialog


CDlgCfgAlarmSlotBond::CDlgCfgAlarmSlotBond(CWnd* pParent /* = NULL */, LLONG lLoginID /* = NULL */)
	: CDialog(CDlgCfgAlarmSlotBond::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmSlotBond)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginID = lLoginID;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgAlarmSlotBond::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmSlotBond)
	DDX_Control(pDX, IDC_ALARMSLOTBOND_CMB_TYPE, m_cmbType);
	DDX_Control(pDX, IDC_ALARMSLOTBOND_CMB_INDEX, m_cmbIndex);
	DDX_Control(pDX, IDC_ALARMSLOTBOND_CMB_CHN, m_cmbChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmSlotBond, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmSlotBond)
	ON_BN_CLICKED(IDC_ALARMSLOTBOND_BTN_GET, OnAlarmslotbondBtnGet)
	ON_BN_CLICKED(IDC_ALARMSLOTBOND_BTN_SET, OnAlarmslotbondBtnSet)
	ON_CBN_SELCHANGE(IDC_ALARMSLOTBOND_CMB_CHN, OnSelchangeAlarmslotbondCmbChn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSlotBond private method

void CDlgCfgAlarmSlotBond::InitDlg()
{
	int i = 0;

	m_cmbChannel.ResetContent();
	for (i = 0; i < 256; i++)
	{
		CString csItem;
		csItem.Format("%s %03d", ConvertString("Channel", DLG_CFG_ALARM), i + 1);
		m_cmbChannel.InsertString(-1, csItem);
	}
	m_cmbChannel.SetCurSel(0);

	m_cmbType.ResetContent();
	for (i = 0; i < sizeof(stuDemoInterfaceType)/sizeof(stuDemoInterfaceType[0]); i++)
	{
		m_cmbType.InsertString(-1, ConvertString(stuDemoInterfaceType[i].szInfo, DLG_CFG_ALARMSLOTBOND));
	}
	m_cmbType.SetCurSel(0);

	m_cmbIndex.ResetContent();
	for (i = 0; i < 256; i++)
	{
		CString csItem;
		csItem.Format("%s %03d", ConvertString("Index", DLG_CFG_ALARMSLOTBOND), i + 1);
		m_cmbIndex.InsertString(-1, csItem);
	}
	m_cmbIndex.SetCurSel(0);
}

BOOL CDlgCfgAlarmSlotBond::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = m_cmbChannel.GetCurSel() == -1 ? 0 : m_cmbChannel.GetCurSel();
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_ALARM_SLOT_BOND, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_ALARM_SLOT_BOND, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse AlarmSlotBond error..."), DLG_CFG_ALARMSLOTBOND), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig AlarmSlotBond error:", DLG_CFG_ALARMSLOTBOND), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgAlarmSlotBond::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};    
    BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARM_SLOT_BOND, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet AlarmSlotBond error..."), DLG_CFG_ALARMSLOTBOND), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
		int nSelChn = m_cmbChannel.GetCurSel() == -1 ? 0 : m_cmbChannel.GetCurSel();
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_ALARM_SLOT_BOND, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig AlarmSlotBond failed:", DLG_CFG_ALARMSLOTBOND), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig AlarmSlotBond ok!"), DLG_CFG_ALARMSLOTBOND), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgAlarmSlotBond::showInfo()
{
	// type
	m_cmbType.SetCurSel((int)m_stuInfo.emType);

	// index
	m_cmbIndex.SetCurSel(m_stuInfo.nIndex);

    return TRUE;
}

BOOL CDlgCfgAlarmSlotBond::getInfo()
{
	// type
	m_stuInfo.emType = (EM_CFG_INTERFACE_TYPE)m_cmbType.GetCurSel();

	// index
	m_stuInfo.nIndex = m_cmbIndex.GetCurSel();

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSlotBond message handlers

BOOL CDlgCfgAlarmSlotBond::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARMSLOTBOND);
	// TODO: Add extra initialization here
	InitDlg();
	if (GetConfigFromDevice())
	{
		showInfo();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarmSlotBond::OnAlarmslotbondBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgAlarmSlotBond::OnAlarmslotbondBtnSet() 
{
	// TODO: Add your control notification handler code here
	getInfo();
    SetConfigToDevice();
}

void CDlgCfgAlarmSlotBond::OnSelchangeAlarmslotbondCmbChn() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
    {
        showInfo();
	}
}
