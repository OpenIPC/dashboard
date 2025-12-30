// GlobalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "GlobalDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGlobalDlg dialog


CGlobalDlg::CGlobalDlg(CWnd* pParent /*=NULL*/,LLONG lLoginId)
	: CDialog(CGlobalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGlobalDlg)
	m_nHumidity = 0;
	m_fAtmosphericTemp = 0.0f;
	m_fCoefficient = 0.0f;
	m_nDistance = 0;
	m_fReflectedTemp = 0.0f;
	m_nSatTemp = 0;
	m_nMaxTemp = 0;
	m_nMidTemp = 0;
	m_nMinTemp = 0;
	m_nBottom = 0;
	m_nLeft = 0;
	m_nRight = 0;
	m_nTop = 0;
	m_nHighAlpha = 0;
	m_nHighBlue = 0;
	m_nHighGreen = 0;
	m_nHighRed = 0;
	m_nLowAlpha = 0;
	m_nLowBlue = 0;
	m_nLowGreen = 0;
	m_nLowRed = 0;
	//}}AFX_DATA_INIT
    m_iLoginID = lLoginId;
}


void CGlobalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGlobalDlg)
	DDX_Control(pDX, IDC_COMBO_UNIT, m_Combo_Unit);
	DDX_Control(pDX, IDC_COMBO_SWITCH, m_Combo_Switch);
	DDX_Control(pDX, IDC_COMBO_HOTSPOTFOLLOW, m_Combo_HotSpotFollow);
	DDX_Control(pDX, IDC_COMBO_COLORBAR, m_Combo_ColorBar);
	DDX_Control(pDX, IDC_COMBO_COLORBATCH, m_Combo_ColorBatch);
	DDX_Text(pDX, IDC_EDIT_RELATIVE_HUMIDITY, m_nHumidity);
	DDX_Text(pDX, IDC_EDIT_ATMOSPHERICTEMP, m_fAtmosphericTemp);
	DDX_Text(pDX, IDC_EDIT_COEFFICIENT, m_fCoefficient);
	DDX_Text(pDX, IDC_EDIT_DISTANCE, m_nDistance);
	DDX_Text(pDX, IDC_EDIT_RETEMP, m_fReflectedTemp);
	DDX_Text(pDX, IDC_EDIT_SATURATIONTEMP, m_nSatTemp);
	DDX_Text(pDX, IDC_EDIT_MAXTEMP, m_nMaxTemp);
	DDX_Text(pDX, IDC_EDIT_MIDTEMP, m_nMidTemp);
	DDX_Text(pDX, IDC_EDIT_MINTEMP, m_nMinTemp);
	DDX_Text(pDX, IDC_EDIT_COLORTEMP_BOTTOM, m_nBottom);
	DDX_Text(pDX, IDC_EDIT_COLORTEMP_LEFT, m_nLeft);
	DDX_Text(pDX, IDC_EDIT_COLORTEMP_RIGHT, m_nRight);
	DDX_Text(pDX, IDC_EDIT_COLORTEMP_TOP, m_nTop);
	DDX_Text(pDX, IDC_EDIT_HIGH_ALPHA, m_nHighAlpha);
	DDX_Text(pDX, IDC_EDIT_HIGH_BLUE, m_nHighBlue);
	DDX_Text(pDX, IDC_EDIT_HIGH_GREEN, m_nHighGreen);
	DDX_Text(pDX, IDC_EDIT_HIGH_RED, m_nHighRed);
	DDX_Text(pDX, IDC_EDIT_LOW_ALPHA, m_nLowAlpha);
	DDX_Text(pDX, IDC_EDIT_LOW_BLUE, m_nLowBlue);
	DDX_Text(pDX, IDC_EDIT_LOW_GREEN, m_nLowGreen);
	DDX_Text(pDX, IDC_EDIT_LOW_RED, m_nLowRed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGlobalDlg, CDialog)
	//{{AFX_MSG_MAP(CGlobalDlg)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGlobalDlg message handlers

BOOL CGlobalDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = 1;
    
    BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_THERMOMETRY,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        CFG_THERMOMETRY_INFO stuInfo = {0};
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_THERMOMETRY,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            return FALSE;
        }
        else
        {
            m_nHumidity = stuInfo.nRelativeHumidity;
            m_fAtmosphericTemp = stuInfo.fAtmosphericTemperature;
            m_fCoefficient = stuInfo.fObjectEmissivity;
            m_nDistance = stuInfo.nObjectDistance;
            m_fReflectedTemp = stuInfo.fReflectedTemperature;
            m_Combo_ColorBatch.SetCurSel(stuInfo.bIsothermEnable);
            m_Combo_Unit.SetCurSel(stuInfo.nTemperatureUnit);
            m_nMinTemp = stuInfo.nMinLimitTemp;
            m_nMidTemp = stuInfo.nMediumTemp;
            m_nMaxTemp = stuInfo.nMaxLimitTemp;
            m_nSatTemp = stuInfo.nSaturationTemp;
            m_nTop = stuInfo.stIsothermRect.nTop;
            m_nLeft = stuInfo.stIsothermRect.nLeft;
            m_nRight = stuInfo.stIsothermRect.nRight;
            m_nBottom = stuInfo.stIsothermRect.nBottom;
            m_Combo_ColorBar.SetCurSel(stuInfo.bColorBarDisplay);
            m_Combo_HotSpotFollow.SetCurSel(stuInfo.bHotSpotFollow);
            m_Combo_Switch.SetCurSel(stuInfo.bTemperEnable);
            m_nHighRed = stuInfo.stHighCTMakerColor.nRed;
            m_nHighBlue = stuInfo.stHighCTMakerColor.nBlue;
            m_nHighGreen = stuInfo.stHighCTMakerColor.nGreen;
            m_nHighAlpha = stuInfo.stHighCTMakerColor.nAlpha;
            m_nLowRed = stuInfo.stLowCTMakerColor.nRed;
            m_nLowBlue = stuInfo.stLowCTMakerColor.nBlue;
            m_nLowGreen = stuInfo.stLowCTMakerColor.nGreen;
            m_nLowAlpha = stuInfo.stLowCTMakerColor.nAlpha;
        }
    }
    else
    {
        return FALSE;
    }
	
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CGlobalDlg::OnBtnSet() 
{
    UpdateData();
    CFG_THERMOMETRY_INFO stuInfo = {0};
    stuInfo.nRelativeHumidity = m_nHumidity;
    stuInfo.fAtmosphericTemperature = m_fAtmosphericTemp;
    stuInfo.fObjectEmissivity = m_fCoefficient;
    stuInfo.nObjectDistance = m_nDistance;
    stuInfo.fReflectedTemperature = m_fReflectedTemp;
    stuInfo.bIsothermEnable = m_Combo_ColorBatch.GetCurSel();
    stuInfo.nTemperatureUnit = m_Combo_Unit.GetCurSel();
    stuInfo.nMinLimitTemp = m_nMinTemp;
    stuInfo.nMediumTemp = m_nMidTemp;
    stuInfo.nMaxLimitTemp = m_nMaxTemp;
    stuInfo.nSaturationTemp = m_nSatTemp;
    stuInfo.stIsothermRect.nTop = m_nTop;
    stuInfo.stIsothermRect.nLeft = m_nLeft;
    stuInfo.stIsothermRect.nRight = m_nRight;
    stuInfo.stIsothermRect.nBottom = m_nBottom;
    stuInfo.bColorBarDisplay = m_Combo_ColorBar.GetCurSel();
    stuInfo.bHotSpotFollow = m_Combo_HotSpotFollow.GetCurSel();
    stuInfo.bTemperEnable = m_Combo_Switch.GetCurSel();
    stuInfo.stHighCTMakerColor.nRed = m_nHighRed;
    stuInfo.stHighCTMakerColor.nBlue = m_nHighBlue;
    stuInfo.stHighCTMakerColor.nGreen = m_nHighGreen;
    stuInfo.stHighCTMakerColor.nAlpha = m_nHighAlpha;
    stuInfo.stLowCTMakerColor.nRed = m_nLowRed;
    stuInfo.stLowCTMakerColor.nBlue = m_nLowBlue;
    stuInfo.stLowCTMakerColor.nGreen = m_nLowGreen;
    stuInfo.stLowCTMakerColor.nAlpha = m_nLowAlpha;
    char szJsonBuf[1024 * 40] = {0};
    BOOL bRet = CLIENT_PacketData(CFG_CMD_THERMOMETRY, &stuInfo, sizeof(stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        int nChannelID = -1;
        bRet = CLIENT_SetNewDevConfig(m_iLoginID, CFG_CMD_THERMOMETRY, nChannelID, szJsonBuf, strlen(szJsonBuf), &nerror, &nrestart, 3000);
        if (!bRet)
        {
            
        }
        
    }
}

void CGlobalDlg::OnBtnGet() 
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = -1;
    
    BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_THERMOMETRY,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        CFG_THERMOMETRY_INFO stuInfo = {0};
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_THERMOMETRY,szJsonBuf,(char*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
            return ;
        }
        else
        {
            m_nHumidity = stuInfo.nRelativeHumidity;
            m_fAtmosphericTemp = stuInfo.fAtmosphericTemperature;
            m_fCoefficient = stuInfo.fObjectEmissivity;
            m_nDistance = stuInfo.nObjectDistance;
            m_fReflectedTemp = stuInfo.fReflectedTemperature;
            m_Combo_ColorBatch.SetCurSel(stuInfo.bIsothermEnable);
            m_Combo_Unit.SetCurSel(stuInfo.nTemperatureUnit);
            m_nMinTemp = stuInfo.nMinLimitTemp;
            m_nMidTemp = stuInfo.nMediumTemp;
            m_nMaxTemp = stuInfo.nMaxLimitTemp;
            m_nSatTemp = stuInfo.nSaturationTemp;
            m_nTop = stuInfo.stIsothermRect.nTop;
            m_nLeft = stuInfo.stIsothermRect.nLeft;
            m_nRight = stuInfo.stIsothermRect.nRight;
            m_nBottom = stuInfo.stIsothermRect.nBottom;
            m_Combo_ColorBar.SetCurSel(stuInfo.bColorBarDisplay);
            m_Combo_HotSpotFollow.SetCurSel(stuInfo.bHotSpotFollow);
            m_Combo_Switch.SetCurSel(stuInfo.bTemperEnable);
            m_nHighRed = stuInfo.stHighCTMakerColor.nRed;
            m_nHighBlue = stuInfo.stHighCTMakerColor.nBlue;
            m_nHighGreen = stuInfo.stHighCTMakerColor.nGreen;
            m_nHighAlpha = stuInfo.stHighCTMakerColor.nAlpha;
            m_nLowRed = stuInfo.stLowCTMakerColor.nRed;
            m_nLowBlue = stuInfo.stLowCTMakerColor.nBlue;
            m_nLowGreen = stuInfo.stLowCTMakerColor.nGreen;
            m_nLowAlpha = stuInfo.stLowCTMakerColor.nAlpha;
            
        }
    }
    else
    {
        MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
        return ;
    }
    
	UpdateData(FALSE);
}
