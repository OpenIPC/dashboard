// HumanTraitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HumanTrait.h"
#include "HumanTraitDlg.h"
#include "SearchPictureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CHumanTraitDlg dialog

// Device ip
#define DEVICE_IP           "172.23.12.138"     
// Connect port  
#define CONNECT_PORT        37777            
// Connect user name  
#define CONNECT_USERNAME    "admin"          
// Connect password
#define CONNECT_PASSWORD    "admin123"    

// 衣服颜色 包括上衣颜色和裤子颜色
char* g_pColor[] = {"Unknown", "White", "Orange", "Pink", "Black", "Red", "Yellow", "Gray", "Blue", "Green", "Purple", "Brown"};
char* g_pCoatType[] = {"Unknown", "LongSleeve", "ShortSleeve"};
char* g_pTrousersType[] = {"Unknown", "LongPants", "ShortPants", "Skirt"};

// Device disconnect Callback
void CALLBACK DisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    if(0 != dwUser)
    {
        PostMessage(((CHumanTraitDlg *)dwUser)->GetSafeHwnd(), WM_DEVICE_DISCONNECT, 0, 0);
    }
}

// Device reconnect Callback
void CALLBACK ReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    if(0 != dwUser)
    {
        PostMessage(((CHumanTraitDlg *)dwUser)->GetSafeHwnd(), WM_REDEVICE_RECONNECT, 0, 0);
    }
}

struct HUMAN_TRAIT_EVENT_INFO 
{
    DEV_EVENT_HUMANTRAIT_INFO stuEvent;
    BYTE* pBuf;
    int nBufSize;
};

//Intelligent traffic event  call back
int CALLBACK CbIntelligentEvent(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
    if (0 == dwUser)
    {
        return -1;
    }

    if (dwAlarmType == EVENT_IVS_HUMANTRAIT)
    {
        HUMAN_TRAIT_EVENT_INFO *stuHumanInfo = new HUMAN_TRAIT_EVENT_INFO;
        memcpy((void*)&(stuHumanInfo->stuEvent), pAlarmInfo, sizeof(DEV_EVENT_HUMANTRAIT_INFO));
        stuHumanInfo->nBufSize = dwBufSize;
        stuHumanInfo->pBuf = new BYTE[dwBufSize];
        memset(stuHumanInfo->pBuf, 0, dwBufSize);
        memcpy(stuHumanInfo->pBuf, pBuffer, dwBufSize);
        
        PostMessage(((CHumanTraitDlg *)dwUser)->GetSafeHwnd(), WM_INTELLIGENT_EVENT, (WPARAM)stuHumanInfo, 0);
    }
	return 0;
}

// Handle message : WM_DEVICE_DISCONNECT
LRESULT CHumanTraitDlg::OnDeviceDisConnect(WPARAM wParam, LPARAM lParam)
{
    SetWindowText(ConvertString("Network disconnected!"));
    return 0;
}

// Handle message : WM_REDEVICE_RECONNECT
LRESULT CHumanTraitDlg::OnDeviceReconnect(WPARAM wParam, LPARAM lParam)
{
    SetWindowText(ConvertString("HumanTrait"));
    return 0;
}

// Handle message : WM_INTELLIGENT_EVENT
LRESULT CHumanTraitDlg::OnIntelligentEvent(WPARAM wParam, LPARAM lParam)
{
    if (wParam == 0)
    {
        return 0;
    }
    HUMAN_TRAIT_EVENT_INFO* stuHumanInfo =  (HUMAN_TRAIT_EVENT_INFO*)wParam;    
    DEV_EVENT_HUMANTRAIT_INFO* pInfo = &(stuHumanInfo->stuEvent);
    
    // Show picture
    BYTE* pBuf = stuHumanInfo->pBuf;

    int nHumanPicLegnth = pInfo->stuHumanImage.nOffSet+pInfo->stuHumanImage.nLength;
    if (pInfo->stuHumanImage.nLength != 0 && nHumanPicLegnth <= stuHumanInfo->nBufSize)
    {
        SetDlgItemText(IDC_EDIT_COAT_COLOR, ConvertString(g_pColor[pInfo->stuHumanAttributes.emCoatColor]));
        SetDlgItemText(IDC_EDIT_TROUSERS_COLOR, ConvertString(g_pColor[pInfo->stuHumanAttributes.emTrousersColor]));
        SetDlgItemText(IDC_EDIT_COAT_TYPE, ConvertString(g_pCoatType[pInfo->stuHumanAttributes.emCoatType]));
        SetDlgItemText(IDC_EDIT_TROUSERS_TYPE, ConvertString(g_pTrousersType[pInfo->stuHumanAttributes.emTrousersType]));
        m_ctrHumanTratPicture.SetImageDate(pBuf + pInfo->stuHumanImage.nOffSet, pInfo->stuHumanImage.nLength);
    }
    else
    {
        SetDlgItemText(IDC_EDIT_COAT_COLOR, "");
        SetDlgItemText(IDC_EDIT_TROUSERS_COLOR, "");
        SetDlgItemText(IDC_EDIT_COAT_TYPE, "");
        SetDlgItemText(IDC_EDIT_TROUSERS_TYPE, "");
        m_ctrHumanTratPicture.SetImageFile(NULL);
        Invalidate();
    }

    nHumanPicLegnth = pInfo->stuFaceImage.nOffSet+pInfo->stuFaceImage.nLength;
    if (pInfo->stuFaceImage.nLength != 0 && nHumanPicLegnth <= stuHumanInfo->nBufSize)
    {
        m_ctrFacePicture.SetImageDate(pBuf + pInfo->stuFaceImage.nOffSet, pInfo->stuFaceImage.nLength);
       
    }
    else
    {
        m_ctrFacePicture.SetImageFile(NULL);
        Invalidate();
      
    }

    delete[] stuHumanInfo->pBuf;
    delete stuHumanInfo;
    return 0;
}


CHumanTraitDlg::CHumanTraitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHumanTraitDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_lLoginHandle = 0;
    m_lRealPlayHandle = 0;
    m_lRealLoadHandle = 0;
    m_nChannelNum = 0;
}

void CHumanTraitDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_START_PLAY_AND_STOP_PLAY, m_btnPlay);
    DDX_Control(pDX, IDC_CMB_CHANNEL, m_cmbChannel);
    DDX_Control(pDX, IDC_BTN_SUBSCIRBE, m_btnSubscibe);
    DDX_Control(pDX, IDC_BTN_SEARCH_PICTURE, m_btnSearch);
    DDX_Control(pDX, IDC_STATIC_HUMMAN_TRAIT_PIC, m_ctrHumanTratPicture);
    DDX_Control(pDX, IDC_STATIC_FACE_PIC, m_ctrFacePicture);
}

BEGIN_MESSAGE_MAP(CHumanTraitDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_LOGIN_AND_LOGOUT, &CHumanTraitDlg::OnBnClickedBtnLoginAndLogout)
    ON_BN_CLICKED(IDC_BTN_SUBSCIRBE, &CHumanTraitDlg::OnBnClickedBtnSubscirbe)
    ON_BN_CLICKED(IDC_BTN_START_PLAY_AND_STOP_PLAY, &CHumanTraitDlg::OnBnClickedBtnStartPlayAndStopPlay)
    ON_MESSAGE(WM_DEVICE_DISCONNECT,&CHumanTraitDlg::OnDeviceDisConnect)
    ON_MESSAGE(WM_REDEVICE_RECONNECT,&CHumanTraitDlg::OnDeviceReconnect)
    ON_MESSAGE(WM_INTELLIGENT_EVENT, &CHumanTraitDlg::OnIntelligentEvent)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BTN_SEARCH_PICTURE, &CHumanTraitDlg::OnBnClickedBtnSearchPicture)
END_MESSAGE_MAP()

// CHumanTraitDlg message handlers

BOOL CHumanTraitDlg::PreTranslateMessage(MSG* pMsg)
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


BOOL CHumanTraitDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    SetDlgItemText(IDC_IPADDRESS1, DEVICE_IP);
    SetDlgItemInt(IDC_EDIT_PORT, CONNECT_PORT);
    SetDlgItemText(IDC_EDIT_USER, CONNECT_USERNAME);
    SetDlgItemText(IDC_EDIT_PWD, CONNECT_PASSWORD);
    
    m_btnPlay.EnableWindow(FALSE);
    m_btnSubscibe.EnableWindow(FALSE);
    m_btnSearch.EnableWindow(FALSE);
    
    g_SetWndStaticText(this);
    CLIENT_Init(DisConnect, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
    CLIENT_SetAutoReconnect(ReConnect, (LDWORD)this);
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CHumanTraitDlg::OnDestroy()
{
    CDialog::OnDestroy();

    if (0 != m_lLoginHandle)
    {
        OnBnClickedBtnLoginAndLogout();
    }

    CLIENT_Cleanup();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHumanTraitDlg::OnPaint()
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
HCURSOR CHumanTraitDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHumanTraitDlg::OnBnClickedBtnLoginAndLogout()
{
    CButton* pBtnLoginAndLogout =  (CButton*)GetDlgItem(IDC_BTN_LOGIN_AND_LOGOUT);
    if (0 != m_lLoginHandle )
    {
        if (0 != m_lRealPlayHandle)
        {
            OnBnClickedBtnStartPlayAndStopPlay();
        }
        if (0 != m_lRealLoadHandle)
        {
            OnBnClickedBtnSubscirbe();
        }

        CLIENT_Logout(m_lLoginHandle);
        m_lLoginHandle = 0;
        m_cmbChannel.ResetContent();
        m_btnPlay.EnableWindow(FALSE);
        m_btnSubscibe.EnableWindow(FALSE);
        m_btnSearch.EnableWindow(FALSE);
        pBtnLoginAndLogout->SetWindowText(ConvertString("Login"));
        SetWindowText(ConvertString("HumanTrait"));
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
            m_nChannelNum = stOutparam.stuDeviceInfo.nChanNum;
            for (int i = 0 ;i < stOutparam.stuDeviceInfo.nChanNum; i++)
            {
                CString csChannel;
                csChannel.Format("%d", i+1);
                m_cmbChannel.InsertString(i, csChannel);
            }
            m_cmbChannel.SetCurSel(0);
        
            m_btnPlay.EnableWindow(TRUE);
            m_btnSubscibe.EnableWindow(TRUE);
            m_btnSearch.EnableWindow(TRUE);
            pBtnLoginAndLogout->SetWindowText(ConvertString("Logout"));
        }
        else
        {   
            MessageBox(ConvertString("Login failed!"), ConvertString("Prompt")); 
            m_lLoginHandle = 0;
        }
    }
}

void CHumanTraitDlg::OnBnClickedBtnStartPlayAndStopPlay()
{
    if (0 != m_lRealPlayHandle)
    {
        CLIENT_StopRealPlay(m_lRealPlayHandle);
        m_lRealPlayHandle =0;
        m_btnPlay.SetWindowText(ConvertString("Start Play"));
        Invalidate(TRUE);
    }
    else
    {
        int nChannel = m_cmbChannel.GetCurSel();
        HWND hWnd = GetDlgItem(IDC_STATIC_STRAREAN)->GetSafeHwnd();

        m_lRealPlayHandle = CLIENT_StartRealPlay(m_lLoginHandle, nChannel, hWnd, DH_RType_Realplay, NULL, NULL, NULL);
        if (0 == m_lRealPlayHandle)
        {
            MessageBox(ConvertString("Real play failed!"), ConvertString("Prompt"));
            return;
        }
        m_btnPlay.SetWindowText(ConvertString("Stop Play"));
        CLIENT_RenderPrivateData(m_lRealPlayHandle, TRUE);
        
    }
}


void CHumanTraitDlg::OnBnClickedBtnSubscirbe()
{
    if (0 != m_lRealLoadHandle)
    {
        CLIENT_StopLoadPic(m_lRealLoadHandle);
        m_lRealLoadHandle = 0;
        m_ctrFacePicture.SetImageFile(NULL);
        m_ctrHumanTratPicture.SetImageFile(NULL);
        Invalidate();
        m_btnSubscibe.SetWindowText(ConvertString("Subscribe"));
        SetDlgItemText(IDC_EDIT_COAT_COLOR, "");
        SetDlgItemText(IDC_EDIT_TROUSERS_COLOR, "");
        SetDlgItemText(IDC_EDIT_COAT_TYPE, "");
        SetDlgItemText(IDC_EDIT_TROUSERS_TYPE, "");
    }
    else
    { 
        int nChannel = m_cmbChannel.GetCurSel();
        m_lRealLoadHandle = CLIENT_RealLoadPictureEx(m_lLoginHandle, nChannel, EVENT_IVS_ALL, TRUE, CbIntelligentEvent, (LDWORD)this, NULL);
        if (m_lRealLoadHandle == 0)
        {		
            MessageBox(ConvertString("Subscribe intelligent failed!"), ConvertString("Prompt"));
            return;
        }
        m_btnSubscibe.SetWindowText(ConvertString("Unsubscribe"));
    }
}



void CHumanTraitDlg::OnBnClickedBtnSearchPicture()
{
    CSearchPictureDlg dlg;
    dlg.Init(m_lLoginHandle, m_nChannelNum);
    dlg.DoModal();
}
