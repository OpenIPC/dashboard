// DevInitDlg.cpp : implementation file
//

#include "stdafx.h"
#include <ASSERT.H>
#include <iostream>
#include "DevInit.h"
#include "DevInitDlg.h"
#include "Winsock2.h"
#include "ChangePassword.h"
#include "InitDeviceDlg.h"
#include "ChangePassword.h"
#include "NetCardInfo.h"

#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
#endif


//defines NEW operator to prevent an exception from bing throw when new fails ,causing the return value to be invalid
#pragma warning(disable:4291)
#ifndef NEW
#define NEW new(std::nothrow)
#endif

#define WM_SEARCHDEVICES	(WM_USER + 100)
#define WM_TALK_DISCONNECT	(WM_USER + 101)
#define WM_TALK_RECONNECT	(WM_USER + 102)

#define BUFFER_SIZE 16

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
// CDevInitDlg dialog

CDevInitDlg::CDevInitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDevInitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDevInitDlg)
	m_nSearchTime = 3000;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_IsUnicast = 0;
	m_nDeviceCount= 0;
	m_nSelected = -1;
	m_strPwdResetWay = _T("");
	m_pNetCardInfo = new CNetCardInfo;
	//Set vector capability
	m_DevNetInfo.reserve(MAX_DEV_INFO_COUNT); 
}

CDevInitDlg::~CDevInitDlg()
{
	if (m_pNetCardInfo)
	{
		delete m_pNetCardInfo;
		m_pNetCardInfo = NULL;
	}
}

void CDevInitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDevInitDlg)
	DDX_Control(pDX, IDC_LIST_DEVICEINFO, m_ctlListInfo);
	DDX_Text(pDX, IDC_EDIT_SEARCHTIME, m_nSearchTime);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDevInitDlg, CDialog)
	//{{AFX_MSG_MAP(CDevInitDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonSearch)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SEARCHDEVICESBYIPS, OnButtonSearchdevicesbyips)
	ON_MESSAGE(WM_SEARCHDEVICES, OnSearchDevice)
	ON_BN_CLICKED(IDC_BUTTON_INITDEVICE, OnButtonInitDevice)
	ON_BN_CLICKED(IDC_BUTTON_FINDPASSWORD, OnButtonResetPassword)
	ON_MESSAGE(WM_SEARCHDEVICEBYIPS, BeginSearchdevicesbyips)
	ON_BN_CLICKED(IDC_BUTTON_MODIFY_IP, OnButtonModifyIp)
	ON_MESSAGE(WM_TALK_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_TALK_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDevInitDlg message handlers

BOOL CDevInitDlg::OnInitDialog()
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

	InitListView();
	InitNetSDK();
	m_pNetCardInfo->Init();

	GetDlgItem(IDC_BUTTON_INITDEVICE)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON_FINDPASSWORD)->EnableWindow(FALSE); 
	GetDlgItem(IDC_BUTTON_MODIFY_IP)->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDevInitDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDevInitDlg::OnPaint() 
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
HCURSOR CDevInitDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//Callback function when device disconnected 
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CDevInitDlg *pThis = (CDevInitDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_TALK_DISCONNECT, NULL, NULL);		
}

//Callback function when device reconnect 
void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CDevInitDlg *pThis = (CDevInitDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_TALK_RECONNECT, NULL, NULL);		
}

//Process when device disconnected 
LRESULT CDevInitDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CDevInitDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("DevInit"));
	return 0;
}

//Call back of CLIENT_StartSearchDevices  

void CALLBACK cbSearchDevicesEx(LLONG lSearchHandle,DEVICE_NET_INFO_EX2 *pDevNetInfo, void* pUserData)
{
	if(pDevNetInfo != NULL)
	{
		CDevInitDlg *dlg = (CDevInitDlg *)pUserData;
		DEVICE_NET_INFO_EX2 *pData = NEW DEVICE_NET_INFO_EX2;
		memcpy(pData, pDevNetInfo, sizeof(DEVICE_NET_INFO_EX2));
		LONG bIsUnicast = dlg->GetQueryDeviceType();
		dlg->PostMessage(WM_SEARCHDEVICES, (unsigned int)pData, bIsUnicast);
	}
}
void CALLBACK cbSearchDevices(DEVICE_NET_INFO_EX *pDevNetInfo, void* pUserData)
{
	if(pDevNetInfo != NULL)
	{
		CDevInitDlg *dlg = (CDevInitDlg *)pUserData;
		DEVICE_NET_INFO_EX2 *pData = NEW DEVICE_NET_INFO_EX2;
		pData->szLocalIP[0] = '\0';
		pData->stuDevInfo = *pDevNetInfo;
		LONG bIsUnicast = dlg->GetQueryDeviceType();
		dlg->PostMessage(WM_SEARCHDEVICES, (unsigned int)pData, bIsUnicast);
	}
}

void CDevInitDlg::InitListView()
{
	m_ctlListInfo.SetExtendedStyle(m_ctlListInfo.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctlListInfo.InsertColumn(0,ConvertString("No."),LVCFMT_LEFT,40,-1);
	m_ctlListInfo.InsertColumn(1,ConvertString("Status"),LVCFMT_LEFT,80,0);
	m_ctlListInfo.InsertColumn(2,ConvertString("IPVersion"),LVCFMT_LEFT,50,2);
	m_ctlListInfo.InsertColumn(3,ConvertString("IP Address"),LVCFMT_LEFT,150,2);
	m_ctlListInfo.InsertColumn(4,ConvertString("Port"),LVCFMT_LEFT,50,3);
	m_ctlListInfo.InsertColumn(5,ConvertString("Subnet Mask"),LVCFMT_LEFT,110,4);
	m_ctlListInfo.InsertColumn(6,ConvertString("Gateway"),LVCFMT_LEFT,100,5);
	m_ctlListInfo.InsertColumn(7,ConvertString("Mac Address"),LVCFMT_LEFT,120,6);
	m_ctlListInfo.InsertColumn(8,ConvertString("Dev Type"),LVCFMT_LEFT,70,7);
    m_ctlListInfo.InsertColumn(9,ConvertString("DetailType"),LVCFMT_LEFT,70,8);
	m_ctlListInfo.InsertColumn(10,ConvertString("Http"),LVCFMT_LEFT,50,9);
}

void CDevInitDlg::InitNetSDK()
{
	BOOL bRet = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	if (!bRet)
	{
		CString strTmp;
		strTmp.Format("%s:%d", ConvertString("Initialize SDK Failed with Error"), CLIENT_GetLastError() & 0x7fffffff);
		MessageBox(strTmp, ConvertString("Prompt"));
	}
	else
	{
		LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
		CLIENT_LogOpen(&stLogPrintInfo);
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
}

//Get the state of the device initialization  
BOOL CDevInitDlg::GetInitStatus(BYTE initStatus)
{
	int result = initStatus & 3;  
	//Uninitialize
	if (result == 1 )
	{
		return FALSE;       
	}
	//Initialize (include new device that initialize and old device that default initialize )
	else
	{
		return TRUE;       
	}
}

//Add device information to list view
void CDevInitDlg::AddDeviceInfoToListView(DEVICE_NET_INFO_EX2* pDevNetInfo, LONG IsUnicast)
{
    if (m_nDeviceCount >= MAX_DEV_INFO_COUNT)
    {
        return;
    }
	//先进行组播/广播搜索时，在进行单播搜索，此时如果有其他程序一直进行组播/广播搜索，
	//会导致在PostMessage的队列中留有组播/广播设备信息，执行单播搜索的时候，显示列表里会有
	//组播/广播的设备信息显示。
	//因此在此做了个区分本次搜索设备回复信息是组播/广播 还是单播
	if (m_IsUnicast != IsUnicast)
	{
		return;
	}
	//whether vector already have device information,if yes then return   
	for (int i = 0; i < m_nDeviceCount; i++)
	{
		if (0 == strcmp(m_DevNetInfo[i].stuDevInfo.szMac, pDevNetInfo->stuDevInfo.szMac))
		{
			return;
		}
	}
	m_DevNetInfo.push_back(*pDevNetInfo);
	m_nDeviceCount++;

	int nIndex = m_ctlListInfo.GetItemCount();
	CString strNum;
	strNum.Format("%d",nIndex);
	CString strIPiIPVersion;
	strIPiIPVersion.Format("%d",pDevNetInfo->stuDevInfo.iIPVersion);
	CString strIP;
	strIP.Format("%s",pDevNetInfo->stuDevInfo.szIP);
	CString strPort;
	strPort.Format("%d",pDevNetInfo->stuDevInfo.nPort);
	CString strSubnetMask;
	strSubnetMask.Format("%s",pDevNetInfo->stuDevInfo.szSubmask);
	CString strGateWay;
	strGateWay.Format("%s",pDevNetInfo->stuDevInfo.szGateway);
	CString strMacAddress;
	strMacAddress.Format("%s",pDevNetInfo->stuDevInfo.szMac);
	CString strDevType;
	strDevType.Format("%s",pDevNetInfo->stuDevInfo.szDetailType);
	CString strHttp;
	strHttp.Format("%d",pDevNetInfo->stuDevInfo.nHttpPort);
	CString strInitStatus;
	BOOL bRet = GetInitStatus(pDevNetInfo->stuDevInfo.byInitStatus);
	if(FALSE == bRet)
	{
		strInitStatus.Format(ConvertString("Uninitialize"));
		m_ctlListInfo.SetItemColor(nIndex,RGB(255,255,255), RGB(255,0,0));
	}
	else
	{
		strInitStatus.Format(ConvertString("Initialize"));
	}

	m_ctlListInfo.InsertItem(LVIF_TEXT|LVIF_STATE,nIndex,strNum,0,LVIS_SELECTED,0,0);
	m_ctlListInfo.SetItemText(nIndex,1,strInitStatus);                  
	m_ctlListInfo.SetItemText(nIndex,2,strIPiIPVersion);
	m_ctlListInfo.SetItemText(nIndex,3,strIP);
	m_ctlListInfo.SetItemText(nIndex,4,strPort);
	m_ctlListInfo.SetItemText(nIndex,5,strSubnetMask);
	m_ctlListInfo.SetItemText(nIndex,6,strGateWay);
	m_ctlListInfo.SetItemText(nIndex,7,strMacAddress);
	m_ctlListInfo.SetItemText(nIndex,8,pDevNetInfo->stuDevInfo.szDeviceType);
	m_ctlListInfo.SetItemText(nIndex,9,pDevNetInfo->stuDevInfo.szDetailType);
	m_ctlListInfo.SetItemText(nIndex,10,strHttp); 
	
}

void CDevInitDlg::ChangeDevNetIp(CString strIP,int nIPversion,CString strUserName,CString strPassWord,CString strSubnetMask,CString strGateWay)
{
	DEVICE_NET_INFO_EX2 tmp;
	memcpy(&tmp,&m_DevNetInfo[m_nSelected],sizeof(DEVICE_NET_INFO_EX2));
	tmp.stuDevInfo.iIPVersion = nIPversion;
	strncpy(tmp.stuDevInfo.szIP,		strIP,			sizeof(tmp.stuDevInfo.szIP) - 1);
	strncpy(tmp.stuDevInfo.szPassWord, strPassWord,	sizeof(tmp.stuDevInfo.szPassWord) - 1);
	strncpy(tmp.stuDevInfo.szUserName, strUserName,	sizeof(tmp.stuDevInfo.szUserName) - 1);
	strncpy(tmp.stuDevInfo.szGateway,	strGateWay,		sizeof(tmp.stuDevInfo.szGateway) - 1);
	strncpy(tmp.stuDevInfo.szSubmask,	strSubnetMask,	sizeof(tmp.stuDevInfo.szSubmask) - 1);

	int nError = 0;

	//CLIENT_NET_API BOOL CALL_METHOD CLIENT_ModifyDeviceEx(NET_IN_MODIFY_IP* pInBuf, NET_OUT_MODIFY_IP* pOutBuf, int nWaitTime);
	NET_IN_MODIFY_IP stuInparam = {sizeof(stuInparam), tmp};
	NET_OUT_MODIFY_IP stuOutparam = {sizeof(stuOutparam)};

    BOOL bSuccess = CLIENT_ModifyDeviceEx(&stuInparam, &stuOutparam, 5000);
	if(!bSuccess)
	{		
		CString strTmp;
		strTmp.Format("%s:%d", ConvertString("Modify IP Failed with Error"), CLIENT_GetLastError() & 0x7fffffff);
		MessageBox(strTmp, ConvertString("Prompt"));
		return;
	}

	m_DevNetInfo[m_nSelected].stuDevInfo.iIPVersion = nIPversion;
	strncpy(m_DevNetInfo[m_nSelected].stuDevInfo.szIP,			strIP,			sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szIP) - 1);
	strncpy(m_DevNetInfo[m_nSelected].stuDevInfo.szPassWord,	strPassWord,	sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szPassWord) - 1);
	strncpy(m_DevNetInfo[m_nSelected].stuDevInfo.szUserName,	strUserName,	sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szUserName) - 1);
	strncpy(m_DevNetInfo[m_nSelected].stuDevInfo.szGateway,	strGateWay,		sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szGateway) - 1);
	strncpy(m_DevNetInfo[m_nSelected].stuDevInfo.szSubmask,	strSubnetMask,	sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szSubmask) - 1);
}

LONG CDevInitDlg::GetQueryDeviceType() const
{
	return m_IsUnicast;
}

void CDevInitDlg::OnButtonSearch() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON_INITDEVICE)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON_FINDPASSWORD)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON_MODIFY_IP)->EnableWindow(TRUE);

	
	//Delete list view items
	m_ctlListInfo.DeleteAllItems();
    //Clear list view color
    m_ctlListInfo.ClearColor();
	//Clear vector
	m_DevNetInfo.clear();              
    m_nDeviceCount = 0;

	//搜索前，停止前一次搜索。因为SDK搜索机制是每次搜索引用计数加1，所以当多次搜索时，如果未停止多次搜索，会导致组播搜索还会一直往上发。
	
	StopSearchDevice();
	m_IsUnicast = 0;
	
	m_pNetCardInfo->Init();	//重新初始化网卡列表
	std::set<std::string> IPVector;
	m_pNetCardInfo->GetNetCardIp(IPVector);

	for (std::set<std::string>::iterator it = IPVector.begin(); it != IPVector.end(); ++it)
	{
		std::string strLocalIP = *it;

		NET_IN_STARTSERACH_DEVICE pInBuf = { 0 };
		NET_OUT_STARTSERACH_DEVICE pOutBuf = { 0 };
		LLONG seachHandle = 0;

		pInBuf.dwSize = sizeof(NET_IN_STARTSERACH_DEVICE);
		pInBuf.cbSearchDevices = cbSearchDevicesEx;
		pInBuf.pUserData = this;
		int nMaxCopyLen = MAX_LOCAL_IP_LEN - 1;

		strncpy(pInBuf.szLocalIp, strLocalIP.c_str(), sizeof(pInBuf.szLocalIp) - 1);
		pOutBuf.dwSize = sizeof(NET_OUT_STARTSERACH_DEVICE);

		seachHandle = CLIENT_StartSearchDevicesEx(&pInBuf, &pOutBuf);
		if (seachHandle)
		{
			m_searchVecotr.push_back(seachHandle);
		}

		/*if(!seachHandle)
		{
			MessageBox(ConvertString("Search by multicast and broadcast failed"),ConvertString("Prompt"));
			return;
		}
		m_searchVecotr.push_back(seachHandle);*/
	}
}


void CDevInitDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	StopSearchDevice();
	
    m_ctlListInfo.DeleteAllItems();   
    m_ctlListInfo.ClearColor();       
	m_DevNetInfo.clear();

	if (m_pNetCardInfo)
	{
		delete m_pNetCardInfo;
		m_pNetCardInfo = NULL;
	}
	CLIENT_Cleanup();
	// TODO: Add your message handler code here
}

void CDevInitDlg::OnButtonSearchdevicesbyips() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_BUTTON_INITDEVICE)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON_FINDPASSWORD)->EnableWindow(TRUE); 
	GetDlgItem(IDC_BUTTON_MODIFY_IP)->EnableWindow(TRUE);

	UpdateData(TRUE);
	GetIPDlg dlg;
	dlg.DoModal();
}
afx_msg LRESULT CDevInitDlg::BeginSearchdevicesbyips(WPARAM dwStartIP, LPARAM dwEndIP)
{
	//搜索前，停止前一次搜索。
	StopSearchDevice();


	m_pNetCardInfo->Init();	//重新初始化网卡列表

	m_ctlListInfo.DeleteAllItems();   
    m_ctlListInfo.ClearColor();         
	m_DevNetInfo.clear();
    m_nDeviceCount = 0;
	DEVICE_IP_SEARCH_INFO DevIpSearchInfo = {0};
	char buf[BUFFER_SIZE] = {0};
	DevIpSearchInfo.dwSize = sizeof(DEVICE_IP_SEARCH_INFO);
	if(dwEndIP < dwStartIP)
	{
		MessageBox(ConvertString("Illegal IP format"), ConvertString("Prompt"));
		return 0;
	}
	if(dwEndIP - dwStartIP + 1 > 256)
	{
		MessageBox(ConvertString("IP amount exceed 256"), ConvertString("Prompt"));
		return 0;
	}
	DevIpSearchInfo.nIpNum = dwEndIP - dwStartIP + 1;
	DWORD dwIPs = dwStartIP;
	for(int i = 0 ;i < DevIpSearchInfo.nIpNum; i ++)
	{
		IPtoStr(dwIPs, buf, BUFFER_SIZE);
		strncpy(DevIpSearchInfo.szIP[i], buf, sizeof(DevIpSearchInfo.szIP[i]) - 1);
		dwIPs++;
	}
	m_IsUnicast = 1;
	BOOL bret = CLIENT_SearchDevicesByIPs(&DevIpSearchInfo, cbSearchDevices, (DWORD)this, NULL, m_nSearchTime);
	if(!bret)
	{
		MessageBox(ConvertString("Search by point to point failed"), ConvertString("Prompt"));
	}
	return 1;
}


LRESULT CDevInitDlg::OnSearchDevice(WPARAM w, LPARAM l)
{
	if (w != NULL)
	{
		DEVICE_NET_INFO_EX2 *pData = (DEVICE_NET_INFO_EX2*)w;
		if (pData)
		{
			AddDeviceInfoToListView(pData,(LONG)l);
			if (pData)
			{
				delete pData;
			}
		}
	}

	return S_OK;
};


void CDevInitDlg::IPtoStr(DWORD ip, char* buf, unsigned int nBufferSize)
{
	ZeroMemory(buf,nBufferSize);
	WORD add1,add2,add3,add4;
	add1 = (WORD)(ip&255);
	add2 = (WORD)((ip>>8)&255);
	add3 = (WORD)((ip>>16)&255);
	add4 = (WORD)((ip>>24)&255);
	sprintf(buf,"%d.%d.%d.%d",add4,add3,add2,add1);
}

void CDevInitDlg::GetPwdRestWay(BYTE pwdRestWay)
{
	if(1 == (pwdRestWay & 1))
    {
        m_strPwdResetWay.Format(ConvertString("Cell Phone"));         
    }
    else if(1 == (pwdRestWay>>1 & 1))
    {
        m_strPwdResetWay.Format(ConvertString("Mail Box"));         
    }    
}


//Initialize device
void CDevInitDlg::OnButtonInitDevice() 
{
	// TODO: Add your control notification handler code here
	m_nSelected = m_ctlListInfo.GetNextItem(-1,LVNI_SELECTED);
	if( -1 == m_nSelected)
	{
		MessageBox(ConvertString("Please select device to initialize"), ConvertString("Prompt"));
		return;
	}

	BOOL  bRet=GetInitStatus(m_DevNetInfo[m_nSelected].stuDevInfo.byInitStatus);
	if( FALSE != bRet)
	{
		MessageBox(ConvertString("Please select uninitialized device"), ConvertString("Prompt"));
		return;
	}

	GetPwdRestWay(m_DevNetInfo[m_nSelected].stuDevInfo.byPwdResetWay);
    CInitDeviceDlg dlg;
	dlg.m_strPwdRestWay=m_strPwdResetWay;

    int nRet = dlg.DoModal();	
    if ( IDOK != nRet)
    {
        return;
    }

	char* szUserName = dlg.m_strUserName.GetBuffer(0);
	char* szPwd = dlg.m_strPwd.GetBuffer(0);

    NET_IN_INIT_DEVICE_ACCOUNT sInitAccountIn = {0};
	NET_OUT_INIT_DEVICE_ACCOUNT sInitAccountOut = {0};
	sInitAccountIn.dwSize = sizeof(sInitAccountIn);
	sInitAccountOut.dwSize = sizeof(sInitAccountOut);
	DWORD dwWaitTime = 5000;

	//password reset way
    sInitAccountIn.byPwdResetWay = m_DevNetInfo[m_nSelected].stuDevInfo.byPwdResetWay;        
	strncpy(sInitAccountIn.szMac,		m_DevNetInfo[m_nSelected].stuDevInfo.szMac, sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szMac) - 1);                         
	strncpy(sInitAccountIn.szUserName,	szUserName, sizeof(sInitAccountIn.szUserName) - 1);          
	strncpy(sInitAccountIn.szPwd,		szPwd, sizeof(sInitAccountIn.szPwd) - 1);                              

	dlg.m_strUserName.ReleaseBuffer();
	dlg.m_strPwd.ReleaseBuffer();

	char* szRig = dlg.m_strRig.GetBuffer(0);	
	//Save cell phone and mail box
    if (1 == (m_DevNetInfo[m_nSelected].stuDevInfo.byPwdResetWay & 1))
    {
        // cell phone
        strncpy(sInitAccountIn.szCellPhone, szRig, sizeof(sInitAccountIn.szCellPhone) - 1); 
    }
    else if(1 ==(m_DevNetInfo[m_nSelected].stuDevInfo.byPwdResetWay>>1 & 1))
    {
        // email
        strncpy(sInitAccountIn.szMail, szRig, sizeof(sInitAccountIn.szMail) - 1);                   
    }
    dlg.m_strRig.ReleaseBuffer();

	//Initialize device
	nRet = CLIENT_InitDevAccount(&sInitAccountIn, &sInitAccountOut, dwWaitTime,  m_DevNetInfo[m_nSelected].szLocalIP);		
	if( FALSE == nRet)
	{
		CString strTmp;
		strTmp.Format("%s:%d", ConvertString("Initialize Failed with Error"), CLIENT_GetLastError() & 0x7fffffff);
		MessageBox(strTmp, ConvertString("Prompt"));
		return;
	}
	MessageBox(ConvertString("Initialize Success"), ConvertString("Prompt"));
	//Modify initialize information 
	m_DevNetInfo[m_nSelected].stuDevInfo.byInitStatus = 2;   
	CString strInitStatus;
	strInitStatus.Format(ConvertString("Initialize"));
	//Update list view
	m_ctlListInfo.SetItemText(m_nSelected,1,strInitStatus);       
    m_ctlListInfo.SetItemColor(m_nSelected,RGB(0,0,0), RGB(255,255,255));
}

//Find password 
void CDevInitDlg::OnButtonResetPassword() 
{
	// TODO: Add your control notification handler code here
	m_nSelected = m_ctlListInfo.GetNextItem(-1,LVNI_SELECTED);
	if( -1 == m_nSelected)
	{
		CString strRet;
		strRet.Format(ConvertString("Please select initialized device"));
		MessageBox(strRet,  ConvertString("Prompt"));
		return;
	}
	
    NET_IN_DESCRIPTION_FOR_RESET_PWD DescriptionIn = {0}; 
    NET_OUT_DESCRIPTION_FOR_RESET_PWD DescriptionOut = {0}; 
	DescriptionIn.dwSize = sizeof(DescriptionIn);
	DescriptionOut.dwSize = sizeof(DescriptionOut);

	DWORD dwWaitTime = 2000;
	strncpy(DescriptionIn.szMac, m_DevNetInfo[m_nSelected].stuDevInfo.szMac, sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szMac) - 1);                                      
	strncpy(DescriptionIn.szUserName, m_DevNetInfo[m_nSelected].stuDevInfo.szUserName, sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szUserName) - 1);   
	DescriptionIn.byInitStatus = 1;        
	DescriptionOut.nQrCodeLen = MAX_QRCODE_LEN;
	DescriptionOut.pQrCode = NEW char[DescriptionOut.nQrCodeLen];
	memset(DescriptionOut.pQrCode, 0, sizeof(char)*DescriptionOut.nQrCodeLen);

	//Get reset information:cell phone,mail box and Qrcode 
	int nRet = CLIENT_GetDescriptionForResetPwd(&DescriptionIn, &DescriptionOut, dwWaitTime, m_DevNetInfo[m_nSelected].szLocalIP);	
	if (0 == nRet)
	{
		delete[] DescriptionOut.pQrCode;
		DescriptionOut.pQrCode = NULL;
		CString strTmp;
		strTmp.Format("%s:%d", ConvertString("Get Description For Reset Password Failed with Error"), CLIENT_GetLastError() & 0x7fffffff);
		MessageBox(strTmp, ConvertString("Prompt"));
		return;
	}

	CResetPassword dlg;
    
    strncpy(dlg.m_cQrCode, DescriptionOut.pQrCode, sizeof(dlg.m_cQrCode)-1);
    delete [] DescriptionOut.pQrCode;
    DescriptionOut.pQrCode = NULL;

	// support Telephone
	if (1 == (m_DevNetInfo[m_nSelected].stuDevInfo.byPwdResetWay & 1))
	{		
		dlg.SetQRCodeScanDescription(false);				
	}
	if (1 == ((m_DevNetInfo[m_nSelected].stuDevInfo.byPwdResetWay >> 1) & 1)) // support email
	{
		dlg.SetQRCodeScanDescription(true);
	}	

	dlg.SetResetPwdParam(m_DevNetInfo[m_nSelected].stuDevInfo.szMac, sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szMac), m_DevNetInfo[m_nSelected].stuDevInfo.szUserName, sizeof(m_DevNetInfo[m_nSelected].stuDevInfo.szUserName), m_DevNetInfo[m_nSelected].stuDevInfo.byInitStatus);
	dlg.DoModal();	
}

void CDevInitDlg::OnButtonModifyIp() 
{
	// TODO: Add your control notification handler code here
	m_nSelected = m_ctlListInfo.GetNextItem(-1,LVNI_SELECTED);
	if(m_nSelected == -1)
		return;

	StopSearchDevice();
	
	ModifyIPDlg dlg;
	dlg.m_IP.Format("%s",m_DevNetInfo[m_nSelected].stuDevInfo.szIP);
	dlg.m_IpVersion = m_DevNetInfo[m_nSelected].stuDevInfo.iIPVersion;
	dlg.m_strSubnetMask.Format("%s",m_DevNetInfo[m_nSelected].stuDevInfo.szSubmask);
	dlg.m_strGateWay.Format("%s",m_DevNetInfo[m_nSelected].stuDevInfo.szGateway);
	dlg.DoModal();
}

BOOL CDevInitDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CDevInitDlg::StopSearchDevice()
{
	for (int i = 0; i < m_searchVecotr.size(); i++)
	{
		if (0 != m_searchVecotr[i])
		{
			BOOL bSuccess = CLIENT_StopSearchDevices(m_searchVecotr[i]);
			if(!bSuccess)
			{
				MessageBox(ConvertString("Stop search failed!"));
			}
			else
			{
				m_searchVecotr[i] = 0;
			}
		}
	}
	m_searchVecotr.clear();
}