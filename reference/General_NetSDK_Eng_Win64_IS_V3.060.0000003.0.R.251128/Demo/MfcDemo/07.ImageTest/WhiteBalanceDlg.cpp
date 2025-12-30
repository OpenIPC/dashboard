// WhiteBalanceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "imagetest.h"
#include "WhiteBalanceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWhiteBalanceDlg dialog


CWhiteBalanceDlg::CWhiteBalanceDlg(CWnd* pParent /*=NULL*/,int nChannel,LLONG lLoginID)
	: CDialog(CWhiteBalanceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWhiteBalanceDlg)
	m_nColorTemperature = 0;
	//}}AFX_DATA_INIT
    m_nChannel = nChannel;
    m_LoginID = lLoginID;
    memset(&m_stuWhiteBalance, 0, sizeof(m_stuWhiteBalance));
    m_stuWhiteBalance.dwSize = sizeof(m_stuWhiteBalance);
}


void CWhiteBalanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWhiteBalanceDlg)
	DDX_Control(pDX, IDC_SLIDER_RED, m_Slider_Red);
	DDX_Control(pDX, IDC_SLIDER_GREEN, m_Slider_Green);
	DDX_Control(pDX, IDC_SLIDER_BLUE, m_Slider_Blue);
	DDX_Control(pDX, IDC_COMBO_WHITEBALANCETYPE, m_Cmb_WhiteBalance);
	DDX_Control(pDX, IDC_COMBO_TYPE, m_Cmb_Type);
	DDX_Text(pDX, IDC_EDIT_COLORTEMPERATURE, m_nColorTemperature);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWhiteBalanceDlg, CDialog)
	//{{AFX_MSG_MAP(CWhiteBalanceDlg)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_BN_CLICKED(IDC_BTN_MINUS_RED, OnBtnMinusRed)
	ON_BN_CLICKED(IDC_BTN_MINUS_BLUE, OnBtnMinusBlue)
	ON_BN_CLICKED(IDC_BTN_MINUS_GREEN, OnBtnMinusGreen)
	ON_BN_CLICKED(IDC_BTN_PLUS_RED, OnBtnPlusRed)
	ON_BN_CLICKED(IDC_BTN_PLUS_BLUE, OnBtnPlusBlue)
	ON_BN_CLICKED(IDC_BTN_PLUS_GREEN, OnBtnPlusGreen)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RED, OnCustomdrawSliderRed)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BLUE, OnCustomdrawSliderBlue)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_GREEN, OnCustomdrawSliderGreen)
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE, OnSelchangeComboType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhiteBalanceDlg message handlers

BOOL CWhiteBalanceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
/*	
		白天
		夜晚
		普通
		
		
		未知
		关闭
		自动
		自定义
		晴天
		阴天
		家里
		办公
		夜晚
		高色温区间
		低色温区间
		自动色温区间
		自定义色温等级
		室内
		室外
		跟踪
		手动
		室外自动
		钠灯自动
		钠灯
		自定义单区域白平衡
		自定义多区域白平衡
		自然光
		路灯
*/
    m_Slider_Blue.SetRange(0,100);
    m_Slider_Green.SetRange(0,100);
    m_Slider_Red.SetRange(0,100);
    m_Cmb_Type.SetCurSel(0);


	int nIndex; 
	int i = 0; 
	CString ConfigMode[3]={"day","night","normal"}; 
	m_Cmb_Type.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_Type.AddString(ConvertString(ConfigMode[i])); 
		m_Cmb_Type.SetItemData(nIndex,i+1); 
	} 
	m_Cmb_Type.SetCurSel(0);

	
	CString WhiteBalance[24]={"unknown","close","automatic","custom","sunny","cloudy","home","office","night","High color-temperature range","Low Color-temperature range",
		"Auto Color-temperature range","Custom Color-temperature level","indoor","outdoor","following","manual","outdoor automatic","sodium lamp automatic","sodium lamp",
		"custom single-region white balance","custom multi-region white balance","natural light","road lamp"}; 
	m_Cmb_WhiteBalance.ResetContent(); 
	for(i=0;i<24;i++) 
	{ 
		nIndex = m_Cmb_WhiteBalance.AddString(ConvertString(WhiteBalance[i])); 
		m_Cmb_WhiteBalance.SetItemData(nIndex,i+1); 
	} 



    if (GetVideoIn(NET_EM_CONFIG_DAYTIME))
    {
        StuToDlg();
    }
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWhiteBalanceDlg::StuToDlg()
{
    m_Cmb_WhiteBalance.SetCurSel(m_stuWhiteBalance.emWhiteBalanceType);
    m_Slider_Blue.SetPos(m_stuWhiteBalance.nGainBlue);
    CString str;
    str.Format("%d",m_stuWhiteBalance.nGainBlue);
    SetDlgItemText(IDC_STATIC_BLUE,str);
    m_Slider_Green.SetPos(m_stuWhiteBalance.nGainGreen);
    str.Format("%d",m_stuWhiteBalance.nGainGreen);
    SetDlgItemText(IDC_STATIC_GREEN,str);
    m_Slider_Red.SetPos(m_stuWhiteBalance.nGainRed);
    str.Format("%d",m_stuWhiteBalance.nGainRed);
    SetDlgItemText(IDC_STATIC_RED,str);
    m_nColorTemperature = m_stuWhiteBalance.nColorTemperature;
    UpdateData(FALSE);
}
void CWhiteBalanceDlg::DlgToStu()
{
    UpdateData();
    m_stuWhiteBalance.emWhiteBalanceType = (NET_EM_WHITEBALANCE_TYPE)m_Cmb_WhiteBalance.GetCurSel();
    m_stuWhiteBalance.nGainBlue = m_Slider_Blue.GetPos();
    m_stuWhiteBalance.nGainGreen = m_Slider_Green.GetPos();
    m_stuWhiteBalance.nGainRed = m_Slider_Red.GetPos();
    m_stuWhiteBalance.nColorTemperature = m_nColorTemperature;
}

BOOL CWhiteBalanceDlg::GetVideoIn(NET_EM_CONFIG_TYPE emConfigType)
{
    BOOL bRet = FALSE;
    m_stuWhiteBalance.emCfgType = emConfigType;
    bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_WHITEBALANCE,m_nChannel,&m_stuWhiteBalance,sizeof(m_stuWhiteBalance),TIMEOUT);
    if (!bRet)
    {   
        MessageBox(ConvertString("Get white balancing config failed!"), ConvertString("Prompt"));
    }
    return bRet;
}
    
BOOL CWhiteBalanceDlg::SetVideoIn(NET_EM_CONFIG_TYPE emConfigType)
{
    m_stuWhiteBalance.emCfgType = emConfigType;
    BOOL bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_WHITEBALANCE,m_nChannel,&m_stuWhiteBalance,sizeof(m_stuWhiteBalance),TIMEOUT);
    if (!bRet)
    {   
        MessageBox(ConvertString("Set white balancing config failed!"), ConvertString("Prompt"));
    }
    return TRUE;
}

void CWhiteBalanceDlg::OnGet() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn((NET_EM_CONFIG_TYPE)i))
    {
        StuToDlg();
    }
}

void CWhiteBalanceDlg::OnSet() 
{
    int i = m_Cmb_Type.GetCurSel();
	DlgToStu();
    SetVideoIn((NET_EM_CONFIG_TYPE)i);
}

void CWhiteBalanceDlg::OnBtnMinusRed() 
{
    int nPos = m_Slider_Red.GetPos();
    if (nPos<=0)
    {
        return;
    }
    nPos--;
    m_Slider_Red.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_RED,str);
}

void CWhiteBalanceDlg::OnBtnMinusBlue() 
{
    int nPos = m_Slider_Blue.GetPos();
    if (nPos<=0)
    {
        return;
    } 
    nPos--;
    m_Slider_Blue.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_BLUE,str);
}

void CWhiteBalanceDlg::OnBtnMinusGreen() 
{
    int nPos = m_Slider_Green.GetPos();
    if (nPos<=0)
    {
        return;
    } 
    nPos--;
    m_Slider_Green.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_GREEN,str);
}

void CWhiteBalanceDlg::OnBtnPlusRed() 
{
    int nPos = m_Slider_Red.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Red.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_RED,str);
}

void CWhiteBalanceDlg::OnBtnPlusBlue() 
{
    int nPos = m_Slider_Blue.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Blue.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_BLUE,str);
}

void CWhiteBalanceDlg::OnBtnPlusGreen() 
{
    int nPos = m_Slider_Green.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Green.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_GREEN,str);

}

void CWhiteBalanceDlg::OnCustomdrawSliderRed(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Red.GetPos();
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_RED,str);
	
	*pResult = 0;
}

void CWhiteBalanceDlg::OnCustomdrawSliderBlue(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Blue.GetPos();
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_BLUE,str);
	*pResult = 0;
}

void CWhiteBalanceDlg::OnCustomdrawSliderGreen(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Green.GetPos();
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_GREEN,str);
	*pResult = 0;
}

void CWhiteBalanceDlg::OnSelchangeComboType() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn((NET_EM_CONFIG_TYPE)i))
    {
        StuToDlg();
    }
}
