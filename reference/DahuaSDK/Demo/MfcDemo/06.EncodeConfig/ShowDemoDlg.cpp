// ShowDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ShowDemo.h"
#include "ShowDemoDlg.h"
#include "math.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShowDemoDlg dialog

const char* const profileName[] = {"H.264 B", "H.264  ", "H.264 E", "H.264 H"};
const char* const strFormat[] = {"Main_Normal","Main_MoveExamine","Main_Alarm","Extra1","Extra2","Extra3"};
const char* const strSnapFormat[] = {"Normal Snap", "MoveExamine Snap", "Alarm Snap"};
const char* const strBitRate_CTL[] = {"BITRATE_CBR", "EM_BITRATE_VBR"};
const char* const strVideoCompression[] = {"MPEG4","MS_MPEG4","MPEG2","MPEG1","H263","MJPG","FCC_MPEG4","H264","H.265","SVAC"};
const char* const strImageQuarity[] = {"Image Quality 10%","Image Quality 30%","Image Quality 50%","Image Quality 60%","Image Quality 80%","Image Quality 100%"};
const char* const strName[] = {"D1", "HD1", "BCIF", "CIF", "QCIF", "VGA", "QVGA", "SVCD", "QQVGA", 
"SVGA", "XVGA", "WXGA", "SXGA", "WSXGA", "UXGA", "WUXGA", "LFT", "720P", "1080P", "1_3M", 
"2M","5M","3M","5_0M","1_2M", "1408_1024", "8M", "2560_1920", "960H", "960_720"
 		,"NHD" , "QNHD", "QQNHD","4000*3000","4096*2160","3840*2160"};
const char* const strFrameType[2] = {"DHAV","PS"};
int bitRateAll[] = {32, 48, 64, 80, 96, 128, 160, 192, 224,256, 320, 384, 448, 512, 640, 768, 896, 
        1024, 1280, 1536, 1792, 2048, 3072, 4096, 6144, 7168,8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528};
const char* const strAudioCompression[] = {"G711a","PCM","G711u","ARM","AAC","G726","G729","ADPCM","MPEG2","MPEG2L2","OGG",
"MP3","G7221"};
const char* const strSampleDepth[] = {"8Bit", "16Bit", "24Bit"};
const size_t encodeCount = sizeof(strVideoCompression)/sizeof(strVideoCompression[0]);
const size_t profileCount = sizeof(profileName)/sizeof(profileName[0]);
#define  STRDEFINE "user-define"

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)


CShowDemoDlg::CShowDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShowDemoDlg)
	m_DvrUserName = _T("admin");
	m_DvrPassword = _T("admin");
	m_DvrPort = 37777;
	m_nPicTime = 0;
	m_nPicHour = 0;
	m_nTrigPic = 0;
	m_nPacketReriod = 0;
	m_nInterval = 0;
	m_nSvc = 0;
	m_nSnapFrameRate = 0;
	m_nEditBitRate = 0;
	m_nMinFps = 0;
	m_nMaxFps = 0;
	//}}AFX_DATA_INIT
    m_LoginID = 0;
    m_DispHanle = 0;
    m_nChannel = 0;
    memset(&m_stuEncodeInfo, 0, sizeof(CFG_ENCODE_INFO));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    memset(&m_stuSnapInfo ,0 ,sizeof(NET_ENCODE_SNAP_INFO));
    m_stuSnapInfo.dwSize = sizeof(m_stuSnapInfo);

    memset(&m_stuSnapTimeInfo,0,sizeof(m_stuSnapTimeInfo));
    m_stuSnapTimeInfo.dwSize = sizeof(m_stuSnapTimeInfo);
    memset(&m_stuEncodeAudioInfo,0,sizeof(m_stuEncodeAudioInfo));
    m_stuEncodeAudioInfo.dwSize = sizeof(m_stuEncodeAudioInfo);
    memset(&m_stuEncodeAudioCompression ,0 ,sizeof(NET_ENCODE_AUDIO_COMPRESSION_INFO));
    m_stuEncodeAudioCompression.dwSize = sizeof(m_stuEncodeAudioCompression);
    memset(&m_stuEncodeVideoInfo ,0 ,sizeof(NET_ENCODE_VIDEO_INFO));
    m_stuEncodeVideoInfo.dwSize = sizeof(m_stuEncodeVideoInfo);

    memset(&m_stuVideoProFile,0,sizeof(m_stuVideoProFile));
    m_stuVideoProFile.dwSize = sizeof(m_stuVideoProFile);

    memset(&m_stuVideoSVC,0,sizeof(m_stuVideoSVC));
    m_stuVideoSVC.dwSize = sizeof(m_stuVideoSVC);

    memset(&m_stuPack,0,sizeof(m_stuPack));
    m_stuPack.dwSize = sizeof(m_stuPack);
	
	memset(&m_stuCaps, 0 , sizeof(m_stuCaps));
	m_stuCaps.dwSize = sizeof(m_stuCaps);		
}

void CShowDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShowDemoDlg)
	DDX_Control(pDX, IDC_STATIC_BITRATE, m_BitRate);
	DDX_Control(pDX, IDC_EDIT_BITRATE, m_EditBitRate);
	DDX_Control(pDX, IDC_EDIT_SNAPFRAMERATE, m_EditSnapFrameRate);
	DDX_Control(pDX, IDC_CHECK_VIDEO_ENABLE, m_VideoEnable);
	DDX_Control(pDX, IDC_CMB_PACK, m_Cmb_Pack);
	DDX_Control(pDX, IDC_CMB_FREQUENCY, m_Cmb_FreQuency);
	DDX_Control(pDX, IDC_CMB_DEPTH, m_Cmb_Depth);
	DDX_Control(pDX, IDC_CMB_MODE, m_Cmb_AudioMode);
	DDX_Control(pDX, IDC_CHECK_AUDIO_ENABLE, m_AudioEnable);
	DDX_Control(pDX, IDC_CHECK_SNAPENABLE, m_SnapEnable);
	DDX_Control(pDX, IDC_CMB_SNAPTYPE, m_Cmb_SnapType);
	DDX_Control(pDX, IDC_CMB_SNAP_QUALITY, m_Cmb_SnapQuality);
	DDX_Control(pDX, IDC_CMB_SNAP_FRAMERATE, m_Cmb_SnapFrameRate);
	DDX_Control(pDX, IDC_CMB_SNAPCOMPRESSION, m_Cmb_SnapCompression);
	DDX_Control(pDX, IDC_CMB_SNAPRESOLUTION, m_Cmb_SnapResolution);
	DDX_Control(pDX, IDC_CMB_AUDIO_FORMAT, m_Cmb_AudioFormat);
	DDX_Control(pDX, IDC_CMB_BITRATE, m_Cmb_BitRate);
	DDX_Control(pDX, IDC_CMB_FRAME_TYPE, m_Cmb_FrameType);
	DDX_Control(pDX, IDC_CMB_RESOLUTION, m_Cmb_Resolution);
	DDX_Control(pDX, IDC_CMB_FRAMERATE, m_Cmb_FrameRate);
	DDX_Control(pDX, IDC_STATIC_IMAGE_QUALITY, m_ImageQuality);
	DDX_Control(pDX, IDC_CMB_IMAGE_QUALITY, m_Cmb_ImageQuality);
	DDX_Control(pDX, IDC_CMB_COMPRESSION, m_Cmb_Compression);
	DDX_Control(pDX, IDC_CMB_BITRATE_CTL, m_Cmb_BitRate_Ctl);
	DDX_Control(pDX, IDC_CMB_FORMAT, m_Cmb_Format);
	DDX_Control(pDX, IDC_COMBO_CHANNELCOUNT, m_Cmb_ChannelCount);
	DDX_Control(pDX, IDC_IPADDRESS, m_DvrIPAddr);
	DDX_Text(pDX, IDC_EDIT_USERMANE, m_DvrUserName);
	DDX_Text(pDX, IDC_EDIT_PASSWD, m_DvrPassword);
	DDX_Text(pDX, IDC_EDIT_PORT, m_DvrPort);
	DDX_Text(pDX, IDC_EDIT_PACKETPERIOD, m_nPacketReriod);
	DDX_Text(pDX, IDC_EDIT_IFRAME_INTERVAL, m_nInterval);
	DDX_Text(pDX, IDC_EDIT_SVC, m_nSvc);
	DDX_Text(pDX, IDC_EDIT_SNAPFRAMERATE, m_nSnapFrameRate);
	DDX_Text(pDX, IDC_EDIT_BITRATE, m_nEditBitRate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CShowDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CShowDemoDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, OnBtnLogout)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELCOUNT, OnSelchangeChannel)
	ON_CBN_SELCHANGE(IDC_CMB_BITRATE_CTL, OnSelchangeCmbBitrateCtl)
	ON_CBN_SELCHANGE(IDC_CMB_FORMAT, OnSelchangeCmbFormat)
	ON_CBN_SELCHANGE(IDC_CMB_SNAPTYPE, OnSelchangeCmbSnaptype)
	ON_BN_CLICKED(IDC_SETSNAP, OnSetsnap)
	ON_BN_CLICKED(IDC_SETAUDIO, OnSetaudio)
	ON_BN_CLICKED(IDC_SETVIDEO, OnSetvideo)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_CMB_RESOLUTION, OnSelchangeCmbResolution)
	ON_CBN_SELCHANGE(IDC_CMB_COMPRESSION, OnSelchangeCmbCompression)
	ON_CBN_SELCHANGE(IDC_CMB_FRAMERATE, OnSelchangeCmbFramerate)
	ON_CBN_SELCHANGE(IDC_CMB_SNAP_FRAMERATE, OnSelchangeCmbSnapFramerate)
	ON_CBN_SELCHANGE(IDC_CMB_BITRATE, OnSelchangeCmbBitrate)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShowDemoDlg message handlers

void __stdcall DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CShowDemoDlg *pThis = (CShowDemoDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
}

void __stdcall ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CShowDemoDlg *pThis = (CShowDemoDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}

//Set static text in the dialogue box(English->current language) 
void g_SetWndStaticText(CWnd * pWnd)
{
	CString strCaption,strText;
	
	//Set main window title 
	pWnd->GetWindowText(strCaption);
	if(strCaption.GetLength()>0)
	{
		strText=ConvertString(strCaption);
		pWnd->SetWindowText(strText);
	}
	
	//Set sub-window title 
	CWnd * pChild=pWnd->GetWindow(GW_CHILD);
	CString strClassName;
	while(pChild)
	{
		//////////////////////////////////////////////////////////////////////////		
		//Added by Jackbin 2005-03-11
		strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
		if(strClassName == "CEdit")
		{
			//The next sub-window 
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}
		
		//////////////////////////////////////////////////////////////////////////	
		
		//Set current language text in the sub-window 
		pChild->GetWindowText(strCaption);
		strText=ConvertString(strCaption);
		pChild->SetWindowText(strText);
		
		//The next sub-window 
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}

BOOL CShowDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_DvrIPAddr.SetAddress(171,35,10,22);
	// TODO: Add extra initialization here
	CLIENT_Init(DisConnectFunc, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
    m_EditSnapFrameRate.ShowWindow(FALSE);
    m_BitRate.ShowWindow(FALSE);
    m_EditBitRate.ShowWindow(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

CString ConvertString(CString strText)
{
    char *val = new char[200];
    CString strIniPath,strRet;
    
    memset(val,0,200);
    GetPrivateProfileString("String",strText,"",
        val,200,"./langchn.ini");
    strRet = val;
    if(strRet.GetLength()==0)
    {
        //If there is no corresponding string in ini file then set it to be default value(English)
        strRet=strText;
    }
    delete val;
    return strRet;
}



void CShowDemoDlg::OnPaint() 
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
HCURSOR CShowDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

CString CShowDemoDlg::GetDvrIP()
{
    CString strRet="";
    BYTE nField0,nField1,nField2,nField3;
    m_DvrIPAddr.GetAddress(nField0,nField1,nField2,nField3);
    strRet.Format("%d.%d.%d.%d",nField0,nField1,nField2,nField3);
    return strRet;
}

void CShowDemoDlg::InitDlg()
{
    GetSnapConfig(EM_SNAP_NORMAL);
    {
      OnSnapStuToDlg();  
    }
    GetAudioConfig(EM_FORMAT_MAIN_NORMAL);
    {
        OnAudioStuToDlg();
    }
    GetVideoConfig(EM_FORMAT_MAIN_NORMAL);
    {
        OnVideoStuToDlg();
    }
}
void CShowDemoDlg::ShowLoginErrorReason(int nError)
{
    if(1 == nError)		MessageBox(ConvertString("Invalid password!"), ConvertString("Prompt"));
    else if(2 == nError)	MessageBox(ConvertString("Invalid account!"), ConvertString("Prompt"));
    else if(3 == nError)	MessageBox(ConvertString("Timeout!"), ConvertString("Prompt"));
    else if(4 == nError)	MessageBox(ConvertString("The user has logged in!"), ConvertString("Prompt"));
    else if(5 == nError)	MessageBox(ConvertString("The user has been locked!"), ConvertString("Prompt"));
    else if(6 == nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("Prompt"));
    else if(7 == nError)	MessageBox(ConvertString("The system is busy!"), ConvertString("Prompt"));
    else if(9 == nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("Prompt"));
    else	MessageBox(ConvertString("Login failed!"), ConvertString("Prompt"));
}

//Process when device disconnected 
LRESULT CShowDemoDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CShowDemoDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("Encoding Config"));
	return 0;
}

void CShowDemoDlg::OnBtnLogin() 
{
    BOOL bValid = UpdateData();
    if (!bValid)
    {
        return ;
    }

    char *pchDVRIP;
    CString strDvrIP = GetDvrIP();
    pchDVRIP = (LPSTR)(LPCSTR)strDvrIP;
    WORD wDVRPort=(WORD)m_DvrPort;
    char *pchUserName=(LPSTR)(LPCSTR)m_DvrUserName;
    char *pchPassword=(LPSTR)(LPCSTR)m_DvrPassword;

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, pchDVRIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, pchPassword, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, pchUserName, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = wDVRPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
    if(0 != lRet)
    {
        m_LoginID = lRet;
        GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);
        AddChannelCount(stOutparam.stuDeviceInfo.nChanNum);
        GetEncodeCaps(0, true);
        InitEncodeCtrl(m_stuCaps);
        InitSnapCtrl(m_stuCaps);
        InitDlg();
        LLONG lRet = CLIENT_RealPlayEx(m_LoginID,0,GetDlgItem(IDC_REALPALY)->m_hWnd,DH_RType_Realplay_0);
        if(0 != lRet)
        {
            m_DispHanle = lRet;
        }
        else
        {
            MessageBox(ConvertString("Play failed"), ConvertString("Prompt"));
        }
    }
    else
    {
        ShowLoginErrorReason(stOutparam.nError);
    }
	SetWindowText(ConvertString("Encoding Config"));
}

void CShowDemoDlg::OnBtnLogout() 
{
    CLIENT_StopRealPlay(m_DispHanle);
    BOOL bRet = CLIENT_Logout(m_LoginID);
    //Clear handle as o after logout 
    if(bRet)
    {
        m_LoginID=0;
        m_nChannel = 0;
        GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
    }
    Invalidate();
	SetWindowText(ConvertString("Encoding Config"));
}

void CShowDemoDlg::AddChannelCount(int nChannelCount)
{
    int nIndex = 0;
    CString str;
    m_Cmb_ChannelCount.ResetContent();
	int i = 0;
    for (i= 0;i<nChannelCount; ++i)
    {
        str.Format("%d",i+1);
        nIndex = m_Cmb_ChannelCount.AddString(str);
        m_Cmb_ChannelCount.SetItemData(nIndex,i);
        
    }
    m_Cmb_ChannelCount.SetCurSel(0);

    m_Cmb_Format.ResetContent();
    for (i = 0; i < 6; ++i)
    {
        nIndex = m_Cmb_Format.AddString(ConvertString(strFormat[i]));
        m_Cmb_Format.SetItemData(nIndex,i);
    }
    m_Cmb_Format.SetCurSel(0);

	int nSnapCount = sizeof(strSnapFormat)/sizeof(strSnapFormat[0]);
	m_Cmb_SnapType.ResetContent();
	for (i = 0; i < nSnapCount; i++)
	{
		nIndex = m_Cmb_SnapType.AddString(ConvertString(strSnapFormat[i]));
        m_Cmb_SnapType.SetItemData(nIndex,i);
	}
    m_Cmb_SnapType.SetCurSel(0);
}

void CShowDemoDlg::InitEncodeCtrl(NET_OUT_ENCODE_CFG_CAPS& stuCaps,int nStreamtype)
{
    int nIndex = 0;
    m_Cmb_BitRate_Ctl.ResetContent();
	int i = 0;
    for (i = 0; i<2; i++)
    {
        nIndex = m_Cmb_BitRate_Ctl.AddString(ConvertString(strBitRate_CTL[i]));
        m_Cmb_BitRate_Ctl.SetItemData(nIndex,i);
    }
    m_Cmb_ImageQuality.ResetContent();
    for (i = 0; i < 6; i++)
    {
        nIndex = m_Cmb_ImageQuality.AddString(ConvertString(strImageQuarity[i]));
        m_Cmb_ImageQuality.SetItemData(nIndex,i);
    }
    m_Cmb_FrameType.ResetContent();
    for (i = 0; i < 2; i++)
    {
        int nIndex = m_Cmb_FrameType.AddString(strFrameType[i]);
        m_Cmb_FrameType.SetItemData(nIndex,i);
    }
    m_Cmb_AudioFormat.ResetContent();
    for (i = 0; i < 13;i++)
    {
        int nIndex = m_Cmb_AudioFormat.AddString(strAudioCompression[i]);
        m_Cmb_AudioFormat.SetItemData(nIndex,i);
    }
    if (nStreamtype>=0 &&nStreamtype<3)
    {
        m_Cmb_FrameRate.ResetContent();
        if (stuCaps.stuMainFormatCaps[nStreamtype].nFPSMax == 0)
        {
            stuCaps.stuMainFormatCaps[nStreamtype].nFPSMax = 25;
        }
        for (i =0; i<stuCaps.stuMainFormatCaps[nStreamtype].nFPSMax;++i)
        {
            CString str;
            str.Format("%d",i+1);
            int nIndex = m_Cmb_FrameRate.AddString(str);
            m_Cmb_FrameRate.SetItemData(nIndex,i);
        }
        int  nCmbRate = int(m_stuEncodeVideoInfo.nFrameRate) - 1;
        m_Cmb_FrameRate.SetCurSel(nCmbRate);
        if (nCmbRate > stuCaps.stuMainFormatCaps[nStreamtype].nFPSMax)
        {
            m_Cmb_FrameRate.SetCurSel(stuCaps.stuMainFormatCaps[nStreamtype].nFPSMax-1);
        }
        m_Cmb_Compression.ResetContent();
        const int nH264ProfileRankNum = stuCaps.stuMainFormatCaps[nStreamtype].nH264ProfileRankNum;
        const BYTE* bH264ProfileRank = stuCaps.stuMainFormatCaps[nStreamtype].bH264ProfileRank;
        for (i = 0; i<encodeCount;i++)
        {
            if (stuCaps.stuMainFormatCaps[nStreamtype].dwEncodeModeMask & (0x01<<i))
            {
                
                if (i == VIDEO_FORMAT_H264 && nH264ProfileRankNum)
                {
                    for (int j = 0; j < nH264ProfileRankNum; ++j)
                    {
                        int nIndex = m_Cmb_Compression.AddString(profileName[bH264ProfileRank[j]-1]);
                        m_Cmb_Compression.SetItemData(nIndex,bH264ProfileRank[j] - 1 - profileCount);
                    }
                }
                else
                {
                    int nIndex = m_Cmb_Compression.AddString(strVideoCompression[i]);
                    m_Cmb_Compression.SetItemData(nIndex,i);
                }
            }
        }
        m_Cmb_Resolution.ResetContent();
        if (!stuCaps.stuMainFormatCaps[nStreamtype].abIndivResolution)
        {
            for (i = 0; i <stuCaps.stuMainFormatCaps[nStreamtype].nResolutionTypeNum; i++)
            {
                int nWidth = stuCaps.stuMainFormatCaps[nStreamtype].stuResolutionTypes[i].snWidth;
                int nHeight = stuCaps.stuMainFormatCaps[nStreamtype].stuResolutionTypes[i].snHight;
                int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
                int nIndex = -1;
                if (-1 == dwImageSizeMask)
                {
                    CString str;
                    str.Format("%d*%d",nWidth,nHeight);
                    nIndex = m_Cmb_Resolution.AddString(str);
                }
                else
                {
                    nIndex = m_Cmb_Resolution.AddString(strName[dwImageSizeMask]);
                }
                m_Cmb_Resolution.SetItemData(nIndex,i);
    	    }
        }
        else
        {
            for (i = 0; i<encodeCount;i++)
            {
                if (stuCaps.stuMainFormatCaps[nStreamtype].dwEncodeModeMask & (0x01<<i))
                {
                    for (int j = 0; j<stuCaps.stuMainFormatCaps[nStreamtype].nIndivResolutionNums[i];j++)
                    {
                        int nWidth = stuCaps.stuMainFormatCaps[nStreamtype].stuIndivResolutionTypes[i][j].snWidth;
                        int nHeight = stuCaps.stuMainFormatCaps[nStreamtype].stuIndivResolutionTypes[i][j].snHight;
                        int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
                        int nIndex = -1;
                        if (-1 == dwImageSizeMask)
                        {
                            CString str;
                            str.Format("%d*%d",nWidth,nHeight);
                            nIndex = m_Cmb_Resolution.AddString(str);
                        }
                        else
                        {
                            nIndex = m_Cmb_Resolution.AddString(strName[dwImageSizeMask]);
                        }
                        m_Cmb_Resolution.SetItemData(nIndex,j);
                    }
                    break;
                }
            }
        }
        m_Cmb_BitRate.ResetContent();
        int nMinBitRate = 0;
        int nMaxBitRate = 31;
        for (i = 0; i < 32; i++)
        {
            if (0 == nMinBitRate && stuCaps.stuMainFormatCaps[nStreamtype].nMinBitRateOptions <= bitRateAll[i])
            {
                nMinBitRate = i;
                break;
            }
        }

        for (i = 31; i >= 0; i--)
        {
            if (31 == nMaxBitRate && stuCaps.stuMainFormatCaps[nStreamtype].nMaxBitRateOptions >= bitRateAll[i])
            {
                nMaxBitRate =i;
                break;
            }
        }
        for (i = 0; i<(nMaxBitRate-nMinBitRate+1); i++)
        {
            CString str;
            str.Format("%d",bitRateAll[nMinBitRate+i]);
            int nIndex = m_Cmb_BitRate.AddString(str);
            m_Cmb_BitRate.SetItemData(nIndex,i);
        }
        int nIndex = m_Cmb_BitRate.AddString(ConvertString(STRDEFINE));
        m_Cmb_BitRate.SetItemData(nIndex,-1);
    }
    else if (nStreamtype>=3 && nStreamtype<6)
    {
        nStreamtype = nStreamtype -3;
        m_Cmb_FrameRate.ResetContent();
        if (stuCaps.stuExtraFormatCaps[nStreamtype].nFPSMax == 0)
        {
            stuCaps.stuExtraFormatCaps[nStreamtype].nFPSMax = 25;
        }
        for (i = 0; i < stuCaps.stuExtraFormatCaps[nStreamtype].nFPSMax; ++i)
        {
            CString str;
            str.Format("%d",i+1);
            int nIndex = m_Cmb_FrameRate.AddString(str);
            m_Cmb_FrameRate.SetItemData(nIndex,i);
        }
        int  nCmbRate = int(m_stuEncodeVideoInfo.nFrameRate) - 1;
        m_Cmb_FrameRate.SetCurSel(nCmbRate);
        if (nCmbRate > stuCaps.stuExtraFormatCaps[nStreamtype].nFPSMax)
        {
            m_Cmb_FrameRate.SetCurSel(stuCaps.stuExtraFormatCaps[nStreamtype].nFPSMax-1);
        }
        m_Cmb_Compression.ResetContent();
        const int nH264ProfileRankNum = stuCaps.stuExtraFormatCaps[nStreamtype].nH264ProfileRankNum;
        const BYTE* bH264ProfileRank = stuCaps.stuExtraFormatCaps[nStreamtype].bH264ProfileRank;
        for (i = 0; i < encodeCount; i++)
        {
            if (stuCaps.stuExtraFormatCaps[nStreamtype].dwEncodeModeMask & (0x01<<i))
            {
                
                if (i == VIDEO_FORMAT_H264 && nH264ProfileRankNum)
                {
                    for (int j = 0; j < nH264ProfileRankNum; ++j)
                    {
                        int nIndex = m_Cmb_Compression.AddString(profileName[bH264ProfileRank[j]-1]);
                        m_Cmb_Compression.SetItemData(nIndex,bH264ProfileRank[j] - 1 - profileCount);
                    }
                }
                else
                {
                    int nIndex = m_Cmb_Compression.AddString(strVideoCompression[i]);
                    m_Cmb_Compression.SetItemData(nIndex,i);
                }
            }
        }
//         m_Cmb_Resolution.ResetContent();
//         for (i = 0; i <stuCaps.stuExtraFormatCaps[nStreamtype].nResolutionTypeNum; i++)
//         {
//             int nWidth = stuCaps.stuExtraFormatCaps[nStreamtype].stuResolutionTypes[i].snWidth;
//             int nHeight = stuCaps.stuExtraFormatCaps[nStreamtype].stuResolutionTypes[i].snHight;
//             int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
//             int nIndex = m_Cmb_Resolution.AddString(strName[dwImageSizeMask]);
//             m_Cmb_Resolution.SetItemData(nIndex,i);
//         }
        m_Cmb_Resolution.ResetContent();
        if (!stuCaps.stuExtraFormatCaps[nStreamtype].abIndivResolution)
        {
            for (i = 0; i < stuCaps.stuExtraFormatCaps[nStreamtype].nResolutionTypeNum; i++)
            {
                int nWidth = stuCaps.stuExtraFormatCaps[nStreamtype].stuResolutionTypes[i].snWidth;
                int nHeight = stuCaps.stuExtraFormatCaps[nStreamtype].stuResolutionTypes[i].snHight;
                int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
                int nIndex = -1;
                if (-1 == dwImageSizeMask)
                {
                    CString str;
                    str.Format("%d*%d",nWidth,nHeight);
                    nIndex = m_Cmb_Resolution.AddString(str);
                }
                else
                {
                    nIndex = m_Cmb_Resolution.AddString(strName[dwImageSizeMask]);
                }
                m_Cmb_Resolution.SetItemData(nIndex,i);
            }
        }
        else
        {
            for (i = 0; i < encodeCount;i++)
            {
                if (stuCaps.stuExtraFormatCaps[nStreamtype].dwEncodeModeMask & (0x01<<i))
                {
                    for (int j = 0; j<stuCaps.stuExtraFormatCaps[nStreamtype].nIndivResolutionNums[i];j++)
                    {
                        int nWidth = stuCaps.stuExtraFormatCaps[nStreamtype].stuIndivResolutionTypes[i][j].snWidth;
                        int nHeight = stuCaps.stuExtraFormatCaps[nStreamtype].stuIndivResolutionTypes[i][j].snHight;
                        int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
                        int nIndex = -1;
                        if (-1 == dwImageSizeMask)
                        {
                            CString str;
                            str.Format("%d*%d",nWidth,nHeight);
                            nIndex = m_Cmb_Resolution.AddString(str);
                        }
                        else
                        {
                            nIndex = m_Cmb_Resolution.AddString(strName[dwImageSizeMask]);
                        }
                        m_Cmb_Resolution.SetItemData(nIndex,j);
                    }
                    break;
                }
            }
        }
        m_Cmb_BitRate.ResetContent();
        int nMinBitRate = 0;
        int nMaxBitRate = 0;
        for (i = 0; i < 32; i++)
        {
            if (stuCaps.stuExtraFormatCaps[nStreamtype].nMinBitRateOptions == bitRateAll[i])
            {
                nMinBitRate = i;
            }
            if (stuCaps.stuExtraFormatCaps[nStreamtype].nMaxBitRateOptions == bitRateAll[i])
            {
                nMaxBitRate =i;
            }
        }
        for (i = 0; i < (nMaxBitRate-nMinBitRate+1); i++)
        {
            CString str;
            str.Format("%d",bitRateAll[nMinBitRate+i]);
            int nIndex = m_Cmb_BitRate.AddString(str);
            m_Cmb_BitRate.SetItemData(nIndex,i);
        }
        int nIndex = m_Cmb_BitRate.AddString(ConvertString(STRDEFINE));
        m_Cmb_BitRate.SetItemData(nIndex,-1);
    }

	if (0 == m_Cmb_Format.GetCurSel())
	{
		SetDlgItemInt(IDC_STATIC_MIN, stuCaps.stuMainFormatCaps[0].nMinBitRateOptions);
		SetDlgItemInt(IDC_STATIC_MAX, stuCaps.stuMainFormatCaps[0].nMaxBitRateOptions);
	}
	else if (1 == m_Cmb_Format.GetCurSel())
	{
		SetDlgItemInt(IDC_STATIC_MIN, stuCaps.stuMainFormatCaps[1].nMinBitRateOptions);
		SetDlgItemInt(IDC_STATIC_MAX, stuCaps.stuMainFormatCaps[1].nMaxBitRateOptions);
	}
	else if (2 == m_Cmb_Format.GetCurSel())
	{
		SetDlgItemInt(IDC_STATIC_MIN, stuCaps.stuMainFormatCaps[2].nMinBitRateOptions);
		SetDlgItemInt(IDC_STATIC_MAX, stuCaps.stuMainFormatCaps[2].nMaxBitRateOptions);
	}
	else if (3 == m_Cmb_Format.GetCurSel())
	{
		SetDlgItemInt(IDC_STATIC_MIN, stuCaps.stuExtraFormatCaps[0].nMinBitRateOptions);
		SetDlgItemInt(IDC_STATIC_MAX, stuCaps.stuExtraFormatCaps[0].nMaxBitRateOptions);
	}
	else if (4 == m_Cmb_Format.GetCurSel())
	{
		SetDlgItemInt(IDC_STATIC_MIN, stuCaps.stuExtraFormatCaps[1].nMinBitRateOptions);
		SetDlgItemInt(IDC_STATIC_MAX, stuCaps.stuExtraFormatCaps[1].nMaxBitRateOptions);
	}
	else if (5 == m_Cmb_Format.GetCurSel())
	{
		SetDlgItemInt(IDC_STATIC_MIN, stuCaps.stuExtraFormatCaps[2].nMinBitRateOptions);
		SetDlgItemInt(IDC_STATIC_MAX, stuCaps.stuExtraFormatCaps[2].nMaxBitRateOptions);
	}	
}

void CShowDemoDlg::OnSelchangeChannel() 
{
    CLIENT_StopRealPlay(m_DispHanle);
    m_DispHanle = 0;
    const int i= m_Cmb_ChannelCount.GetCurSel();
	if (m_nChannel == i)
	{
        return;
	}
    m_nChannel = i;
    LLONG lRet = CLIENT_RealPlayEx(m_LoginID,m_nChannel,GetDlgItem(IDC_REALPALY)->m_hWnd,DH_RType_Realplay_0);
    if(0 != lRet)
    {
        m_DispHanle = lRet;
    }
    else
    {
        MessageBox(ConvertString("Play failed"), ConvertString("Prompt"));
    }
    m_Cmb_Format.SetCurSel(0);
    GetEncodeCaps(m_nChannel, true);
    InitEncodeCtrl(m_stuCaps,0);
    InitSnapCtrl(m_stuCaps,0);

    CShowDemoDlg::InitDlg();

    Invalidate();
}

void CShowDemoDlg::OnSelchangeCmbFormat() 
{
    int nStreamType = m_Cmb_Format.GetCurSel();
    InitEncodeCtrl(m_stuCaps,nStreamType);
    GetAudioConfig((NET_EM_FORMAT_TYPE)(nStreamType +1));
    OnAudioStuToDlg();
    GetVideoConfig((NET_EM_FORMAT_TYPE)(nStreamType +1));
    OnVideoStuToDlg();

    if (nStreamType>= 0&&nStreamType<3)
    {
        CLIENT_StopRealPlay(m_DispHanle);
        m_DispHanle = 0;
        const int i= m_Cmb_ChannelCount.GetCurSel();
        m_nChannel = i;
        LLONG lRet = CLIENT_RealPlayEx(m_LoginID,m_nChannel,GetDlgItem(IDC_REALPALY)->m_hWnd,DH_RType_Realplay_0);
        if(0 != lRet)
        {
            m_DispHanle = lRet;
        }
        else
        {
            MessageBox(ConvertString("Play failed"), ConvertString("Prompt"));
        }
    }
    else if(nStreamType >=3 && nStreamType<6)
    {
        CLIENT_StopRealPlay(m_DispHanle);
        m_DispHanle = 0;
        const int i= m_Cmb_ChannelCount.GetCurSel();
        m_nChannel = i;
        LLONG lRet = CLIENT_RealPlayEx(m_LoginID,m_nChannel,GetDlgItem(IDC_REALPALY)->m_hWnd,DH_RealPlayType(DH_RType_Realplay_0+nStreamType-2));
        if(0 != lRet)
        {
            m_DispHanle = lRet;
        }
        else
        {
            MessageBox(ConvertString("Play failed"), ConvertString("Prompt"));
        }
    }
}

void CShowDemoDlg::OnSelchangeCmbBitrateCtl() 
{
   int i = m_Cmb_BitRate_Ctl.GetCurSel();
   if (0 == i)
   {
        m_Cmb_ImageQuality.ShowWindow(FALSE);
        m_ImageQuality.ShowWindow(FALSE);
   }
   else
   {
       m_Cmb_ImageQuality.ShowWindow(TRUE);
       m_ImageQuality.ShowWindow(TRUE);
   }
}
void CShowDemoDlg::GetEncodeCaps(int nChannel, bool bFirst, int streamType, CFG_VIDEO_COMPRESSION cmp, CFG_H264_PROFILE_RANK profile, int nWidth, int nHeight, int FPS)
{
    UpdateData();
    if (m_LoginID ==0)
    {
        return;
    }
    char *szBuffer = new char[512*1024];
	if (NULL == szBuffer)
	{
		return;
	}

    int nerror = 0;
    BOOL bRet = CLIENT_GetNewDevConfig(m_LoginID, CFG_CMD_ENCODE,nChannel,szBuffer,512*1024,&nerror,TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Get video channel property setup failed!"), ConvertString("Prompt"));
	}
    int nRetLen = 0;
    bRet = CLIENT_ParseData(CFG_CMD_ENCODE, szBuffer, (char *)&m_stuEncodeInfo, sizeof(CFG_ENCODE_INFO), &nRetLen);	
    if (!bFirst)
    {
        // 改变字段值
        if (streamType < 3)
        {
            m_stuEncodeInfo.stuMainStream[streamType].stuVideoFormat.emCompression = cmp;
            if (cmp == VIDEO_FORMAT_H264)
            {
                // 修改profile
                m_stuEncodeInfo.stuMainStream[streamType].stuVideoFormat.abProfile = TRUE;
                m_stuEncodeInfo.stuMainStream[streamType].stuVideoFormat.emProfile = profile;
            }

            m_stuEncodeInfo.stuMainStream[streamType].stuVideoFormat.nWidth = nWidth;
            m_stuEncodeInfo.stuMainStream[streamType].stuVideoFormat.nHeight = nHeight;
            m_stuEncodeInfo.stuMainStream[streamType].stuVideoFormat.nFrameRate = FPS * 1.0f;
        }
        else 
        {
            streamType -= 3;
            m_stuEncodeInfo.stuExtraStream[streamType].stuVideoFormat.emCompression = cmp;
            if (cmp == VIDEO_FORMAT_H264)
            {
                // 修改profile
                m_stuEncodeInfo.stuExtraStream[streamType].stuVideoFormat.abProfile = TRUE;
                m_stuEncodeInfo.stuExtraStream[streamType].stuVideoFormat.emProfile = profile;
            }

            m_stuEncodeInfo.stuExtraStream[streamType].stuVideoFormat.nWidth = nWidth;
            m_stuEncodeInfo.stuExtraStream[streamType].stuVideoFormat.nHeight = nHeight;
            m_stuEncodeInfo.stuExtraStream[streamType].stuVideoFormat.nFrameRate = FPS * 1.0f;
        }

        // 更新当前配置
        m_stuEncodeVideoInfo.emCompression = (NET_EM_VIDEO_COMPRESSION)cmp;
        m_stuEncodeVideoInfo.nWidth = nWidth;
        m_stuEncodeVideoInfo.nHeight = nHeight;
        m_stuEncodeVideoInfo.nFrameRate = FPS * 1.0f;

        if (profile > 0)
        {
            m_stuVideoProFile.emProfile = (NET_EM_H264_PROFILE_RANK)profile;
        }
    }
    bRet = CLIENT_PacketData(CFG_CMD_ENCODE,(char *)&m_stuEncodeInfo,sizeof(CFG_ENCODE_INFO),szBuffer,512*1024);
    if (bRet)
    {
        memset(&m_stuCaps,0,sizeof(NET_OUT_ENCODE_CFG_CAPS));
        NET_IN_ENCODE_CFG_CAPS stuInEncodeCaps = {sizeof(stuInEncodeCaps)};
        stuInEncodeCaps.nChannelId = nChannel;
        stuInEncodeCaps.pchEncodeJson = szBuffer;
        m_stuCaps.dwSize = sizeof(NET_OUT_ENCODE_CFG_CAPS);
        
		m_stuCaps.stuMainFormatCaps[0].dwSize = sizeof(NET_STREAM_CFG_CAPS);
        m_stuCaps.stuMainFormatCaps[1].dwSize = sizeof(NET_STREAM_CFG_CAPS);
        m_stuCaps.stuMainFormatCaps[2].dwSize = sizeof(NET_STREAM_CFG_CAPS);      
      
		m_stuCaps.stuExtraFormatCaps[0].dwSize = sizeof(NET_STREAM_CFG_CAPS);
        m_stuCaps.stuExtraFormatCaps[1].dwSize = sizeof(NET_STREAM_CFG_CAPS);
        m_stuCaps.stuExtraFormatCaps[2].dwSize = sizeof(NET_STREAM_CFG_CAPS);
       
		m_stuCaps.stuSnapFormatCaps[0].dwSize = sizeof(NET_STREAM_CFG_CAPS);
        m_stuCaps.stuSnapFormatCaps[1].dwSize = sizeof(NET_STREAM_CFG_CAPS);
        
        BOOL bSuccess = CLIENT_GetDevCaps(m_LoginID, NET_ENCODE_CFG_CAPS,&stuInEncodeCaps,&m_stuCaps,TIMEOUT);
		if (!bSuccess)
		{
			MessageBox(ConvertString("Get encode config capacity failed!"), ConvertString("Prompt"));
		}
        
    }
    delete[] szBuffer;
	szBuffer = NULL;
}

void CShowDemoDlg::InitSnapCtrl(NET_OUT_ENCODE_CFG_CAPS& stuCaps,int nStreamtype)
{
    m_Cmb_SnapQuality.ResetContent();
	int i = 0;
    for (i = 0; i<6 ;i++)
    {
        int nIndex = m_Cmb_SnapQuality.AddString(ConvertString(strImageQuarity[i]));
        m_Cmb_SnapQuality.SetItemData(nIndex,i);
    }
    if (m_stuCaps.nSnapFormatCaps == 0)
    {
        memcpy(&stuCaps.stuSnapFormatCaps[nStreamtype],&stuCaps.stuMainFormatCaps[nStreamtype],sizeof(stuCaps.stuMainFormatCaps[nStreamtype]));
    }
    m_Cmb_SnapCompression.ResetContent();
    for (i = 0; i < encodeCount; i++)
    {
        if (stuCaps.stuSnapFormatCaps[nStreamtype].dwEncodeModeMask & (0x01<<i))
        {
            int nIndex = m_Cmb_SnapCompression.AddString(strVideoCompression[i]);
            m_Cmb_SnapCompression.SetItemData(nIndex,i);
        }
    }

    m_Cmb_SnapResolution.ResetContent();
    if (!stuCaps.stuSnapFormatCaps[nStreamtype].abIndivResolution)
    {
        for (i = 0; i <stuCaps.stuSnapFormatCaps[nStreamtype].nResolutionTypeNum; i++)
        {
            int nWidth = stuCaps.stuSnapFormatCaps[nStreamtype].stuResolutionTypes[i].snWidth;
            int nHeight = stuCaps.stuSnapFormatCaps[nStreamtype].stuResolutionTypes[i].snHight;
            int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
            int nIndex = -1;
            if (-1 == dwImageSizeMask)
            {
                CString str;
                str.Format("%d*%d",nWidth,nHeight);
                nIndex = m_Cmb_SnapResolution.AddString(str);
            }
            else
            {
                nIndex = m_Cmb_SnapResolution.AddString(strName[dwImageSizeMask]);
            }
            m_Cmb_SnapResolution.SetItemData(nIndex,i);
        }
    }
    else
    {
        for (i = 0; i < encodeCount; i++)
        {
            if (stuCaps.stuSnapFormatCaps[nStreamtype].dwEncodeModeMask & (0x01<<i))
            {
                for (int j = 0; j<stuCaps.stuSnapFormatCaps[nStreamtype].nIndivResolutionNums[i];j++)
                {
                    int nWidth = stuCaps.stuSnapFormatCaps[nStreamtype].stuIndivResolutionTypes[i][j].snWidth;
                    int nHeight = stuCaps.stuSnapFormatCaps[nStreamtype].stuIndivResolutionTypes[i][j].snHight;
                    int dwImageSizeMask = ResolutionInttoSize(nWidth,nHeight);
                    int nIndex = -1;
                    if (-1 == dwImageSizeMask)
                    {
                        CString str;
                        str.Format("%d*%d",nWidth,nHeight);
                        nIndex = m_Cmb_SnapResolution.AddString(str);
                    }
                    else
                    {
                        nIndex = m_Cmb_SnapResolution.AddString(strName[dwImageSizeMask]);
                    }
                    m_Cmb_SnapResolution.SetItemData(nIndex,j);
                }
                break;
            }
        }
    }
    m_Cmb_SnapFrameRate.ResetContent();
    for (i = 0; i < 8; ++i)
    {
        if ( i == 7)
        {
            int nIndex = m_Cmb_SnapFrameRate.AddString(ConvertString(STRDEFINE));
            m_Cmb_SnapFrameRate.SetItemData(nIndex,i);
            break;
        }
        CString str;
        str.Format("%d",i+1);
        int nIndex = m_Cmb_SnapFrameRate.AddString(str);
        m_Cmb_SnapFrameRate.SetItemData(nIndex,i);
    }
}

void CShowDemoDlg::ResolutionSizetoInt(int nResolution, int *nWidth, int *nHeight)
{
	
    switch (nResolution)
    {
	    case 0: 			*nWidth = 704;	*nHeight = 576;		break;//D1
	    case 1:				*nWidth = 352;	*nHeight = 576;		break;//HD1
	    case 2:				*nWidth = 704;	*nHeight = 288;		break;//BCIF
	    case 3:				*nWidth = 352;	*nHeight = 288;		break;//CIF
	    case 4:				*nWidth = 176;	*nHeight = 144;		break;//QCIF
	    case 5:				*nWidth = 640;	*nHeight = 480;		break;//VGA
	    case 6:				*nWidth = 320;	*nHeight = 240;		break;//QVGA
	    case 7:				*nWidth = 480;	*nHeight = 480;		break;//SVCD
	    case 8:				*nWidth = 160;	*nHeight = 128;		break;//QQVGA
	    case 9:				*nWidth = 800;  *nHeight = 592;		break;//SVGA
	    case 10:			*nWidth = 1024;	*nHeight = 768;		break;//XVGA
	    case 11: 			*nWidth = 1280;	*nHeight = 800;		break;//WXGA
	    case 12:			*nWidth = 1280;	*nHeight = 1024;	break;//SXGA
	    case 13:			*nWidth = 1600;	*nHeight = 1024;	break;//WSXGA
	    case 14:			*nWidth = 1600;	*nHeight = 1200;	break;//UXGA
	    case 15:			*nWidth = 1920;	*nHeight = 1200;	break;//WUXGA
	    case 16:			*nWidth = 240;	*nHeight = 192;		break;//LTF
	    case 17:			*nWidth = 1280;	*nHeight = 720;		break;//720
	    case 18:			*nWidth = 1920;	*nHeight = 1080;	break;//1080
	    case 19:			*nWidth = 1280;	*nHeight = 960;		break;//1_3M
	    case 20:			*nWidth = 1872;	*nHeight = 1408;	break;//2M
	    case 21:			*nWidth = 3744;	*nHeight = 1408;	break;//5M
	    case 22:			*nWidth = 2048;	*nHeight = 1536;	break;//3M
	    case 23:			*nWidth = 2432;	*nHeight = 2050;	break;//5_0M
	    case 24:			*nWidth = 1216;	*nHeight = 1024;	break;//1_2M
	    case 25:			*nWidth = 1408;	*nHeight = 1024;	break;//1408_1024
	    case 26:			*nWidth = 3296;	*nHeight = 2472;	break;//8M
	    case 27:			*nWidth = 2560;	*nHeight = 1920;	break;//2560_1920
	    case 28:			*nWidth = 960;	*nHeight = 576;		break;//960H
	    case 29:			*nWidth = 960;  *nHeight = 720;		break;//960_720
	    case 30:			*nWidth = 640;	*nHeight = 360;		break;//NHD
	    case 31:			*nWidth = 320;	*nHeight = 180;		break;//QNHD
	    case 32:			*nWidth = 160;	*nHeight = 90;		break;//QQNHD
        case 33:            *nWidth = 4000; *nHeight = 3000;    break;
        case 34:            *nWidth = 4096; *nHeight = 2160;    break;
        case 35:            *nWidth = 3840; *nHeight = 2160;    break; 
	    default:	ASSERT(FALSE);				break;
    }

}

int CShowDemoDlg::ResolutionInttoSize(int nWidth, int nHeight)
{
	int nResolution = 0;

	if (nWidth == 704 && nHeight == 576)
	{
		nResolution = 0;
	}
	else if (nWidth == 352 && nHeight == 576)
	{
		nResolution = 1;
	}
	else if (nWidth == 704 && nHeight == 288)
	{
		nResolution = 2;
	}
	else if (nWidth == 352 && nHeight == 288)
	{
		nResolution = 3;
	}
	else if (nWidth == 176 && nHeight == 144)
	{
		nResolution = 4;
	}
	else if (nWidth == 640 && nHeight == 480)
	{
		nResolution = 5;
	}
	else if (nWidth == 320 && nHeight == 240)
	{
		nResolution = 6;
	}
	else if (nWidth == 480 && nHeight == 480)
	{
		nResolution = 7;
	}
	else if (nWidth == 160 && nHeight == 128)
	{
		nResolution = 8;
	}
	else if (nWidth == 800 && nHeight == 592)
	{
		nResolution = 9;
	}
	else if (nWidth == 1024 && nHeight == 768)
	{
		nResolution = 10;
	}
	else if (nWidth == 1280 && nHeight == 800)
	{
		nResolution = 11;
	}
	else if (nWidth == 1280 && nHeight == 1024)
	{
		nResolution = 12;
	}
	else if (nWidth == 1600 && nHeight == 1024)
	{
		nResolution = 13;
	}
	else if (nWidth == 1600 && nHeight == 1200)
	{
		nResolution = 14;
	}
	else if (nWidth == 1920 && nHeight == 1200)
	{
		nResolution = 15;
	}
	else if (nWidth == 240 && nHeight == 192)
	{
		nResolution = 16;
	}
	else if (nWidth == 1280 && nHeight == 720)
	{
		nResolution = 17;
	}
	else if (nWidth == 1920 && nHeight == 1080)
	{
		nResolution = 18;
	}
	else if (nWidth == 1280 && nHeight == 960)
	{
		nResolution = 19;
	}
	else if (nWidth == 1872 && nHeight == 1408)
	{
		nResolution = 20;
	}
	else if (nWidth == 3744 && nHeight == 1408)
	{
		nResolution = 21;
	}
	else if (nWidth == 2048 && nHeight == 1536)
	{
		nResolution = 22;
	}
	else if (nWidth == 2432 && nHeight == 2050)
	{
		nResolution = 23;
	}
	else if (nWidth == 1216 && nHeight == 1024)
	{
		nResolution = 24;
	}
	else if (nWidth == 1408 && nHeight == 1024)
	{
		nResolution = 25;
	}
	else if (nWidth == 3296 && nHeight == 2472)
	{
		nResolution = 26;
	}
	else if (nWidth == 2560 && nHeight == 1920)
	{
		nResolution = 27;
	}
	else if (nWidth == 960 && nHeight == 576)
	{
		nResolution = 28;
	}
	else if (nWidth == 960 && nHeight == 720)
	{
		nResolution = 29;
	}
	else if (nWidth == 640 && nHeight == 360)
	{
		nResolution = 30;
	}
	else if (nWidth == 320 && nHeight == 180)
	{
		nResolution = 31;
	}
	else if (nWidth == 160 && nHeight == 90)
	{
		nResolution = 32;
	}
    else if (nWidth == 4000 && nHeight == 3000)
	{
        nResolution = 33;
    }
    else if (nWidth == 4096 && nHeight == 2160)
    {
        nResolution = 34;
	}
    else if (nWidth == 3840 && nHeight == 2160)
    {
        nResolution = 35;
	}
    else
    {
        nResolution = -1;
    }
	return nResolution;
}

void CShowDemoDlg::ResolutionSizeToInt(int nSize, int& nWidth, int& nHeight)
{
    switch (nSize)
    {
    case 0:
        nWidth = 704 ; nHeight = 576 ;
        break;
    case 1:
        nWidth = 352 ; nHeight = 576 ;
        break;
    case 2:
        nWidth = 704 ; nHeight = 288 ;
        break;
    case 3:
        nWidth = 352 ; nHeight = 288 ;
        break;
    case 4:
        nWidth = 176 ; nHeight = 144 ;
        break;
    case 5:
        nWidth = 640 ; nHeight = 480 ;
        break;
    case 6:
        nWidth = 320 ; nHeight = 240 ;
        break;
    case 7:
        nWidth = 480 ; nHeight = 480 ;
        break;
    case 8:
        nWidth = 160 ; nHeight = 128 ;
        break;
    case 9:
        nWidth = 800 ; nHeight = 592 ;
        break;
    case 10:
        nWidth = 1024 ; nHeight = 768 ;
        break;
    case 11:
        nWidth = 1280 ; nHeight = 800 ;
        break;
    case 12:
        nWidth = 1280 ; nHeight = 1024 ;
        break;
    case 13:
        nWidth = 1600 ; nHeight = 1024 ;
        break;
    case 14:
        nWidth = 1600 ; nHeight = 1200 ;
        break;
    case 15:
        nWidth = 1920 ; nHeight = 1200 ;
        break;
    case 16:
        nWidth = 240 ; nHeight = 192 ;
        break;
    case 17:
        nWidth = 1280 ; nHeight = 720 ;
        break;
    case 18:
        nWidth = 1920 ; nHeight = 1080 ;
        break;
    case 19:
        nWidth = 1280 ; nHeight = 960 ;
        break;
    case 20:
        nWidth = 1872 ; nHeight = 1408 ;
        break;
    case 21:
        nWidth = 3744 ; nHeight = 1408 ;
        break;
    case 22:
        nWidth = 2048 ; nHeight = 1536 ;
        break;
    case 23:
        nWidth = 2432 ; nHeight = 2050 ;
        break;
    case 24:
        nWidth = 1216 ; nHeight = 1024 ;
        break;
    case 25:
        nWidth = 1408 ; nHeight = 1024 ;
        break;
    case 26:
        nWidth = 3296 ; nHeight = 2472 ;
        break;
    case 27:
        nWidth = 2560 ; nHeight = 1920 ;
        break;
    case 28:
        nWidth = 960 ; nHeight = 576 ;
        break;
    case 29:
        nWidth = 960 ; nHeight = 720 ;
        break;
    case 30:
        nWidth = 640 ; nHeight = 360 ;
        break;
    case 31:
        nWidth = 320 ; nHeight = 180 ;
        break;
    case 32:
        nWidth = 160 ; nHeight = 90 ;
        break;
    case 33:
        nWidth = 4000; nHeight = 3000;
        break;
    case 34:
        nWidth = 4096; nHeight = 2160;
        break;
    case 35:
        nWidth = 3840; nHeight = 2160;
        break; 
    default:
        break;
    }
}

BOOL CShowDemoDlg::GetSnapConfig(NET_EM_SNAP_TYPE emType)
{
    m_stuSnapInfo.emSnapType =  emType;
    BOOL bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_ENCODE_SNAP_INFO, m_nChannel,  &m_stuSnapInfo, sizeof(m_stuSnapInfo),TIMEOUT);
    bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_ENCODE_SNAPTIME,m_nChannel,&m_stuSnapTimeInfo,sizeof(m_stuSnapTimeInfo),TIMEOUT);
    return bRet;
}

BOOL CShowDemoDlg::SetSnapConfig()
{
    BOOL bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_SNAP_INFO, m_nChannel, &m_stuSnapInfo, sizeof(m_stuSnapInfo),TIMEOUT);
//    bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_SNAPTIME, m_nChannel, &m_stuSnapTimeInfo,sizeof(m_stuSnapTimeInfo));
    return bRet;
}
void CShowDemoDlg::OnSnapStuToDlg()
{
    m_Cmb_SnapType.SetCurSel(m_stuSnapInfo.emSnapType-1);
    DWORD dwResolution = ResolutionInttoSize(m_stuSnapInfo.nWidth,m_stuSnapInfo.nHeight);
    int nIndex = 0;
    if ( -1 == dwResolution)
    {
        CString str;
        str.Format("%d*%d",m_stuSnapInfo.nWidth,m_stuSnapInfo.nHeight);
        nIndex = m_Cmb_SnapResolution.FindStringExact(0,str);
    }
    else
    {
        nIndex = m_Cmb_SnapResolution.FindStringExact(0,strName[dwResolution]);
    }
    m_Cmb_SnapResolution.SetCurSel(nIndex);
    nIndex = m_Cmb_SnapCompression.FindStringExact(0, strVideoCompression[m_stuSnapInfo.emCompression]);
    m_Cmb_SnapCompression.SetCurSel(nIndex);
    float fTmp = ceilf(1.0f / m_stuSnapInfo.nFrameRate - 0.499); // rounding 四舍五入
    m_nSnapFrameRate = (int)fTmp;
    int nTmp = (int)(fTmp - 1);
    m_Cmb_SnapFrameRate.SetCurSel(nTmp) ;
    if (nTmp > 6)
    {
        m_Cmb_SnapFrameRate.SetCurSel(7);
        m_EditSnapFrameRate.ShowWindow(TRUE);
    }
    else
    {
        m_EditSnapFrameRate.ShowWindow(FALSE);
    }
    m_Cmb_SnapQuality.SetCurSel(m_stuSnapInfo.emImageQuality -1);
    if (m_stuSnapInfo.bSnapEnable)
    {
        m_SnapEnable.SetCheck(BST_CHECKED);
    }
    else
    {
        m_SnapEnable.SetCheck(BST_UNCHECKED);
    }
    m_nPicTime =  m_stuSnapTimeInfo.shPicTimeInterval;
    m_nPicHour = m_stuSnapTimeInfo.bPicIntervalHour;
    m_nTrigPic = m_stuSnapTimeInfo.dwTrigPicIntervalSecond;
    UpdateData(FALSE);
}

void CShowDemoDlg::OnSnapDlgToStu()
{
    UpdateData();
    CString str;
    m_stuSnapInfo.emSnapType = (NET_EM_SNAP_TYPE)(m_Cmb_SnapType.GetCurSel()+1);
    int nIndex = m_Cmb_SnapResolution.GetCurSel();
    if (nIndex >=0)
    {
        m_Cmb_SnapResolution.GetLBText(nIndex, str);
    }
    int n = str.Find("*",0);
	
	int i;
    for (i = 0; i < 36; i++)
    {
        int nWidth = 0;
        int nHeight = 0;
        if (str == strName[i])
        {
            ResolutionSizetoInt(i,&nWidth,&nHeight);
            m_stuSnapInfo.nWidth = nWidth;
            m_stuSnapInfo.nHeight = nHeight;
            break;
        }
    }
    if (i == 36)
    {
        m_stuSnapInfo.nWidth = atoi(str.Left(n));
        int nLen = str.GetLength();
        m_stuSnapInfo.nHeight = atoi(str.Right(nLen - n - 1));
    }
    nIndex = m_Cmb_SnapCompression.GetCurSel();
    if (nIndex >=0)
    {
        m_Cmb_SnapCompression.GetLBText(nIndex , str);
    }
    for (i =0; i<encodeCount;i++)
    {
        if (str == strVideoCompression[i])
        {
            m_stuSnapInfo.emCompression = (NET_EM_VIDEO_COMPRESSION)i;
        }
    }
    int nRate = m_Cmb_SnapFrameRate.GetCurSel();
    if (nRate > 6)
    {
        m_stuSnapInfo.nFrameRate = 1.0f / m_nSnapFrameRate;
    }
    else
    {
        m_stuSnapInfo.nFrameRate = 1.0f / (nRate + 1);
    }
    m_stuSnapInfo.emImageQuality = (NET_EM_IMAGE_QUALITY)(m_Cmb_SnapQuality.GetCurSel()+1);
    m_stuSnapInfo.bSnapEnable = m_SnapEnable.GetCheck();
    m_stuSnapTimeInfo.shPicTimeInterval = m_nPicTime;
    m_stuSnapTimeInfo.bPicIntervalHour = m_nPicHour;
    m_stuSnapTimeInfo.dwTrigPicIntervalSecond = m_nTrigPic;

}
void CShowDemoDlg::OnSelchangeCmbSnaptype() 
{
    int i = m_Cmb_SnapType.GetCurSel()+1;
    GetSnapConfig((NET_EM_SNAP_TYPE)i);
    OnSnapStuToDlg();
}

BOOL CShowDemoDlg::GetAudioConfig(NET_EM_FORMAT_TYPE emType)
{
    m_stuEncodeAudioInfo.emFormatType = emType;
    BOOL bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_ENCODE_AUDIO_INFO,m_nChannel, &m_stuEncodeAudioInfo,sizeof(m_stuEncodeAudioInfo),TIMEOUT);
    m_stuEncodeAudioCompression.emFormatType = emType;
    bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_ENCODE_AUDIO_COMPRESSION, m_nChannel,&m_stuEncodeAudioCompression,sizeof(m_stuEncodeAudioCompression),TIMEOUT);
    return bRet;
}

BOOL CShowDemoDlg::SetAudioConfig()
{
    BOOL bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_AUDIO_COMPRESSION, m_nChannel,&m_stuEncodeAudioCompression,sizeof(m_stuEncodeAudioCompression),TIMEOUT);
    bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_AUDIO_INFO,m_nChannel, &m_stuEncodeAudioInfo,sizeof(m_stuEncodeAudioInfo),TIMEOUT);
    return bRet;
}
void CShowDemoDlg::OnAudioStuToDlg()
{
    if (m_stuEncodeAudioCompression.bAudioEnable)
    {
        m_AudioEnable.SetCheck(BST_CHECKED);
    }
    else
    {
        m_AudioEnable.SetCheck(BST_UNCHECKED);
    }
    m_Cmb_AudioFormat.SetCurSel(m_stuEncodeAudioCompression.emCompression-1);
    m_Cmb_AudioMode.SetCurSel(m_stuEncodeAudioInfo.nMode);

	for (int i = 0; i < 3; i++)
	{
		int nIndex = m_Cmb_Depth.AddString(ConvertString(strSampleDepth[i]));
		m_Cmb_Depth.SetItemData(nIndex, i);
	}
    m_Cmb_Depth.SetCurSel(m_stuEncodeAudioInfo.nDepth/8 -1);

    CString str;
    str.Format("%d",m_stuEncodeAudioInfo.nFrequency);
    int nIndex = m_Cmb_FreQuency.FindStringExact(0, str);
    m_Cmb_FreQuency.SetCurSel(nIndex);
    m_Cmb_Pack.SetCurSel(m_stuEncodeAudioInfo.nFrameType);
    m_nPacketReriod = m_stuEncodeAudioInfo.nPacketPeriod;
    UpdateData(FALSE);
}

void CShowDemoDlg::OnAudioDlgToStu()
{
    UpdateData();
    m_stuEncodeAudioCompression.bAudioEnable = m_AudioEnable.GetCheck();
    m_stuEncodeAudioCompression.emCompression = (NET_EM_AUDIO_FORMAT)(m_Cmb_AudioFormat.GetCurSel()+1);
    m_stuEncodeAudioCompression.emFormatType = (NET_EM_FORMAT_TYPE)(m_Cmb_Format.GetCurSel()+1);
    m_stuEncodeAudioInfo.emFormatType = (NET_EM_FORMAT_TYPE)(m_Cmb_Format.GetCurSel()+1);
    m_stuEncodeAudioInfo.nDepth = (m_Cmb_Depth.GetCurSel()+1)*8;
    CString str;
    int nIndex = m_Cmb_FreQuency.GetCurSel();
    if (nIndex >=0)
    {
        m_Cmb_FreQuency.GetLBText(nIndex , str);
    }
    m_stuEncodeAudioInfo.nFrequency = atoi(str);
    m_stuEncodeAudioInfo.nMode = m_Cmb_AudioMode.GetCurSel();
    m_stuEncodeAudioInfo.nFrameType = m_Cmb_Pack.GetCurSel();
    m_stuEncodeAudioInfo.nPacketPeriod = m_nPacketReriod;
}
BOOL CShowDemoDlg::GetVideoConfig(NET_EM_FORMAT_TYPE emType)
{
    m_stuEncodeVideoInfo.emFormatType = emType;
    BOOL bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_ENCODE_VIDEO,m_nChannel,&m_stuEncodeVideoInfo,sizeof(m_stuEncodeVideoInfo),TIMEOUT);
    m_stuVideoProFile.emFormatType = emType;
    bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_ENCODE_VIDEO_PROFILE, m_nChannel,&m_stuVideoProFile,sizeof(m_stuVideoProFile),TIMEOUT);
    m_stuVideoSVC.emFormatType = emType;
    bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_ENCODE_VIDEO_SVC,m_nChannel, &m_stuVideoSVC,sizeof(m_stuVideoSVC),TIMEOUT);
    m_stuPack.emFormatType = emType;
    bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_ENCODE_VIDEO_PACK, m_nChannel ,&m_stuPack,sizeof(m_stuPack),TIMEOUT);
    return bRet;
}

BOOL CShowDemoDlg::SetVideoConfig()
{
    BOOL bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_ENCODE_VIDEO,m_nChannel,&m_stuEncodeVideoInfo,sizeof(m_stuEncodeVideoInfo),TIMEOUT);
    bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_VIDEO_PROFILE, m_nChannel,&m_stuVideoProFile,sizeof(m_stuVideoProFile),TIMEOUT);
    bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_VIDEO_SVC,m_nChannel, &m_stuVideoSVC,sizeof(m_stuVideoSVC),TIMEOUT);
    bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_ENCODE_VIDEO_PACK, m_nChannel ,&m_stuPack,sizeof(m_stuPack),TIMEOUT);
    return bRet;
}
void CShowDemoDlg::OnVideoStuToDlg()
{
    if (m_stuEncodeVideoInfo.bVideoEnable)
    {
        m_VideoEnable.SetCheck(BST_CHECKED);
    }
    else
    {
        m_VideoEnable.SetCheck(BST_UNCHECKED);
    }
    if (m_stuCaps.stuMainFormatCaps[0].nH264ProfileRankNum == 0)
    {
        m_stuVideoProFile.emProfile = EM_PROFILE_UNKNOWN;
    }
    if (m_stuEncodeVideoInfo.emCompression == VIDEO_FORMAT_H264 && m_stuVideoProFile.emProfile != EM_PROFILE_UNKNOWN)
    {
        int nIndex = m_Cmb_Compression.FindStringExact(0,profileName[m_stuVideoProFile.emProfile - 1]);
        m_Cmb_Compression.SetCurSel(nIndex);
    }
    else
    {
        int nIndex = m_Cmb_Compression.FindStringExact(0,strVideoCompression[m_stuEncodeVideoInfo.emCompression]);
        m_Cmb_Compression.SetCurSel(nIndex);
    }
    int  nCmbRate = int(m_stuEncodeVideoInfo.nFrameRate) - 1;
    m_Cmb_FrameRate.SetCurSel(nCmbRate);
    if (m_stuCaps.stuMainFormatCaps[0].nFPSMax == 0)
    {
        m_stuCaps.stuMainFormatCaps[0].nFPSMax = 25;
    }
    if (nCmbRate > m_stuCaps.stuMainFormatCaps[0].nFPSMax)
    {
        m_Cmb_FrameRate.SetCurSel(m_stuCaps.stuMainFormatCaps[0].nFPSMax-1);
    }
    DWORD dwResolution = ResolutionInttoSize(m_stuEncodeVideoInfo.nWidth,m_stuEncodeVideoInfo.nHeight);
    int nIndex = -1;
    if ( -1 == dwResolution)
    {
        CString str;
        str.Format("%d*%d",m_stuEncodeVideoInfo.nWidth,m_stuEncodeVideoInfo.nHeight);
        nIndex = m_Cmb_Resolution.FindStringExact(0,str);
    }
    else
    {
        nIndex = m_Cmb_Resolution.FindStringExact(0,strName[dwResolution]);
    }
    m_Cmb_Resolution.SetCurSel(nIndex);
    m_nInterval = m_stuEncodeVideoInfo.nIFrameInterval;
    m_Cmb_BitRate_Ctl.SetCurSel(m_stuEncodeVideoInfo.emBitRateControl);
    if (0 == m_stuEncodeVideoInfo.emBitRateControl)
    {
        m_Cmb_ImageQuality.ShowWindow(FALSE);
        m_ImageQuality.ShowWindow(FALSE);
    }
    else
    {
        m_Cmb_ImageQuality.ShowWindow(TRUE);
        m_ImageQuality.ShowWindow(TRUE);
    }
    CString str;
    str.Format("%d",m_stuEncodeVideoInfo.nBitRate);
    nIndex = m_Cmb_BitRate.FindStringExact(0, str);
    if (nIndex == CB_ERR)
    {
        nIndex = m_Cmb_BitRate.GetCount();
        m_Cmb_BitRate.SetCurSel(nIndex - 1);
        m_BitRate.ShowWindow(TRUE);
        m_EditBitRate.ShowWindow(TRUE);
        m_nEditBitRate = m_stuEncodeVideoInfo.nBitRate;
    }
    else
    {
        m_Cmb_BitRate.SetCurSel(nIndex);
        m_BitRate.ShowWindow(FALSE);
        m_EditBitRate.ShowWindow(FALSE);
        m_nEditBitRate = m_stuEncodeVideoInfo.nBitRate;
    }
    
    m_Cmb_FrameType.SetCurSel(m_stuPack.emPackType -1);
    m_Cmb_ImageQuality.SetCurSel(m_stuEncodeVideoInfo.emImageQuality-1);
    m_nSvc = m_stuVideoSVC.nSVC;
    UpdateData(FALSE);
}

void CShowDemoDlg::OnVideoDlgToStu()
{
    UpdateData();
    m_stuEncodeVideoInfo.emFormatType = (NET_EM_FORMAT_TYPE)(m_Cmb_Format.GetCurSel()+1);
    m_stuEncodeVideoInfo.bVideoEnable = m_VideoEnable.GetCheck();
    CString str;
    int nIndex = m_Cmb_Compression.GetCurSel();
    if (nIndex >=0)
    {
        m_Cmb_Compression.GetLBText(nIndex , str);
    }

	int i = 0;
    for (i = 0;i < encodeCount; i++)
    {
        if (str == strVideoCompression[i])
        {
            m_stuEncodeVideoInfo.emCompression = (NET_EM_VIDEO_COMPRESSION)i;
            break;
        }
    }
    m_stuVideoProFile.emFormatType = NET_EM_FORMAT_TYPE(m_Cmb_Format.GetCurSel()+1);
    for (i = 0;i<profileCount;i++)
    {
        if (str == profileName[i])
        {
            m_stuEncodeVideoInfo.emCompression = EM_VIDEO_FORMAT_H264;
            m_stuVideoProFile.emProfile = (NET_EM_H264_PROFILE_RANK)(i+1);
        }
    }
    m_stuEncodeVideoInfo.nFrameRate = m_Cmb_FrameRate.GetCurSel() + 1.0f;
    nIndex = m_Cmb_Resolution.GetCurSel();
    if (nIndex >=0)
    {
        m_Cmb_Resolution.GetLBText(nIndex, str);
    }
    int n = str.Find("*",0);	
    for (i = 0;i < 36;i++)
    {
        int nWidth = 0;
        int nHeight = 0;
        if (str == strName[i])
        {
            ResolutionSizetoInt(i,&nWidth,&nHeight);
            m_stuEncodeVideoInfo.nWidth = nWidth;
            m_stuEncodeVideoInfo.nHeight = nHeight;
            break;
        }
    }
    if (i == 36)
    {
        m_stuEncodeVideoInfo.nWidth = atoi(str.Left(n));
        int nLen = str.GetLength();
        m_stuEncodeVideoInfo.nHeight = atoi(str.Right(nLen - n - 1));
    }
    m_stuEncodeVideoInfo.nIFrameInterval = m_nInterval;
    m_stuEncodeVideoInfo.emBitRateControl = (NET_EM_BITRATE_CONTROL)m_Cmb_BitRate_Ctl.GetCurSel();
    nIndex = m_Cmb_BitRate.GetCurSel();
    if (nIndex >=0)
    {
        m_Cmb_BitRate.GetLBText(nIndex,str);
    }
    if (0 == str.Compare(ConvertString(STRDEFINE)))
    {
        m_stuEncodeVideoInfo.nBitRate = m_nEditBitRate;
    }
    else
    {
        m_stuEncodeVideoInfo.nBitRate = atoi(str);
    }
    
    m_stuEncodeVideoInfo.emImageQuality = (NET_EM_IMAGE_QUALITY)(m_Cmb_ImageQuality.GetCurSel()+1);
    m_stuVideoSVC.emFormatType = (NET_EM_FORMAT_TYPE)(m_Cmb_Format.GetCurSel()+1);
    m_stuVideoSVC.nSVC = m_nSvc;
    m_stuPack.emFormatType = (NET_EM_FORMAT_TYPE)(m_Cmb_Format.GetCurSel()+1);
    m_stuPack.emPackType = (NET_EM_PACK_TYPE)(m_Cmb_FrameType.GetCurSel()+1);
}
void CShowDemoDlg::OnSetsnap() 
{
    OnSnapDlgToStu();
    if (!SetSnapConfig())
    {
        MessageBox(ConvertString("Setting capture config failed!"), ConvertString("Prompt"));
    }
}

void CShowDemoDlg::OnSetaudio() 
{
	OnAudioDlgToStu();
    if (!SetAudioConfig())
    {
        MessageBox(ConvertString("Setting Audio Config failed!"),ConvertString("Prompt"));
    }
}

void CShowDemoDlg::OnSetvideo() 
{
	OnVideoDlgToStu();
    if (!SetVideoConfig())
    {
       MessageBox(ConvertString("Setting Video Config failed!"),ConvertString("Prompt"));
    }
}

void CShowDemoDlg::OnClose() 
{	
	CDialog::OnClose();
}


void CShowDemoDlg::OnSelchangeCmbResolution() 
{
	// TODO: Add your control notification handler code here
	// 分辨率变化，需要更新帧率、码流范围

    int nIndex = m_Cmb_Resolution.GetCurSel();
    if(CB_ERR != nIndex)
    {
        updateEncodeCaps();
    }
}

void CShowDemoDlg::OnSelchangeCmbCompression() 
{
	// TODO: Add your control notification handler code here
	// 压缩模式变化，需要更新分辨率、帧率、码流范围
    int nIndex = m_Cmb_Compression.GetCurSel();
    if(CB_ERR != nIndex)
    {
        updateEncodeCaps();
    }
}

void CShowDemoDlg::OnSelchangeCmbFramerate() 
{
	// TODO: Add your control notification handler code here
	// 帧率变化，需要更新码流范围

    int nIndex = m_Cmb_FrameRate.GetCurSel();
    if(CB_ERR != nIndex)
    {
        updateEncodeCaps();
    }
}

void CShowDemoDlg::updateEncodeCaps() 
{
    int nIndex = m_Cmb_ChannelCount.GetCurSel();
    int nChn = (int)m_Cmb_ChannelCount.GetItemData(nIndex);
    
    nIndex = m_Cmb_Format.GetCurSel();
    int nStream = nIndex;
    
    nIndex = m_Cmb_Compression.GetCurSel();
    CString strCmp;
    if (nIndex>= 0)
    {
        m_Cmb_Compression.GetLBText(nIndex, strCmp);
    }
    CFG_VIDEO_COMPRESSION emCmp = VIDEO_FORMAT_MPEG4;
    CFG_H264_PROFILE_RANK emProfile = PROFILE_BASELINE;
    int i = 0;
    for (i = 0; i < 4; i++)
    {
        if (0 == strCmp.Compare(CString(profileName[i])))
        {
            emCmp = VIDEO_FORMAT_H264;
            emProfile = (CFG_H264_PROFILE_RANK)(emProfile + i);
            break;
        }
    }
    
    if (i == 4)
    {
        for (int j = 0; j < sizeof(strVideoCompression) / sizeof(char *); j++)
        {
            if (0 == strCmp.Compare(CString(strVideoCompression[j])))
            {
                emCmp = (CFG_VIDEO_COMPRESSION)j;
                break;
            }
        }
    }
    
    nIndex = m_Cmb_Resolution.GetCurSel();
    CString strRes;
    if (nIndex>= 0)
    {
        m_Cmb_Resolution.GetLBText(nIndex, strRes);
    }
    int n = strRes.Find("*",0);
    int nW = 0;
    int nH = 0;
    for (i = 0; i < 36; i++)
    {
        if (0 == strRes.Compare(CString(strName[i])))
        {
            ResolutionSizeToInt(i, nW, nH);
            break;
        }
    }
    if (i == 36)
    {
        nW = atoi(strRes.Left(n));
        int nLen = strRes.GetLength();
        nH = atoi(strRes.Right(nLen - n - 1));
    }
    
    nIndex = m_Cmb_FrameRate.GetCurSel();
    int nFps = 0;
    if (nIndex>=0)
    {
        CString strFps;
        m_Cmb_FrameRate.GetLBText(nIndex, strFps);
        nFps = atoi(strFps);
        
    }
    
    GetEncodeCaps(nChn, false, nStream, emCmp, emProfile, nW, nH, nFps);

    InitEncodeCtrl(m_stuCaps, nStream);
    OnVideoStuToDlg();
    OnAudioStuToDlg();
    Invalidate();
}

void CShowDemoDlg::OnSelchangeCmbSnapFramerate() 
{
    int nRate = m_Cmb_SnapFrameRate.GetCurSel();
    if (nRate>6)
    {
        m_EditSnapFrameRate.ShowWindow(TRUE);
        m_nSnapFrameRate = 8;
    }
    else
    {
        m_EditSnapFrameRate.ShowWindow(FALSE);
    }
    UpdateData(FALSE);
}

void CShowDemoDlg::OnSelchangeCmbBitrate() 
{
    int nIndex = m_Cmb_BitRate.GetCurSel();
    CString csStr;
    m_Cmb_BitRate.GetLBText(nIndex,csStr);
    if (0 == csStr.Compare(ConvertString(STRDEFINE)))
    {
        m_BitRate.ShowWindow(TRUE);
        m_EditBitRate.ShowWindow(TRUE);
		m_nEditBitRate = m_stuEncodeVideoInfo.nBitRate;
    }
    else
    {
        m_BitRate.ShowWindow(FALSE);
        m_EditBitRate.ShowWindow(FALSE);
    }
}

void CShowDemoDlg::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();
}


BOOL CShowDemoDlg::PreTranslateMessage(MSG* pMsg) 
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
void CShowDemoDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO

	if (m_DispHanle)
	{
		CLIENT_StopRealPlay(m_DispHanle);
	}
	CLIENT_Logout(m_LoginID);
	CLIENT_Cleanup();
}
