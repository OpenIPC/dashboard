// IntelligentDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelligentDevice.h"
#include "IntelligentDeviceDlg.h"
#include "QueryDownload.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <math.h>
#include <atlconv.h>
#include <algorithm>


#define WM_USER_ALAMR_COME (WM_USER + 111)
#define WM_EVENT_DISCONNECT (WM_USER + 112)
#define WM_EVENT_RECONNECT (WM_USER + 113)

#define LISTSIZE 50
#define GUIDLEN  64

struct EventPara
{
	LLONG lAnalyzerHandle;
	DWORD dwAlarmType;
	char* pAlarmInfo;
	BYTE *pBuffer;
	DWORD dwBufSize;
	int nSequence;
};

typedef struct __ALARM_ITEM
{
	std::string strGUID;
	int		nAlarmType;

}ALARM_ITEM, *LPALARM_ITEM;


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


// CIntelligentDeviceDlg dialog


CIntelligentDeviceDlg::CIntelligentDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIntelligentDeviceDlg::IDD, pParent)
{
	m_lPlayID = 0;
	m_loginID = 0;
	m_lAttchID = 0;
	m_nChannelNum = 0;

	m_nFilterEventType = EVENT_IVS_ALL;
	m_index = 0;


	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CIntelligentDeviceDlg::~CIntelligentDeviceDlg()
{
	DeleteAllStoreEventData();
	if (m_loginID)
	{
		CLIENT_Logout(m_loginID);
		m_loginID = 0;
	}
	StopAttch();
	CLIENT_Cleanup();
}

void CIntelligentDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_EVENTINFO, m_lcEventInfo);
	DDX_Control(pDX, IDC_CBX_CHANNEL, m_cbxChannel);
	DDX_Control(pDX, IDC_CBX_EVENT, m_cbxEvent);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_EventPic);
}

BEGIN_MESSAGE_MAP(CIntelligentDeviceDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CIntelligentDeviceDlg::OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CIntelligentDeviceDlg::OnBtnLogout)

	ON_BN_CLICKED(IDC_BTN_STARTREALPLAY, &CIntelligentDeviceDlg::OnBtnStartrealplay)
	ON_BN_CLICKED(IDC_BTN_STOPREALPLAY, &CIntelligentDeviceDlg::OnBtnStoprealplay)
	ON_BN_CLICKED(IDC_BTN_ATTACHEVENT, &CIntelligentDeviceDlg::OnBtnAttachevent)
	ON_BN_CLICKED(IDC_BTN_DETACHEVENT, &CIntelligentDeviceDlg::OnBtnDetachevent)
	ON_MESSAGE(WM_USER_ALAMR_COME, &CIntelligentDeviceDlg::OnAlarmCome)
	ON_MESSAGE(WM_EVENT_DISCONNECT, &CIntelligentDeviceDlg::OnDeviceDisConnect)
	ON_MESSAGE(WM_EVENT_RECONNECT, &CIntelligentDeviceDlg::OnDeviceReconnect)
	ON_CBN_SELCHANGE(IDC_CBX_EVENT, &CIntelligentDeviceDlg::OnCbnSelchangeCbxEvent)

	ON_COMMAND(ID_QUERYDOWNLOAD, &CIntelligentDeviceDlg::OnQueryDownload)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_EVENTINFO, &CIntelligentDeviceDlg::OnNMDblclkListEventinfo)

	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CIntelligentDeviceDlg::OnQueryDownload()
{
	if (m_nChannelNum && m_loginID)
	{
		CQueryDownload dlg(m_nChannelNum, m_loginID);
		dlg.DoModal();
	}
	else
	{
		MessageBox(ConvertString("Please Login First!"), "");
	}
}

BOOL CIntelligentDeviceDlg::PreTranslateMessage(MSG* pMsg)
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

void CIntelligentDeviceDlg::OnBtnLogin() 
{
	// TODO: Add your control notification handler code here

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

	//Call log in interface 
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if(0 != lRet)
	{
		m_loginID = lRet;
		m_nChannelNum = stOutparam.stuDeviceInfo.nChanNum;

		/*update control state*/
		m_cbxChannel.EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_STARTREALPLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOPREALPLAY)->EnableWindow(FALSE);

		GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);

		InitEventCombox();

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

void CIntelligentDeviceDlg::OnBtnLogout() 
{
	//TODO: Add your control notification handler code here
	SetWindowText(ConvertString(_T("IntelligentDevice")));
	OnBtnStoprealplay();

	if (m_loginID)
	{
		CLIENT_Logout(m_loginID);
	}
	m_loginID = 0;
	m_nChannelNum = 0;
	m_cbxEvent.SetCurSel(0);

	// set login button  and logout button status
	GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);

	// set preview button status
	GetDlgItem(IDC_BTN_STARTREALPLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOPREALPLAY)->EnableWindow(FALSE);

	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);

	m_cbxChannel.Clear();
	m_cbxChannel.ResetContent();
	m_cbxChannel.EnableWindow(FALSE);

	m_cbxEvent.Clear();
	m_cbxEvent.ResetContent();
	m_cbxEvent.EnableWindow(FALSE);

	DeleteAllStoreEventData();
	DeleteAllItems();

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_REALPLAY_PIC));
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PICTURE));
	m_EventPic.SetImageFile(NULL);

	Invalidate(TRUE);

	m_index = 0;
}

// CIntelligentDeviceDlg message handlers

BOOL CIntelligentDeviceDlg::OnInitDialog()
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

	g_SetWndStaticText(this);

	m_dlgShowPic.Create(IDD_SHOWPIC);

	SetDlgItemText(IDC_IPADDRESS, "172.30.7.243");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWD, "admin123");

	InitEventListCtrl();

	InitNetSDK();

	CMenu m_Menu;
	m_Menu.LoadMenu(IDR_MENU_QUERYDOWNLAD);
	m_Menu.ModifyMenu(0,MF_BYPOSITION,ID_QUERYDOWNLOAD,ConvertString(_T("QueryDownload")));
	SetMenu(&m_Menu);

	// TODO: Add extra initialization here
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIntelligentDeviceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CIntelligentDeviceDlg::OnPaint()
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
HCURSOR CIntelligentDeviceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CIntelligentDeviceDlg::OnBtnStartrealplay() 
{
	// TODO: Add your control notification handler code here
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
		OnBtnStoprealplay();
	}

	int nIndex =  m_cbxChannel.GetCurSel();
	if(CB_ERR != nIndex)
	{
		int nChannel = m_cbxChannel.GetItemData(nIndex) ;
		HWND hWnd = GetDlgItem(IDC_STATIC_PREVIEW)->m_hWnd;
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

				GetDlgItem(IDC_BTN_STARTREALPLAY)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_STOPREALPLAY)->EnableWindow(TRUE);
			}
		}
	}
}


void CIntelligentDeviceDlg::OnBtnStoprealplay() 
{
	// TODO: Add your control notification handler code here
	if (m_lPlayID != 0)
	{
		BOOL bRealPlay = CLIENT_StopRealPlayEx(m_lPlayID);
		if (bRealPlay == FALSE)
		{
			MessageBox(ConvertString(_T("Stop RealPlay failed!")),ConvertString(_T("Prompt")));
		}	
		m_lPlayID = 0;
	}
	GetDlgItem(IDC_BTN_STARTREALPLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOPREALPLAY)->EnableWindow(FALSE);
	Invalidate(TRUE);
}


void CIntelligentDeviceDlg::OnBtnAttachevent() 
{
	// TODO: Add your control notification handler code here
	// before attach intelligent event,you must login device first

	StopAttch();
	m_index = 0;

	int nSel = m_cbxChannel.GetCurSel();
	if (nSel != CB_ERR)
	{
		int nChannel = m_cbxChannel.GetItemData(nSel);
		LLONG lRet = CLIENT_RealLoadPictureEx(m_loginID, nChannel, EVENT_IVS_ALL, TRUE, CIntelligentDeviceDlg::RealLoadPicCallback, (LDWORD)this, NULL);
		if (lRet == 0)
		{
			int n = CLIENT_GetLastError();
			MessageBox(ConvertString(_T("Subscribe to Intelligent Event failed!")), ConvertString(_T("Prompt")));
		}
		else
		{
			m_lAttchID = lRet;
		}
	}

	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(TRUE);
	m_cbxEvent.EnableWindow(TRUE);
}

void CIntelligentDeviceDlg::OnBtnDetachevent() 
{
	StopAttch();

	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);

	m_lcEventInfo.DeleteAllItems();

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PICTURE));
	m_EventPic.SetImageFile(NULL);

	DeleteAllStoreEventData();
	DeleteAllItems();

	m_cbxEvent.SetCurSel(0);
	m_cbxEvent.EnableWindow(FALSE);
}

void CIntelligentDeviceDlg::StopAttch()
{
	if (m_lAttchID)
	{
		CLIENT_StopLoadPic(m_lAttchID);
	}
	m_lAttchID = 0;
}

BOOL CIntelligentDeviceDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DestroyWindow();
}
void CIntelligentDeviceDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	DeleteAllItems();
	OnBtnLogout();
	UnInitNetSDK();
	m_dlgShowPic.DestroyWindow();

}	

LRESULT CIntelligentDeviceDlg::OnDeviceDisConnect(WPARAM wParam, LPARAM lParam)
{
	//	MessageBox("Network disconnected!", "Prompt");
	SetWindowText(ConvertString(_T("Network disconnected!")));
	return 0;
}

LRESULT CIntelligentDeviceDlg::OnDeviceReconnect(WPARAM wParam, LPARAM lParam)
{	
	//	MessageBox("Reconnect Device Success!", "Prompt");
	SetWindowText(ConvertString(_T("IntelligentDevice")));
	return 0;
}

/************************************************************************/
/*handle message WM_USER_ALAMR_COME                                        */
/************************************************************************/
LRESULT CIntelligentDeviceDlg::OnAlarmCome(WPARAM wParam, LPARAM lParam)
{
	EventPara* ep = (EventPara*)wParam;

	this->DealWithEvent(ep->lAnalyzerHandle, ep->dwAlarmType, ep->pAlarmInfo, ep->pBuffer, ep->dwBufSize, ep->nSequence);

	delete[] ep->pBuffer;
	delete[] ep->pAlarmInfo;
	delete ep;
	return 0;
}

/************************************************************************/
/* handle the receive event												*/
/* picture group: one picture, multi  events							*/                     
/* event group:one event, multi pictures			                    */
/************************************************************************/
void CIntelligentDeviceDlg::DealWithEvent(LLONG lAnalyzerHandle, DWORD dwAlarmType, char* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, int nSequence)
{
	// get event occur time 

	// some device one picture corresponding  multi events
	// identity the picture 
	if (nSequence == 0) // picture group begin 
	{

	}

	else if (nSequence == 2) // picture group end
	{

	}

	DisplayEventInfo(m_nFilterEventType,dwAlarmType,pAlarmInfo, pBuffer, dwBufSize );

}
/************************************************************************/
/* SDK回调函数                                                          */
/************************************************************************/
int CALL_METHOD CIntelligentDeviceDlg::RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata)
{
	DWORD nAlarmInfoSize = 0;

	int nRet =  GetAlarmInfoSize(dwAlarmType, &nAlarmInfoSize);
	if (nRet == -1)
	{
		return -1;
	}

	//Get the CIntelligentDeviceDlg instance
	CIntelligentDeviceDlg *pEventInstance = (CIntelligentDeviceDlg*)dwUser;

	EventPara* eventData = new EventPara;
	eventData->lAnalyzerHandle = lAnalyzerHandle;
	eventData->dwAlarmType = dwAlarmType;
	eventData->dwBufSize = dwBufSize;
	eventData->nSequence = nSequence;

	eventData->pBuffer = new BYTE[dwBufSize];

	memcpy(eventData->pBuffer, pBuffer, dwBufSize);

	eventData->pAlarmInfo = new char[nAlarmInfoSize];

	memcpy(eventData->pAlarmInfo, pAlarmInfo, nAlarmInfoSize);

	HWND hwnd = pEventInstance->GetSafeHwnd();                                     

	::PostMessage(hwnd, WM_USER_ALAMR_COME, (WPARAM)(eventData), 0);
	return 0;
}

void CIntelligentDeviceDlg::InitNetSDK()
{
	BOOL bSuccess = CLIENT_Init(&CIntelligentDeviceDlg::DisConnectFunc,(LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(&CIntelligentDeviceDlg::HaveReConnectFunc, (LDWORD)this);
}

void CIntelligentDeviceDlg::UnInitNetSDK()
{
	CLIENT_Cleanup();
}

void CALLBACK CIntelligentDeviceDlg::DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CIntelligentDeviceDlg *pDlgInstance = (CIntelligentDeviceDlg *)dwUser;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_EVENT_DISCONNECT, 0, 0);	
	}

}

void CALLBACK CIntelligentDeviceDlg::HaveReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CIntelligentDeviceDlg *pDlgInstance = (CIntelligentDeviceDlg *)dwUser;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_EVENT_RECONNECT, 0, 0);
	}
}

void CIntelligentDeviceDlg::ShowLoginErrorReason(int nError)
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



void CIntelligentDeviceDlg::DisplayEventInfo(DWORD filterType, DWORD dwAlarmType, char* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize)
{
	// 0: all, 
	// 1: EVENT_IVS_CROSSLINEDETECTION, 
	// 2: EVENT_IVS_CROSSREGIONDETECTION, 

	std::string strTime;
	std::string strdetail;
	std::string strObjectType;
	int channel = 1;
	int nRet = GetShowItemInfo(dwAlarmType, pAlarmInfo, strTime,channel,strdetail,strObjectType);

	GUID guid = {0};
	HRESULT hr =  CoCreateGuid(&guid);
	char bufGUID[64] = {0};
	sprintf_s(
		bufGUID,
		"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	std::string strGUID(bufGUID);

	m_index++;
	StoreEventInfoToList(channel,dwAlarmType,pBuffer,dwBufSize,strGUID,strTime,strdetail, strObjectType );


	if (filterType == EVENT_IVS_ALL || filterType == dwAlarmType)
	{
		m_EventPic.SetImageDate(pBuffer, dwBufSize);

		LV_ITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.pszText = _T("");
		lvi.iImage = 0;
		lvi.iItem = 0;

		CString cstrChannel ;
		cstrChannel.Format(_T("%d"),channel);

		CString cstrIndex;
		cstrIndex.Format(_T("%d"), m_index);

		CString cstrDetail(strdetail.c_str());
		CString cstrTime(strTime.c_str());
		CString cstrObject(strObjectType.c_str());

		m_lcEventInfo.InsertItem(&lvi);

		m_lcEventInfo.SetItemText(0,0,cstrIndex);
		m_lcEventInfo.SetItemText(0,1,cstrChannel);
		m_lcEventInfo.SetItemText(0,2, ConvertString(cstrDetail));
		m_lcEventInfo.SetItemText(0,3, ConvertString(cstrObject));
		m_lcEventInfo.SetItemText(0,4, cstrTime);


		char* itemData = new char[GUIDLEN];
		memset(itemData, 0, GUIDLEN - 1);
		strncpy(itemData, strGUID.c_str(), GUIDLEN -1);
		m_lcEventInfo.SetItemData(0, (DWORD_PTR)itemData);

		if (m_lcEventInfo.GetItemCount() > LISTSIZE)
		{
			char* pCh = (char*) m_lcEventInfo.GetItemData(LISTSIZE);
			delete[] pCh;

			m_lcEventInfo.DeleteItem(LISTSIZE);
		}
	}
}

int  CIntelligentDeviceDlg::GetAlarmInfoSize( DWORD AlarmType, DWORD *pAlarmSize)
{
	switch(AlarmType)
	{
	case EVENT_IVS_CROSSLINEDETECTION:
		{
			*pAlarmSize = sizeof(DEV_EVENT_CROSSLINE_INFO);
		}
		break;
	case EVENT_IVS_CROSSREGIONDETECTION:
		{
			*pAlarmSize = sizeof(DEV_EVENT_CROSSREGION_INFO);
		}
		break;

	default:
		return -1; // demo not support this event type 
	}

	return 0;
}


int  CIntelligentDeviceDlg::GetShowItemInfo(DWORD AlarmType,char* pAlarmInfo, std::string& strTime, int& channel, std::string& strDetail, std::string& strObject)
{
	char szTime[128] = {0};

	switch(AlarmType)
	{
	case EVENT_IVS_CROSSLINEDETECTION:
		{
			DEV_EVENT_CROSSLINE_INFO* pInfo = (DEV_EVENT_CROSSLINE_INFO*)pAlarmInfo;
			sprintf_s(szTime, 
				"%04d-%02d-%02d %02d:%02d:%02d.%3d",
				pInfo->UTC.dwYear,
				pInfo->UTC.dwMonth,
				pInfo->UTC.dwDay,
				pInfo->UTC.dwHour,
				pInfo->UTC.dwMinute,
				pInfo->UTC.dwSecond,
				pInfo->UTC.dwMillisecond);
			channel = pInfo->nChannelID + 1;
			strDetail = "Tripwire detection";
			strObject = pInfo->stuObject.szObjectType;
		}
		break;
	case EVENT_IVS_CROSSREGIONDETECTION:
		{
			DEV_EVENT_CROSSREGION_INFO* pInfo = (DEV_EVENT_CROSSREGION_INFO*)pAlarmInfo;
			sprintf_s(szTime, 
				"%04d-%02d-%02d %02d:%02d:%02d.%3d",
				pInfo->UTC.dwYear,
				pInfo->UTC.dwMonth,
				pInfo->UTC.dwDay,
				pInfo->UTC.dwHour,
				pInfo->UTC.dwMinute,
				pInfo->UTC.dwSecond,
				pInfo->UTC.dwMillisecond);
			channel = pInfo->nChannelID + 1;
			strDetail = "Intrusion detection";

			strObject = pInfo->stuObject.szObjectType;
		}
		break;
	default:
		return -1; // demo not support this event type 
	}

	strTime = szTime;


	return 0;

}

bool CIntelligentDeviceDlg::StoreEventInfoToList(int channel, int EventID, BYTE* EventData,DWORD dataSize,
												 std::string& strGUID,std::string& strTime, std::string& strDetail, std::string& strObject)
{
	int i = -1;

	EventType2Index(EventID, &i);
	if ( i == -1)
	{
		return false;
	}

	EventDataInfo eventInfo;
	eventInfo.nChannel = channel;
	eventInfo.EventID = EventID;
	eventInfo.strGUID = strGUID;
	eventInfo.strTime = strTime;
	eventInfo.strDetail = strDetail;
	eventInfo.index = m_index;
	eventInfo.strObjectType = strObject;

	eventInfo.Data = new BYTE[dataSize];
	memcpy(eventInfo.Data , EventData, dataSize);
	eventInfo.dataSize = dataSize;

	m_eventList[i].push_front(eventInfo);
	m_allEvents.push_front(&(*(m_eventList[i].begin())));

	if (m_eventList[i].size() > LISTSIZE)
	{
		EventList::reverse_iterator it= m_eventList[i].rbegin();

		m_allEvents.remove(&(*it));

		delete[] it->Data;
		m_eventList[i].pop_back();
	}
	return true;
}

void CIntelligentDeviceDlg::InitEventListCtrl()
{
	m_lcEventInfo.SetExtendedStyle(m_lcEventInfo.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_lcEventInfo.SetExtendedStyle(m_lcEventInfo.GetExtendedStyle()|LVS_EX_GRIDLINES); 

	CRect rect;
	int width ;
	m_lcEventInfo.GetClientRect(&rect);
	width = rect.Width();

	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;

	CString strToConvert = ConvertString(_T("Index"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 - 20;
	lvc.iSubItem = 0;
	m_lcEventInfo.InsertColumn(0, &lvc);

	strToConvert = ConvertString(_T("Channel"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 - 20;
	lvc.iSubItem = 0;
	m_lcEventInfo.InsertColumn(1, &lvc);

	strToConvert = ConvertString(_T("Event name"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 20;
	lvc.iSubItem = 0;
	m_lcEventInfo.InsertColumn(2, &lvc);

	strToConvert = ConvertString(_T("ObjectType"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 20;
	lvc.iSubItem = 0;
	m_lcEventInfo.InsertColumn(3, &lvc);

	strToConvert = ConvertString(_T("UTC"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 20;
	lvc.iSubItem = 0;
	m_lcEventInfo.InsertColumn(4, &lvc);
}


void CIntelligentDeviceDlg::InitEventCombox()
{
	CString tmp  = ConvertString(_T("All Events"));
	m_cbxEvent.AddString(tmp);
	m_cbxEvent.SetItemData(0, (DWORD_PTR)EVENT_IVS_ALL);

	tmp = ConvertString(_T("Tripwire"));
	m_cbxEvent.AddString(tmp);
	m_cbxEvent.SetItemData(1, (DWORD_PTR)EVENT_IVS_CROSSLINEDETECTION);

	tmp = ConvertString(_T("Intrusion"));
	m_cbxEvent.AddString(tmp);
	m_cbxEvent.SetItemData(2, (DWORD_PTR)EVENT_IVS_CROSSREGIONDETECTION);

	m_cbxEvent.SetCurSel(0);
}

void CIntelligentDeviceDlg::OnCbnSelchangeCbxEvent()
{
	USES_CONVERSION;
	// TODO: 在此添加控件通知处理程序代码
	if (m_loginID == 0)
	{
		return ;
	}


	DWORD eventType = m_cbxEvent.GetItemData(m_cbxEvent.GetCurSel());

	if (eventType == m_nFilterEventType)
	{
		return;
	}


	m_nFilterEventType = eventType;

	DeleteAllItems();
	int i = -1;

	EventType2Index(eventType,&i );
	if (i == -1)
	{
		return;
	}

	if (i < EVENT_NUM)
	{
		CString cstrChannel;
		CString cstrIndex;
		CString cstrDetail;
		CString cstrTime;
		CString cstrObjectType;

		EventList::iterator it = m_eventList[i].begin(); 

		for (int j = 0; it != m_eventList[i].end();++it,++j)
		{
			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = j;


			cstrChannel.Format(_T("%d"),it->nChannel);
			cstrIndex.Format(_T("%d"), it->index);
			cstrDetail.Format(_T("%s"), it->strDetail.c_str());
			cstrTime.Format(_T("%s"), it->strTime.c_str() );
			cstrObjectType.Format(_T("%s"), it->strObjectType.c_str() );

			m_lcEventInfo.InsertItem(&lvi);
			m_lcEventInfo.SetItemText(j,0,cstrIndex);
			m_lcEventInfo.SetItemText(j,1,cstrChannel);
			m_lcEventInfo.SetItemText(j,2, ConvertString(cstrDetail));
			m_lcEventInfo.SetItemText(j,3, ConvertString(cstrObjectType));
			m_lcEventInfo.SetItemText(j,4, cstrTime);

			char* itemData = new char[GUIDLEN];
			memset(itemData, 0, GUIDLEN);
			strncpy(itemData, it->strGUID.c_str(), GUIDLEN -1);
			m_lcEventInfo.SetItemData(j, (DWORD_PTR)(itemData));
		}
	}

	else if ( i == EVENT_NUM)
	{
		AllEventsInfo::iterator it = m_allEvents.begin();

		CString cstrChannel;
		CString cstrIndex;
		CString cstrDetail;
		CString cstrTime;
		CString cstrObjectType;

		for (int j = 0 ; it != m_allEvents.end() && j < LISTSIZE; ++it,++j)
		{
			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = j;

			cstrChannel.Format(_T("%d"),(*it)->nChannel);
			cstrIndex.Format(_T("%d"), (*it)->index);
			cstrDetail.Format(_T("%s"), (*it)->strDetail.c_str());
			cstrTime.Format(_T("%s"), (*it)->strTime.c_str());
			cstrObjectType.Format(_T("%s"), (*it)->strObjectType.c_str());

			m_lcEventInfo.InsertItem(&lvi);
			m_lcEventInfo.SetItemText(j,0,cstrIndex);
			m_lcEventInfo.SetItemText(j,1,cstrChannel);
			m_lcEventInfo.SetItemText(j,2, ConvertString(cstrDetail));
			m_lcEventInfo.SetItemText(j,3, ConvertString(cstrObjectType));
			m_lcEventInfo.SetItemText(j,4, cstrTime);

			char* itemData = new char[GUIDLEN];
			memset(itemData, 0, GUIDLEN-1);
			strncpy(itemData, (*it)->strGUID.c_str(), GUIDLEN-1);
			m_lcEventInfo.SetItemData(j, (DWORD_PTR)itemData);
		}
	}
}

void CIntelligentDeviceDlg::DeleteAllItems()
{
	int nItemCount = m_lcEventInfo.GetItemCount();
	for ( int i = nItemCount - 1; i >= 0; i-- )
	{
		char* pCh = (char*) m_lcEventInfo.GetItemData(i);
		delete[] pCh;
		pCh = NULL;
	}
	m_lcEventInfo.DeleteAllItems();
}

void CIntelligentDeviceDlg::DeleteAllStoreEventData()
{
	m_allEvents.clear();

	for (int i = 0 ; i < EVENT_NUM ; i++)
	{
		EventList::iterator it = m_eventList[i].begin();

		for (; it != m_eventList[i].end(); ++it)
		{
			delete[] it->Data;
		}

		m_eventList[i].clear();
	}

}

void CIntelligentDeviceDlg::EventType2Index(DWORD type, int* pIndex)
{
	switch (type)
	{
	case EVENT_IVS_ALL:
		{
			*pIndex = 2;
		}
		break;

	case EVENT_IVS_CROSSLINEDETECTION:
		{
			*pIndex = 0;
		}
		break;

	case EVENT_IVS_CROSSREGIONDETECTION:
		{
			*pIndex = 1;
		}
		break;

	default:
		break;
	}
}

void CIntelligentDeviceDlg::OnNMDblclkListEventinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	NM_LISTVIEW* pNMListView  = (NM_LISTVIEW*)pNMHDR;

	int nItem = pNMListView->iItem;

	if (nItem < 0 ||  nItem >= m_lcEventInfo.GetItemCount())
	{
		return ;
	}

	char* pGuid = (char*)m_lcEventInfo.GetItemData(nItem);

	if ( m_nFilterEventType == EVENT_IVS_ALL)
	{
		AllEventsInfo::iterator	 it = m_allEvents.begin();
		for (; it != m_allEvents.end(); ++it)
		{
			if (strncmp((*it)->strGUID.c_str(), pGuid, (*it)->strGUID.size()) == 0)
			{
				CRect rect;
				m_dlgShowPic.GetWindowRect(&rect);

				m_dlgShowPic.SetWindowPos(NULL, rect.top,rect.top,0,0,SWP_NOSIZE);
				m_dlgShowPic.ShowWindow(TRUE);
				m_dlgShowPic.ShowPicture((*it)->Data,(*it)->dataSize);

				break;
			}
		}
	}
	else 
	{
		int i = m_cbxEvent.GetCurSel() - 1;
		EventList::iterator it = m_eventList[i].begin();

		for (; it != m_eventList[i].end(); ++it)
		{
			if (strncmp(it->strGUID.c_str(), pGuid, it->strGUID.size()) == 0)
			{
				CRect rect;
				m_dlgShowPic.GetWindowRect(&rect);

				m_dlgShowPic.SetWindowPos(NULL, rect.top,rect.top,0,0,SWP_NOSIZE);
				m_dlgShowPic.ShowWindow(TRUE);
				m_dlgShowPic.ShowPicture(it->Data,it->dataSize);

				break;
			}
		}
	}
	*pResult = 0;
}
