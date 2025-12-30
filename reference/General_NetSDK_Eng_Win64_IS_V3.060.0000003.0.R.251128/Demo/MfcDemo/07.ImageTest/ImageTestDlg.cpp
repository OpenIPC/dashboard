// ImageTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageTest.h"
#include "ImageTestDlg.h"
#include "BlackLight.h"
#include "Defog.h"
#include "ExposureDlg.h"
#include "WhiteBalanceDlg.h"
#include "AudioSource.h"
#include "FocusValueDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
const char* const szAudioName[] = {"Coaxial", "BNC", "HDCVI_BNC", "LineIn","LineIn1","LineIn2","LineIn3",
                                   "Mic","Mic1","Mic2","Mic3","MicOut","Remote","Remote1","Remote2","Remote3"};

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

/////////////////////////////////////////////////////////////////////////////
// CImageTestDlg dialog

CImageTestDlg::CImageTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImageTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImageTestDlg)
	m_DvrPassword = _T("admin");
	m_DvrUserName = _T("admin");
	m_DvrPort = 37777;
	m_szChannelTitle = _T("");
	m_szAudioIn = 0;
	m_szAudioOut = 0;
	m_Static_Contrast = _T("");
	m_Static_Gamma = _T("");
	m_Static_Saturation = _T("");
	m_Static_Bright = _T("");
	m_bEnhancement = FALSE;
	m_nSensitivity = 0;
	m_nDelay = 0;
	m_nCorrection = 0;
	m_nSensitive = 0;
	m_bIrisAuto = FALSE;
	m_nLevel = 0;
	m_nSharpness = 0;
	m_nVideoNoise = 0;
	m_b2D = FALSE;
	m_n2DLevel = 0;
	m_nSnfLevel = 0;
	m_nTnfLevel = 0;
	m_nRadio = -1;
	m_nEndHour = 0;
	m_nEndMinute = 0;
	m_nEndSecond = 0;
	m_nStartHour = 0;
	m_nStartMinute = 0;
	m_nStartSecond = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_LoginID = 0;
    m_DispHanle = 0;
    m_nChannel = 0;
    memset(&m_stuVideoInImage,0,sizeof(m_stuVideoInImage));
    m_stuVideoInImage.dwSize = sizeof(m_stuVideoInImage);

    memset(&m_stuChannelTitle, 0,sizeof(m_stuChannelTitle));
    m_stuChannelTitle.dwSize = sizeof(m_stuChannelTitle);
    memset(&m_stuDeNoise,0,sizeof(m_stuDeNoise));
    m_stuDeNoise.dwSize = sizeof(m_stuDeNoise);
    memset(&m_stuAudioInVolume,0,sizeof(m_stuAudioInVolume));
    m_stuAudioInVolume.dwSize = sizeof(m_stuAudioInVolume);
    memset(&m_stuAudioOutVolume,0,sizeof(m_stuAudioOutVolume));
    m_stuAudioOutVolume.dwSize = sizeof(m_stuAudioOutVolume);
    memset(&m_stuVideoMode,0,sizeof(m_stuVideoMode));
    m_stuVideoMode.dwSize = sizeof(m_stuVideoMode);
    memset(&m_stuVideoColor,0,sizeof(m_stuVideoColor));
    m_stuVideoColor.dwSize = sizeof(m_stuVideoColor);
    memset(&m_stuImageEnhancement,0,sizeof(m_stuImageEnhancement));
    m_stuImageEnhancement.dwSize = sizeof(m_stuImageEnhancement);
    memset(&m_stuDayNight,0,sizeof(m_stuDayNight));
    m_stuDayNight.dwSize = sizeof(m_stuDayNight);
    memset(&m_stuLighting,0,sizeof(m_stuLighting));
    m_stuLighting.dwSize = sizeof(m_stuLighting);
    memset(&m_stuStable,0,sizeof(m_stuStable));
    m_stuStable.dwSize = sizeof(m_stuStable);
    memset(&m_stuIrisAuto,0,sizeof(m_stuIrisAuto));
    m_stuIrisAuto.dwSize = sizeof(m_stuIrisAuto);
    memset(&m_stuSharpness,0,sizeof(m_stuSharpness));
    m_stuSharpness.dwSize = sizeof(m_stuSharpness);
    memset(&m_stuVideoDenoise,0,sizeof(m_stuVideoDenoise));
    m_stuVideoDenoise.dwSize = sizeof(m_stuVideoDenoise);
    memset(&m_stuVideo3DDenoise, 0, sizeof(m_stuVideo3DDenoise));
    m_stuVideo3DDenoise.dwSize = sizeof(m_stuVideo3DDenoise);

    memset(&m_stuICR, 0, sizeof(m_stuICR));
    m_stuICR.dwSize = sizeof(m_stuICR);
}

void CImageTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImageTestDlg)
	DDX_Control(pDX, IDC_COMBO_ALGORITHMTYPE, m_Cmb_AlgorithmType);
	DDX_Control(pDX, IDC_COMBO_STABLE, m_Cmb_Stable);
	DDX_Control(pDX, IDC_COMBO_LIGHTMODE, m_Cmb_LightMode);
	DDX_Control(pDX, IDC_COMBO_DAYNIGHT, m_Cmb_DayNight);
	DDX_Control(pDX, IDC_SLIDER_ENHANCEMENT, m_Slider_Enhancement);
	DDX_Control(pDX, IDC_SLIDER_SATURATION, m_Slider_Saturation);
	DDX_Control(pDX, IDC_SLIDER_GAMMA, m_Slider_Gamma);
	DDX_Control(pDX, IDC_SLIDER_CONTRASE, m_Slider_Contrast);
	DDX_Control(pDX, IDC_SLIDER_BRIGHT, m_Slider_Bright);
	DDX_Control(pDX, IDC_CMB_MODE, m_Cmb_Mode);
	DDX_Control(pDX, IDC_CMB_ROTATE, m_Cmb_Ratate);
	DDX_Control(pDX, IDC_COMBO_CHANNELCOUNT, m_Cmb_ChannelCount);
	DDX_Control(pDX, IDC_CMB_TYPE, m_Cmb_Type);
	DDX_Control(pDX, IDC_COMBO_VIDEONOISE, m_Cmb_VideoNoise);
	DDX_Control(pDX, IDC_COMBO_SHARPNESSMODE, m_Cmb_SharpnessMode);
	DDX_Control(pDX, IDC_IPADDRESS, m_DvrIPAddr);
	DDX_Text(pDX, IDC_EDIT_PASSWD, m_DvrPassword);
	DDX_Text(pDX, IDC_EDIT_USERMANE, m_DvrUserName);
	DDX_Text(pDX, IDC_EDIT_PORT, m_DvrPort);
	DDX_Text(pDX, IDC_EDIT_CHANNELTITLE, m_szChannelTitle);
	DDX_Text(pDX, IDC_EDIT_AUDIOIN, m_szAudioIn);
	DDX_Text(pDX, IDC_EDIT_AUDIOOUT, m_szAudioOut);
	DDX_Text(pDX, IDC_STATIC_CONTRASE, m_Static_Contrast);
	DDX_Text(pDX, IDC_STATIC_GAMMA, m_Static_Gamma);
	DDX_Text(pDX, IDC_STATIC_SATURATION, m_Static_Saturation);
	DDX_Text(pDX, IDC_STATIC_BRIGHT, m_Static_Bright);
	DDX_Check(pDX, IDC_CHECK_ENHANCEMENT, m_bEnhancement);
	DDX_Text(pDX, IDC_EDIT_SENSITIVITY, m_nSensitivity);
	DDX_Text(pDX, IDC_EDIT_DELAY, m_nDelay);
	DDX_Text(pDX, IDC_EDIT_CORRECTION, m_nCorrection);
	DDX_Text(pDX, IDC_EDIT_SENSITIVE, m_nSensitive);
	DDX_Check(pDX, IDC_CHECK_IRISAUTO, m_bIrisAuto);
	DDX_Text(pDX, IDC_EDIT_LEVEL, m_nLevel);
	DDX_Text(pDX, IDC_EDIT_SHARPNESS, m_nSharpness);
	DDX_Text(pDX, IDC_EDIT_NOISELEVEL, m_nVideoNoise);
	DDX_Check(pDX, IDC_CHECK_2D, m_b2D);
	DDX_Text(pDX, IDC_EDIT_2DLEVEL, m_n2DLevel);
	DDX_Text(pDX, IDC_EDIT_SNFLEVEL, m_nSnfLevel);
	DDX_Text(pDX, IDC_EDIT_TNFLEVEL, m_nTnfLevel);
	DDX_Radio(pDX, IDC_RADIO_ELECTRON, m_nRadio);
	DDX_Text(pDX, IDC_EDIT_ENDTIME_HOUR, m_nEndHour);
	DDX_Text(pDX, IDC_EDIT_ENDTIME_MINUTE, m_nEndMinute);
	DDX_Text(pDX, IDC_EDIT_ENDTIME_SECOND, m_nEndSecond);
	DDX_Text(pDX, IDC_EDIT_STARTTIME_HOUR, m_nStartHour);
	DDX_Text(pDX, IDC_EDIT_STARTTIME_MINUTE, m_nStartMinute);
	DDX_Text(pDX, IDC_EDIT_STARTTIME_SECOND, m_nStartSecond);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImageTestDlg, CDialog)
	//{{AFX_MSG_MAP(CImageTestDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, OnBtnLogout)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELCOUNT, OnSelchangeComboChannelcount)
	ON_BN_CLICKED(IDC_SHARP_GET, OnSharpGet)
	ON_BN_CLICKED(IDC_SHARP_SET, OnSharpSet)
	ON_BN_CLICKED(IDC_BTN_GETCHANNELTITLE, OnBtnGetchanneltitle)
	ON_BN_CLICKED(IDC_BTN_SETCHANNELTITLE, OnBtnSetchanneltitle)
	ON_BN_CLICKED(IDC_BTN_GETAUDIO, OnBtnGetaudio)
	ON_BN_CLICKED(IDC_BTN_SETAUDIO, OnBtnSetaudio)
	ON_BN_CLICKED(IDC_BTN_GETMODE, OnBtnGetmode)
	ON_BN_CLICKED(IDC_BTN_SETMODE, OnBtnSetmode)
	ON_BN_CLICKED(IDC_BTN_MINUS_BRIGHT, OnBtnMinusBright)
	ON_BN_CLICKED(IDC_BTN_PLUS_BRIGHT, OnBtnPlusBright)
	ON_BN_CLICKED(IDC_BTN_MINUS_CONTRASE, OnBtnMinusContrase)
	ON_BN_CLICKED(IDC_BTN_PLUS_CONTRASE, OnBtnPlusContrase)
	ON_BN_CLICKED(IDC_BTN_MINUS_SATURATION, OnBtnMinusSaturation)
	ON_BN_CLICKED(IDC_BTN_PLUS_SATURATION, OnBtnPlusSaturation)
	ON_BN_CLICKED(IDC_BTN_MINUS_GAMMA, OnBtnMinusGamma)
	ON_BN_CLICKED(IDC_BTN_PLUS_GAMMA, OnBtnPlusGamma)
	ON_BN_CLICKED(IDC_BTN_GETVIDEOCOLOR, OnBtnGetvideocolor)
	ON_BN_CLICKED(IDC_BTN_SETVIDEOCOLOR, OnBtnSetvideocolor)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BRIGHT, OnCustomdrawSliderBright)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CONTRASE, OnCustomdrawSliderContrase)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SATURATION, OnCustomdrawSliderSaturation)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_GAMMA, OnCustomdrawSliderGamma)
	ON_BN_CLICKED(IDC_BTN_MINUS_ENHANCEMENT, OnBtnMinusEnhancement)
	ON_BN_CLICKED(IDC_BTN_PLUS_ENHANCEMENT, OnBtnPlusEnhancement)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_ENHANCEMENT, OnCustomdrawSliderEnhancement)
	ON_BN_CLICKED(IDC_BTN_GETENHANCEMENT, OnBtnGetenhancement)
	ON_BN_CLICKED(IDC_BTN_SETENHANCEMENT, OnBtnSetenhancement)
	ON_BN_CLICKED(IDC_BTN_GETDAYNIGHT, OnBtnGetdaynight)
	ON_BN_CLICKED(IDC_BTN_SETDAYNIGHT, OnBtnSetdaynight)
	ON_BN_CLICKED(IDC_BTN_GETLIGHTING, OnBtnGetlighting)
	ON_BN_CLICKED(IDC_BTN_SETLIGHTING, OnBtnSetlighting)
	ON_BN_CLICKED(IDC_BTN_GETSTABLE, OnBtnGetstable)
	ON_BN_CLICKED(IDC_BTN_SETSTABLE, OnBtnSetstable)
	ON_BN_CLICKED(IDC_BTN_GETIRIS, OnBtnGetiris)
	ON_BN_CLICKED(IDC_BTN_SETIRIS, OnBtnSetiris)
	ON_BN_CLICKED(IDC_BTN_BLACKLIGHT, OnBtnBlacklight)
	ON_BN_CLICKED(IDC_BTN_DEFOG, OnBtnDefog)
	ON_BN_CLICKED(IDC_BTN_EXPOSURE, OnBtnExposure)
	ON_BN_CLICKED(IDC_BTN_WHITEBALANCE, OnBtnWhitebalance)
	ON_BN_CLICKED(IDC_BTN_GETSHARPNESS, OnBtnGetsharpness)
	ON_BN_CLICKED(IDC_BTN_SETSHARPNESS, OnBtnSetsharpness)
	ON_CBN_SELCHANGE(IDC_COMBO_LIGHTMODE, OnSelchangeComboLightmode)
	ON_BN_CLICKED(IDC_BTN_GETVIDEONOISE, OnBtnGetvideonoise)
	ON_BN_CLICKED(IDC_BTN_SETVIDEONOISE, OnBtnSetvideonoise)
	ON_CBN_SELCHANGE(IDC_CMB_TYPE, OnSelchangeCmbType)
	ON_BN_CLICKED(IDC_BTN_AUDIOTYPE, OnBtnAudiotype)
	ON_BN_CLICKED(IDC_FOCUSVALUE, OnFocusvalue)
	ON_CBN_SELCHANGE(IDC_COMBO_ALGORITHMTYPE, OnSelchangeComboAlgorithmtype)
	ON_CBN_SELCHANGE(IDC_COMBO_VIDEONOISE, OnSelchangeComboVideonoise)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageTestDlg message handlers

void __stdcall DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CImageTestDlg *pThis = (CImageTestDlg *)dwUser;
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

	CImageTestDlg *pThis = (CImageTestDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}


BOOL CImageTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	int nIndex; 
	int i = 0; 
	CString ConfigMode[3]={"day","night","normal"}; 
	m_Cmb_Type.ResetContent(); 
	for(i=0;i<3;i++) 
	{ 
		nIndex = m_Cmb_Type.AddString(ConvertString(ConfigMode[i])); 
		m_Cmb_Type.SetItemData(nIndex,i+1); 
	} 

	CString strRate[3] = {"non-rotating","rotate 90 cw","rotate 90 ccw"};
	m_Cmb_Ratate.ResetContent();
	for(i=0; i < 3; i++)
	{
		nIndex = m_Cmb_Ratate.AddString(ConvertString(strRate[i]));
		m_Cmb_Ratate.SetItemData(nIndex,i+1);
	}

	CString strDayNight[3] = {"always color","auto switch based on brightness","alwasy B&W"};
	m_Cmb_DayNight.ResetContent();
	for(i=0; i < 3; i++)
	{
		nIndex = m_Cmb_DayNight.AddString(ConvertString(strDayNight[i]));
		m_Cmb_DayNight.SetItemData(nIndex,i+1);
	}

	CString strStable[4]={"close","DIS","OIS","control anti-shake"};
	m_Cmb_Stable.ResetContent();
	for(i=0; i < 4; i++)
	{
		nIndex = m_Cmb_Stable.AddString(ConvertString(strStable[i]));
		m_Cmb_Stable.SetItemData(nIndex,i+1);
	}

	CString strMode[5]={"always use day config","switch based on brightness","switch based on time","always use night config","use normal config"};
	m_Cmb_Mode.ResetContent();
	for(i=0; i < 5; i++)
	{
		nIndex = m_Cmb_Mode.AddString(ConvertString(strMode[i]));
		m_Cmb_Mode.SetItemData(nIndex,i+1);
	}

	CString strSharpnessMode[2] = {"automatic","manual"};
	m_Cmb_SharpnessMode.ResetContent();
	for(i=0; i < 2; i++)
	{
		nIndex = m_Cmb_SharpnessMode.AddString(ConvertString(strSharpnessMode[i]));
		m_Cmb_SharpnessMode.SetItemData(nIndex,i+1);
	}

	CString strLightMode[7] = {"unknown mode","manual mode","automatic mode","close mode","scale first mode","timing mode","SmartLight Mode"};
	m_Cmb_LightMode.ResetContent();
	for(i=0; i < 7; i++)
	{
		nIndex = m_Cmb_LightMode.AddString(ConvertString(strLightMode[i]));
		m_Cmb_LightMode.SetItemData(nIndex,i+1);
	}

	
	CString strVideoNoise[2] = {"close","automatic"};
    m_Cmb_VideoNoise.ResetContent();
    for(i=0; i < 2; i++)
	{
	   nIndex = m_Cmb_VideoNoise.AddString(ConvertString(strVideoNoise[i]));
	   m_Cmb_VideoNoise.SetItemData(nIndex,i+1);
	}

	CString strAlgorithmType[2] = {"close","manual"};
    m_Cmb_AlgorithmType.ResetContent();
    for(i=0; i < 2; i++)
	{
		nIndex = m_Cmb_AlgorithmType.AddString(ConvertString(strAlgorithmType[i]));
		m_Cmb_AlgorithmType.SetItemData(nIndex,i+1);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_DvrIPAddr.SetAddress(171,35,10,103);
	// TODO: Add extra initialization here
	CLIENT_Init(DisConnectFunc, LDWORD(this));
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);

    GetDlgItem(IDC_EDIT_CORRECTION)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_SENSITIVE)->EnableWindow(FALSE);

    GetDlgItem(IDC_EDIT_TNFLEVEL)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_SNFLEVEL)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT_NOISELEVEL)->EnableWindow(FALSE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CImageTestDlg::OnPaint() 
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
HCURSOR CImageTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

CString CImageTestDlg::GetDvrIP()
{
    CString strRet="";
    BYTE nField0,nField1,nField2,nField3;
    m_DvrIPAddr.GetAddress(nField0,nField1,nField2,nField3);
    strRet.Format("%d.%d.%d.%d",nField0,nField1,nField2,nField3);
    return strRet;
}

void CImageTestDlg::AddChannelCount(int nChannelCount)
{
    int nIndex = 0;
    CString str;
    m_Cmb_ChannelCount.ResetContent();
    for (int i= 0;i<nChannelCount; ++i)
    {
        str.Format("%d",i+1);
        nIndex = m_Cmb_ChannelCount.AddString(str);
        m_Cmb_ChannelCount.SetItemData(nIndex,i);
        
    }
    m_Cmb_ChannelCount.SetCurSel(0);
}
void CImageTestDlg::InitStu()
{
    memset(&m_stuVideoInImage,0,sizeof(m_stuVideoInImage));
    m_stuVideoInImage.dwSize = sizeof(m_stuVideoInImage);

    memset(&m_stuChannelTitle, 0,sizeof(m_stuChannelTitle));
    m_stuChannelTitle.dwSize = sizeof(m_stuChannelTitle);
}
void CImageTestDlg::InitDlg()
{
    m_Slider_Bright.SetRange(0,100);
    m_Slider_Contrast.SetRange(0,100);
    m_Slider_Gamma.SetRange(0,100);
    m_Slider_Saturation.SetRange(0,100);
    m_Slider_Enhancement.SetRange(0,100);
    m_Cmb_Type.SetCurSel(0);

    if (GetVideoIn(NET_EM_CFG_ENCODE_CHANNELTITLE))
    {
        StuToDlg(NET_EM_CFG_ENCODE_CHANNELTITLE);
    }
    if (GetVideoIn(NET_EM_CFG_AUDIOIN_DENOISE))
    {
        StuToDlg(NET_EM_CFG_AUDIOIN_DENOISE);
    }
    if (GetVideoIn(NET_EM_CFG_AUDIOIN_VOLUME))
    {
        StuToDlg(NET_EM_CFG_AUDIOIN_VOLUME);
    }
    if (GetVideoIn(NET_EM_CFG_AUDIOOUT_VOLUME))
    {
        StuToDlg(NET_EM_CFG_AUDIOOUT_VOLUME);
	}
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_LIGHTING))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_LIGHTING);
    }
    if (GetVideoIn(NET_EM_CFG_AUDIOIN_SOURCE))
    {
        StuToDlg(NET_EM_CFG_AUDIOIN_SOURCE);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_SWITCHMODE))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_SWITCHMODE);
    }
    else
    {
        GetAllConfigByType(NET_EM_CONFIG_DAYTIME);
        m_Cmb_Type.SetCurSel(0);
    }
}

void CImageTestDlg::OnBtnLogin() 
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
        InitDlg();        
        LLONG lRet = CLIENT_RealPlayEx(m_LoginID,0,GetDlgItem(IDC_REALPALY)->m_hWnd,DH_RType_Realplay_0);
        if(0 != lRet)
        {
            m_DispHanle = lRet;
        }
        else
        {
            MessageBox(ConvertString("Play failed!"), ConvertString("Prompt"));
        }
    }
    else
    {
        ShowLoginErrorReason(stOutparam.nError);
    }
	SetWindowText(ConvertString("Camera Attributes"));
}
void CImageTestDlg::ShowLoginErrorReason(int nError)
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

void CImageTestDlg::OnBtnLogout() 
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
	SetWindowText(ConvertString("Camera Attributes"));
}

void CImageTestDlg::OnSelchangeComboChannelcount() 
{
    CLIENT_StopRealPlay(m_DispHanle);
    m_DispHanle = 0;
    const int i= m_Cmb_ChannelCount.GetCurSel();
    if (m_nChannel == i)
    {
        return;
    }
    m_nChannel = i;
    InitDlg(); 
    LLONG lRet = CLIENT_RealPlayEx(m_LoginID,m_nChannel,GetDlgItem(IDC_REALPALY)->m_hWnd,DH_RType_Realplay_0);
    if(0 != lRet)
    {
         m_DispHanle = lRet;
    }
    else
    {
        MessageBox(ConvertString("Play failed!"), ConvertString("Prompt"));
    }
    
    Invalidate();
}

BOOL CImageTestDlg::GetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType,NET_EM_CONFIG_TYPE EmConfigType)
{
    BOOL bRet = FALSE;
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_IMAGE_OPT:
        {
            m_stuVideoInImage.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_IMAGE_OPT,m_nChannel,&m_stuVideoInImage,sizeof(m_stuVideoInImage),TIMEOUT);
            if (!bRet)
            {
                MessageBox(ConvertString("Get image attributes failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_ENCODE_CHANNELTITLE:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_ENCODE_CHANNELTITLE,m_nChannel,&m_stuChannelTitle,sizeof(m_stuChannelTitle),TIMEOUT);
            if (!bRet)
            {
                MessageBox(ConvertString("Get channel name failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_AUDIOIN_DENOISE:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_AUDIOIN_DENOISE,m_nChannel,&m_stuDeNoise,sizeof(m_stuDeNoise),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get audio denoise failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_AUDIOIN_VOLUME:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_AUDIOIN_VOLUME,m_nChannel,&m_stuAudioInVolume,sizeof(m_stuAudioInVolume),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get audio input volume failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_AUDIOOUT_VOLUME:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_AUDIOOUT_VOLUME,m_nChannel,&m_stuAudioOutVolume,sizeof(m_stuAudioOutVolume),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get audio output volume failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_SWITCHMODE:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_SWITCHMODE,m_nChannel,&m_stuVideoMode,sizeof(m_stuVideoMode),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get switch mode failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_COLOR:
        {
            m_stuVideoColor.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_COLOR,m_nChannel,&m_stuVideoColor,sizeof(m_stuVideoColor),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get video input color failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT,m_nChannel,&m_stuImageEnhancement,sizeof(m_stuImageEnhancement),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get image enhancement config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT:
        {
            m_stuDayNight.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_DAYNIGHT,m_nChannel,&m_stuDayNight,sizeof(m_stuDayNight),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get day and night switch config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR:
        {
            m_stuICR.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR,m_nChannel,&m_stuICR,sizeof(m_stuICR),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get day and night switch mode failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_LIGHTING:
        {
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_LIGHTING, m_nChannel,&m_stuLighting,sizeof(m_stuLighting),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get flash lamp config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_STABLE:
        {
            m_stuStable.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_STABLE,m_nChannel,&m_stuStable,sizeof(m_stuStable),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get anti-shake config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_IRISAUTO:
        {
            m_stuIrisAuto.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_IRISAUTO,m_nChannel,&m_stuIrisAuto,sizeof(m_stuIrisAuto),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get auto-iris config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_SHARPNESS:
        {
            m_stuSharpness.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_SHARPNESS,m_nChannel,&m_stuSharpness,sizeof(m_stuSharpness),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get Sharpness Config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_COMM_DENOISE:
        {
            m_stuVideoDenoise.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_COMM_DENOISE,m_nChannel,&m_stuVideoDenoise,sizeof(m_stuVideoDenoise),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Get video denoise config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_3D_DENOISE:
        {
            m_stuVideo3DDenoise.emCfgType = EmConfigType;
            bRet = CLIENT_GetConfig(m_LoginID, NET_EM_CFG_VIDEOIN_3D_DENOISE, m_nChannel, &m_stuVideo3DDenoise, sizeof(m_stuVideo3DDenoise),TIMEOUT);
            if (!bRet)
            {
                MessageBox(ConvertString("Get 3D denoise config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    default:
        break;
    }
    return bRet;
}

BOOL CImageTestDlg::SetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType,NET_EM_CONFIG_TYPE EmConfigType)
{
    BOOL bRet = FALSE;
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_IMAGE_OPT:
        {
            m_stuVideoInImage.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_IMAGE_OPT,m_nChannel,&m_stuVideoInImage,sizeof(m_stuVideoInImage),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set image attributes failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_ENCODE_CHANNELTITLE:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_ENCODE_CHANNELTITLE,m_nChannel,&m_stuChannelTitle,sizeof(m_stuChannelTitle),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set channel name failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_AUDIOIN_DENOISE:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_AUDIOIN_DENOISE,m_nChannel,&m_stuDeNoise,sizeof(m_stuDeNoise),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set audio denoise failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_AUDIOIN_VOLUME:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_AUDIOIN_VOLUME,m_nChannel,&m_stuAudioInVolume,sizeof(m_stuAudioInVolume),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set audio input volume failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_AUDIOOUT_VOLUME:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_AUDIOOUT_VOLUME,m_nChannel,&m_stuAudioOutVolume,sizeof(m_stuAudioOutVolume),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set audio output volume failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_SWITCHMODE:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_SWITCHMODE,m_nChannel,&m_stuVideoMode,sizeof(m_stuVideoMode),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set switch mode failed!"), ConvertString("Prompt"));
            }
            else
            {
                if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_WIGHT)
                {
                    GetAllConfigByType(NET_EM_CONFIG_DAYTIME);
                    m_Cmb_Type.SetCurSel(0);
                }
                else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_NIGHT)
                {
                    GetAllConfigByType(NET_EM_CONFIG_NIGHT);
                    m_Cmb_Type.SetCurSel(1);
                }
                else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_NORMAL)
                {
                    GetAllConfigByType(NET_EM_CONFIG_NORMAL);
                    m_Cmb_Type.SetCurSel(2);
                }
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_COLOR:
        {
            m_stuVideoColor.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_COLOR,m_nChannel,&m_stuVideoColor,sizeof(m_stuVideoColor),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set video input colour failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT,m_nChannel,&m_stuImageEnhancement,sizeof(m_stuImageEnhancement),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set image enhancement config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT:
        {
            m_stuDayNight.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_DAYNIGHT,m_nChannel,&m_stuDayNight,sizeof(m_stuDayNight),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set day and night swithc config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR:
        {
            m_stuICR.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR,m_nChannel,&m_stuICR,sizeof(m_stuICR),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set day and night swithc mode failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_LIGHTING:
        {
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_LIGHTING,m_nChannel,&m_stuLighting,sizeof(m_stuLighting),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set flash lamp config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_STABLE:
        {
            m_stuStable.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_STABLE,m_nChannel,&m_stuStable,sizeof(m_stuStable),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set anti-shake config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_IRISAUTO:
        {
            m_stuIrisAuto.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_IRISAUTO,m_nChannel,&m_stuIrisAuto,sizeof(m_stuIrisAuto),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set auto-iris config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_SHARPNESS:
        {
            m_stuSharpness.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_SHARPNESS,m_nChannel,&m_stuSharpness,sizeof(m_stuSharpness),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set Sharpness Config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_COMM_DENOISE:
        {
            m_stuVideoDenoise.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_VIDEOIN_COMM_DENOISE,m_nChannel,&m_stuVideoDenoise,sizeof(m_stuVideoDenoise),TIMEOUT);
            if (!bRet)
            {   
                MessageBox(ConvertString("Set denoise config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_3D_DENOISE:
        {
            m_stuVideo3DDenoise.emCfgType = EmConfigType;
            bRet = CLIENT_SetConfig(m_LoginID, NET_EM_CFG_VIDEOIN_3D_DENOISE, m_nChannel, &m_stuVideo3DDenoise, sizeof(m_stuVideo3DDenoise),TIMEOUT);
            if (!bRet)
            {
                MessageBox(ConvertString("Set 3D denoise config failed!"), ConvertString("Prompt"));
            }
            break;
        }
    default:
        break;
    }
    return bRet;
}

void CImageTestDlg::OnSharpGet() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_IMAGE_OPT,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_IMAGE_OPT);
    }
}

void CImageTestDlg::OnSharpSet() 
{
    DlgToStu(NET_EM_CFG_VIDEOIN_IMAGE_OPT);
	SetVideoIn(NET_EM_CFG_VIDEOIN_IMAGE_OPT,NET_EM_CONFIG_DAYTIME);
    SetVideoIn(NET_EM_CFG_VIDEOIN_IMAGE_OPT,NET_EM_CONFIG_NIGHT);
    SetVideoIn(NET_EM_CFG_VIDEOIN_IMAGE_OPT,NET_EM_CONFIG_NORMAL);
}

void CImageTestDlg::StuToDlg(NET_EM_CFG_OPERATE_TYPE emOperateType)
{
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_IMAGE_OPT:
        {
            ((CButton*)GetDlgItem(IDC_CHECK_MIRROR))->SetCheck(m_stuVideoInImage.bMirror);
            ((CButton*)GetDlgItem(IDC_CHECK_FLIP))->SetCheck(m_stuVideoInImage.bFlip);
            m_Cmb_Ratate.SetCurSel(m_stuVideoInImage.nRotate90);
            break;
        }
    case NET_EM_CFG_ENCODE_CHANNELTITLE:
        {
            m_szChannelTitle = m_stuChannelTitle.szChannelName;
            break;
        }
    case NET_EM_CFG_AUDIOIN_DENOISE:
        {
            ((CButton*)GetDlgItem(IDC_CHECK_DENOISE))->SetCheck(m_stuDeNoise.bEnable);
            break;
        }
    case NET_EM_CFG_AUDIOIN_VOLUME:
        {
            m_szAudioIn = m_stuAudioInVolume.nVolume;
            break;
        }
    case NET_EM_CFG_AUDIOOUT_VOLUME:
        {
            m_szAudioOut = m_stuAudioOutVolume.nVolume;
            break;
        }
    case NET_EM_CFG_VIDEOIN_SWITCHMODE:
        {
            m_Cmb_Mode.SetCurSel(m_stuVideoMode.emSwitchMode);
            if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_WIGHT)
            {
                GetAllConfigByType(NET_EM_CONFIG_DAYTIME);
                m_Cmb_Type.SetCurSel(0);
            }
            else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_NIGHT)
            {
                GetAllConfigByType(NET_EM_CONFIG_NIGHT);
                m_Cmb_Type.SetCurSel(1);
            }
            else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_NORMAL)
            {
                GetAllConfigByType(NET_EM_CONFIG_NORMAL);
                m_Cmb_Type.SetCurSel(2);
            }
            else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_TIME)
            {
                m_nStartHour = int(m_stuVideoMode.stuSunRiseTime.nHour);
                m_nStartMinute = int(m_stuVideoMode.stuSunRiseTime.nMinute);
                m_nStartSecond =  int(m_stuVideoMode.stuSunRiseTime.nSecond);
                int nStartSecond = 0;                           // 转换成秒换算大小
                nStartSecond = m_nStartHour*3600 + m_nStartMinute*60 + m_nStartSecond;
                m_nEndHour = m_stuVideoMode.stuSunSetTime.nHour;
                m_nEndMinute = m_stuVideoMode.stuSunSetTime.nMinute;
                m_nEndSecond =  int(m_stuVideoMode.stuSunSetTime.nSecond);
//                 CTime enTime(1970,12,12,hour,minute,second,0);
//                 m_EndTime.SetTime(&enTime);
                CTime LocalTime = CTime::GetCurrentTime();
                int nEndSecond = 0;                           // 转换成秒换算大小
                nEndSecond = m_nEndHour*3600 + m_nEndMinute*60 + m_nEndSecond;
                int Localhour = LocalTime.GetHour();
                int Localminute = LocalTime.GetMinute();
                int Localsecond = LocalTime.GetSecond();
                int nLocalTime = 0;                           // 转换成秒换算大小
                nLocalTime = Localhour*3600 + Localminute*60 + Localsecond;
                if ((nLocalTime >= nStartSecond) &&(nLocalTime <= nEndSecond))
                {
                    GetAllConfigByType(NET_EM_CONFIG_DAYTIME);
                    m_Cmb_Type.SetCurSel(0);
                }
                else
                {
                    GetAllConfigByType(NET_EM_CONFIG_NIGHT);
                    m_Cmb_Type.SetCurSel(1);
                }
              
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_COLOR:
        {
            m_Slider_Bright.SetPos(m_stuVideoColor.nBrightness);
            m_Static_Bright.Format("%d",m_stuVideoColor.nBrightness);
            m_Slider_Contrast.SetPos(m_stuVideoColor.nContrast);
            m_Static_Contrast.Format("%d",m_stuVideoColor.nContrast);
            m_Slider_Saturation.SetPos(m_stuVideoColor.nSaturation);
            m_Static_Saturation.Format("%d",m_stuVideoColor.nSaturation);
            m_Slider_Gamma.SetPos(m_stuVideoColor.nGamma);
            m_Static_Gamma.Format("%d",m_stuVideoColor.nGamma);
            break;
        }
    case NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT:
        {
            m_bEnhancement = m_stuImageEnhancement.bEnable;
            m_Slider_Enhancement.SetPos(m_stuImageEnhancement.nLevel);
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT:
        {
            if (m_stuDayNight.emDayNightType > 2)
            {
                m_Cmb_DayNight.SetCurSel(m_stuDayNight.emDayNightType-1);
            }
            else
            {
                m_Cmb_DayNight.SetCurSel(m_stuDayNight.emDayNightType);
            }
            m_nSensitivity = m_stuDayNight.nDayNightSensitivity;
            m_nDelay = m_stuDayNight.nDayNightSwitchDelay;
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR:
        {
            m_nRadio = m_stuICR.emType -1;
            break;
        }
    case NET_EM_CFG_VIDEOIN_LIGHTING:
        {
            m_Cmb_LightMode.SetCurSel(m_stuLighting.emLightMode);
            m_nSensitive = m_stuLighting.nSensitive;
            m_nCorrection = m_stuLighting.nCorrection;
            if (m_stuLighting.emLightMode == NET_EM_LIGHTING_ZOOMPRIO)
            {
                GetDlgItem(IDC_EDIT_CORRECTION)->EnableWindow(TRUE);
                GetDlgItem(IDC_EDIT_SENSITIVE)->EnableWindow(TRUE);
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_STABLE:
        {
            if (m_stuStable.emStableType == NET_EM_STABLE_CONTORL)
            {
                m_Cmb_Stable.SetCurSel(m_stuStable.emStableType -1);
            }
            else
            {
                m_Cmb_Stable.SetCurSel(m_stuStable.emStableType);
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_IRISAUTO:
        {
            m_bIrisAuto = m_stuIrisAuto.bIrisAuto;
            break;
        }
    case NET_EM_CFG_VIDEOIN_SHARPNESS:
        {
            m_Cmb_SharpnessMode.SetCurSel(m_stuSharpness.emSharpnessMode);
            m_nSharpness = m_stuSharpness.nSharpness;
            m_nLevel = m_stuSharpness.nLevel;
            break;
        }
    case NET_EM_CFG_VIDEOIN_COMM_DENOISE:
        {
            m_b2D = m_stuVideoDenoise.b2DEnable;
            m_n2DLevel = m_stuVideoDenoise.n2DLevel;
            m_Cmb_AlgorithmType.SetCurSel(m_stuVideoDenoise.stuAlgorithm.emAlgorithmType -1);
            if (m_stuVideoDenoise.stuAlgorithm.emAlgorithmType == NET_EM_ALGORITHM_MANUAL)
            {
                GetDlgItem(IDC_EDIT_TNFLEVEL)->EnableWindow(TRUE);
                GetDlgItem(IDC_EDIT_SNFLEVEL)->EnableWindow(TRUE);
            }
            else
            {
                GetDlgItem(IDC_EDIT_TNFLEVEL)->EnableWindow(FALSE);
                GetDlgItem(IDC_EDIT_SNFLEVEL)->EnableWindow(FALSE);
            }
            m_nSnfLevel = m_stuVideoDenoise.stuAlgorithm.nSnfLevel;
            m_nTnfLevel = m_stuVideoDenoise.stuAlgorithm.nTnfLevel;
            break;
        }
    case NET_EM_CFG_VIDEOIN_3D_DENOISE:
        {
            m_Cmb_VideoNoise.SetCurSel(m_stuVideo3DDenoise.em3DType -1);
            if(m_stuVideo3DDenoise.em3DType == NET_EM_3D_AUTO)
            {
                GetDlgItem(IDC_EDIT_NOISELEVEL)->EnableWindow(TRUE);
            }
            else
            {
                GetDlgItem(IDC_EDIT_NOISELEVEL)->EnableWindow(FALSE);
            }
            m_nVideoNoise = m_stuVideo3DDenoise.nAutoLevel;
            break;
        }
    default:
        break;
    }
    UpdateData(FALSE);
}
void CImageTestDlg::DlgToStu(NET_EM_CFG_OPERATE_TYPE emOperateType)
{
    UpdateData();
    switch(emOperateType)
    {
    case NET_EM_CFG_VIDEOIN_IMAGE_OPT:
        {
            if (((CButton*)GetDlgItem(IDC_CHECK_MIRROR))->GetCheck())
            {
                m_stuVideoInImage.bMirror = TRUE;
            }
            else
            {
                m_stuVideoInImage.bMirror = FALSE;
            }
            if (((CButton*)GetDlgItem(IDC_CHECK_FLIP))->GetCheck())
            {
                m_stuVideoInImage.bFlip = TRUE;
            }
            else
            {
                m_stuVideoInImage.bFlip = FALSE;
            }
            m_stuVideoInImage.nRotate90 = m_Cmb_Ratate.GetCurSel();
            break;
        }
    case NET_EM_CFG_ENCODE_CHANNELTITLE:
        {
            memset(&m_stuChannelTitle, 0,sizeof(m_stuChannelTitle));
            m_stuChannelTitle.dwSize = sizeof(m_stuChannelTitle);
			strncpy(m_stuChannelTitle.szChannelName, m_szChannelTitle.GetBuffer(), sizeof(m_stuChannelTitle.szChannelName) - 1);
			m_szChannelTitle.ReleaseBuffer();           
            break;
        }
    case NET_EM_CFG_AUDIOIN_DENOISE:
        {
            m_stuDeNoise.bEnable = ((CButton*)GetDlgItem(IDC_CHECK_DENOISE))->GetCheck();
            break;
        }
    case NET_EM_CFG_AUDIOIN_VOLUME:
        {
            m_stuAudioInVolume.nVolume = m_szAudioIn;
            break;
        }
    case NET_EM_CFG_AUDIOOUT_VOLUME:
        {
            m_stuAudioOutVolume.nVolume = m_szAudioOut;
            break;
        }
    case NET_EM_CFG_VIDEOIN_SWITCHMODE:
        {

            m_stuVideoMode.emSwitchMode = (NET_EM_SWITCH_MODE)m_Cmb_Mode.GetCurSel();

            m_stuVideoMode.stuSunRiseTime.nHour = m_nStartHour;
            m_stuVideoMode.stuSunRiseTime.nMinute = m_nStartMinute;
            m_stuVideoMode.stuSunRiseTime.nSecond = m_nStartSecond;

            m_stuVideoMode.stuSunSetTime.nHour = m_nEndHour;
            m_stuVideoMode.stuSunSetTime.nMinute = m_nEndMinute;
            m_stuVideoMode.stuSunSetTime.nSecond = m_nEndSecond;

            if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_WIGHT)
            {
                GetAllConfigByType(NET_EM_CONFIG_DAYTIME);
                m_Cmb_Type.SetCurSel(0);
            }
            else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_NIGHT)
            {
                GetAllConfigByType(NET_EM_CONFIG_NIGHT);
                m_Cmb_Type.SetCurSel(1);
            }
            else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_NORMAL)
            {
                GetAllConfigByType(NET_EM_CONFIG_NORMAL);
                m_Cmb_Type.SetCurSel(2);
            }
            else if (m_stuVideoMode.emSwitchMode == NET_EM_SWITCH_MODE_TIME)
            {
                m_nStartHour = int(m_stuVideoMode.stuSunRiseTime.nHour);
                m_nStartMinute = int(m_stuVideoMode.stuSunRiseTime.nMinute);
                m_nStartSecond =  int(m_stuVideoMode.stuSunRiseTime.nSecond);
//                 CTime stTime(1970,12,12,hour,minute,second,0);
//                 m_StartTime.SetTime(&stTime);
                int nStartSecond = 0;                           // 转换成秒换算大小
                nStartSecond = m_nStartHour*3600 + m_nStartMinute*60 + m_nStartSecond;
                m_nEndHour = m_stuVideoMode.stuSunSetTime.nHour;
                m_nEndMinute = m_stuVideoMode.stuSunSetTime.nMinute;
                m_nEndSecond =  int(m_stuVideoMode.stuSunSetTime.nSecond);

                CTime LocalTime = CTime::GetCurrentTime();
                int nEndSecond = 0;                           // 转换成秒换算大小
                nEndSecond = m_nEndHour*3600 + m_nEndMinute*60 + m_nEndSecond;
                int Localhour = LocalTime.GetHour();
                int Localminute = LocalTime.GetMinute();
                int Localsecond = LocalTime.GetSecond();
                int nLocalTime = 0;                           // 转换成秒换算大小
                nLocalTime = Localhour*3600 + Localminute*60 + Localsecond;
                if ((nLocalTime >= nStartSecond) &&(nLocalTime <= nEndSecond))
                {
                    GetAllConfigByType(NET_EM_CONFIG_DAYTIME);
                    m_Cmb_Type.SetCurSel(0);
                }
                else
                {
                    GetAllConfigByType(NET_EM_CONFIG_NIGHT);
                    m_Cmb_Type.SetCurSel(1);
                }
                
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_COLOR:
        {
            m_stuVideoColor.nBrightness = m_Slider_Bright.GetPos();
            m_stuVideoColor.nContrast = m_Slider_Contrast.GetPos();
            m_stuVideoColor.nSaturation = m_Slider_Saturation.GetPos();
            m_stuVideoColor.nGamma = m_Slider_Gamma.GetPos();
            break;
        }
    case NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT:
        {
            m_stuImageEnhancement.bEnable = m_bEnhancement;
            m_stuImageEnhancement.nLevel = m_Slider_Enhancement.GetPos();
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT:
        {
            if (m_Cmb_DayNight.GetCurSel()>2)
            {
                m_stuDayNight.emDayNightType = (NET_EM_DAYNIGHT_TYPE)(m_Cmb_DayNight.GetCurSel()+1);
            }
            else
            {
                m_stuDayNight.emDayNightType = (NET_EM_DAYNIGHT_TYPE)m_Cmb_DayNight.GetCurSel();
            }
            m_stuDayNight.nDayNightSensitivity = m_nSensitivity;
            m_stuDayNight.nDayNightSwitchDelay = m_nDelay;
            break;
        }
    case NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR:
        {
            m_stuICR.emType = (NET_EM_ICR_TYPE)(m_nRadio+1);
            break;
        }
    case NET_EM_CFG_VIDEOIN_LIGHTING:
        {
            m_stuLighting.emLightMode = (NET_EM_LIGHTING_MODE)m_Cmb_LightMode.GetCurSel();
            m_stuLighting.nSensitive = m_nSensitive;
            m_stuLighting.nCorrection = m_nCorrection;
            break;
        }
    case NET_EM_CFG_VIDEOIN_STABLE:
        {
            if (m_Cmb_Stable.GetCurSel() == 3)
            {
                m_stuStable.emStableType = NET_EM_STABLE_CONTORL;
            }
            else
            {
                m_stuStable.emStableType = (NET_EM_STABLE_TYPE)m_Cmb_Stable.GetCurSel();
            }
            break;
        }
    case NET_EM_CFG_VIDEOIN_IRISAUTO:
        {
            m_stuIrisAuto.bIrisAuto = m_bIrisAuto;
            break;
        }
    case NET_EM_CFG_VIDEOIN_SHARPNESS:
        {
            m_stuSharpness.emSharpnessMode = (NET_EM_SHARPNESS_MODE)m_Cmb_SharpnessMode.GetCurSel();
            m_stuSharpness.nSharpness = m_nSharpness;
            m_stuSharpness.nLevel = m_nLevel;
            break;
        }
    case NET_EM_CFG_VIDEOIN_COMM_DENOISE:
        {
            m_stuVideoDenoise.b2DEnable = m_b2D;
            m_stuVideoDenoise.n2DLevel = m_n2DLevel;
            m_stuVideoDenoise.stuAlgorithm.emAlgorithmType = NET_EM_ALGORITHM_TYPE(m_Cmb_AlgorithmType.GetCurSel()+1);
            m_stuVideoDenoise.stuAlgorithm.nSnfLevel = m_nSnfLevel;
            m_stuVideoDenoise.stuAlgorithm.nTnfLevel = m_nTnfLevel;
            break;
        }
    case NET_EM_CFG_VIDEOIN_3D_DENOISE:
        {
            m_stuVideo3DDenoise.em3DType = (NET_EM_3D_TYPE)(m_Cmb_VideoNoise.GetCurSel()+1);
            m_stuVideo3DDenoise.nAutoLevel = m_nVideoNoise;
            break;
        }
    default:
        break;
    }
}

void CImageTestDlg::OnBtnGetchanneltitle() 
{
    if (GetVideoIn(NET_EM_CFG_ENCODE_CHANNELTITLE))
    {
        StuToDlg(NET_EM_CFG_ENCODE_CHANNELTITLE);
    }
}

void CImageTestDlg::OnBtnSetchanneltitle() 
{
    DlgToStu(NET_EM_CFG_ENCODE_CHANNELTITLE);
    SetVideoIn(NET_EM_CFG_ENCODE_CHANNELTITLE);
}

void CImageTestDlg::OnBtnGetaudio() 
{
	if (GetVideoIn(NET_EM_CFG_AUDIOIN_DENOISE))
	{
        StuToDlg(NET_EM_CFG_AUDIOIN_DENOISE);
	}
    if (GetVideoIn(NET_EM_CFG_AUDIOIN_VOLUME))
    {
        StuToDlg(NET_EM_CFG_AUDIOIN_VOLUME);
	}
    if (GetVideoIn(NET_EM_CFG_AUDIOOUT_VOLUME))
    {
        StuToDlg(NET_EM_CFG_AUDIOOUT_VOLUME);
	}
    if (GetVideoIn(NET_EM_CFG_AUDIOIN_SOURCE))
    {
        StuToDlg(NET_EM_CFG_AUDIOIN_SOURCE);
    }
}

void CImageTestDlg::OnBtnSetaudio() 
{
    DlgToStu(NET_EM_CFG_AUDIOIN_DENOISE);
    SetVideoIn(NET_EM_CFG_AUDIOIN_DENOISE);
    DlgToStu(NET_EM_CFG_AUDIOIN_VOLUME);
    SetVideoIn(NET_EM_CFG_AUDIOIN_VOLUME);
    DlgToStu(NET_EM_CFG_AUDIOOUT_VOLUME);
    SetVideoIn(NET_EM_CFG_AUDIOOUT_VOLUME);
}

void CImageTestDlg::OnBtnGetmode() 
{
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_SWITCHMODE))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_SWITCHMODE);
	}
}

void CImageTestDlg::OnBtnSetmode() 
{
	UpdateData(TRUE);
	if (m_nStartHour >= 24 || m_nStartHour < 0 ||
		m_nStartMinute > 59 || m_nStartMinute < 0 ||
		m_nStartSecond > 59 || m_nStartSecond < 0 ||
		m_nEndHour >= 24 || m_nEndHour < 0 ||
		m_nEndMinute > 59 || m_nEndMinute < 0 ||
		m_nEndSecond > 59 || m_nEndSecond < 0  )
	{
		MessageBox(ConvertString("Please input correct time!"), ConvertString("Prompt"));
		return;
	}

	DlgToStu(NET_EM_CFG_VIDEOIN_SWITCHMODE);
    SetVideoIn(NET_EM_CFG_VIDEOIN_SWITCHMODE);
}

void CImageTestDlg::OnBtnMinusBright() 
{
    int nPos = m_Slider_Bright.GetPos();
    if (nPos<=0)
    {
        return;
    }
    nPos--;
	m_Slider_Bright.SetPos(nPos);
    m_Static_Bright.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnPlusBright() 
{
    int nPos = m_Slider_Bright.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Bright.SetPos(nPos);
    m_Static_Bright.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnMinusContrase() 
{
    int nPos = m_Slider_Contrast.GetPos();
    if (nPos<=0)
    {
        return;
    }
    nPos--;
    m_Slider_Contrast.SetPos(nPos);
    m_Static_Contrast.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnPlusContrase() 
{
    int nPos = m_Slider_Contrast.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Contrast.SetPos(nPos);
    m_Static_Contrast.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnMinusSaturation() 
{
    int nPos = m_Slider_Saturation.GetPos();
    if (nPos<=0)
    {
        return;
    }
    nPos--;
    m_Slider_Saturation.SetPos(nPos);
    m_Static_Saturation.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnPlusSaturation() 
{
    int nPos = m_Slider_Saturation.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Saturation.SetPos(nPos);
    m_Static_Saturation.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnMinusGamma() 
{
    int nPos = m_Slider_Gamma.GetPos();
    if (nPos<=0)
    {
        return;
    }
    nPos--;
    m_Slider_Gamma.SetPos(nPos);
    m_Static_Gamma.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnPlusGamma() 
{
    int nPos = m_Slider_Gamma.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Gamma.SetPos(nPos);
    m_Static_Gamma.Format("%d",nPos);
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnGetvideocolor() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_COLOR,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_COLOR);
    }
}

void CImageTestDlg::OnBtnSetvideocolor() 
{
    int i = m_Cmb_Type.GetCurSel();
	DlgToStu(NET_EM_CFG_VIDEOIN_COLOR);
    SetVideoIn(NET_EM_CFG_VIDEOIN_COLOR,NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnCustomdrawSliderBright(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	int nPos = m_Slider_Bright.GetPos();
    m_Static_Bright.Format("%d",nPos);
	*pResult = 0;

    UpdateData(FALSE);
}

void CImageTestDlg::OnCustomdrawSliderContrase(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Contrast.GetPos();
    m_Static_Contrast.Format("%d",nPos);
    *pResult = 0;
    
    UpdateData(FALSE);
}

void CImageTestDlg::OnCustomdrawSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 int nPos = m_Slider_Saturation.GetPos();
    m_Static_Saturation.Format("%d",nPos);
    *pResult = 0;
    
    UpdateData(FALSE);
}

void CImageTestDlg::OnCustomdrawSliderGamma(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Gamma.GetPos();
    m_Static_Gamma.Format("%d",nPos);
    *pResult = 0;
    
    UpdateData(FALSE);
}

void CImageTestDlg::OnBtnMinusEnhancement() 
{
    int nPos = m_Slider_Enhancement.GetPos();
    if (nPos<=0)
    {
        return;
    }
    nPos--;
    m_Slider_Enhancement.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_ENHANCEMENT,str);
}

void CImageTestDlg::OnBtnPlusEnhancement() 
{
    int nPos = m_Slider_Enhancement.GetPos();
    if (nPos>=100)
    {
        return;
    }
    nPos++;
    m_Slider_Enhancement.SetPos(nPos);
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_ENHANCEMENT,str);
}

void CImageTestDlg::OnCustomdrawSliderEnhancement(NMHDR* pNMHDR, LRESULT* pResult) 
{
    int nPos = m_Slider_Enhancement.GetPos();
    CString str;
    str.Format("%d",nPos);
    SetDlgItemText(IDC_STATIC_ENHANCEMENT,str);
    *pResult = 0;
}

void CImageTestDlg::OnBtnGetenhancement() 
{
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT);
    }
}

void CImageTestDlg::OnBtnSetenhancement() 
{
    DlgToStu(NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT);
    SetVideoIn(NET_EM_CFG_VIDEOIN_IMAGEENHANCEMENT);
}

void CImageTestDlg::OnBtnGetdaynight() 
{
    int i= m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_DAYNIGHT,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_DAYNIGHT);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR);
    }
}

void CImageTestDlg::OnBtnSetdaynight() 
{
    int i= m_Cmb_Type.GetCurSel();
	DlgToStu(NET_EM_CFG_VIDEOIN_DAYNIGHT);
    SetVideoIn(NET_EM_CFG_VIDEOIN_DAYNIGHT,NET_EM_CONFIG_TYPE(i));
    DlgToStu(NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR);
    SetVideoIn(NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR,NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnBtnGetlighting() 
{
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_LIGHTING))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_LIGHTING);
    }

	int i= m_Cmb_LightMode.GetCurSel();
	if (i == NET_EM_LIGHTING_ZOOMPRIO)
	{
		GetDlgItem(IDC_EDIT_CORRECTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SENSITIVE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_CORRECTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SENSITIVE)->EnableWindow(FALSE);
	}
}

void CImageTestDlg::OnBtnSetlighting() 
{
    DlgToStu(NET_EM_CFG_VIDEOIN_LIGHTING);
    SetVideoIn(NET_EM_CFG_VIDEOIN_LIGHTING);
}

void CImageTestDlg::OnBtnGetstable() 
{
    int i = m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_STABLE,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_STABLE);
    }
}

void CImageTestDlg::OnBtnSetstable() 
{
    int i = m_Cmb_Type.GetCurSel();
	DlgToStu(NET_EM_CFG_VIDEOIN_STABLE);
    SetVideoIn(NET_EM_CFG_VIDEOIN_STABLE,NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnBtnGetiris() 
{
    int i= m_Cmb_Type.GetCurSel();
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_IRISAUTO,NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_IRISAUTO);
    }
}

void CImageTestDlg::OnBtnSetiris() 
{
    int i= m_Cmb_Type.GetCurSel();
    DlgToStu(NET_EM_CFG_VIDEOIN_IRISAUTO);
    SetVideoIn(NET_EM_CFG_VIDEOIN_IRISAUTO,NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnBtnBlacklight() 
{
	BlackLight dlg(NULL,m_nChannel,m_LoginID);
    dlg.DoModal();
}

void CImageTestDlg::OnBtnDefog() 
{
    CDefog dlg(NULL,m_nChannel,m_LoginID);
    dlg.DoModal();
}

void CImageTestDlg::OnBtnExposure() 
{
    CExposureDlg dlg(NULL,m_nChannel,m_LoginID);
    dlg.DoModal();
}

void CImageTestDlg::OnBtnWhitebalance() 
{
    CWhiteBalanceDlg dlg(NULL,m_nChannel,m_LoginID);
    dlg.DoModal();
}

void CImageTestDlg::OnBtnGetsharpness() 
{
    int i = m_Cmb_Type.GetCurSel();
	if (GetVideoIn(NET_EM_CFG_VIDEOIN_SHARPNESS,NET_EM_CONFIG_TYPE(i)))
	{
        StuToDlg(NET_EM_CFG_VIDEOIN_SHARPNESS);
	}
}

void CImageTestDlg::OnBtnSetsharpness() 
{
    int i = m_Cmb_Type.GetCurSel();
	DlgToStu(NET_EM_CFG_VIDEOIN_SHARPNESS);
    SetVideoIn(NET_EM_CFG_VIDEOIN_SHARPNESS,NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnSelchangeComboLightmode() 
{
    int i= m_Cmb_LightMode.GetCurSel();
    if (i == NET_EM_LIGHTING_ZOOMPRIO)
    {
        GetDlgItem(IDC_EDIT_CORRECTION)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_SENSITIVE)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_CORRECTION)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_SENSITIVE)->EnableWindow(FALSE);
    }
}

void CImageTestDlg::OnBtnGetvideonoise() 
{
    int i= m_Cmb_Type.GetCurSel();
	if (GetVideoIn(NET_EM_CFG_VIDEOIN_COMM_DENOISE,NET_EM_CONFIG_TYPE(i)))
	{
        StuToDlg(NET_EM_CFG_VIDEOIN_COMM_DENOISE);
	}
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_3D_DENOISE, NET_EM_CONFIG_TYPE(i)))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_3D_DENOISE);
    }
}

void CImageTestDlg::OnBtnSetvideonoise() 
{
    int i= m_Cmb_Type.GetCurSel();
	DlgToStu(NET_EM_CFG_VIDEOIN_COMM_DENOISE);
    DlgToStu(NET_EM_CFG_VIDEOIN_3D_DENOISE);
    SetVideoIn(NET_EM_CFG_VIDEOIN_COMM_DENOISE,NET_EM_CONFIG_TYPE(i));
    SetVideoIn(NET_EM_CFG_VIDEOIN_3D_DENOISE,NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnSelchangeCmbType() 
{
    int i = m_Cmb_Type.GetCurSel();
    GetAllConfigByType(NET_EM_CONFIG_TYPE(i));
}

void CImageTestDlg::OnBtnAudiotype() 
{
	CAudioSource dlg(NULL,m_nChannel,m_LoginID);
    dlg.DoModal();
}

void CImageTestDlg::GetAllConfigByType(NET_EM_CONFIG_TYPE EmConfigType)
{
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_IMAGE_OPT,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_IMAGE_OPT);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_COLOR,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_COLOR);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_DAYNIGHT,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_DAYNIGHT);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_DAYNIGHT_ICR);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_STABLE,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_STABLE);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_SHARPNESS,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_SHARPNESS);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_COMM_DENOISE,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_COMM_DENOISE);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_3D_DENOISE, EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_3D_DENOISE);
    }
    if (GetVideoIn(NET_EM_CFG_VIDEOIN_IRISAUTO,EmConfigType))
    {
        StuToDlg(NET_EM_CFG_VIDEOIN_IRISAUTO);
    }
}

void CImageTestDlg::OnFocusvalue() 
{
	CFocusValueDlg Dlg(NULL,m_nChannel,m_LoginID);
    Dlg.DoModal();
}

void CImageTestDlg::OnSelchangeComboAlgorithmtype() 
{
	int i = m_Cmb_AlgorithmType.GetCurSel();
    if (1 == i)
    {
        GetDlgItem(IDC_EDIT_TNFLEVEL)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_SNFLEVEL)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_TNFLEVEL)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_SNFLEVEL)->EnableWindow(FALSE);
    }
}

void CImageTestDlg::OnSelchangeComboVideonoise() 
{
	int i = m_Cmb_VideoNoise.GetCurSel();
    if (1 == i)
    {
        GetDlgItem(IDC_EDIT_NOISELEVEL)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_NOISELEVEL)->EnableWindow(FALSE);
    }
}


//Process when device disconnected 
LRESULT CImageTestDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CImageTestDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Camera Attributes"));
	return 0;
}

BOOL CImageTestDlg::PreTranslateMessage(MSG* pMsg) 
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
void CImageTestDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (0 != m_DispHanle)
	{
		CLIENT_StopRealPlay(m_DispHanle);
	}
	
	CLIENT_Cleanup();
}
