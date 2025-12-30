// Defog.cpp : implementation file
//

#include "stdafx.h"
#include "imagetest.h"
#include "Defog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDefog dialog


CDefog::CDefog(CWnd* pParent /*=NULL*/,int nChannel,LLONG lLoginID)
	: CDialog(CDefog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDefog)
	m_bDefog = FALSE;
	m_nIntensity = 0;
	m_nLightLevel = 0;
	//}}AFX_DATA_INIT
    m_nChannel = nChannel;
    m_LoginID = lLoginID;
    memset(&m_stuDefog,0,sizeof(m_stuDefog));
    m_stuDefog.dwSize = sizeof(m_stuDefog);
}


void CDefog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDefog)
	DDX_Control(pDX, IDC_CMB_TYPE, m_Cmb_Type);
	DDX_Control(pDX, IDC_COMBO_INTENSITYMODE, m_Cmb_IntensityMode);
	DDX_Control(pDX, IDC_COMBO_DEFOGMODE, m_Cmb_DefogMode);
	DDX_Check(pDX, IDC_CHECK_DEFOG, m_bDefog);
	DDX_Text(pDX, IDC_EDIT_INTENSITY, m_nIntensity);
	DDX_Text(pDX, IDC_EDIT_LIGHTLEVEL, m_nLightLevel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDefog, CDialog)
	//{{AFX_MSG_MAP(CDefog)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_CBN_SELCHANGE(IDC_CMB_TYPE, OnSelchangeCmbType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDefog message handlers
BOOL CDefog::GetVideoIn(NET_EM_CONFIG_TYPE emConfigType)
{
    BOOL bRet = FALSE;
    m_stuDefog.emCfgType = emConfigType;
    bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_DEFOG,m_nChannel,&m_stuDefog,sizeof(m_stuDefog),TIMEOUT);
    if (!bRet)
    {
	        MessageBox(ConvertString("Get defog config failed!"), ConvertString("Prompt"));
    }
    return bRet;
}

BOOL CDefog::SetVideoIn(NET_EM_CONFIG_TYPE emConfigType)
{
    BOOL bRet = FALSE;
    m_stuDefog.emCfgType = emConfigType;
    bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_DEFOG,m_nChannel,&m_stuDefog,sizeof(m_stuDefog),TIMEOUT);
    if (!bRet)
    {   
        MessageBox(ConvertString("Set defog config failed!"), ConvertString("Prompt"));
    }
    return bRet;
}
void CDefog::StuToDlg()
{
    m_bDefog = m_stuDefog.bCamDefogEnable;
    m_Cmb_DefogMode.SetCurSel(m_stuDefog.emDefogMode);
    m_Cmb_IntensityMode.SetCurSel(m_stuDefog.emIntensityMode);
    m_nIntensity = m_stuDefog.nIntensity;
    m_nLightLevel = m_stuDefog.nLightIntensityLevel;

    UpdateData(FALSE);

}
void CDefog::DlgToStu()
{
    UpdateData();
    m_stuDefog.bCamDefogEnable = m_bDefog;
    m_stuDefog.emDefogMode = (NET_EM_DEFOG_MODE)m_Cmb_DefogMode.GetCurSel();
    m_stuDefog.emIntensityMode = (NET_EM_INTENSITY_MODE)m_Cmb_IntensityMode.GetCurSel();
    m_stuDefog.nIntensity = m_nIntensity;
    m_stuDefog.nLightIntensityLevel = m_nLightLevel;
}

void CDefog::OnGet() 
{
	int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
}

void CDefog::OnSet() 
{
    int i = m_Cmb_Type.GetCurSel();
    DlgToStu();
    SetVideoIn(NET_EM_CONFIG_TYPE(i));
}

BOOL CDefog::OnInitDialog() 
{
	CDialog::OnInitDialog();
/*
		白天
		夜晚
		普通
		
		未知
		关闭
		自动
		手动
		
		未知
		自动
		手动
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

	
	CString IntensityMode[3]={"unknown","automatic","manual"}; 
	m_Cmb_IntensityMode.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_IntensityMode.AddString(ConvertString(IntensityMode[i])); 
		m_Cmb_IntensityMode.SetItemData(nIndex,i+1); 
	} 

	CString DefogMode[4]={"unknown", "close", "automatic", "manual"}; 
	m_Cmb_DefogMode.ResetContent(); 
	for(i=0;i<4;i++) 
	{ 
		nIndex = m_Cmb_DefogMode.AddString(ConvertString(DefogMode[i])); 
		m_Cmb_DefogMode.SetItemData(nIndex,i+1); 
	} 


	g_SetWndStaticText(this);
	
	if (GetVideoIn(NET_EM_CONFIG_DAYTIME))
	{
        StuToDlg();
	}
    m_Cmb_Type.SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDefog::OnSelchangeCmbType() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg();
    }
}
