// IntelligentTrafficCameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelligentTrafficCamera.h"
#include "IntelligentTrafficCameraDlg.h"
#include "BWListDlg.h"
#include "TrafficFlowDlg.h"
#include "QueryTrafficPictureDlg.h"
#include <iostream>
#include <assert.h>
#include <math.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef WIN32
#pragma warning (disable:4786)
#endif

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

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
// CRealLoadPictureDlg dialog

CRealLoadPictureDlg::CRealLoadPictureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRealLoadPictureDlg::IDD, pParent)
{	
	//{{AFX_DATA_INIT(CRealLoadPictureDlg)
	m_lLoginID = 0;
	m_strUserName = "";
	m_nPort = 0;
	m_strPasswd = "";

	m_lLoginID = 0;
	m_lPlayID = 0;
	m_lRealLoadPicHandle = 0;
	memset(&m_netDevInfo, 0, sizeof(m_netDevInfo));
	m_nEventCount = 0;	
	m_nChannel = 0;

	m_Font = NULL;
	InitializeCriticalSection(&m_csMap);
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
}

CRealLoadPictureDlg::~CRealLoadPictureDlg()
{		
	if (NULL != m_Font)
	{
		m_Font->DeleteObject();
		delete m_Font;
	}

	ClearRecordData();
	DeleteCriticalSection(&m_csMap);
}
void CRealLoadPictureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRealLoadPictureDlg)
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_ctrlIP);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPasswd);
	DDX_Control(pDX, IDC_COMBO_DISPLAY_FILTER, m_cbDisplayFilter);
	DDX_Control(pDX, IDC_LISTCTRL_REAL_EVENT, m_lcRealEvent);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_cbChannel);
	DDX_Control(pDX, IDC_STATIC_EVENT_LP_IMAGE, m_ctrlPlateImage);
	//}}AFX_DATA_MAP	
}

BEGIN_MESSAGE_MAP(CRealLoadPictureDlg, CDialog)
	//{{AFX_MSG_MAP(CRealLoadPictureDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, OnButtonLogout)
	ON_BN_CLICKED(IDC_BUTTON_START_PREVIEW, OnButtonStartPreview)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PREVIEW, OnButtonStopPreview)
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_CBN_SELCHANGE(IDC_COMBO_DISPLAY_FILTER, OnSelchangeComboDisplayFilter)
	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_REAL_EVENT, OnNMClickListctrlRealEvent)
	ON_WM_DESTROY()	
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnButtonClear)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LISTCTRL_REAL_EVENT, OnKeydownListctrlRealEvent)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_SNAP, OnButtonManualSnap)
	ON_MESSAGE(WM_USER_EVENT_COME, OnEventCome)
	ON_WM_TIMER()		
	ON_WM_CLOSE()
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_BUTTON_BW_LIST, &CRealLoadPictureDlg::OnBnClickedButtonBwList)
	ON_BN_CLICKED(IDC_BUTTON_TRAFFIC_FLOW, &CRealLoadPictureDlg::OnBnClickedButtonTrafficFlow)
	ON_BN_CLICKED(IDC_BUTTON_TRAFFIC_QUERY, &CRealLoadPictureDlg::OnBnClickedButtonTrafficQuery)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRealLoadPictureDlg message handlers

BOOL CRealLoadPictureDlg::OnInitDialog()
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

	::CoInitialize(NULL);	

	SetPictureStorePath();
	
	string strCfgFilePath;
	string strCfgDefaultFolder;
	GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append(CFG_CFG_FILE_NAME);
	strCfgDefaultFolder.append(PC_DATA_STORE_FOLDER);
	char szFolder[1024] = {0};
	::GetPrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), szFolder, 1024, strCfgFilePath.c_str());

	//Create direct any way
	::CreateDirectory(szFolder, NULL);
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);
	
	SetDlgItemText(IDC_IPADDRESS_IP, "10.34.3.235");
	SetDlgItemInt(IDC_EDIT_PORT, 37777);
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWORD, "admin");
	
	m_Font = new CFont;
	if (NULL != m_Font)
	{
		m_Font->CreateFont(30, 0, 0, 0, 800, 
			FALSE, 
			FALSE,
			0, 
			DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, 
			DEFAULT_QUALITY, 
			DEFAULT_PITCH | FF_MODERN, 
			"MS Sans Serif" );
		GetDlgItem(IDC_STATIC_LP)->SetFont(m_Font);
	}
	GetDlgItem(IDC_STATIC_LP)->SetWindowText("");

	InitRealEventCtrl();	
	
	m_cPictureWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1981);
	CRect screenRect;
	GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(&screenRect);
	ScreenToClient(&screenRect);
	m_cPictureWnd.MoveWindow(screenRect);
	m_cPictureWnd.ShowWindow(SW_SHOW);
	for (int i = 0; i < sizeof(nEventTypeList)/sizeof(int); i++)
	{	
		m_cbDisplayFilter.AddString(ConvertString(strEventList[i]));
		m_cbDisplayFilter.SetItemData(i, nEventTypeList[i]);
	}
		
	m_cbDisplayFilter.SetCurSel(0);

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE));

	CLIENT_Init(&CRealLoadPictureDlg::DevDisConnect, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(&CRealLoadPictureDlg::DevReConnect, (LDWORD)this);

	NET_PARAM netParam = {0};
	netParam.nGetConnInfoTime = 3000; 
	netParam.nPicBufSize = 5 * 1024 * 1024; // set picture buffer: 5M
	CLIENT_SetNetworkParam(&netParam);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRealLoadPictureDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRealLoadPictureDlg::OnPaint() 
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
HCURSOR CRealLoadPictureDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CRealLoadPictureDlg::DevReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CRealLoadPictureDlg *pThis = (CRealLoadPictureDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
	return;
}

void CRealLoadPictureDlg::DevDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CRealLoadPictureDlg *pThis = (CRealLoadPictureDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);	
	return;
}

void CRealLoadPictureDlg::OnButtonLogin() 
{
	if (UpdateData(TRUE) == FALSE)
	{
		return;
	}

	char szIp[32] = {0};
	m_ctrlIP.GetWindowText(szIp, sizeof(szIp));

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, szIp, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, m_strPasswd.GetBuffer(0), sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, m_strUserName.GetBuffer(0), sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = (WORD)m_nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lLoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (lLoginID == 0)
	{
		MessageBox(ConvertString("Login failed!"),ConvertString("Prompt"));
		return;
	}

	//Login ok
	m_lLoginID = lLoginID;
	m_nChannel = stOutparam.stuDeviceInfo.nChanNum;
	m_netDevInfo = stOutparam.stuDeviceInfo;

	GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(TRUE);

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_MANUAL_SNAP)->EnableWindow(TRUE);	

	GetDlgItem(IDC_BUTTON_TRAFFIC_FLOW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_TRAFFIC_QUERY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_BW_LIST)->EnableWindow(TRUE);

	UpdateChannelCtrl(TRUE);
	SetWindowText(ConvertString("IntelligentTrafficCamera"));
}

void CRealLoadPictureDlg::OnButtonLogout() 
{
	if (m_lLoginID == 0)
	{
		MessageBox(ConvertString("Not login yet!"),ConvertString("Prompt"));
	}
	
	OnButtonStopPreview();
	OnButtonStop();	
	OnButtonClear();
	UpdateChannelCtrl(FALSE);

	CLIENT_Logout(m_lLoginID);
	m_lLoginID = 0;

	GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);	

	GetDlgItem(IDC_BUTTON_MANUAL_SNAP)->EnableWindow(FALSE);	

	SetWindowText(ConvertString("IntelligentTrafficCamera"));	

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);		

	GetDlgItem(IDC_BUTTON_TRAFFIC_FLOW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_TRAFFIC_QUERY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_BW_LIST)->EnableWindow(FALSE);
}


void CRealLoadPictureDlg::OnClose()
{	
	CDialog::OnClose();
}

BOOL CRealLoadPictureDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg-> message   ==   WM_KEYDOWN) 
    { 
		switch(pMsg-> wParam) 
		{
		case   VK_RETURN:
			return   TRUE; 
		case   VK_ESCAPE:
			return   TRUE; 
		} 
	 } 
	return CDialog::PreTranslateMessage(pMsg);
}


void CRealLoadPictureDlg::UpdateChannelCtrl(BOOL bLogin)
{
	if (bLogin)
	{		
		int i= 0;
		for (i = 0; i < m_netDevInfo.nChanNum; i++)
		{
			char szContent[8] = {0};
			_snprintf(szContent, sizeof(szContent), "%d", i + 1);
			m_cbChannel.AddString(szContent);
			m_cbChannel.SetItemData(i, (DWORD)i);
		}
		m_cbChannel.SetCurSel(0);			
		return;
	}
	else
	{
		ZeroMemory(&m_netDevInfo, sizeof(m_netDevInfo));

		while (m_cbChannel.GetCount() > 0)
		{
			m_cbChannel.DeleteString(0);
		}
		m_cbChannel.Clear();				
		this->ChangeUI(UI_STATE_SUBSCRIBE_STOP);	
	}
}

void CRealLoadPictureDlg::OnButtonStartPreview() 
{
	HWND hWnd = GetDlgItem(IDC_STATIC_PREVIEW)->GetSafeHwnd();
	int nSel = m_cbChannel.GetCurSel();
	if ( (nSel < 0) || (m_lLoginID == 0) )
	{
		MessageBox(ConvertString("Please login and select a channel before preview!"),ConvertString("Prompt"));
		return;
	}
	//Open the preview
	int nChannelID = nSel;
	if (m_lPlayID != 0)
	{
		OnButtonStopPreview();
	}

	LLONG lRet = CLIENT_RealPlayEx(m_lLoginID, nChannelID, hWnd);
	if (0 == lRet)
	{
		MessageBox(ConvertString("Fail to play!"), ConvertString("Prompt"));
		return ;
	}

	m_lPlayID = lRet;

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(TRUE);	
}

void CRealLoadPictureDlg::OnButtonStopPreview() 
{
	if (0 == m_lPlayID)
	{
		return;
	}
	
	BOOL bRealPlay = CLIENT_StopRealPlayEx(m_lPlayID);
	if (!bRealPlay)
	{
		MessageBox(ConvertString("Stop Realplay failed"), ConvertString("Prompt"));
		return;
	}

	m_lPlayID = 0;

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PREVIEW));

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);
}

void CRealLoadPictureDlg::ChangeDisplayTitle()
{		
	int nItemCount = m_lcRealEvent.GetItemCount();
	for (int i = 0 ; i < nItemCount; i++)
	{
		StuListItemData* pItemData = (StuListItemData*)m_lcRealEvent.GetItemData(i);
		if (NULL != pItemData)
		{
			delete[] pItemData->szGUID;
			delete[] pItemData->pBuffer;
			delete pItemData;
		}		
	}
	m_lcRealEvent.DeleteAllItems();			
	
	{
		int nIndex = 0;
		CLock lock(m_csMap);		
		for (EventMap::iterator iter = m_mapAllEventItem.begin(); iter != m_mapAllEventItem.end(); iter++)
		{						
			if (WhetherToDisplayEventInCtrlList(iter->second.dwEventType))
			{
				++nIndex;
				UpdateListCtrlInfo(iter->first.c_str(), &(iter->second), true, nIndex);
			}
		}	
	}	
}

struct EventParam
{
	LLONG	lAnalyzerHandle;
	DWORD	dwEventType;		// event type
	void*	pEventInfo;			// event info
	BYTE*	pBuffer;			// picture info
	DWORD	dwBufSize;			// pBuffer size
	DWORD	dwOffset;			// plate picture offset in pBuffer
	DWORD	dwOffsetLength;		// plate picture length
	int		nSequence;
	string	strDeviceAddress;	// event address
	string	strUTCTime;			// UTC time, format is %04d%02d%02d%02d%02d%02d%03d

	EventParam() : lAnalyzerHandle(0), dwEventType(0), pEventInfo(0), pBuffer(0), dwBufSize(0), dwOffset(0), dwOffsetLength(0), nSequence(0), strDeviceAddress(""), strUTCTime("")
	{
		
	}
};


int  CRealLoadPictureDlg::RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata)
{
	CRealLoadPictureDlg* pThis = (CRealLoadPictureDlg*)dwUser;
	if (NULL == pThis)
	{
		return -1;
	}

	StuEventInfo stuEventInfo;
	BOOL bRet = GetStructInfo(dwAlarmType, pAlarmInfo, GET_STRUCT_SIZE | GET_DEVICE_ADDRESS | GET_PLATE_PICTURE_INFO, stuEventInfo);
	if (FALSE == bRet)
	{
		OutputDebugString("Alarm type not supported yet");		
		return -1;
	}	

	EventParam* eventData = new EventParam;
	if (NULL == eventData)
	{
		OutputDebugString("New memory fail");
		return -1;
	}
	
	eventData->lAnalyzerHandle = lAnalyzerHandle;
	eventData->dwEventType = dwAlarmType;
	eventData->dwBufSize = dwBufSize;
	eventData->dwOffset = stuEventInfo.dwOffset;
	eventData->dwOffsetLength = stuEventInfo.dwOffsetLength;
	eventData->nSequence = nSequence;
	eventData->strDeviceAddress = stuEventInfo.strDeviceAddress; // note, szDeviceAddress in pAlarmInfo is allocated by NetSDK, so needs to save szDeviceAddress first
	eventData->strUTCTime = stuEventInfo.strUTCTime;

	if (dwBufSize > 0)
	{
		eventData->pBuffer = new BYTE[dwBufSize]; // if dwBufSize is zero, then eventData->pBuffer is NULL
		if (NULL == eventData->pBuffer)
		{
			delete eventData;
			return -1;
		}

		memset(eventData->pBuffer, 0, dwBufSize);	
		memcpy(eventData->pBuffer, pBuffer, dwBufSize);		
	}	

	eventData->pEventInfo = new char[stuEventInfo.nStructSize];
	if (NULL == eventData->pEventInfo && dwBufSize > 0)
	{
		delete[] eventData->pBuffer;
		delete eventData;
		return -1;
	}

	memset(eventData->pEventInfo, 0, stuEventInfo.nStructSize);
	memcpy(eventData->pEventInfo, pAlarmInfo, stuEventInfo.nStructSize);

	HWND hwnd = pThis->GetSafeHwnd();
	if (!::PostMessage(hwnd, WM_USER_EVENT_COME, (WPARAM)(eventData), 0))
	{
		delete[] eventData->pBuffer;
		delete[] eventData->pEventInfo;
		delete eventData;
	}
	return 0;
}


LRESULT CRealLoadPictureDlg::OnEventCome(WPARAM wParam, LPARAM lParam)
{
	EventParam* ep = (EventParam*)wParam;
	OnReceiveEvent(ep);

	delete[] ep->pBuffer;
	delete[] ep->pEventInfo;
	delete ep;
	return 0;
}

void CRealLoadPictureDlg::OnReceiveEvent(EventParam* pEventParam)
{		
	GUID guid = {0};
	HRESULT hr =  CoCreateGuid(&guid);
	std::string strGUID(pEventParam->strUTCTime);
	strGUID.append(GuidToString(guid));	
	
	std::string strCfgFolder = GetDataFolder();
	std::string strCfgFile = strCfgFolder + std::string(".ini");
	std::string strJpgFile = strCfgFolder + strGUID + std::string(".jpg");	
	::CreateDirectory(strCfgFolder.c_str(), NULL);	

	// Store event info
	this->StoreEvent(strGUID.c_str(), strCfgFile.c_str(), strJpgFile.c_str(), pEventParam);
	
	// Update event number
	m_nEventCount++;
	SetDlgItemInt(IDC_STATIC_ALARM_COUNT, m_nEventCount);

	StuEventItem* pEventItem = GetEventItemByGUID(strGUID.c_str());
	if (pEventItem == NULL)
	{
		return;
	}
	
	// Update ListCtrl Info
	if (WhetherToDisplayEventInCtrlList(pEventItem->dwEventType))
	{
		UpdateListCtrlInfo(strGUID.c_str(), pEventItem);	
	}	

	// Update License Plate Text	
	UpdateLicensePlateNumber(pEventItem->dwEventType, pEventItem->pEventInfo);	

	// Display event image and license plate image
	DisplayImage(strGUID.c_str(), strJpgFile.c_str(), pEventItem);	
}


BOOL CRealLoadPictureDlg::UpdateListCtrlInfo(const char* szGUID, StuEventItem* pEventItem, bool bFilter/* = false*/, int nIndex/* = 0*/)
{			
	int nItemCount = m_lcRealEvent.GetItemCount();
	if (nItemCount >= MAX_EVENT_TO_DISPLAY)
	{
		StuListItemData* pItemData = (StuListItemData*)m_lcRealEvent.GetItemData(MAX_EVENT_TO_DISPLAY - 1);
		char* szGUID = pItemData->szGUID;
		BYTE* szBuf  = pItemData->pBuffer;
		delete[] szGUID;
		delete[] szBuf;
		delete pItemData;

		m_lcRealEvent.DeleteItem(MAX_EVENT_TO_DISPLAY - 1);	
	}	

	DWORD dwEventType = pEventItem->dwEventType;
	StuEventInfo stuEventInfo;
	BOOL bRet = GetStructInfo(dwEventType, pEventItem->pEventInfo, GET_EVENT_CONTENT, stuEventInfo);
	if (FALSE == bRet)
	{
		return FALSE;
	}
	
	char* szGUIDTmp = new char[64];
	if (NULL == szGUIDTmp)
	{
		return FALSE;
	}

	BYTE* szBuf = new BYTE[pEventItem->listItemData.dwBufSize];
	if (NULL == szBuf)
	{
		delete[] szGUIDTmp;
		return FALSE;
	}

	StuListItemData* pItemData = new StuListItemData;
	if (NULL == pItemData)
	{
		delete[] szGUIDTmp;
		delete[] szBuf;
		return FALSE;
	}

	memset(szGUIDTmp, 0, 64);
	strncpy(szGUIDTmp, szGUID, 63);
	memcpy(szBuf, pEventItem->listItemData.pBuffer, pEventItem->listItemData.dwBufSize);

	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = "";
	lvi.iImage = 0;
	lvi.iItem = 0;
	m_lcRealEvent.InsertItem(&lvi);	

	pItemData->szGUID		= szGUIDTmp;
	pItemData->pBuffer		= szBuf;
	pItemData->dwBufSize	= pEventItem->listItemData.dwBufSize;	
	m_lcRealEvent.SetItemData(0, (LDWORD)(pItemData));

	//Display data			
	stuEventInfo.stuEventInfoToDisplay.strCountNum = Int2Str(bFilter ? nIndex : CalculateEventNum(GetFilterEventType()));	// update the count number of event list ctrl
	stuEventInfo.stuEventInfoToDisplay.strDeviceAddress = pEventItem->strDeviceAddress;

	m_lcRealEvent.SetItemText(0, 0, stuEventInfo.stuEventInfoToDisplay.strCountNum.c_str());
	m_lcRealEvent.SetItemText(0, 1, stuEventInfo.stuEventInfoToDisplay.strTime.c_str());
	m_lcRealEvent.SetItemText(0, 2, stuEventInfo.stuEventInfoToDisplay.strEventType.c_str());

	m_lcRealEvent.SetItemText(0, 3, stuEventInfo.stuEventInfoToDisplay.strPlateNumber.c_str());
	m_lcRealEvent.SetItemText(0, 4, stuEventInfo.stuEventInfoToDisplay.strPlateColor.c_str());
	m_lcRealEvent.SetItemText(0, 5, stuEventInfo.stuEventInfoToDisplay.strPlateType.c_str());
	
	m_lcRealEvent.SetItemText(0, 6, stuEventInfo.stuEventInfoToDisplay.strVehicleType.c_str());
	m_lcRealEvent.SetItemText(0, 7, stuEventInfo.stuEventInfoToDisplay.strVehicleColor.c_str());
	m_lcRealEvent.SetItemText(0, 8, stuEventInfo.stuEventInfoToDisplay.strVehicleSize.c_str());

	m_lcRealEvent.SetItemText(0, 9, stuEventInfo.stuEventInfoToDisplay.strFileIndex.c_str());
	m_lcRealEvent.SetItemText(0, 10, stuEventInfo.stuEventInfoToDisplay.strFileCount.c_str());
	m_lcRealEvent.SetItemText(0, 11, stuEventInfo.stuEventInfoToDisplay.strGroupId.c_str());

	m_lcRealEvent.SetItemText(0, 12, stuEventInfo.stuEventInfoToDisplay.strLane.c_str());
	m_lcRealEvent.SetItemText(0, 13, stuEventInfo.stuEventInfoToDisplay.strDeviceAddress.c_str());	

	return TRUE;
}

BOOL CRealLoadPictureDlg::DisplayImage(const char* szGUID , const char* szJpgFilePath, StuEventItem* pEventItem)
{
//	DisplayEventImageAndLicensePlateImage(szJpgFilePath, pEventItem);

	DisplayEventImage(szJpgFilePath);
	DisplayLicensePlateImage(pEventItem->listItemData.pBuffer, pEventItem->listItemData.dwBufSize);
	return TRUE;
}

BOOL CRealLoadPictureDlg::DisplayLicensePlateImage(BYTE* pBuffer, size_t nSize)
{
	if (NULL == pBuffer || 0 == nSize)
	{
		FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE));
		return FALSE;
	}
	return m_ctrlPlateImage.LoadFromStream(pBuffer, nSize);
}

BOOL CRealLoadPictureDlg::DisplayEventImage(const char* szJPGPath)
{	
	CWnd* pImageWindow = GetDlgItem(IDC_STATIC_PICTURE);
	if (NULL == pImageWindow)
	{
		return FALSE;
	}

	CDC* pDC = pImageWindow->GetWindowDC();	
	if (NULL == pDC)
	{
		return FALSE;
	}

	USES_CONVERSION;
	IPicture* pPic = NULL;
	HRESULT hr = S_FALSE;
	BOOL bRet = FALSE;

	hr = OleLoadPicturePath(T2OLE(szJPGPath), NULL, 0, RESERVED_COLOR, IID_IPicture, (LPVOID*)&pPic);
	if (FAILED(hr))
	{
		OutputDebugString("OleLoadPicturePath Load Picture Fail!");
	}
	if (pPic == NULL)
	{
		goto e_clearup;
	}

	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	pPic->get_Width(&hmWidth); 
	pPic->get_Height(&hmHeight); 
	RECT rect;
	pImageWindow->GetWindowRect(&rect);
	hr = pPic->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	if (FAILED(hr))
	{
		FillCWndWithDefaultColor(pImageWindow);
		OutputDebugString("Render Fail!"); 
	}

	if (pPic != NULL)
	{
		pPic->Release(); 
	}

e_clearup:
	pImageWindow->ReleaseDC(pDC);			
	return 0;
}

void CRealLoadPictureDlg::AddEventInfo(StuEventItem& item)
{
	CLock lock(m_csMap);
	if (m_mapAllEventItem.size() >= MAX_EVENT_STORE_IN_MAP)
	{
		ClearRecordData();
	}
	
	std::pair<EventMap::iterator, bool> it = m_mapAllEventItem.insert(std::pair<std::string, StuEventItem>(std::string(item.listItemData.szGUID), item));
	if ( !it.second )
	{
	//	delete[] (item.pEventInfo);
	//	delete[] (item.listItemData.pBuffer);
	//	delete[] (item.listItemData.szGUID);
	} 
}

void CRealLoadPictureDlg::ClearRecordData()
{
	CLock lock(m_csMap);
	EventMap::iterator iter = m_mapAllEventItem.begin();
	for (; iter != m_mapAllEventItem.end(); iter++)
	{
		delete[] (iter->second.pEventInfo);
		delete[] (iter->second.listItemData.pBuffer);
		delete[] (iter->second.listItemData.szGUID);
	}
	m_mapAllEventItem.clear();
}

BOOL CRealLoadPictureDlg::StoreEvent(const char* szGUID, const char* szCfgFilePath, const char* szJpgFilePath, EventParam* pEventParam)
{
	int nStructSize = 0;
	StuEventInfo stuEventInfo;
	BOOL bRet = GetStructInfo(pEventParam->dwEventType, pEventParam->pEventInfo, GET_STRUCT_SIZE, stuEventInfo);
	if (!bRet)
	{
		return FALSE;
	}

	nStructSize = stuEventInfo.nStructSize;
	char* pStructInfo = new char[nStructSize];
	if (NULL == pStructInfo)
	{
		return FALSE;
	}
	memset(pStructInfo, 0, nStructSize);	
	memcpy(pStructInfo, pEventParam->pEventInfo, nStructSize);		

	BYTE* pBuf = NULL;
	if (pEventParam->pBuffer != NULL)
	{
		pBuf = new BYTE[pEventParam->dwOffsetLength];
		if (NULL == pBuf)
		{
			delete[] pStructInfo;
			return FALSE;
		}
		memset(pBuf, 0, pEventParam->dwOffsetLength);
		memcpy(pBuf, pEventParam->pBuffer + pEventParam->dwOffset, pEventParam->dwOffsetLength);
	}	

	char* szGUIDTmp = new char[64];
	if (NULL == szGUIDTmp)
	{
		delete[] pStructInfo;
		if (pBuf != NULL)
		{
			delete[] pBuf;			
		}
		return FALSE;		
	}
	memset(szGUIDTmp, 0, 64);
	strncpy(szGUIDTmp, szGUID, 63);	
	
	StuEventItem eventItem;
	eventItem.dwEventType = pEventParam->dwEventType;
	eventItem.pEventInfo = pStructInfo;	
	eventItem.nLen       = nStructSize;
	eventItem.nCheckFlag = 0;//Not checked
	eventItem.strDeviceAddress = pEventParam->strDeviceAddress;	

	eventItem.listItemData.pBuffer		= pBuf;
	if (pEventParam->dwBufSize > 0)
	{
		eventItem.listItemData.dwBufSize	= pEventParam->dwOffsetLength;
	}	
	eventItem.listItemData.szGUID		= szGUIDTmp;
	AddEventInfo(eventItem);
	
	if (NULL == pEventParam->pBuffer || pEventParam->dwBufSize == 0)
	{
		OutputDebugString("No Pic data\n");
		return TRUE;
	}

	FILE* fp = fopen(szJpgFilePath, "wb");
	if (fp == NULL)
	{
		OutputDebugString("Save picture failed!");		
		return FALSE;
	}
	
	fwrite(pEventParam->pBuffer, pEventParam->dwBufSize, 1, fp);
	fclose(fp);
	return TRUE;
}

void CRealLoadPictureDlg::OnButtonStart() 
{	
	if (m_lLoginID == 0)
	{
		MessageBox(ConvertString("Please login first!"),ConvertString("Prompt"));
		return;
	}
	
	NET_RESERVED_COMMON stReservedCommon = {0};
	stReservedCommon.dwStructSize = sizeof(NET_RESERVED_COMMON);

	ReservedPara stReserved;
	stReserved.dwType = RESERVED_TYPE_FOR_COMMON;
	stReserved.pData = &stReservedCommon;	
	for (int i = 0; i <= 6; i++)
	{
		stReservedCommon.dwSnapFlagMask += 1 << i;
	}
	
	bool bSuccess = false;
	int nSel = m_cbChannel.GetCurSel();
	if (nSel < 0)
	{
		MessageBox(ConvertString("Please select a channel"),ConvertString("Prompt"));
		return;
	}

	LLONG lRet = CLIENT_RealLoadPictureEx(m_lLoginID, nSel, EVENT_IVS_ALL, TRUE, CRealLoadPictureDlg::RealLoadPicCallback, (LDWORD)this, (void*)&stReserved);
	
	if (lRet == 0)
	{		
		MessageBox(ConvertString("Subscribe picture failed!"), ConvertString("Prompt"));
		return;
	}

	m_lRealLoadPicHandle = lRet;
	bSuccess = true;
	//Change UI
	if (bSuccess)
	{
		this->ChangeUI(UI_STATE_SUBSCRIBE_NOW);
	}
}


void CRealLoadPictureDlg::OnButtonStop() 
{
	if (0 == m_lRealLoadPicHandle)
	{
		return;
	}

	BOOL bRet = CLIENT_StopLoadPic(m_lRealLoadPicHandle);
	if (!bRet)
	{
		MessageBox(ConvertString("Stop Subscribe picture failed"), ConvertString("Prompt"));
		return;
	}

	m_lRealLoadPicHandle = 0;
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PICTURE));
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE));
	GetDlgItem(IDC_STATIC_LP)->SetWindowText("");

	this->ChangeUI(UI_STATE_SUBSCRIBE_STOP);
}

void CRealLoadPictureDlg::OnSelchangeComboDisplayFilter() 
{
	int nCurSel = m_cbDisplayFilter.GetCurSel();
	if (nCurSel < 0)
	{
		return;
	}	
	this->ChangeDisplayTitle();
}

void CRealLoadPictureDlg::OnNMClickListctrlRealEvent(NMHDR *pNMHDR, LRESULT *pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	if (phdn->iItem == -1)
	{
		return;
	}
	int newIndex = phdn->iItem;
	DisplaySelectedPicInfo(newIndex, &m_lcRealEvent);

	*pResult = 0;
}

void CRealLoadPictureDlg::OnDestroy()
{	
	CDialog::OnDestroy();

	ClearInfoInEventCtrl();
	CLIENT_StopRealPlayEx(m_lPlayID);

	CLIENT_Cleanup(); 
}

void CRealLoadPictureDlg::ChangeUI( int nState )
{
	switch (nState)
	{
	case UI_STATE_SUBSCRIBE_NOW:
		{				
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
		}
		break;

	case UI_STATE_SUBSCRIBE_STOP:
		{					
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		}
		break;

	default:
		break;
	}
}

void CRealLoadPictureDlg::OnButtonClear() 
{
	ClearInfoInEventCtrl();	
	ClearRecordData();

	m_nEventCount = 0;
	SetDlgItemInt(IDC_STATIC_ALARM_COUNT, m_nEventCount, FALSE);
}


void CRealLoadPictureDlg::OnKeydownListctrlRealEvent(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	int nCurIndex = (int)m_lcRealEvent.GetFirstSelectedItemPosition() - 1;

	if (pLVKeyDow->wVKey == 38 || pLVKeyDow->wVKey == 40)
	{
		int newIndex = 0;
		if (pLVKeyDow->wVKey == 38)//up
		{
			newIndex = (nCurIndex == 0)? nCurIndex : nCurIndex -1;
		}
		else//down
		{
			newIndex = (nCurIndex == m_lcRealEvent.GetItemCount()-1) ? nCurIndex : nCurIndex + 1;
		}
		DisplaySelectedPicInfo(newIndex, &m_lcRealEvent);
	}
	*pResult = 0;
}


void CRealLoadPictureDlg::DisplaySelectedPicInfo(int nItemIndex, CListCtrl* listCtrl)
{	
	if ( (nItemIndex < 0) ||(nItemIndex >= listCtrl->GetItemCount()))
	{
		return;
	}
	
	std::string strDataFolder = GetDataFolder();	

	StuListItemData* pItemData = (StuListItemData*)listCtrl->GetItemData(nItemIndex);
	if (NULL == pItemData || NULL == pItemData->szGUID)
	{
		return;
	}

	strDataFolder.append(pItemData->szGUID);	
	
	std::string strJpgFilePath = strDataFolder.append(".jpg");	
	StuEventItem* pEventItem = GetEventItemByGUID(pItemData->szGUID);
	if (pEventItem == NULL)
	{
		return;
	}	
	
	UpdateLicensePlateNumber(pEventItem->dwEventType, pEventItem->pEventInfo);
	DisplayImage(pItemData->szGUID, strJpgFilePath.c_str(), pEventItem);
}


HBRUSH CRealLoadPictureDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);	
	
	return hbr;
}

void CRealLoadPictureDlg::OnButtonManualSnap() 
{
	UpdateData(TRUE);
	int nChannel = m_cbChannel.GetCurSel();
	if ( (nChannel < 0) || (m_lLoginID == 0) )
	{
		MessageBox(ConvertString("Please login and select a channel before manual snap!"),ConvertString("Prompt"));
		return;
	}

	if (0 == m_lRealLoadPicHandle)
	{
		MessageBox(ConvertString("Please start subscribe before manual snap"),ConvertString("Prompt"));
		return;
	}

	MANUAL_SNAP_PARAMETER stuSanpParam = {0};
	stuSanpParam.nChannel = 0;	
	CString strSnapSeq = "1213"; // if the device supports snap sequence, it will return this value in szManualSnapNo of DEV_EVENT_TRAFFIC_MANUALSNAP_INFO 
	memcpy(stuSanpParam.bySequence, strSnapSeq, strSnapSeq.GetLength());
	BOOL bRet = CLIENT_ControlDeviceEx(m_lLoginID, DH_MANUAL_SNAP, &stuSanpParam, NULL, 3000);
	if (!bRet)
	{		
		MessageBox(ConvertString("Manual Snap failed!"),ConvertString("Prompt"));
		return;
	}
}


StuEventItem* CRealLoadPictureDlg::GetEventItemByGUID(const char* szGUID)
{
	std::string strGUID(szGUID);

	CLock lock(m_csMap);
	EventMap::iterator iter = m_mapAllEventItem.find(strGUID);
	if ( iter != m_mapAllEventItem.end() )
	{
		return &(iter->second);
	}
	return NULL;
}

//Process when device disconnected 
LRESULT CRealLoadPictureDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CRealLoadPictureDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("IntelligentTrafficCamera"));
	return 0;
}

void CRealLoadPictureDlg::InitRealEventCtrl(void)
{
	m_lcRealEvent.SetExtendedStyle(m_lcRealEvent.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_lcRealEvent.SetExtendedStyle(m_lcRealEvent.GetExtendedStyle()|LVS_EX_GRIDLINES); 	
	
	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("Index", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("Type", 120));

	vecTitles.push_back(ColDesObj("Plate Text", 100));
	vecTitles.push_back(ColDesObj("Plate Color", 60));
	vecTitles.push_back(ColDesObj("Plate Type", 60));

	vecTitles.push_back(ColDesObj("Vehicle Type", 70));
	vecTitles.push_back(ColDesObj("Vehicle Color", 60));
	vecTitles.push_back(ColDesObj("Vehicle Size", 70));

	vecTitles.push_back(ColDesObj("FileIndex", 80));
	vecTitles.push_back(ColDesObj("FileCount", 80));
	vecTitles.push_back(ColDesObj("GroupId", 80));	

	vecTitles.push_back(ColDesObj("Lane Number", 60));
	vecTitles.push_back(ColDesObj("Address", 100));

	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_lcRealEvent.InsertColumn(j, &lvc);
	}
}


void CRealLoadPictureDlg::ClearInfoInEventCtrl()
{
	int nItemCount = m_lcRealEvent.GetItemCount();
	for (int i = 0 ; i < nItemCount; i++)
	{
		StuListItemData* pItemData = (StuListItemData*)m_lcRealEvent.GetItemData(i);
		delete[] pItemData->pBuffer;
		delete[] pItemData->szGUID;
		delete pItemData;		
	}
	m_lcRealEvent.DeleteAllItems();
}

void CRealLoadPictureDlg::SetPictureStorePath(void)
{
	string strCfgFilePath;
	string strCfgDefaultFolder;
	GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append(CFG_CFG_FILE_NAME);
	strCfgDefaultFolder.append(PC_DATA_STORE_FOLDER);		
	
	::WritePrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), strCfgFilePath.c_str());
}


BOOL CRealLoadPictureDlg::UpdateLicensePlateNumber(const DWORD dwEventType, const void* pEventInfo)
{
	if (NULL == pEventInfo)
	{
		return FALSE;
	}

	std::string strLicensePlate = "";
	switch(dwEventType)
	{		
	case EVENT_IVS_TRAFFICJUNCTION:
		{
			DEV_EVENT_TRAFFICJUNCTION_INFO* pInfo = (DEV_EVENT_TRAFFICJUNCTION_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);			
		}
		break;			
	case EVENT_IVS_TRAFFIC_RUNREDLIGHT:
		{
			DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO* pInfo = (DEV_EVENT_TRAFFIC_RUNREDLIGHT_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);				
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERLINE:
		{			
			DEV_EVENT_TRAFFIC_OVERLINE_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERLINE_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);			
		}
		break;
	case EVENT_IVS_TRAFFIC_RETROGRADE:
		{
			DEV_EVENT_TRAFFIC_RETROGRADE_INFO* pInfo = (DEV_EVENT_TRAFFIC_RETROGRADE_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);			
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERSPEED:
		{
			DEV_EVENT_TRAFFIC_OVERSPEED_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERSPEED_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_UNDERSPEED:
		{	
			DEV_EVENT_TRAFFIC_UNDERSPEED_INFO* pInfo = (DEV_EVENT_TRAFFIC_UNDERSPEED_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;	
	case EVENT_IVS_TRAFFIC_PARKING:
		{			
			DEV_EVENT_TRAFFIC_PARKING_INFO* pInfo = (DEV_EVENT_TRAFFIC_PARKING_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_WRONGROUTE:
		{			
			DEV_EVENT_TRAFFIC_WRONGROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_WRONGROUTE_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_CROSSLANE:
		{	
			DEV_EVENT_TRAFFIC_CROSSLANE_INFO* pInfo = (DEV_EVENT_TRAFFIC_CROSSLANE_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stuTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERYELLOWLINE:
		{
			DEV_EVENT_TRAFFIC_OVERYELLOWLINE_INFO* pInfo = (DEV_EVENT_TRAFFIC_OVERYELLOWLINE_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;	
	case EVENT_IVS_TRAFFIC_YELLOWPLATEINLANE:
		{
			DEV_EVENT_TRAFFIC_YELLOWPLATEINLANE_INFO* pInfo = (DEV_EVENT_TRAFFIC_YELLOWPLATEINLANE_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;	
	case EVENT_IVS_TRAFFIC_VEHICLEINROUTE:
		{
			DEV_EVENT_TRAFFIC_VEHICLEINROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_VEHICLEINROUTE_INFO*)pEventInfo;			
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_MANUALSNAP:
		{
			DEV_EVENT_TRAFFIC_MANUALSNAP_INFO* pInfo = (DEV_EVENT_TRAFFIC_MANUALSNAP_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;

	case EVENT_IVS_TRAFFIC_PEDESTRAINPRIORITY:
		{
			DEV_EVENT_TRAFFIC_PEDESTRAINPRIORITY_INFO* pInfo = (DEV_EVENT_TRAFFIC_PEDESTRAINPRIORITY_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_VEHICLEINBUSROUTE:
		{
			DEV_EVENT_TRAFFIC_VEHICLEINBUSROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_VEHICLEINBUSROUTE_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
		}
		break;
	case EVENT_IVS_TRAFFIC_WITHOUT_SAFEBELT:
		{	
			DEV_EVENT_TRAFFIC_WITHOUT_SAFEBELT* pInfo = (DEV_EVENT_TRAFFIC_WITHOUT_SAFEBELT*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stuTrafficCar.szPlateNumber);	
		}
		break;

	case EVENT_IVS_TRAFFICJAM:
		{
			DEV_EVENT_TRAFFICJAM_INFO* pInfo = (DEV_EVENT_TRAFFICJAM_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;

	case EVENT_IVS_TRAFFIC_PEDESTRAIN:	
		{
			strLicensePlate = "";
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNLEFT:
		{
			DEV_EVENT_TRAFFIC_TURNLEFT_INFO* pInfo = (DEV_EVENT_TRAFFIC_TURNLEFT_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNRIGHT:
		{
			DEV_EVENT_TRAFFIC_TURNRIGHT_INFO* pInfo = (DEV_EVENT_TRAFFIC_TURNRIGHT_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_UTURN:
		{
			DEV_EVENT_TRAFFIC_UTURN_INFO* pInfo = (DEV_EVENT_TRAFFIC_UTURN_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_OVERSTOPLINE:
		{
			DEV_EVENT_TRAFFIC_OVERSTOPLINE* pInfo = (DEV_EVENT_TRAFFIC_OVERSTOPLINE*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stuTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_DRIVER_SMOKING:
		{
			DEV_EVENT_TRAFFIC_DRIVER_SMOKING* pInfo = (DEV_EVENT_TRAFFIC_DRIVER_SMOKING*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stuTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_DRIVER_CALLING:
		{
			DEV_EVENT_TRAFFIC_DRIVER_CALLING* pInfo = (DEV_EVENT_TRAFFIC_DRIVER_CALLING*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stuTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNLEFTAFTERSTRAIGHT:
		{
			DEV_EVENT_TURNLEFTAFTERSTRAIGHT_INFO* pInfo = (DEV_EVENT_TURNLEFTAFTERSTRAIGHT_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_QUEUEJUMP:
		{
			DEV_EVENT_TRAFFIC_QUEUEJUMP_INFO* pInfo = (DEV_EVENT_TRAFFIC_QUEUEJUMP_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNRIGHTAFTERSTRAIGHT:
		{
			DEV_EVENT_TURNRIGHTAFTERSTRAIGHT_INFO* pInfo = (DEV_EVENT_TURNRIGHTAFTERSTRAIGHT_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_TURNRIGHTAFTERPEOPLE:
		{
			DEV_EVENT_TURNRIGHTAFTERPEOPLE_INFO* pInfo = (DEV_EVENT_TURNRIGHTAFTERPEOPLE_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_TRUCKFORBID:
		{
			DEV_EVENT_TRAFFICTRUCKFORBID_INFO* pInfo = (DEV_EVENT_TRAFFICTRUCKFORBID_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_PEDESTRIAN_JUNCTION:
		{
			strLicensePlate = "";
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTORINMOTORROUTE:
		{
			DEV_EVENT_TRAFFIC_NONMOTORINMOTORROUTE_INFO* pInfo = (DEV_EVENT_TRAFFIC_NONMOTORINMOTORROUTE_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTOR_OVERLOAD:
		{
			strLicensePlate = "";
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTOR_WITHOUTSAFEHAT:
		{
			strLicensePlate = "";
		}
		break;
	case EVENT_IVS_TRAFFIC_NONMOTOR_HOLDUMBRELLA:
		{
			strLicensePlate = "";
		}
		break;
	default:
		strLicensePlate = "";
		break;
	}

	CWnd* pWnd = GetDlgItem(IDC_STATIC_LP);
	if (pWnd)
	{
		pWnd->SetWindowText(strLicensePlate.c_str());
	}
	
	return TRUE;
}

BOOL CRealLoadPictureDlg::DisplayEventImageAndLicensePlateImage(const char* szJpgPath, const StuEventItem* item)
{	
	CWnd* pEventImageWnd	= GetDlgItem(IDC_STATIC_PICTURE);
	CWnd* pLicensePlateWnd	= GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE);

	CDC* pLPDC = pLicensePlateWnd->GetWindowDC();
	CDC* pDC = pEventImageWnd->GetWindowDC();
	CDC* pSrcDC = CWnd::GetDesktopWindow()->GetDC();

	USES_CONVERSION;
	IPicture* pPic = NULL;
	HRESULT hr = S_FALSE;
	BOOL bRet = FALSE;

	hr = OleLoadPicturePath(T2OLE(szJpgPath), NULL, 0, RESERVED_COLOR, IID_IPicture, (LPVOID*)&pPic);
	if (FAILED(hr))
	{
		OutputDebugString("OleLoadPicturePath Load Picture Fail!");
	}
	if (pPic == NULL)
	{
		goto e_clearup;
	}

	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	pPic->get_Width(&hmWidth); 
	pPic->get_Height(&hmHeight); 
	RECT rect;
	pEventImageWnd->GetWindowRect(&rect);
	hr = pPic->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	if (FAILED(hr))
	{
		OutputDebugString("Render Fail!"); 
	}

	if (IsTypeHasLP(item->dwEventType))
	{		
		StuEventInfo stuEventInfo;
		BOOL bRet = GetStructInfo(item->dwEventType, item->pEventInfo, GET_PICTURE_RECT, stuEventInfo);
		if (!bRet)
		{
			FillCWndWithDefaultColor(pLicensePlateWnd);
			goto e_clearup;
		}

		DH_RECT& rectBoundingBox = stuEventInfo.rectBoundingBox;

		//1.BoundingBox的值是在8192*8192坐标系下的值，必须转化为图片中的坐标
		//2.OSD在图片中占了64行,如果没有OSD，下面的关于OSD的处理需要去掉(把OSD_HEIGHT置为0)
		const int OSD_HEIGHT = 64;
		long nWidth;
		long nHeight;
		nWidth =  MulDiv(hmWidth, pSrcDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		nHeight = MulDiv(hmHeight, pSrcDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);
		nHeight = nHeight - OSD_HEIGHT;
		if ( (nWidth <= 0) || (nHeight <=0) )
		{
			goto e_clearup;
			return FALSE;
		}

		DH_RECT dstRect = {0};

		dstRect.left	= ceil((double)(nWidth  * rectBoundingBox.left)   / 8192 );
		dstRect.right	= ceil((double)(nWidth  * rectBoundingBox.right)  / 8192 );
		dstRect.bottom	= ceil((double)(nHeight * rectBoundingBox.bottom) / 8192 );
		dstRect.top		= ceil((double)(nHeight * rectBoundingBox.top)  / 8192 );

		double rate = (double)hmWidth / (double)nWidth;
		int x = (double)dstRect.left * rate;
		int y = (double)(dstRect.top + OSD_HEIGHT) * rate;
		int w = (double)(dstRect.right - dstRect.left) * rate;
		int h = (double)(dstRect.bottom - dstRect.top) * rate;

		pLicensePlateWnd->GetWindowRect(&rect);
		hr = pPic->Render(*pLPDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 
			x, 
			hmHeight - y, 
			w, 
			0-h, 	
			NULL);

		if (FAILED(hr))
		{
			FillCWndWithDefaultColor(pLicensePlateWnd);
			OutputDebugString("Render Fail!"); 
		}
	}


e_clearup:
	if (pPic != NULL)
	{
		pPic->Release(); 
	}

	pEventImageWnd->ReleaseDC(pDC);
	pLicensePlateWnd->ReleaseDC(pLPDC);
	CWnd::GetDesktopWindow()->ReleaseDC(pSrcDC);
	return 0;
}


void CRealLoadPictureDlg::FillCWndWithDefaultColor(CWnd* cwnd) 
{
	if (NULL == cwnd)
	{
		return;
	}

	CDC* cdc = cwnd->GetDC();
	if (NULL == cdc)
	{
		return;
	}

	RECT rect;
	cwnd->GetClientRect(&rect);
	CBrush brush(PICTURE_BACK_COLOR);
	cdc->FillRect(&rect, &brush);
	cwnd->ReleaseDC(cdc);
}

size_t CRealLoadPictureDlg::CalculateEventNum(DWORD dwType)
{
	if (dwType == EVENT_IVS_ALL)
	{
		return m_nEventCount;
	}
	
	size_t nCount = 0;

	CLock lock(m_csMap);
	EventMap::iterator iter = m_mapAllEventItem.begin();
	for (; iter != m_mapAllEventItem.end(); iter++)
	{
		if (dwType == iter->second.dwEventType)
		{
			nCount ++;
		}
	}
	return nCount;
}
void CRealLoadPictureDlg::OnBnClickedButtonBwList()
{
	// TODO: 在此添加控件通知处理程序代码
	CBWListDlg dlg(NULL,m_nChannel,m_lLoginID);
	dlg.DoModal();
}

void CRealLoadPictureDlg::OnBnClickedButtonTrafficFlow()
{
	// TODO: 在此添加控件通知处理程序代码
	CTrafficFlowDlg dlg(NULL,m_nChannel,m_lLoginID);
	dlg.DoModal();
}

void CRealLoadPictureDlg::OnBnClickedButtonTrafficQuery()
{
	// TODO: 在此添加控件通知处理程序代码
	CQueryTrafficPictureDlg dlg(NULL,m_nChannel,m_lLoginID);
	dlg.DoModal();
}
