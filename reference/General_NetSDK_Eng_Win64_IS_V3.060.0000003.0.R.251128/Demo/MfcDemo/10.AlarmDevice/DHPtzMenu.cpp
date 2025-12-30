// DHPtzMenu.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DHPtzMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDHPtzMenu dialog


CDHPtzMenu::CDHPtzMenu(CWnd* pParent /*=NULL*/)
	: CDialog(CDHPtzMenu::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDHPtzMenu)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Channel=-1;
	m_DeviceID=0;
}


void CDHPtzMenu::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDHPtzMenu)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDHPtzMenu, CDialog)
	//{{AFX_MSG_MAP(CDHPtzMenu)
	ON_BN_CLICKED(IDC_OPR_UP, OnOprUp)
	ON_BN_CLICKED(IDC_OPR_DOWN, OnOprDown)
	ON_BN_CLICKED(IDC_OPR_LEFT, OnOprLeft)
	ON_BN_CLICKED(IDC_OPR_RIGHT, OnOprRight)
	ON_BN_CLICKED(IDC_OPR_OPENMENU, OnOprOpenmenu)
	ON_BN_CLICKED(IDC_OPR_CLOSEMENU, OnOprClosemenu)
	ON_BN_CLICKED(IDC_OPR_OK, OnOprOk)
	ON_BN_CLICKED(IDC_OPR_CANCEL, OnOprCancel)
	ON_WM_CANCELMODE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDHPtzMenu message handlers

void CDHPtzMenu::SetPtzParam(LLONG iHandle, int iChannel)
{
	m_DeviceID = iHandle;
	m_Channel = iChannel;
}

void CDHPtzMenu::PtzMemuControl(DWORD dwCommand)
{
	if(0 != m_DeviceID)
	{
		BOOL bRet = CLIENT_DHPTZControl(m_DeviceID, m_Channel,dwCommand ,0,0,0,FALSE);
		if(!bRet)
		{	
			MessageBox(ConvertString("operate PTZ menu "), ConvertString("Prompt"));
		}
	}
}

//Move up 
void CDHPtzMenu::OnOprUp() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUUP);
}

//Move down 
void CDHPtzMenu::OnOprDown() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUDOWN);
}

//Move left 
void CDHPtzMenu::OnOprLeft() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENULEFT);
}

//Move right 
void CDHPtzMenu::OnOprRight() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENURIGHT);
}

//Open menu 
void CDHPtzMenu::OnOprOpenmenu() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_OPENMENU);
}

//Close menu 
void CDHPtzMenu::OnOprClosemenu() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_CLOSEMENU);
}

//Confirm 
void CDHPtzMenu::OnOprOk() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUOK);
}

//Cancel 
void CDHPtzMenu::OnOprCancel() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUCANCEL);
}

BOOL CDHPtzMenu::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDHPtzMenu::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}




















































































































































































































































































































































