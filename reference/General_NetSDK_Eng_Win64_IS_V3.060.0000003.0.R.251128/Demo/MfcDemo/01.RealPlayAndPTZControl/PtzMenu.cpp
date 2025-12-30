// DHPtzMenu.cpp : implementation file
//

#include "stdafx.h"
#include "RealPlayAndPTZControl.h"
#include "PtzMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPtzMenu dialog


CPtzMenu::CPtzMenu(CWnd* pParent /*=NULL*/)
	: CDialog(CPtzMenu::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPtzMenu)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Channel=-1;
	m_DeviceID=0;
}


void CPtzMenu::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPtzMenu)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPtzMenu, CDialog)
	//{{AFX_MSG_MAP(CPtzMenu)
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
// CPtzMenu message handlers

void CPtzMenu::SetPtzParam(LLONG iHandle, int iChannel)
{
	m_DeviceID = iHandle;
	m_Channel = iChannel;
}

void CPtzMenu::PtzMemuControl(DWORD dwCommand)
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
void CPtzMenu::OnOprUp() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUUP);
}

//Move down 
void CPtzMenu::OnOprDown() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUDOWN);
}

//Move left 
void CPtzMenu::OnOprLeft() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENULEFT);
}

//Move right 
void CPtzMenu::OnOprRight() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENURIGHT);
}

//Open menu 
void CPtzMenu::OnOprOpenmenu() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_OPENMENU);
}

//Close menu 
void CPtzMenu::OnOprClosemenu() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_CLOSEMENU);
}

//Confirm 
void CPtzMenu::OnOprOk() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUOK);
}

//Cancel 
void CPtzMenu::OnOprCancel() 
{
	// TODO: Add your control notification handler code here
	PtzMemuControl(DH_EXTPTZ_MENUCANCEL);
}

BOOL CPtzMenu::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPtzMenu::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}




















































































































































































































































































































































