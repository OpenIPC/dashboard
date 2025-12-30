// GateDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GateDemo.h"
#include "GateDemoDlg.h"
#include "CardManagerDlg.h"
#include "RecordManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct EventPara
{
	LLONG lAnalyzerHandle;
	DWORD dwAlarmType;
	char* pAlarmInfo;
	BYTE *pBuffer;
	DWORD dwBufSize;
	int nSequence;
};

#define WM_USER_ALAMR_COME		(WM_USER + 110)
#define WM_DEVICE_DISCONNECT	(WM_USER + 111)
#define WM_DEVICE_RECONNECT		(WM_USER + 112)

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CGateDemoDlg dialog




CGateDemoDlg::CGateDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGateDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_lLoginID = 0;
	m_nChannelNum = 0;
	m_lAttchID = 0;
}

CGateDemoDlg::~CGateDemoDlg()
{
	CloseAttchHandle();
	CloseLoginHandle();
	CLIENT_Cleanup();
}

void CGateDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBX_CHANNEL, m_cbxChannel);
	DDX_Control(pDX, IDC_STATIC_PIC, m_EventPic);
}

BEGIN_MESSAGE_MAP(CGateDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_CARD_OPERATE, &CGateDemoDlg::OnBnClickedBtnCardOperate)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CGateDemoDlg::OnBnClickedBtnLogin)


	ON_MESSAGE(WM_USER_ALAMR_COME, &CGateDemoDlg::OnAlarmCome)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, &CGateDemoDlg::OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, &CGateDemoDlg::OnReConnect)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CGateDemoDlg::OnBnClickedBtnLogout)
	ON_BN_CLICKED(IDC_BTN_ATTACHEVENT, &CGateDemoDlg::OnBnClickedBtnAttachevent)
	ON_BN_CLICKED(IDC_BTN_DETACHEVENT, &CGateDemoDlg::OnBnClickedBtnDetachevent)
	ON_BN_CLICKED(IDC_BTN_RECORD_OPERATE, &CGateDemoDlg::OnBnClickedBtnRecordOperate)
END_MESSAGE_MAP()


// CGateDemoDlg message handlers

BOOL CGateDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	g_SetWndStaticText(this);

	SetDlgItemText(IDC_IPADDRESS, "172.5.3.26");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWD, "admin123");

	InitSdk();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGateDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGateDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGateDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/************************************************************************/
/* SDK回调函数                                                          */
/************************************************************************/
int CALLBACK RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata)
{
	

	//Get the CIntelligentDeviceDlg instance
	CGateDemoDlg *pEventInstance = (CGateDemoDlg*)dwUser;

	//ACCESS event
	if (dwAlarmType == EVENT_IVS_ACCESS_CTL)
	{
		DWORD nAlarmInfoSize = 0;
		EventPara *eventData = new EventPara;
		if (eventData == NULL)
		{
			return 0;
		}

		nAlarmInfoSize = sizeof(DEV_EVENT_ACCESS_CTL_INFO);
		eventData->lAnalyzerHandle = lAnalyzerHandle;
		eventData->dwAlarmType = dwAlarmType;

		eventData->pBuffer = new BYTE[dwBufSize];
		memcpy(eventData->pBuffer, pBuffer, dwBufSize);
		eventData->pAlarmInfo = new char[nAlarmInfoSize];
		memcpy(eventData->pAlarmInfo, pAlarmInfo, nAlarmInfoSize);

		eventData->dwBufSize = dwBufSize;
		eventData->nSequence = nSequence;
		HWND hwnd = pEventInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_USER_ALAMR_COME, (WPARAM)(eventData), 0);
	}
	return 0;
}

void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CGateDemoDlg *pThis = (CGateDemoDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
}

void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CGateDemoDlg *pThis = (CGateDemoDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}

void CGateDemoDlg::InitSdk()
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
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
}

//Process when device disconnected 
LRESULT CGateDemoDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device disconnected
	SetWindowText(ConvertString("Network disconnected!"));
	return 0;
}

//Process when device reconnect
LRESULT CGateDemoDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device reconnect
	SetWindowText(ConvertString("GateDemo"));
	return 0;
}

/************************************************************************/
/*handle message WM_USER_ALAMR_COME                                        */
/************************************************************************/
LRESULT CGateDemoDlg::OnAlarmCome(WPARAM wParam, LPARAM lParam)
{
	EventPara *ep = (EventPara*)wParam;
	this->DealWithEvent(ep->lAnalyzerHandle, ep->dwAlarmType, ep->pAlarmInfo, ep->pBuffer, ep->dwBufSize, ep->nSequence);

	delete[] ep->pBuffer;
	delete[] ep->pAlarmInfo;
	delete ep;
	return 0;
}

void CGateDemoDlg::DealWithEvent(LLONG lAnalyzerHandle, DWORD dwAlarmType, char* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, int nSequence)
{
	if (0 == lAnalyzerHandle || NULL == pAlarmInfo)
	{
		MessageBox(ConvertString("Invalid param!"), ConvertString("Prompt"));
		return;
	}

	if (dwAlarmType == EVENT_IVS_ACCESS_CTL)
	{
		DEV_EVENT_ACCESS_CTL_INFO *pInfo = (DEV_EVENT_ACCESS_CTL_INFO*)pAlarmInfo;
		CString strTime;
		CString strOpenMethod;
		CString strCardName;
		CString strCardNo;
		CString strUserID;
		CString strOpenStauts;

		double mPts = pInfo->PTS;
		CTime mtime;

		strTime.Format("%d-%02d-%02d %02d:%02d:%02d",
					pInfo->UTC.dwYear,
					pInfo->UTC.dwMonth,
					pInfo->UTC.dwDay,
					pInfo->UTC.dwHour,
					pInfo->UTC.dwMinute,
					pInfo->UTC.dwSecond);

		if (pInfo->emOpenMethod == NET_ACCESS_DOOROPEN_METHOD_CARD)
		{
			SetDlgItemText(IDC_EDIT_EVENTNAME, ConvertString("Card Method"));
		}
		else if (pInfo->emOpenMethod == NET_ACCESS_DOOROPEN_METHOD_FACE_RECOGNITION)
		{
			SetDlgItemText(IDC_EDIT_EVENTNAME, ConvertString("Face Method"));
		}
		else
		{
			SetDlgItemText(IDC_EDIT_EVENTNAME, ConvertString("Unknown"));
		}

		if (pInfo->bStatus == TRUE)
		{
			strOpenStauts = "success";
		}
		else
		{
			strOpenStauts = "fail";
		}

		strCardName.Format("%s", pInfo->szCardName);
		strCardNo.Format("%s", pInfo->szCardNo);
		strUserID.Format("%s", pInfo->szUserID);

		SetDlgItemText(IDC_EDIT_OPENSTATUS, ConvertString(strOpenStauts) );

		SetDlgItemText(IDC_EDIT_EVENTTIME, strTime );
		
		SetDlgItemText(IDC_EDIT_CARDNAME, strCardName);
		SetDlgItemText(IDC_EDIT_CARDNO, strCardNo);
		SetDlgItemText(IDC_EDIT_USERID, strUserID);

		m_EventPic.SetImageDate(pBuffer, dwBufSize);
	}
}

void CGateDemoDlg::OnBnClickedBtnCardOperate()
{
	CCardManagerDlg dlg(m_lLoginID);
	dlg.DoModal();
}

BOOL CGateDemoDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	// Escape key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CGateDemoDlg::OnBnClickedBtnLogin()
{
	CString strIp;
	CString strUser;
	CString strPwd;
	int		nPort;

	GetDlgItemText(IDC_IPADDRESS, strIp);
	GetDlgItemText(IDC_EDIT_USERNAME, strUser);
	GetDlgItemText(IDC_EDIT_PASSWD, strPwd);
	nPort = GetDlgItemInt(IDC_EDIT_PORT);

	char *pcIP = (LPSTR)(LPCSTR)strIp;
	char *pcUser = (LPSTR)(LPCSTR)strUser;
	char *pcPwssd = (LPSTR)(LPCSTR)strPwd;

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, pcIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, pcPwssd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, pcUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if(0 != lRet)
	{
		m_lLoginID = lRet;
		m_nChannelNum = stOutparam.stuDeviceInfo.nChanNum;

		/*update control state*/
		m_cbxChannel.EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);


		GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);

		GetDlgItem(IDC_BTN_CARD_OPERATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_RECORD_OPERATE)->EnableWindow(TRUE);

		for(int i = 0; i < m_nChannelNum; ++i)
		{
			CString tmp ;
			tmp.Format(ConvertString(_T("%d")), i+1) ;
			m_cbxChannel.AddString(tmp);
			m_cbxChannel.SetItemData(i, (DWORD_PTR)i);
		}
		m_cbxChannel.SetCurSel(0);

		m_EventPic.SetImageFile(NULL);
	}
	else
	{
		ShowLoginErrorReason(stOutparam.nError);
	}
}

void CGateDemoDlg::ShowLoginErrorReason(int nError)
{
	if(1 == nError)		MessageBox(ConvertString(_T("Invalid password!")), ConvertString(_T("Prompt")));
	else if(2 == nError)	MessageBox(ConvertString(_T("Invalid account!")),ConvertString(_T("Prompt")));
	else if(3 == nError)	MessageBox(ConvertString(_T("Timeout!")), ConvertString(_T("Prompt")));
	else if(4 == nError)	MessageBox(ConvertString(_T("The user has logged in!")),ConvertString(_T("Prompt")));
	else if(5 == nError)	MessageBox(ConvertString(_T("The user has been locked!")), ConvertString(_T("Prompt")));
	else if(6 == nError)	MessageBox(ConvertString(_T("The user has listed into illegal!")), ConvertString(_T("Prompt")));
	else if(7 == nError)	MessageBox(ConvertString(_T("The system is busy!")),ConvertString(_T( "Prompt")));
	else if(9 == nError)	MessageBox(ConvertString(_T("You Can't find the network server!")), ConvertString(_T("Prompt")));
	else	MessageBox(ConvertString(_T("Login failed!")), ConvertString(_T("Prompt")));
}

void CGateDemoDlg::CloseLoginHandle()
{
	if (m_lLoginID)
	{
		CLIENT_Logout(m_lLoginID);
		m_lLoginID = 0;
	}
}

void CGateDemoDlg::CloseAttchHandle()
{
	if (m_lAttchID)
	{
		CLIENT_StopLoadPic(m_lAttchID);
		m_lAttchID = 0;
	}
}

void CGateDemoDlg::OnBnClickedBtnLogout()
{
	// TODO: 在此添加控件通知处理程序代码
	SetWindowText(ConvertString("GateDemo"));

	m_cbxChannel.Clear();
	m_cbxChannel.ResetContent();
	m_cbxChannel.EnableWindow(FALSE);

	CloseAttchHandle();
	CloseLoginHandle();
	GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CARD_OPERATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_RECORD_OPERATE)->EnableWindow(FALSE);

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PIC));
	m_EventPic.SetImageFile(NULL);

	ClearEventInfo();
}

void CGateDemoDlg::ClearEventInfo()
{
	 SetDlgItemText(IDC_EDIT_EVENTTIME, "");
	 SetDlgItemText(IDC_EDIT_EVENTNAME, "");
	 SetDlgItemText(IDC_EDIT_CARDNAME, "");
	 SetDlgItemText(IDC_EDIT_CARDNO, "");
	 SetDlgItemText(IDC_EDIT_USERID, "");
	 SetDlgItemText(IDC_EDIT_OPENSTATUS, "");
}

void CGateDemoDlg::OnBnClickedBtnAttachevent()
{
	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	int nSel = m_cbxChannel.GetCurSel();
	if (nSel != CB_ERR)
	{
		int nChannel = m_cbxChannel.GetItemData(nSel);
		LLONG lRet = CLIENT_RealLoadPictureEx(m_lLoginID, nChannel, EVENT_IVS_ALL, TRUE, RealLoadPicCallback, (LDWORD)this, NULL);
		if (lRet == 0)
		{
			MessageBox(ConvertString(_T("Subscribe to Access Event failed!")), ConvertString(_T("Prompt")));
			GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
		}
		else
		{
			m_lAttchID = lRet;
			GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(TRUE);
		}
	}
}

void CGateDemoDlg::OnBnClickedBtnDetachevent()
{
	// TODO: 在此添加控件通知处理程序代码
	CloseAttchHandle();
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PIC));
	m_EventPic.SetImageFile(NULL);

	ClearEventInfo();

	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
}

void CGateDemoDlg::OnBnClickedBtnRecordOperate()
{
	// TODO: 在此添加控件通知处理程序代码
	CRecordManagerDlg dlg(m_lLoginID);
	dlg.DoModal();
}
