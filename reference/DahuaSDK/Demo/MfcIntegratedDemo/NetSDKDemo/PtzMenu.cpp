// DHPtzMenu.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "PtzMenu.h"
#include "NetSDKDemoDlg.h"

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
	m_DeviceID = 0;
	m_Channel = 0;
	//{{AFX_DATA_INIT(CPtzMenu)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
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
	ON_BN_CLICKED(IDC_OPR_CANCEL, OnOprCancel)
	ON_BN_CLICKED(IDC_OPR_CLOSEMENU, OnOprClosemenu)
	ON_BN_CLICKED(IDC_OPR_DOWN, OnOprDown)
	ON_BN_CLICKED(IDC_OPR_LEFT, OnOprLeft)
	ON_BN_CLICKED(IDC_OPR_OK, OnOprOk)
	ON_BN_CLICKED(IDC_OPR_OPENMENU, OnOprOpenmenu)
	ON_BN_CLICKED(IDC_OPR_RIGHT, OnOprRight)
	ON_BN_CLICKED(IDC_OPR_UP, OnOprUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPtzMenu message handlers
void CPtzMenu::SetPtzParam(LLONG iHandle, int iChannel)
{
	m_DeviceID = iHandle;
	m_Channel = iChannel;

}
void CPtzMenu::OnOprCancel() 
{
	PtzMemuControl(DH_EXTPTZ_MENUCANCEL);
}

void CPtzMenu::OnOprClosemenu() 
{
	PtzMemuControl(DH_EXTPTZ_CLOSEMENU);
}

void CPtzMenu::OnOprDown() 
{
	PtzMemuControl(DH_EXTPTZ_MENUDOWN);
}

void CPtzMenu::OnOprLeft() 
{
	PtzMemuControl(DH_EXTPTZ_MENULEFT);
}

void CPtzMenu::OnOprOk() 
{
	PtzMemuControl(DH_EXTPTZ_MENUOK);
}

void CPtzMenu::OnOprOpenmenu() 
{
	PtzMemuControl(DH_EXTPTZ_OPENMENU);
}

void CPtzMenu::OnOprRight() 
{
	PtzMemuControl(DH_EXTPTZ_MENURIGHT);
}

void CPtzMenu::OnOprUp() 
{
	PtzMemuControl(DH_EXTPTZ_MENUUP);
}

void CPtzMenu::PtzMemuControl(DWORD dwCommand)
{
	if(!m_DeviceID)
	{
		MessageBox(ConvertString(MSG_PTZCTRL_NOCHANNEL));
	}
	BOOL ret = CLIENT_DHPTZControl(m_DeviceID, m_Channel,dwCommand ,0,0,0,FALSE);
	if(!ret)
	{	
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		MessageBox(ConvertString(MSG_PTZCTRL_CTRLFAILED));
	}
	
}

BOOL CPtzMenu::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	return TRUE;
}
