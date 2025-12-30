// BlackLight.cpp : implementation file
//

#include "stdafx.h"
#include "imagetest.h"
#include "BlackLight.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BlackLight dialog


BlackLight::BlackLight(CWnd* pParent /*=NULL*/,int nChannel,LLONG lLoginID)
	: CDialog(BlackLight::IDD, pParent)
{
	//{{AFX_DATA_INIT(BlackLight)
	m_nBottom = 0;
	m_nLeft = 0;
	m_nRight = 0;
	m_nTop = 0;
	//}}AFX_DATA_INIT
    
    m_nChannel = nChannel;
    m_LoginID = lLoginID;
    memset(&m_stuBlackLight,0,sizeof(m_stuBlackLight));
    m_stuBlackLight.dwSize = sizeof(m_stuBlackLight);
}


void BlackLight::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BlackLight)
	DDX_Control(pDX, IDC_SLIDER_WIDE, m_Slider_Wide);
	DDX_Control(pDX, IDC_SLIDER_GLARE, m_Slider_Glare);
	DDX_Control(pDX, IDC_COMBO_BLACKLIGHTMODE, m_Cmb_BlackLightMode);
	DDX_Control(pDX, IDC_COMBO_BLACKMODE, m_Cmb_BlackMode);
	DDX_Control(pDX, IDC_CMB_TYPE, m_Cmb_Type);
	DDX_Text(pDX, IDC_EDIT_BOTTOM, m_nBottom);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_nLeft);
	DDX_Text(pDX, IDC_EDIT_RIGHT, m_nRight);
	DDX_Text(pDX, IDC_EDIT_TOP, m_nTop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BlackLight, CDialog)
	//{{AFX_MSG_MAP(BlackLight)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_CBN_SELCHANGE(IDC_CMB_TYPE, OnSelchangeCmbType)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_WIDE, OnCustomdrawSliderWide)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_GLARE, OnCustomdrawSliderGlare)
	ON_BN_CLICKED(IDC_BTN_MINUS_WIDE, OnBtnMinusWide)
	ON_BN_CLICKED(IDC_BTN_PLUS_WIDE, OnBtnPlusWide)
	ON_BN_CLICKED(IDC_BTN_MINUS_GLARE, OnBtnMinusGlare)
	ON_BN_CLICKED(IDC_BTN_PLUS_GLARE, OnBtnPlusGlare)
	ON_CBN_SELCHANGE(IDC_COMBO_BLACKMODE, OnSelchangeComboBlackmode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BlackLight message handlers
BOOL BlackLight::GetVideoIn(NET_EM_CONFIG_TYPE EmConfigType)
{
    BOOL bRet = FALSE;
    m_stuBlackLight.emCfgType = EmConfigType;
    bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_BACKLIGHT,m_nChannel,&m_stuBlackLight,sizeof(m_stuBlackLight),TIMEOUT);
    if (!bRet)
    {   
        MessageBox(ConvertString("Get backlight config failed!"), ConvertString("Prompt"));
    }
    return bRet;
}
BOOL BlackLight::SetVideoIn(NET_EM_CONFIG_TYPE EmConfigType)
{
    BOOL bRet = FALSE;
    m_stuBlackLight.emCfgType = EmConfigType;
    bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_BACKLIGHT,m_nChannel,&m_stuBlackLight,sizeof(m_stuBlackLight),TIMEOUT);
    if (!bRet)
    {   
        MessageBox(ConvertString("Set backlight config failed!"), ConvertString("Prompt"));
    }
    return bRet;
}
void BlackLight::StuToDlg()
{
    m_Cmb_BlackMode.SetCurSel(m_stuBlackLight.emBlackMode);
    m_Cmb_BlackLightMode.SetCurSel(m_stuBlackLight.emBlackLightMode);
    m_Slider_Wide.SetPos(m_stuBlackLight.nWideDynamicRange);
    m_Slider_Glare.SetPos(m_stuBlackLight.nGlareInhibition);
    m_nLeft = m_stuBlackLight.stuBacklightRegion.nLeft;
    m_nRight = m_stuBlackLight.stuBacklightRegion.nRight;
    m_nTop = m_stuBlackLight.stuBacklightRegion.nTop;
    m_nBottom = m_stuBlackLight.stuBacklightRegion.nBottom;
    if (m_stuBlackLight.emBlackMode == NET_EM_BACKLIGHT_MODE_WIDEDYNAMIC)
    {
        GetDlgItem(IDC_BTN_PLUS_WIDE)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_MINUS_WIDE)->EnableWindow(TRUE);
        GetDlgItem(IDC_SLIDER_WIDE)->EnableWindow(TRUE);
    }
    else if (m_stuBlackLight.emBlackMode == NET_EM_BACKLIGHT_MODE_GLAREINHIBITION)
    {
        GetDlgItem(IDC_BTN_PLUS_GLARE)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_MINUS_GLARE)->EnableWindow(TRUE);
        GetDlgItem(IDC_SLIDER_GLARE)->EnableWindow(TRUE);
    }
    UpdateData(FALSE);
}
void BlackLight::DlgToStu()
{
    UpdateData();
    m_stuBlackLight.emBlackLightMode = (NET_EM_BLACKLIGHT_MODE)m_Cmb_BlackLightMode.GetCurSel();
    m_stuBlackLight.emBlackMode = (NET_EM_BACK_MODE)m_Cmb_BlackMode.GetCurSel();
    m_stuBlackLight.nWideDynamicRange = m_Slider_Wide.GetPos();
    m_stuBlackLight.nGlareInhibition = m_Slider_Glare.GetPos();
    m_stuBlackLight.stuBacklightRegion.nLeft = m_nLeft;
    m_stuBlackLight.stuBacklightRegion.nRight = m_nRight;
    m_stuBlackLight.stuBacklightRegion.nBottom = m_nBottom;
    m_stuBlackLight.stuBacklightRegion.nTop = m_nTop;
}

BOOL BlackLight::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	m_Slider_Wide.SetRange(1,100);
    m_Slider_Glare.SetRange(1,100);

    GetDlgItem(IDC_BTN_PLUS_WIDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_MINUS_WIDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_PLUS_GLARE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_MINUS_GLARE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SLIDER_WIDE)->EnableWindow(FALSE);
    GetDlgItem(IDC_SLIDER_GLARE)->EnableWindow(FALSE);    

/*
白天
夜晚
普通
	
未知
关闭
背光补偿
宽动态
强光抑制
场景自适应
	  
未知
默认
自定义区域
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

	CString BackLightMode[9]={"unknown","close","BLC","WDR","HLC","scene adaptive"}; 
	m_Cmb_BlackMode.ResetContent(); 
	for(i=0;i<6;i++) 
	{ 
		nIndex = m_Cmb_BlackMode.AddString(ConvertString(BackLightMode[i])); 
		m_Cmb_BlackMode.SetItemData(nIndex,i+1); 
	} 


	CString CompensationMode[3]={"unknown","default","custom locale"}; 
	m_Cmb_BlackLightMode.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_BlackLightMode.AddString(ConvertString(CompensationMode[i])); 
		m_Cmb_BlackLightMode.SetItemData(nIndex,i+1); 
	} 

	GetVideoIn();
	StuToDlg();
	m_Cmb_Type.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void BlackLight::OnGet() 
{
    int i = m_Cmb_Type.GetCurSel();

	if (GetVideoIn((NET_EM_CONFIG_TYPE )i))
	{
        StuToDlg();
	}
}

void BlackLight::OnSet() 
{
    int i = m_Cmb_Type.GetCurSel();
    DlgToStu();
    SetVideoIn((NET_EM_CONFIG_TYPE )i);
}

void BlackLight::OnSelchangeCmbType() 
{
    int i = m_Cmb_Type.GetCurSel();
    
    if (GetVideoIn((NET_EM_CONFIG_TYPE )i))
    {
        StuToDlg();
	}
}

void BlackLight::OnCustomdrawSliderWide(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Wide.GetPos();
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_WIDE,str);
    *pResult = 0;
}

void BlackLight::OnCustomdrawSliderGlare(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Glare.GetPos();
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_GLARE,str);
    *pResult = 0;
}

void BlackLight::OnBtnMinusWide() 
{
    int nPos = m_Slider_Wide.GetPos();
    if (nPos<=1)
    {
        return;
    }
    nPos--;
    m_Slider_Wide.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_WIDE,str);
}

void BlackLight::OnBtnPlusWide() 
{
    int nPos = m_Slider_Wide.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Wide.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_WIDE,str);
}

void BlackLight::OnBtnMinusGlare() 
{
    int nPos = m_Slider_Glare.GetPos();
    if (nPos<=1)
    {
        return;
    }
    nPos--;
    m_Slider_Glare.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_GLARE,str);
}

void BlackLight::OnBtnPlusGlare() 
{
    int nPos = m_Slider_Glare.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Glare.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_GLARE,str);
}

void BlackLight::OnSelchangeComboBlackmode() 
{
	int i = m_Cmb_BlackMode.GetCurSel();
    if (i == NET_EM_BACKLIGHT_MODE_WIDEDYNAMIC)
    {
        GetDlgItem(IDC_BTN_PLUS_WIDE)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_MINUS_WIDE)->EnableWindow(TRUE);
        GetDlgItem(IDC_SLIDER_WIDE)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_BTN_PLUS_WIDE)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_MINUS_WIDE)->EnableWindow(FALSE);
        GetDlgItem(IDC_SLIDER_WIDE)->EnableWindow(FALSE);
    }
    if (i == NET_EM_BACKLIGHT_MODE_GLAREINHIBITION)
    {
        GetDlgItem(IDC_BTN_PLUS_GLARE)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_MINUS_GLARE)->EnableWindow(TRUE);
        GetDlgItem(IDC_SLIDER_GLARE)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_BTN_PLUS_GLARE)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_MINUS_GLARE)->EnableWindow(FALSE);
        GetDlgItem(IDC_SLIDER_GLARE)->EnableWindow(FALSE);
    }
}
