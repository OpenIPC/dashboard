// TalkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Talk.h"
#include "TalkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
#endif

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

BOOL CALLBACK MessCallBackFunc(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

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
// CTalkDlg dialog

CTalkDlg::CTalkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTalkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTalkDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
	m_lLoginHandle = 0;
	m_hTalkHandle = 0;
	m_bRecordStatus = FALSE;
	memset(&m_curTalkMode, 0, sizeof(m_curTalkMode));	
	m_lTalkChnNum = 0;
	m_lDevChnNum = 0;
}

void CTalkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTalkDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_COMBO_TRANSMIT_TYPE, m_ctlTransmitType);
	DDX_Control(pDX, IDC_COMBO_SPEAKER, m_ctlSpeakerChl);
	DDX_Control(pDX, IDC_COMBO_ENCODETYPE, m_ctlEncodeType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTalkDlg, CDialog)
	//{{AFX_MSG_MAP(CTalkDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, OnBtnLogout)
	ON_BN_CLICKED(IDC_BTN_STARTTALK, OnBtnStarttalk)
	ON_BN_CLICKED(IDC_BTN_STOPTALK, OnBtnStoptalk)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_TRANSMIT_TYPE, OnSelchangeComboTransmitType)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTalkDlg message handlers
//Callback function when device disconnected
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CTalkDlg *pThis = (CTalkDlg *)dwUser;
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

	CTalkDlg *pThis = (CTalkDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}


BOOL CTalkDlg::OnInitDialog()
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
	CLIENT_Init(DisConnectFunc, (LDWORD)this);  
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	
	NET_PARAM stuNetParam = {0};
	
	stuNetParam.nGetDevInfoTime = 3000;
	CLIENT_SetNetworkParam(&stuNetParam);

	SetDlgItemText(IDC_IPADDRESS_IP, "172.23.1.111");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USER, "admin");
	SetDlgItemText(IDC_EDIT_PWD, "admin");
	UpdateData(FALSE);
	CLIENT_SetDVRMessCallBack(MessCallBackFunc, (LDWORD)this);	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTalkDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTalkDlg::OnPaint() 
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
HCURSOR CTalkDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CTalkDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default	

	CDialog::OnClose();
}


void CTalkDlg::OnBtnLogin() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CString strIp = "";
	CString strUser = "";
	CString strPwd = "";
	int nPort = 0;

	GetDlgItemText(IDC_IPADDRESS_IP, strIp);
	GetDlgItemText(IDC_EDIT_USER, strUser);
	GetDlgItemText(IDC_EDIT_PWD, strPwd);
	nPort = GetDlgItemInt(IDC_EDIT_PORT);

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, (LPSTR)(LPCSTR)strIp, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, 		(LPSTR)(LPCSTR)strPwd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, (LPSTR)(LPCSTR)strUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LONG lLoginHandle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (lLoginHandle != 0)
	{
		m_lLoginHandle = lLoginHandle;
		m_lDevChnNum = stOutparam.stuDeviceInfo.nChanNum;
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(TRUE);

		m_ctlEncodeType.ResetContent();

		int nIndexEncodeType = 0;

		CString s;
		s.Format("PCM_%d%s_%d%s", 16, ConvertString("Bit"), 8000, ConvertString("SampleRate"));
		nIndexEncodeType = m_ctlEncodeType.AddString(s);
		m_ctlEncodeType.SetItemData(nIndexEncodeType, 0);
		m_ctlEncodeType.SetCurSel(0);

		DWORD dwRetBytes = 0;
		DHDEV_SYSTEM_ATTR_CFG stuAttr = { sizeof(DHDEV_SYSTEM_ATTR_CFG) };
		if (!CLIENT_GetDevConfig(lLoginHandle, DH_DEV_DEVICECFG, -1, &stuAttr, stuAttr.dwSize, &dwRetBytes, DEMO_SDK_WAITTIME))
		{
			MessageBox(ConvertString("Get device info failed!"), ConvertString("Prompt"));
		}
		m_lTalkChnNum = stuAttr.byTalkOutChanNum;
		m_ctlSpeakerChl.EnableWindow(FALSE);


		m_ctlTransmitType.ResetContent();
		m_ctlTransmitType.EnableWindow(TRUE);
		int nIndex = m_ctlTransmitType.AddString(ConvertString("Local(not transmit)"));
		m_ctlTransmitType.SetItemData(nIndex, FALSE);

		nIndex = m_ctlTransmitType.AddString(ConvertString("Remote(transmit)"));
		m_ctlTransmitType.SetItemData(nIndex, TRUE);
		m_ctlTransmitType.SetCurSel(0);

		OnSelchangeComboTransmitType();
	}
	else
	{
		m_lLoginHandle = 0;
		ShowLoginErrorReason(stOutparam.nError);
	}
	SetWindowText(ConvertString("Talk"));
}

void CTalkDlg::OnBtnLogout() 
{
	// TODO: Add your control notification handler code here
	if(m_bRecordStatus)
	{
		CLIENT_RecordStop();
		m_bRecordStatus = FALSE;
	}

	if(0 != m_hTalkHandle)
	{
		CLIENT_StopTalkEx(m_hTalkHandle);
		m_hTalkHandle = 0;
	}

	BOOL bRet = CLIENT_Logout(m_lLoginHandle);
	if(bRet)
	{
		m_lLoginHandle = 0;
		m_lDevChnNum = 0;
		m_lTalkChnNum = 0;
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(FALSE);

		m_ctlEncodeType.ResetContent();
		m_ctlSpeakerChl.ResetContent();
		m_ctlTransmitType.ResetContent();
	}
	else
	{
		MessageBox(ConvertString("Logout failed!"), ConvertString("Prompt"));
	}
	SetWindowText(ConvertString("Talk"));
}

//Audio data of audio talk callback function  
void CALLBACK AudioDataCallBack(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	CTalkDlg *dlg = (CTalkDlg *)dwUser;
	dlg->AudioData(lTalkHandle, pDataBuf,dwBufSize,byAudioFlag);
}

//Audio data of the audio talk callback 
void CTalkDlg::AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag)
{
	if(m_hTalkHandle == lTalkHandle)
	{
		if(0 == byAudioFlag)
		{
			//It means it is the audio data from the local reocrd library
			long lSendLen = CLIENT_TalkSendData(lTalkHandle, pDataBuf, dwBufSize);
			if(lSendLen != (long)dwBufSize)
			{
				//Error occurred when sending the user audio data to the device
			}

		}
		else if(1 == byAudioFlag)
		{
			//It means it has received the audio data from the device.
			CLIENT_AudioDec(pDataBuf, dwBufSize);			
		}
	}
}

void CTalkDlg::OnBtnStarttalk() 
{
	// TODO: Add your control notification handler code here
	if(0 != m_lLoginHandle)
	{
		//	First set audio encode format in audio talk 
		m_curTalkMode.encodeType = DH_TALK_PCM; // DH_TALK_PCM default way. Usually it is pcm type with head.
		m_curTalkMode.dwSampleRate = 8000;
		m_curTalkMode.nAudioBit = 16;

		BOOL bSuccess = CLIENT_SetDeviceMode(m_lLoginHandle, DH_TALK_ENCODE_TYPE, &m_curTalkMode);
		NET_TALK_TRANSFER_PARAM stTransfer = {sizeof(NET_TALK_TRANSFER_PARAM)};
		int nIndex = m_ctlTransmitType.GetCurSel();
		if (-1 != nIndex)
		{
			DWORD dwItemData = m_ctlTransmitType.GetItemData(nIndex);
			stTransfer.bTransfer = dwItemData;
		}

		bSuccess = CLIENT_SetDeviceMode(m_lLoginHandle, DH_TALK_TRANSFER_MODE, &stTransfer);
		if (!bSuccess)
		{
			MessageBox(ConvertString("Set transfer mode failure!"), ConvertString("Prompt"));
		}

		if ( nIndex == 1 &&  m_ctlSpeakerChl.IsWindowEnabled())
		{
			int nChannle = m_ctlSpeakerChl.GetItemData(m_ctlSpeakerChl.GetCurSel());
			bSuccess = CLIENT_SetDeviceMode(m_lLoginHandle, DH_TALK_TALK_CHANNEL, &nChannle);
			if (!bSuccess)
			{
				MessageBox(ConvertString("Set transfer channel failure!"), ConvertString("Prompt"));
			}
		}

		//	Then enable audio talk 
		LLONG lTalkHandle = CLIENT_StartTalkEx(m_lLoginHandle, AudioDataCallBack, (LDWORD)this);
		if(0 != lTalkHandle)
		{
			m_hTalkHandle = lTalkHandle;
			GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(TRUE);

			//	Then enbale local record function.(Does not call it if it is a one-direction audio talk.)
			BOOL bSuccess = CLIENT_RecordStart();
			if(bSuccess)
			{
				m_bRecordStatus = TRUE;
			}
			else
			{
				CLIENT_StopTalkEx(m_hTalkHandle);
				m_hTalkHandle = 0;
				GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(FALSE);
				MessageBox(ConvertString("Start local record failed!"), ConvertString("Prompt"));
			}
		}
		else
		{			
			MessageBox(ConvertString("Open talk failed!"), ConvertString("Prompt"));
		}
	}
	else
	{
		MessageBox(ConvertString("Please login first!"), ConvertString("Prompt"));
	}
}

void CTalkDlg::OnBtnStoptalk() 
{
	// TODO: Add your control notification handler code here
	if(0 != m_lLoginHandle)
	{
		if(m_bRecordStatus)
		{
			//Stop local record 
			CLIENT_RecordStop();
			m_bRecordStatus = FALSE;
		}

		if(0 != m_hTalkHandle)
		{
			//Stop audio talk to the device 
			BOOL bSuccess = CLIENT_StopTalkEx(m_hTalkHandle);
			if(bSuccess)
			{
				m_hTalkHandle = 0;
				GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(FALSE);
			}
			else
			{
				MessageBox(ConvertString("Stop talk failed!"), ConvertString("Prompt"));
			}
		}
	}
	else
	{
		MessageBox(ConvertString("Please login first!"), ConvertString("Prompt"));
	}
}

BOOL CALLBACK MessCallBackFunc(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if (NULL == pBuf)
	{
		return FALSE;
	}
	CString str;
	switch(lCommand)
	{        
	case DH_TALK_FAILD_EVENT:
		{
			DEV_TALK_RESULT *pSetRet =(DEV_TALK_RESULT *)pBuf;
			CTalkDlg* pClientDlg = (CTalkDlg*)dwUser;
			if (NULL != pClientDlg)
			{
				PostMessage(pClientDlg->GetSafeHwnd(), WM_COMMAND, IDC_BTN_STOPTALK, 0);
				pClientDlg->ShowTalkErrorReason(pSetRet->dwResultCode);
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}


void CTalkDlg::ShowLoginErrorReason(int nError)
{
	if(1 == nError)		MessageBox(ConvertString("incorrect password!"), ConvertString("Prompt"));
	else if(2 == nError)	MessageBox(ConvertString("no this account! "), ConvertString("Prompt"));
	else if(3 == nError)	MessageBox(ConvertString("time out!"), ConvertString("Prompt"));
	else if(4 == nError)	MessageBox(ConvertString("account have been logined!"), ConvertString("Prompt"));
	else if(5 == nError)	MessageBox(ConvertString("account have been locked!"), ConvertString("Prompt"));
	else if(6 == nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("Prompt"));
	else if(7 == nError)	MessageBox(ConvertString("The system is busy!"), ConvertString("Prompt"));
	else if(9 == nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("Prompt"));
	else	MessageBox(ConvertString("Login falied!"), ConvertString("Prompt"));
}

void CTalkDlg::ShowTalkErrorReason(int nError)
{
	if(NET_ERROR_TALK_REJECT == nError)		MessageBox(ConvertString("Talk reject!"), ConvertString("Prompt"));
	else if(NET_ERROR_TALK_RESOURCE_CONFLICIT == nError)	MessageBox(ConvertString("Talk resource conflict!"), ConvertString("Prompt"));
	else if(NET_ERROR_TALK_UNSUPPORTED_ENCODE == nError)	MessageBox(ConvertString("Talk unsupported Encode!"), ConvertString("Prompt"));
	else if(NET_ERROR_TALK_RIGHTLESS == nError)	MessageBox(ConvertString("Talk rightless!"), ConvertString("Prompt"));
	else	MessageBox(ConvertString("Talk failed!"), ConvertString("Prompt"));
}

void CTalkDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	if(m_bRecordStatus)
	{
		CLIENT_RecordStop();
	}

	if(0 != m_hTalkHandle)
	{
		CLIENT_StopTalkEx(m_hTalkHandle);
	}

	if (m_lLoginHandle != 0)
	{
		CLIENT_Logout(m_lLoginHandle);
	}

	CLIENT_Cleanup();
}

void CTalkDlg::OnSelchangeComboTransmitType() 
{
	// TODO: Add your control notification handler code here
	int nIndex = m_ctlTransmitType.GetCurSel();
	if (-1 != nIndex)
	{
		LONG lChnNum = 0;
		DWORD dwItemData = m_ctlTransmitType.GetItemData(nIndex);
		if (FALSE == dwItemData)
		{
			lChnNum = m_lTalkChnNum;
			m_ctlSpeakerChl.EnableWindow(FALSE);
		}
		else if (TRUE == dwItemData)
		{
			lChnNum = m_lDevChnNum;
			m_ctlSpeakerChl.EnableWindow(TRUE);
		}

		m_ctlSpeakerChl.ResetContent();

		for(int i=0; i< lChnNum; i++)
		{
			char szTmp[MAX_PATH] = {0};
			sprintf(szTmp,"speaker_%d", i);
			int nIndex = m_ctlSpeakerChl.AddString(szTmp);
			m_ctlSpeakerChl.SetItemData(nIndex, i);
			m_ctlSpeakerChl.SetCurSel(0);
		}
	}
}

//Process when device disconnected 
LRESULT CTalkDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CTalkDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("Talk"));
	return 0;
}

BOOL CTalkDlg::PreTranslateMessage(MSG* pMsg) 
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