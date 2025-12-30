// NormalBtnPannel.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "NormalBtnPannel.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNormalBtnPannel dialog

CNormalBtnPannel::CNormalBtnPannel(CWnd* pParent /*=NULL*/)
	: CDialog(CNormalBtnPannel::IDD, pParent)
{
	m_myBrush.CreateSolidBrush(RGB(200,200,255));
	//{{AFX_DATA_INIT(CNormalBtnPannel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNormalBtnPannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNormalBtnPannel)
	DDX_Control(pDX, IDC_SUBTYPESEL, m_subtypesel);
	DDX_Control(pDX, IDC_SPLITTESEL, m_splitsel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNormalBtnPannel, CDialog)
	//{{AFX_MSG_MAP(CNormalBtnPannel)
	ON_CBN_SELCHANGE(IDC_SPLITTESEL, OnSelchangeSplittesel)
	ON_BN_CLICKED(IDC_FULLSCREEN, OnFullscreen)
	ON_BN_CLICKED(IDC_ADD_DEVICE, OnAddDevice)
	ON_BN_CLICKED(IDC_DELETE_DEVICE, OnDeleteDevice)
	ON_BN_CLICKED(IDC_OPEN_CHANNEL, OnOpenChannel)
	ON_BN_CLICKED(IDC_OPEN_MULTIPLAY, OnOpenMultiplay)
	ON_BN_CLICKED(IDC_CLOSE_SCREEN, OnCloseScreen)
	ON_BN_CLICKED(IDC_CLOSE_ALLCHANNEL, OnCloseAllchannel)
	ON_BN_CLICKED(IDC_SEARCH_RECORD, OnSearchRecord)
	ON_BN_CLICKED(IDC_CYCLE_MONITOR, OnCycleMonitor)
	ON_BN_CLICKED(IDC_PLAYBACKBYTIME, OnPlaybackbytime)
	ON_BN_CLICKED(IDC_DOWNLOADBYTIME, OnDownloadbytime)
	ON_BN_CLICKED(IDC_SYSCONFIG, OnSysconfig)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNormalBtnPannel message handlers

BOOL CNormalBtnPannel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNormalBtnPannel::OnSelchangeSplittesel() 
{
	int nSplit = m_splitsel.GetCurSel();
	((CNetSDKDemoDlg *)GetParent())->SwitchMultiWnd(nSplit);
}

void CNormalBtnPannel::OnFullscreen() 
{
	((CNetSDKDemoDlg *)GetParent())->SwitchFullScreen();
}

int CNormalBtnPannel::GetSplit()
{
	return (m_splitsel.GetCurSel());
}

void CNormalBtnPannel::SetSplit(int nSplit)
{
	m_splitsel.SetCurSel(nSplit);
}

void CNormalBtnPannel::InitSplit(int curSplit)
{
	char *nStrSplit[SPLIT_TOTAL] = {MSG_DEMODLG_SPLITTYPE_1,
									MSG_DEMODLG_SPLITTYPE_4,
									MSG_DEMODLG_SPLITTYPE_9,
									MSG_DEMODLG_SPLITTYPE_16};
	
	//here curSplit must be lower 4 so ...
	for(int i = SPLIT1; i <= curSplit ; i++)
	{
		m_splitsel.InsertString(i, nStrSplit[i]);
	}
	ConvertComboBox(m_splitsel);

	m_subtypesel.InsertString(0, ConvertString(NAME_STREAM_MAIN));
	m_subtypesel.InsertString(1, ConvertString(NAME_STREAM_SUB1));
	m_subtypesel.InsertString(2, ConvertString(NAME_STREAM_SUB2));
	m_subtypesel.InsertString(3, ConvertString(NAME_STREAM_SUB3));
	
	m_subtypesel.SetCurSel(0);
	m_splitsel.SetCurSel(curSplit);
}

void CNormalBtnPannel::OnAddDevice() 
{
	((CNetSDKDemoDlg *)GetParent())->AddDevice();
}

void CNormalBtnPannel::OnDeleteDevice() 
{
	((CNetSDKDemoDlg *)GetParent())->DeleteDevice();
}

void CNormalBtnPannel::OnOpenChannel() 
{
	((CNetSDKDemoDlg *)GetParent())->OpenChannel();
}

void CNormalBtnPannel::OnOpenMultiplay() 
{
	((CNetSDKDemoDlg *)GetParent())->OpenMultiplay();
}

void CNormalBtnPannel::OnCloseScreen() 
{
	((CNetSDKDemoDlg *)GetParent())->CloseScreen();
}

void CNormalBtnPannel::OnCloseAllchannel() 
{
	((CNetSDKDemoDlg *)GetParent())->CloseAllChannel();
}

void CNormalBtnPannel::EnableMultiplay(BOOL bEnable)
{
//	GetDlgItem(IDC_OPEN_MULTIPLAY)->EnableWindow(bEnable);
}

void CNormalBtnPannel::OnSearchRecord() 
{
	((CNetSDKDemoDlg *)GetParent())->SearchRecord();
}

void CNormalBtnPannel::OnCycleMonitor() 
{
	((CNetSDKDemoDlg *)GetParent())->CycleMonitor();
}

void CNormalBtnPannel::OnPlaybackbytime() 
{
	((CNetSDKDemoDlg *)GetParent())->Playbackbytime();
}

void CNormalBtnPannel::OnDownloadbytime() 
{
	((CNetSDKDemoDlg *)GetParent())->Downloadbytime();
}

void CNormalBtnPannel::OnSysconfig() 
{
	((CNetSDKDemoDlg *)GetParent())->Sysconfig();
}


HBRUSH CNormalBtnPannel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd->GetDlgCtrlID() == IDC_SPLITTESEL ||
		pWnd->GetDlgCtrlID() == IDC_SPLITTESEL) 
	{
		return hbr;
	}

	return m_myBrush;
}

BOOL CNormalBtnPannel::PreTranslateMessage(MSG* pMsg) 
{
	if (WM_KEYDOWN == pMsg->message && 
		(VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam))
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

int CNormalBtnPannel::GetSubType()
{
	return m_subtypesel.GetCurSel();
}