// VehicleGPSDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VehicleGPS.h"
#include "VehicleGPSDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_EVENT_DISCONNECT (WM_USER + 112)
#define WM_EVENT_RECONNECT (WM_USER + 113)
#define WM_EVENT_GPSINFO	(WM_USER + 114)
#define WM_LOGIN_DISCONNECTED (WM_USER + 115)
#define WM_LOGIN_DEVICE		(WM_USER + 116)

struct ListenDevInfo
{
	CString	strSerial;
	CString	strIP;
	int		nPort;
};

struct GPSInfo
{
	double	dLongitude;
	double	dLatitude;
	double	dSpeed;
};
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{

}
BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVehicleGPSDlg dialog

CVehicleGPSDlg::CVehicleGPSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVehicleGPSDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bRadio = FALSE;
	m_loginID = 0;
	m_strListenDevID = "";
	m_strListenDevName = "";
	m_strListenDevUser = "";
	m_strListenDevPwd = "";
	m_lListenHdl = 0;
	m_bLogined = FALSE;
}

CVehicleGPSDlg::~CVehicleGPSDlg()
{

}

void CVehicleGPSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_IPLOGIN, m_bRadio);
}

BEGIN_MESSAGE_MAP(CVehicleGPSDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CVehicleGPSDlg::OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CVehicleGPSDlg::OnBtnLogout)
	ON_MESSAGE(WM_EVENT_DISCONNECT, &CVehicleGPSDlg::OnDeviceDisConnect)
	ON_MESSAGE(WM_EVENT_RECONNECT, &CVehicleGPSDlg::OnDeviceReconnect)
	ON_MESSAGE(WM_EVENT_GPSINFO, &CVehicleGPSDlg::OnGPSInfo)
	ON_MESSAGE(WM_LOGIN_DISCONNECTED, &CVehicleGPSDlg::OnLoginDisconnected)
	ON_MESSAGE(WM_LOGIN_DEVICE, &CVehicleGPSDlg::OnLoginDevice)
	ON_WM_DESTROY()

	ON_BN_CLICKED(IDC_RADIO_IPLOGIN, &CVehicleGPSDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIO_AUTO, &CVehicleGPSDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_BTN_STARTLISTEN, &CVehicleGPSDlg::OnBnClickedBtnStartlisten)
	ON_BN_CLICKED(IDC_BTN_STOPLISTEN, &CVehicleGPSDlg::OnBnClickedBtnStoplisten)
	ON_BN_CLICKED(IDC_BTN_ATTACHEVENT, &CVehicleGPSDlg::OnBnClickedBtnAttachevent)
	ON_BN_CLICKED(IDC_BTN_DETACHEVENT, &CVehicleGPSDlg::OnBnClickedBtnDetachevent)
	ON_BN_CLICKED(IDC_BTN_LISTEN_LOGOUT, &CVehicleGPSDlg::OnBnClickedBtnListenLogout)
END_MESSAGE_MAP()


// CVehicleGPSDlg message handlers

BOOL CVehicleGPSDlg::OnInitDialog()
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
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, ConvertString(strAboutMenu));
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	g_SetWndStaticText(this);

	SetDlgItemText(IDC_IPADDRESS, "172.23.29.51");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWD, "admin123");

	SetDlgItemText(IDC_LISTEN_IPADDRESS, "10.34.3.174");
	SetDlgItemText(IDC_EDIT_LISTENPORT, "9500");
	SetDlgItemText(IDC_EDIT_DEVICEID, "2A03308PAQ00227");
	SetDlgItemText(IDC_EDIT_DEVICENAME, "admin");
	SetDlgItemText(IDC_EDIT_DEVICEUSER, "admin");
	SetDlgItemText(IDC_EDIT_DEVICEPWD, "admin123");

	// TODO: Add extra initialization here
	InitDlgItem();
	InitNetSDK();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVehicleGPSDlg::InitDlgItem()
{
	((CButton*)GetDlgItem(IDC_RADIO_IPLOGIN))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_RADIO_AUTO))->SetCheck(BST_UNCHECKED);
	GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);

	wchar_t wstrItued[32] = {0};
	swprintf(wstrItued,L"0.000000°");
	CString strItued(wstrItued);

	//经度
	SetDlgItemText(IDC_EDIT_LONGITUDE, strItued);
	//纬度
	SetDlgItemText(IDC_EDIT_LATITUDE, strItued);
	//速度
	SetDlgItemText(IDC_EDIT_SPEED, "0km/h");
}

void CVehicleGPSDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	OnBtnLogout();
	UnInitNetSDK();
}	

void CVehicleGPSDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CVehicleGPSDlg::OnPaint()
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
HCURSOR CVehicleGPSDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CVehicleGPSDlg::OnBtnLogin()
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
		
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);

		GetDlgItem(IDC_RADIO_IPLOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(FALSE);
	}
	else
	{
		ShowLoginErrorReason(stOutparam.nError);
	}
}

void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CVehicleGPSDlg *pDlgInstance = (CVehicleGPSDlg *)dwUser;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_EVENT_DISCONNECT, 0, 0);	
	}

}

void CALLBACK HaveReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(0 != dwUser)
	{
		CVehicleGPSDlg *pDlgInstance = (CVehicleGPSDlg *)dwUser;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		::PostMessage(hwnd, WM_EVENT_RECONNECT, 0, 0);
	}

}

void CVehicleGPSDlg::InitNetSDK()
{
	CLIENT_Init(DisConnectFunc,(LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(HaveReConnectFunc, (LDWORD)this);
}

void CVehicleGPSDlg::UnInitNetSDK()
{
	CLIENT_Cleanup();
}

LRESULT CVehicleGPSDlg::OnDeviceDisConnect(WPARAM wParam, LPARAM lParam)
{
	SetWindowText(ConvertString(_T("Network disconnected!")));
	if (m_loginID != NULL)
	{
		CLIENT_SubcribeGPS(m_loginID,FALSE,-1,1);//取消GPS订阅
		if (m_bRadio)//主动注册登出设备
		{
			CLIENT_Logout(m_loginID);
			m_loginID = 0;
			m_bLogined = FALSE;
		}
	}
	return 0;
}

LRESULT CVehicleGPSDlg::OnDeviceReconnect(WPARAM wParam, LPARAM lParam)
{	
	SetWindowText(ConvertString(_T("VehicleGPS")));
	if (!m_bRadio)//IP登录设备方式
	{
		GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
	}
	return 0;
}

LRESULT CVehicleGPSDlg::OnLoginDisconnected(WPARAM wParam, LPARAM lParam)
{
	MessageBox(ConvertString(_T("Logining DisConnect!")), ConvertString(_T("Prompt")));
	return 0;
}

void CVehicleGPSDlg::ShowLoginErrorReason(int nError)
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

void CVehicleGPSDlg::OnBtnLogout()
{
	//TODO: Add your control notification handler code here
	SetWindowText(ConvertString(_T("VehicleGPS")));

	if (m_loginID != NULL)
	{
		CLIENT_SubcribeGPS(m_loginID,FALSE,-1,1);//取消GPS订阅
		CLIENT_Logout(m_loginID);
		m_loginID = 0;
	}
	// set login button  and logout button status
	GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);

	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);

	GetDlgItem(IDC_RADIO_IPLOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(TRUE);
}

void CVehicleGPSDlg::OnBnClickedRadio()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!m_bRadio)
	{
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STARTLISTEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOPLISTEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LISTEN_LOGOUT)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STARTLISTEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOPLISTEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LISTEN_LOGOUT)->EnableWindow(FALSE);
	}
}


//Listening server callback function original shape
int CALLBACK ListenCallback(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, LDWORD dwUserData)
{
	if(0 != dwUserData)
	{
		CString strInfo;
		CVehicleGPSDlg *dlg = (CVehicleGPSDlg *)dwUserData;
		HWND hwnd = dlg->GetSafeHwnd();
		dlg->DealListenCB(lHandle, pIp,wPort, lCommand, pParam, dwParamLen, hwnd);
	}

	return 0;
}

void CVehicleGPSDlg::DealListenCB(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, HWND hwnd)
{
	if (DH_DVR_DISCONNECT == lCommand)
	{
		char* szDevSerial =(char*)pParam;
		CString strSerial = szDevSerial;
		if (strSerial == m_strListenDevID)
		{
			::PostMessage(hwnd,WM_LOGIN_DISCONNECTED,0,0);
		}
	}
	else if (DH_DVR_SERIAL_RETURN == lCommand)
	{
		char* szDevSerial =(char*)pParam;
		CString strSerial = szDevSerial;
		if (strSerial == m_strListenDevID && !m_bLogined)//相同的序列号且未登录的设备
		{
			ListenDevInfo *pInfo = new ListenDevInfo;
			pInfo->strSerial = (char*)pParam;
			pInfo->strIP = pIp;
			pInfo->nPort = wPort;
			
			::PostMessage(hwnd,WM_LOGIN_DEVICE,WPARAM(pInfo),0);		
		}
	}
}

LRESULT CVehicleGPSDlg::OnLoginDevice(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == NULL)
	{
		return 0;
	}
	
	ListenDevInfo *pInfo = (ListenDevInfo*)wParam;
	char szSerial[64] = {0};
	char szIP[64] = {0};
	int nPort = 0;

	strncpy(szSerial, pInfo->strSerial, sizeof(szSerial) - 1);
	strncpy(szIP, pInfo->strIP, sizeof(szIP) - 1);
	nPort = pInfo->nPort;

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, szIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, m_strListenDevPwd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, m_strListenDevUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_SERVER_CONN;
	stInparam.pCapParam = (void*)szSerial;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
	if (lRet)
	{
		m_loginID = lRet;
		m_bLogined = TRUE;

		GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LISTEN_LOGOUT)->EnableWindow(TRUE);
		GetDlgItem(IDC_LISTEN_IPADDRESS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_LISTENPORT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEVICEID)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEVICENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEVICEUSER)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_DEVICEPWD)->EnableWindow(FALSE);

		SetWindowText(ConvertString(_T("VehicleGPS")));
	}
	else
	{
		ShowLoginErrorReason(stOutparam.nError);
	}
	delete pInfo;
	pInfo = NULL;
	return 0;
}

void CVehicleGPSDlg::OnBnClickedBtnStartlisten()
{
	// TODO: 在此添加控件通知处理程序代码
	int nPort = 0;
	CString strIp;

	GetDlgItemText(IDC_LISTEN_IPADDRESS, strIp);
	nPort = GetDlgItemInt(IDC_EDIT_LISTENPORT);
	GetDlgItemText(IDC_EDIT_DEVICEID, m_strListenDevID);
	GetDlgItemText(IDC_EDIT_DEVICENAME, m_strListenDevName);
	GetDlgItemText(IDC_EDIT_DEVICEUSER, m_strListenDevUser);
	GetDlgItemText(IDC_EDIT_DEVICEPWD, m_strListenDevPwd);

	if (strIp.IsEmpty() || nPort == 0 || m_strListenDevID.IsEmpty() || m_strListenDevName.IsEmpty() ||
		m_strListenDevUser.IsEmpty() || m_strListenDevPwd.IsEmpty())
	{
		MessageBox(ConvertString(_T("DeviceInfo is not right,Add Fail!")), ConvertString(_T("Prompt")));
		return;
	}

	char *pcIP = (LPSTR)(LPCSTR)strIp;//监听IP

	m_lListenHdl = CLIENT_ListenServer(pcIP, nPort, 1000, ListenCallback, (LDWORD)this);
	if (0 == m_lListenHdl)
	{
		CString str;
		str.Format(ConvertString(_T("Listen Fail! ip:%s, port:%d")),pcIP, nPort);
		MessageBox(str,ConvertString(_T("Prompt")));
	}
	else
	{
		GetDlgItem(IDC_RADIO_IPLOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(FALSE);
		if (!m_loginID)
		{
			GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
		}
		GetDlgItem(IDC_BTN_STARTLISTEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STOPLISTEN)->EnableWindow(TRUE);
	}

}

void CVehicleGPSDlg::OnBnClickedBtnStoplisten()
{
	// TODO: 在此添加控件通知处理程序代码
	if (0 != m_lListenHdl)
	{
		CString strInfo;
		BOOL bRet = CLIENT_StopListenServer(m_lListenHdl);
		if (TRUE != bRet)
		{
			MessageBox(ConvertString(_T("Stop Listen Fail!")), ConvertString(_T("Prompt")));
		}
		else
		{
			if (!m_loginID)
			{
				GetDlgItem(IDC_RADIO_IPLOGIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(TRUE);
			}
			GetDlgItem(IDC_BTN_STARTLISTEN)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_STOPLISTEN)->EnableWindow(FALSE);
		}
		m_lListenHdl = 0;
	}
}

LRESULT CVehicleGPSDlg::OnGPSInfo(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == NULL)
	{
		return 0;
	}

	GPSInfo* pInfo= (GPSInfo *)wParam;
	if (pInfo)
	{
		//经度
		CString strlongitude = ChangeToLONLAT(pInfo->dLongitude, 1);
		SetDlgItemText(IDC_EDIT_LONGITUDE, strlongitude);
		//纬度
		CString strlatidude = ChangeToLONLAT(pInfo->dLatitude, 2);
		SetDlgItemText(IDC_EDIT_LATITUDE, strlatidude);
		//速度
		CString strspeed = "";
		pInfo->dSpeed = (pInfo->dSpeed)/(1000)*(1.852);//海里转公里
		strspeed.Format("%dkm/h", (int)pInfo->dSpeed);
		SetDlgItemText(IDC_EDIT_SPEED, strspeed);
		
		delete pInfo;
		pInfo = NULL;
	}

	return 0;
}

CString CVehicleGPSDlg::ChangeToLONLAT(double dParam, int nType)
{
	CString strInfo;
	if (nType == 1)//经度
	{
		int nloninteger = (int)dParam/1000000 - 180;//经纬度单位百万分之一
		//取余
		double nlondecimal = (int)dParam%1000000;
		double nlondecimal_1 = nlondecimal/1000000; //防止nlondecimal余数第一位是0	
		wchar_t wDecimal[32] = {0};			  //余数值转Cstring
		swprintf(wDecimal,L"%.6lf", nlondecimal_1);
		CString strDecimal(&wDecimal[2]);
		if (nloninteger > 0)
		{
			wchar_t wInteger[32] = {0};					//商数值转Cstring
			swprintf(wInteger,L"%d", nloninteger);
			CString strInteger(wInteger);
			strInfo = strInteger + "." + strDecimal + "°E";
		}
		else
		{
			wchar_t wInteger[32] = {0};					//商数值转Cstring
			swprintf(wInteger,L"%d", abs(nloninteger));
			CString strInteger(wInteger);
			strInfo = strInteger + "." + strDecimal + "°W";
		}
	}
	else if(nType == 2)//纬度
	{
		int nlatinteger = (int)dParam/1000000 - 90;
		//取余
		double nlatdecimal = (int)dParam%1000000;
		double nlatdecimal_1 = nlatdecimal/1000000; //防止nlondecimal余数第一位是0	
		wchar_t wDecimal[32] = {0};			  //余数值转Cstring
		swprintf(wDecimal,L"%.6lf", nlatdecimal_1);
		CString strDecimal(&wDecimal[2]);
		if (nlatinteger > 0)
		{
			wchar_t wInteger[32] = {0};					//商数值转Cstring
			swprintf(wInteger,L"%d", nlatinteger);
			CString strInteger(wInteger);
			strInfo = strInteger + "." + strDecimal + "°N";
		}
		else
		{
			wchar_t wInteger[32] = {0};					//商数值转Cstring
			swprintf(wInteger,L"%d", abs(nlatinteger));
			CString strInteger(wInteger);
			strInfo = strInteger + "." + strDecimal + "°S";
		}
	}
	return strInfo;
}


// GPS信息订阅回调--扩展
void CALLBACK AfGPSRevEx2(LLONG lLoginID, const LPNET_GPS_LOCATION_INFO lpData, LDWORD dwUserData, void *reserved)
{
	if(dwUserData)
	{
		CVehicleGPSDlg *pDlgInstance = (CVehicleGPSDlg *)dwUserData;
		HWND hwnd = pDlgInstance->GetSafeHwnd();
		GPSInfo* pInfo = new GPSInfo;
		pInfo->dLongitude = lpData->stuGpsInfo.longitude;
		pInfo->dLatitude = lpData->stuGpsInfo.latidude;
		pInfo->dSpeed = lpData->stuGpsInfo.speed;
		::PostMessage(hwnd, WM_EVENT_GPSINFO, WPARAM(pInfo), 0);
	}
}

void CVehicleGPSDlg::OnBnClickedBtnAttachevent()
{
	// TODO: 在此添加控件通知处理程序代码
	CLIENT_SetSubcribeGPSCallBackEX2(AfGPSRevEx2,(LDWORD)this);
	BOOL bResult = CLIENT_SubcribeGPS(m_loginID,TRUE,-1,1);//永久订阅，GPS信息每隔1s上报一次，客户可根据自己的需求改变上报时间。
	if (!bResult)
	{
		MessageBox(ConvertString(_T("Subcribe GPS failed!")), ConvertString(_T("Prompt")));
		return;
	}
	wchar_t wstrItued[32] = {0};
	swprintf(wstrItued,L"0.000000°");
	CString strItued(wstrItued);
	//经度
	SetDlgItemText(IDC_EDIT_LONGITUDE, strItued);
	//纬度
	SetDlgItemText(IDC_EDIT_LATITUDE, strItued);
	//高度
	SetDlgItemText(IDC_EDIT_SPEED, "0km/h");

	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(TRUE);
}

void CVehicleGPSDlg::OnBnClickedBtnDetachevent()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bResult = CLIENT_SubcribeGPS(m_loginID,FALSE,-1,1);//永久订阅，GPS信息每隔1s上报一次，客户可根据自己的需求改变上报时间。
	if (!bResult)
	{
		MessageBox(ConvertString(_T("Detach GPS failed!")), ConvertString(_T("Prompt")));
		return;
	}
	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
}

void CVehicleGPSDlg::OnBnClickedBtnListenLogout()
{
	// TODO: 在此添加控件通知处理程序代码
	//TODO: Add your control notification handler code here
	SetWindowText(ConvertString(_T("VehicleGPS")));

	if (m_loginID != NULL)
	{
		CLIENT_SubcribeGPS(m_loginID,FALSE,-1,1);//取消GPS订阅
		CLIENT_Logout(m_loginID);
		m_loginID = 0;
	}

	m_bLogined = FALSE;

	GetDlgItem(IDC_BTN_LISTEN_LOGOUT)->EnableWindow(FALSE);
	// set login button  and logout button status
	GetDlgItem(IDC_BTN_ATTACHEVENT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETACHEVENT)->EnableWindow(FALSE);
	//停止监听及登出设备
	if (!m_lListenHdl)
	{
		GetDlgItem(IDC_RADIO_IPLOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(TRUE);
	}
	GetDlgItem(IDC_LISTEN_IPADDRESS)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_LISTENPORT)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_DEVICEID)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_DEVICENAME)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_DEVICEUSER)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_DEVICEPWD)->EnableWindow(TRUE);
}

BOOL CVehicleGPSDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_KEYDOWN == pMsg->message)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			return true;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
