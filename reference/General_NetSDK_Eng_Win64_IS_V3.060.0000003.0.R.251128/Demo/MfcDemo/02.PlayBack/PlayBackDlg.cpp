// ClientDemo3Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlayBack.h"
#include "PlayBackDlg.h"
#include "MessageText.h"
#include "dhplay.h"
#include "PlayApi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMER_MSEC 62.5
#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

CPlayAPI g_PlayAPI;
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
// PlayBackDlg dialog

CPlayBackDlg::CPlayBackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayBackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(PlayBackDlg)
	m_strUserName = _T("admin");
	m_strPwd = _T("admin1234");
	m_nPort = 37777;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_LoginID = 0;
	m_nChannelCount = 0;
	m_hPlayBack = 0;
	m_dwTotalSize = 0;
	m_dwCurValue = 0;
	m_nTimer = 0;
	m_pDownLoadFile = 0;
	m_pDownLoadByTime = 0;
	m_PlayBackType = EM_TIMEBACK;
	m_ePlayMode = EM_DIRECTMODE;
	m_bSpeedCtrl = FALSE;
	m_nChannelID =  -1;
    m_eSpeed = SPEED_NORMAL;
	m_nPlayDirection = 0;

	m_pPlaybackTimeCtl = new CPlayBackTimeView(this);

	memset(&m_PlayBackByTimeStart, 0, sizeof(m_PlayBackByTimeStart));
	memset(&m_PlayBackByTimeEnd, 0, sizeof(m_PlayBackByTimeEnd));
	memset(&m_MultyPlayBackParam,0,sizeof(m_MultyPlayBackParam));
	memset(&m_stuBeginTime, 0, sizeof(m_stuBeginTime));
	m_MultyPlayBackParam.dwSize = sizeof(m_MultyPlayBackParam);

	m_bPlayBackByTimeMode = 0;
	m_StartTime = 0;
	m_TotalTime = 0;
	m_pTabRecordDlg = NULL;
	m_pTabTimeDlg = NULL;
	m_nReverseMode = 0;
	m_pCurrentTime = 0;
}

void CPlayBackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PlayBackDlg)
	DDX_Control(pDX, IDC_TAB1, m_Tab);
	DDX_Control(pDX, IDC_IPADDRESS_DVRIP, m_ctlDvrIP);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_EDIT_DVRPWD, m_strPwd);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_PLAYBACKVIEW_PARENT, m_timeCtlParentView);
	DDX_Control(pDX, IDC_STATIC_TEST, m_staticTest);
	DDX_Control(pDX, IDC_STATIC_SHOWTIME, m_showPlaybackTime);
}

BEGIN_MESSAGE_MAP(CPlayBackDlg, CDialog)
	//{{AFX_MSG_MAP(PlayBackDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, OnBtnLogout)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_PLAY, OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_SLOW, OnButtonSlow)
	ON_BN_CLICKED(IDC_BUTTON_FAST, OnButtonFast)
	ON_BN_CLICKED(IDC_BUTTON_NORMALSPEED, OnButtonNormalspeed)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_PICTURE, OnButtonCapturePicture)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PlayBackDlg message handlers

BOOL CPlayBackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	g_PlayAPI.LoadPlayDll();
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

	RECT rect1;
	GetDlgItem(IDC_PLAYBACKVIEW_PARENT)->GetClientRect(&rect1);

	m_pPlaybackTimeCtl->Create(NULL,NULL,WS_CHILD | WS_VISIBLE | WS_TABSTOP  ,
		rect1, this->GetDlgItem(IDC_PLAYBACKVIEW_PARENT), NULL/*ID_PLAYBACK_CONTROL*/);
	m_pPlaybackTimeCtl->SetFocus();


	m_pPlaybackTimeCtl->SetShowTimeCtl(&m_showPlaybackTime);
	
	

	// TODO: Add extra initialization here
	//Device default IP
    m_ctlDvrIP.SetAddress(172, 23, 12, 146);
	InitTabControl();
	InitNetSDK();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPlayBackDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPlayBackDlg::OnPaint() 
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
HCURSOR CPlayBackDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//Log in user to the device
void CPlayBackDlg::OnBtnLogin() 
{
	// TODO: Add your control notification handler code here
	SetWindowText(ConvertString("PlayBack"));
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		char *pchDVRIP;
		CString strDvrIP = GetDvrIP();
		pchDVRIP = (LPSTR)(LPCSTR)strDvrIP;
		WORD wDVRPort=(WORD)m_nPort;
		char *pchUserName=(LPSTR)(LPCSTR)m_strUserName;
		char *pchPassword=(LPSTR)(LPCSTR)m_strPwd;

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

		if(0 == lRet)
		{
			//Display log in failed reason
			ShowLoginErrorReason(stOutparam.nError);
		}
		else
		{
			m_LoginID = lRet;
			int nRetLen = 0;
			NET_DEV_CHN_COUNT_INFO stuChn = { sizeof(NET_DEV_CHN_COUNT_INFO) };
			stuChn.stuVideoIn.dwSize = sizeof(stuChn.stuVideoIn);
			stuChn.stuVideoOut.dwSize = sizeof(stuChn.stuVideoOut);
			BOOL bGetChnCnt = CLIENT_QueryDevState(m_LoginID, DH_DEVSTATE_DEV_CHN_COUNT, (char*)&stuChn, stuChn.dwSize, &nRetLen);
	        if (bGetChnCnt)
			{
				m_nChannelCount = __max(stOutparam.stuDeviceInfo.nChanNum,stuChn.stuVideoIn.nMaxTotal);
			}
			else
			{
				m_nChannelCount = stOutparam.stuDeviceInfo.nChanNum;
			}
	
			m_pTabTimeDlg->InitComboBox(m_nChannelCount,m_LoginID);
			m_pTabRecordDlg->InitComboBox(m_nChannelCount,m_LoginID);
			

			GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);  

			DH_DEV_ENABLE_INFO stuEnableInfo = {0};
			nRetLen = 0;
			BOOL bRet = CLIENT_QuerySystemInfo(m_LoginID, ABILITY_DEVALL_INFO, (char*)&stuEnableInfo, sizeof(DH_DEV_ENABLE_INFO), &nRetLen);
			if (bRet)
			{
				if(stuEnableInfo.IsFucEnable[EN_PLAYBACK_SPEED_CTRL] != 0)
				{
					m_bSpeedCtrl = TRUE;
				}
			}

            int nSelect = m_Tab.GetCurSel();
            if(nSelect>=0)
            {
                DoTab(nSelect);
	        }
		}
	}
}

//Log off device user
void CPlayBackDlg::OnBtnLogout() 
{
	// TODO: Add your control notification handler code here
	SetWindowText(ConvertString("PlayBack"));
	OnButtonStop();
	//Close playback
	m_pPlaybackTimeCtl->UninitTimeBar();
	//Reset process bar
	m_dwTotalSize = 0;
	m_dwCurValue = 0;
	//Close download
	if(0 != m_pDownLoadFile)
	{
		CLIENT_StopDownload(m_pDownLoadFile);
		m_pDownLoadFile = 0;
	}

	if(0 != m_pDownLoadByTime)
	{
		CLIENT_StopDownload(m_pDownLoadByTime);
		m_pDownLoadByTime = 0;
	}

	BOOL bRet = CLIENT_Logout(m_LoginID);
	//Log off and then clear log in handle
	if(bRet)
	{
		m_LoginID=0;
		m_nChannelCount = 0;
		//Set channel menu 
		m_pTabRecordDlg->InitComboBox(m_nChannelCount,m_LoginID);
		m_pTabTimeDlg->InitComboBox(m_nChannelCount,m_LoginID);

        //Log off and then clear
        m_pTabRecordDlg->Clean();
        m_pTabTimeDlg->Clean();

		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_CAPTURE_PICTURE)->EnableWindow(FALSE);
        
	}
	else
	{
		MessageBox(ConvertString("Logout failed!"), ConvertString("Prompt"));
	}
}

//Callback function when device disconnected
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CPlayBackDlg *pThis = (CPlayBackDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
}

void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CPlayBackDlg *pThis = (CPlayBackDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}

//Initialize net SDK
void CPlayBackDlg::InitNetSDK()
{
	BOOL ret = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	if (!ret)
	{
		MessageBox(ConvertString("Initialize SDK failed!"), ConvertString("Prompt"));
	}
	else
	{
		LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
		CLIENT_LogOpen(&stLogPrintInfo);
		NET_PARAM stuNetParam = {0};
		stuNetParam.nGetConnInfoTime = 5000;
		CLIENT_SetNetworkParam(&stuNetParam);
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
    
}


//Process when device disconnected 
LRESULT CPlayBackDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device disconnected
	SetWindowText(ConvertString("Network disconnected!"));
	return 0;
}

//Process when device reconnect
LRESULT CPlayBackDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device reconnect
	SetWindowText(ConvertString("PlayBack"));
	return 0;
}


void CPlayBackDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if(0 != m_hPlayBack)
	{
		CLIENT_StopPlayBack(m_hPlayBack);
	}

	if(0 != m_pDownLoadFile)
	{
		//Close download
		CLIENT_StopDownload(m_pDownLoadFile);
	}
	if(0 != m_pDownLoadByTime)
	{
		CLIENT_StopDownload(m_pDownLoadByTime);
	}

	//Log off
	if(0 != m_LoginID)
	{
		BOOL bLogout = CLIENT_Logout(m_LoginID);
	}
	
	//Clear SDK and then release occupied resources.
	CLIENT_Cleanup();

	if (m_pTabRecordDlg)
	{
		m_pTabRecordDlg->DestroyWindow();
		delete m_pTabRecordDlg;
	}

	if (m_pTabTimeDlg)
	{
		m_pTabTimeDlg->DestroyWindow();
		delete m_pTabTimeDlg;
	}
}

//Get DVR IP address
CString CPlayBackDlg::GetDvrIP()
{
	CString strRet="";
	BYTE nField0,nField1,nField2,nField3;
	m_ctlDvrIP.GetAddress(nField0,nField1,nField2,nField3);
	strRet.Format("%d.%d.%d.%d",nField0,nField1,nField2,nField3);
	return strRet;
}

//Display log in failure reason
void CPlayBackDlg::ShowLoginErrorReason(int nError)
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

//Initialize TabControl 
void CPlayBackDlg::InitTabControl()
{
	m_pTabTimeDlg = new CPlaybackByTime();
	m_pTabRecordDlg = new CLockRecord();

	//Create no-mode dialogue box. Specified tag control is the main window of the no-mode dialogue box.
	m_pTabTimeDlg->Create(IDD_PLAYBACK_BYTIME,&m_Tab);
	m_pTabRecordDlg->Create(IDD_LOCKRECORD,&m_Tab);
	//m_pTabMultyDlg->Create(IDD_PLAYBACK_MULTY, &m_Tab);

	m_Tab.InsertItem(0, ConvertString("Playback by time"));
	m_Tab.InsertItem(1, ConvertString("Record locking"));
	
	m_Tab.SetMinTabWidth(-1);
	m_Tab.SetPadding(CSize(4,4));
	m_Tab.SetCurSel(0);
	DoTab(0);
}

void CPlayBackDlg::DoTab(int nTab)
{
	//Confirm nTab value is within the threshold.
	if (nTab < 0 || nTab > 2)
    {
        nTab = 0;
    }
    BOOL bTab[2];
    for (int i = 0; i < 2; ++i)
    {
        if (i == nTab)
        {
            bTab[i]=TRUE;
        }
        else
        {
            bTab[i]=FALSE;
        }
    }
	
	//Display or hide dialogue box
	SetDlgState(m_pTabTimeDlg,bTab[0]);
	SetDlgState(m_pTabRecordDlg,bTab[1]);
}

void CPlayBackDlg::SetDlgState(CWnd *pWnd, BOOL bShow)
{
	if(bShow)
	{
		pWnd->ShowWindow(SW_SHOW);
		//Display in the middle 
		pWnd->CenterWindow();
	}
	else
	{
		pWnd->ShowWindow(SW_HIDE);
	}
}

void CPlayBackDlg::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nSelect = m_Tab.GetCurSel();
	if(nSelect>=0)
	{
		DoTab(nSelect);
	}
	*pResult = 0;
}

//Process status callback
void CALLBACK PlayCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CPlayBackDlg *dlg = (CPlayBackDlg *)dwUser;
		dlg->ReceivePlayPos(lPlayHandle, dwTotalSize, dwDownLoadSize);
	}
}

// for test
int CALLBACK PlayBackDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CPlayBackDlg *dlg = (CPlayBackDlg *)dwUser;
		dlg->BackDataCallBack(lRealHandle, dwDataType, pBuffer, dwBufSize);
	}

	return 1;
}

void CPlayBackDlg::BackDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize)
{
	if (0 == dwDataType)
	{
		if ((0 != lRealHandle) && (NULL != pBuffer) && (0 < dwBufSize))
		{
			FILE *fPic = fopen("D:\\playback.mp4", "ab+");
			if (fPic)
			{
				fwrite(pBuffer, 1, dwBufSize, fPic);

				fclose(fPic);
			}
		}
	}
}

//Process status process 
void CPlayBackDlg::ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize)
{
	if(lPlayHandle == m_hPlayBack)
	{
		m_dwTotalSize = dwTotalSize;
		m_dwCurValue = dwDownLoadSize;
	}
	else if(lPlayHandle == m_pDownLoadFile)
	{
		//Download process in file mode
		if(-1 != dwDownLoadSize)
		{
			m_pTabRecordDlg->m_dwTotalSize = dwTotalSize;
			m_pTabRecordDlg->m_dwCurValue = dwDownLoadSize;
		}
		else
		{
			m_pTabRecordDlg->m_dwTotalSize = 0;
			m_pTabRecordDlg->m_dwCurValue = 0;
			if(0 != m_pDownLoadFile)
			{
				CLIENT_StopDownload(m_pDownLoadFile);
				m_pDownLoadFile = 0;
			}
		}
	}
}

void CPlayBackDlg::SetPlayControlButton(BOOL bFlag)
{
    GetDlgItem(IDC_BUTTON_PLAY)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(bFlag);
    GetDlgItem(IDC_BUTTON_CAPTURE_PICTURE)->EnableWindow(bFlag);

    if (EM_FILEBACK == m_PlayBackType || EM_TIMEBACK == m_PlayBackType)
    {
        GetDlgItem(IDC_BUTTON_SLOW)->EnableWindow(bFlag);
		GetDlgItem(IDC_STATIC_SPEED)->EnableWindow(bFlag);
        GetDlgItem(IDC_BUTTON_NORMALSPEED)->EnableWindow(bFlag);
        GetDlgItem(IDC_BUTTON_FAST)->EnableWindow(bFlag);
    }
}

//Stop play
void CPlayBackDlg::OnButtonStop() 
{
	// TODO: Add your control notification handler code here
	if(0 != m_hPlayBack)
	{
		ClosePlayBack();
		m_hPlayBack = 0;
		GetDlgItem(IDC_SCREEN_PLAYBACK)->Invalidate();

		COleDateTime startTime(m_PlayBackByTimeStart.dwYear, 
			m_PlayBackByTimeStart.dwMonth, 
			m_PlayBackByTimeStart.dwDay,
			m_StartTime/3600,
			(m_StartTime%3600)/60,
			m_StartTime%3600%60);
		m_pPlaybackTimeCtl->SeekTrickerByTime(startTime);
        ChangeSpeedStatus(0);
        SetPlayControlButton(FALSE);
	}

    if (EM_FILEBACK == m_PlayBackType && EM_SERVERMODE == m_ePlayMode)
    {
        SetDlgItemText(IDC_BUTTON_PLAY,ConvertString("PlayForward"));
    }
    else
    {
        SetDlgItemText(IDC_BUTTON_PLAY,ConvertString("Play"));
    }
	
	m_dwTotalSize = 0;
	m_dwCurValue = 0;

	if(0 != m_nTimer)
	{
		KillTimer(m_nTimer);
		m_nTimer = 0;
	}

	m_pPlaybackTimeCtl->UninitTimeBar();
}

#ifdef _WIN64
void CPlayBackDlg::OnTimer(UINT_PTR nIDEvent)
#elif defined(_WIN32)
void CPlayBackDlg::OnTimer(UINT nIDEvent)
#endif 
{
	if(nIDEvent == m_nTimer)
	{
		if (m_dwTotalSize != 0 && m_dwCurValue != -1)
		{
			int nIndex = m_pTabTimeDlg->m_ctlPlayMode.GetCurSel();
			if (m_ePlayMode == EM_DIRECTMODE)
			{
				NET_TIME ntTime = {0};
				NET_TIME ntTimeStart = {0};
				NET_TIME ntTimeEnd = {0};
				BOOL bRet = CLIENT_GetPlayBackOsdTime(m_hPlayBack, &ntTime, &ntTimeStart, &ntTimeEnd);
				if (bRet)
				{
					if (ntTime.dwDay == m_PlayBackByTimeStart.dwDay)
					{

						LONG lCurrentTime = ntTime.dwHour*3600 
							+ ntTime.dwMinute*60 
							+ ntTime.dwSecond;
						
					
						if (0 == m_nPlayDirection)
						{
							long lEndTime = m_PlayBackByTimeEnd.dwHour*3600
								+ m_PlayBackByTimeEnd.dwMinute*60
								+ m_PlayBackByTimeEnd.dwSecond;

							if (lCurrentTime >= lEndTime)
							{	
								OnButtonStop();
								return;
							}
						}
						else if (1 == m_nPlayDirection)
						{
							long lStartTime = m_PlayBackByTimeStart.dwHour*3600
								+ m_PlayBackByTimeStart.dwMinute*60
								+ m_PlayBackByTimeStart.dwSecond;

							if (lCurrentTime <= lStartTime)
							{
								OnButtonStop();
								return;
							}
						}
						
						
						COleDateTime tempOleTime(ntTime.dwYear, ntTime.dwMonth, ntTime.dwDay,
												ntTime.dwHour,ntTime.dwMinute,ntTime.dwSecond);

						m_pPlaybackTimeCtl->SeekTrickerByTime(tempOleTime);
						
					}
				}
			}
			else
			{
				NET_TIME nettime = {0};
				int nReturnLen = 0;
				int nsize = sizeof(NET_TIME);
				BOOL bRet = g_PlayAPI.PLAY_QueryInfo(1, PLAY_CMD_GetTime, (char *)&nettime, sizeof(NET_TIME), &nReturnLen);
				if (bRet)
				{
					if (nettime.dwDay == m_PlayBackByTimeStart.dwDay)
					{

						LONG lCurrentTime = nettime.dwHour*3600 + nettime.dwMinute*60 + nettime.dwSecond;

						if (0 == m_nPlayDirection)
						{
							long lEndTime = m_PlayBackByTimeEnd.dwHour*3600
								+ m_PlayBackByTimeEnd.dwMinute*60
								+ m_PlayBackByTimeEnd.dwSecond;

							if (lCurrentTime >= lEndTime)
							{
								//m_DayTimePickSdk.SetCurrentTime(m_StartTime);
								OnButtonStop();
								return;
							}
						}
						else if (1 == m_nPlayDirection)
						{
							long lStartTime = m_PlayBackByTimeStart.dwHour*3600
								+ m_PlayBackByTimeStart.dwMinute*60
								+ m_PlayBackByTimeStart.dwSecond;

							if (lCurrentTime <= lStartTime)
							{
								//m_DayTimePickSdk.SetCurrentTime(m_StartTime);
								OnButtonStop();
								return;
							}
						}

						COleDateTime tempOleTime(nettime.dwYear, nettime.dwMonth, nettime.dwDay,
							nettime.dwHour,nettime.dwMinute,nettime.dwSecond);
						m_pPlaybackTimeCtl->SeekTrickerByTime(tempOleTime);
					}
				}
			}
		}
		else if (m_dwTotalSize != 0 && m_dwCurValue == -1)
		{
			BOOL bPlayEnd = FALSE;
	//		if (m_ePlayMode == EM_DIRECTMODE)
	//		{
	//			bPlayEnd = TRUE;
	//		}

      
            if (g_PlayAPI.PLAY_GetSourceBufferRemain(1) == 0 && g_PlayAPI.PLAY_GetBufferValue(1, BUF_VIDEO_RENDER) == 0)
            {
                bPlayEnd = TRUE;
            }
            

			if (bPlayEnd)
			{
				m_dwTotalSize = 0;
				m_dwCurValue = 0;
                if (EM_FILEBACK == m_PlayBackType && EM_SERVERMODE == m_ePlayMode)
                {
                    SetDlgItemText(IDC_BUTTON_PLAY,ConvertString("PlayForward"));
                }
                else
                {
                    SetDlgItemText(IDC_BUTTON_PLAY,ConvertString("Play"));
                }

                if (0 != m_hPlayBack)
				{
					OnButtonStop();
				}
			}
		}
		else
		{

		}
	}
	
	CDialog::OnTimer(nIDEvent);
}

//Display function execution failure reason
void CPlayBackDlg::LastError()
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

//Playback in accordance with time
void CPlayBackDlg::PlayBackByTime(int nChannelID, int nPlayDirection, int nRecordFileType)
{
    if (0 != m_LoginID)
    {
        ClosePlayBack();
		m_hPlayBack = 0;

		DWORD dwStream = 0;//NET_RECORD_TYPE_ALL
		if (1 == nRecordFileType)
		{
			dwStream = NET_RECORD_TYPE_ALARM;
		}
		else if (2 == nRecordFileType)
		{
			dwStream = NET_RECORD_TYPE_MOTION;
		}

		BOOL bRet = CLIENT_SetDeviceMode(m_LoginID, DH_RECORD_TYPE, &dwStream);

        HWND hPlayBack = GetDlgItem(IDC_SCREEN_PLAYBACK)->m_hWnd;

		NET_IN_PLAY_BACK_BY_TIME_INFO stuPlayBackIn = {0};
		NET_OUT_PLAY_BACK_BY_TIME_INFO stuPlayBackOut = {0};
		stuPlayBackIn.stStartTime = m_PlayBackByTimeStart;
		stuPlayBackIn.stStopTime = m_PlayBackByTimeEnd;
		stuPlayBackIn.hWnd = hPlayBack;
		stuPlayBackIn.cbDownLoadPos = PlayCallBack;
		stuPlayBackIn.dwPosUser = (LDWORD)this;
		stuPlayBackIn.nPlayDirection = nPlayDirection;
		stuPlayBackIn.nWaittime = 5000;
		LLONG lHandle = CLIENT_PlayBackByTimeEx2(m_LoginID,nChannelID,&stuPlayBackIn, &stuPlayBackOut);
        if(0 != lHandle)
        {
            if(0 == m_nTimer)
            {
                m_nTimer = SetTimer(1, TIMER_MSEC, NULL);
            }
			m_nPlayDirection = nPlayDirection;
             m_nChannelID = nChannelID;
            m_hPlayBack = lHandle;
            m_ePlayMode = EM_DIRECTMODE;
            SetDlgItemText(IDC_BUTTON_PLAY, ConvertString("Pause"));
            m_PlayBackType = EM_TIMEBACK;
            m_nReverseMode = 0;
            ChangeSpeedStatus(0);
            SetPlayControlButton(TRUE);
        }
        else
        {
        	GetDlgItem(IDC_SCREEN_PLAYBACK)->Invalidate();
			
			if (0 == nPlayDirection)
			{
				MessageBox(ConvertString("Playback failed!"), ConvertString("Prompt"));
			}
			else if (1 == nPlayDirection)
			{
				MessageBox(ConvertString("Playback reverse failed!"), ConvertString("Prompt"));
			}
	    }
    }
}

//Turn NET_TIME type to CTime
CTime CPlayBackDlg::ConvertNetTimeToCTime(LPNET_TIME lpNetTime)
{
	int nYear = (int)lpNetTime->dwYear;
	int nMonth = (int)lpNetTime->dwMonth;
	int nDay = (int)lpNetTime->dwDay;
	int nHour = (int)lpNetTime->dwHour;
	int nMinute = (int)lpNetTime->dwMinute;
	int nSecond = (int)lpNetTime->dwSecond;
	CTime time(nYear,nMonth,nDay,nHour,nMinute,nSecond);
	return time;
}

//Download callback in time mode
void CALLBACK TimeDownLoadPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO recordfileinfo, LDWORD dwUser)
{
	CPlayBackDlg * dlg = (CPlayBackDlg *)dwUser;
	dlg->TimeDownLoadCallBack(lPlayHandle, dwTotalSize, dwDownLoadSize, index, recordfileinfo,dwUser);
}

//Download process in time mode 
void CPlayBackDlg::TimeDownLoadCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO& recordfileinfo,LDWORD dwUser)
{
	if(lPlayHandle == m_pDownLoadByTime)
	{
		if(-1 != dwDownLoadSize)
		{
			m_pTabTimeDlg->m_dwTotalSize = dwTotalSize;
			m_pTabTimeDlg->m_dwCurValue = dwDownLoadSize;
		}
		else
		{
			m_pTabTimeDlg->m_dwTotalSize = 0;
			m_pTabTimeDlg->m_dwCurValue = 0;

			CPlayBackDlg * dlg = (CPlayBackDlg *)dwUser;
			if(0 != m_pDownLoadByTime)
			{
				CLIENT_StopDownload(m_pDownLoadByTime);
				m_pDownLoadByTime = 0;
			}
			m_pTabTimeDlg->SetDlgItemText(IDC_BUTTON_DOWNLOAD, ConvertString("Download"));
			m_pTabTimeDlg->SetDlgItemText(IDC_BUTTON_DOWNLOAD2, ConvertString("DownloadByType"));
		}
	}
}


//Download in accordance with time
BOOL CPlayBackDlg::DownLoadByTime(int nChannelId, LPNET_TIME tmStart, LPNET_TIME tmEnd, BOOL bLoad)
{
	BOOL bRet =FALSE;
	if(0 == m_LoginID)
	{
		MessageBox(ConvertString("Please login first"), ConvertString("Prompt"));
		return bRet;
	}
	if(bLoad)
	{
		//Download
		if(0 != m_LoginID)
		{
			if(0 != m_pDownLoadByTime)
			{
				//Close download
				BOOL bSuccess = CLIENT_StopDownload(m_pDownLoadByTime);
				if(bSuccess)
				{
					m_pDownLoadByTime = 0;
				}
				else
				{
					MessageBox(ConvertString("Stop download failed!"), ConvertString("Prompt"));
				}
			}
			
			CFileDialog dlg(FALSE,"*.dav","download.dav",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
				"All File(*.dav)|*.*||",this);
			if(dlg.DoModal() == IDOK)
			{
				CString strFilePathName = dlg.GetPathName();
				LLONG lRet = CLIENT_DownloadByTime(m_LoginID, nChannelId, 0, 
					tmStart, tmEnd, (LPSTR)(LPCSTR)strFilePathName,TimeDownLoadPos, (LDWORD)this);
				if(0 != lRet)
				{
					m_pDownLoadByTime = lRet;
					bRet = TRUE;
				}
				else
				{
					LastError();
                    MessageBox(ConvertString("Download failed!"), ConvertString("Prompt"));
				}
			}
		}
	}
	else
	{
		//Stop download
		if(0 != m_pDownLoadByTime)
		{
			BOOL bSuccess = CLIENT_StopDownload(m_pDownLoadByTime);
			if(bSuccess)
			{
				m_pDownLoadByTime = 0;
				bRet = TRUE;
			}
			else
			{
				MessageBox(ConvertString("Stop download failed!"), ConvertString("Prompt"));
			}
		}
	}
	return bRet;
}
//Download in accordance with time
BOOL CPlayBackDlg::DownLoadByType(int nChannelId, LPNET_TIME tmStart, LPNET_TIME tmEnd, BOOL bLoad,int ConvertType)
{
	BOOL bRet =FALSE;
	if(0 == m_LoginID)
	{
		MessageBox(ConvertString("Please login first"), ConvertString("Prompt"));
		return bRet;
	}
	if(bLoad)
	{
		//Download
		if(0 != m_LoginID)
		{
			if(0 != m_pDownLoadByTime)
			{
				//Close download
				BOOL bSuccess = CLIENT_StopDownload(m_pDownLoadByTime);
				if(bSuccess)
				{
					m_pDownLoadByTime = 0;
				}
				else
				{
					MessageBox(ConvertString("Stop download failed!"), ConvertString("Prompt"));
				}
			}
			
			CFileDialog dlg(FALSE,"*.dav","download.dav",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
				"All File(*.dav)|*.*||",this);
			if(dlg.DoModal() == IDOK)
			{
				CString strFilePathName = dlg.GetPathName();
				NET_IN_DOWNLOAD_BY_DATA_TYPE stuIn = {sizeof(stuIn)};
				stuIn.nChannelID = nChannelId;
				stuIn.emRecordType = EM_RECORD_TYPE_ALL;
				stuIn.szSavedFileName = (LPSTR)(LPCSTR)strFilePathName;
				memcpy(&stuIn.stStartTime,tmStart,sizeof(stuIn.stStartTime));
				memcpy(&stuIn.stStopTime,tmEnd,sizeof(stuIn.stStopTime));
				stuIn.cbDownLoadPos = TimeDownLoadPos;
				stuIn.emDataType = (EM_REAL_DATA_TYPE)ConvertType;
				stuIn.dwPosUser = (LDWORD)this;
				NET_OUT_DOWNLOAD_BY_DATA_TYPE stuOut = {sizeof(stuOut)};

				LLONG lRet = CLIENT_DownloadByDataType(m_LoginID, &stuIn, &stuOut, 3000);
				if(0 != lRet)
				{
					m_pDownLoadByTime = lRet;
					bRet = TRUE;
				}
				else
				{
					LastError();
                    MessageBox(ConvertString("Download failed!"), ConvertString("Prompt"));
				}
			}
		}
	}
	else
	{
		//Stop download
		if(0 != m_pDownLoadByTime)
		{
			BOOL bSuccess = CLIENT_StopDownload(m_pDownLoadByTime);
			if(bSuccess)
			{
				m_pDownLoadByTime = 0;
				bRet = TRUE;
			}
			else
			{
				MessageBox(ConvertString("Stop download failed!"), ConvertString("Prompt"));
			}
		}
	}
	return bRet;
}
//Play and pause
void CPlayBackDlg::OnButtonPlay() 
{
	// TODO: Add your control notification handler code here
	if((0 != m_hPlayBack) && (0 != m_LoginID))
	{
		CString strText;
		GetDlgItemText(IDC_BUTTON_PLAY,strText);
		if(strText ==  ConvertString("Play") || strText ==  ConvertString("PlayForward"))
		{
			BOOL bSuccess = FALSE;
			if(m_ePlayMode == EM_DIRECTMODE)
			{
				bSuccess = CLIENT_PausePlayBack(m_hPlayBack,FALSE);
			}
			else if(m_ePlayMode == EM_SERVERMODE)
			{

				bSuccess = g_PlayAPI.PLAY_Pause(1,FALSE);

			}

			if(bSuccess)
			{
				SetDlgItemText(IDC_BUTTON_PLAY, ConvertString("Pause"));
			}
			else
			{
				MessageBox( ConvertString("Play failed!"), ConvertString("Prompt"));
			}
		}
		else
		{
			//Pause
			BOOL bSuccess = FALSE;
			if(m_ePlayMode == EM_DIRECTMODE)
			{
				bSuccess = CLIENT_PausePlayBack(m_hPlayBack,TRUE);
			}
			else if(m_ePlayMode == EM_SERVERMODE)
			{
				bSuccess = g_PlayAPI.PLAY_Pause(1,TRUE);
			}
			if(bSuccess)
			{
				if (EM_FILEBACK == m_PlayBackType && EM_SERVERMODE == m_ePlayMode)
				{
					SetDlgItemText(IDC_BUTTON_PLAY,ConvertString("PlayForward"));
				}
				else
				{
					SetDlgItemText(IDC_BUTTON_PLAY,ConvertString("Play"));
				}
			}
			else
			{
				MessageBox(ConvertString("Pause failed!"), ConvertString("Prompt"));
			}
		}


	}
	else
	{
		MessageBox(ConvertString("Please play first!"), ConvertString("Prompt"));
	}
}

static float fSpeed[] = {1/8.0f, 1/4.0f, 1/2.0f, 1.0f, 2.0f, 4.0f, 8.0f};

BOOL DoPlayFast( LONG nPort, PlaySpeed nCurSpeed )
{
    return g_PlayAPI.PLAY_SetPlaySpeed(nPort, fSpeed[nCurSpeed + 1]);  
}

BOOL DoPlaySlow( LONG nPort, PlaySpeed nCurSpeed )
{
    return g_PlayAPI.PLAY_SetPlaySpeed(nPort, fSpeed[nCurSpeed - 1]);   
}

LPCTSTR ChangeSpeed( int nMode, PlaySpeed& speed )
{
    if (nMode == 0)
    {
        speed = SPEED_NORMAL;
    }
    else if (nMode > 0)	// 加速
    {
        if (speed >= SPEED_MIN && speed < SPEED_MAX)
        {
            speed = (PlaySpeed)((int)speed + 1);
        }
    }
    else if (nMode < 0)	// 减速
    {
        if (speed > SPEED_MIN && speed <= SPEED_MAX)
        {
            speed = (PlaySpeed)((int)speed - 1);
        }
    }
    
    LPCTSTR lpszStr = NULL;
    static LPCTSTR aSpeed[] = {"1/8X", "1/4X", "1/2X", "1X", "2X", "4X", "8X"};
    if (speed >= SPEED_MIN && speed <= SPEED_MAX)
    {
        lpszStr = aSpeed[speed - SPEED_MIN];
    }
    return lpszStr;
}

void CPlayBackDlg::ChangeSpeedStatus( int nMode )
{
    SetDlgItemText(IDC_STATIC_SPEED, ChangeSpeed(nMode, m_eSpeed));
}

//Slow forward
void CPlayBackDlg::OnButtonSlow() 
{
	// TODO: Add your control notification handler code here
	if((0 != m_hPlayBack) && (0 != m_LoginID))
	{
		BOOL bSuccess = FALSE;
		if (m_eSpeed > SPEED_MIN && m_eSpeed <= SPEED_MAX)
		{
			if(m_ePlayMode == EM_DIRECTMODE)
			{
				bSuccess = CLIENT_SlowPlayBack(m_hPlayBack);
			}
			else if(m_ePlayMode == EM_SERVERMODE)
			{
				bSuccess = DoPlaySlow(1, m_eSpeed);
				if (bSuccess && m_bSpeedCtrl)
				{
					bSuccess = CLIENT_SlowPlayBack(m_hPlayBack);
				}
				else
				{
				    bSuccess = FALSE;
				}
			}
		}
	    if(!bSuccess)
		{
			MessageBox(ConvertString("Slow failed!"), ConvertString("Prompt"));
		}        
		else
		{
			ChangeSpeedStatus(-1);
			if (m_eSpeed==SPEED_MIN)
			{
				GetDlgItem(IDC_BUTTON_SLOW)->EnableWindow(FALSE);
			}
			if (m_eSpeed==SPEED_MAX)
			{
				GetDlgItem(IDC_BUTTON_FAST)->EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDC_BUTTON_FAST)->EnableWindow(TRUE);
			}
		}
	}
	else
	{
		MessageBox(ConvertString("Please play first!"), ConvertString("Prompt"));
	}
}

//fast forward
void CPlayBackDlg::OnButtonFast() 
{
	// TODO: Add your control notification handler code here
	if((0 != m_hPlayBack) && (0 != m_LoginID))
	{
		BOOL bSuccess = FALSE;
		if (m_eSpeed >= SPEED_MIN && m_eSpeed < SPEED_MAX)
		{
			if(m_ePlayMode == EM_DIRECTMODE)
			{
				bSuccess = CLIENT_FastPlayBack(m_hPlayBack);
			}
			else if(m_ePlayMode == EM_SERVERMODE)
			{
				bSuccess = DoPlayFast(1, m_eSpeed);
				if (bSuccess && m_bSpeedCtrl)
				{
					bSuccess = CLIENT_FastPlayBack(m_hPlayBack);
				}
				else
				{
			         bSuccess = FALSE;
				}
			}
		}
		if(!bSuccess)
		{
			MessageBox(ConvertString("Fast failed!"), ConvertString("Prompt"));
		}
        else
        {
            ChangeSpeedStatus(1);
			if (m_eSpeed==SPEED_MAX)
			{
				GetDlgItem(IDC_BUTTON_FAST)->EnableWindow(FALSE);
			}
			if (m_eSpeed==SPEED_MIN)
			{
				GetDlgItem(IDC_BUTTON_SLOW)->EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDC_BUTTON_SLOW)->EnableWindow(TRUE);
			}
		}
	}
	else
	{
		MessageBox(ConvertString("Please play first!"), ConvertString("Prompt"));
	}
}

//Data callback
int CALLBACK DataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser)
{
	int nRet = 1;
	if(dwUser == 0)
	{
		return nRet;
	}
	CPlayBackDlg *dlg = (CPlayBackDlg *)dwUser;
	nRet = dlg->ReceiveRealData(lRealHandle,dwDataType, pBuffer, dwBufSize);
	return nRet;
}

//Data callback mode 
int CPlayBackDlg::ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize)
{
	int nRet = 1;
	if(lRealHandle == m_hPlayBack)
	{	
		BOOL bSuccess = TRUE;
		switch(dwDataType) {
		case 0:
			//Original data 
			bSuccess = g_PlayAPI.PLAY_InputData(1,pBuffer,dwBufSize);
			if(!bSuccess)
			{
				nRet = 0;
			}

// 			if ((0 != lRealHandle) && (NULL != pBuffer) && (0 < dwBufSize))
// 			{
// 				FILE *fPic = fopen("D:\\playbackCallBack.mp4", "ab+");
// 				if (fPic)
// 				{
// 					fwrite(pBuffer, 1, dwBufSize, fPic);
// 
// 					fclose(fPic);
// 				}
// 			}

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
		case 4:
			//Original audio data 
			
			break;
		default:
			break;
		}	
	}
	return nRet;
}

//Playback in accordance with time in data callback mode.
//void PlayBackDlg::PlayBackByTimeServerMode(int nChannelID, LPNET_TIME lpStartTime, LPNET_TIME lpStopTime)
void CPlayBackDlg::PlayBackByTimeServerMode(int nChannelID, int nPlayDirection, int nRecordFileType)
{
	if(0 != m_LoginID)
	{
		//First close video 
		ClosePlayBack();
		m_hPlayBack = 0;

		DWORD dwStream = 0;//NET_RECORD_TYPE_ALL
		if (1 == nRecordFileType)
		{
			dwStream = NET_RECORD_TYPE_ALARM;
		}
		else if (2 == nRecordFileType)
		{
			dwStream = NET_RECORD_TYPE_MOTION;
		}

		BOOL bRet = CLIENT_SetDeviceMode(m_LoginID, DH_RECORD_TYPE, &dwStream);

		//Set stream play mode
		BOOL bOpenRet = g_PlayAPI.PLAY_SetStreamOpenMode(1,STREAME_FILE);
		//Open stream
		bOpenRet = g_PlayAPI.PLAY_OpenStream(1,0,0,1024*900);
		if(bOpenRet)
		{
			//Begin play
			HWND hPlayBack = GetDlgItem(IDC_SCREEN_PLAYBACK)->m_hWnd;
			BOOL bPlayRet = g_PlayAPI.PLAY_Play(1,hPlayBack);
			if(bPlayRet)
			{
				g_PlayAPI.PLAY_SetPlayDirection(1, nPlayDirection);
				//Playback
				NET_IN_PLAY_BACK_BY_TIME_INFO stuPlayBackIn = {0};
				NET_OUT_PLAY_BACK_BY_TIME_INFO stuPlayBackOut = {0};
				stuPlayBackIn.stStartTime = m_PlayBackByTimeStart;
				stuPlayBackIn.stStopTime = m_PlayBackByTimeEnd;
				stuPlayBackIn.hWnd = 0;
				stuPlayBackIn.cbDownLoadPos = PlayCallBack;
				stuPlayBackIn.dwPosUser = (LDWORD)this;
				stuPlayBackIn.nPlayDirection = nPlayDirection;
				stuPlayBackIn.dwDataUser = (LDWORD)this;
				stuPlayBackIn.nWaittime = 5000;
				stuPlayBackIn.fDownLoadDataCallBack = DataCallBack;
				LLONG lHandle = CLIENT_PlayBackByTimeEx2(m_LoginID,nChannelID,&stuPlayBackIn, &stuPlayBackOut);
				if(0 != lHandle)
				{
                    if(0 == m_nTimer)
                    {
                        m_nTimer = SetTimer(1, TIMER_MSEC, NULL);
                    }
					m_nChannelID = nChannelID;
					m_hPlayBack = lHandle;
					m_ePlayMode = EM_SERVERMODE;
					SetDlgItemText(IDC_BUTTON_PLAY, ConvertString("Pause"));
					m_PlayBackType = EM_TIMEBACK;
                    m_nReverseMode = 0;
                    ChangeSpeedStatus(0);
                    SetPlayControlButton(TRUE);
				}
				else
				{
					GetDlgItem(IDC_SCREEN_PLAYBACK)->Invalidate();
					g_PlayAPI.PLAY_Stop(1);
					g_PlayAPI.PLAY_CloseStream(1);

					if (0 == nPlayDirection)
					{
						MessageBox(ConvertString("Playback failed!"), ConvertString("Prompt"));
					}
					else if (1 == nPlayDirection)
					{
						MessageBox(ConvertString("Playback reverse failed!"), ConvertString("Prompt"));
					}
				}
			}
			else
			{
				GetDlgItem(IDC_SCREEN_PLAYBACK)->Invalidate();
				g_PlayAPI.PLAY_CloseStream(1);
			}
		}
	}
	else
	{
		MessageBox(ConvertString("Please login first!"), ConvertString("Prompt"));
	}
}

//Close video 
void CPlayBackDlg::ClosePlayBack()
{
	if(m_ePlayMode == EM_DIRECTMODE)
	{
		//Close video directly
		if(0 != m_hPlayBack)
		{
			CLIENT_StopPlayBack(m_hPlayBack);
            SetPlayControlButton(FALSE);
			m_hPlayBack = 0;
		}
	}
	else if(m_ePlayMode == EM_SERVERMODE)
	{
		//Close video in data callback mode
		if(0 != m_hPlayBack)
		{
			BOOL bSuccess = CLIENT_StopPlayBack(m_hPlayBack);
			m_hPlayBack = 0;
			//Then close PLAY_Play
			bSuccess = g_PlayAPI.PLAY_Stop(1);
			//At last close PLAY_OpenStream
			bSuccess = g_PlayAPI.PLAY_CloseStream(1);

			bSuccess = g_PlayAPI.PLAY_ResetBuffer(1, BUF_VIDEO_SRC);
			bSuccess = g_PlayAPI.PLAY_ResetBuffer(1, BUF_AUDIO_SRC);
			bSuccess = g_PlayAPI.PLAY_ResetBuffer(1, BUF_VIDEO_RENDER);
			bSuccess = g_PlayAPI.PLAY_ResetBuffer(1, BUF_AUDIO_RENDER);
            SetPlayControlButton(FALSE);
		}
	}
}

//Click mouse event in process bar
void CPlayBackDlg::ExSliderButtonDown()
{
	if(0 != m_nTimer)
	{
		KillTimer(m_nTimer);
		m_nTimer = 0;
	}
}

//Mouse release event in process bar
void CPlayBackDlg::ExSliderButtonUp()
{
	if(0 == m_nTimer)
	{
		m_nTimer = SetTimer(1, TIMER_MSEC, NULL);
	}
}

void CPlayBackDlg::OnButtonNormalspeed() 
{
	// TODO: Add your control notification handler code here
	if((0 != m_LoginID) && (0 != m_hPlayBack))
	{
		BOOL bSuccess = TRUE;
		if(m_ePlayMode == EM_DIRECTMODE)
		{
			bSuccess = CLIENT_NormalPlayBack(m_hPlayBack);
		}
		else if(m_ePlayMode == EM_SERVERMODE)
		{
			HWND hPlayBack = GetDlgItem(IDC_SCREEN_PLAYBACK)->m_hWnd;
			bSuccess = g_PlayAPI.PLAY_Play(1, hPlayBack);
			if (bSuccess && m_bSpeedCtrl)
			{
				bSuccess = CLIENT_NormalPlayBack(m_hPlayBack);
			}
			
		}
		if(!bSuccess)
		{
			MessageBox(ConvertString("Recover normal speed failed!"), ConvertString("Prompt"));
		}
        else
        {
			GetDlgItem(IDC_BUTTON_SLOW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_FAST)->EnableWindow(TRUE);
            ChangeSpeedStatus(0);
		}
	}
	else
	{
		MessageBox(ConvertString("Please play first!"), ConvertString("Prompt"));
	}
}

int Compare( const NET_TIME *pSrcTime, const NET_TIME *pDestTime)
{
	SYSTEMTIME srcTime={0}, destTime={0};
	srcTime.wYear     =    (unsigned short)pSrcTime->dwYear;
	srcTime.wMonth  =    (unsigned short)pSrcTime->dwMonth;
	srcTime.wDay       =   (unsigned short)pSrcTime->dwDay;
	srcTime.wHour     =   (unsigned short)pSrcTime->dwHour;
	srcTime.wMinute   =  (unsigned short)pSrcTime->dwMinute;
	srcTime.wSecond  =   (unsigned short)pSrcTime->dwSecond;
    /// dest
	destTime.wYear     =    (unsigned short)pDestTime->dwYear;
	destTime.wMonth  =    (unsigned short)pDestTime->dwMonth;
	destTime.wDay       =   (unsigned short)pDestTime->dwDay;
	destTime.wHour     =   (unsigned short)pDestTime->dwHour;
	destTime.wMinute   =  (unsigned short)pDestTime->dwMinute;
	destTime.wSecond  =   (unsigned short)pDestTime->dwSecond;
	FILETIME   srcFileTime={0}, destFileTime={0};
	SystemTimeToFileTime(&srcTime, &srcFileTime);
	SystemTimeToFileTime(&destTime, &destFileTime);
	return CompareFileTime(&srcFileTime, &destFileTime);
}

void UpdateNetTime(NET_TIME& ntStart,  NET_TIME ntCur)
{
	if (Compare(&ntStart, &ntCur) == 1) // ntCur比ntStart小
	{
		ntStart = ntCur;
	}

}

void UpdateNetTime2(NET_TIME& ntEnd, NET_TIME ntCur)
{
	if (ntCur.dwYear > ntEnd.dwYear
		|| ntCur.dwMonth > ntEnd.dwMonth 
		|| ntCur.dwDay > ntEnd.dwDay )
	{
		ntEnd.dwHour = 23;
		ntEnd.dwMinute = 59;
		ntEnd.dwSecond = 59;
	}
	else
	{
		if (Compare(&ntEnd, &ntCur) == -1) // ntCur比ntEnd大
		{
			ntEnd = ntCur;
		}
	}
}

void CPlayBackDlg::StopPlay()
{
    OnButtonStop();
}

void CPlayBackDlg::AddTimeToDateTimeCtl_Multy(NET_RECORDFILE_INFO fileInfo[2000], int nFileCount)
{
    OnButtonStop();

    bool bRecordFlag = false;
    LONG lStartTime = 0;
    LONG lEndTime = 0;
//    m_DayTimePickSdk.ClearTimeBlock();
	
	m_pPlaybackTimeCtl->UninitTimeBar();

    NET_TIME ntStart = m_PlayBackByTimeStart;
    ntStart.dwHour = fileInfo[0].starttime.dwHour;
    ntStart.dwMinute = fileInfo[0].starttime.dwMinute;
    ntStart.dwSecond = fileInfo[0].starttime.dwSecond;
    NET_TIME ntEnd = m_PlayBackByTimeEnd;
    ntEnd.dwHour = fileInfo[0].endtime.dwHour;
    ntEnd.dwMinute = fileInfo[0].endtime.dwMinute;
    ntEnd.dwSecond = fileInfo[0].endtime.dwSecond;
    
	CPlayBackTimeView::VideoTime videoTime;
	m_timeList.clear();

    for (int i = 0;i < nFileCount;i++)
    {
        
        if (fileInfo[i].starttime.dwDay != m_PlayBackByTimeStart.dwDay)
        {
            if (true == bRecordFlag)
            {
                continue;
            }
        }
		
        bRecordFlag = true;
            
        lStartTime = fileInfo[i].starttime.dwHour*3600 + fileInfo[i].starttime.dwMinute*60 + fileInfo[i].starttime.dwSecond;
        if (fileInfo[i].endtime.dwYear == fileInfo[i].starttime.dwYear + 1
            || fileInfo[i].endtime.dwMonth == fileInfo[i].starttime.dwMonth + 1
            || fileInfo[i].endtime.dwDay == fileInfo[i].starttime.dwDay + 1)
        {
            lEndTime = 24*3600;
        }
        else
        {
            lEndTime = fileInfo[i].endtime.dwHour*3600 + fileInfo[i].endtime.dwMinute*60 + fileInfo[i].endtime.dwSecond;
        }
//        m_DayTimePickSdk.AddTimeBlock(lStartTime, lEndTime, RGB(0,255,0));
        
		videoTime.startTime = COleDateTime(
			fileInfo[i].starttime.dwYear,
			fileInfo[i].starttime.dwMonth,
			fileInfo[i].starttime.dwDay,
			fileInfo[i].starttime.dwHour,
			fileInfo[i].starttime.dwMinute,
			fileInfo[i].starttime.dwSecond
			);
		videoTime.endTime = COleDateTime(
			fileInfo[i].endtime.dwYear,
			fileInfo[i].endtime.dwMonth,
			fileInfo[i].endtime.dwDay,
			fileInfo[i].endtime.dwHour,
			fileInfo[i].endtime.dwMinute,
			fileInfo[i].endtime.dwSecond
			);
		m_timeList.push_back(videoTime);

        UpdateNetTime(ntStart, fileInfo[i].starttime);
        UpdateNetTime2(ntEnd, fileInfo[i].endtime);
        
    }
    
    m_pCurrentTime = ntStart.dwHour*3600 + ntStart.dwMinute*60 + ntStart.dwSecond;
    m_StartTime = m_pCurrentTime;
 //   m_DayTimePickSdk.SetCurrentTime(m_StartTime);
	COleDateTime startTime(ntStart.dwYear, ntStart.dwMonth, ntStart.dwDay,
							ntStart.dwHour, ntStart.dwMinute, ntStart.dwSecond);
	m_pPlaybackTimeCtl->InitTimeBar(startTime,m_timeList);
    
    m_PlayBackByTimeStart.dwHour = ntStart.dwHour;
    m_PlayBackByTimeStart.dwMinute = ntStart.dwMinute;
    m_PlayBackByTimeStart.dwSecond = ntStart.dwSecond;
    
    m_PlayBackByTimeEnd.dwHour = ntEnd.dwHour;
    m_PlayBackByTimeEnd.dwMinute = ntEnd.dwMinute;
    m_PlayBackByTimeEnd.dwSecond = ntEnd.dwSecond;

 //   m_DayTimePickSdk.Invalidate(FALSE);
}

void CPlayBackDlg::AddTimeToDateTimeCtl(NET_RECORDFILE_INFO fileInfo[2000], int nFileCount, int nPlayDirection)
{
	OnButtonStop();

	m_nPlayDirection = nPlayDirection;
	
	m_pPlaybackTimeCtl->UninitTimeBar();

	NET_TIME ntStart = m_PlayBackByTimeStart;
	ntStart.dwHour = fileInfo[0].starttime.dwHour;
	ntStart.dwMinute = fileInfo[0].starttime.dwMinute;
	ntStart.dwSecond = fileInfo[0].starttime.dwSecond;
	NET_TIME ntEnd = m_PlayBackByTimeEnd;
	ntEnd.dwHour = fileInfo[0].endtime.dwHour;
	ntEnd.dwMinute = fileInfo[0].endtime.dwMinute;
	ntEnd.dwSecond = fileInfo[0].endtime.dwSecond;

	CPlayBackTimeView::VideoTime videoTime;
	m_timeList.clear();

	for (int i = 0;i < nFileCount;i++)
	{
		
		if (fileInfo[i].starttime.dwDay != m_PlayBackByTimeStart.dwDay)
		{
			continue;
		}

		videoTime.startTime = COleDateTime(
			fileInfo[i].starttime.dwYear,
			fileInfo[i].starttime.dwMonth,
			fileInfo[i].starttime.dwDay,
			fileInfo[i].starttime.dwHour,
			fileInfo[i].starttime.dwMinute,
			fileInfo[i].starttime.dwSecond
			);
		videoTime.endTime = COleDateTime(
			fileInfo[i].endtime.dwYear,
			fileInfo[i].endtime.dwMonth,
			fileInfo[i].endtime.dwDay,
			fileInfo[i].endtime.dwHour,
			fileInfo[i].endtime.dwMinute,
			fileInfo[i].endtime.dwSecond
			);

		m_timeList.push_back(videoTime);

		UpdateNetTime(ntStart, fileInfo[i].starttime);
		UpdateNetTime2(ntEnd, fileInfo[i].endtime);
		
	}

	memcpy(&m_stuBeginTime, &ntStart, sizeof(m_stuBeginTime));
	m_pCurrentTime = ntStart.dwHour*3600 + ntStart.dwMinute*60 + ntStart.dwSecond;
	m_StartTime = m_pCurrentTime;
//	m_DayTimePickSdk.SetCurrentTime(m_StartTime);

    COleDateTime startTime(ntStart.dwYear, ntStart.dwMonth, ntStart.dwDay,
			ntStart.dwHour, ntStart.dwMinute, ntStart.dwSecond);
	m_pPlaybackTimeCtl->InitTimeBar(startTime, m_timeList);

	m_PlayBackByTimeStart.dwHour = ntStart.dwHour;
    m_PlayBackByTimeStart.dwMinute = ntStart.dwMinute;
	m_PlayBackByTimeStart.dwSecond = ntStart.dwSecond;
	
	m_PlayBackByTimeEnd.dwHour = ntEnd.dwHour;
    m_PlayBackByTimeEnd.dwMinute = ntEnd.dwMinute;
	m_PlayBackByTimeEnd.dwSecond = ntEnd.dwSecond;

    m_pTabTimeDlg->OnButtonPlay(nPlayDirection);
}

BEGIN_EVENTSINK_MAP(CPlayBackDlg, CDialog)
    //{{AFX_EVENTSINK_MAP(PlayBackDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CPlayBackDlg::InitTimeCtlBySingleFileInfo(NET_RECORDFILE_INFO & fileInfo)
{
	ClosePlayBack();
	m_pPlaybackTimeCtl->UninitTimeBar();
	m_timeList.clear();

	m_PlayBackByTimeStart = fileInfo.starttime;
	m_PlayBackByTimeEnd = fileInfo.endtime;
	
	
	long lStartTime = fileInfo.starttime.dwHour*3600 + fileInfo.starttime.dwMinute*60 + fileInfo.starttime.dwSecond;
	long lEndTime = fileInfo.endtime.dwHour*3600 + fileInfo.endtime.dwMinute*60 + fileInfo.endtime.dwSecond;
	if (fileInfo.endtime.dwYear > fileInfo.starttime.dwYear
		|| fileInfo.endtime.dwMonth > fileInfo.starttime.dwMonth
		|| fileInfo.endtime.dwDay > fileInfo.starttime.dwDay)
	{
		m_PlayBackByTimeEnd = m_PlayBackByTimeStart;
		m_PlayBackByTimeEnd.dwHour = 23;
		m_PlayBackByTimeEnd.dwMinute = 59;
		m_PlayBackByTimeEnd.dwSecond = 59;
		lEndTime = 24*60*60;
	}

	COleDateTime startOleTime(m_PlayBackByTimeStart.dwYear, m_PlayBackByTimeStart.dwMonth,m_PlayBackByTimeStart.dwDay,
							m_PlayBackByTimeStart.dwHour, m_PlayBackByTimeStart.dwMinute,m_PlayBackByTimeStart.dwSecond);
	CPlayBackTimeView::VideoTime videoTime;
	videoTime.startTime = COleDateTime(m_PlayBackByTimeStart.dwYear, m_PlayBackByTimeStart.dwMonth,m_PlayBackByTimeStart.dwDay,
		m_PlayBackByTimeStart.dwHour, m_PlayBackByTimeStart.dwMinute,m_PlayBackByTimeStart.dwSecond);
	videoTime.endTime = COleDateTime(m_PlayBackByTimeEnd.dwYear, m_PlayBackByTimeEnd.dwMonth,m_PlayBackByTimeEnd.dwDay,
		m_PlayBackByTimeEnd.dwHour, m_PlayBackByTimeEnd.dwMinute,m_PlayBackByTimeEnd.dwSecond);
	m_timeList.push_back(videoTime);

	m_pPlaybackTimeCtl->InitTimeBar(startOleTime, m_timeList);

	m_TotalTime = lEndTime - lStartTime;
	m_StartTime = lStartTime;

}


void CPlayBackDlg::OnButtonCapturePicture() 
{
	// TODO: Add your control notification handler code here

	COleDateTime nowPlayBack = m_pPlaybackTimeCtl->GetCurrentTime(); 
	
    DWORD dwHour = nowPlayBack.GetHour();
    DWORD dwMin = nowPlayBack.GetMinute();
	DWORD dwSec = nowPlayBack.GetSecond();

    if((0 != m_hPlayBack) && (0 != m_LoginID))
    {
        BOOL bSuccess = TRUE;
        int nError = 0;
        int nDirectoryLength = 64;
        char* pDirectory = NULL;
        while(1)
        {
            if (NULL == pDirectory)
            {
                pDirectory = new char[nDirectoryLength];
            }

            if (NULL == pDirectory)
            {
                MessageBox(ConvertString("System error!"), ConvertString("Prompt"));
                return;
            }
            memset(pDirectory, 0, nDirectoryLength);

            int Ret = GetModuleFileName(NULL,  pDirectory, nDirectoryLength);
			*(pDirectory + (Ret-1)) = 0;
            char* pLastPos = NULL;
            for (int i = 0; (pDirectory[i]) && (i < nDirectoryLength); ++i)
            {
                if ('\\' == pDirectory[i])
                {
                    pLastPos = pDirectory + i;
                }
            }

            if (NULL == pLastPos)
            {
                MessageBox(ConvertString("System error!"), ConvertString("Prompt"));
				if (pDirectory)
				{
					delete[] pDirectory;
					pDirectory = NULL;
				}
                return;
            }
            else
            {
                *(pLastPos + 1) = 0;
            }

            if (Ret >= nDirectoryLength)   // 缓冲区不够
            {
                delete [] pDirectory;
                pDirectory = NULL;
                nDirectoryLength = nDirectoryLength * 2;
            }
            else
            {
                break;
            }
        }

        int nPictureNameLength = nDirectoryLength + 50;
        char* pPictureName = new char[nPictureNameLength];
        if (NULL == pPictureName)
        {
            MessageBox(ConvertString("System error!"), ConvertString("Prompt"));
            return;
        }

        memset(pPictureName, 0, nPictureNameLength);

        _snprintf(pPictureName, nPictureNameLength - 1, "%s%s_%2d_%2d_%2d.bmp", pDirectory, (m_ePlayMode == EM_DIRECTMODE)?"Picture_Direct":"Picture_Server", dwHour, dwMin, dwSec);
        if (NULL != pDirectory)
        {
            delete [] pDirectory;
            pDirectory = NULL;
        }

        if(m_ePlayMode == EM_DIRECTMODE)
        {
            //bSuccess = CLIENT_CapturePictureEx(m_hPlayBack, "D://Picture_Direct_6_5_3.bmp", NET_CAPTURE_BMP);
            bSuccess = CLIENT_CapturePictureEx(m_hPlayBack, pPictureName, NET_CAPTURE_BMP);
        }
        else if(m_ePlayMode == EM_SERVERMODE)
        {
            //bSuccess = PLAY_CatchPicEx(1, "D://Picture_Direct_6_5_3.bmp", PicFormat_BMP);
            bSuccess = g_PlayAPI.PLAY_CatchPicEx(1, pPictureName, PicFormat_BMP);
        }

        if (NULL != pPictureName)
        {
            delete [] pPictureName;
            pPictureName = NULL;
        }

        if(!bSuccess)
        {
            nError = CLIENT_GetLastError();
            MessageBox(ConvertString("Capture picture failed!"), ConvertString("Prompt"));
        }
    }
    else
    {
        MessageBox(ConvertString("Please play first!"), ConvertString("Prompt"));
	}
	
}

long CPlayBackDlg::GetUTCTimebyNetTime(const NET_TIME& stTime)
{
    struct tm when = {0};
    when.tm_year = stTime.dwYear - 1900;
    when.tm_mon = stTime.dwMonth - 1;
    when.tm_mday = stTime.dwDay;
    when.tm_hour = stTime.dwHour;
    when.tm_min = stTime.dwMinute;
    when.tm_sec = stTime.dwSecond;
    when.tm_isdst = -1;
    
    long UTC = mktime(&when);
    return UTC < 0 ? 0 : UTC;
}



void CPlayBackDlg::invoke(const COleDateTime* pTime )
{
	if (0 != m_hPlayBack)
	{
		if(0 != m_nTimer) // 定时器
		{
			KillTimer(m_nTimer);
			m_nTimer = 0;
		}

		ClosePlayBack();

		if (0 == m_nPlayDirection)
		{
			m_PlayBackByTimeStart.dwHour = pTime->GetHour();
			m_PlayBackByTimeStart.dwMinute = pTime->GetMinute();
			m_PlayBackByTimeStart.dwSecond = pTime->GetSecond();
		}
		else if (1 == m_nPlayDirection)
		{
			m_PlayBackByTimeStart.dwHour = m_stuBeginTime.dwHour;
			m_PlayBackByTimeStart.dwMinute = m_stuBeginTime.dwMinute;
			m_PlayBackByTimeStart.dwSecond = m_stuBeginTime.dwSecond;

			m_PlayBackByTimeEnd.dwHour = pTime->GetHour();
			m_PlayBackByTimeEnd.dwMinute = pTime->GetMinute();
			m_PlayBackByTimeEnd.dwSecond = pTime->GetSecond();
		}

		if(m_ePlayMode == EM_DIRECTMODE)
		{
			PlayBackByTime(m_nChannelID, m_nPlayDirection);
		}
		else
		{
			PlayBackByTimeServerMode(m_nChannelID, m_nPlayDirection);
		}
	}
	else
	{
		return;
		//ClosePlayBack();
	}

}

BOOL CPlayBackDlg::PreTranslateMessage(MSG* pMsg) 
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