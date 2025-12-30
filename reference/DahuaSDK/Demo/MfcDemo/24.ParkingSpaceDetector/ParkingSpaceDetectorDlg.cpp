// IntelligentTrafficDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ParkingSpaceDetector.h"
#include "ParkingSpaceDetectorDlg.h"
#include "ParkingspacelightgroupDlg.h"
#include "ParkingspacelightstateDlg.h"
#include "Controller.h"
#include "Device.h"
#include "Alaram.h"
#include <iostream>
#include <assert.h>
#include <math.h>

using namespace std;

static const int MAX_EVENT_TO_DISPLAY = 100 ;
static const int MAX_EVENT_STORE_IN_MAP = 200;
static const int HIMETRIC_INCH = 2540;

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
// CParkingSpaceDetectorDlg dialog

CParkingSpaceDetectorDlg::CParkingSpaceDetectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParkingSpaceDetectorDlg::IDD, pParent)
	,m_nEventCount(0)
{	
	//{{AFX_DATA_INIT(CParkingSpaceDetectorDlg)

	m_pCtl = new CController(this);
	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	
}

CParkingSpaceDetectorDlg::~CParkingSpaceDetectorDlg()
{	
	if (m_pCtl)
	{
		delete m_pCtl;
		m_pCtl = NULL;
	}
}
void CParkingSpaceDetectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParkingSpaceDetectorDlg)
	DDX_Control(pDX, IDC_LISTCTRL_REAL_EVENT, m_lcRealEvent);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_cbChannel);
	//DDX_Control(pDX, IDC_STATIC_EVENT_LP_IMAGE, m_ctrlPlateImage);
	//}}AFX_DATA_MAP	
}

BEGIN_MESSAGE_MAP(CParkingSpaceDetectorDlg, CDialog)
	//{{AFX_MSG_MAP(CParkingSpaceDetectorDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, OnButtonLogout)
	ON_BN_CLICKED(IDC_BUTTON_START_PREVIEW, OnButtonStartPreview)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PREVIEW, OnButtonStopPreview)
	ON_BN_CLICKED(IDC_BUTTON_START, OnButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_REAL_EVENT, OnNMClickListctrlRealEvent)
	ON_WM_DESTROY()	
	ON_NOTIFY(LVN_KEYDOWN, IDC_LISTCTRL_REAL_EVENT, OnKeydownListctrlRealEvent)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_USER_EVENT_COME, OnEventCome)
	ON_WM_TIMER()		
	ON_WM_CLOSE()
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP		
	ON_COMMAND(ID_SET_PARKINGSPACELIGHTGROUP, &CParkingSpaceDetectorDlg::OnSetParkingspacelightgroup)
	ON_COMMAND(ID_SET_PARKINGSPACELIGHTSTATE, &CParkingSpaceDetectorDlg::OnSetParkingspacelightstate)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CParkingSpaceDetectorDlg::OnBnClickedButtonClear)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParkingSpaceDetectorDlg message handlers

BOOL CParkingSpaceDetectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);
	
	InitCtrl();	
	InitLoadPicturePath();
	InitPictureWnd();
	InitMenu();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CParkingSpaceDetectorDlg::InitLoadPicturePath()
{
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
}

void CParkingSpaceDetectorDlg::InitPictureWnd()
{
	m_cPictureWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1981);
	CRect screenRect;
	GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(&screenRect);
	ScreenToClient(&screenRect);
	m_cPictureWnd.MoveWindow(screenRect);
	m_cPictureWnd.ShowWindow(SW_SHOW);
}

void CParkingSpaceDetectorDlg::InitMenu()
{
	// Add "About..." menu item to system menu.
	m_Menu.LoadMenu(IDR_MENU1);
	m_Menu.ModifyMenu(0,MF_BYPOSITION,ID_SET_SET1,ConvertString("LightSet"));
	m_Menu.GetSubMenu(0)->ModifyMenu(0,MF_BYPOSITION,ID_SET_PARKINGSPACELIGHTGROUP,ConvertString("ParkingSpaceLightGroup"));
	m_Menu.GetSubMenu(0)->ModifyMenu(1,MF_BYPOSITION,ID_SET_PARKINGSPACELIGHTSTATE,ConvertString("ParkingSpaceLightState"));
	m_Menu.GetSubMenu(0)->ModifyMenu(2,MF_BYPOSITION,ID_SET_PARKINGSPACELIGHTPLAN,ConvertString("ParkingSpaceLightPlan"));
	SetMenu(&m_Menu);
}

void CParkingSpaceDetectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CParkingSpaceDetectorDlg::OnPaint() 
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
HCURSOR CParkingSpaceDetectorDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CParkingSpaceDetectorDlg::OnButtonLogin() 
{
	CString strIp;
	CString strUser;
	CString strPwd;
	int		nPort;

	GetDlgItemText(IDC_IPADDRESS_IP, strIp);
	GetDlgItemText(IDC_EDIT_USERNAME, strUser);
	GetDlgItemText(IDC_EDIT_PASSWORD, strPwd);
	nPort = GetDlgItemInt(IDC_EDIT_PORT);

	stuDeviceInfo stuDevInfo;
	strncpy(stuDevInfo.szIp, strIp, sizeof(stuDevInfo.szIp)-1);
	strncpy(stuDevInfo.szPasswd, strPwd, sizeof(stuDevInfo.szPasswd)-1);
	strncpy(stuDevInfo.szUserName, strUser, sizeof(stuDevInfo.szUserName)-1);
	stuDevInfo.nPort = nPort;

	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}
	if (m_pCtl->Login(stuDevInfo) == false)
	{
		MessageBox(ConvertString("Login failed!"),ConvertString("Prompt"));
		return;
	}

	GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(TRUE);

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_CLEAR)->EnableWindow(TRUE);

	UpdateChannelCtrl(TRUE);
	SetWindowText(ConvertString("ParkingSpaceDetector"));
}

void CParkingSpaceDetectorDlg::OnButtonLogout() 
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}
	
	OnButtonStopPreview();
	OnButtonStop();	

	ClearCount();
	UpdateChannelCtrl(FALSE);
	
	//µÇ³ö
	m_pCtl->LogOut();

	GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(FALSE);

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);	

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);		

	GetDlgItem(IDC_BUTTON_CLEAR)->EnableWindow(FALSE);

	SetWindowText(ConvertString("ParkingSpaceDetector"));	
}

void CParkingSpaceDetectorDlg::OnClose()
{	
	CDialog::OnClose();
}

BOOL CParkingSpaceDetectorDlg::PreTranslateMessage(MSG* pMsg) 
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


void CParkingSpaceDetectorDlg::UpdateChannelCtrl(BOOL bLogin)
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}
	int nChannelNum = m_pCtl->GetChannelNum();

	if (bLogin)
	{		
		int i= 0;
		for (i = 0; i < nChannelNum; i++)
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
		while (m_cbChannel.GetCount() > 0)
		{
			m_cbChannel.DeleteString(0);
		}
		m_cbChannel.Clear();				
		this->ChangeUI(UI_STATE_SUBSCRIBE_STOP);	
	}
}

void CParkingSpaceDetectorDlg::OnButtonStartPreview() 
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}

	HWND hWnd = GetDlgItem(IDC_STATIC_PREVIEW)->GetSafeHwnd();
	int nSel = m_cbChannel.GetCurSel();
	if ( (nSel < 0) || (m_pCtl->isOnline() == false) )
	{
		MessageBox(ConvertString("Please login and select a channel before preview!"),ConvertString("Prompt"));
		return;
	}
	//Open the preview
	int nChannelID = nSel;

	if(m_pCtl->StartRealPlay(nChannelID, hWnd) == false)
	{
		MessageBox(ConvertString("Fail to play!"), ConvertString("Prompt"));
		return ;
	}

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(TRUE);	
}

void CParkingSpaceDetectorDlg::OnButtonStopPreview() 
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}
	m_pCtl->StopRealPlay();


	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PREVIEW));

	GetDlgItem(IDC_BUTTON_START_PREVIEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP_PREVIEW)->EnableWindow(FALSE);
}

LRESULT CParkingSpaceDetectorDlg::OnEventCome(WPARAM wParam, LPARAM lParam)
{
	CAlaram* pAlarm = (CAlaram*)wParam;

	if (pAlarm == NULL)
	{
		return -1;
	}

	StuEventInfo stuEventInfo;
	pAlarm->GetEventInfo(stuEventInfo);

	std::string strGuid = pAlarm->GetGuid();

	//save picture to local
	std::string strCfgFolder = GetDataFolder();
	std::string strCfgFile = strCfgFolder + std::string(".ini");
	std::string strJpgFile = strCfgFolder + strGuid + std::string(".jpg");	
	::CreateDirectory(strCfgFolder.c_str(), NULL);	

	// Store event info
	this->StorePicture(strGuid.c_str(), strCfgFile.c_str(), strJpgFile.c_str(), pAlarm);

	// Update ListCtrl Info

	UpdateListCtrlInfo(strGuid.c_str(), stuEventInfo);	

	// Display event image and license plate image
	DisplayImage(strGuid.c_str(), strJpgFile.c_str());	

	delete pAlarm;
	pAlarm = NULL;
	return 0;
}

BOOL CParkingSpaceDetectorDlg::UpdateListCtrlInfo(const char* szGUID, StuEventInfo& stuEventInfo)
{			
	m_nEventCount++;
	int nItemCount = m_lcRealEvent.GetItemCount();
	if (nItemCount >= MAX_EVENT_TO_DISPLAY)
	{

		char* pData = (char*)m_lcRealEvent.GetItemData(MAX_EVENT_TO_DISPLAY - 1);
		delete[] pData;
		pData = NULL;

		m_lcRealEvent.DeleteItem(MAX_EVENT_TO_DISPLAY - 1);	
	}	
	
	char* szGUIDTmp = new char[64];
	if (NULL == szGUIDTmp)
	{
		return FALSE;
	}

	memset(szGUIDTmp, 0, 64);
	strncpy(szGUIDTmp, szGUID, 63);

	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = "";
	lvi.iImage = 0;
	lvi.iItem = 0;
	m_lcRealEvent.InsertItem(&lvi);	

	m_lcRealEvent.SetItemData(0, (LDWORD)(szGUIDTmp) );

	//Display data			
	stuEventInfo.stuEventInfoToDisplay.strCountNum = Int2Str(m_nEventCount);	// update the count number of event list ctrl

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
	m_lcRealEvent.SetItemText(0, 13, stuEventInfo.strDeviceAddress.c_str());

	return TRUE;
}

BOOL CParkingSpaceDetectorDlg::DisplayImage(const char* szGUID , const char* szJpgFilePath)
{
	DisplayEventImage(szJpgFilePath);
	return TRUE;
}


BOOL CParkingSpaceDetectorDlg::DisplayEventImage(const char* szJPGPath)
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


BOOL CParkingSpaceDetectorDlg::StorePicture(const char* szGUID, const char* szCfgFilePath, const char* szJpgFilePath, CAlaram* pAlarm)
{
	int nStructSize = 0;
	DWORD dwPicSize = 0;
	BYTE* pBuf = pAlarm->GetPictureInfo(dwPicSize);
	
	if (NULL == pBuf || dwPicSize == 0)
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
	
	fwrite(pBuf, dwPicSize, 1, fp);
	fclose(fp);
	return TRUE;
}

void CParkingSpaceDetectorDlg::OnButtonStart() 
{	
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}

	if (m_pCtl->isOnline() == false)
	{
		MessageBox(ConvertString("Please login first!"),ConvertString("Prompt"));
		return;
	}
	
	bool bSuccess = false;
	int nSel = m_cbChannel.GetCurSel();
	if (nSel < 0)
	{
		MessageBox(ConvertString("Please select a channel"),ConvertString("Prompt"));
		return;
	}

	if (m_pCtl->StartRealLoadPicture(nSel) == false)
	{
		MessageBox(ConvertString("Subscribe picture failed!"), ConvertString("Prompt"));
		return;
	}

	bSuccess = true;
	//Change UI
	if (bSuccess)
	{
		this->ChangeUI(UI_STATE_SUBSCRIBE_NOW);
	}
}


void CParkingSpaceDetectorDlg::OnButtonStop() 
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}
	m_pCtl->StopRealLoadPicture();
	
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PICTURE));

	this->ChangeUI(UI_STATE_SUBSCRIBE_STOP);
}

void CParkingSpaceDetectorDlg::OnNMClickListctrlRealEvent(NMHDR *pNMHDR, LRESULT *pResult)
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

void CParkingSpaceDetectorDlg::OnDestroy()
{	
	CDialog::OnDestroy();

	ClearInfoInEventCtrl();
}

void CParkingSpaceDetectorDlg::ChangeUI( int nState )
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



void CParkingSpaceDetectorDlg::ClearCount() 
{
	ClearInfoInEventCtrl();	
//	ClearRecordData();

	m_nEventCount = 0;
}

void CParkingSpaceDetectorDlg::OnKeydownListctrlRealEvent(NMHDR* pNMHDR, LRESULT* pResult) 
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


void CParkingSpaceDetectorDlg::DisplaySelectedPicInfo(int nItemIndex, CListCtrl* listCtrl)
{	
	if ( (nItemIndex < 0) ||(nItemIndex >= listCtrl->GetItemCount()))
	{
		return;
	}
	
	std::string strDataFolder = GetDataFolder();	

	char *pGuid = (char*)listCtrl->GetItemData(nItemIndex);
	strDataFolder.append(pGuid);
	
	std::string strJpgFilePath = strDataFolder.append(".jpg");	
	
	DisplayImage(pGuid, strJpgFilePath.c_str());
}


HBRUSH CParkingSpaceDetectorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);	
	
	return hbr;
}

//Process when device disconnected 
LRESULT CParkingSpaceDetectorDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CParkingSpaceDetectorDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("ParkingSpaceDetector"));
	return 0;
}

void CParkingSpaceDetectorDlg::InitCtrl(void)
{
	SetDlgItemText(IDC_IPADDRESS_IP, "172.24.0.54");
	SetDlgItemInt(IDC_EDIT_PORT, 37777);
	SetDlgItemText(IDC_EDIT_USERNAME, "admin");
	SetDlgItemText(IDC_EDIT_PASSWORD, "admin123");

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

void CParkingSpaceDetectorDlg::ClearInfoInEventCtrl()
{
	int nItemCount = m_lcRealEvent.GetItemCount();
	for (int i = 0 ; i < nItemCount; i++)
	{
		char* pData = (char*)m_lcRealEvent.GetItemData(i);
		delete[] pData;
	}
	m_lcRealEvent.DeleteAllItems();
}

void CParkingSpaceDetectorDlg::SetPictureStorePath(void)
{
	string strCfgFilePath;
	string strCfgDefaultFolder;
	GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append(CFG_CFG_FILE_NAME);
	strCfgDefaultFolder.append(PC_DATA_STORE_FOLDER);		
	
	::WritePrivateProfileString(CFG_SECTION_CFG, CFG_KEY_DATA_PATH, strCfgDefaultFolder.c_str(), strCfgFilePath.c_str());
}

void CParkingSpaceDetectorDlg::FillCWndWithDefaultColor(CWnd* cwnd) 
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

void CParkingSpaceDetectorDlg::OnSetParkingspacelightgroup()
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}

	if (m_pCtl->isOnline())
	{
		ParkingspacelightgroupDlg dlg(m_pCtl);
		dlg.DoModal();
	}
	else
	{
		MessageBox(ConvertString("Please Login First!"), "");
	}
}

void CParkingSpaceDetectorDlg::OnSetParkingspacelightstate()
{
	if (m_pCtl == NULL)
	{
		MessageBox(ConvertString("Device init failed"),ConvertString("Prompt"));
		return;
	}
	if (m_pCtl->isOnline())
	{
		ParkingspacelightstateDlg dlg(m_pCtl);
		dlg.DoModal();
	}
	else
	{
		MessageBox(ConvertString("Please Login First!"), "");
	}
}

void CParkingSpaceDetectorDlg::OnBnClickedButtonClear()
{
	ClearInfoInEventCtrl();	

	m_nEventCount = 0;
	SetDlgItemInt(IDC_STATIC_ALARM_COUNT, m_nEventCount, FALSE);
}


void CParkingSpaceDetectorDlg::Update(int nType, void* pMsg)
{
	HWND hWnd = this->GetSafeHwnd();
	EM_NOTIFY_TYPE emNotifyType = (EM_NOTIFY_TYPE)nType;
	switch(emNotifyType)
	{
	case EM_NOTIFY_DISCONNECT:	//update dialog interface for DisConnection
		::PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
		break;
	case EM_NOTIFY_RECONNECT:	//update dialog interface for ReConnection
		::PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);
		break;
	case EM_NOTIFY_LOADPICTURE:	//update dialog interface for RealLoadPicture
		if (!::PostMessage(hWnd, WM_USER_EVENT_COME, (WPARAM)pMsg, 0))
		{
			if (pMsg)
			{
				CAlaram* pAlarm = (CAlaram*)pMsg;
				delete pAlarm;
				pAlarm = NULL;
			}
		}
		break;
	default:
		break;
	}
}