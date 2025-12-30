// DlgCfgAirCondition.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgAirCondition.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAirCondition dialog


CDlgCfgAirCondition::CDlgCfgAirCondition(CWnd* pParent /*=NULL*/, LLONG lLoginId /*= 0*/)
	: CDialog(CDlgCfgAirCondition::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAirCondition)	
    //}}AFX_DATA_INIT
	m_nRange = 0;
	m_strAddr = "";
	m_strName = "";
	m_strBrand = "";
	m_lLoginID = lLoginId;
	m_nCurrentDevID = 0;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}

void CDlgCfgAirCondition::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAirCondition)
    DDX_Control(pDX, IDC_CFG_CMB_DEVICEID, m_cmbDevID);
    DDX_Text(pDX, IDC_CFG_AIRCONDITION_EDT_NAME, m_strName);
    DDX_Text(pDX, IDC_CFG_AIRCONDITION_EDT_BRAND, m_strBrand);
    DDX_Control(pDX, IDC_CFG_AIRCONDITON_CHECK_ON, m_ctlON);
    DDX_Text(pDX, IDC_CFG_AIRCONDITION_EDT_RANGE, m_nRange);
    DDX_Text(pDX, IDC_CFG_AIRCONDITION_EDT_COMMADDR, m_strAddr);
    DDX_Control(pDX, IDC_CFG_AIRCONDITION_CMB_MODE, m_cmbMode);
    DDX_Control(pDX, IDC_CFG_AIRCONDITION_CMB_WINDMODE, m_cmbWindMode);

		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAirCondition, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAirCondition)
	ON_CBN_EDITCHANGE(IDC_CFG_CMB_DEVICEID, OnEditchangeCfgCmbDeviceid)
	ON_BN_CLICKED(IDC_CFG_AIRCONDITION_BTN_GET, OnCfgAirconditionBtnGet)
	ON_BN_CLICKED(IDC_CFG_AIRCONDITION_BTN_SET, OnCfgAirconditionBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAirCondition message handlers

void CDlgCfgAirCondition::InitDlg()
{
    m_cmbMode.ResetContent();
    for (int i = 0; i < sizeof(stuDemoAirConditionMode) / sizeof(stuDemoAirConditionMode[0]); i++)
    {
        m_cmbMode.InsertString(-1, ConvertString(stuDemoAirConditionMode[i].pszName, DLG_CFG_AIRCONDITION));
    }
	m_cmbMode.SetCurSel(-1);

    m_cmbWindMode.ResetContent();
    for (int j = 0; j < sizeof(stuDemoWindMode) / sizeof(stuDemoWindMode[0]); j++)
    {
        m_cmbWindMode.InsertString(-1, ConvertString(stuDemoWindMode[j].pszName, DLG_CFG_AIRCONDITION));
    }
	m_cmbWindMode.SetCurSel(-1);

    //m_strName = NULL;
	SetDlgItemText(IDC_CFG_AIRCONDITION_EDT_NAME, NULL);
	SetDlgItemText(IDC_CFG_AIRCONDITION_EDT_BRAND, NULL);
	SetDlgItemInt(IDC_CFG_AIRCONDITION_EDT_RANGE, 0);
    SetDlgItemText(IDC_CFG_AIRCONDITION_EDT_COMMADDR, NULL);
	m_ctlON.SetCheck(BST_UNCHECKED);

}

void CDlgCfgAirCondition::StuToDlg()
{
	//m_cmbDevID.GetCurSel();
    if (0 > m_nCurrentDevID 
		|| m_stuInfo.nAirConditionNum <= m_nCurrentDevID
		|| MAX_AIRCONDITION_NUM <= m_nCurrentDevID)
    {
        return;
    }
    CFG_AIRCONDITION_DETAIL& stuAirCondition = m_stuInfo.stuAirConditions[m_nCurrentDevID];
	SetDlgItemText(IDC_CFG_AIRCONDITION_EDT_NAME, stuAirCondition.szName);
	SetDlgItemText(IDC_CFG_AIRCONDITION_EDT_BRAND, stuAirCondition.szBrand);
    SetDlgItemInt(IDC_CFG_AIRCONDITION_EDT_RANGE, stuAirCondition.nRange);
    CString strAddr;
    for (int i = 0; i < stuAirCondition.stuCommAddr.nAddressNum - 1; ++i)
    {
        CString strAddrTemp;
        strAddrTemp.Format("%d,", stuAirCondition.stuCommAddr.nAddress[i]);
        strAddr += strAddrTemp;
    }
    if (0 < stuAirCondition.stuCommAddr.nAddressNum)
    {
        CString strAddrTemp;
        strAddrTemp.Format("%d", stuAirCondition.stuCommAddr.nAddress[stuAirCondition.stuCommAddr.nAddressNum - 1]);
        strAddr += strAddrTemp;
    }
    SetDlgItemText(IDC_CFG_AIRCONDITION_EDT_COMMADDR, strAddr.GetBuffer(0));
	if (TRUE == m_stuInfo.stuAirConditions[m_nCurrentDevID].nState)
	{
		m_ctlON.SetCheck(BST_CHECKED);
	}
	else
	{
		m_ctlON.SetCheck(BST_UNCHECKED);
	}
    m_cmbMode.SetCurSel((int)stuAirCondition.emAirconditionMode - 1);
    m_cmbWindMode.SetCurSel((int)stuAirCondition.emAirconditionWindMode - 1);

}

void CDlgCfgAirCondition::DlgToStu()
{
    if (0 > m_nCurrentDevID 
		|| m_stuInfo.nAirConditionNum <= m_nCurrentDevID
		|| MAX_AIRCONDITION_NUM <= m_nCurrentDevID)
    {
        return;
    }
    CFG_AIRCONDITION_DETAIL& stuAirCondition = m_stuInfo.stuAirConditions[m_nCurrentDevID];
	GetDlgItemText(IDC_CFG_AIRCONDITION_EDT_NAME, stuAirCondition.szName, sizeof(stuAirCondition.szName) - 1);
    GetDlgItemText(IDC_CFG_AIRCONDITION_EDT_BRAND, stuAirCondition.szBrand, sizeof(stuAirCondition.szBrand) - 1);
    CString strAddr;
    GetDlgItemText(IDC_CFG_AIRCONDITION_EDT_COMMADDR, strAddr);
    // ´®¿ÚµØÖ·
    int& nAddrNum = stuAirCondition.stuCommAddr.nAddressNum;
    nAddrNum = 0;
    while (true)
    {
        CString strTemp = strAddr.SpanExcluding(",");
        if (MAX_ADDRESS_NUM <= nAddrNum || strTemp.IsEmpty())
        {
            break;
        }
        nAddrNum++;
        stuAirCondition.stuCommAddr.nAddress[nAddrNum - 1] = atoi(strTemp.GetBuffer(0));
        strAddr = strAddr.Right(strAddr.GetLength() - strTemp.GetLength() - 1);
        if (strAddr.IsEmpty())
        {
            break;
        }
    }
	stuAirCondition.nRange = GetDlgItemInt(IDC_CFG_AIRCONDITION_EDT_RANGE, NULL, TRUE);
	stuAirCondition.nState = m_ctlON.GetCheck() ? TRUE : FALSE;
    stuAirCondition.emAirconditionMode = (EM_CFG_AIRCONDITION_MODE)(m_cmbMode.GetCurSel() + 1);
    stuAirCondition.emAirconditionWindMode = (EM_CFG_AIRCONDITION_WINDMODE)(m_cmbWindMode.GetCurSel() +1);
}

BOOL CDlgCfgAirCondition::GetConfig()
{
	char szBuf[1024*32] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_AIRCONDITION, -1, szBuf, sizeof(szBuf), &nErr, SDK_API_WAIT);
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
    m_nCurrentDevID = 0;
	return TRUE;
}

BOOL CDlgCfgAirCondition::SetConfig()
{
	char szBuf[1024*32] = {0};
	int nErr = 0;
	int nRestart = 0;
	BOOL bRet = CLIENT_PacketData(CFG_CMD_AIRCONDITION, &m_stuInfo, sizeof(m_stuInfo), szBuf, sizeof(szBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet config of AirCondition error", DLG_CFG_AIRCONDITION), ConvertString("Prompt"));
		return FALSE;
	}
	bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_AIRCONDITION, -1, szBuf, strlen(szBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!bRet)
	{
		CString csInfo;
		csInfo.Format("%s:%08x", ConvertString("Set config of AirCondition error", DLG_CFG_AIRCONDITION), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}
	else
	{
		MessageBox(ConvertString("Set config of AirCondition ok", DLG_CFG_AIRCONDITION), ConvertString("Prompt"));
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAirCondition message handlers

BOOL CDlgCfgAirCondition::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_AIRCONDITION);
	// TODO: Add extra initialization here
	InitDlg();
	if (TRUE == GetConfig())
	{
		StuToDlg();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAirCondition::OnEditchangeCfgCmbDeviceid() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
    m_nCurrentDevID = m_cmbDevID.GetCurSel();
    StuToDlg();
}

void CDlgCfgAirCondition::OnCfgAirconditionBtnGet() 
{
	// TODO: Add your control notification handler code here
    if (TRUE == GetConfig())
    {
        StuToDlg();
	}
}

void CDlgCfgAirCondition::OnCfgAirconditionBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
    SetConfig();
}
