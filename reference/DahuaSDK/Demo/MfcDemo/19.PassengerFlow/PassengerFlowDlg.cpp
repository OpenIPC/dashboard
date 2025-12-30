// PassengerFlowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PassengerFlow.h"
#include "PassengerFlowDlg.h"
#include "HistoryQuery.h"
#include "AlarmSubscribe.h"
#include "RealAttch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_EVENT_DISCONNECT (WM_USER + 112)
#define WM_EVENT_RECONNECT (WM_USER + 113)

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


// CPassengerFlowDlg dialog


CPassengerFlowDlg::CPassengerFlowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPassengerFlowDlg::IDD, pParent)
	, m_loginID(0)
	, m_nChannelNum(0)
	, m_lPlayID(0)
	, m_lAttachID(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pTabAlarmDlg = NULL;
	m_pTabRealAttchDlg = NULL;
}

void CPassengerFlowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CBX_CHANNEL, m_cbxChannel);
	DDX_Control(pDX, IDC_TAB1, m_Tab);
}

BEGIN_MESSAGE_MAP(CPassengerFlowDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HISTORYQUERY, &CPassengerFlowDlg::OnHistoryQuery)
	ON_BN_CLICKED(IDC_BTN_START, &CPassengerFlowDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CPassengerFlowDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_BTN_STOP, &CPassengerFlowDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CPassengerFlowDlg::OnBnClickedBtnLogout)
	ON_MESSAGE(WM_EVENT_DISCONNECT, &CPassengerFlowDlg::OnDeviceDisConnect)
	ON_MESSAGE(WM_EVENT_RECONNECT, &CPassengerFlowDlg::OnDeviceReconnect)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CPassengerFlowDlg::OnTcnSelchangeTab1)
	ON_CBN_SELCHANGE(IDC_CBX_CHANNEL, &CPassengerFlowDlg::OnCbnSelchangeCbxChannel)
END_MESSAGE_MAP()


// CPassengerFlowDlg message handlers

BOOL CPassengerFlowDlg::OnInitDialog()
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

	//SetDlgItemText(IDC_IPADDRESS, "10.11.16.227");
	SetDlgItemText(IDC_IPADDRESS, "171.2.5.123");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWD, "123dahua");

	InitNetSDK();

	InitControlTab();
	CMenu m_Menu;
	m_Menu.LoadMenu(IDR_MENU_HISTORYQUERY);
	m_Menu.ModifyMenu(0,MF_BYPOSITION,ID_HISTORYQUERY,ConvertString(_T("HistoryQuery")));
	SetMenu(&m_Menu);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPassengerFlowDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	OnBnClickedBtnStop();
	CLIENT_Logout(m_loginID);
	//OnBtnLogout();
	UnInitNetSDK();

	if (m_pTabAlarmDlg)
	{
		m_pTabAlarmDlg->DestroyWindow();
		delete m_pTabAlarmDlg;
		m_pTabAlarmDlg = NULL;
	}

	if (m_pTabRealAttchDlg)
	{
		m_pTabRealAttchDlg->DestroyWindow();
		delete m_pTabRealAttchDlg;
		m_pTabRealAttchDlg = NULL;
	}
}

void CPassengerFlowDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPassengerFlowDlg::OnPaint()
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

void CPassengerFlowDlg::InitNetSDK()
{
	BOOL bSuccess = CLIENT_Init(&CPassengerFlowDlg::DisConnectFunc,(LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);

	// close AutoReconnect
	//CLIENT_SetAutoReconnect(&CPassengerFlowDlg::HaveReConnectFunc, (LDWORD)this);
}

void CPassengerFlowDlg::UnInitNetSDK()
{
	CLIENT_Cleanup();
}

void CALLBACK CPassengerFlowDlg::DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CPassengerFlowDlg *pDlgInstance = (CPassengerFlowDlg *)dwUser;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_EVENT_DISCONNECT, 0, 0);	
	}

}

void CALLBACK CPassengerFlowDlg::HaveReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CPassengerFlowDlg *pDlgInstance = (CPassengerFlowDlg *)dwUser;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_EVENT_RECONNECT, 0, 0);
	}
}

LRESULT CPassengerFlowDlg::OnDeviceDisConnect(WPARAM wParam, LPARAM lParam)
{
	SetWindowText(ConvertString(_T("Network disconnected!")));
	return 0;
}

LRESULT CPassengerFlowDlg::OnDeviceReconnect(WPARAM wParam, LPARAM lParam)
{	
	SetWindowText(ConvertString(_T("PassengerFlow")));
	return 0;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPassengerFlowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPassengerFlowDlg::OnBnClickedBtnStart()
{
	int nSel = m_cbxChannel.GetCurSel();
	if ( (nSel < 0) || (m_loginID == 0) )
	{
		MessageBox(ConvertString(_T("Please login and select a channel before preview!")),_T("Prompt"));
		return;
	}
	//Open the preview
	int nChannelID = m_cbxChannel.GetItemData(nSel);
	if (m_lPlayID != 0)
	{
		OnBnClickedBtnStop();
	}

	int nIndex =  m_cbxChannel.GetCurSel();
	if(CB_ERR != nIndex)
	{
		int nChannel = m_cbxChannel.GetItemData(nIndex) ;
		HWND hWnd = GetDlgItem(IDC_STATIC_test)->m_hWnd;
		if (NULL == hWnd)
		{
			MessageBox(ConvertString("Failed to get window handle!"), "");
		}
		else
		{

			m_lPlayID = CLIENT_RealPlayEx(m_loginID, nChannel, hWnd, DH_RType_Realplay_0);
			if (0 == m_lPlayID)
			{
				MessageBox(ConvertString("Failed to real play!"), "");
			}
			else
			{
				BOOL bRet = CLIENT_RenderPrivateData(m_lPlayID, TRUE);
				if (!bRet)
				{
					MessageBox(ConvertString("Failed to render private data failed!"), "");
				}

				GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
			}
		}
	}
}

void CPassengerFlowDlg::OnHistoryQuery()
{
	if (m_nChannelNum && m_loginID)
	{
		CHistoryQuery dlg(m_loginID, m_nChannelNum);
		dlg.DoModal();
	}
	else
	{
		MessageBox(ConvertString(_T("Please login device first!")), "");
	}
}
void CPassengerFlowDlg::OnBnClickedBtnLogin()
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
		m_loginID = lRet;
		m_nChannelNum = stOutparam.stuDeviceInfo.nChanNum;

		/*update control state*/
		m_cbxChannel.EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);

		for(int i = 0; i < m_nChannelNum; ++i)
		{
			CString tmp ;
			tmp.Format(ConvertString(_T("%d")), i+1) ;
			m_cbxChannel.AddString(tmp);
			m_cbxChannel.SetItemData(i, (DWORD_PTR)i);
		}
		m_cbxChannel.SetCurSel(0);

		m_pTabAlarmDlg->InitALarmDlg(m_nChannelNum, m_loginID);
		m_pTabRealAttchDlg->InitRealAttchDlg(m_nChannelNum, m_loginID);

	}
	else
	{
		ShowLoginErrorReason(stOutparam.nError);
	}
}

void CPassengerFlowDlg::ShowLoginErrorReason(int nError)
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
void CPassengerFlowDlg::OnBnClickedBtnStop()
{
	if (m_lPlayID != 0)
	{
		BOOL bRealPlay = CLIENT_StopRealPlayEx(m_lPlayID);	
		if (bRealPlay == FALSE)
		{
			MessageBox(ConvertString(_T("Stop RealPlay failed!")), ConvertString(_T("Prompt")));
		}
		m_lPlayID = 0;
	}
	GetDlgItem(IDC_BTN_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	Invalidate(TRUE);
}

void CPassengerFlowDlg::OnBnClickedBtnLogout()
{
	SetWindowText(ConvertString(_T("PassengerFlow")));
	//OnBtnStopPreview();
	OnBnClickedBtnStop();
	BOOL bSuccess = CLIENT_Logout(m_loginID);
	m_loginID = 0;

	// set login button  and logout button status
	GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);

	// set preview button status
	GetDlgItem(IDC_BTN_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);

	m_cbxChannel.Clear();
	m_cbxChannel.ResetContent();
	m_cbxChannel.EnableWindow(FALSE);

	//m_pTabRealAttchDlg->DeleteAllItems();
	m_pTabRealAttchDlg->OnBnClickedButtonRealdetach();
	m_pTabRealAttchDlg->ExitDlg();

	//m_pTabAlarmDlg->DeleteAllItems();
	m_pTabAlarmDlg->OnBnClickedButtonAlarmdetach();
	m_pTabAlarmDlg->ExitDlg();
}

void CPassengerFlowDlg::InitControlTab()
{
	m_pTabAlarmDlg = new CAlarmSubscribe;
	m_pTabRealAttchDlg = new CRealAttch;

	//Create no-mode dialogue box. Specified tag control is the main window of the no-mode dialogue box.
	m_pTabAlarmDlg->Create(IDD_ALARM_ATTCH,&m_Tab);
	m_pTabRealAttchDlg->Create(IDD_REAL_ATTCH, &m_Tab);

	m_Tab.InsertItem(0, ConvertString("Real Subscribe"));
	m_Tab.InsertItem(1, ConvertString("ALARM Subscribe"));

	m_Tab.SetMinTabWidth(-1);
	m_Tab.SetPadding(CSize(4,4));
	m_Tab.SetCurSel(0);
	DoTab(0);
}

void CPassengerFlowDlg::DoTab(int nTab)
{
	//Confirm nTab value is within the threshold.
	if (nTab < 0 || nTab > 2)
	{
		nTab = 0;
	}

	if (1 == nTab)
	{
		//m_pTabMultyDlg->EnableComboBox(m_nChannelCount);
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
	SetDlgState(m_pTabRealAttchDlg,bTab[0]);
	SetDlgState(m_pTabAlarmDlg,bTab[1]);

}

void CPassengerFlowDlg::SetDlgState(CWnd *pWnd, BOOL bShow)
{
	if(bShow)
	{
		//pWnd->ShowWindow(SW_SHOW);
		pWnd->ShowWindow(SW_SHOW); 
		CRect rc;
		pWnd->GetWindowRect(rc);
		pWnd->MoveWindow(10, 30, rc.Width(), rc.Height());
		pWnd->ShowWindow(SW_SHOW); 

	}
	else
	{
		pWnd->ShowWindow(SW_HIDE);
	}
}
void CPassengerFlowDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nSelect = m_Tab.GetCurSel();
	if(nSelect>=0)
	{
		DoTab(nSelect);
	}
	*pResult = 0;
}

void CPassengerFlowDlg::OnCbnSelchangeCbxChannel()
{
	// TODO: 在此添加控件通知处理程序代码
	int nSel = m_cbxChannel.GetCurSel();
	if (nSel == CB_ERR)
	{
		return;
	}
	int nChannelID = m_cbxChannel.GetItemData(nSel);
	m_pTabRealAttchDlg->SetChannelNum(nChannelID);
	m_pTabAlarmDlg->SetChannelNum(nChannelID);
}

BOOL CPassengerFlowDlg::PreTranslateMessage(MSG* pMsg)
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
