// AirConditionOperate.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "AirConditionOperate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAirConditionOperate dialog


CAirConditionOperate::CAirConditionOperate(CWnd* pParent /*=NULL*/, LLONG lLoginId /*= 0*/)
	: CDialog(CAirConditionOperate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAirConditionOperate)
	m_lLoginID = lLoginId;
	m_fActualTemp = 0.0f;
    m_nAdjustTemp = 0;
	m_nTempState = 0;
    m_nTemp = 26;
	m_nWaitTime = SDK_API_WAIT;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	memset(&m_stuState, 0, sizeof(m_stuState));
	m_stuState.dwSize = sizeof(m_stuState);
	//}}AFX_DATA_INIT
}

void CAirConditionOperate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAirConditionOperate)
    DDX_Control(pDX, IDC_AIRCONDITION_CMB_DEVICEID, m_cmbDevID);
    DDX_Control(pDX, IDC_AIRCONDITION_CHECK_ON, m_ctlON);
    DDX_Control(pDX, IDC_AIRCONDITION_CHECK_ONLINE, m_ctlONLine);
	DDX_Text(pDX, IDC_AIRCONDITION_EDT_TEMP, m_nTempState);
    DDX_Control(pDX, IDC_AIRCONDITION_CMB_MODE, m_cmbModeState);
    DDX_Control(pDX, IDC_AIRCONDITION_CMB_WINDMODE, m_cmbWindModeState);
    DDX_Control(pDX, IDC_AIRCONDITION_CMB_SET_MODE, m_cmbMode);
    DDX_Control(pDX, IDC_AIRCONDITION_CMB_SET_WINDMODE, m_cmbWindMode);
	DDX_Text(pDX, IDC_AIRCONDITION_EDT_ACTUALTEMP, m_fActualTemp);
    DDX_Text(pDX, IDC_AIRCONDITION_EDT_SETTEMP, m_nTemp);
    DDX_Text(pDX, IDC_AIRCONDITION_EDT_ADJUSTTEMP, m_nAdjustTemp);
	DDX_Text(pDX, IDC_AIRCONDITION_EDT_TIME, m_nWaitTime);
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAirConditionOperate, CDialog)
	//{{AFX_MSG_MAP(CAirConditionOperate)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_OPEN, OnAirconditionBtnOpen)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_CLOSE, OnAirconditionBtnClose)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_MODE, OnAirconditionBtnMode)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_TEMP, OnAirconditionBtnTemp)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_WINDMODE, OnAirconditionBtnWindmode)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_ADJUST, OnAirconditionBtnAdjust)
	ON_BN_CLICKED(IDC_AIRCONDITION_BTN_GET_STATE, OnAirconditionBtnGetState)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAirConditionOperate message handlers

void CAirConditionOperate::InitDlg()
{
    m_cmbMode.ResetContent();
    int i = 0;
    for (i = 0; i < sizeof(stuDemoAirConditionMode) / sizeof(stuDemoAirConditionMode[0]); i++)
    {
        m_cmbMode.InsertString(-1, ConvertString(stuDemoAirConditionMode[i].pszName, DLG_CFG_AIRCONDITION));
    }
    m_cmbMode.SetCurSel(-1);
    
    m_cmbWindMode.ResetContent();
    for (i = 0; i < sizeof(stuDemoWindMode) / sizeof(stuDemoWindMode[0]); i++)
    {
        m_cmbWindMode.InsertString(-1, ConvertString(stuDemoWindMode[i].pszName, DLG_CFG_AIRCONDITION));
    }
    m_cmbWindMode.SetCurSel(-1);
    
    m_cmbModeState.ResetContent();
    for (i = 0; i < sizeof(stuDemoAirConditionMode) / sizeof(stuDemoAirConditionMode[0]); i++)
    {
        m_cmbModeState.InsertString(-1, ConvertString(stuDemoAirConditionMode[i].pszName, DLG_CFG_AIRCONDITION));
    }
    m_cmbModeState.SetCurSel(-1);
    
    m_cmbWindModeState.ResetContent();
    for (i = 0; i < sizeof(stuDemoWindMode) / sizeof(stuDemoWindMode[0]); i++)
    {
        m_cmbWindModeState.InsertString(-1, ConvertString(stuDemoWindMode[i].pszName, DLG_CFG_AIRCONDITION));
    }
    m_cmbWindModeState.SetCurSel(-1);
    
    m_ctlON.SetCheck(BST_UNCHECKED);
    m_ctlONLine.SetCheck(BST_UNCHECKED);
	UpdateData(FALSE);
}

BOOL CAirConditionOperate::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_AIRCONDITION_OPERATE);
	// TODO: Add extra initialization here
	InitDlg();
	if (TRUE == GetConfig())
	{
		GetState();
        StuToDlg();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAirConditionOperate::OnAirconditionBtnOpen() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
    int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return;
    }
    int nRetLen = 0;
    NET_CTRL_OPEN_AIRCONDITION stuOperate = {0};
    stuOperate.dwSize = sizeof(NET_CTRL_OPEN_AIRCONDITION);
    memcpy(stuOperate.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_AIRCONDITION_OPEN, &stuOperate, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Open AirCondition error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}

void CAirConditionOperate::OnAirconditionBtnClose() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
    int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return;
    }
    int nRetLen = 0;
    NET_CTRL_CLOSE_AIRCONDITION stuOperate = {0};
    stuOperate.dwSize = sizeof(NET_CTRL_CLOSE_AIRCONDITION);
    memcpy(stuOperate.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_AIRCONDITION_CLOSE, &stuOperate, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Close AirCondition error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
	
}

void CAirConditionOperate::OnAirconditionBtnMode() 
{
	// TODO: Add your control notification handler code here
    UpdateData(TRUE);
	int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return;
    }
    int nRetLen = 0;
    NET_CTRL_AIRCONDITION_SETMODE stuOperate = {0};
    stuOperate.dwSize = sizeof(NET_CTRL_AIRCONDITION_SETMODE);
	stuOperate.emAirconditionMode = (EM_AIRCONDITION_MODE)(m_cmbMode.GetCurSel() + 1);
    stuOperate.nTemperature = m_nTemp;
    memcpy(stuOperate.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_AIRCONDITION_SETMODE, &stuOperate, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Set Mode error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}

void CAirConditionOperate::OnAirconditionBtnTemp() 
{
	// TODO: Add your control notification handler code here
    UpdateData(TRUE);
    int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return;
    }
    int nRetLen = 0;
    NET_CTRL_SET_TEMPERATURE stuOperate = {0};
    stuOperate.dwSize = sizeof(NET_CTRL_SET_TEMPERATURE);
    memcpy(stuOperate.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    stuOperate.nTemperature = m_nTemp;
    BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_AIRCONDITION_SET_TEMPERATURE, &stuOperate, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Set Temperature error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}

void CAirConditionOperate::OnAirconditionBtnWindmode() 
{
	// TODO: Add your control notification handler code here
    int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return;
    }
    int nRetLen = 0;
    NET_CTRL_AIRCONDITION_SETWINDMODE stuOperate = {0};
    stuOperate.dwSize = sizeof(NET_CTRL_AIRCONDITION_SETWINDMODE);
    memcpy(stuOperate.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    stuOperate.emAirconditionWindMode = (EM_AIRCONDITION_WINDMODE)(m_cmbWindMode.GetCurSel() + 1);
    BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_AIRCONDITION_SETWINDMODE, &stuOperate, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Set WindMode error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}

void CAirConditionOperate::OnAirconditionBtnAdjust() 
{
	// TODO: Add your control notification handler code here
    UpdateData(TRUE);
    int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return;
    }
    int nRetLen = 0;
    NET_CTRL_ADJUST_TEMPERATURE stuOperate = {0};
    stuOperate.dwSize = sizeof(NET_CTRL_ADJUST_TEMPERATURE);
    memcpy(stuOperate.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    stuOperate.nTemperatureScale = m_nAdjustTemp;
    BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_AIRCONDITION_ADJUST_TEMPERATURE, &stuOperate, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Adjust Temperature error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}

void CAirConditionOperate::OnAirconditionBtnGetState() 
{
	// TODO: Add your control notification handler code here
	if (TRUE == GetState())
	{
	    StuToDlg();
	}
}

BOOL CAirConditionOperate::GetConfig()
{
	UpdateData(TRUE);
    char szBuf[1024*32] = {0};
    int nErr = 0;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_AIRCONDITION, 
        -1, szBuf, sizeof(szBuf), &nErr, m_nWaitTime);
    if (!bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Get config of AirCondition error", DLG_CFG_AIRCONDITION), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
        return FALSE;
    }
    
    int nRetLen = 0;
    bRet = CLIENT_ParseData(CFG_CMD_AIRCONDITION, szBuf, &m_stuInfo, sizeof(m_stuInfo), &nRetLen);
    if (!bRet || nRetLen != sizeof(m_stuInfo))
    {
        MessageBox(ConvertString("Parse config of AirCondition error", DLG_CFG_AIRCONDITION), ConvertString("Prompt"));
        return FALSE;
    }
    m_cmbDevID.ResetContent();
    for (int i = 0; i < m_stuInfo.nAirConditionNum; i++)
    {
        m_cmbDevID.InsertString(-1, m_stuInfo.stuAirConditions[i].szDeviceID);
    }
    m_cmbDevID.SetCurSel(0);
    return TRUE;
}

BOOL CAirConditionOperate::GetState()
{
	UpdateData(TRUE);
    int nCurrentDevID = m_cmbDevID.GetCurSel();
    if (0 > nCurrentDevID 
        || m_stuInfo.nAirConditionNum <= nCurrentDevID
        || MAX_AIRCONDITION_NUM <= nCurrentDevID)
    {
        return FALSE;
    }
    int nRetLen = 0;
    memset(&m_stuState, 0, sizeof(NET_GET_AIRCONDITION_STATE));
    m_stuState.dwSize = sizeof(NET_GET_AIRCONDITION_STATE);
    m_stuState.stuAirConditionState.dwSize = sizeof(NET_AIRCONDITION_STATE_INFO);

    memcpy(m_stuState.szDeviceID, m_stuInfo.stuAirConditions[nCurrentDevID].szDeviceID, MAX_DEVICE_ID_LEN);
    BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_AIRCONDITION_STATE, 
        (char*)&m_stuState,sizeof(m_stuState),&nRetLen, m_nWaitTime);
    if (FALSE == bRet)
    {
        CString csInfo;
        csInfo.Format("%s:%08x", ConvertString("Get AirCondition State error", DLG_AIRCONDITION_OPERATE), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
	return bRet;
}

void CAirConditionOperate::StuToDlg()
{
    NET_AIRCONDITION_STATE_INFO& stuAirConditionState = m_stuState.stuAirConditionState;
    m_nTempState = stuAirConditionState.nTemperature;
    m_fActualTemp = stuAirConditionState.fActualTemperature;
    m_ctlON.SetCheck(stuAirConditionState.bIsON);
    m_ctlONLine.SetCheck(stuAirConditionState.bIsOnline);
    
    m_cmbModeState.SetCurSel((int)stuAirConditionState.emAirconditionMode - 1);
    m_cmbWindModeState.SetCurSel((int)stuAirConditionState.emAirconditionWindMode - 1);
    UpdateData(FALSE);
}
