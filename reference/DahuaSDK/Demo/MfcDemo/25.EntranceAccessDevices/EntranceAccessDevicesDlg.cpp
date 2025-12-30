// IntelligentTrafficDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "EntranceAccessDevicesDlg.h"
#include "BlackWhiltListDlg.h"
#include "LatticeScreenDlg.h"
#include "LatticeScreenDlg2.h"
#include "StrobeSetDlg.h"
#include <iostream>
#include <assert.h>
#include <math.h>
using namespace std;

//#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#endif

#ifdef WIN32
#pragma warning (disable:4786)
#endif

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

#define HIMETRIC_INCH           2540
#define MAX_EVENT_NUM			100

const COLORREF RESERVED_COLOR = RGB(255, 255, 255);

typedef enum UI_STATE
{
	UI_STATE_START_PREVIEW,
	UI_STATE_STOP_PREVIEW,
	UI_STATE_ATTACH_SNAP,
	UI_STATE_DETACH_SNAP,
	UI_STATE_ATTACH_EVENT,
	UI_STATE_DETACH_EVENT,
	UI_STATE_LOGIN,
	UI_STATE_LOGOUT
};

struct AlarmInfo
{
	long	lCommand;
	char*	pBuf;
	DWORD	dwBufLen;

	AlarmInfo()
	{
		lCommand = 0;
		pBuf = NULL;
		dwBufLen = 0;
	}

	~AlarmInfo()
	{
		if (NULL != pBuf)
		{
			delete []pBuf;
			pBuf = NULL;
		}
	}
};

int CALLBACK RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata)
{
	CEntranceAccessDevicesDlg* pThis = (CEntranceAccessDevicesDlg*)dwUser;
	if (NULL == pThis)
	{
		return -1;
	}
	
	Alarm* alarm = AlarmFactory::GetInstance().CreateAlarm(dwAlarmType);
	if (NULL != alarm)
	{
		alarm->CreateParam(lAnalyzerHandle, pAlarmInfo, pBuffer, dwBufSize, nSequence);
		HWND hwnd = pThis->GetSafeHwnd();
		if (!::PostMessage(hwnd, WM_PICTUREALARM_COME, (WPARAM)(alarm), 0))
		{
			if (NULL != alarm)
			{
				alarm->DestroyParam();
				delete alarm;
				alarm = NULL;
			}
		}
	}
	return 0;
}

BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(!dwUser) 
	{
		return FALSE;
	}
	BOOL bRet = FALSE;
	CEntranceAccessDevicesDlg* dlg = (CEntranceAccessDevicesDlg*)dwUser;
	if (dlg != NULL)
	{
		if (DH_ALARM_TALKING_INVITE == lCommand && dlg->m_bListenInvite)
		{
			dlg->pTalkDlg->PostMessage(WM_EVENTALARM_INVITE, (WPARAM)0, (LPARAM)0);
		}
		else if (DH_ALARM_TRAFFIC_VEHICLE_POSITION == lCommand && dlg->m_bListenPosition)
		{
			if (dlg != NULL && dlg->GetSafeHwnd())
			{
				AlarmInfo* pInfo = NEW AlarmInfo;
				if (!pInfo)
				{
					return FALSE;
				}
				pInfo->lCommand = lCommand;
				pInfo->pBuf = NEW char[dwBufLen];
				if (!pInfo->pBuf)
				{
					delete pInfo;
					pInfo = NULL;
					return FALSE;
				}
				memcpy(pInfo->pBuf, pBuf, dwBufLen);
				pInfo->dwBufLen = dwBufLen;
				dlg->PostMessage(WM_EVENTALARM_POSITION_INFO, (WPARAM)pInfo, (LPARAM)0);
			}
		}
	}

	return bRet;	
}

void CALLBACK DevReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	CEntranceAccessDevicesDlg *pThis = (CEntranceAccessDevicesDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);
	return;
}

void CALLBACK DevDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	CEntranceAccessDevicesDlg *pThis = (CEntranceAccessDevicesDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);	
	return;
}

void CALLBACK DevSubDisConnect(EM_INTERFACE_TYPE emInterfaceType, BOOL bOnline, LLONG lOperateHandle, LLONG lLoginID, LDWORD dwUser)
{
	return;
}

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
// CEntranceAccessDevicesDlg dialog

CEntranceAccessDevicesDlg::CEntranceAccessDevicesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEntranceAccessDevicesDlg::IDD, pParent)
{	
	//{{AFX_DATA_INIT(CEntranceAccessDevicesDlg)
	m_strUserName = "";
	m_nPort = 0;
	m_strPasswd = "";
	m_nPictureAlarmCount = 0;	
	m_nEventAlarmCount = 0;
	m_bListenInvite = false;
	m_bListenPosition = false;
	m_Font = NULL;
	pTalkDlg = NULL;
	m_bSubIntelligent = FALSE;;
	m_bOrdinary = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
}

CEntranceAccessDevicesDlg::~CEntranceAccessDevicesDlg()
{		
	if (NULL != m_Font)
	{
		m_Font->DeleteObject();
		delete m_Font;
		m_Font = NULL;
	}
	ClearPictureAlarmRecordData();
}
void CEntranceAccessDevicesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEntranceAccessDevicesDlg)
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_ctrlIP);
	DDX_Control(pDX, IDC_LISTCTRL_REAL_EVENT, m_listPictureAlarm);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_cbChannel);
	DDX_Control(pDX, IDC_STATIC_EVENT_LP_IMAGE, m_ctrlPlateImage);
	//}}AFX_DATA_MAP	
	DDX_Control(pDX, IDC_LISTCTRL_EVENT, m_listEventAlarm);
	DDX_Control(pDX, IDC_CHECK_OPEN_TYPE1, m_check_open_type1);
	DDX_Control(pDX, IDC_CHECK_OPEN_TYPE2, m_check_open_type2);
}

BEGIN_MESSAGE_MAP(CEntranceAccessDevicesDlg, CDialog)
	//{{AFX_MSG_MAP(CEntranceAccessDevicesDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, OnButtonLogout)
	ON_BN_CLICKED(IDC_BUTTON_START_PREVIEW, OnButtonStartPreview)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PREVIEW, OnButtonStopPreview)
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStartSubscribePicture)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStopSubscribePicture)
	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_REAL_EVENT, OnNMClickListctrlRealEvent)
	ON_WM_DESTROY()	

	ON_NOTIFY(LVN_KEYDOWN, IDC_LISTCTRL_REAL_EVENT, OnKeydownListctrlRealEvent)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_PICTUREALARM_COME, OnPictureAlarmCome)
	ON_MESSAGE(WM_EVENTALARM_POSITION_INFO, OnEventAlarmInfo)
	ON_MESSAGE(WM_LISTENINVITE, ListenInvite)
	ON_WM_TIMER()		
	ON_WM_CLOSE()
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_SNAP, &CEntranceAccessDevicesDlg::OnBnClickedButtonManualSnap)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_STROBE, &CEntranceAccessDevicesDlg::OnBnClickedButtonOpenStrobe)
	ON_COMMAND(ID_STROBESET, &CEntranceAccessDevicesDlg::OnStrobeset)
	ON_COMMAND(ID_BLACKWHILTLIST, &CEntranceAccessDevicesDlg::OnBlackwhiltlist)
	ON_COMMAND(ID_TALK, &CEntranceAccessDevicesDlg::OnTalk)
	ON_COMMAND(ID_LATTICESCREEN, &CEntranceAccessDevicesDlg::OnLatticescreen)
	ON_BN_CLICKED(IDC_BUTTON_START2, &CEntranceAccessDevicesDlg::OnButtonStartSubscribeEvent)
	ON_BN_CLICKED(IDC_BUTTON_STOP2, &CEntranceAccessDevicesDlg::OnButtonStopSubscribeEvent)
	ON_BN_CLICKED(IDC_CHECK_OPEN_TYPE1, &CEntranceAccessDevicesDlg::OnBnClickedCheckOpenType1)
	ON_BN_CLICKED(IDC_CHECK_OPEN_TYPE2, &CEntranceAccessDevicesDlg::OnBnClickedCheckOpenType2)
	ON_COMMAND(ID_LATTICESCREEN2, &CEntranceAccessDevicesDlg::OnLatticescreen2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEntranceAccessDevicesDlg message handlers

BOOL CEntranceAccessDevicesDlg::OnInitDialog()
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
	g_SetWndStaticText(this);

	InitMenu();
	InitPicturePath();
	InitCtrls();
	InitNetSdk();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEntranceAccessDevicesDlg::InitMenu()
{
	m_Menu.LoadMenu(IDR_MENU1);
	m_Menu.ModifyMenu(0,MF_BYPOSITION,ID_STROBESET,ConvertString("StrobeSet"));
	m_Menu.ModifyMenu(1,MF_BYPOSITION,ID_BLACKWHILTLIST,ConvertString("BlackWhiltList"));
	m_Menu.ModifyMenu(2,MF_BYPOSITION,ID_TALK,ConvertString("Talk"));
	m_Menu.ModifyMenu(3,MF_BYPOSITION,ID_LATTICESCREEN,ConvertString("LatticeScreen"));
	m_Menu.ModifyMenu(4,MF_BYPOSITION,ID_LATTICESCREEN2,ConvertString("LatticeScreen2"));
	SetMenu(&m_Menu);
}

void CEntranceAccessDevicesDlg::InitPicturePath()
{
	string strCfgFilePath;
	string strCfgDefaultFolder;
	GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append(CFG_CFG_FILE_NAME);
	strCfgDefaultFolder.append(PC_DATA_STORE_FOLDER);		
	::WritePrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), strCfgFilePath.c_str());
	char szFolder[1024] = {0};
	::GetPrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), szFolder, 1024, strCfgFilePath.c_str());
	::CreateDirectory(szFolder, NULL);
}

void CEntranceAccessDevicesDlg::InitCtrls()
{
	SetDlgItemText(IDC_IPADDRESS_IP, "10.18.128.122");//10.18.128.122  172.24.31.71
	SetDlgItemInt(IDC_EDIT_PORT, 37777);
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWORD, "admin123");

	m_tt.Create(this);
	m_tt.Activate(TRUE);
	m_tt.AddTool(&m_check_open_type1,ConvertString("Need to subscribe to ① traffic junction event (receive picture and license plate information)"));
	m_tt.AddTool(&m_check_open_type2,ConvertString("Need to subscribe to ① vehicle location event (first receive license plate information for opening business) ② traffic junction event (for receiving supplementary pictures)"));

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

	InitPictureAlarmCtrl();
	InitEventAlarmCtrl();

	m_cPictureWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1981);
	CRect screenRect;
	GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(&screenRect);
	ScreenToClient(&screenRect);
	m_cPictureWnd.MoveWindow(screenRect);
	m_cPictureWnd.ShowWindow(SW_SHOW);

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE));
}

void CEntranceAccessDevicesDlg::InitNetSdk()
{
	Device::GetInstance().SetReconnect(DevDisConnect,DevReConnect,DevSubDisConnect,(LDWORD)this);
}

void CEntranceAccessDevicesDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CEntranceAccessDevicesDlg::OnPaint() 
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
HCURSOR CEntranceAccessDevicesDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEntranceAccessDevicesDlg::OnButtonLogin() 
{
	char szIp[32] = {0};
	m_ctrlIP.GetWindowText(szIp, sizeof(szIp));
	m_nPort = GetDlgItemInt(IDC_EDIT_PORT);
	GetDlgItemText(IDC_EDIT_USERNAME,m_strUserName);
	GetDlgItemText(IDC_EDIT_PASSWORD,m_strPasswd);

	BOOL bret = Device::GetInstance().Login(szIp, (WORD)m_nPort, m_strUserName.GetBuffer(0), m_strPasswd.GetBuffer(0));
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
	else
	{
		this->ChangeUI(UI_STATE_LOGIN);
		SetWindowText(ConvertString("EntranceAccessDevices"));
	}
}

void CEntranceAccessDevicesDlg::OnButtonLogout() 
{
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"),ConvertString("Prompt"));
		return;
	}
	
	OnButtonStopPreview();

	OnButtonStopSubscribePicture();	
	ClearPictureAlarmUIInfo();

	m_bListenInvite = false;
	m_bListenPosition = false;
	StopListenEventAlarm();
	ClearEventAlarmUIInfo();

	Device::GetInstance().Logout();
	this->ChangeUI(UI_STATE_LOGOUT);
	SetWindowText(ConvertString("EntranceAccessDevices"));
}

void CEntranceAccessDevicesDlg::OnClose()
{	
	CDialog::OnClose();
}

BOOL CEntranceAccessDevicesDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (NULL != m_tt.GetSafeHwnd())
	{
		m_tt.RelayEvent(pMsg);
	}
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

void CEntranceAccessDevicesDlg::OnButtonStartPreview() 
{
	HWND hWnd = GetDlgItem(IDC_STATIC_PREVIEW)->GetSafeHwnd();
	int nSel = m_cbChannel.GetCurSel();
	if ( (nSel < 0) || (!Device::GetInstance().GetLoginState()))
	{
		MessageBox(ConvertString("Please login and select a channel before preview!"),ConvertString("Prompt"));
		return;
	}
	//Open the preview
	int nChannelID = nSel;

	Device::GetInstance().StopRealPlay();
	if (!Device::GetInstance().StartRealPlay(nChannelID, hWnd))
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
		return;
	}
	this->ChangeUI(UI_STATE_START_PREVIEW);	
}

void CEntranceAccessDevicesDlg::OnButtonStopPreview() 
{
	if (!Device::GetInstance().StopRealPlay())
	{
		return;
	}
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PREVIEW));
	this->ChangeUI(UI_STATE_STOP_PREVIEW);	
}

LRESULT CEntranceAccessDevicesDlg::OnPictureAlarmCome(WPARAM wParam, LPARAM lParam)
{
	Alarm* alarm = (Alarm*)wParam;
	if (NULL != alarm)
	{
		OnReceivePictureAlarm(alarm);
		alarm->DestroyParam();
	}
	return 0;
}

void CEntranceAccessDevicesDlg::OnReceivePictureAlarm(Alarm* alarm)
{
	if (NULL == alarm)
	{
		return;
	}
	EventParam* pEventParam = alarm->pAlarmData->GetEventParam()/*eventData*/;
	std::string strGUID = alarm->GetGUID();
	std::string strCfgFolder = GetDataFolder();
	std::string strJpgFile = strCfgFolder + strGUID + std::string(".jpg");	
	::CreateDirectory(strCfgFolder.c_str(), NULL);	
	
	//存储照片
	if (NULL != pEventParam)
	{
		StorePicture(strJpgFile.c_str(), pEventParam);
	}
	
	AlarmFactory::GetInstance().AddAlarm(alarm);

	m_nPictureAlarmCount++;
	
	StuAlarmItem* pAlarmItem = GetAlarmItemByGUID(strGUID.c_str());
	if (pAlarmItem == NULL)
	{
		return;
	}
	
	UpdatePictureAlarmListCtrlInfo(strGUID.c_str(), pAlarmItem);	

	// Update License Plate Text	
	UpdateLicensePlateNumber(pAlarmItem->dwAlarmType, pAlarmItem->pAlarmInfo);	

	// Display event image and license plate image
	DisplayImage(strGUID.c_str(), strJpgFile.c_str(), pAlarmItem);	
	
	int nOpenType2 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->GetCheck();
	int nOpenType1 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE1))->GetCheck();
	if (nOpenType2 != 1 && nOpenType1 == 1 && pAlarmItem->dwAlarmType == EVENT_IVS_TRAFFICJUNCTION)
	{
		BOOL bret = OpenStrobe();
	}
}

BOOL CEntranceAccessDevicesDlg::UpdatePictureAlarmListCtrlInfo(const char* szGUID, StuAlarmItem* pEventItem, bool bFilter/* = false*/, int nIndex/* = 0*/)
{		
	int nItemCount = m_listPictureAlarm.GetItemCount();
	if (nItemCount >= MAX_EVENT_NUM)
	{
		StuListItemData* pItemData = (StuListItemData*)m_listPictureAlarm.GetItemData(MAX_EVENT_NUM - 1);
		if (NULL != pItemData)
		{
			char* szGUID = pItemData->szGUID;
			BYTE* szBuf  = pItemData->pBuffer;
			if (NULL != szGUID)
			{
				delete[] szGUID;
			}
			if (NULL != szBuf)
			{
				delete[] szBuf;
			}
			if (NULL != pItemData)
			{
				delete pItemData;
				pItemData = NULL;
			}
			m_listPictureAlarm.DeleteItem(MAX_EVENT_NUM - 1);
		}
	}	
	
	Alarm* alarm = AlarmFactory::GetInstance().GetAlarm(szGUID);
	if (alarm == NULL)
	{
		return FALSE;
	}
	StuEventInfo stuEventInfo = alarm->pAlarmData->GetEventInfo()/*stuEventInfo*/;

	char* szGUIDTmp = NEW char[64];
	if (NULL == szGUIDTmp)
	{
		return FALSE;
	}

	BYTE* szBuf = NEW BYTE[pEventItem->dwBufSize];
	if (NULL == szBuf)
	{
		delete[] szGUIDTmp;
		return FALSE;
	}

	StuListItemData* pItemData = NEW StuListItemData;
	if (NULL == pItemData)
	{
		delete[] szGUIDTmp;
		delete[] szBuf;
		return FALSE;
	}

	memset(szGUIDTmp, 0, 64);
	strncpy(szGUIDTmp, szGUID, 63);
	memcpy(szBuf, pEventItem->pBuffer, pEventItem->dwBufSize);

	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = "";
	lvi.iImage = 0;
	lvi.iItem = 0;
	m_listPictureAlarm.InsertItem(&lvi);	

	pItemData->szGUID		= szGUIDTmp;
	pItemData->pBuffer		= szBuf;
	pItemData->dwBufSize	= pEventItem->dwBufSize;	
	m_listPictureAlarm.SetItemData(0, (LDWORD)(pItemData));

	//Display data			
	stuEventInfo.strCountNum = Int2Str(m_nPictureAlarmCount);//Int2Str(bFilter ? nIndex : CalculateEventNum(GetFilterEventType()));	// update the count number of event list ctrl
	stuEventInfo.strDeviceAddress = pEventItem->strDeviceAddress;

	m_listPictureAlarm.SetItemText(0, 0, stuEventInfo.strCountNum.c_str());
	m_listPictureAlarm.SetItemText(0, 1, stuEventInfo.strTime.c_str());
	m_listPictureAlarm.SetItemText(0, 2, stuEventInfo.strEventType.c_str());
	m_listPictureAlarm.SetItemText(0, 3, stuEventInfo.strPlateNumber.c_str());
	m_listPictureAlarm.SetItemText(0, 4, stuEventInfo.strPlateColor.c_str());
	m_listPictureAlarm.SetItemText(0, 5, stuEventInfo.strPlateType.c_str());
	m_listPictureAlarm.SetItemText(0, 6, stuEventInfo.strVehicleType.c_str());
	m_listPictureAlarm.SetItemText(0, 7, stuEventInfo.strVehicleColor.c_str());
	m_listPictureAlarm.SetItemText(0, 8, stuEventInfo.strVehicleSize.c_str());
	m_listPictureAlarm.SetItemText(0, 9, stuEventInfo.strFileIndex.c_str());
	m_listPictureAlarm.SetItemText(0, 10, stuEventInfo.strFileCount.c_str());
	m_listPictureAlarm.SetItemText(0, 11, stuEventInfo.strGroupId.c_str());
	m_listPictureAlarm.SetItemText(0, 12, stuEventInfo.strLane.c_str());
	m_listPictureAlarm.SetItemText(0, 13, stuEventInfo.strDeviceAddress.c_str());	

	return TRUE;
}

BOOL CEntranceAccessDevicesDlg::DisplayImage(const char* szGUID , const char* szJpgFilePath, StuAlarmItem* pEventItem)
{
	DisplayEventImage(szJpgFilePath);
	DisplayLicensePlateImage(pEventItem->pBuffer, pEventItem->dwBufSize);
	return TRUE;
}

BOOL CEntranceAccessDevicesDlg::DisplayLicensePlateImage(BYTE* pBuffer, size_t nSize)
{
	if (NULL == pBuffer || 0 == nSize)
	{
		FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE));
		return FALSE;
	}
	return m_ctrlPlateImage.LoadFromStream(pBuffer, nSize);
}

BOOL CEntranceAccessDevicesDlg::DisplayEventImage(const char* szJPGPath)
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

void CEntranceAccessDevicesDlg::ClearPictureAlarmRecordData()
{
	AlarmFactory::GetInstance().ClearAlarm();
}
BOOL CEntranceAccessDevicesDlg::StorePicture(const char* szJpgFilePath, EventParam* pEventParam)
{
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

void CEntranceAccessDevicesDlg::OnButtonStartSubscribePicture() 
{	
	int nSel = m_cbChannel.GetCurSel();
	if (nSel < 0)
	{
		MessageBox(ConvertString("Please select a channel"),ConvertString("Prompt"));
		return;
	}
	m_nPictureAlarmCount = 0;
	m_bSubIntelligent = Device::GetInstance().StartRealLoadPicture(nSel, EVENT_IVS_ALL, TRUE, RealLoadPicCallback, (LDWORD)this);
	if (!m_bSubIntelligent)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
		return;
	}
	this->ChangeUI(UI_STATE_ATTACH_SNAP);
}

void CEntranceAccessDevicesDlg::OnButtonStopSubscribePicture() 
{
	if (!Device::GetInstance().StopRealLoadPicture())
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
		return;
	}
	m_bSubIntelligent = FALSE;
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PICTURE));
	m_ctrlPlateImage.FreeData();
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_EVENT_LP_IMAGE));
	GetDlgItem(IDC_STATIC_LP)->SetWindowText("");
	int nOpenType1 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE1))->GetCheck();
	if (nOpenType1 == 1)
	{
		((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE1))->SetCheck(0);
	}
	int nOpenType2 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->GetCheck();
	if (nOpenType2 == 1)
	{
		((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->SetCheck(0);
	}
	this->ChangeUI(UI_STATE_DETACH_SNAP);
}

void CEntranceAccessDevicesDlg::OnNMClickListctrlRealEvent(NMHDR *pNMHDR, LRESULT *pResult)
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	if (phdn->iItem == -1)
	{
		return;
	}
	int NEWIndex = phdn->iItem;
	DisplaySelectedPicInfo(NEWIndex, &m_listPictureAlarm);

	*pResult = 0;
}

void CEntranceAccessDevicesDlg::OnDestroy()
{	
	CDialog::OnDestroy();
	ClearPictureAlarmInEventCtrl();
	Device::GetInstance().StopRealPlay();
	Device::GetInstance().Destroy();
}

void CEntranceAccessDevicesDlg::ChangeUI( int nState )
{
	switch (nState)
	{
	case UI_STATE_START_PREVIEW:
		{
			GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(TRUE);	
		}
		break;
	case UI_STATE_STOP_PREVIEW:
		{
			GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);
		}
		break;
	case UI_STATE_ATTACH_SNAP:
		{				
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
		}
		break;
	case UI_STATE_DETACH_SNAP:
		{					
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		}
		break;
	case UI_STATE_ATTACH_EVENT:
		{				
			GetDlgItem(IDC_BUTTON_START2)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(TRUE);
		}
		break;
	case UI_STATE_DETACH_EVENT:
		{					
			GetDlgItem(IDC_BUTTON_START2)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);
		}
		break;
	case UI_STATE_LOGIN:
		{
			GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_START2)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_OPEN_STROBE)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_MANUAL_SNAP)->EnableWindow(TRUE);

			int i= 0;
			for (i = 0; i < Device::GetInstance().GetChanNum(); i++)
			{
				char szContent[8] = {0};
				_snprintf(szContent, sizeof(szContent), "%d", i + 1);
				m_cbChannel.AddString(szContent);
				m_cbChannel.SetItemData(i, (DWORD)i);
			}
			m_cbChannel.SetCurSel(0);	

		}
		break;
	case UI_STATE_LOGOUT:
		{
			GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);		
			GetDlgItem(IDC_BUTTON_START2)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_OPEN_STROBE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_MANUAL_SNAP)->EnableWindow(FALSE);

			while (m_cbChannel.GetCount() > 0)
			{
				m_cbChannel.DeleteString(0);
			}
			m_cbChannel.Clear();				
		}
		break;
	default:
		break;
	}
}

void CEntranceAccessDevicesDlg::ClearPictureAlarmUIInfo() 
{
	ClearPictureAlarmInEventCtrl();	
	ClearPictureAlarmRecordData();
	m_nPictureAlarmCount = 0;
}

void CEntranceAccessDevicesDlg::ClearEventAlarmUIInfo() 
{
	m_listEventAlarm.DeleteAllItems();
	m_nEventAlarmCount = 0;
}

void CEntranceAccessDevicesDlg::OnKeydownListctrlRealEvent(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	int nCurIndex = (int)m_listPictureAlarm.GetFirstSelectedItemPosition() - 1;

	if (pLVKeyDow->wVKey == 38 || pLVKeyDow->wVKey == 40)
	{
		int NEWIndex = 0;
		if (pLVKeyDow->wVKey == 38)//up
		{
			NEWIndex = (nCurIndex == 0)? nCurIndex : nCurIndex -1;
		}
		else//down
		{
			NEWIndex = (nCurIndex == m_listPictureAlarm.GetItemCount()-1) ? nCurIndex : nCurIndex + 1;
		}
		DisplaySelectedPicInfo(NEWIndex, &m_listPictureAlarm);
	}
	*pResult = 0;
}


void CEntranceAccessDevicesDlg::DisplaySelectedPicInfo(int nItemIndex, CListCtrl* listCtrl)
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
	StuAlarmItem* pEventItem = GetAlarmItemByGUID(pItemData->szGUID);
	if (pEventItem == NULL)
	{
		return;
	}	
	
	UpdateLicensePlateNumber(pEventItem->dwAlarmType, pEventItem->pAlarmInfo);
	DisplayImage(pItemData->szGUID, strJpgFilePath.c_str(), pEventItem);
}


HBRUSH CEntranceAccessDevicesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);	
	
	return hbr;
}

StuAlarmItem* CEntranceAccessDevicesDlg::GetAlarmItemByGUID(const char* szGUID)
{
	std::string strGUID(szGUID);

	Alarm* alarm = AlarmFactory::GetInstance().GetAlarm(strGUID);
	if (alarm != NULL)
	{
		return alarm->pAlarmData->GetAlarmItem()/*eventItem*/;
	}
	else
	{
		return NULL;
	}

}

void CEntranceAccessDevicesDlg::InitPictureAlarmCtrl(void)
{
	m_listPictureAlarm.SetExtendedStyle(m_listPictureAlarm.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listPictureAlarm.SetExtendedStyle(m_listPictureAlarm.GetExtendedStyle()|LVS_EX_GRIDLINES); 	
	
	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("Index", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("Type", 80));

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
		m_listPictureAlarm.InsertColumn(j, &lvc);
	}
}

void CEntranceAccessDevicesDlg::InitEventAlarmCtrl(void)
{
	m_listEventAlarm.SetExtendedStyle(m_listEventAlarm.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listEventAlarm.SetExtendedStyle(m_listEventAlarm.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("Index", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("Type", 100));
	vecTitles.push_back(ColDesObj("Plate Text", 100));
	vecTitles.push_back(ColDesObj("Position", 60));
	vecTitles.push_back(ColDesObj("OpenStrobeState", 60));

	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_listEventAlarm.InsertColumn(j, &lvc);
	}
}

void CEntranceAccessDevicesDlg::ClearPictureAlarmInEventCtrl()
{
	int nItemCount = m_listPictureAlarm.GetItemCount();
	for (int i = 0 ; i < nItemCount; i++)
	{
		StuListItemData* pItemData = (StuListItemData*)m_listPictureAlarm.GetItemData(i);
		if (NULL != pItemData)
		{
			delete[] pItemData->pBuffer;
			delete[] pItemData->szGUID;
			delete pItemData;
			pItemData = NULL;
		}
	}
	m_listPictureAlarm.DeleteAllItems();
}

BOOL CEntranceAccessDevicesDlg::UpdateLicensePlateNumber(const DWORD dwEventType, const void* pEventInfo)
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
	case EVENT_IVS_TRAFFIC_MANUALSNAP:
		{
			DEV_EVENT_TRAFFIC_MANUALSNAP_INFO* pInfo = (DEV_EVENT_TRAFFIC_MANUALSNAP_INFO*)pEventInfo;
			strLicensePlate = MakeLP(pInfo->stTrafficCar.szPlateNumber);	
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

void CEntranceAccessDevicesDlg::FillCWndWithDefaultColor(CWnd* cwnd) 
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

void CEntranceAccessDevicesDlg::OnBnClickedButtonManualSnap()
{
	int nSelChan = m_cbChannel.GetCurSel();
	if ( (nSelChan < 0) )
	{
		MessageBox(ConvertString("Please login and select a channel before manual snap!"),ConvertString("Prompt"));
		return;
	}

	if (!m_bSubIntelligent)
	{
		OnButtonStartSubscribePicture();
	}

	BOOL bret = Device::GetInstance().ManualSnap(DH_MANUAL_SNAP,nSelChan);
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
}

void CEntranceAccessDevicesDlg::OnBnClickedButtonOpenStrobe()
{
	int nSelChan = m_cbChannel.GetCurSel();
	if (nSelChan < 0)
	{
		MessageBox(ConvertString("Please select a channel!"),ConvertString("Prompt"));
		return;
	}

	BOOL bret = Device::GetInstance().StrobeControl(DH_CTRL_OPEN_STROBE,nSelChan);
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
}

BOOL CEntranceAccessDevicesDlg::OpenStrobe()
{

	int nSel = m_cbChannel.GetCurSel();
	if (nSel < 0)
	{
		return FALSE;
	}

	return Device::GetInstance().StrobeControl(DH_CTRL_OPEN_STROBE,nSel);
}

void CEntranceAccessDevicesDlg::OnStrobeset()
{
	StrobeSetDlg dlg(this);
	dlg.DoModal();
}

void CEntranceAccessDevicesDlg::OnBlackwhiltlist()
{
	BlackWhiltListDlg dlg(this);
	dlg.DoModal();
}

void CEntranceAccessDevicesDlg::OnTalk()
{
	pTalkDlg = new TalkDlg(this);
	if (NULL != pTalkDlg)
	{
		pTalkDlg->DoModal();
		delete pTalkDlg;
		pTalkDlg = NULL;
	}
}

void CEntranceAccessDevicesDlg::OnLatticescreen()
{
	LatticeScreenDlg dlg(this);
	dlg.DoModal();
}

BOOL CEntranceAccessDevicesDlg::StartListenEventAlarm()
{
	if (!m_bListenPosition && !m_bListenInvite)
	{
		BOOL bret = Device::GetInstance().StartListenEvent(MessCallBack,(LDWORD)this);
		if (!bret)
		{
			MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CEntranceAccessDevicesDlg::StopListenEventAlarm()
{
	if (!m_bListenPosition && !m_bListenInvite)
	{
		if (!Device::GetInstance().StopListenEvent())
		{
			return FALSE;
		}
		this->ChangeUI(UI_STATE_DETACH_EVENT);
	}
	return TRUE;
}

void CEntranceAccessDevicesDlg::OnButtonStartSubscribeEvent()
{
	m_nEventAlarmCount = 0;
	m_bOrdinary = StartListenEventAlarm();	
	if (m_bOrdinary)
	{
		m_bListenPosition = true;
		this->ChangeUI(UI_STATE_ATTACH_EVENT);
	}
}

void CEntranceAccessDevicesDlg::OnButtonStopSubscribeEvent()
{
	m_bListenPosition = false;
	BOOL bret = StopListenEventAlarm();
	if (bret)
	{
		m_bOrdinary = FALSE;
		int nOpenType2 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->GetCheck();
		if (nOpenType2 == 1)
		{
			((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->SetCheck(0);
		}
		this->ChangeUI(UI_STATE_DETACH_EVENT);
	}
}

LRESULT CEntranceAccessDevicesDlg::ListenInvite(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)//start
	{
		BOOL bret = StartListenEventAlarm();	
		if (bret)
		{
			m_bListenInvite = true;
			this->ChangeUI(UI_STATE_ATTACH_EVENT);
		}
	}
	else//stop
	{
		m_bListenInvite = false;
		BOOL bret = StopListenEventAlarm();
	}
	return 0;
}

LRESULT CEntranceAccessDevicesDlg::OnEventAlarmInfo(WPARAM wParam, LPARAM lParam)
{
	AlarmInfo* pAlarmInfo = (AlarmInfo*)wParam;
	if (!pAlarmInfo || !pAlarmInfo->pBuf || pAlarmInfo->dwBufLen <= 0)
	{
		if (NULL != pAlarmInfo)
		{
			delete pAlarmInfo;
			pAlarmInfo = NULL;
		}
		return -1;
	}
	m_listEventAlarm.SetRedraw(FALSE);
	SYSTEMTIME st;
	GetLocalTime(&st);

	if (pAlarmInfo->lCommand == DH_ALARM_TRAFFIC_VEHICLE_POSITION)
	{
		ALARM_TRAFFIC_VEHICLE_POSITION *AlarmPositionInfo = (ALARM_TRAFFIC_VEHICLE_POSITION*)pAlarmInfo->pBuf;
		if (m_listEventAlarm.GetItemCount() > MAX_MSG_SHOW)
		{
			m_listEventAlarm.DeleteItem(MAX_MSG_SHOW);
		}

		CString strIndex;
		strIndex.Format("%d",m_nEventAlarmCount + 1);
		LV_ITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.pszText = _T("");
		lvi.iImage = 0;
		lvi.iItem = 0;
		m_listEventAlarm.InsertItem(&lvi);
		m_listEventAlarm.SetItemText(0, 0, strIndex);
		CString str;
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		
		m_listEventAlarm.SetItemText(0, 3, AlarmPositionInfo->szPlateNumber);
		str.Format("%d",AlarmPositionInfo->nPosition);
		m_listEventAlarm.SetItemText(0, 4, str);

		if (AlarmPositionInfo->byOpenStrobeState == NET_OPEN_STROBE_STATE_UNKOWN)
		{
			m_listEventAlarm.SetItemText(0, 5, "unkown");
		} 
		else if(AlarmPositionInfo->byOpenStrobeState == NET_OPEN_STROBE_STATE_CLOSE)
		{
			m_listEventAlarm.SetItemText(0, 5, "close");
		}
		else if(AlarmPositionInfo->byOpenStrobeState == NET_OPEN_STROBE_STATE_AUTO)
		{
			m_listEventAlarm.SetItemText(0, 5, "auto");
		}
		else if(AlarmPositionInfo->byOpenStrobeState == NET_OPEN_STROBE_STATE_MANUAL)
		{
			m_listEventAlarm.SetItemText(0, 5, "manual");
		}
		
		m_nEventAlarmCount++;

		int nOpen = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->GetCheck();
		if (nOpen == 1)
		{
			BOOL bret = OpenStrobe();
		}
	}
	
	m_listEventAlarm.SetRedraw(TRUE);
	if (NULL != pAlarmInfo)
	{
		delete pAlarmInfo;
		pAlarmInfo = NULL;
	}
	return 0;
}

void CEntranceAccessDevicesDlg::OnBnClickedCheckOpenType1()
{
	int nOpenType1 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE1))->GetCheck();
	if (nOpenType1 == 1)
	{
		if (!m_bSubIntelligent)
		{
			MessageBox(ConvertString("Please subscribe to traffic junction event first!"),ConvertString("Prompt"));
			((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE1))->SetCheck(0);
		}
		
		int nOpenType2 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->GetCheck();
		if (nOpenType2 == 1)
		{
			MessageBox(ConvertString("When two opening modes of vehicle position event opening and traffic junction event opening are opened at the same time, the opening is based on the vehicle position event."),ConvertString("Prompt"));
		}
	}
}

void CEntranceAccessDevicesDlg::OnBnClickedCheckOpenType2()
{
	int nOpenType2 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->GetCheck();
	if (nOpenType2 == 1)
	{
		if (!m_bSubIntelligent || !m_bOrdinary)
		{
			MessageBox(ConvertString("Please subscribe to vehicle position event and traffic junction event first!"),ConvertString("Prompt"));
			((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE2))->SetCheck(0);
		}
		int nOpenType1 = ((CButton*)GetDlgItem(IDC_CHECK_OPEN_TYPE1))->GetCheck();
		if (nOpenType1 == 1)
		{
			MessageBox(ConvertString("When two opening modes of vehicle position event opening and traffic junction event opening are opened at the same time, the opening is based on the vehicle position event."),ConvertString("Prompt"));
		}

	}
}

LRESULT CEntranceAccessDevicesDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

LRESULT CEntranceAccessDevicesDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	SetWindowText(ConvertString("EntranceAccessDevices"));
	return 0;
}
void CEntranceAccessDevicesDlg::OnLatticescreen2()
{
	// TODO: 在此添加命令处理程序代码
	LatticeScreenDlg2  Dlg; 
	Dlg.DoModal();

}
