// Control.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "Control.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CControl dialog


CControl::CControl(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CControl::IDD, pParent)
{
	//{{AFX_DATA_INIT(CControl)
	m_Channel = 0;
	m_PresetID = 0;
	m_PresetName = _T("Unknown");
	m_RuleID = 0;
	//}}AFX_DATA_INIT
    m_lLoginId = lLoginId;
}


void CControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CControl)
	DDX_Control(pDX, IDC_COMBO_METERTYPE, m_ComboMeterType);
	DDX_Text(pDX, IDC_EDIT_CHANNEL, m_Channel);
	DDX_Text(pDX, IDC_EDIT_PRESETID, m_PresetID);
	DDX_Text(pDX, IDC_EDIT_PRESETNAME, m_PresetName);
	DDX_Text(pDX, IDC_EDIT_RULEID, m_RuleID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CControl, CDialog)
	//{{AFX_MSG_MAP(CControl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CControl message handlers

void CControl::OnOK() 
{
    UpdateData(TRUE);
    BOOL ret = FALSE;
    NET_IN_RADIOMETRY_SETOSDMARK stuOSDIn = {sizeof(stuOSDIn)};
    stuOSDIn.dwSize = sizeof(stuOSDIn);
    stuOSDIn.stCondition.nPresetId = m_PresetID;
    stuOSDIn.stCondition.nRuleId = m_RuleID;
    stuOSDIn.stCondition.nChannel = 1;
    int i = m_ComboMeterType.GetCurSel();
    if (0 == i)
    {
        stuOSDIn.stCondition.nMeterType = NET_RADIOMETRY_METERTYPE_SPOT;
    }
    else if (1 == i)
    {
        stuOSDIn.stCondition.nMeterType = NET_RADIOMETRY_METERTYPE_LINE;
    }
    else if (2 == i)
    {
        stuOSDIn.stCondition.nMeterType = NET_RADIOMETRY_METERTYPE_AREA;
    }
    strncpy(stuOSDIn.stCondition.szName,m_PresetName, sizeof(stuOSDIn.stCondition.szName) - 1);
    NET_OUT_RADIOMETRY_SETOSDMARK stuOSDOut = {sizeof(stuOSDOut)};
    ret = CLIENT_ControlDeviceEx(m_lLoginId, DH_CTRL_RADIOMETRY_SETOSDMARK, &stuOSDIn, &stuOSDOut,3000);
	CDialog::OnOK();
}

BOOL CControl::OnInitDialog() 
{
	CDialog::OnInitDialog();
    InitDlg();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CControl::InitDlg()
{
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
}
