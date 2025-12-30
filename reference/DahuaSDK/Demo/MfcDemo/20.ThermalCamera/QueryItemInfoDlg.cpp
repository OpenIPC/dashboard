// QueryItemInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "ThermalCameraDlg.h"
#include "QueryItemInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryItemInfoDlg dialog


CQueryItemInfoDlg::CQueryItemInfoDlg(CWnd* pParent /*=NULL*/,LLONG iLoginId,int nChannel)
	: CDialog(CQueryItemInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryItemInfoDlg)
	m_RuleID = 0;
	m_PresetID = 0;
	m_Measuretype = _T("unknown");
	m_MaxTemp = 0.0f;
	m_Unit = _T("unknown");
	m_AveTemp = 0.0f;
	m_MidTemp = 0.0f;
	m_MinTemp = 0.0f;
	m_StdTemp = 0.0f;
	//}}AFX_DATA_INIT
    m_iLoginID = iLoginId;
    m_nChannel = nChannel;
}


void CQueryItemInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryItemInfoDlg)
	DDX_Control(pDX, IDC_COMBO_METERTYPE, m_ComboMeterType);
	DDX_Text(pDX, IDC_EDIT_RULEID, m_RuleID);
	DDX_Text(pDX, IDC_EDIT_PRESETID, m_PresetID);
	DDX_Text(pDX, IDC_EDIT_MEASURETYPE, m_Measuretype);
	DDX_Text(pDX, IDC_EDIT_MAXTEMP, m_MaxTemp);
	DDX_Text(pDX, IDC_EDIT_UNIT, m_Unit);
	DDX_Text(pDX, IDC_EDIT_TEMP, m_AveTemp);
	DDX_Text(pDX, IDC_EDIT_MIDTEMP, m_MidTemp);
	DDX_Text(pDX, IDC_EDIT_MINTEMP, m_MinTemp);
	DDX_Text(pDX, IDC_EDIT_STDTEMP, m_StdTemp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryItemInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryItemInfoDlg)
	ON_BN_CLICKED(IDC_QUERYITEMINFO, OnQueryiteminfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryItemInfoDlg message handlers

void CQueryItemInfoDlg::OnQueryiteminfo() 
{
    UpdateData();
    NET_IN_RADIOMETRY_GETTEMPER stuRadioMetryInTemper = {sizeof(stuRadioMetryInTemper)};  
    NET_OUT_RADIOMETRY_GETTEMPER stuRadioMetryOutTemper = {sizeof(stuRadioMetryOutTemper)};
    stuRadioMetryInTemper.stCondition.nChannel = m_nChannel;
    stuRadioMetryInTemper.stCondition.nPresetId = m_PresetID;
    stuRadioMetryInTemper.stCondition.nRuleId = m_RuleID;
    int i = m_ComboMeterType.GetCurSel();    
    if (0 == i)
    {
        stuRadioMetryInTemper.stCondition.nMeterType = NET_RADIOMETRY_METERTYPE_SPOT;
    }
    else if (1 == i)
    {
        stuRadioMetryInTemper.stCondition.nMeterType = NET_RADIOMETRY_METERTYPE_LINE;
    }
    else if (2 == i)
    {
        stuRadioMetryInTemper.stCondition.nMeterType = NET_RADIOMETRY_METERTYPE_AREA;
    }
    BOOL ret = CLIENT_QueryDevInfo(m_iLoginID,NET_QUERY_DEV_RADIOMETRY_TEMPER,&stuRadioMetryInTemper,&stuRadioMetryOutTemper,NULL,3000);
    if (0 != ret)
    {
        if (NET_RADIOMETRY_METERTYPE_UNKNOWN == stuRadioMetryOutTemper.stTempInfo.nMeterType)
        {
            m_Measuretype = ConvertString("Unknown");
        }
        else if (NET_RADIOMETRY_METERTYPE_SPOT == stuRadioMetryOutTemper.stTempInfo.nMeterType)
        {
            m_Measuretype = ConvertString("Spot");
        }
        else if (NET_RADIOMETRY_METERTYPE_LINE == stuRadioMetryOutTemper.stTempInfo.nMeterType)
        {
            m_Measuretype = ConvertString("Line");
        }
        else if (NET_RADIOMETRY_METERTYPE_AREA == stuRadioMetryOutTemper.stTempInfo.nMeterType)
        {
            m_Measuretype = ConvertString("Area");
        }
        if (TEMPERATURE_UNIT_UNKNOWN == stuRadioMetryOutTemper.stTempInfo.nTemperUnit)
        {
            m_Unit = ConvertString("Unknown");
        }
        else if (TEMPERATURE_UNIT_CENTIGRADE == stuRadioMetryOutTemper.stTempInfo.nTemperUnit)
        {
            m_Unit = ConvertString("Celsius");
        }
        else if (TEMPERATURE_UNIT_FAHRENHEIT == stuRadioMetryOutTemper.stTempInfo.nTemperUnit)
        {
            m_Unit = ConvertString("Fahrenheit");
        }
        m_AveTemp = stuRadioMetryOutTemper.stTempInfo.fTemperAver;
        m_MaxTemp = stuRadioMetryOutTemper.stTempInfo.fTemperMax;
        m_MinTemp = stuRadioMetryOutTemper.stTempInfo.fTemperMin;
        m_MidTemp = stuRadioMetryOutTemper.stTempInfo.fTemperMid;
        m_StdTemp = stuRadioMetryOutTemper.stTempInfo.fTemperStd;
    }
	else
	{
		MessageBox(ConvertString("Query failed. Please check if the input information is incorrect."), ConvertString("Prompt"));
	}
    UpdateData(FALSE);
}

BOOL CQueryItemInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    g_SetWndStaticText(this);
    int nIndex = 0;
    CString strMeterType[3] = {ConvertString("Spot"),ConvertString("Line"),ConvertString("Area")};
    m_ComboMeterType.ResetContent();
    nIndex = m_ComboMeterType.AddString(strMeterType[0]);
    m_ComboMeterType.SetItemData(nIndex,SPOT);
    nIndex = m_ComboMeterType.AddString(strMeterType[1]);
    m_ComboMeterType.SetItemData(nIndex,LINE);
    nIndex = m_ComboMeterType.AddString(strMeterType[2]);
    m_ComboMeterType.SetItemData(nIndex,AREA);
    m_ComboMeterType.SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
