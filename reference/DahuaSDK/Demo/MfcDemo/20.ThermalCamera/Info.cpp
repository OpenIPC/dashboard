// Info.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "ThermalCameraDlg.h"
#include "Info.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfo dialog


CInfo::CInfo(CWnd* pParent /*=NULL*/, LLONG lLoginId,int nChannel)
	: CDialog(CInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfo)
	m_iBrightness = 0;
	m_iSharpness = 0;
	m_iEZoom = 0;
	m_iThermographyGamma = 0;
	m_Colorization = _T("");
	m_iSmartOptimizer = 0;
	m_iAgc = 0;
	m_iAgcMaxGain = 0;
	m_iAgcPlateau = 0;
	m_szSerialNum = _T("");
	m_szSoftVersion = _T("");
	m_szFirmVersion = _T("");
	m_szLibraryVersion = _T("");
	m_szOptRegion = _T("");
	m_szRoiType = _T("");
	m_Amout = 0;
	m_left = 0;
	m_top = 0;
	m_right = 0;
	m_bottom = 0;
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginId;
    m_nChannel = nChannel;
}


void CInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfo)
	DDX_Control(pDX, IDC_COMBO_REGIONNUM, m_ComboRegionNum);
	DDX_Text(pDX, IDC_EDIT_BRIGHTNESS, m_iBrightness);
	DDX_Text(pDX, IDC_EDIT_SHARPNESS, m_iSharpness);
	DDX_Text(pDX, IDC_EDIT_ZOOM, m_iEZoom);
	DDX_Text(pDX, IDC_EDIT_GAMMA, m_iThermographyGamma);
	DDX_Text(pDX, IDC_EDIT_COLOR, m_Colorization);
	DDX_Text(pDX, IDC_EDIT_OPTINDICATOR, m_iSmartOptimizer);
	DDX_Text(pDX, IDC_EDIT_GAINCTL, m_iAgc);
	DDX_Text(pDX, IDC_EDIT_MAXGAIN, m_iAgcMaxGain);
	DDX_Text(pDX, IDC_EDIT_GAINBALANCE, m_iAgcPlateau);
	DDX_Text(pDX, IDC_EDIT_SERIALNUM, m_szSerialNum);
	DDX_Text(pDX, IDC_SOFTVER, m_szSoftVersion);
	DDX_Text(pDX, IDC_EDIT_FIRMVER, m_szFirmVersion);
	DDX_Text(pDX, IDC_EDIT_LIBVER, m_szLibraryVersion);
	DDX_Text(pDX, IDC_EDIT_OPTREGION, m_szOptRegion);
	DDX_Text(pDX, IDC_EDIT_ROITYPE, m_szRoiType);
	DDX_Text(pDX, IDC_EDIT_AMOUNT, m_Amout);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_left);
	DDX_Text(pDX, IDC_EDIT_TOP, m_top);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_right);
	DDX_Text(pDX, IDC_EDIT_BOTTOM, m_bottom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfo, CDialog)
	//{{AFX_MSG_MAP(CInfo)
	ON_CBN_SELCHANGE(IDC_COMBO_REGIONNUM, OnSelchangeComboRegionnum)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfo message handlers

BOOL CInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();
    g_SetWndStaticText(this);
	OnGetPresetInfo();
    OnGetExternalSystemInfo();
    OnGetROIInfo();
    UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInfo::OnGetPresetInfo()
{
    NET_IN_THERMO_GET_PRESETINFO stuInGetPresetInfo = {sizeof(stuInGetPresetInfo)};
    stuInGetPresetInfo.dwSize = sizeof(stuInGetPresetInfo);
    stuInGetPresetInfo.nChannel = m_nChannel;
    stuInGetPresetInfo.emMode = NET_THERMO_MODE_DEFAULT;
    NET_OUT_THERMO_GET_PRESETINFO stuOutGetPresetInfo = {sizeof(stuOutGetPresetInfo)}; 
    BOOL ret = CLIENT_QueryDevInfo(m_lLoginID, NET_QUERY_DEV_THERMO_GRAPHY_PRESET, &stuInGetPresetInfo,&stuOutGetPresetInfo,NULL,1000);
    if (0 != ret)
    {
        m_iBrightness = stuOutGetPresetInfo.stInfo.nBrightness;
        m_iSharpness = stuOutGetPresetInfo.stInfo.nSharpness;
        m_iEZoom = stuOutGetPresetInfo.stInfo.nEZoom;
        m_iThermographyGamma = stuOutGetPresetInfo.stInfo.nThermographyGamma;
        if (stuOutGetPresetInfo.stInfo.nColorization == NET_THERMO_COLORIZATION_UNKNOWN)
        {
            m_Colorization = ConvertString("Unknown");
        }
        else if (stuOutGetPresetInfo.stInfo.nColorization == NET_THERMO_COLORIZATION_WHITE_HOT)
        {
            m_Colorization = ConvertString("WhiteHot");
        }
        else if (stuOutGetPresetInfo.stInfo.nColorization == NET_THERMO_COLORIZATION_BLACK_HOT)
        {
            m_Colorization = ConvertString("BlackHot");
        }
        else if (stuOutGetPresetInfo.stInfo.nColorization == NET_THERMO_COLORIZATION_IRONBOW2)
        {
            m_Colorization = ConvertString("IronBow");
        }
        else if (stuOutGetPresetInfo.stInfo.nColorization == NET_THERMO_COLORIZATION_ICEFIRE)
        {
            m_Colorization = ConvertString("Icefire");
        }
        m_iSmartOptimizer = stuOutGetPresetInfo.stInfo.nSmartOptimizer;
        m_iAgc = stuOutGetPresetInfo.stInfo.nAgc;
        m_iAgcMaxGain = stuOutGetPresetInfo.stInfo.nAgcMaxGain;
        m_iAgcPlateau = stuOutGetPresetInfo.stInfo.nAgcPlateau;
    }
    else
    {
        MessageBox(ConvertString("get info error"), ConvertString("Prompt"));
    }
}
void CInfo::OnGetROIInfo()
{
    NET_IN_THERMO_GET_OPTREGION stuInGetOptRegion = {sizeof(stuInGetOptRegion)};
    NET_OUT_THERMO_GET_OPTREGION stuOutGetOptRegion = {sizeof(stuOutGetOptRegion)};
    stuInGetOptRegion.nChannel = m_nChannel;
    BOOL ret = CLIENT_QueryDevInfo(m_lLoginID,NET_QUERY_DEV_THERMO_GRAPHY_OPTREGION,&stuInGetOptRegion,&stuOutGetOptRegion,NULL,1000);
    if (0 != ret)
    {
        if (0 == stuOutGetOptRegion.stInfo.bOptimizedRegion)
        {
            m_szOptRegion = ConvertString("Disable");
        }
        else
        {
            m_szOptRegion = ConvertString("Enable");
        }
        if (NET_THERMO_ROI_UNKNOWN == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Unknown");
        }
        else if (NET_THERMO_ROI_FULL_SCREEN == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Full-screen");
        }
        else if (NET_THERMO_ROI_SKY == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Top");
        }
        else if (NET_THERMO_ROI_HORIZONTAL == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Middle");
        }
        else if (NET_THERMO_ROI_GROUND == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Bottom");
        }
        else if (NET_THERMO_ROI_CENTER_75 == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Centerpoint 75%");
        }
        else if (NET_THERMO_ROI_CENTER_50 == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Centerpoint 50%");
        }
        else if (NET_THERMO_ROI_CENTER_25 == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Centerpoint 25%");
        }
        else if (NET_THERMO_ROI_CUSTOM == stuOutGetOptRegion.stInfo.nOptimizedROIType)
        {
            m_szRoiType = ConvertString("Customized");
            m_Amout = stuOutGetOptRegion.stInfo.nCustomRegion;
            int index = 0;
            for (int i=1;i<=m_Amout;i++)
            {
                CString str;
                str.Format("%d",i);
                index = m_ComboRegionNum.AddString(str);
                m_ComboRegionNum.SetItemData(index,i);
            }
            m_ComboRegionNum.SetCurSel(0);
            m_left = stuOutGetOptRegion.stInfo.stCustomRegions[0].nLeft;
            m_right = stuOutGetOptRegion.stInfo.stCustomRegions[0].nRight;
            m_top = stuOutGetOptRegion.stInfo.stCustomRegions[0].nTop;
            m_bottom = stuOutGetOptRegion.stInfo.stCustomRegions[0].nBottom;
        }
    }
}
void CInfo::OnGetExternalSystemInfo()
{
    NET_IN_THERMO_GET_EXTSYSINFO stuInGetExtsysInfo = {sizeof(stuInGetExtsysInfo)};
    NET_OUT_THERMO_GET_EXTSYSINFO stuOutGetExtsysInfo = {sizeof(stuOutGetExtsysInfo)};
    stuInGetExtsysInfo.nChannel = m_nChannel;
    BOOL ret = CLIENT_QueryDevInfo(m_lLoginID,NET_QUERY_DEV_THERMO_GRAPHY_EXTSYSINFO,&stuInGetExtsysInfo,&stuOutGetExtsysInfo,NULL,1000);
    if (0 != ret)
    {
        m_szSerialNum = stuOutGetExtsysInfo.stInfo.szSerialNumber;
        m_szFirmVersion = stuOutGetExtsysInfo.stInfo.szFirmwareVersion;
        m_szSoftVersion = stuOutGetExtsysInfo.stInfo.szSoftwareVersion;
        m_szLibraryVersion = stuOutGetExtsysInfo.stInfo.szLibVersion;
    }
}

void CInfo::OnSelchangeComboRegionnum() 
{
    NET_IN_THERMO_GET_OPTREGION stuInGetOptRegion = {sizeof(stuInGetOptRegion)};
    NET_OUT_THERMO_GET_OPTREGION stuOutGetOptRegion = {sizeof(stuOutGetOptRegion)};
    stuInGetOptRegion.nChannel = m_nChannel;
    BOOL ret = CLIENT_QueryDevInfo(m_lLoginID,NET_QUERY_DEV_THERMO_GRAPHY_OPTREGION,&stuInGetOptRegion,&stuOutGetOptRegion,NULL,1000);
    if (0 != ret)
    {
        int i = m_ComboRegionNum.GetCurSel();
        m_left = stuOutGetOptRegion.stInfo.stCustomRegions[i].nLeft;
        m_right = stuOutGetOptRegion.stInfo.stCustomRegions[i].nRight;
        m_top = stuOutGetOptRegion.stInfo.stCustomRegions[i].nTop;
        m_bottom = stuOutGetOptRegion.stInfo.stCustomRegions[i].nBottom;
    }
}

