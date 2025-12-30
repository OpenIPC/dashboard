// Property.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "Property.h"
#include "GlobalDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProperty dialog


CProperty::CProperty(CWnd* pParent /*=NULL*/,LLONG lLoginId)
	: CDialog(CProperty::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProperty)
	m_nZoom = 0;
	m_nGamma = 0;
	m_nOptIndicator = 0;
	m_nAmount = 0;
	m_nAgc = 0;
	m_nAgcMaxGain = 0;
	m_nAgcPlateau = 0;
	m_nHighValue = 0;
	m_nLHRoi = 0;
	m_nHLRoi = 0;
	m_nLowValue = 0;
	m_nBottom = 0;
	m_nLeft = 0;
	m_nRight = 0;
	m_nTop = 0;
	//}}AFX_DATA_INIT
    m_iLoginID = lLoginId;
}


void CProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProperty)
	DDX_Control(pDX, IDC_COMBO_GAINMODE, m_Combo_GainMode);
	DDX_Control(pDX, IDC_COMBO_NUM, m_Combo_Num);
	DDX_Control(pDX, IDC_COMBO_ROITYPE, m_Combo_RoiType);
	DDX_Control(pDX, IDC_COMBO_OPTREGION, m_Combo_OptRegion);
	DDX_Control(pDX, IDC_COMBO_PSEUDOCOLOR, m_Combo_Pseudocolor);
	DDX_Text(pDX, IDC_EDIT_ZOOM, m_nZoom);
	DDX_Text(pDX, IDC_EDIT_GAMMA, m_nGamma);
	DDX_Text(pDX, IDC_EDIT_OPTINDICATOR, m_nOptIndicator);
	DDX_Text(pDX, IDC_EDIT_AMOUNT, m_nAmount);
	DDX_Text(pDX, IDC_EDIT_GAINCTL, m_nAgc);
	DDX_Text(pDX, IDC_EDIT_MAXGAIN, m_nAgcMaxGain);
	DDX_Text(pDX, IDC_EDIT_GAINBALANCE, m_nAgcPlateau);
	DDX_Text(pDX, IDC_EDIT_HIGHVALUE, m_nHighValue);
	DDX_Text(pDX, IDC_EDIT_LHROI, m_nLHRoi);
	DDX_Text(pDX, IDC_EDIT_HLROI, m_nHLRoi);
	DDX_Text(pDX, IDC_EDIT_LOWVALUE, m_nLowValue);
	DDX_Text(pDX, IDC_EDIT_BOTTOM, m_nBottom);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_nLeft);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_nRight);
	DDX_Text(pDX, IDC_EDIT_TOP, m_nTop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProperty, CDialog)
	//{{AFX_MSG_MAP(CProperty)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	ON_CBN_SELCHANGE(IDC_COMBO_NUM, OnSelchangeComboNum)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProperty message handlers

BOOL CProperty::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = 1;
    CString str;
    int index = 0;
    BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_THERMO_GRAPHY,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        CFG_THERMOGRAPHY_INFO stuInfo = {0};
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_THERMO_GRAPHY,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            m_nZoom = stuInfo.stOptions[0].nEZoom;
            m_nGamma = stuInfo.stOptions[0].nThermographyGamma;
            m_Combo_Pseudocolor.SetCurSel(stuInfo.stOptions[0].nColorization);
            m_nOptIndicator = stuInfo.stOptions[0].nSmartOptimizer;
            m_Combo_OptRegion.SetCurSel(stuInfo.stOptions[0].bOptimizedRegion);
            m_Combo_RoiType.SetCurSel(stuInfo.stOptions[0].nOptimizedROIType);
            m_nAmount = stuInfo.stOptions[0].nCustomRegion;
            if (stuInfo.stOptions[0].nOptimizedROIType == NET_THERMO_ROI_CUSTOM)
            {
                for (int i = 0;i<m_nAmount;++i)
                {
                    m_Combo_Num.ResetContent();
                    for (int i = 0;i<m_nAmount;++i)
                    {
                        index = 0;
                        str.Format("%d",i);
                        index = m_Combo_Num.AddString(str);
                        m_Combo_Num.SetItemData(index,i);
                    }
                }
                m_Combo_Num.SetCurSel(0);
                m_nLeft = stuInfo.stOptions[0].stCustomRegions[0].nLeft;
                m_nRight = stuInfo.stOptions[0].stCustomRegions[0].nRight;
                m_nTop = stuInfo.stOptions[0].stCustomRegions[0].nTop;
                m_nBottom = stuInfo.stOptions[0].stCustomRegions[0].nBottom;
            }
            m_nAgc = stuInfo.stOptions[0].stuLowTempGain.nAgc;
            m_nAgcMaxGain = stuInfo.stOptions[0].stuLowTempGain.nAgcMaxGain;
            m_nAgcPlateau = stuInfo.stOptions[0].stuLowTempGain.nAgcPlateau;
            m_Combo_GainMode.SetCurSel(stuInfo.stOptions[0].nGainMode);
            if (stuInfo.stOptions[0].nGainMode == CFG_THERMO_GAIN_MODE_AUTO)
            {
                m_nHighValue = stuInfo.stOptions[0].stAutoGain.nHighToLow;
                m_nHLRoi = stuInfo.stOptions[0].stAutoGain.nHLROI;
                m_nLHRoi = stuInfo.stOptions[0].stAutoGain.nLHROI;
                m_nLowValue = stuInfo.stOptions[0].stAutoGain.nLowToHigh;
            }
            else
            {
                m_nHighValue = 0;
                m_nHLRoi = 0;
                m_nLHRoi = 0;
                m_nLowValue = 0;
            }
        }
    }
    else
    {
        MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
    }
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProperty::OnOK() 
{
	
	
	CDialog::OnOK();
}

void CProperty::OnBtnGet() 
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = 1;
    CString str;
    int index = 0;
    BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_THERMO_GRAPHY,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        CFG_THERMOGRAPHY_INFO stuInfo = {0};
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_THERMO_GRAPHY,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            return ;
        }
        else
        {
            m_nZoom = stuInfo.stOptions[0].nEZoom;
            m_nGamma = stuInfo.stOptions[0].nThermographyGamma;
            m_Combo_Pseudocolor.SetCurSel(stuInfo.stOptions[0].nColorization);
            m_nOptIndicator = stuInfo.stOptions[0].nSmartOptimizer;
            m_Combo_OptRegion.SetCurSel(stuInfo.stOptions[0].bOptimizedRegion);
            m_Combo_RoiType.SetCurSel(stuInfo.stOptions[0].nOptimizedROIType);
            m_nAmount = stuInfo.stOptions[0].nCustomRegion;
            if (stuInfo.stOptions[0].nOptimizedROIType == NET_THERMO_ROI_CUSTOM)
            {
                m_Combo_Num.ResetContent();
                for (int i = 0;i<m_nAmount;++i)
                {
                    index = 0;
                    str.Format("%d",i);
                    index = m_Combo_Num.AddString(str);
                    m_Combo_Num.SetItemData(index,i);
                }
                m_Combo_Num.SetCurSel(0);
                m_nLeft = stuInfo.stOptions[0].stCustomRegions[0].nLeft;
                m_nRight = stuInfo.stOptions[0].stCustomRegions[0].nRight;
                m_nTop = stuInfo.stOptions[0].stCustomRegions[0].nTop;
                m_nBottom = stuInfo.stOptions[0].stCustomRegions[0].nBottom;
            }
            m_nAgc = stuInfo.stOptions[0].stuLowTempGain.nAgc;
            m_nAgcMaxGain = stuInfo.stOptions[0].stuLowTempGain.nAgcMaxGain;
            m_nAgcPlateau = stuInfo.stOptions[0].stuLowTempGain.nAgcPlateau;
            m_Combo_GainMode.SetCurSel(stuInfo.stOptions[0].nGainMode);
            if (stuInfo.stOptions[0].nGainMode == CFG_THERMO_GAIN_MODE_AUTO)
            {
                m_nHighValue = stuInfo.stOptions[0].stAutoGain.nHighToLow;
                m_nHLRoi = stuInfo.stOptions[0].stAutoGain.nHLROI;
                m_nLHRoi = stuInfo.stOptions[0].stAutoGain.nLHROI;
                m_nLowValue = stuInfo.stOptions[0].stAutoGain.nLowToHigh;
            }
            else
            {
                m_nHighValue = 0;
                m_nHLRoi = 0;
                m_nLHRoi = 0;
                m_nLowValue = 0;
            }
        }
    }
	UpdateData(FALSE);
}

void CProperty::OnSelchangeComboNum() 
{
    UpdateData();
	int i = m_Combo_Num.GetCurSel();
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = 1;
    CString str;
    int index = 0;
    BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_THERMO_GRAPHY,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        CFG_THERMOGRAPHY_INFO stuInfo = {0};
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_THERMO_GRAPHY,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            return ;
        }
        else
        {
            m_nLeft = stuInfo.stOptions[0].stCustomRegions[i].nLeft;
            m_nRight = stuInfo.stOptions[0].stCustomRegions[i].nRight;
            m_nTop = stuInfo.stOptions[0].stCustomRegions[i].nTop;
            m_nBottom = stuInfo.stOptions[0].stCustomRegions[i].nBottom;
        }
    }
	UpdateData(FALSE);
}

void CProperty::OnBtnSet() 
{
    UpdateData();
	CFG_THERMOGRAPHY_INFO stuInfo = {sizeof(stuInfo)};
    stuInfo.stOptions[0].nEZoom = m_nZoom;
    stuInfo.stOptions[0].nThermographyGamma = m_nGamma;
    stuInfo.stOptions[0].nColorization = m_Combo_Pseudocolor.GetCurSel();
    stuInfo.stOptions[0].nSmartOptimizer = m_nOptIndicator;
    stuInfo.stOptions[0].bOptimizedRegion = m_Combo_OptRegion.GetCurSel();
    stuInfo.stOptions[0].nOptimizedROIType = m_Combo_RoiType.GetCurSel();
    stuInfo.stOptions[0].nCustomRegion = m_nAmount;
    int i = m_Combo_Num.GetCurSel();
    stuInfo.stOptions[0].stCustomRegions[i].nLeft = m_nLeft;
    stuInfo.stOptions[0].stCustomRegions[i].nRight = m_nRight;
    stuInfo.stOptions[0].stCustomRegions[i].nTop = m_nTop;
    stuInfo.stOptions[0].stCustomRegions[i].nBottom = m_nBottom;
    stuInfo.stOptions[0].stuLowTempGain.nAgc = m_nAgc;
    stuInfo.stOptions[0].stuLowTempGain.nAgcMaxGain = m_nAgcMaxGain;
    stuInfo.stOptions[0].stuLowTempGain.nAgcPlateau = m_nAgcPlateau;
    stuInfo.stOptions[0].nGainMode = m_Combo_GainMode.GetCurSel();
    if (stuInfo.stOptions[0].nGainMode == CFG_THERMO_GAIN_MODE_AUTO)
    {
        stuInfo.stOptions[0].stAutoGain.nHighToLow = m_nHighValue;
        stuInfo.stOptions[0].stAutoGain.nHLROI = m_nHLRoi;
        stuInfo.stOptions[0].stAutoGain.nLHROI = m_nLHRoi;
        stuInfo.stOptions[0].stAutoGain.nLowToHigh = m_nLowValue;
    }
    char szJsonBuf[512 * 40] = {0};
    BOOL bRet = CLIENT_PacketData(CFG_CMD_THERMOMETRY,(LPVOID)&stuInfo, sizeof(stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        int nChannelID = -1;
        bRet = CLIENT_SetNewDevConfig(m_iLoginID, CFG_CMD_THERMOMETRY, nChannelID, szJsonBuf, 512*40, &nerror, &nrestart, 3000);
        if (!bRet)
        {
            
        }
    }
    UpdateData(FALSE);
}
