// ExposureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "imagetest.h"
#include "ExposureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExposureDlg dialog


CExposureDlg::CExposureDlg(CWnd* pParent /*=NULL*/,int nChannel, LLONG lLoginID)
	: CDialog(CExposureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExposureDlg)
	m_nCompensation = 0;
	m_nGainMax = 0;
	m_nGainMin = 0;
	m_bSlowShutter = FALSE;
	m_nSlowExposure = 0;
	m_nRecoveryTime = 0;
	m_nIrisMax = 0;
	m_nIrisMin = 0;
	m_bAuto = FALSE;
	m_nExposureIris = 0;
	m_fExposureMax = 0;
	m_fExposureMin = 0;
	m_fShutter = 0;
	m_nPhase = 0;
	m_nSlowSpeed = 0;
	m_nGain = 0;
	//}}AFX_DATA_INIT
    m_nChannel = nChannel;
    m_LoginID = lLoginID;
    memset(&m_stuExposure_Normal, 0, sizeof(m_stuExposure_Normal));
    m_stuExposure_Normal.dwSize = sizeof(m_stuExposure_Normal);
    memset(&m_stuExposure_Other, 0, sizeof(m_stuExposure_Other));
    m_stuExposure_Other.dwSize = sizeof(m_stuExposure_Other);
    memset(&m_stuExposure_Shutter , 0, sizeof(m_stuExposure_Shutter));
    m_stuExposure_Shutter.dwSize = sizeof(m_stuExposure_Shutter);
}


void CExposureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExposureDlg)
	DDX_Control(pDX, IDC_CMB_ANTIFLICKER, m_Cmb_AntiFlicker);
	DDX_Control(pDX, IDC_CMB_EXPOSURE, m_Cmb_Exposure);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_Cmb_Type);
	DDX_Text(pDX, IDC_EDIT_COMPAENSATION, m_nCompensation);
	DDX_Text(pDX, IDC_EDIT_GAINMAX, m_nGainMax);
	DDX_Text(pDX, IDC_EDIT_GAINMIN, m_nGainMin);
	DDX_Check(pDX, IDC_CHECK_SLOWSHUTTER, m_bSlowShutter);
	DDX_Text(pDX, IDC_EDIT_SLOWEEXPOSURE, m_nSlowExposure);
	DDX_Text(pDX, IDC_EDIT_RECOVERYTIME, m_nRecoveryTime);
	DDX_Text(pDX, IDC_EDIT_IRISMAX, m_nIrisMax);
	DDX_Text(pDX, IDC_EDIT_IRISMIN, m_nIrisMin);
	DDX_Check(pDX, IDC_CHECK_AUTOSYNCPHASE, m_bAuto);
	DDX_Text(pDX, IDC_EDIT_EXPOSUREIRIS, m_nExposureIris);
	DDX_Text(pDX, IDC_EDIT_EXPOSUREMAX, m_fExposureMax);
	DDX_Text(pDX, IDC_EDIT_EXPOSUREMIN, m_fExposureMin);
	DDX_Text(pDX, IDC_EDIT_SHUTTER, m_fShutter);
	DDX_Text(pDX, IDC_EDIT_PHASE, m_nPhase);
	DDX_Text(pDX, IDC_EDIT_SLOWSPEED, m_nSlowSpeed);
	DDX_Text(pDX, IDC_EDIT_GAIN, m_nGain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExposureDlg, CDialog)
	//{{AFX_MSG_MAP(CExposureDlg)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnSelchangeComboType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExposureDlg message handlers

BOOL CExposureDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
/*
		白天
		夜晚
		普通
		
		默认自动
		低噪声
		防拖影
		手动区间
		光圈优先
		手动固定
		增益优先
		快门优先
		闪光灯匹配
		
		室外
		50Hz防闪烁
		60Hz防闪烁
*/


	int nIndex; 
	int i = 0; 
	CString ConfigMode[3]={"day","night","normal"}; 
	m_Cmb_Type.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_Type.AddString(ConvertString(ConfigMode[i])); 
		m_Cmb_Type.SetItemData(nIndex,i+1); 
	} 

	
	CString ExposureMode[9]={"default(auto)","low noise","anti-streak","manual interval","aperture first","manual fixed","gain first","shutter first","flash lamp match"}; 
	m_Cmb_Exposure.ResetContent(); 
	for(i=0;i<9;i++) 
	{ 
		nIndex = m_Cmb_Exposure.AddString(ConvertString(ExposureMode[i])); 
		m_Cmb_Exposure.SetItemData(nIndex,i+1); 
	} 

	
	CString AntiFlicker[3]={"outdoor","Anti-glare(50hz)","Anti-glare(60hz)"}; 
	m_Cmb_AntiFlicker.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_AntiFlicker.AddString(ConvertString(AntiFlicker[i])); 
		m_Cmb_AntiFlicker.SetItemData(nIndex,i+1); 
	} 


	m_Cmb_Type.SetCurSel(0);
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL,NET_EM_CONFIG_DAYTIME))
    {
        StuToDlg();
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER,NET_EM_CONFIG_DAYTIME))
    {
        StuToDlg();
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER,NET_EM_CONFIG_DAYTIME))
    {
        StuToDlg();
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExposureDlg::StuToDlg()
{
    if (m_stuExposure_Normal.emExposureMode>2)
    {
        m_Cmb_Exposure.SetCurSel(m_stuExposure_Normal.emExposureMode - 1);
    }
    else
    {
        m_Cmb_Exposure.SetCurSel(m_stuExposure_Normal.emExposureMode);
    }
    m_Cmb_AntiFlicker.SetCurSel(m_stuExposure_Normal.nAntiFlicker);
    m_nCompensation = m_stuExposure_Normal.nCompensation;
    m_nGainMax = m_stuExposure_Normal.nGainMax;
    m_nGainMin = m_stuExposure_Normal.nGainMin;
    m_nExposureIris = m_stuExposure_Normal.nExposureIris;
    m_nGain = m_stuExposure_Normal.nGain;

    m_bSlowShutter = m_stuExposure_Other.bSlowShutter;
    m_nSlowExposure = m_stuExposure_Other.nSlowAutoExposure;
    m_nRecoveryTime = m_stuExposure_Other.nRecoveryTime;
    m_nIrisMax = m_stuExposure_Other.nIrisMax;
    m_nIrisMin = m_stuExposure_Other.nIrisMin;
    m_nSlowSpeed = m_stuExposure_Other.nSlowSpeed;

    m_bAuto = m_stuExposure_Shutter.bAutoSyncPhase;
    m_fShutter = m_stuExposure_Shutter.fShutter;
    m_fExposureMax = m_stuExposure_Normal.dbExposureValue2;
    m_fExposureMin = m_stuExposure_Normal.dbExposureValue1;
    m_nPhase = m_stuExposure_Shutter.nPhase;

    UpdateData(FALSE);

}
void CExposureDlg::DlgToStu()
{
    UpdateData();
    int i = m_Cmb_Exposure.GetCurSel();
    if (i>2)
    {
        i++;
    }
    m_stuExposure_Normal.emExposureMode = (NET_EM_EXPOSURE_MODE)i;
    m_stuExposure_Normal.nAntiFlicker = m_Cmb_AntiFlicker.GetCurSel();
    m_stuExposure_Normal.nCompensation = m_nCompensation;
    m_stuExposure_Normal.nGainMax = m_nGainMax;
    m_stuExposure_Normal.nGainMin = m_nGainMin;
    m_stuExposure_Normal.nExposureIris = m_nExposureIris;
    m_stuExposure_Normal.nGain = m_nGain;

    m_stuExposure_Other.bSlowShutter = m_bSlowShutter;
    m_stuExposure_Other.nSlowAutoExposure = m_nSlowExposure;
    m_stuExposure_Other.nRecoveryTime = m_nRecoveryTime;
    m_stuExposure_Other.nIrisMax = m_nIrisMax;
    m_stuExposure_Other.nIrisMin = m_nIrisMin;
    m_stuExposure_Other.nSlowSpeed = m_nSlowSpeed;

    m_stuExposure_Shutter.bAutoSyncPhase = m_bAuto;
    m_stuExposure_Shutter.fShutter = m_fShutter;
    m_stuExposure_Normal.dbExposureValue1 = m_fExposureMin;
    m_stuExposure_Normal.dbExposureValue2 = m_fExposureMax;
    m_stuExposure_Shutter.nPhase = m_nPhase;
}

BOOL CExposureDlg::GetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperate, NET_EM_CONFIG_TYPE emConfigType)
{
    BOOL bRet = FALSE;
    if (emOperate == NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL)
    {
        m_stuExposure_Normal.emCfgType = emConfigType;
        bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL,m_nChannel,&m_stuExposure_Normal,sizeof(m_stuExposure_Normal),TIMEOUT);
        if (!bRet)
        {   
            MessageBox(ConvertString("Get exposure config failed!"), ConvertString("Prompt"));
        }
        return bRet;
    }
    else if (emOperate == NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER)
    {
        m_stuExposure_Other.emCfgType = emConfigType;
        bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER,0,&m_stuExposure_Other,sizeof(m_stuExposure_Other),TIMEOUT);
        if (!bRet)
        {   
            MessageBox(ConvertString("Get exposure other config failed!"), ConvertString("Prompt"));
        }
        return bRet;
    }
    else if (emOperate == NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER)
    {
        bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER,m_nChannel,&m_stuExposure_Shutter,sizeof(m_stuExposure_Shutter),TIMEOUT);
        if (!bRet)
        {   
            MessageBox(ConvertString("Get exposure Shutter config failed!"), ConvertString("Prompt"));
        }
        return bRet;
    }
    else
    {
        return bRet;
    }
}
BOOL CExposureDlg::SetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperate, NET_EM_CONFIG_TYPE emConfigType)
{
    BOOL bRet = FALSE;
    if (emOperate == NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL)
    {
        m_stuExposure_Normal.emCfgType = emConfigType;
        bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL,m_nChannel,&m_stuExposure_Normal,sizeof(m_stuExposure_Normal),TIMEOUT);
        if (!bRet)
        {   
            MessageBox(ConvertString("Set exposure config failed!"), ConvertString("Prompt"));
        }
        return bRet;
    }
    else if (emOperate == NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER)
    {
        m_stuExposure_Other.emCfgType = emConfigType;
        bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER,m_nChannel,&m_stuExposure_Other,sizeof(m_stuExposure_Other),TIMEOUT);
        if (!bRet)
        {   
            MessageBox(ConvertString("Set exposure other config failed!"), ConvertString("Prompt"));
        }
        return bRet;
    }
    else if (emOperate == NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER)
    {
        bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER,m_nChannel,&m_stuExposure_Shutter,sizeof(m_stuExposure_Shutter),TIMEOUT);
        if (!bRet)
        {   
            MessageBox(ConvertString("Set exposure Shutter config failed!"), ConvertString("Prompt"));
        }
        return bRet;
    }
    else
    {
        return bRet;
    }
}

void CExposureDlg::OnGet() 
{
    int i = m_Cmb_Type.GetCurSel();
	if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL,NET_EM_CONFIG_TYPE(i)))
	{
        StuToDlg();
	}
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
}

void CExposureDlg::OnSet() 
{
	int i = m_Cmb_Type.GetCurSel();
    DlgToStu();
    SetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL, NET_EM_CONFIG_TYPE(i));
    SetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER, NET_EM_CONFIG_TYPE(i));
    SetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER, NET_EM_CONFIG_TYPE(i));
}

void CExposureDlg::OnSelchangeComboType() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_NORMAL,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_OTHER,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_EXPOSURE_SHUTTER,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
}
