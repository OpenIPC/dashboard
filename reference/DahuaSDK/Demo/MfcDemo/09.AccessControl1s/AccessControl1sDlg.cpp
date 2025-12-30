// IntelligentTrafficDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "AccessControl1sDlg.h"
#include "DevInfo.h"
#include "DevTime.h"
#include "NetWork.h"
#include "ModifyPassword.h"
#include "DevUpgrade.h"
#include "AutoMatrix.h"
#include "TimeSchedule.h"
#include "DoorConfig.h"
#include "OpenDoorGroup.h"
#include "FirstEnter.h"
#include "ABLock.h"
#include "OpenDoorRoute.h"
#include "AccessCtlPWD.h"
#include "DoorRecord.h"
#include "AlarmRecord.h"
#include "Log.h"

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

BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(!dwUser) 
	{
		return FALSE;
	}
	BOOL bRet = FALSE;
	CAccessControlDlg* dlg = (CAccessControlDlg*)dwUser;
	if (dlg != NULL)
	{
		if (DH_ALARM_FINGER_PRINT == lCommand)
		{
			ALARM_CAPTURE_FINGER_PRINT_INFO *pstuFingerInfo = (ALARM_CAPTURE_FINGER_PRINT_INFO *)pBuf;
			if (pstuFingerInfo->nPacketLen <= 0 || pstuFingerInfo->nPacketNum <= 0 || 
				pstuFingerInfo->szFingerPrintInfo == NULL)
			{
				if (NULL != dlg->pCardDlg)
				{
					dlg->pCardDlg->PostMessage(WM_ACCESS_FINGERPRINT, NULL, NULL);
				}
				return FALSE;
			}

			ALARM_CAPTURE_FINGER_PRINT_INFO *pstuFingerMsg = new ALARM_CAPTURE_FINGER_PRINT_INFO;
			if (NULL == pstuFingerMsg)
			{
				return FALSE;
			}
			memcpy(pstuFingerMsg, pstuFingerInfo, sizeof(*pstuFingerMsg));
			pstuFingerMsg->szFingerPrintInfo  = new char[pstuFingerInfo->nPacketLen * pstuFingerInfo->nPacketNum];
			if (NULL == pstuFingerMsg->szFingerPrintInfo)
			{
				delete pstuFingerMsg;
				pstuFingerMsg = NULL;
				return FALSE;
			}
			memcpy(pstuFingerMsg->szFingerPrintInfo, pstuFingerInfo->szFingerPrintInfo, pstuFingerInfo->nPacketLen * pstuFingerInfo->nPacketNum);

			if (NULL == dlg->pCardDlg)
			{
				delete []pstuFingerMsg->szFingerPrintInfo;
				pstuFingerMsg->szFingerPrintInfo = NULL;
				delete pstuFingerMsg;
				pstuFingerMsg = NULL;
				return FALSE;
			}
			dlg->pCardDlg->PostMessage(WM_ACCESS_FINGERPRINT, (WPARAM)pstuFingerMsg, NULL);

		}
		else
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
				dlg->PostMessage(WM_EVENTALARM_INFO, (WPARAM)pInfo, (LPARAM)0);
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
	CAccessControlDlg *pThis = (CAccessControlDlg *)dwUser;
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
	CAccessControlDlg *pThis = (CAccessControlDlg *)dwUser;
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
// CAccessControlDlg dialog

CAccessControlDlg::CAccessControlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAccessControlDlg::IDD, pParent)
{	
	//{{AFX_DATA_INIT(CAccessControlDlg)
	m_strUserName = "";
	m_nPort = 0;
	m_strPasswd = "";
	m_nEventAlarmCount = 0;
	m_bListenEvent = FALSE;
	m_bListenFingerPrint = FALSE;
	pCardDlg = NULL;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
}

CAccessControlDlg::~CAccessControlDlg()
{		

}
void CAccessControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAccessControlDlg)
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_ctrlIP);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_cbChannel);
	//}}AFX_DATA_MAP	
	DDX_Control(pDX, IDC_LISTCTRL_EVENT, m_listEventAlarm);
}

BEGIN_MESSAGE_MAP(CAccessControlDlg, CDialog)
	//{{AFX_MSG_MAP(CAccessControlDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, OnButtonLogout)
	ON_WM_DESTROY()	
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_EVENTALARM_INFO, OnEventAlarmInfo)
	ON_MESSAGE(WM_FINGERPRINT, OnGetFingerPrint)
	ON_WM_TIMER()		
	ON_WM_CLOSE()
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_BUTTON_START2, &CAccessControlDlg::OnButtonStartSubscribeEvent)
	ON_BN_CLICKED(IDC_BUTTON_STOP2, &CAccessControlDlg::OnButtonStopSubscribeEvent)
	ON_COMMAND(ID_GENERALCONFIG_DEVINFO, &CAccessControlDlg::OnGeneralconfigDevinfo)
	ON_COMMAND(ID_GENERALCONFIG_NET, &CAccessControlDlg::OnGeneralconfigNet)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_DOOR, &CAccessControlDlg::OnBnClickedButtonOpenDoor)
	ON_COMMAND(ID_GENERALCONFIG_DEVTIME, &CAccessControlDlg::OnGeneralconfigDevtime)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_DOOR, &CAccessControlDlg::OnBnClickedButtonCloseDoor)
	ON_COMMAND(ID_MAINTENANCE_CHANGEPASSWORD, &CAccessControlDlg::OnMaintenanceChangepassword)
	ON_COMMAND(ID_MAINTENANCE_REBOOT, &CAccessControlDlg::OnMaintenanceReboot)
	ON_COMMAND(ID_MAINTENANCE_RESET, &CAccessControlDlg::OnMaintenanceReset)
	ON_COMMAND(ID_MAINTENANCE_UPGRADE, &CAccessControlDlg::OnMaintenanceUpgrade)
	ON_COMMAND(ID_MAINTENANCE_AUTOMATRIX, &CAccessControlDlg::OnMaintenanceAutomatrix)
	ON_COMMAND(ID_ACCESSCONFIG_TIMESCHEDULE, &CAccessControlDlg::OnAccessconfigTimeschedule)
	ON_COMMAND(ID_ACCESSCONFIG_DOORCONIFG, &CAccessControlDlg::OnAccessconfigDoorconifg)
	ON_COMMAND(ID_ACCESSCONFIG_OPENDOORGROUP, &CAccessControlDlg::OnAccessconfigOpendoorgroup)
	ON_COMMAND(ID_ACCESSCONFIG_FIRSTENTER, &CAccessControlDlg::OnAccessconfigFirstenter)
	ON_COMMAND(ID_ACCESSCONFIG_ABLOCK, &CAccessControlDlg::OnAccessconfigAblock)
	ON_COMMAND(ID_ACCESSCONFIG_OPENDOORROUTE, &CAccessControlDlg::OnAccessconfigOpendoorroute)
	ON_COMMAND(ID_ACCESSCONFIG_ACCESSPASSWORD, &CAccessControlDlg::OnAccessconfigAccesspassword)
	ON_COMMAND(ID_QUERYRECORD_DOORRECORD, &CAccessControlDlg::OnQueryrecordDoorrecord)
	ON_COMMAND(ID_QUERYRECORD_ALARMRECORD, &CAccessControlDlg::OnQueryrecordAlarmrecord)
	ON_COMMAND(ID_QUERYRECORD_LOG, &CAccessControlDlg::OnQueryrecordLog)
	ON_COMMAND(ID_USERMANAGER, &CAccessControlDlg::OnUsermanager)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_ALWAYS, &CAccessControlDlg::OnBnClickedButtonOpenAlways)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_ALWAYS, &CAccessControlDlg::OnBnClickedButtonCloseAlways)
	ON_BN_CLICKED(IDC_BUTTON_GET_DOORSTATE, &CAccessControlDlg::OnBnClickedButtonGetDoorstate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAccessControlDlg message handlers

BOOL CAccessControlDlg::OnInitDialog()
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
	InitCtrls();
	InitNetSdk();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAccessControlDlg::InitMenu()
{
	m_Menu.LoadMenu(IDR_MENU1);
	m_Menu.ModifyMenu(0,MF_BYPOSITION,NULL,ConvertString("GeneralConfig"));
	m_Menu.GetSubMenu(0)->ModifyMenu(0,MF_BYPOSITION,ID_GENERALCONFIG_DEVINFO,ConvertString("DevInfo"));
	m_Menu.GetSubMenu(0)->ModifyMenu(1,MF_BYPOSITION,ID_GENERALCONFIG_NET,ConvertString("Net"));
	m_Menu.GetSubMenu(0)->ModifyMenu(2,MF_BYPOSITION,ID_GENERALCONFIG_DEVTIME,ConvertString("DevTime"));

	m_Menu.ModifyMenu(1,MF_BYPOSITION,NULL,ConvertString("Maintenance"));
	m_Menu.GetSubMenu(1)->ModifyMenu(0,MF_BYPOSITION,ID_MAINTENANCE_CHANGEPASSWORD,ConvertString("ChangePassword"));
	m_Menu.GetSubMenu(1)->ModifyMenu(1,MF_BYPOSITION,ID_MAINTENANCE_REBOOT,ConvertString("Reboot"));
	m_Menu.GetSubMenu(1)->ModifyMenu(2,MF_BYPOSITION,ID_MAINTENANCE_RESET,ConvertString("Reset"));
	m_Menu.GetSubMenu(1)->ModifyMenu(3,MF_BYPOSITION,ID_MAINTENANCE_UPGRADE,ConvertString("Upgrade"));
	m_Menu.GetSubMenu(1)->ModifyMenu(4,MF_BYPOSITION,ID_MAINTENANCE_AUTOMATRIX,ConvertString("AutoMatrix"));

	m_Menu.ModifyMenu(2,MF_BYPOSITION,ID_USERMANAGER,ConvertString("UserManagement"));
	m_Menu.ModifyMenu(3,MF_BYPOSITION,NULL,ConvertString("AccessConfig"));
	m_Menu.GetSubMenu(3)->ModifyMenu(0,MF_BYPOSITION,ID_ACCESSCONFIG_TIMESCHEDULE,ConvertString("TimeSchedule"));
	m_Menu.GetSubMenu(3)->ModifyMenu(1,MF_BYPOSITION,ID_ACCESSCONFIG_DOORCONIFG,ConvertString("DoorConfig"));
	m_Menu.GetSubMenu(3)->ModifyMenu(2,MF_BYPOSITION,ID_ACCESSCONFIG_OPENDOORGROUP,ConvertString("OpenDoorGroup"));
	m_Menu.GetSubMenu(3)->ModifyMenu(3,MF_BYPOSITION,ID_ACCESSCONFIG_FIRSTENTER,ConvertString("FirstEnter"));
	m_Menu.GetSubMenu(3)->ModifyMenu(4,MF_BYPOSITION,ID_ACCESSCONFIG_ABLOCK,ConvertString("ABLock"));
	m_Menu.GetSubMenu(3)->ModifyMenu(5,MF_BYPOSITION,ID_ACCESSCONFIG_OPENDOORROUTE,ConvertString("OpenDoorRoute"));
	m_Menu.GetSubMenu(3)->ModifyMenu(6,MF_BYPOSITION,ID_ACCESSCONFIG_ACCESSPASSWORD,ConvertString("AccessPassword"));

	m_Menu.ModifyMenu(4,MF_BYPOSITION,NULL,ConvertString("QueryRecord"));
	m_Menu.GetSubMenu(4)->ModifyMenu(0,MF_BYPOSITION,ID_QUERYRECORD_DOORRECORD,ConvertString("DoorRecord"));
	//m_Menu.GetSubMenu(4)->ModifyMenu(1,MF_BYPOSITION,ID_QUERYRECORD_ALARMRECORD,ConvertString("AlarmRecord"));
	m_Menu.GetSubMenu(4)->ModifyMenu(1,MF_BYPOSITION,ID_QUERYRECORD_LOG,ConvertString("Log"));
	SetMenu(&m_Menu);
}

void CAccessControlDlg::InitCtrls()
{
	SetDlgItemText(IDC_IPADDRESS_IP, "172.23.32.61");
	SetDlgItemInt(IDC_EDIT_PORT, 37777);
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWORD, "admin123");
	InitEventAlarmCtrl();
}

void CAccessControlDlg::InitNetSdk()
{
	Device::GetInstance().SetReconnect(DevDisConnect,DevReConnect,DevSubDisConnect,(LDWORD)this);
}

void CAccessControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAccessControlDlg::OnPaint() 
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
HCURSOR CAccessControlDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CAccessControlDlg::OnButtonLogin() 
{
	char szIp[32] = {0};
	m_ctrlIP.GetWindowText(szIp, sizeof(szIp));
	m_nPort = GetDlgItemInt(IDC_EDIT_PORT);
	GetDlgItemText(IDC_EDIT_USERNAME,m_strUserName);
	GetDlgItemText(IDC_EDIT_PASSWORD,m_strPasswd);

	BOOL bret = Device::GetInstance().Login(szIp, (WORD)m_nPort, m_strUserName.GetBuffer(0), m_strPasswd.GetBuffer(0));
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Login failed!"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		this->ChangeUI(UI_STATE_LOGIN);
		SetWindowText(ConvertString("AccessControl1s"));
	}
}

void CAccessControlDlg::OnButtonLogout() 
{
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"),ConvertString("Prompt"));
		return;
	}

	OnButtonStopSubscribeEvent();
	ClearEventAlarmUIInfo();

	Device::GetInstance().Logout();
	this->ChangeUI(UI_STATE_LOGOUT);
	SetWindowText(ConvertString("AccessControl1s"));
}

void CAccessControlDlg::OnClose()
{	
	CDialog::OnClose();
}

BOOL CAccessControlDlg::PreTranslateMessage(MSG* pMsg) 
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

void CAccessControlDlg::OnDestroy()
{	
	CDialog::OnDestroy();

	if (Device::GetInstance().GetLoginState())
	{
		OnButtonStopSubscribeEvent();
		ClearEventAlarmUIInfo();
		Device::GetInstance().Logout();
	}
	Device::GetInstance().Destroy();
}

void CAccessControlDlg::ChangeUI( int nState )
{
 	switch (nState)
 	{
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
  			GetDlgItem(IDC_BUTTON_START2)->EnableWindow(TRUE);
  			GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);
 			BYTE i= 0;
			int nChn = 4;
			int nAccessGroup = 0;
			BOOL bret = Device::GetInstance().GetAccessCount(nAccessGroup);
			if (bret && nAccessGroup > 0)
			{
				nChn = nAccessGroup;
			}
 			for (i = 0; i < nChn; i++)
 			{
 				char szContent[8] = {0};
 				_snprintf(szContent, sizeof(szContent), "%d", i);
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
  			GetDlgItem(IDC_BUTTON_START2)->EnableWindow(FALSE);
  			GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);
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

void CAccessControlDlg::ClearEventAlarmUIInfo() 
{
	m_listEventAlarm.DeleteAllItems();
	m_nEventAlarmCount = 0;
}

HBRUSH CAccessControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);	
	
	return hbr;
}

void CAccessControlDlg::InitEventAlarmCtrl(void)
{
	m_listEventAlarm.SetExtendedStyle(m_listEventAlarm.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listEventAlarm.SetExtendedStyle(m_listEventAlarm.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("Index", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("Type", 100));
	vecTitles.push_back(ColDesObj("UserID", 60));
	vecTitles.push_back(ColDesObj("CardNo", 60));
	vecTitles.push_back(ColDesObj("Door", 60));
	vecTitles.push_back(ColDesObj("OpenMethod", 60));
	vecTitles.push_back(ColDesObj("Status", 60));

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

void CAccessControlDlg::OnButtonStartSubscribeEvent()
{
	m_nEventAlarmCount = 0;
	BOOL bret = StartListenEventAlarm();	
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Subscribe failed!"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		m_bListenEvent = true;
		this->ChangeUI(UI_STATE_ATTACH_EVENT);
	}
}

void CAccessControlDlg::OnButtonStopSubscribeEvent()
{
	m_bListenEvent = false;
	BOOL bret = StopListenEventAlarm();
	if (bret)
	{
		this->ChangeUI(UI_STATE_DETACH_EVENT);
	}
}

BOOL CAccessControlDlg::StartListenEventAlarm()
{
	if (!m_bListenEvent && !m_bListenFingerPrint)
	{
		BOOL bret =  Device::GetInstance().StartListenEvent(MessCallBack,(LDWORD)this);
		return bret;
	}
	return TRUE;
}

BOOL CAccessControlDlg::StopListenEventAlarm()
{
	if (!m_bListenEvent && !m_bListenFingerPrint)
	{
		if (!Device::GetInstance().StopListenEvent())
		{
			return FALSE;
		}
		this->ChangeUI(UI_STATE_DETACH_EVENT);
	}
	return TRUE;
}

LRESULT CAccessControlDlg::OnGetFingerPrint(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 1)//start
	{
		BOOL bret = StartListenEventAlarm();	
		if (bret)
		{
			m_bListenFingerPrint = true;
			this->ChangeUI(UI_STATE_ATTACH_EVENT);
		}
	}
	else//stop
	{
		m_bListenFingerPrint = false;
		BOOL bret = StopListenEventAlarm();
	}
	return 0;
}

LRESULT CAccessControlDlg::OnEventAlarmInfo(WPARAM wParam, LPARAM lParam)
{
	AlarmInfo* pAlarmInfo = (AlarmInfo*)wParam;
	if (!pAlarmInfo || !pAlarmInfo->pBuf || pAlarmInfo->dwBufLen <= 0)
	{
		if (NULL != pAlarmInfo)
		{
			if (NULL != pAlarmInfo->pBuf )
			{
				delete []pAlarmInfo->pBuf;
				pAlarmInfo->pBuf = NULL;
			}
			delete pAlarmInfo;
			pAlarmInfo = NULL;
		}
		return -1;
	}
	m_listEventAlarm.SetRedraw(FALSE);
	SYSTEMTIME st;
	GetLocalTime(&st);
	if (pAlarmInfo->lCommand == DH_ALARM_ACCESS_CTL_EVENT)
	{
		ALARM_ACCESS_CTL_EVENT_INFO *AlarmCtlEventInfo = (ALARM_ACCESS_CTL_EVENT_INFO*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmCtlEventInfo->stuTime.dwYear, AlarmCtlEventInfo->stuTime.dwMonth, AlarmCtlEventInfo->stuTime.dwDay, AlarmCtlEventInfo->stuTime.dwHour, AlarmCtlEventInfo->stuTime.dwMinute, AlarmCtlEventInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(AlarmCtlEventInfo->emEventType);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, AlarmCtlEventInfo->szUserID);
		m_listEventAlarm.SetItemText(0, 4, AlarmCtlEventInfo->szCardNo);
		str.Format("%d",AlarmCtlEventInfo->nDoor);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, (AccessDoorOpenMethod2Str(AlarmCtlEventInfo->emOpenMethod)).c_str());
		str = AlarmCtlEventInfo->bStatus ? "True":"False";
		m_listEventAlarm.SetItemText(0, 7, ConvertString(str));
		m_nEventAlarmCount++;
	}
	else if (pAlarmInfo->lCommand == DH_ALARM_ACCESS_CTL_NOT_CLOSE)
	{
		ALARM_ACCESS_CTL_NOT_CLOSE_INFO *AlarmCtlNotCloseInfo = (ALARM_ACCESS_CTL_NOT_CLOSE_INFO*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmCtlNotCloseInfo->stuTime.dwYear, AlarmCtlNotCloseInfo->stuTime.dwMonth, AlarmCtlNotCloseInfo->stuTime.dwDay, AlarmCtlNotCloseInfo->stuTime.dwHour, AlarmCtlNotCloseInfo->stuTime.dwMinute, AlarmCtlNotCloseInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, "");
		m_listEventAlarm.SetItemText(0, 4, "");
		str.Format("%d",AlarmCtlNotCloseInfo->nDoor);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, "");
		if (AlarmCtlNotCloseInfo->nAction >0)
		{
			m_listEventAlarm.SetItemText(0, 7, ConvertString("Stop"));
		}
		else
		{
			m_listEventAlarm.SetItemText(0, 7, ConvertString("Start"));
		}
		m_nEventAlarmCount++;
	}
	else if (pAlarmInfo->lCommand == DH_ALARM_ACCESS_CTL_BREAK_IN)
	{
		ALARM_ACCESS_CTL_BREAK_IN_INFO *AlarmCtlBreakInInfo = (ALARM_ACCESS_CTL_BREAK_IN_INFO*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmCtlBreakInInfo->stuTime.dwYear, AlarmCtlBreakInInfo->stuTime.dwMonth, AlarmCtlBreakInInfo->stuTime.dwDay, AlarmCtlBreakInInfo->stuTime.dwHour, AlarmCtlBreakInInfo->stuTime.dwMinute, AlarmCtlBreakInInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, "");
		m_listEventAlarm.SetItemText(0, 4, "");
		str.Format("%d",AlarmCtlBreakInInfo->nDoor);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, "");
		m_listEventAlarm.SetItemText(0, 7, "");
		m_nEventAlarmCount++;
	}
	else if (pAlarmInfo->lCommand == DH_ALARM_ACCESS_CTL_REPEAT_ENTER)
	{
		ALARM_ACCESS_CTL_REPEAT_ENTER_INFO *AlarmCtlRepeatEnterInfo = (ALARM_ACCESS_CTL_REPEAT_ENTER_INFO*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmCtlRepeatEnterInfo->stuTime.dwYear, AlarmCtlRepeatEnterInfo->stuTime.dwMonth, AlarmCtlRepeatEnterInfo->stuTime.dwDay, AlarmCtlRepeatEnterInfo->stuTime.dwHour, AlarmCtlRepeatEnterInfo->stuTime.dwMinute, AlarmCtlRepeatEnterInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, "");
		m_listEventAlarm.SetItemText(0, 4, AlarmCtlRepeatEnterInfo->szCardNo);
		str.Format("%d",AlarmCtlRepeatEnterInfo->nDoor);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, "");
		m_listEventAlarm.SetItemText(0, 7, "");
		m_nEventAlarmCount++;
	}
	else if (pAlarmInfo->lCommand == DH_ALARM_ACCESS_CTL_DURESS)
	{
		ALARM_ACCESS_CTL_DURESS_INFO *AlarmCtlDuressInfo = (ALARM_ACCESS_CTL_DURESS_INFO*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmCtlDuressInfo->stuTime.dwYear, AlarmCtlDuressInfo->stuTime.dwMonth, AlarmCtlDuressInfo->stuTime.dwDay, AlarmCtlDuressInfo->stuTime.dwHour, AlarmCtlDuressInfo->stuTime.dwMinute, AlarmCtlDuressInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, AlarmCtlDuressInfo->szUserID);
		m_listEventAlarm.SetItemText(0, 4, AlarmCtlDuressInfo->szCardNo);
		str.Format("%d",AlarmCtlDuressInfo->nDoor);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, "");
		m_listEventAlarm.SetItemText(0, 7, "");
		m_nEventAlarmCount++;
	}
	else if (pAlarmInfo->lCommand == DH_ALARM_CHASSISINTRUDED)
	{
		ALARM_CHASSISINTRUDED_INFO *AlarmChassisIntrudedInfo = (ALARM_CHASSISINTRUDED_INFO*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmChassisIntrudedInfo->stuTime.dwYear, AlarmChassisIntrudedInfo->stuTime.dwMonth, AlarmChassisIntrudedInfo->stuTime.dwDay, AlarmChassisIntrudedInfo->stuTime.dwHour, AlarmChassisIntrudedInfo->stuTime.dwMinute, AlarmChassisIntrudedInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		int nReadIDLen = strlen(AlarmChassisIntrudedInfo->szReaderID);
		if (nReadIDLen > 0)
		{
			dstr = ConvertString("CardreaderAntidemolition");
		}
		else
		{
			dstr = EventType2Str(pAlarmInfo->lCommand);
		}
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, "");
		m_listEventAlarm.SetItemText(0, 4, "");
		str.Format("%d",AlarmChassisIntrudedInfo->nChannelID);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, "");
		if (AlarmChassisIntrudedInfo->nAction >0)
		{
			m_listEventAlarm.SetItemText(0, 7, ConvertString("Stop"));
		}
		else
		{
			m_listEventAlarm.SetItemText(0, 7, ConvertString("Start"));
		}
		m_nEventAlarmCount++;
	}
	else if (pAlarmInfo->lCommand == DH_ALARM_ALARM_EX2)
	{
		ALARM_ALARM_INFO_EX2 *AlarmExInfo = (ALARM_ALARM_INFO_EX2*)pAlarmInfo->pBuf;
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
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",AlarmExInfo->stuTime.dwYear, AlarmExInfo->stuTime.dwMonth, AlarmExInfo->stuTime.dwDay, AlarmExInfo->stuTime.dwHour, AlarmExInfo->stuTime.dwMinute, AlarmExInfo->stuTime.dwSecond);
		m_listEventAlarm.SetItemText(0, 1, str);
		string dstr = EventType2Str(pAlarmInfo->lCommand);
		m_listEventAlarm.SetItemText(0, 2, dstr.c_str());
		m_listEventAlarm.SetItemText(0, 3, "");
		m_listEventAlarm.SetItemText(0, 4, "");
		str.Format("%d",AlarmExInfo->nChannelID);
		m_listEventAlarm.SetItemText(0, 5, str);
		m_listEventAlarm.SetItemText(0, 6, "");
		if (AlarmExInfo->nAction >0)
		{
			m_listEventAlarm.SetItemText(0, 7, ConvertString("Stop"));
		}
		else
		{
			m_listEventAlarm.SetItemText(0, 7, ConvertString("Start"));
		}
		m_nEventAlarmCount++;
	}


	m_listEventAlarm.SetRedraw(TRUE);
	if (NULL != pAlarmInfo)
	{
		if (NULL != pAlarmInfo->pBuf )
		{
			delete []pAlarmInfo->pBuf;
			pAlarmInfo->pBuf = NULL;
		}
		delete pAlarmInfo;
		pAlarmInfo = NULL;
	}
	return 0;
}

LRESULT CAccessControlDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

LRESULT CAccessControlDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	SetWindowText(ConvertString("AccessControl1s"));
	return 0;
}

void CAccessControlDlg::OnBnClickedButtonOpenDoor()
{
	GetDlgItem(IDC_BUTTON_OPEN_DOOR)->EnableWindow(FALSE);
	int nChannel = m_cbChannel.GetCurSel();
	if (nChannel >= 0)
	{
		BOOL bret = Device::GetInstance().OpenDoor(nChannel);
		if (!bret)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_OPEN_DOOR)->EnableWindow(TRUE);
}

void CAccessControlDlg::OnBnClickedButtonCloseDoor()
{
	GetDlgItem(IDC_BUTTON_CLOSE_DOOR)->EnableWindow(FALSE);
	int nChannel = m_cbChannel.GetCurSel();
	if (nChannel >= 0)
	{
		BOOL bret = Device::GetInstance().CloseDoor(nChannel);
		if (!bret)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_CLOSE_DOOR)->EnableWindow(TRUE);
}

void CAccessControlDlg::OnGeneralconfigDevinfo()
{
	DevInfo dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnGeneralconfigNet()
{
	NetWork dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnGeneralconfigDevtime()
{
	DevTime dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnMaintenanceChangepassword()
{
	ModifyPassword dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnMaintenanceReboot()
{
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		return;
	}

	if (MessageBox(ConvertString("Are you sure to reboot?"), ConvertString("Prompt"), MB_YESNO) == IDYES)
	{
		BOOL bRet = Device::GetInstance().DevReboot();
		if (!bRet)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Reboot failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
}

void CAccessControlDlg::OnMaintenanceReset()
{
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		return;
	}
	if (MessageBox(ConvertString("Are you sure to reset all?"), ConvertString("Prompt"), MB_YESNO) == IDYES)
	{
		BOOL bRet = Device::GetInstance().DevReset();
		if (!bRet)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Restore all failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
}

void CAccessControlDlg::OnMaintenanceUpgrade()
{
	DevUpgrade dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnMaintenanceAutomatrix()
{
	AutoMatrix dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigTimeschedule()
{
	TimeSchedule dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigDoorconifg()
{
	DoorConfig dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigOpendoorgroup()
{
	OpenDoorGroup dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigFirstenter()
{
	FirstEnter dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigAblock()
{
	ABLock dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigOpendoorroute()
{
	OpenDoorRoute dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnAccessconfigAccesspassword()
{
	AccessCtlPWD dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnQueryrecordDoorrecord()
{
	DoorRecord dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnQueryrecordAlarmrecord()
{
	AlarmRecord dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnQueryrecordLog()
{
	Log dlg(this);
	dlg.DoModal();
}

void CAccessControlDlg::OnUsermanager()
{
	pCardDlg = new CCard(this);
	if (NULL != pCardDlg)
	{
		pCardDlg->DoModal();
		delete pCardDlg;
		pCardDlg = NULL;
	}
}

void CAccessControlDlg::OnBnClickedButtonOpenAlways()
{
	GetDlgItem(IDC_BUTTON_OPEN_ALWAYS)->EnableWindow(FALSE);
	int nChannel = m_cbChannel.GetCurSel();
	if (nChannel >= 0)
	{
		BOOL bret = Device::GetInstance().OpenAlways(nChannel);
		if (!bret)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_OPEN_ALWAYS)->EnableWindow(TRUE);
}

void CAccessControlDlg::OnBnClickedButtonCloseAlways()
{
	GetDlgItem(IDC_BUTTON_CLOSE_ALWAYS)->EnableWindow(FALSE);
	int nChannel = m_cbChannel.GetCurSel();
	if (nChannel >= 0)
	{
		BOOL bret = Device::GetInstance().CloseAlways(nChannel);
		if (!bret)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_CLOSE_ALWAYS)->EnableWindow(TRUE);
}

void CAccessControlDlg::OnBnClickedButtonGetDoorstate()
{
	int nChannel = m_cbChannel.GetCurSel();
	if (nChannel >= 0)
	{
		EM_NET_DOOR_STATUS_TYPE emStateType;
		BOOL bret = Device::GetInstance().GetDoorState(nChannel,emStateType);
		if (!bret)
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
		else
		{
			switch(emStateType)
			{
			case EM_NET_DOOR_STATUS_UNKNOWN:
				MessageBox(ConvertString("Unknown"), ConvertString("Prompt"));
				break;
			case EM_NET_DOOR_STATUS_OPEN:
				MessageBox(ConvertString("Open"), ConvertString("Prompt"));
				break;
			case EM_NET_DOOR_STATUS_CLOSE:
				MessageBox(ConvertString("Close"), ConvertString("Prompt"));
				break;
			case EM_NET_DOOR_STATUS_BREAK:
				MessageBox(ConvertString("Break"), ConvertString("Prompt"));
				break;
			default:
				break;
			}
		}
	}
}
