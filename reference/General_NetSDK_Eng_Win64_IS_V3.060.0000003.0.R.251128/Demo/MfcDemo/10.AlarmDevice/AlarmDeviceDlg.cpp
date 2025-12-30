// AlarmDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "AlarmDeviceDlg.h"
#include "DHPtzMenu.h"
#include "MultiPlay.h"
#include "MessageText.h"

#include "DlgCapability.h"
#include "AlarmSubscribe.h"
#include "DlgAlarmBell.h"
#include "DlgUpgrade.h"
#include "DlgTalk.h"

#include "DlgCfgAlarmCenter.h"
#include "DlgCfgAlarmOut.h"
#include "DlgCfgGuard.h"
#include "DlgCfgAlarm.h"
#include "DlgFileManager.h"
#include "DlgOptPlaysound.h"
#include "DlgStateBattery.h"
#include "DlgCfgPSTNAlarmServer.h"
#include "DlgAlarmState.h"
#include "DlgCfgAlarmKeyboard.h"
#include "DlgCfgAlarmPowerFault.h"
#include "DlgCfgAlarmChassisIntrusion.h"
#include "DlgCfgAlarmExAlarmBox.h"
#include "DlgCfgExAlarmInput.h"
#include "DlgCfgExAlarmOutput.h"
#include "DlgExAlarmChannel.h"
#include "DlgCfgWireless.h"
#include "DlgCfgSerial.h"
#include "DlgCfgNetwork.h"
#include "DlgCfgLocales.h"
#include "DlgCfgNTP.h"
#include "DlgCfgMobile.h"
#include "DlgCfgPhoneNotify.h"
#include "DlgCfgAirCondition.h"

#include "DlgAccess.h"
#include "DlgFileList.h"
#include "DlgOptAccess.h"
#include "DlgUserManage.h"
#include "DlgVersion.h"
#include "DlgUserInfo.h"
#include "DlgUpgrade.h"
#include "DlgSetBypass.h"
#include "DlgDeviceTime.h"
#include "DlgActivatedDefenceArea.h"
#include "DlgCfgAutoMaintain.h"
#include "DlgSetNetWorkParam.h"
// #include "DlgCfgChange.h"
#include "DlgSetArmMode.h"
#include "TabUserManageDlg.h"
#include "DlgBypassCtl.h"
#include "DlgCfgAlarmUrgency.h"
#include "DlgQueryLog.h"
#include "DlgCfgSTP.h"
#include "DlgCfgSensorSampling.h"
#include "DlgAnalogAlarmChannels.h"
#include "DlgCfgAnalogAlarm.h"
#include "DlgAnalogAlarmData.h"
#include "DlgCfgSensorInfoOfVW.h"
#include "DlgCfgBatteryLowPower.h"
#include "DlgAlarmChannel.h"
#include "DlgCfgAlarmSubSystem.h"
#include "DlgAlarmSubSystem.h"
#include "DlgLowRateWPAN.h"
#include "DlgCfgSnapLinkage.h"
#include "DlgAlarmOutSet.h"
#include "AirConditionOperate.h"
#include "DlgCfgNetAbort.h"
#include "DlgCfgIPConflict.h"
#include "DlgCfgMacConflict.h"
#include "DlgRecoverConfig.h"
#include "DlgCfgPSTNBreakLine.h"
#include "DlgCfgNetCollection.h"
#include "DlgCfgAlarmSlotBond.h"
#include "DlgCfgRCEmergencyCall.h"
#include "DlgCfgAlarmBell.h"
#include "DlgCfgPSTNPersonServer.h"
#include "DlgCfgArmLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

//Forbid opening two programs at the same time 
#pragma data_seg("sharesec")
__declspec (allocate("sharesec")) HWND g_share_hWnd = NULL;
#pragma comment(linker,"/SECTION:sharesec,RWS")

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClientDemoDlg dialog

CAlarmDeviceDlg::CAlarmDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmDeviceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClientDemoDlg)
	m_DvrUserName = _T("admin");
	m_DvrPassword = _T("admin123");
	m_DvrPort = 37777;
	m_presetData = 1;
	m_crviseGroup = 1;
	m_moveNo = 1;
	m_posX = 0;
	m_posY = 0;
	m_posZoom = 0;
	m_uiAlarmIn = 0;
	m_uiAlarmOut = 0;
	m_uiAnalogAlarmIn = 0;
	m_emDeviceType = NET_PRODUCT_NONE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nChannelCount = 0;
	m_LoginID = 0;
	//Clear the 9-window real-time monitor handle 
	for(int i=0;i<9;i++)
	{
		m_DispHanle[i]=0;
		//Write 9-window information into CVideoNodeInfo array.
		m_videoNodeInfo[i].SetVideoInfo(i+1,"",0,-1,"","",DirectMode);
	}
	m_CurScreen =0;
}

void CAlarmDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClientDemoDlg)
	DDX_Control(pDX, IDC_CMB_CONTROLQUERY, m_cmbControlQueryType);
	DDX_Control(pDX, IDC_CMB_CONFIG, m_cmbConfig);
	DDX_Control(pDX, IDC_COMBO_PLAYMODE, m_comboPlayMode);
	DDX_Control(pDX, IDC_COMBO_AUX_NO, m_auxNosel);
	DDX_Control(pDX, IDC_COMBO_PTZDATA, m_comboPTZData);
	DDX_Control(pDX, IDC_IRIS_OPEN, m_iris_open);
	DDX_Control(pDX, IDC_IRIS_CLOSE, m_iris_close);
	DDX_Control(pDX, IDC_FOCUS_FAR, m_focus_far);
	DDX_Control(pDX, IDC_FOCUS_NEAR, m_focus_near);
	DDX_Control(pDX, IDC_ZOOM_TELE, m_zoom_tele);
	DDX_Control(pDX, IDC_ZOOM_WIDE, m_zoom_wide);
	DDX_Control(pDX, IDC_PTZ_RIGHTDOWN, m_ptz_rightdown);
	DDX_Control(pDX, IDC_PTZ_RIGHTUP, m_ptz_rightup);
	DDX_Control(pDX, IDC_PTZ_LEFTDOWN, m_ptz_leftdown);
	DDX_Control(pDX, IDC_PTZ_LEFTUP, m_ptz_leftup);
	DDX_Control(pDX, IDC_PTZ_RIGHT, m_ptz_right);
	DDX_Control(pDX, IDC_PTZ_LEFT, m_ptz_left);
	DDX_Control(pDX, IDC_PTZ_DOWN, m_ptz_down);
	DDX_Control(pDX, IDC_PTZ_UP, m_ptz_up);
	DDX_Control(pDX, IDC_COMBO_Channel, m_comboChannel);
	DDX_Control(pDX, IDC_COMBO_DispNum, m_comboDispNum);
	DDX_Control(pDX, IDC_DvrIPAddress, m_DvrIPAddr);
	DDX_Text(pDX, IDC_EDIT_UserName, m_DvrUserName);
	DDX_Text(pDX, IDC_EDIT_Password, m_DvrPassword);
	DDX_Text(pDX, IDC_EDIT_PORT, m_DvrPort);
	DDX_Text(pDX, IDC_PRESET_DATA, m_presetData);
//	DDV_MinMaxInt(pDX, m_presetData, 0, 100);
	DDX_Text(pDX, IDC_CRUISE_GROUP, m_crviseGroup);
	DDX_Text(pDX, IDC_MODE_NO, m_moveNo);
	DDX_Text(pDX, IDC_POS_X, m_posX);
	DDX_Text(pDX, IDC_POS_Y, m_posY);
	DDX_Text(pDX, IDC_POS_ZOOM, m_posZoom);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAlarmDeviceDlg, CDialog)
	//{{AFX_MSG_MAP(CClientDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BT_Login, OnBTLogin)
	ON_BN_CLICKED(IDC_BUTTON_Play, OnBUTTONPlay)
	ON_BN_CLICKED(IDC_BT_Leave, OnBTLeave)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_BN_CLICKED(IDC_BTN_PTZEXCTRL, OnBtnPtzexctrl)
	ON_BN_CLICKED(IDC_PRESET_SET, OnPresetSet)
	ON_BN_CLICKED(IDC_PRESET_ADD, OnPresetAdd)
	ON_BN_CLICKED(IDC_PRESET_DELE, OnPresetDele)
	ON_BN_CLICKED(IDC_START_CRUISE, OnStartCruise)
	ON_BN_CLICKED(IDC_CRUISE_ADD_POINT, OnCruiseAddPoint)
	ON_BN_CLICKED(IDC_CRUISE_DEL_POINT, OnCruiseDelPoint)
	ON_BN_CLICKED(IDC_CRUISE_DEL_GROUP, OnCruiseDelGroup)
	ON_BN_CLICKED(IDC_MODE_SET_BEGIN, OnModeSetBegin)
	ON_BN_CLICKED(IDC_MODE_START, OnModeStart)
	ON_BN_CLICKED(IDC_MODE_SET_DELETE, OnModeSetDelete)
	ON_BN_CLICKED(IDC_LINE_SET_LEFT, OnLineSetLeft)
	ON_BN_CLICKED(IDC_LINE_SET_RIGHT, OnLineSetRight)
	ON_BN_CLICKED(IDC_LINE_START, OnLineStart)
	ON_BN_CLICKED(IDC_FAST_GO, OnFastGo)
	ON_BN_CLICKED(IDC_EXACT_GO, OnExactGo)
	ON_BN_CLICKED(IDC_RESET, OnResetZero)
	ON_BN_CLICKED(IDC_ROTATE_START, OnRotateStart)
	ON_BN_CLICKED(IDC_ROTATE_STOP, OnRotateStop)
	ON_BN_CLICKED(IDC_AUX_OPEN, OnAuxOpen)
	ON_BN_CLICKED(IDC_AUX_CLOSE, OnAuxClose)
	ON_BN_CLICKED(IDC_BTN_PTZMENU, OnBtnPtzmenu)
	ON_CBN_SELCHANGE(IDC_COMBO_DispNum, OnSelchangeCOMBODispNum)
	ON_CBN_CLOSEUP(IDC_COMBO_DispNum, OnCloseupCOMBODispNum)
	ON_BN_CLICKED(IDC_BTN_CONFIG, OnBtnConfig)
	ON_BN_CLICKED(IDC_BTN_CAP, OnBtnCap)
	ON_BN_CLICKED(IDC_BTN_SUBSCRIBE, OnBtnSubscribe)
	ON_BN_CLICKED(IDC_BTN_CONTROLQUERY, OnBtnControlQuery)
	ON_BN_CLICKED(IDC_BTN_TALK, OnBtnTalk)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClientDemoDlg message handlers

BOOL CAlarmDeviceDlg::OnInitDialog()
{
	//Forbid opening two programs at the same time 
	this->IsValid();
	
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_ptzScreen.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,1981);
	UpdataScreenPos();
	m_ptzScreen.ShowWindow(SW_SHOW);
	m_ptzScreen.SetCallBack(MessageProcFunc,(LDWORD)this,
							GetParamsFunc,(LDWORD)this,
							SetParamsFunc,(LDWORD)this,
							RectEventFunc,(LDWORD)this);
	m_ptzScreen.SetShowPlayWin(SPLIT9,0);
	//Set initial IP address 
	{
		CString csIp = ConvertString("ip", DEVICE_PARAM);
		if (csIp == CString("ip"))
		{
			m_DvrIPAddr.SetWindowText("172.12.10.152");
		} 
		else
		{
			m_DvrIPAddr.SetWindowText(csIp);
		}
		
		CString csPort = ConvertString("port", DEVICE_PARAM);
		if (csPort == CString("port"))
		{
			SetDlgItemInt(IDC_EDIT_PORT, 37777);
		} 
		else
		{
			GetDlgItem(IDC_EDIT_PORT)->SetWindowText(csPort);
		}

		CString csName = ConvertString("username", DEVICE_PARAM);
		if (csName == CString("username"))
		{
			GetDlgItem(IDC_EDIT_UserName)->SetWindowText("admin");
		} 
		else
		{
			GetDlgItem(IDC_EDIT_UserName)->SetWindowText(csName);
		}
		
		CString csPsw = ConvertString("password", DEVICE_PARAM);
		if (csPsw == CString("password"))
		{
			GetDlgItem(IDC_EDIT_Password)->SetWindowText("admin");
		} 
		else
		{
			GetDlgItem(IDC_EDIT_Password)->SetWindowText(csPsw);
		}
	}
	//m_DvrIPAddr.SetAddress(172,11,3,59);
	
	//Zoom dialogux box 
	CRect rectSeparator;
	GetWindowRect(&m_rectLarge);
	GetDlgItem(IDC_SEPERATOR)->GetWindowRect(&rectSeparator);
	m_rectSmall.left=m_rectLarge.left;
	m_rectSmall.top=m_rectLarge.top;
	m_rectSmall.right=m_rectLarge.right;
	m_rectSmall.bottom=rectSeparator.bottom;
	SetWindowPos(NULL,0,0,m_rectSmall.Width(),m_rectSmall.Height(),SWP_NOMOVE | SWP_NOZORDER);
	
	//Command definition setup of PTZ control 
	InitPTZControl();
	//Initialize channel and window dropdown menu 
	InitComboBox();
	//Initialize net SDK
	InitNetSDK();

	
	m_cmbConfig.ResetContent();
	for (int i = 0; i < sizeof(stuConfigType) / sizeof(stuConfigType[0]); i++)
	{
		m_cmbConfig.InsertString(-1, ConvertString(stuConfigType[i].szName, DLG_MAIN));
	}
	m_cmbConfig.SetCurSel(-1);

	m_cmbControlQueryType.ResetContent();
	for (int j = 0; j < sizeof(stuControQueryType) / sizeof(stuControQueryType[0]); j++)
	{
		m_cmbControlQueryType.InsertString(-1, ConvertString(stuControQueryType[j].szName, DLG_MAIN));
	}
	m_cmbControlQueryType.SetCurSel(-1);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CAlarmDeviceDlg::UpdataScreenPos()
{
	GetDlgItem(IDC_SCREEN_WINDOW)->GetClientRect(&m_clientRect);
	
	m_screenRect = m_clientRect;
	m_ptzScreen.MoveWindow(m_screenRect);
}

void CAlarmDeviceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAlarmDeviceDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAlarmDeviceDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CAlarmDeviceDlg::PreTranslateMessage(MSG* pMsg) 
{
    // TODO: Add your specialized code here and/or call the base class
    if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
    {
        if (VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam)
        {
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}

//Forbid opening two programs at the same time
void CAlarmDeviceDlg::IsValid()
{
	if(g_share_hWnd)
	{
		MessageBox(ConvertString(_T("Only one program will be allowed to open!")), ConvertString("Prompt"));
		CWnd* pWnd = CWnd::FromHandle(g_share_hWnd);
		if(pWnd)
		{
			if (pWnd->IsIconic())
			{
				pWnd->ShowWindow(SW_RESTORE);
			}
			pWnd->SetForegroundWindow(); 
		}
		exit(0);
	}
	else
	{
		g_share_hWnd = m_hWnd;
	}
}

//Callback function when device disconnected
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CAlarmDeviceDlg *pThis = (CAlarmDeviceDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
}

//Callback function when device reconnect
void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CAlarmDeviceDlg *pThis = (CAlarmDeviceDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);
}


//Initialize net SDK
void CAlarmDeviceDlg::InitNetSDK()
{
	//Initialize net sdk, All callback begins here.
	BOOL bSuccess = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	if (!bSuccess)
	{
		//Display function error occurrs reason.
		LastError();
	}
	CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);

	NET_PARAM stuParam = {0};

	CString csConnectBufSize = ConvertString("ConnectBufSize", SDK_PARAM);
	if (csConnectBufSize == CString("ConnectBufSize"))
	{
		stuParam.nConnectBufSize = 250 * 1024;
	}
	else
	{
		stuParam.nConnectBufSize = atoi(csConnectBufSize.GetBuffer(0));
	}
	
	CString csConnectTime = ConvertString("ConnectTime", SDK_PARAM);
	if (csConnectTime == CString("ConnectTime"))
	{
		stuParam.nConnectTime = 1500;
	} 
	else
	{
		stuParam.nConnectTime = atoi(csConnectTime.GetBuffer(0));
	}
	
	CString csConnectTryNum = ConvertString("ConnetTryNum", SDK_PARAM);
	if (csConnectTryNum == CString("ConnetTryNum"))
	{
		stuParam.nConnectTryNum = 1;
	} 
	else
	{
		stuParam.nConnectTryNum = atoi(csConnectTryNum.GetBuffer(0));
	}
	
	CString csGetDevInfoTime = ConvertString("GetDevInfoTime", SDK_PARAM);
	if (csGetDevInfoTime == CString("GetDevInfoTime"))
	{
		stuParam.nGetDevInfoTime = 5000;
	} 
	else
	{
		stuParam.nGetDevInfoTime = atoi(csGetDevInfoTime.GetBuffer(0));
	}
	
	CString csWaitTime = ConvertString("WaitTime", SDK_PARAM);
	if (csWaitTime == CString("WaitTime"))
	{
		stuParam.nWaittime = 5000;
	} 
	else
	{
		stuParam.nWaittime = atoi(csWaitTime.GetBuffer(0));
	}
	
	CLIENT_SetNetworkParam(&stuParam);
}

void CAlarmDeviceDlg::OnBTLogin() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);	//Get interface input 
	if(bValid)
	{
		char *pchDVRIP;
		CString strDvrIP = GetDvrIP();
		pchDVRIP = (LPSTR)(LPCSTR)strDvrIP;
		WORD wDVRPort=(WORD)m_DvrPort;
		char *pchUserName=(LPSTR)(LPCSTR)m_DvrUserName;
		char *pchPassword=(LPSTR)(LPCSTR)m_DvrPassword;

		NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
		memset(&stInparam, 0, sizeof(stInparam));
		stInparam.dwSize = sizeof(stInparam);
		strncpy(stInparam.szIP, pchDVRIP, sizeof(stInparam.szIP) - 1);
		strncpy(stInparam.szPassword, pchPassword, sizeof(stInparam.szPassword) - 1);
		strncpy(stInparam.szUserName, pchUserName, sizeof(stInparam.szUserName) - 1);
		stInparam.nPort = wDVRPort;
		stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

		NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
		memset(&stOutparam, 0, sizeof(stOutparam));
		stOutparam.dwSize = sizeof(stOutparam);
		LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

		if(0 != lRet)
		{
			m_LoginID = lRet;
			GetDlgItem(IDC_BT_Login)->EnableWindow(FALSE);
			GetDlgItem(IDC_BT_Leave)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_Play)->EnableWindow(TRUE);
			//Device channel and channel dropdown menu 
			m_nChannelCount = (int)stOutparam.stuDeviceInfo.nChanNum;
			m_uiAlarmIn = stOutparam.stuDeviceInfo.nAlarmInPortNum;
			m_uiAlarmOut = stOutparam.stuDeviceInfo.nAlarmOutPortNum;
			m_uiAnalogAlarmIn = GetAnalogAlarmIn();
			int nIndex = 0;
			m_comboChannel.ResetContent();
			for(int i=0;i<m_nChannelCount;i++)
			{
				CString str;
				str.Format("%d",i+1);
				nIndex = m_comboChannel.AddString(str);
				m_comboChannel.SetItemData(nIndex,i);
			}
			if(0 < m_comboChannel.GetCount())
			{
				nIndex = m_comboChannel.AddString(ConvertString("Multi_Preview"));
				m_comboChannel.SetItemData(nIndex,-1);
				m_comboChannel.SetCurSel(0);
			}

			m_emDeviceType = (NET_DEVICE_TYPE)stOutparam.stuDeviceInfo.nDVRType;
		}
		else
		{
			//Display log in failure reason 
			ShowLoginErrorReason(stOutparam.nError);
		}
	}
	SetWindowText(ConvertString("AlarmDevice"));
}

//Play 
void CAlarmDeviceDlg::OnBUTTONPlay() 
{
	// TODO: Add your control notification handler code here
	//CLIENT_API LONG CLIENT_RealPlay(LONG lLoginID, int nChannelID, HWND hWnd);
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		int nIndex = m_comboDispNum.GetCurSel();
		if(CB_ERR != nIndex)
		{
			int iDispNum =m_comboDispNum.GetItemData(nIndex);
			//Get video handle 
			HWND hWnd=GetDispHandle(iDispNum);
			if(0 != hWnd)
			{
				nIndex = m_comboChannel.GetCurSel();
				if(CB_ERR != nIndex)
				{
					//Get channel number 
					int iChannel = m_comboChannel.GetItemData(nIndex);
					nIndex = m_comboPlayMode.GetCurSel();
					if(CB_ERR != nIndex)
					{
						//Get play mode 
						RealPlayMode ePlayMode = (RealPlayMode)m_comboPlayMode.GetItemData(nIndex);
						if(-1 == iChannel && ePlayMode == DirectMode)
						{
							//Play directly in multiple-window preview 
							MultiPlayMode(iDispNum,hWnd);
						}
						if(-1 == iChannel && ePlayMode == ServerMode)
						{
							//Data callback play mode in multiple-window preview mode. 
							MultiPlayServerMode(iDispNum,hWnd);
						}
						if(-1 != iChannel && ePlayMode == DirectMode)
						{
							//Play directly 
							DirectPlayMode(iDispNum,iChannel,hWnd);
						}
						if(-1 != iChannel && ePlayMode == ServerMode)
						{
							//Play in data callback mode 
							ServerPlayMode(iDispNum,iChannel,hWnd);
						}
					}
				}
			}
		}
	}
}

void CAlarmDeviceDlg::OnBTLeave() 
{
	// TODO: Add your control notification handler code here
	//CLIENT_API BOOL CLIENT_Logout(LONG lLoginID)
	SetWindowText(ConvertString("AlarmDevice"));
	if(0 != m_LoginID)
	{
		//Clear 9-window real-time monitor handle 
		for(int i=0;i<9;i++)
		{
			CloseDispVideo(i + 1);
			m_DispHanle[i]=0;
			//Write 9-window message into CVideoNodeInfo array 
			m_videoNodeInfo[i].SetVideoInfo(i+1,"",0, -1,"","",DirectMode);
		}

		BOOL bSuccess = CLIENT_Logout(m_LoginID);
		if(bSuccess)
		{
			m_LoginID = 0;
			m_comboChannel.ResetContent();
			GetDlgItem(IDC_BT_Login)->EnableWindow(TRUE);
			GetDlgItem(IDC_BT_Leave)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_Play)->EnableWindow(FALSE);
			Invalidate();
		}
		else
		{
			MessageBox(ConvertString("Fail to Logout!"), ConvertString("Prompt"));
		}
	}
}

//Get input IP
CString CAlarmDeviceDlg::GetDvrIP()
{
	CString strRet="";
	BYTE nField0,nField1,nField2,nField3;
	m_DvrIPAddr.GetAddress(nField0,nField1,nField2,nField3);
	strRet.Format("%d.%d.%d.%d",nField0,nField1,nField2,nField3);
	return strRet;
}

void CAlarmDeviceDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	for(int i=0;i<9;i++)
	{
		CloseDispVideo(i + 1);
		m_DispHanle[i]=0;
	}

	// TODO: Add your message handler code here
	if(0 != m_LoginID)
	{
		CLIENT_Logout(m_LoginID);
	}
	//Clear SDK and then release occupied resources.
	CLIENT_Cleanup();
}

//Get video handle 
HWND CAlarmDeviceDlg::GetDispHandle(int nNum)
{
	HWND hWnd=0;
	hWnd = ((CWnd *)(m_ptzScreen.GetPage(m_CurScreen)))->m_hWnd;
	return hWnd;	
}

void CAlarmDeviceDlg::InitComboBox()
{
	//Video dropdown menu initialization 
	int nIndex;
	int i = 0;
	CString strDispNum[9]={"1","2","3","4","5","6","7","8","9"};
	m_comboDispNum.ResetContent();
	for(i=0;i<9;i++)
	{
		nIndex = m_comboDispNum.AddString(strDispNum[i]);
		m_comboDispNum.SetItemData(nIndex,i+1);
	}
	m_comboDispNum.SetCurSel(0);
	//Channel dropdown menu initialization 
	m_comboChannel.ResetContent();
	for(i=0;i<m_nChannelCount;i++)
	{
		CString str;
		str.Format("%d",i+1);
		nIndex = m_comboChannel.AddString(str);
		m_comboChannel.SetItemData(nIndex,i);
	}
	if(0 < m_comboChannel.GetCount())
	{
		nIndex = m_comboChannel.AddString(ConvertString("Multi_Preview"));
		m_comboChannel.SetItemData(nIndex,-1);
		m_comboChannel.SetCurSel(0);
	}
	//Control parameter dropdown menu initialization 
	CString strPTZData[8]={"1","2","3","4","5","6","7","8"};
	m_comboPTZData.ResetContent();
	for(i=0;i<8;i++)
	{
		nIndex = m_comboPTZData.AddString(strPTZData[i]);
		m_comboPTZData.SetItemData(nIndex,i+1);
	}
	m_comboPTZData.SetCurSel(3);
	//Auxiliary dropdown menu initialization
	CString strAuxNo[5]={"23","24","27","41","43"};
	m_auxNosel.ResetContent();
	for(i=0;i<5;i++)
	{
		m_auxNosel.AddString(strAuxNo[i]);
	}
	m_auxNosel.SetCurSel(0);
	//Play mode dropdown menu initialization 
	CString strPlayMode[3]={ConvertString("Direct-play"), ConvertString("Data-callback")};
	m_comboPlayMode.ResetContent();

	nIndex = m_comboPlayMode.AddString(strPlayMode[0]);
	m_comboPlayMode.SetItemData(nIndex,DirectMode);

	nIndex = m_comboPlayMode.AddString(strPlayMode[1]);
	m_comboPlayMode.SetItemData(nIndex,ServerMode);
	m_comboPlayMode.SetCurSel(0);
}

//Stop play
void CAlarmDeviceDlg::OnButtonStop() 
{
	// TODO: Add your control notification handler code here
	//Get video number 
	int nIndex = m_comboDispNum.GetCurSel();
	if(CB_ERR != nIndex)
	{
		int iDispNum = m_comboDispNum.GetItemData(nIndex);
		//Close current video 
		CloseDispVideo(iDispNum);
		//Refresh page 
		Invalidate();
		m_DispHanle[iDispNum-1] = 0;
	}
}

//Save video information 
void CAlarmDeviceDlg::SetPlayVideoInfo(int iDispNum,int iChannel,enum RealPlayMode ePlayMode)
{
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		CString strDvrIP = GetDvrIP();
		WORD wDVRPort=(WORD)m_DvrPort;
		m_videoNodeInfo[iDispNum-1].SetVideoInfo(iDispNum,strDvrIP,wDVRPort,iChannel,m_DvrUserName,m_DvrPassword,ePlayMode);
	}
}

//PTZ control 
void CAlarmDeviceDlg::PtzControl(int type, BOOL stop)
{
	SetDlgItemText(IDC_PTZSTATUS,"");
	if(0 != m_LoginID)
	{
		//Get channel number 
		UpdateData(TRUE);
		CString strDispNum;
		m_comboDispNum.GetWindowText(strDispNum);
		int iDispNum = atoi(strDispNum);
		int iChannel=m_videoNodeInfo[iDispNum-1].GetDvrChannel();
		if(-1 == iChannel)
		{
			return;
		}
		BYTE param1=0,param2=0;
		BYTE bPTZData=(unsigned char)GetDlgItemInt(IDC_COMBO_PTZDATA);
		switch(type) {
		case DH_PTZ_UP_CONTROL:
			//Up
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_DOWN_CONTROL:
			//Down
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_LEFT_CONTROL:
			//Left 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_RIGHT_CONTROL:
			//Right 
			param1=0;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_LEFTTOP:
			//Up left
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_LEFTDOWN:
			//Up down 
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_RIGHTTOP:
			//Up right 
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_RIGHTDOWN:
			//Up down 
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_PTZ_ZOOM_DEC_CONTROL:
			//Zoom out 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_ZOOM_ADD_CONTROL:
			//Zoom in 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_FOCUS_DEC_CONTROL:
			//Focus zoom in 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_FOCUS_ADD_CONTROL:
			//Focus zoom out 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_APERTURE_DEC_CONTROL:
			//Aperture zoom out 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_APERTURE_ADD_CONTROL:
			//Aperture zoom in 
			param1=0;
			param2=bPTZData;
			break;
		default:
			break;
		}
		BOOL bRet=CLIENT_DHPTZControl(m_LoginID,iChannel,type,param1,param2,0,stop);
		if(bRet)
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Succeed"));
		}
		else
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Fail"));
		}
	}
}

//Dialogue box zoom 
void CAlarmDeviceDlg::OnBtnPtzexctrl() 
{
	// TODO: Add your control notification handler code here
	CString str;
	if(GetDlgItemText(IDC_BTN_PTZEXCTRL,str),(str=="Advance>>" || str == ConvertString("Advance>>")))
	{
		SetDlgItemText(IDC_BTN_PTZEXCTRL, ConvertString("Close Advance<<"));
		SetWindowPos(NULL,0,0,m_rectLarge.Width(),m_rectLarge.Height(),SWP_NOMOVE | SWP_NOZORDER);
		GetDlgItem(IDC_SEPERATOR)->ShowWindow(SW_SHOW);
	}
	else
	{
		SetDlgItemText(IDC_BTN_PTZEXCTRL, ConvertString("Advance>>"));
		SetWindowPos(NULL,0,0,m_rectSmall.Width(),m_rectSmall.Height(),SWP_NOMOVE | SWP_NOZORDER);
		GetDlgItem(IDC_SEPERATOR)->ShowWindow(SW_HIDE);
	}
}

//PTZ control command definition setup 
void CAlarmDeviceDlg::InitPTZControl()
{
	m_ptz_up.SetButtonCommand(DH_PTZ_UP_CONTROL);
	m_ptz_down.SetButtonCommand(DH_PTZ_DOWN_CONTROL);
	m_ptz_left.SetButtonCommand(DH_PTZ_LEFT_CONTROL);
	m_ptz_right.SetButtonCommand(DH_PTZ_RIGHT_CONTROL);
	m_zoom_wide.SetButtonCommand(DH_PTZ_ZOOM_DEC_CONTROL);
	m_zoom_tele.SetButtonCommand(DH_PTZ_ZOOM_ADD_CONTROL);
	m_focus_near.SetButtonCommand(DH_PTZ_FOCUS_DEC_CONTROL);
	m_focus_far.SetButtonCommand(DH_PTZ_FOCUS_ADD_CONTROL);
	m_iris_open.SetButtonCommand(DH_PTZ_APERTURE_ADD_CONTROL);
	m_iris_close.SetButtonCommand(DH_PTZ_APERTURE_DEC_CONTROL);
	
	m_ptz_rightup.SetButtonCommand(DH_EXTPTZ_RIGHTTOP);
	m_ptz_rightdown.SetButtonCommand(DH_EXTPTZ_RIGHTDOWN);
	m_ptz_leftup.SetButtonCommand(DH_EXTPTZ_LEFTTOP);
	m_ptz_leftdown.SetButtonCommand(DH_EXTPTZ_LEFTDOWN);
}

//PTZ control extensive function 
void CAlarmDeviceDlg::PtzExtControl(DWORD dwCommand, DWORD dwParam)
{
	if(0 != m_LoginID)
	{
		//Get channel number 
		CString strDispNum;
		CString strAuxNo;
		m_comboDispNum.GetWindowText(strDispNum);
		int iDispNum = atoi(strDispNum);
		int iChannel=m_videoNodeInfo[iDispNum-1].GetDvrChannel();
		if(-1 == iChannel)
		{
			return;
		}
		long param1=0,param2=0,param3=0;
		switch(dwCommand) {
		case DH_PTZ_POINT_MOVE_CONTROL:
			//Go to preset 
			param1=0;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_PTZ_POINT_SET_CONTROL:
			//Add preset 
			param1=0;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_PTZ_POINT_DEL_CONTROL:
			//Delete preset 
			param1=0;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_PTZ_POINT_LOOP_CONTROL:
			//Begin tour/stop tour 
			if (0 == dwParam) 
			{
				param1=(long)m_crviseGroup;
				param2=0;
				param3=76;
			}
			else
			{
				param1=(long)m_crviseGroup;
				param2=0;
				param3=96;
			}
			break;
		case DH_EXTPTZ_ADDTOLOOP:
			//Add tour 
			param1=(long)m_crviseGroup;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_EXTPTZ_DELFROMLOOP:
			//Delete tour 
			param1=(long)m_crviseGroup;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_EXTPTZ_CLOSELOOP:
			//Delete tour group 
			param1=(long)m_crviseGroup;
			param2=0;//(long)m_presetData;
			param3=0;
			break;
		case DH_EXTPTZ_SETMODESTART:
			//begin record 
			param1=(long)m_moveNo;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_SETMODESTOP:
			//Stop record 
			param1=(long)m_moveNo;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_RUNMODE:
			//begin pattern 
			param1=(long)m_moveNo;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STOPMODE:
			//Stop pattern 
			param1=(long)m_moveNo;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_DELETEMODE:
			//Delete pattern 
			param1=(long)m_moveNo;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_SETLEFTBORDER:
			//set left limit
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_SETRIGHTBORDER:
			//set right limit
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STARTLINESCAN:
			//begin scan 
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_CLOSELINESCAN:
			//Stop scan 
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_FASTGOTO:
		case DH_EXTPTZ_EXACTGOTO:
			//3D intelligent position 
			param1=m_posX;
			param2=m_posY;
			param3=m_posZoom;
			break;
		case DH_EXTPTZ_RESETZERO:
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STARTPANCRUISE:
			//Begin rotation
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STOPPANCRUISE:
			//Stop rotation 
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_AUXIOPEN:
			//Enable auxilian function 
			m_auxNosel.GetWindowText(strAuxNo);
			param1=atoi(strAuxNo);
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_AUXICLOSE:
			//Stop auxilian function
			m_auxNosel.GetWindowText(strAuxNo);
			param1=atoi(strAuxNo);
			param2=0;
			param3=0;
			break;
		default:
			break;
		}
		BOOL bRet=CLIENT_DHPTZControlEx(m_LoginID,iChannel,dwCommand,param1,param2,param3,FALSE);
		if(bRet)
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Succeed"));
		}
		else
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Fail"));
		}
	}
}

//Go to preset 
void CAlarmDeviceDlg::OnPresetSet() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_PTZ_POINT_MOVE_CONTROL);
	}
}

//Add preset 
void CAlarmDeviceDlg::OnPresetAdd() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_PTZ_POINT_SET_CONTROL);
	}
}

//Delete preset 
void CAlarmDeviceDlg::OnPresetDele() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_PTZ_POINT_DEL_CONTROL);
	}
}

//Begin tour 
void CAlarmDeviceDlg::OnStartCruise() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		CString str;
		if(GetDlgItemText(IDC_START_CRUISE,str),(str == "Start Tour" || str == ConvertString("Start Tour")))
		{
			SetDlgItemText(IDC_START_CRUISE, ConvertString("Stop Tour"));
			PtzExtControl(DH_PTZ_POINT_LOOP_CONTROL, 0);
		}
		else
		{
			SetDlgItemText(IDC_START_CRUISE, ConvertString("Start Tour"));
			PtzExtControl(DH_PTZ_POINT_LOOP_CONTROL, 1);
		}
	}
}

//Add tour 
void CAlarmDeviceDlg::OnCruiseAddPoint() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_ADDTOLOOP);
	}
}

//Delete tour point
void CAlarmDeviceDlg::OnCruiseDelPoint() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_DELFROMLOOP);
	}
}

//Delete tour group 
void CAlarmDeviceDlg::OnCruiseDelGroup() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_CLOSELOOP);
	}
}

//Begin record 
void CAlarmDeviceDlg::OnModeSetBegin() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		CString str;
		if(GetDlgItemText(IDC_MODE_SET_BEGIN,str),(str == "Program Start" || str == ConvertString("Program Start")))
		{
			SetDlgItemText(IDC_MODE_SET_BEGIN, ConvertString("Program Stop"));
			PtzExtControl(DH_EXTPTZ_SETMODESTART);
		}
		else
		{
			SetDlgItemText(IDC_MODE_SET_BEGIN, ConvertString("Program Start"));
			PtzExtControl(DH_EXTPTZ_SETMODESTOP);
		}
	}
}

//Begin pattern 
void CAlarmDeviceDlg::OnModeStart() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		CString str;
		if(GetDlgItemText(IDC_MODE_START,str), (str =="Start Pattern" || str == ConvertString("Start Pattern")))
		{
			SetDlgItemText(IDC_MODE_START, ConvertString("Stop Pattern"));
			PtzExtControl(DH_EXTPTZ_RUNMODE);
		}
		else
		{
			SetDlgItemText(IDC_MODE_START, ConvertString("Start Pattern"));
			PtzExtControl(DH_EXTPTZ_STOPMODE);
		}
	}
}

//Delete pattern 
void CAlarmDeviceDlg::OnModeSetDelete() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_DELETEMODE);
	}
}

//Set left limit
void CAlarmDeviceDlg::OnLineSetLeft() 
{
	// TODO: Add your control notification handler code here
	PtzExtControl(DH_EXTPTZ_SETLEFTBORDER);
}

//Set right limit 
void CAlarmDeviceDlg::OnLineSetRight() 
{
	// TODO: Add your control notification handler code here
	PtzExtControl(DH_EXTPTZ_SETRIGHTBORDER);
}

//Begin scan 
void CAlarmDeviceDlg::OnLineStart() 
{
	// TODO: Add your control notification handler code here
	CString str;
	if(GetDlgItemText(IDC_LINE_START,str),(str == "Start" || str == ConvertString("Start")))
	{
		SetDlgItemText(IDC_LINE_START, ConvertString("Stop"));
		PtzExtControl(DH_EXTPTZ_STARTLINESCAN);	
	}
	else
	{
		SetDlgItemText(IDC_LINE_START, ConvertString("Start"));
		PtzExtControl(DH_EXTPTZ_CLOSELINESCAN);
	}
}

//3D intelligent position 
void CAlarmDeviceDlg::OnFastGo() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_FASTGOTO);
	}
}

void CAlarmDeviceDlg::OnExactGo() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_EXACTGOTO);
	}
}


void CAlarmDeviceDlg::OnResetZero() 
{
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		PtzExtControl(DH_EXTPTZ_RESETZERO);
	}
}


//Begin rotation 
void CAlarmDeviceDlg::OnRotateStart() 
{
	// TODO: Add your control notification handler code here
	PtzExtControl(DH_EXTPTZ_STARTPANCRUISE);
}

//Stop rotation 
void CAlarmDeviceDlg::OnRotateStop() 
{
	// TODO: Add your control notification handler code here
	PtzExtControl(DH_EXTPTZ_STOPPANCRUISE);
}

//Enable auxilian function 
void CAlarmDeviceDlg::OnAuxOpen() 
{
	// TODO: Add your control notification handler code here
	PtzExtControl(DH_EXTPTZ_AUXIOPEN);
}

//Close auxilian function
void CAlarmDeviceDlg::OnAuxClose() 
{
	// TODO: Add your control notification handler code here
	PtzExtControl(DH_EXTPTZ_AUXICLOSE);
}

void CAlarmDeviceDlg::OnBtnPtzmenu() 
{
	// TODO: Add your control notification handler code here
	if(0 == m_LoginID)
	{
		MessageBox(ConvertString("Please login first !"), ConvertString("Prompt"));
	}
	else
	{
		CDHPtzMenu dlg;
		CString strChannel;
		m_comboChannel.GetWindowText(strChannel);
		int iChannel=atoi(strChannel);
		if (iChannel > 0)
		{
			iChannel -= 1;
		}
		dlg.SetPtzParam(m_LoginID, iChannel);
		dlg.DoModal();
	}
}

//Play video directly 
void CAlarmDeviceDlg::DirectPlayMode(int iDispNum,int iChannel,HWND hWnd)
{
	//Close current video 
	CloseDispVideo(iDispNum);
	
	LLONG lRet = CLIENT_RealPlayEx(m_LoginID,iChannel,hWnd);
	if(0 != lRet)
	{
		m_DispHanle[iDispNum-1]=lRet;
		SetPlayVideoInfo(iDispNum,iChannel,DirectMode);
        CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
        pWnd->SetWndPlay(true);
	}
	else
	{
		MessageBox(ConvertString("Fail to play!"), ConvertString("Prompt"));
	}
}

void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize, LLONG lParam, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CAlarmDeviceDlg *dlg = (CAlarmDeviceDlg *)dwUser;
	dlg->ReceiveRealData(lRealHandle,dwDataType, pBuffer, dwBufSize, lParam);
}

//Play video in data callback mode 
void CAlarmDeviceDlg::ServerPlayMode(int iDispNum, int iChannel, HWND hWnd)
{
	//Close current video 
	CloseDispVideo(iDispNum);
	
	//Enable stream
	BOOL bOpenRet = s_PlayAPI.PLAY_OpenStream(iDispNum,0,0,1024*900);
	if(bOpenRet)
	{
		//Begin play 
		BOOL bPlayRet = s_PlayAPI.PLAY_Play(iDispNum,hWnd);
		if(bPlayRet)
		{
			//Real-time play 
			LLONG lRet = CLIENT_RealPlayEx(m_LoginID,iChannel,0);
			if(0 != lRet)
			{
				m_DispHanle[iDispNum-1]=lRet;
				SetPlayVideoInfo(iDispNum,iChannel,ServerMode);
				//Callback monitor data and then save 
				CLIENT_SetRealDataCallBackEx2(lRet, RealDataCallBackEx, (LDWORD)this, 0x0f);
                CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
                pWnd->SetWndPlay(true);
			}
			else
			{
				MessageBox(ConvertString("Fail to play!"), ConvertString("Prompt"));
				s_PlayAPI.PLAY_Stop(iDispNum);
				s_PlayAPI.PLAY_CloseStream(iDispNum);
			}
		}
		else
		{
			s_PlayAPI.PLAY_CloseStream(iDispNum);
		}
	}
}

//Process after receiving real-time data 
void CAlarmDeviceDlg::ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, LDWORD dwBufSize, LLONG lParam)
{
	//Stream port number according to the real-time handle.
	long lRealPort=GetStreamPort(lRealHandle);
	//Input the stream data getting from the card
	BOOL bInput=FALSE;
	if(0 != lRealPort)
	{
		switch(dwDataType) {
		case 0:
			//Original data 
			bInput = s_PlayAPI.PLAY_InputData(lRealPort,pBuffer,dwBufSize);
			break;
		case 1:
			//Standard video data 

			break;
		case 2:
			//yuv data 
			
			break;
		case 3:
			//pcm audio data 
			
			break;		
		default:
			break;
		}	
	}
}

long CAlarmDeviceDlg::GetStreamPort(LLONG lRealHandle)
{
	long lPort=0;
	for(int i=0;i<9;i++)
	{
		if(lRealHandle == m_DispHanle[i])
		{
			lPort=i+1;
			break;
		}
	}
	return lPort;
}

//Close video directly 
void CAlarmDeviceDlg::StopPlayForDirectMode(int iDispNum)
{
	if(0 != m_DispHanle[iDispNum-1])
	{
		BOOL bRet = CLIENT_StopRealPlay(m_DispHanle[iDispNum-1]);
        HWND hWnd=GetDispHandle(iDispNum);
        CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
        pWnd->SetWndPlay(false);
	}
}

//Close video by data callback way 
void CAlarmDeviceDlg::StopPlayForServerMode(int iDispNum)
{
	//First close CLIENT_RealPlay
	if(0 != m_DispHanle[iDispNum-1])
	{
		BOOL bRealPlay = CLIENT_StopRealPlay(m_DispHanle[iDispNum-1]);
		if(bRealPlay)
		{
            HWND hWnd=GetDispHandle(iDispNum);
            CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
            pWnd->SetWndPlay(false);
			//And then close PLAY_Play
			BOOL bPlay = s_PlayAPI.PLAY_Stop(iDispNum);
			if(bPlay)
			{
				//At last close PLAY_OpenStream
				BOOL bStream = s_PlayAPI.PLAY_CloseStream(iDispNum);
			}
		}
	}
}

//Multiple-window preview 
void CAlarmDeviceDlg::MultiPlayMode(int iDispNum, HWND hWnd)
{
	//Close current video 
	CloseDispVideo(iDispNum);
	
	CMultiPlay dlg;
	dlg.SetMultiPlayDlgInfo(m_nChannelCount);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		int nChannel = dlg.m_nChannel;
		int nMultiPlay = dlg.m_nMultiPlay;
		if(0 < nMultiPlay)
		{
			if(0 != m_LoginID)
			{
				LLONG lRet = 0;
				switch(nMultiPlay) {
				case 1:
					lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,hWnd,DH_RType_Multiplay_1);
					break;
				case 4:
					lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,hWnd,DH_RType_Multiplay_4);
					break;
				case 8:
					lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,hWnd,DH_RType_Multiplay_8);
					break;
				case 9:
					lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,hWnd,DH_RType_Multiplay_9);
					break;
				case 16:
					lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,hWnd,DH_RType_Multiplay_16);
					break;
				default:
					break;
				}
				if(0 != lRet)
				{
					m_DispHanle[iDispNum-1]=lRet;
					SetPlayVideoInfo(iDispNum,nChannel,MultiMode);
                    CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
                    pWnd->SetWndPlay(true);
				}
				else
				{
					MessageBox(ConvertString("Fail to play!"), ConvertString("Prompt"));
				} 
			}
		}
	}
}

//Close video by multiple-window preview 
void CAlarmDeviceDlg::StopPlayForMultiMode(int iDispNum)
{
	if(0 != m_DispHanle[iDispNum-1])
	{
		BOOL bRet = CLIENT_StopRealPlayEx(m_DispHanle[iDispNum-1]);
        HWND hWnd=GetDispHandle(iDispNum);
        CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
        pWnd->SetWndPlay(false);
	}
}

//Display log in failure reason 
void CAlarmDeviceDlg::ShowLoginErrorReason(int nError)
{
	if(1 == nError)		MessageBox(ConvertString("Invalid password!"), ConvertString("Prompt"));
	else if(2 == nError)	MessageBox(ConvertString("Invalid account!"), ConvertString("Prompt"));
	else if(3 == nError)	MessageBox(ConvertString("Timeout!"), ConvertString("Prompt"));
	else if(4 == nError)	MessageBox(ConvertString("The user has logged in!"), ConvertString("Prompt"));
	else if(5 == nError)	MessageBox(ConvertString("The user has been locked!"), ConvertString("Prompt"));
	else if(6 == nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("Prompt"));
	else if(7 == nError)	MessageBox(ConvertString("The system is busy!"), ConvertString("Prompt"));
	else if(9 == nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("Prompt"));
	else	MessageBox(ConvertString("Login failed!"), ConvertString("Prompt"));
}

//Display function execution error reason 
void CAlarmDeviceDlg::LastError()
{
	DWORD dwError = CLIENT_GetLastError();
	switch(dwError)
	{
	case NET_NOERROR:					GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NOERROR));
		break;
	case NET_ERROR:						GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_ERROR));
		break;
	case NET_SYSTEM_ERROR:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_SYSTEM_ERROR));
		break;
	case NET_NETWORK_ERROR:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NETWORK_ERROR));
		break;
	case NET_DEV_VER_NOMATCH:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_DEV_VER_NOMATCH));
		break;
	case NET_INVALID_HANDLE:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_INVALID_HANDLE));
		break;
	case NET_OPEN_CHANNEL_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_OPEN_CHANNEL_ERROR));
		break;
	case NET_CLOSE_CHANNEL_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_CLOSE_CHANNEL_ERROR));
		break;
	case NET_ILLEGAL_PARAM:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_ILLEGAL_PARAM));
		break;
	case NET_SDK_INIT_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_SDK_INIT_ERROR));
		break;
	case NET_SDK_UNINIT_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_SDK_UNINIT_ERROR));
		break;
	case NET_RENDER_OPEN_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_OPEN_ERROR));
		break;
	case NET_DEC_OPEN_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_DEC_OPEN_ERROR));
		break;
	case NET_DEC_CLOSE_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_DEC_CLOSE_ERROR));
		break;
	case NET_MULTIPLAY_NOCHANNEL:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_MULTIPLAY_NOCHANNEL));
		break;
	case NET_TALK_INIT_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_TALK_INIT_ERROR));
		break;
	case NET_TALK_NOT_INIT:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_TALK_NOT_INIT));
		break;	
	case NET_TALK_SENDDATA_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_TALK_SENDDATA_ERROR));
		break;
	case NET_NO_TALK_CHANNEL:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NO_TALK_CHANNEL));
		break;
	case NET_NO_AUDIO:					GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NO_AUDIO));
		break;								
	case NET_REAL_ALREADY_SAVING:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_REAL_ALREADY_SAVING));
		break;
	case NET_NOT_SAVING:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NOT_SAVING));
		break;
	case NET_OPEN_FILE_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_OPEN_FILE_ERROR));
		break;
	case NET_PTZ_SET_TIMER_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_PTZ_SET_TIMER_ERROR));
		break;
	case NET_RETURN_DATA_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RETURN_DATA_ERROR));
		break;
	case NET_INSUFFICIENT_BUFFER:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_INSUFFICIENT_BUFFER));
		break;
	case NET_NOT_SUPPORTED:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NOT_SUPPORTED));
		break;
	case NET_NO_RECORD_FOUND:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NO_RECORD_FOUND));
		break;	
	case NET_LOGIN_ERROR_PASSWORD:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_PASSWORD));
		break;
	case NET_LOGIN_ERROR_USER:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_USER));
		break;
	case NET_LOGIN_ERROR_TIMEOUT:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_TIMEOUT));
		break;
	case NET_LOGIN_ERROR_RELOGGIN:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_RELOGGIN));
		break;
	case NET_LOGIN_ERROR_LOCKED:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_LOCKED));
		break;
	case NET_LOGIN_ERROR_BLACKLIST:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_BLOCKLIST));
		break;
	case NET_LOGIN_ERROR_BUSY:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_BUSY));
		break;
	case NET_LOGIN_ERROR_CONNECT:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_CONNECT));
		break;
	case NET_LOGIN_ERROR_NETWORK:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_LOGIN_ERROR_NETWORK));
		break;								
	case NET_RENDER_SOUND_ON_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_SOUND_ON_ERROR));
		break;
	case NET_RENDER_SOUND_OFF_ERROR:	GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_SOUND_OFF_ERROR));
		break;
	case NET_RENDER_SET_VOLUME_ERROR:	GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_SET_VOLUME_ERROR));
		break;
	case NET_RENDER_ADJUST_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_ADJUST_ERROR));
		break;
	case NET_RENDER_PAUSE_ERROR:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_PAUSE_ERROR));
		break;
	case NET_RENDER_SNAP_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_SNAP_ERROR));
		break;
	case NET_RENDER_STEP_ERROR:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_STEP_ERROR));
		break;
	case NET_RENDER_FRAMERATE_ERROR:	GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_RENDER_FRAMERATE_ERROR));
		break;
	case NET_CONFIG_DEVBUSY:			GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_CONFIG_DEVBUSY));
		break;
	case NET_CONFIG_DATAILLEGAL:		GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_CONFIG_DATAILLEGAL));
		break;							
	case NET_NO_INIT:					GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_NO_INIT));
		break;
	case NET_DOWNLOAD_END:				GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_DOWNLOAD_END));
		break;
	default:							GetDlgItem(IDC_LAST_ERROR)->SetWindowText(ConvertString(ERROR_NET_ERROR));						
	}
}

//Data callback mode in multiple-window preview 
void CAlarmDeviceDlg::MultiPlayServerMode(int iDispNum, HWND hWnd)
{
	//Close current video 
	CloseDispVideo(iDispNum);
	
	CMultiPlay dlg;
	dlg.SetMultiPlayDlgInfo(m_nChannelCount);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		int nChannel = dlg.m_nChannel;
		int nMultiPlay = dlg.m_nMultiPlay;
		if(0 < nMultiPlay)
		{
			if(0 != m_LoginID)
			{
				//Enable stream
				BOOL bOpenRet = s_PlayAPI.PLAY_OpenStream(iDispNum,0,0,1024*900);
				if(bOpenRet)
				{
					//Begin play 
					BOOL bPlayRet = s_PlayAPI.PLAY_Play(iDispNum,hWnd);
					if(bPlayRet)
					{
						//Real-time play 
						LLONG lRet = 0;
						switch(nMultiPlay)
                        {
						case 1:
							lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,0,DH_RType_Multiplay_1);
							break;
						case 4:
							lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,0,DH_RType_Multiplay_4);
							break;
						case 8:
							lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,0,DH_RType_Multiplay_8);
							break;
						case 9:
							lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,0,DH_RType_Multiplay_9);
							break;
						case 16:
							lRet = CLIENT_RealPlayEx(m_LoginID,nChannel,0,DH_RType_Multiplay_16);
							break;
						default:
							break;
						}
						if(0 != lRet)
						{
							m_DispHanle[iDispNum-1]=lRet;
							SetPlayVideoInfo(iDispNum,nChannel,MultiServerMode);
							//Save monitor data callback
							CLIENT_SetRealDataCallBackEx2(lRet, RealDataCallBackEx, (LDWORD)this, 0x0f);
                            CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
                            pWnd->SetWndPlay(true);
						}
						else
						{
							s_PlayAPI.PLAY_Stop(iDispNum);

							s_PlayAPI.PLAY_CloseStream(iDispNum);
							MessageBox(ConvertString("Fail to play!"), ConvertString("Prompt"));
						}
					}
				}
			}
		}
	}
}

//Close video by data callback mode in multiple-window preview
void CAlarmDeviceDlg::StopPlayForMultiServer(int iDispNum)
{
	//First close CLIENT_RealPlay
	if(0 != m_DispHanle[iDispNum-1])
	{
		BOOL bRealPlay = CLIENT_StopRealPlay(m_DispHanle[iDispNum-1]);
		if(bRealPlay)
		{
            HWND hWnd=GetDispHandle(iDispNum);
            CPlayWnd* pWnd = (CPlayWnd*)FromHandle(hWnd);
            pWnd->SetWndPlay(false);
			//Then close PLAY_Play
			BOOL bPlay = s_PlayAPI.PLAY_Stop(iDispNum);
			if(bPlay)
			{
				//At last close PLAY_OpenStream
				BOOL bStream = s_PlayAPI.PLAY_CloseStream(iDispNum);
			}
		}
	}
}

//Close video 
void CAlarmDeviceDlg::CloseDispVideo(int iDispNum)
{
	//Close current video 
	enum RealPlayMode ePlayMode = m_videoNodeInfo[iDispNum-1].GetPlayMode();
	if(ePlayMode == DirectMode)
	{
		StopPlayForDirectMode(iDispNum);
	}
	else if(ePlayMode == ServerMode)
	{
		StopPlayForServerMode(iDispNum);
	}
	else if(ePlayMode == MultiMode)
	{
		StopPlayForMultiMode(iDispNum);
	}
	else if(ePlayMode == MultiServerMode)
	{
		StopPlayForMultiServer(iDispNum);
	}
}

/************************************************************************/
/* Callback function                                                      */
/************************************************************************/
void CALLBACK RectEventFunc(RECT WinRect,CPoint &pointStart,CPoint &pointEnd,LDWORD dwUser)
{
	CAlarmDeviceDlg *dlg =(CAlarmDeviceDlg *)dwUser;
	CPoint Origin;
	CPoint SendPoint;
	
	Origin.x = ( WinRect.left + WinRect.right ) / 2;
	Origin.y = ( WinRect.top + WinRect.bottom ) / 2;
	
	int dx = (pointStart.x + pointEnd.x)/2;
	int dy = (pointStart.y + pointEnd.y)/2;
	
	int width  = WinRect.right - WinRect.left;
	int height = WinRect.bottom - WinRect.top; 
	
	SendPoint.x = ( dx - Origin.x) * 8192 * 2 / width;
	SendPoint.y = ( dy - Origin.y) * 8192 * 2  / height ;
	
	int width2 = pointEnd.x - pointStart.x;
	int height2 = pointEnd.y - pointStart.y;
	int multiple = 0;

	if ( height2 !=0 && width2!=0)
	{
		multiple = (width * height) / (width2 * height2);
	}
	
	dlg->m_posX = SendPoint.x;
	dlg->m_posY = SendPoint.y;
	dlg->m_posZoom = multiple;
	dlg->UpdateData(FALSE);
	dlg->PtzExtControl(DH_EXTPTZ_FASTGOTO);
}
void CALLBACK  MessageProcFunc(int nWndID, UINT message, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	
	CAlarmDeviceDlg *dlg = (CAlarmDeviceDlg *)dwUser;
	dlg->MessageProc(nWndID, message);

}
BOOL CALLBACK  GetParamsFunc(int nWndID, int type, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return FALSE;
	}
	CAlarmDeviceDlg *dlg = (CAlarmDeviceDlg *)dwUser;
	return dlg->GetParams(nWndID, type);
}

BOOL CAlarmDeviceDlg::GetParams(int nWndID, int type)
{
	BOOL bRet = FALSE;
	
	if (type == 0)
	{
		bRet = GetExitDecode(nWndID);
	}
	else if (type == 1)
	{
		bRet = GetExitCycle(nWndID);
	}
	
	return bRet;
}

void CALLBACK SetParamsFunc(int nWndID, int type, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	
	CAlarmDeviceDlg *dlg = (CAlarmDeviceDlg *)dwUser;
	dlg->SetParams(nWndID, type);
}

void CAlarmDeviceDlg::SetParams(int nWndID, int type)
{
	if (type == 0)
	{
		SetExitDecode(nWndID);
	}
	else if (type == 1)
	{
		SetExitCycle(nWndID);
	}

}
void CAlarmDeviceDlg::MessageProc(int nWndID, UINT message)
{
	switch(message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
			m_CurScreen =nWndID;
			m_comboDispNum.SetCurSel(m_CurScreen);
			//SetCurWindId(nWndID);
			//UpdateCurScreenInfo();
		}
		break;
	default:
		break;
	}
	
}
BOOL CAlarmDeviceDlg::GetExitDecode(int nCurWndID)
{
	BOOL bRet = FALSE;
	if (nCurWndID<0 || nCurWndID>=16)
	{
		return bRet;
	}
	
	BOOL bIsTimeOut = m_cs.Lock();
	if(bIsTimeOut)
	{
		bRet = m_bWndExitDecode[nCurWndID];
		m_cs.Unlock();
	}
	
	return bRet;
}

BOOL CAlarmDeviceDlg::GetExitCycle(int nCurWndID)
{
	BOOL bRet = FALSE;
	if (nCurWndID<0 || nCurWndID>=16)
	{
		return bRet;
	}
	
	bRet = m_bWndExitCycle[nCurWndID];
	
	return bRet;
}

void CAlarmDeviceDlg::SetExitDecode(int nCurWndID)
{
	if (nCurWndID<0 || nCurWndID>=16)
	{
		return;
	}
	
	BOOL bIsTimeOut = m_cs.Lock();
	if(bIsTimeOut)
	{
		m_bWndExitDecode[nCurWndID] = !m_bWndExitDecode[nCurWndID];
		m_cs.Unlock();
	}
}

void CAlarmDeviceDlg::SetExitCycle(int nCurWndID)
{
	if (nCurWndID<0 || nCurWndID>=16)
	{
		return;
	}
	
	m_bWndExitCycle[nCurWndID] = !m_bWndExitCycle[nCurWndID];
}

void CAlarmDeviceDlg::OnSelchangeCOMBODispNum() 
{
	// TODO: Add your control notification handler code here
	m_CurScreen = m_comboDispNum.GetCurSel();
	m_ptzScreen.SetActiveWnd(m_CurScreen,TRUE);;
}

void CAlarmDeviceDlg::OnCloseupCOMBODispNum() 
{
	// TODO: Add your control notification handler code here
	
}

UINT32 CAlarmDeviceDlg::GetAnalogAlarmIn()
{
	char szBuf[1024 * 10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_LoginID, CFG_CAP_CMD_ANALOGALARM, -1, szBuf, sizeof(szBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_CAP_ANALOGALARM stuInfo = {0};
		int nRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_ANALOGALARM, szBuf, &stuInfo, sizeof(stuInfo), &nRet);
		if (bRet && nRet == sizeof(CFG_CAP_ANALOGALARM))
		{
			return stuInfo.dwLocalNum;
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////

void CAlarmDeviceDlg::AlarmCenterCfg()
{
	CDlgCfgAlarmCenter dlg(this, m_LoginID);
 	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmOutCfg()
{
	DlgCfgAlarmOut dlg(this, m_LoginID, m_uiAlarmOut);
	dlg.DoModal();
}

void CAlarmDeviceDlg::CommGlobalCfg()
{
 	CDlgCfgGuard dlg(this, m_LoginID);
 	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmCfg()
{
	CDlgCfgAlarm dlg(this, m_LoginID, m_uiAlarmIn);
	dlg.DoModal();
}

void CAlarmDeviceDlg::PSTN_Alarm_Server()
{	
	CDlgCfgPSTNAlarmServer dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmPowerFault()
{
	CDlgCfgAlarmPowerFault dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::BatteryLowPower()
{
	CDlgCfgBatteryLowPower dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmChassisIntrusion()
{
	CDlgCfgAlarmChassisIntrusion dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmExAlarmBox()
{
	CDlgCfgAlarmExAlarmBox dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmExAlarmInput()
{
	CDlgCfgExAlarmInput dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmExAlarmOutput()
{
	CDlgCfgExAlarmOutput dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmExAlarmChannel()
{
	CDlgExAlarmChannel dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::OnBtnConfig() 
{
	// TODO: Add your control notification handler code here
	
	tagEMConfigType emType = (tagEMConfigType)m_cmbConfig.GetCurSel();
	switch(emType)
	{
	case emConfigType_Alarm:
		AlarmCfg();
		break;
	case emConfigType_AlarmCenter:
		AlarmCenterCfg();
		break;
	case emConfigType_AlarmChassisIntrusion:
		AlarmChassisIntrusion();
		break;
	case emConfigType_AlarmExAlarmBox:
		AlarmExAlarmBox();
		break;
	case emConfigType_AlarmExAlarmInput:
		AlarmExAlarmInput();
		break;
	case emConfigType_AlarmExAlarmOutput:
		AlarmExAlarmOutput();
		break;
	case emConfigType_AlarmKeyboard:
		AlarmKeyboard();
		break;
	case emConfigType_AlarmSubSystem:
		AlarmSubSystemCfg();
		break;
	case emConfigType_AlarmOut:
		AlarmOutCfg();
		break;
	case emConfigType_AlarmPowerFault:
		AlarmPowerFault();
		break;
	case emConfigType_AnalogAlarm:
		AnalogAlarmCfg();
		break;
	case emConfigType_BatteryLowPower:
		BatteryLowPower();
		break;
	case emConfigType_CommGlobal:
		CommGlobalCfg();
		break;
	case emConfigType_Locales:
		Locales();
		break;
	case emConfigType_Mobile:
		Mobile();
		break;
	case emConfigType_Network:
		NetworkCfg();
		break;
	case emConfigType_NTP:
		NTP();
		break;
	case emConfigType_PhoneEventNotify:
		PhoneEventNotify();
		break;
	case emConfigType_PSTN_Alarm_Server:
		PSTN_Alarm_Server();
		break;
	case emConfigType_SensorSampling:
		SensorSamplingCfg();
		break;
	case emConfigType_Serial:
		SerialCfg();
		break;
	case emConfigType_WireLess:
		WirelessCfg();
		break;
	case emConfigType_AutoMainTain:
		AutoMainTain();
		break;
	case emConfigType_SetNetworkParam:
		SetNetworkParam();
		break;
    case emConfigType_SnapLinkage:
        SnapLinkage();
        break;
	case emConfigType_STP:
		STPCfg();
		break;
	case emConfigType_Urgency:
		Urgency();
		break;
	case emConfigType_VW_SensorInfo:
		VW_SensorInfo();
		break;
    case emConfigType_AirCondition:
		AirConditionCfg();
        break;
    case emConfigType_NetAbort:
        NetAbortCfg();
        break;
    case emConfigType_IPConflict:
        IPConflictCfg();
        break;
    case emConfigType_MacConflict:
        MacConflictCfg();
        break;
    case emConfigType_PSTNBreakLine:
        PSTNBreakLineCfg();
        break;
	case emConfigType_NetCollection:
		NetCollectionCfg();
		break;
	case emConfigType_AlarmSlotBond:
		AlarmSlotBondCfg();
		break;
    case emConfigType_RCEmergencyCall:
        RCEmergencyCallCfg();
        break;
    case emConfigType_AlarmBell:
        AlarmBellCfg();
        break;
    case emConfigType_PSTNPersonServer:
        PSTNPersonServerCfg();
        break;
    case emConfigType_ArmLink:
        ArmLinkCfg();
        break;
	default:
		break;
	}
}

void CAlarmDeviceDlg::OnBtnCap() 
{
	// TODO: Add your control notification handler code here
	CDlgCapability dlg(this, m_LoginID, m_uiAlarmIn, m_uiAlarmOut);
	dlg.DoModal();
}

void CAlarmDeviceDlg::OnBtnSubscribe() 
{
	// TODO: Add your control notification handler code here
	AlarmSubscribe dlg(this, m_LoginID, m_uiAlarmIn);
	dlg.DoModal();
}

void CAlarmDeviceDlg::Locales()
{
	CDlgCfgLocales dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::NTP()
{
	CDlgCfgNTP dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::Mobile()
{
	CDlgCfgMobile dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::PhoneEventNotify()
{
	CDlgCfgPhoneNotify dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::NetworkCfg()
{
	CDlgCfgNetwork dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::SerialCfg()
{
	CDlgCfgSerial dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::WirelessCfg()
{
	CDlgCfgWireless dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AutoMainTain()
{
	CDlgCfgAutoMaintain dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::Urgency()
{
	CDlgCfgAlarmUrgency dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::VW_SensorInfo()
{
	CDlgCfgSensorInfoOfVW dlg(this, m_LoginID, m_nChannelCount);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AirConditionCfg()
{
    CDlgCfgAirCondition dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::NetAbortCfg()
{
    CDlgCfgNetAbort dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::IPConflictCfg()
{
    CDlgCfgIPConflict dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::MacConflictCfg()
{
    CDlgCfgMacConflict dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::PSTNBreakLineCfg()
{
    CDlgCfgPSTNBreakLine dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::PSTNPersonServerCfg()
{
    CDlgCfgPSTNPersonServer dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::ArmLinkCfg()
{
    DlgCfgArmLink dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::OnBtnControlQuery() 
{
	// TODO: Add your control notification handler code here
	switch ((emControlQueryType)m_cmbControlQueryType.GetCurSel())
	{
	case emControlType_AccessManager:
		AccessManager();
		break;
	case emControlType_AlarmBell:
		AlarmBell();
		break;
	case emControlType_AlarmChannels:
		AlarmChannels();
		break;
	case emControlType_AlarmChannelState:
		AlarmChannelState();
		break;
    case emControlType_AlarmOutSet:
        AlarmOutSet();
        break;
// 	case emControlType_AlarmKeyboard:
// 		AlarmKeyboard();
// 		break;
	case emControlType_AnalogAlarmChannels:
		GetAnalogAlarmChannels();
		break;
	case emControlType_AnalogAlarmData:
		AnalogAlarmData();
		break;
	case emControlType_AudioPlay:
		PlaySound();
		break;
	case emControlType_FileManager:
		FileManagerCfg();
		break;
	case emControlType_FileName:
		FileName();
		break;
	case emControlType_Log:
		Log();
		break;
	case emControlType_ModifyPasswd:
		ModifyPsw();
		break;
	case emControlType_OpenAccess:
		OpenAccess();
		break;
	case emControlType_PowerState:
		PowerState();
		break;
    case emControlType_RecoverConfig:
        RestoreCfg();
        break;
	case emControlType_Version:
		Version();
		break;
	case emControlType_UserList:
		UserList();
		break;
	case emControlType_Upgrade:
		Upgrade();
		break;
	case emControlType_SetBypass:
		SetBypassMode();
		break;
	case emControlType_DeviceTime:
		DeviceTime();
		break;
	case emControlType_AlarmExAlarmChannel:
		AlarmExAlarmChannel();
		break;
	case emControlType_ActivatedDefenceArea:
		ActivatedDefenceArea();
		break;
// 	case emControlType_AttachCfgChange:
// 		AttachCfgChange();
// 		break;
	case emControlType_SetArmMode:
		SetArmMode();
		break;
// 	case emControlType_QueryLog:
// 		QueryLog();
// 		break;
	case emControlType_UserManager:
		UserManager();
		break;
	case emControlType_AlarmSubSystem:
		AlarmSubSystem();
		break;
    case emControlType_AirConditonOperate:
        AirConditionOperate();
        break;
	case emControlType_LowRateWPAN:
		GetLowRateWPAN();
		break;
	default:
		break;
	}
}

void CAlarmDeviceDlg::AccessManager()
{
	CDlgAccess dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmBell()
{
	CDlgAlarmBell dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmChannelState()
{
	CDlgAlarmState dlg(this, (LLONG)m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmKeyboard()
{
	CDlgCfgAlarmKeyboard dlg(this, (LLONG)m_LoginID, 256);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmSubSystemCfg()
{
	CDlgCfgAlarmSubSystem dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::PlaySound()
{
	CDlgOptPlaysound dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::FileManagerCfg()
{
	CDlgFileManager dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::FileName()
{
	CDlgFileList dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::Log()
{
	CDlgQueryLog dlg(this, m_LoginID, m_emDeviceType);
	dlg.DoModal();
}

void CAlarmDeviceDlg::ModifyPsw()
{
	CDlgUserManage dlg(this, m_LoginID, m_emDeviceType);
	dlg.DoModal();
}

void CAlarmDeviceDlg::OpenAccess()
{
	CDlgOptAccess dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::PowerState()
{
	CDlgStateBattery dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::RestoreCfg()
{
    CDlgRecoverConfig dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::Version()
{
	CDlgVersion dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::UserList()
{
	CDlgUserInfo dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::Upgrade()
{
	CDlgUpgrade dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::SetBypassMode()
{
	CDlgBypassCtl dlg(this, m_LoginID, m_uiAlarmIn);
	dlg.DoModal();
}

void CAlarmDeviceDlg::DeviceTime()
{
	CDlgDeviceTime dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::ActivatedDefenceArea()
{
	CDlgActivatedDefenceArea dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::OnBtnTalk() 
{
	// TODO: Add your control notification handler code here
	CDlgTalk dlg(this, m_LoginID, m_emDeviceType);
	dlg.DoModal();	
}

void CAlarmDeviceDlg::SetNetworkParam()
{
	CDlgSetNetWorkParam dlg(this, m_LoginID);
	dlg.DoModal();
}

// void CClientDemoDlg::AttachCfgChange()
// {
// 	CDlgCfgChange dlg(this, m_LoginID);
// 	dlg.DoModal();
// }

void CAlarmDeviceDlg::SetArmMode()
{
	CDlgSetArmMode dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::UserManager()
{
	CTabUserManageDlg dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::SensorSamplingCfg()
{
	CDlgCfgSensorSampling dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::SnapLinkage()
{
    CDlgCfgSnapLinkage dlg(this, m_LoginID, m_nChannelCount);
    dlg.DoModal();
}

void CAlarmDeviceDlg::STPCfg()
{
	CDlgCfgSTP dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::GetAnalogAlarmChannels()
{
	CDlgAnalogAlarmChannels dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AnalogAlarmCfg()
{
	CDlgCfgAnalogAlarm dlg(this, m_LoginID, m_uiAnalogAlarmIn);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AnalogAlarmData()
{
	CDlgAnalogAlarmData dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmChannels()
{
	CDlgAlarmChannel dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmSubSystem()
{
	CDlgAlarmSubSystem dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmOutSet()
{
    CDlgAlarmOutSet dlg(this, m_LoginID, m_uiAlarmOut);
    dlg.DoModal();
}

void CAlarmDeviceDlg::AirConditionOperate()
{
    CAirConditionOperate dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::NetCollectionCfg()
{
	CDlgCfgNetCollection dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmSlotBondCfg()
{
	CDlgCfgAlarmSlotBond dlg(this, m_LoginID);
	dlg.DoModal();
}

void CAlarmDeviceDlg::RCEmergencyCallCfg()
{
    CDlgCfgRCEmergencyCall dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::AlarmBellCfg()
{
    CDlgCfgAlarmBell dlg(this, m_LoginID);
    dlg.DoModal();
}

void CAlarmDeviceDlg::GetLowRateWPAN()
{
	CDlgLowRateWPAN dlg(this, m_LoginID);
	dlg.DoModal();
}


//Process when device disconnected 
LRESULT CAlarmDeviceDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CAlarmDeviceDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("AlarmDevice"));
	return 0;
}