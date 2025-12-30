// AdvanceBtnPannel.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "AdvanceBtnPannel.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAdvanceBtnPannel dialog


CAdvanceBtnPannel::CAdvanceBtnPannel(CWnd* pParent /*=NULL*/)
	: CDialog(CAdvanceBtnPannel::IDD, pParent)
{
	m_myBrush.CreateSolidBrush(RGB(170,170,255));
	//{{AFX_DATA_INIT(CAdvanceBtnPannel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAdvanceBtnPannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvanceBtnPannel)
	DDX_Control(pDX, IDC_OPEN_TALK, m_isTalkOpen);
	DDX_Control(pDX, IDC_OPEN_SOUND, m_isSoundOpen);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_SAVE_REALDATA2, m_IsSaveRealData);
	DDX_Control(pDX, IDC_SAVECBDATARAW, m_IsSaveDataRaw);
}


BEGIN_MESSAGE_MAP(CAdvanceBtnPannel, CDialog)
	//{{AFX_MSG_MAP(CAdvanceBtnPannel)
	ON_BN_CLICKED(IDC_OPEN_SOUND, OnOpenSound)
	ON_BN_CLICKED(IDC_OPEN_TALK, OnOpenTalk)
	ON_BN_CLICKED(IDC_UPDATE_DEVICE, OnUpdateDevice)
	ON_BN_CLICKED(IDC_RECORDSTATE, OnRecordstate)
	ON_BN_CLICKED(IDC_REBOOT_DEVICE, OnRebootDevice)
	ON_BN_CLICKED(IDC_SHUTDOWN_DEVICE, OnShutdownDevice)
	ON_BN_CLICKED(IDC_ALARM_IO_CONTROL, OnAlarmIoControl)
	ON_BN_CLICKED(IDC_DEVICE_STATE, OnDeviceState)
	ON_BN_CLICKED(IDC_CAPTURE_SCREEN, OnCaptureScreen)
	ON_BN_CLICKED(IDC_SET_I_FRAME, OnSetIFrame)
	ON_BN_CLICKED(IDC_SHOW_FLUX, OnShowFlux)
	ON_BN_CLICKED(IDC_TRANSCOM, OnTranscom)
	ON_BN_CLICKED(IDC_DDNS_QUERY_IP, OnDdnsQueryIp)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_DEVICE_WORKSTATE, OnDeviceWorkstate)
	ON_BN_CLICKED(IDC_DEVICE_CONTROLDISK, OnDeviceControldisk)
	ON_BN_CLICKED(IDC_DEVICE_USERINFO, OnDeviceUserinfo)
	ON_BN_CLICKED(IDC_TEST, OnTest)
	ON_BN_CLICKED(IDC_AUDIOBROADCAST, OnAudioBroadcast)
	ON_BN_CLICKED(IDC_CYCLE_MONITOR, OnCycleMonitor)
	ON_BN_CLICKED(IDC_SAVE_REALDATA2, OnBnClickedSaveRealdata)
	ON_BN_CLICKED(IDC_SAVECBDATARAW, OnBnClickedSavecbdataraw)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvanceBtnPannel message handlers

BOOL CAdvanceBtnPannel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvanceBtnPannel::OnOpenSound() 
{
	BOOL bOpen = m_isSoundOpen.GetCheck() > 0 ? TRUE : FALSE;
	((CNetSDKDemoDlg *)GetParent())->OpenSound(bOpen);
}

void CAdvanceBtnPannel::OnOpenTalk() 
{
	BOOL bOpen = m_isTalkOpen.GetCheck() > 0 ? TRUE : FALSE;
	BOOL ret = ((CNetSDKDemoDlg *)GetParent())->OpenTalk(bOpen);
	if (!ret)
	{
		m_isTalkOpen.SetCheck(!bOpen);		
	}
}

void CAdvanceBtnPannel::EnableTalk(BOOL bEnable)
{

}

void CAdvanceBtnPannel::SetCheckSound(int nCheck)
{
	m_isSoundOpen.SetCheck(nCheck);
}

void CAdvanceBtnPannel::SetCheckTalk(int nCheck)
{
	m_isTalkOpen.SetCheck(nCheck);
}

BOOL CAdvanceBtnPannel::IsTalkOpen(void)
{
	return m_isTalkOpen.GetCheck() > 0 ? TRUE : FALSE;
}

void CAdvanceBtnPannel::OnUpdateDevice() 
{
	((CNetSDKDemoDlg *)GetParent())->UpdateDevice();
}

void CAdvanceBtnPannel::OnRecordstate() 
{
	((CNetSDKDemoDlg *)GetParent())->Recordstate();
}

void CAdvanceBtnPannel::OnRebootDevice() 
{
	((CNetSDKDemoDlg *)GetParent())->RebootDevice();
}

void CAdvanceBtnPannel::OnShutdownDevice() 
{
	((CNetSDKDemoDlg *)GetParent())->ShutdownDevice();
}

void CAdvanceBtnPannel::OnAlarmIoControl() 
{
	((CNetSDKDemoDlg *)GetParent())->AlarmIOctrl();
}

void CAdvanceBtnPannel::OnDeviceState() 
{

}

void CAdvanceBtnPannel::OnCaptureScreen() 
{
	((CNetSDKDemoDlg *)GetParent())->CaptureScreen();
}

void CAdvanceBtnPannel::OnSetIFrame() 
{
	((CNetSDKDemoDlg *)GetParent())->SetIframe();
}

void CAdvanceBtnPannel::OnShowFlux() 
{
	((CNetSDKDemoDlg *)GetParent())->ShowFluxInfo();
}

void CAdvanceBtnPannel::OnTranscom() 
{
	((CNetSDKDemoDlg *)GetParent())->Transcom();
}

void CAdvanceBtnPannel::OnDdnsQueryIp() 
{
	((CNetSDKDemoDlg *)GetParent())->DDNS_QueryIP();
}

HBRUSH CAdvanceBtnPannel::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	return m_myBrush;
}

BOOL CAdvanceBtnPannel::PreTranslateMessage(MSG* pMsg) 
{
	if (WM_KEYDOWN == pMsg->message && 
		(VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam))
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CAdvanceBtnPannel::OnDeviceWorkstate() 
{
	((CNetSDKDemoDlg *)GetParent())->GetDeviceWorkstate();
}


void CAdvanceBtnPannel::OnDeviceControldisk() 
{
}


void CAdvanceBtnPannel::OnDeviceUserinfo() 
{
	((CNetSDKDemoDlg *)GetParent())->DeviceUserinfo();
}

void CAdvanceBtnPannel::OnTest() 
{
	((CNetSDKDemoDlg *)GetParent())->TestProc();
}

void CAdvanceBtnPannel::OnAudioBroadcast() 
{
	// TODO: Add your control notification handler code here
	((CNetSDKDemoDlg *)GetParent())->OpenAudioBroadcastDlg();
}

void CAdvanceBtnPannel::OnCycleMonitor() 
{
	((CNetSDKDemoDlg *)GetParent())->CycleMonitor();
}

void CAdvanceBtnPannel::OnBnClickedSaveRealdata()
{
	int nCheck = m_IsSaveRealData.GetCheck();
	((CNetSDKDemoDlg *)GetParent())->SaveRealdata(nCheck);
}

void CAdvanceBtnPannel::OnBnClickedSavecbdataraw()
{	
	int nCheck = m_IsSaveDataRaw.GetCheck();
	((CNetSDKDemoDlg *)GetParent())->SavecbdataRaw(nCheck);
}