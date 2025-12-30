// AlarmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Alarm.h"
#include "AlarmDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_DEVICE_ALARM			(WM_USER + 1)				// Self-define message: Device alarm
#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

struct DEVICE_ALARM_INFO 
{
    int             nAlarmType;
    char*           pBuf;
    unsigned int    nBufSize;
};

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
// CAlarmDlg dialog

CAlarmDlg::CAlarmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAlarmDlg)
	m_dvrPort = 37777;
	m_strDvrUserName = _T("admin");
	m_strDvrPwd = _T("admin");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bIsListen = FALSE;

	memset(&stuTime,0,sizeof(SYSTEMTIME));
	memset(&stuTimeLater,0,sizeof(SYSTEMTIME));

	m_LoginID = 0;
	m_pTabDevAlarmDlg = NULL;
	m_pTabQueryAlarm = NULL;
}

void CAlarmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlarmDlg)
	DDX_Control(pDX, IDC_IPADDRESS_DVRIP, m_dvrIP);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
	DDX_Text(pDX, IDC_EDIT_PORT, m_dvrPort);
//	DDV_MinMaxInt(pDX, m_dvrPort, 0, 100000);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strDvrUserName);
	DDX_Text(pDX, IDC_EDIT_DVRPWD, m_strDvrPwd);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAlarmDlg, CDialog)
	//{{AFX_MSG_MAP(CAlarmDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, OnBtnLogout)
    ON_MESSAGE(WM_DEVICE_ALARM, OnDevAlarm)	
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlarmDlg message handlers

BOOL CAlarmDlg::OnInitDialog()
{
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
	//Initialize TabControl
	InitTabControl();
	//Device default IP
	m_dvrIP.SetAddress(172,23,1,112);

	//Initialize net SDK
	InitNetSDK();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAlarmDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAlarmDlg::OnPaint() 
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
HCURSOR CAlarmDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CAlarmDlg::SetDlgState(CWnd *pWnd, BOOL bShow)
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

void CAlarmDlg::DoTab(int nTab)
{
	//Confirm nTab is within the threshold
	if(nTab > 1)
	{
		nTab = 1;
	}
	if(nTab < 0)
	{
		nTab = 0;
	}
	
	BOOL bTab[2];
	bTab[0]=bTab[1]=FALSE;
	bTab[nTab]=TRUE;
 
    SetDlgState(m_pTabDevAlarmDlg, bTab[0]);
	SetDlgState(m_pTabQueryAlarm, bTab[1]);
}

void CAlarmDlg::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nSelect = m_Tab.GetCurSel();
	if(nSelect>=0)
	{
		DoTab(nSelect);
	}
	*pResult = 0;
}

//Initialize TabControl
void CAlarmDlg::InitTabControl()
{	
	m_pTabDevAlarmDlg = new CDevAlarmDlg;
	m_pTabQueryAlarm =new CQueryAlarmState();
	//Create no-mode dialog box, specify tag control is the father-window of the no-mode dialog box.	
    m_pTabDevAlarmDlg->Create(IDD_DIALOG_ALARM, &m_Tab);
	m_pTabQueryAlarm->Create(IDD_DIALOG_QUERYALARMSTATE,&m_Tab);

	m_Tab.InsertItem(0, ConvertString("Device Alarm"));
	m_Tab.InsertItem(1, ConvertString("Alarm Query"));
	
	m_Tab.SetMinTabWidth(80);
	m_Tab.SetPadding(CSize(15,3));
	m_Tab.SetCurSel(0);
	DoTab(0);
}

void CAlarmDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if(m_pTabDevAlarmDlg) delete m_pTabDevAlarmDlg;
	if(m_pTabQueryAlarm) delete m_pTabQueryAlarm;

	//Stop listening device message 
	if(m_bIsListen)
	{
		m_bIsListen = FALSE;
		CLIENT_StopListen(m_LoginID);
	}
	
	//Log off 
	if(0 != m_LoginID)
	{
		CLIENT_Logout(m_LoginID);
	}
	//Clear SDK and then release occupied resources 
	CLIENT_Cleanup();
}

BOOL CAlarmDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN)
	{
		//Enter
		if(pMsg->wParam == VK_RETURN) return TRUE;
		//ESC
		if(pMsg->wParam == VK_ESCAPE) return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

//Initialize net SDK
void CAlarmDlg::InitNetSDK()
{
	BOOL ret = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	if (ret)
	{
		//Set message callback function. It is to receive all device messages by default.
		CLIENT_SetDVRMessCallBack(MessCallBack, (LDWORD)this);
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
	else
	{
		MessageBox(ConvertString("Initialize SDK failed!"), ConvertString("Prompt"));
	}
}

//Callback function when device disconnected 
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CAlarmDlg *pThis = (CAlarmDlg *)dwUser;
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

	CAlarmDlg *pThis = (CAlarmDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);		
}

//Message callback process function. It is a callback of the whole SDK.
BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    DEVICE_ALARM_INFO* pAlarmInfo = new DEVICE_ALARM_INFO;
    pAlarmInfo->nAlarmType = lCommand;
    pAlarmInfo->nBufSize = dwBufLen;
    pAlarmInfo->pBuf = new char[dwBufLen];
    
    memcpy(pAlarmInfo->pBuf, pBuf, dwBufLen);

    ((CAlarmDlg*)dwUser)->PostMessage(WM_DEVICE_ALARM, (WPARAM)pAlarmInfo, NULL);

    return TRUE;	
}

//Process when device disconnected 
LRESULT CAlarmDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CAlarmDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("Alarm"));
	return 0;
}

//Register user to Device 
void CAlarmDlg::OnBtnLogin() 
{
	SetWindowText(ConvertString("Alarm"));
	// TODO: Add your control notification handler code here
	BOOL bValid = UpdateData(TRUE);
	if(bValid)
	{
		char *pchDVRIP;
		CString strDvrIP = GetDvrIP();
		pchDVRIP = (LPSTR)(LPCSTR)strDvrIP;
		WORD wDVRPort=(WORD)m_dvrPort;
		char *pchUserName=(LPSTR)(LPCSTR)m_strDvrUserName;
		char *pchPassword=(LPSTR)(LPCSTR)m_strDvrPwd;
        
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
        m_LoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

		if(0 == m_LoginID)
		{
			//Display log in failure reason 
			ShowLoginErrorReason(stOutparam.nError);
		}
		else
		{		
			GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);
			
			int nSelect = m_Tab.GetCurSel();
			if(nSelect>=0)
			{
				DoTab(nSelect);
			}
			if(CLIENT_StartListenEx(m_LoginID))
			{
				m_bIsListen = TRUE;
			}
		}
	}
}
//Log off device user 
void CAlarmDlg::OnBtnLogout() 
{
	// TODO: Add your control notification handler code here
	//Stop listening device message 
	if(m_bIsListen)
	{
		m_bIsListen = FALSE;
		CLIENT_StopListen(m_LoginID);
	}
	
	BOOL bRet = CLIENT_Logout(m_LoginID);
	//Clear log in handle after logged off 
	if(bRet)
	{
		m_LoginID=0;
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);		
		if ( m_pTabDevAlarmDlg!=NULL)
		{
			m_pTabDevAlarmDlg->ClearAlarmInfo();
		}
		if ( m_pTabQueryAlarm!=NULL)
		{
			m_pTabQueryAlarm->CleanAllInfo();
		}	
	}
	else
	{
		MessageBox(ConvertString("Logout failed!"), ConvertString("Prompt"));
	}
	SetWindowText(ConvertString("Alarm"));

	ClearAlarmInfoList();
}

//Get DVR IP address
CString CAlarmDlg::GetDvrIP()
{
	CString strRet="";
	BYTE nField0,nField1,nField2,nField3;
	m_dvrIP.GetAddress(nField0,nField1,nField2,nField3);
	strRet.Format("%d.%d.%d.%d",nField0,nField1,nField2,nField3);
	return strRet;
}

//Display failure reason 
void CAlarmDlg::ShowLoginErrorReason(int nError)
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

bool CAlarmDlg::FindInCList(int nAlarmType, int nChannel, POSITION& pos)
{
	int nCount = m_listAlarmInfo.GetCount();	
	pos = m_listAlarmInfo.GetHeadPosition();
	for (int i = 0; i < nCount; i++)
	{
		CAlarmInfoEx alarmInfo = m_listAlarmInfo.GetAt(pos);	
		if (alarmInfo.m_nAlarmType == nAlarmType && alarmInfo.m_nChannel == nChannel)
		{
			return true;			
		}
		m_listAlarmInfo.GetNext(pos);
	}
	return false;
}

LRESULT	CAlarmDlg::OnDevAlarm(WPARAM wParam, LPARAM lParam)
{
    DEVICE_ALARM_INFO* pAlarmInfo = (DEVICE_ALARM_INFO*) wParam;
	if(NULL == pAlarmInfo || NULL == pAlarmInfo->pBuf)
	{
		return -1;
	}

	char* pBuf = pAlarmInfo->pBuf;
	unsigned int nBufLen = pAlarmInfo->nBufSize;

	CAlarmInfoEx alarmInfo;
	switch(pAlarmInfo->nAlarmType)
	{
	case DH_ALARM_ALARM_EX:			// 0x2101	// External alarm 		
	case DH_MOTION_ALARM_EX:		// 0x2102	// Motion detection alarm 		
	case DH_VIDEOLOST_ALARM_EX:		// 0x2103	// Video loss alarm 		
	case DH_SHELTER_ALARM_EX:		// 0x2104	// Camera masking alarm 		
	case DH_DISKFULL_ALARM_EX:		// 0x2106	// HDD full alarm 		
	case DH_DISKERROR_ALARM_EX:		// 0x2107	// HDD malfunction alarm 
		{			
			alarmInfo.m_nAlarmType = pAlarmInfo->nAlarmType;					
			for (int i = 0; i < nBufLen; i ++)
			{
				if (1 == pBuf[i]) // alarm start
				{
					POSITION pos;
					bool bFind = FindInCList(pAlarmInfo->nAlarmType, i, pos);
					if (!bFind) // not find
					{
						alarmInfo.m_emAlarmStatus = ALARM_START;
						alarmInfo.m_nChannel = i;
						CTime tm = CTime::GetCurrentTime();
						alarmInfo.m_strTime = tm.Format("%Y-%m-%d %H:%M:%S");
						m_listAlarmInfo.AddHead(alarmInfo);
						UpdateAlarmListCtrl(alarmInfo);
					}
				}
				else  // alarm stop
				{
					POSITION pos;
					bool bFind = FindInCList(pAlarmInfo->nAlarmType, i, pos);
					if (bFind) // find, status changed
					{						
						m_listAlarmInfo.RemoveAt(pos);
						alarmInfo.m_emAlarmStatus = ALARM_STOP;
						alarmInfo.m_nChannel = i;
						CTime tm = CTime::GetCurrentTime();
						alarmInfo.m_strTime = tm.Format("%Y-%m-%d %H:%M:%S");
						UpdateAlarmListCtrl(alarmInfo);
					}					
				}
			}			
		}		
		break;			
	default:
		break;
	}

	delete[] pBuf;
	pBuf = NULL;
	delete pAlarmInfo;
	pAlarmInfo = NULL;
    return 0;
}

void CAlarmDlg::UpdateAlarmListCtrl(const CAlarmInfoEx& alarmInfo)
{	
	m_pTabDevAlarmDlg->UpdateAlarmListCtrl(alarmInfo);	
}

void CAlarmDlg::ClearAlarmInfoList()
{
	m_listAlarmInfo.RemoveAll();
}