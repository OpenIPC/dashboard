// HeatMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HeatMap.h"
#include "HeatMapDlg.h"
#include "Bmp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Self-define message: device disconnect
#define WM_DEVICE_DISCONNECT         (WM_USER + 1)   

// Device ip
#define DEVICE_IP           "172.23.8.94"     
// Connect port  
#define CONNECT_PORT        37777            
// Connect user name  
#define CONNECT_USERNAME    "admin"          
// Connect password
#define CONNECT_PASSWORD    "admin111"       
// HeatMap size 
#define HEAT_MAP_SIZE        1024*1024*2
// File path size
#define FILE_PATH_SIZE       120
// Max Timeout
#define MAX_TIMEOUT          3000

// HeatMap picture name
#define HEAD_MAP_PCITURE_NAME "HeatMap.bmp"

// device disconnect callback
void CALLBACK DisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    if(0 != dwUser)
    {
        PostMessage(((CHeatMapDlg *)dwUser)->GetSafeHwnd(), WM_DEVICE_DISCONNECT, 0, 0);
    }
}

//Handle message : WM_DEVICE_DISCONNECT
LRESULT CHeatMapDlg::OnDeviceDisConnect(WPARAM wParam, LPARAM lParam)
{
    MessageBox(ConvertString("Device disconnect!"), ConvertString("Prompt"));
    if (0 != m_lLoginHandle)
    {
        OnBnClickedBtnLoginAndLogout();
    }
    return 0;
}


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


// CHeatMapDlg dialog


CHeatMapDlg::CHeatMapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHeatMapDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_lLoginHandle = 0;
    m_lRealPlayHandle = 0;
}

void CHeatMapDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_PICTURE, m_ctrPicture);
}

BEGIN_MESSAGE_MAP(CHeatMapDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_LOGIN_AND_LOGOUT, &CHeatMapDlg::OnBnClickedBtnLoginAndLogout)
    ON_BN_CLICKED(IDC_BTN_START_PLAY_AND_STOP_PLAY, &CHeatMapDlg::OnBnClickedBtnStartPlayAndStopPlay)
    ON_BN_CLICKED(IDC_BTN_QUERY_HEAT_MAP, &CHeatMapDlg::OnBnClickedBtnQueryHeatMap)
    ON_MESSAGE(WM_DEVICE_DISCONNECT,&CHeatMapDlg::OnDeviceDisConnect)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


// CHeatMapDlg message handlers

BOOL CHeatMapDlg::PreTranslateMessage(MSG* pMsg) 
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

BOOL CHeatMapDlg::OnInitDialog()
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
	InitDialogContorl();
    InitNetSdk();
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CHeatMapDlg::OnDestroy()
{
    if (0 != m_lLoginHandle)
    {
        // Logout device
        OnBnClickedBtnLoginAndLogout();
    }
    CLIENT_Cleanup();
    CDialog::OnDestroy();
}


void CHeatMapDlg::InitDialogContorl(void)
{
    SetDlgItemText(IDC_IPADDRESS1, DEVICE_IP);
    SetDlgItemInt(IDC_EDIT_PORT, CONNECT_PORT);
    SetDlgItemText(IDC_EDIT_USER, CONNECT_USERNAME);
    SetDlgItemText(IDC_EDIT_PWD, CONNECT_PASSWORD);

    // Set time
    CDateTimeCtrl* pBeginShortDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_BEGIN_SHORT_DATE);
    CDateTimeCtrl* pBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_BEGIN_TIME);
    CDateTimeCtrl* pEndShortDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_SHORT_DATE);
    CDateTimeCtrl* pEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);

    COleDateTime currentTime(COleDateTime::GetCurrentTime());
    pBeginShortDate->SetTime(currentTime);
    pBeginTime->SetTime(currentTime);
    pEndShortDate->SetTime(currentTime);
    pEndTime->SetTime(currentTime);


    CButton* pBtnPlay =  (CButton*)GetDlgItem(IDC_BTN_START_PLAY_AND_STOP_PLAY);
    CButton* pBtnQueryHeadMap =  (CButton*)GetDlgItem(IDC_BTN_QUERY_HEAT_MAP);
    pBtnPlay->EnableWindow(FALSE);
    pBtnQueryHeadMap->EnableWindow(FALSE);
}


void CHeatMapDlg::InitNetSdk(void)
{
    CLIENT_Init(DisConnect, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
}



void CHeatMapDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CHeatMapDlg::OnPaint()
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
HCURSOR CHeatMapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHeatMapDlg::OnBnClickedBtnLoginAndLogout()
{
    CButton* pBtnLoginAndLogout =  (CButton*)GetDlgItem(IDC_BTN_LOGIN_AND_LOGOUT);
    CComboBox* pCmbChannel =  (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL);
    CButton* pBtnPlay =  (CButton*)GetDlgItem(IDC_BTN_START_PLAY_AND_STOP_PLAY);

    if (0 != m_lLoginHandle )
    {
        if (0 != m_lRealPlayHandle)
        {
            OnBnClickedBtnStartPlayAndStopPlay();
        }

        CLIENT_Logout(m_lLoginHandle);
        m_lLoginHandle = 0;
        pBtnLoginAndLogout->SetWindowText(ConvertString("Login"));
        pCmbChannel->ResetContent();
        pBtnPlay->EnableWindow(FALSE);
    }
    else
    {
        CString csIP = "";
        CString csUserName = "";
        CString csPwd = "";
        int nPort = 0; 
        GetDlgItemText(IDC_IPADDRESS1, csIP);
        GetDlgItemText(IDC_EDIT_USER, csUserName);
        GetDlgItemText(IDC_EDIT_PWD, csPwd);
        nPort = GetDlgItemInt(IDC_EDIT_PORT);

		NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
		memset(&stInparam, 0, sizeof(stInparam));
		stInparam.dwSize = sizeof(stInparam);
		strncpy(stInparam.szIP, csIP.GetBuffer(), sizeof(stInparam.szIP) - 1);
		strncpy(stInparam.szPassword, csPwd.GetBuffer(), sizeof(stInparam.szPassword) - 1);
		strncpy(stInparam.szUserName, csUserName.GetBuffer(), sizeof(stInparam.szUserName) - 1);
		stInparam.nPort = nPort;
		stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

		NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
		memset(&stOutparam, 0, sizeof(stOutparam));
		stOutparam.dwSize = sizeof(stOutparam);
		m_lLoginHandle = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
   
        if (0 != m_lLoginHandle)
        {
            for (int i = 0 ;i < stOutparam.stuDeviceInfo.nChanNum; i++)
            {
                CString csChannel;
                csChannel.Format("%d", i+1);
                pCmbChannel->InsertString(i, csChannel);
            }

            pBtnLoginAndLogout->SetWindowText(ConvertString("Logout"));
            pCmbChannel->SetCurSel(0);
            pBtnPlay->EnableWindow(TRUE);
        }
        else
        {   
            MessageBox(ConvertString("Login failed!"), ConvertString("Prompt")); 
            m_lLoginHandle = 0;
        }
    }
}

void CHeatMapDlg::OnBnClickedBtnStartPlayAndStopPlay()
{
    CButton* pBtnPlay =  (CButton*)GetDlgItem(IDC_BTN_START_PLAY_AND_STOP_PLAY);
    CButton* pBtnQueryHeadMap =  (CButton*)GetDlgItem(IDC_BTN_QUERY_HEAT_MAP);

    if (0 != m_lRealPlayHandle)
    {
        if (!CLIENT_StopRealPlay(m_lRealPlayHandle))
        {
            MessageBox(ConvertString("Stop real play failed!"), ConvertString("Prompt"));
            return;
        }
        m_lRealPlayHandle =0;
        pBtnPlay->SetWindowText(ConvertString("StartRealPlay"));
        pBtnQueryHeadMap->EnableWindow(FALSE);
        m_ctrPicture.SetImageFile(NULL);
        Invalidate(TRUE);
    }
    else
    {
        int nChannel = ((CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL))->GetCurSel();
        HWND hWnd = GetDlgItem(IDC_STATIC_STRAREAN)->GetSafeHwnd();

        m_lRealPlayHandle = CLIENT_StartRealPlay(m_lLoginHandle, nChannel, hWnd, DH_RType_Realplay, NULL, NULL, NULL);
        if (0 == m_lRealPlayHandle)
        {
            MessageBox(ConvertString("Real play failed!"), ConvertString("Prompt"));
            return;
        }
        else
        {
            pBtnPlay->SetWindowText(ConvertString("StopRealPlay"));
            pBtnQueryHeadMap->EnableWindow(TRUE);
        }       
    }
}

void CHeatMapDlg::OnBnClickedBtnQueryHeatMap()
{
    // Whether the begin time is less than end time
    bool bRet = IsTimeCorrent();
    if (!bRet)
    {
        MessageBox(ConvertString("The begin time is bigger than end time, please input again!"), ConvertString("Prompt"));
        return;
    }
    
    // Get the config of snap picture
    NET_ENCODE_SNAP_INFO stuSnapInfo = {sizeof(NET_ENCODE_SNAP_INFO)};
    stuSnapInfo.emSnapType = EM_SNAP_NORMAL;
    int nChannel = ((CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL))->GetCurSel();

    bRet = CLIENT_GetConfig(m_lLoginHandle, NET_EM_CFG_ENCODE_SNAP_INFO, nChannel, &stuSnapInfo, sizeof(NET_ENCODE_SNAP_INFO));
    if (!bRet)
    {
        MessageBox(ConvertString("Get the config of snap picture failed!"), ConvertString("Prompt"));
        return;
    }
    
    // Get the current file path
    char szPicturePath[MAX_PATH] = {0};
    GetModuleFileName(NULL, szPicturePath, MAX_PATH);
    string strPicturePath = szPicturePath;
    size_t nPos = strPicturePath.find_last_of('\\', strPicturePath.length());
    strPicturePath = strPicturePath.substr(0, nPos) + "\\" + HEAD_MAP_PCITURE_NAME;

    // Snap a picture from stream
    bRet = CLIENT_CapturePictureEx(m_lRealPlayHandle, strPicturePath.c_str(), NET_CAPTURE_BMP);
    if (!bRet)
    {
        MessageBox(ConvertString("Snap picture failed!"), ConvertString("Prompt"));
        return;
    }

    // Query Heat Map 
    NET_QUERY_HEAT_MAP stHeatMapInfo = {sizeof(NET_QUERY_HEAT_MAP)};
    stHeatMapInfo.stuIn.nChannel = nChannel;
    stHeatMapInfo.stuOut.nBufLen = HEAT_MAP_SIZE;
    stHeatMapInfo.stuOut.pBufData = new char[HEAT_MAP_SIZE];
    CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_BEGIN_SHORT_DATE);
    CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_BEGIN_TIME);
    CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_SHORT_DATE);
    CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);
    GetTimeFromTimeCtr(stHeatMapInfo.stuIn.stuBegin, pCtrBeginDate, pCtrBeginTime);
    GetTimeFromTimeCtr(stHeatMapInfo.stuIn.stuEnd, pCtrEndtDate, pCtrEndTime);
    int pRetLen = 0;

    bRet = CLIENT_QueryDevState(m_lLoginHandle, DH_DEVSTATE_GET_HEAT_MAP, (char *)&stHeatMapInfo, sizeof(NET_QUERY_HEAT_MAP), &pRetLen, MAX_TIMEOUT);
    if (!bRet)
    {
        MessageBox(ConvertString("Query heat map failed!"), ConvertString("Prompt"));
        delete []stHeatMapInfo.stuOut.pBufData;
        return;
    }

    // Display Heat Map and save
    CBmp bmp_Test;
    bmp_Test.SetBmpPathName(strPicturePath.c_str(),stuSnapInfo.nWidth,stuSnapInfo.nHeight); //后面背景图片的分辨率
    bmp_Test.SetoriginalValue(stHeatMapInfo.stuOut.nWidth, stHeatMapInfo.stuOut.nHeight);
    bmp_Test.CreateHeatMapBmp((unsigned char*)stHeatMapInfo.stuOut.pBufData);
    m_ctrPicture.SetImageFile(strPicturePath.c_str());
    
    delete []stHeatMapInfo.stuOut.pBufData;
}

void CHeatMapDlg::GetTimeFromTimeCtr(NET_TIME_EX& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime)
 {
     if (NULL == pCtrDate || NULL == pCtrTime)
     {
         return;
     }

     COleDateTime tmDate;
     COleDateTime tmTime;
     pCtrDate->GetTime(tmDate);
     pCtrTime->GetTime(tmTime);
    
     stTime.dwYear = tmDate.GetYear();
     stTime.dwMonth = tmDate.GetMonth();
     stTime.dwDay = tmDate.GetDay();
     stTime.dwHour = tmTime.GetHour();
     stTime.dwMinute = tmTime.GetMinute();
     stTime.dwSecond = tmTime.GetSecond();
 }

bool CHeatMapDlg::IsTimeCorrent()
{
    CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_BEGIN_SHORT_DATE);
    CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_BEGIN_TIME);
    CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_SHORT_DATE);
    CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);

    COleDateTime beginDate;
    COleDateTime beginTime;
    COleDateTime endDate;
    COleDateTime endTime;
    pCtrBeginDate->GetTime(beginDate);
    pCtrBeginTime->GetTime(beginTime);
    pCtrEndtDate->GetTime(endDate);
    pCtrEndTime->GetTime(endTime);

    if (endDate >= beginDate)
    {
        if (endDate == beginDate)
        {
            if (endTime < beginTime)
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

