// FocusValueDlg.cpp : implementation file
//

#include "stdafx.h"
#include "imagetest.h"
#include "FocusValueDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFocusValueDlg dialog


CFocusValueDlg::CFocusValueDlg(CWnd* pParent /*=NULL*/,int nChannel,LLONG lLoginID)
	: CDialog(CFocusValueDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFocusValueDlg)
	m_bAutoFocus = FALSE;
	m_nLimit = 0;
	//}}AFX_DATA_INIT
    m_nChannel = nChannel;
    m_LoginID = lLoginID;

    memset(&m_stuFocusValue,0,sizeof(m_stuFocusValue));
    m_stuFocusValue.dwSize = sizeof(m_stuFocusValue);

    memset(&m_stuFocusMode,0,sizeof(m_stuFocusMode));
    m_stuFocusMode.dwSize = sizeof(m_stuFocusMode);
}


void CFocusValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFocusValueDlg)
	DDX_Control(pDX, IDC_CMB_FOCUSMODE, m_Cmb_FocusMode);
	DDX_Control(pDX, IDC_CMB_TYPE, m_Cmb_Type);
	DDX_Control(pDX, IDC_CMB_SENSITIVITY, m_Cmb_Sensitivity);
	DDX_Control(pDX, IDC_CMB_LIMITMODE, m_Cmb_LimitMode);
	DDX_Control(pDX, IDC_CMB_IRCORRECTION, m_Cmb_IrCorrection);
	DDX_Check(pDX, IDC_CHECK_AUTOFOCUS, m_bAutoFocus);
	DDX_Text(pDX, IDC_EDIT_LIMIT, m_nLimit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFocusValueDlg, CDialog)
	//{{AFX_MSG_MAP(CFocusValueDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_CBN_SELCHANGE(IDC_CMB_TYPE, OnSelchangeCmbType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFocusValueDlg message handlers

BOOL CFocusValueDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

/*
		白天
		夜晚
		普通
		
		关闭
		辅助聚焦
		自动聚焦
		半自动聚焦
		手动聚焦
		
		高
		默认
		低
		
		不修正
		修正
		自动修正
		
		手动
		自动
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

	
	CString FocusMode[5]={"close","assist-focus","auto-focus","halt-auto-focus","manual-focus"}; 
	m_Cmb_FocusMode.ResetContent(); 
	for(i=0;i<5;i++) 
	{ 
		nIndex = m_Cmb_FocusMode.AddString(ConvertString(FocusMode[i])); 
		m_Cmb_FocusMode.SetItemData(nIndex,i+1); 
	} 
	
	CString Sensitivity[3]={"high","default","low"}; 
	m_Cmb_Sensitivity.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_Sensitivity.AddString(ConvertString(Sensitivity[i])); 
		m_Cmb_Sensitivity.SetItemData(nIndex,i+1); 
	} 
	
	CString IrCorrection[4]={"non-correction","correction","auto-correction"}; 
	m_Cmb_IrCorrection.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_IrCorrection.AddString(ConvertString(IrCorrection[i])); 
		m_Cmb_IrCorrection.SetItemData(nIndex,i+1); 
	} 
	
	CString LimitMode[2]={"manual","automatic"}; 
	m_Cmb_LimitMode.ResetContent(); 
	for(i=0;i<2;i++) 
	{ 
		nIndex = m_Cmb_LimitMode.AddString(ConvertString(LimitMode[i])); 
		m_Cmb_LimitMode.SetItemData(nIndex,i+1); 
	} 


	if (GetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSVALUE))
	{
        StuToDlg(NET_EM_CFG_VIDEOIN_FOCUSVALUE);
	}
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSMODE))
	{
        StuToDlg(NET_EM_CFG_VIDEOIN_FOCUSMODE);
	}
	m_Cmb_Type.SetCurSel(0);




	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFocusValueDlg::GetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType, NET_EM_CONFIG_TYPE EmConfigType)
{
    BOOL bRet = FALSE;
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_FOCUSVALUE:
        {
            m_stuFocusValue.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_FOCUSVALUE,m_nChannel,&m_stuFocusValue,sizeof(m_stuFocusValue),TIMEOUT);
            if (!bRet)
            {
                MessageBox(ConvertString("Get focused info failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_FOCUSMODE:
        {
            m_stuFocusMode.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_FOCUSMODE,m_nChannel,&m_stuFocusMode,sizeof(m_stuFocusMode),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get focused mode failed!"), ConvertString("Prompt"));
            }
            break;
        }
    }
    
    return bRet;
}

BOOL CFocusValueDlg::SetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType, NET_EM_CONFIG_TYPE EmConfigType)
{
    BOOL bRet = FALSE;
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_FOCUSVALUE:
        {
            m_stuFocusValue.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_FOCUSVALUE,m_nChannel,&m_stuFocusValue,sizeof(m_stuFocusValue),TIMEOUT);
            if (!bRet)
            {
                MessageBox(ConvertString("Set focused info failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_FOCUSMODE:
        {
            m_stuFocusMode.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_FOCUSMODE,m_nChannel,&m_stuFocusMode,sizeof(m_stuFocusMode),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set focused mode failed!"), ConvertString("Prompt"));
            }
            break;
        }
    }
    
    return bRet;
}
void CFocusValueDlg::StuToDlg(NET_EM_CFG_OPERATE_TYPE emOperateType)
{
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_FOCUSVALUE:
        {
            m_Cmb_Sensitivity.SetCurSel(m_stuFocusValue.emSensitivity);
            m_Cmb_IrCorrection.SetCurSel(m_stuFocusValue.emIRCorrection);
            m_Cmb_LimitMode.SetCurSel(m_stuFocusValue.emFocusLimitSelectMode -1);
            m_nLimit = m_stuFocusValue.nFocusLimit;
            m_bAutoFocus = m_stuFocusValue.bAutoFocusTrace;
            break;
        }
    case NET_EM_CFG_VIDEOIN_FOCUSMODE:
        {
            m_Cmb_FocusMode.SetCurSel(m_stuFocusMode.emFocusMode);
            break;
        }
    }

    UpdateData(FALSE);
}
void CFocusValueDlg::DlgToStu(NET_EM_CFG_OPERATE_TYPE emOperateType)
{
    UpdateData();
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_FOCUSVALUE:
        {
            m_stuFocusValue.emSensitivity = (NET_EM_SENSITIVITY_TYPE)m_Cmb_Sensitivity.GetCurSel();
            m_stuFocusValue.emIRCorrection = (NET_EM_IRC_TYPE)m_Cmb_IrCorrection.GetCurSel();
            m_stuFocusValue.emFocusLimitSelectMode = (NET_EM_FOCUS_LIMITSELECT_MODE)(m_Cmb_LimitMode.GetCurSel()+1);
            m_stuFocusValue.nFocusLimit = m_nLimit;
            m_stuFocusValue.bAutoFocusTrace = m_bAutoFocus;
            break;
        }
    case NET_EM_CFG_VIDEOIN_FOCUSMODE:
        {
            m_stuFocusMode.emFocusMode = (NET_EM_FOCUS_MODE)m_Cmb_FocusMode.GetCurSel();
            break;
        }
    }
}

void CFocusValueDlg::OnButton1() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSVALUE,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_FOCUSVALUE);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSMODE,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_FOCUSMODE);
    }
}

void CFocusValueDlg::OnSet() 
{
    int i = m_Cmb_Type.GetCurSel();
	DlgToStu(NET_EM_CFG_VIDEOIN_FOCUSVALUE);
    DlgToStu(NET_EM_CFG_VIDEOIN_FOCUSMODE);
    SetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSMODE,NET_EM_CONFIG_TYPE(i));
    SetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSVALUE,NET_EM_CONFIG_TYPE(i));
}

void CFocusValueDlg::OnSelchangeCmbType() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSVALUE,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_FOCUSVALUE);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_FOCUSMODE,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_FOCUSMODE);
    }
}
