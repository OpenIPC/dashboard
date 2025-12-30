// TestOSDDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestOSD.h"
#include "TestOSDDlg.h"
#include "SetCustomTextDlg.h"
#include "SetPTZZoom.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)

HANDLE g_hConsole = INVALID_HANDLE_VALUE;

ComboItemInfo arComboItemInfo[] = {
//{"未知叠加类型",NET_EM_OSD_BLEND_TYPE_UNKNOWN},
{"Overlay to the Major Stream",NET_EM_OSD_BLEND_TYPE_MAIN},
{"Overlay to the Minor Stream 1",NET_EM_OSD_BLEND_TYPE_EXTRA1},
{"Overlay to the Minor Stream 2",NET_EM_OSD_BLEND_TYPE_EXTRA2},
{"Overlay to the Minor Stream 3",NET_EM_OSD_BLEND_TYPE_EXTRA3},
{"Overlay to the Capture Stream",NET_EM_OSD_BLEND_TYPE_SNAPSHOT},
{"Overlay to the Preview Stream",NET_EM_OSD_BLEND_TYPE_PREVIEW}};

ComboItemInfo arComboItemInfo2[] = {
//{"无效的对齐方式",EM_TEXT_ALIGNTYPE_INVALID},
{"Align Left",EM_TEXT_ALIGNTYPE_LEFT},
{"X-coordinate Center Align",EM_TEXT_ALIGNTYPE_XCENTER},
{"Y-coordinate Center Align",EM_TEXT_ALIGNTYPE_YCENTER},
{"Center",EM_TEXT_ALIGNTYPE_CENTER},
{"Align right",EM_TEXT_ALIGNTYPE_RIGHT},
{"Align top",EM_TEXT_ALIGNTYPE_TOP},
{"Align bottom",EM_TEXT_ALIGNTYPE_BOTTOM},
{"Align top left",EM_TEXT_ALIGNTYPE_LEFTTOP},
{"Line align",EM_TEXT_ALIGNTYPE_CHANGELINE}};

int arCoverChannelID[] = {
    IDC_CHECK_COVER_CHANNEL_MAIN,
    IDC_CHECK_COVER_CHANNEL_EXTRA1,
    IDC_CHECK_COVER_CHANNEL_EXTRA2,
    IDC_CHECK_COVER_CHANNEL_EXTRA3,
    IDC_CHECK_COVER_CHANNEL_SNAP,
    IDC_CHECK_COVER_CHANNEL_PREVIEW
};

int arCoverTimeID[] = {
    IDC_CHECK_COVER_TIME_MAIN,
    IDC_CHECK_COVER_TIME_EXTRA1,
    IDC_CHECK_COVER_TIME_EXTRA2,
    IDC_CHECK_COVER_TIME_EXTRA3,
    IDC_CHECK_COVER_TIME_SNAP,
    IDC_CHECK_COVER_TIME_PREVIEW    
};

#define ZERO_STRUCT(st) ZeroMemory(&st,sizeof(st))
#define SET_DWSIZE(st,b) {if(b){ZERO_STRUCT(st);}st.dwSize = sizeof(st);}

LLONG g_lLogin = 0;
LLONG g_lPlay = 0;
BOOL g_bPlayWndMax = FALSE;
RECT g_InitRect = {0};
unsigned int g_iChannelIndex = 0;

NET_OSD_CHANNEL_TITLE g_stChannelTitle = {sizeof(g_stChannelTitle)};
NET_OSD_TIME_TITLE g_stTimeTitle = {sizeof(g_stTimeTitle)};
NET_OSD_COMM_INFO g_stCommonInfo = {sizeof(g_stCommonInfo)};


void InitCombo(CDialog* pDlg,UINT uComboID,ComboItemInfo arInfo[],int iCount)
{
    if (pDlg == NULL)
    {
        return;
    }

    if (arInfo == NULL)
    {
        return;
    }

    if (iCount <= 0)
    {
        return;
    }

    CComboBox* pCombo = (CComboBox*)pDlg->GetDlgItem(uComboID);
    if (pCombo == NULL)
    {
        return;
    }

    pCombo->ResetContent();
    for (int i = 0;i < iCount;i++)
    {
        pCombo->AddString(ConvertString(arInfo[i].szText));
        pCombo->SetItemData(i,arInfo[i].iValue);
    }
    pCombo->SetCurSel(0);

    return;
}

void SetComboWithValue(CDialog* pDlg,UINT uComboID,unsigned int iValue)
{
     CComboBox* pCombo = (CComboBox*)pDlg->GetDlgItem(uComboID);
     for (int i = 0;i < pCombo->GetCount();i++)
     {
         if (iValue == pCombo->GetItemData(i))
         {
             pCombo->SetCurSel(i);
             break;
         }
     }
}


//Callback function when device disconnected 
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CTestOSDDlg *pThis = (CTestOSDDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);		
}

//Callback function when device reconnect 
void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CTestOSDDlg *pThis = (CTestOSDDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);		
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CTestOSDDlg 对话框




CTestOSDDlg::CTestOSDDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestOSDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);   
}


CTestOSDDlg::~CTestOSDDlg()
{
    CLIENT_StopRealPlay(g_lPlay);
    CLIENT_Logout(g_lLogin);

    CLIENT_Cleanup();
}


void CTestOSDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestOSDDlg, CDialog)
	//{{AFX_MSG_MAP(CTestOSDDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()	
    ON_BN_CLICKED(IDC_BUTTON_START_PLAY, OnBnClickedButtonLogin)
    ON_STN_DBLCLK(IDC_STATIC_PIC, OnStnDblclickStaticPic)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_WINDOWPOSCHANGING()
	ON_BN_CLICKED(IDC_BUTTON_SAVE_TIME, OnBnClickedButtonSaveTime)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CHANNEL, OnBnClickedButtonSaveChannel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_TEXT, OnBnClickedButtonSaveText)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_COMMONINFO, OnBnClickedButtonSaveCommoninfo)
	ON_BN_CLICKED(IDC_BUTTON_SET_CUSTOM_TEXT, OnBnClickedButtonSetCustomText)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNEL, OnCbnSelchangeComboChannel)
	ON_BN_CLICKED(IDC_BTN_PTZZOOM, OnBnClickedBtnPtzzoom)
	ON_BN_CLICKED(IDC_BUTTON_GET_CHANNEL, OnBnClickedButtonGetChannel)
	ON_BN_CLICKED(IDC_BUTTON_GET_TIME, OnBnClickedButtonGetTime)
	ON_BN_CLICKED(IDC_BUTTON_GET_COMMONINFO, OnBnClickedButtonGetCommoninfo)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CTestOSDDlg 消息处理程序

BOOL CTestOSDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    GetWindowRect(&g_InitRect);


    ClientToScreen(&g_InitRect);
    
    g_bPlayWndMax = FALSE;

    SetDlgItemText(IDC_EDIT_IP,"171.35.0.150");
    SetDlgItemText(IDC_EDIT_PORT,"37777");
    SetDlgItemText(IDC_EDIT_USER,"admin");
    SetDlgItemText(IDC_EDIT_PWD,"admin");

    InitCombo(this,IDC_COMBO_TYPE_CHANNEL,arComboItemInfo,_countof(arComboItemInfo));
    InitCombo(this,IDC_COMBO_TYPE_TIME,arComboItemInfo,_countof(arComboItemInfo));

    g_iChannelIndex = 0;

	CLIENT_Init(DisConnectFunc, (LDWORD)this);	
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CTestOSDDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTestOSDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}   
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CTestOSDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTestOSDDlg::OnBnClickedButtonLogin()
{
    // TODO: 在此添加控件通知处理程序代码
	SetWindowText(ConvertString("TestOSD"));
    if (g_lLogin != 0)
    {
        CLIENT_StopRealPlay(g_lPlay);
        g_lPlay = 0;

        CLIENT_Logout(g_lLogin);
        g_lLogin = 0;

        MessageBox(ConvertString("logout success!"),ConvertString("Prompt"));
        SetDlgItemText(IDC_BUTTON_START_PLAY,ConvertString("login"));
		Invalidate(FALSE);
        return;
    }

    CString szIP = "";
    CString szPort = "";
    CString szUser = "";
    CString szPwd = "";

    GetDlgItemText(IDC_EDIT_IP,szIP);
    GetDlgItemText(IDC_EDIT_PORT,szPort);
    GetDlgItemText(IDC_EDIT_USER,szUser);
    GetDlgItemText(IDC_EDIT_PWD,szPwd);

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, szIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, szPwd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, szUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = atoi(szPort);
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	g_lLogin = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL);
    if (pkCombo)
    {
        pkCombo->ResetContent();
        for (int i = 0;i < stOutparam.stuDeviceInfo.nChanNum;i++)
        {
            CString szText = "";
            szText.Format(ConvertString("channel %d"),i + 1);
            pkCombo->AddString(szText);
            pkCombo->SetItemData(i,i);
        }
    }
   
    if (g_lLogin == 0)
    {
        ShowLoginErrorReason(stOutparam.nError);
    }
    else
    {
        MessageBox(ConvertString("login device success!"),ConvertString("Prompt"));
        SetDlgItemText(IDC_BUTTON_START_PLAY,ConvertString("Logout"));
    }    

    return;
}

void CTestOSDDlg::OnStnDblclickStaticPic()
{
    // TODO: 在此添加控件通知处理程序代码

    return;
}


void CTestOSDDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
    CDialog::OnWindowPosChanged(lpwndpos);

    // TODO: 在此处添加消息处理程序代码
    return;
}

void CTestOSDDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
    CDialog::OnWindowPosChanging(lpwndpos);

    // TODO: 在此处添加消息处理程序代码
    return;
}

void CTestOSDDlg::OnCbnSelchangeComboOsdType()
{
    // TODO: 在此添加控件通知处理程序代码
}

void CTestOSDDlg::OnBnClickedButtonSaveTime()
{
    // TODO: 在此添加控件通知处理程序代码
    for (int i = 0;i < _countof(arCoverTimeID);i++)
    {
        g_stTimeTitle.emOsdBlendType = (NET_EM_OSD_BLEND_TYPE)(i + 1);
        g_stTimeTitle.bEncodeBlend = ((CButton*)(GetDlgItem(arCoverTimeID[i])))->GetCheck();
        g_stTimeTitle.bShowWeek = ((CButton*)(GetDlgItem(IDC_CHECK_SHOWTIME)))->GetCheck();

        BYTE byR = 0;
        BYTE byG = 0;
        BYTE byB = 0;
        BYTE byA = 0;
        ((CIPAddressCtrl*)(GetDlgItem(IDC_TIME_FCOLOR)))->GetAddress(byR,byG,byB,byA);
        g_stTimeTitle.stuFrontColor.nRed = byR;
        g_stTimeTitle.stuFrontColor.nGreen = byG;
        g_stTimeTitle.stuFrontColor.nBlue = byB;
        g_stTimeTitle.stuFrontColor.nAlpha = byA;

        ((CIPAddressCtrl*)(GetDlgItem(IDC_TIME_BCOLOR)))->GetAddress(byR,byG,byB,byA);
        g_stTimeTitle.stuBackColor.nRed = byR;
        g_stTimeTitle.stuBackColor.nGreen = byG;
        g_stTimeTitle.stuBackColor.nBlue = byB;
        g_stTimeTitle.stuBackColor.nAlpha = byA;


        g_stTimeTitle.stuRect.nLeft = GetDlgItemInt(IDC_EDIT_TIME_TITLE_X);
        g_stTimeTitle.stuRect.nTop = GetDlgItemInt(IDC_EDIT_TIME_TITLE_Y);
        g_stTimeTitle.stuRect.nRight = GetDlgItemInt(IDC_EDIT_TIME_TITLE_X);
        g_stTimeTitle.stuRect.nBottom = GetDlgItemInt(IDC_EDIT_TIME_TITLE_Y);

        BOOL bRet = CLIENT_SetConfig(g_lLogin,NET_EM_CFG_TIMETITLE,g_iChannelIndex,&g_stTimeTitle,sizeof(g_stTimeTitle));
        if (!bRet)
        {
            CString szText = "";
            CString szCover = "";
            GetDlgItemText(arCoverTimeID[i],szCover);
            szText.Format(ConvertString("%s:set time title failed!error code(0x%X)"),szCover,CLIENT_GetLastError());
            MessageBox(szText,ConvertString("Prompt"));
        }
        else
        {
            MessageBox(ConvertString("set time title success!"),ConvertString("Prompt"));
        }
    }    
}

void CTestOSDDlg::OnIpnFieldchangedRect(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    *pResult = 0;
}

void CTestOSDDlg::OnBnClickedButtonSaveChannel()
{
    // TODO: 在此添加控件通知处理程序代码

    for (int i = 0;i < _countof(arCoverChannelID);i++)
    {
        g_stChannelTitle.emOsdBlendType = (NET_EM_OSD_BLEND_TYPE)(i + 1);
        g_stChannelTitle.bEncodeBlend = ((CButton*)(GetDlgItem(arCoverChannelID[i])))->GetCheck();
        BYTE byR = 0;
        BYTE byG = 0;
        BYTE byB = 0;
        BYTE byA = 0;
        ((CIPAddressCtrl*)(GetDlgItem(IDC_CHANNEL_FCOLOR)))->GetAddress(byR,byG,byB,byA);
        g_stChannelTitle.stuFrontColor.nRed = byR;
        g_stChannelTitle.stuFrontColor.nGreen = byG;
        g_stChannelTitle.stuFrontColor.nBlue = byB;
        g_stChannelTitle.stuFrontColor.nAlpha = byA;

        ((CIPAddressCtrl*)(GetDlgItem(IDC_CHANNEL_BCOLOR)))->GetAddress(byR,byG,byB,byA);
        g_stChannelTitle.stuBackColor.nRed = byR;
        g_stChannelTitle.stuBackColor.nGreen = byG;
        g_stChannelTitle.stuBackColor.nBlue = byB;
        g_stChannelTitle.stuBackColor.nAlpha = byA;


        g_stChannelTitle.stuRect.nLeft = GetDlgItemInt(IDC_EDIT_CHANNEL_TITLE_X);
        g_stChannelTitle.stuRect.nTop = GetDlgItemInt(IDC_EDIT_CHANNEL_TITLE_Y);
        g_stChannelTitle.stuRect.nRight = GetDlgItemInt(IDC_EDIT_CHANNEL_TITLE_X);
        g_stChannelTitle.stuRect.nBottom = GetDlgItemInt(IDC_EDIT_CHANNEL_TITLE_Y);

        BOOL bRet = CLIENT_SetConfig(g_lLogin,NET_EM_CFG_CHANNELTITLE,g_iChannelIndex,&g_stChannelTitle,sizeof(g_stChannelTitle));
        if (!bRet)
        {
            CString szText = "";
            CString szCover = "";
            GetDlgItemText(arCoverChannelID[i],szCover);
            szText.Format(ConvertString("%s:set channel title failed!error code(0x%X)"),szCover,CLIENT_GetLastError());            
            MessageBox(szText,ConvertString("Prompt"));
        }
        else
        {
            MessageBox(ConvertString("set channel title success!"),ConvertString("Prompt"));
        }
    }

    return;
}

void CTestOSDDlg::OnBnClickedButtonSaveText()
{
    // TODO: 在此添加控件通知处理程序代码
   
}

void CTestOSDDlg::UpdateChannelTitle()
{
    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL);
    if (pkCombo->GetCurSel() == -1)
    {
        MessageBox(ConvertString("please select channel!"),ConvertString("Prompt"));
        return;
    }

    SET_DWSIZE(g_stChannelTitle,true);
    
    for (int i = 0;i < _countof(arCoverChannelID);i++)
    {
        // emOsdBlendType字段必须要设置成合法的值，不然接口会调用失败
        g_stChannelTitle.emOsdBlendType = (NET_EM_OSD_BLEND_TYPE)(i + 1);

        BOOL bRet = CLIENT_GetConfig(g_lLogin,NET_EM_CFG_CHANNELTITLE,g_iChannelIndex,&g_stChannelTitle,sizeof(g_stChannelTitle));
        if (bRet)
        {
            //SetComboWithValue(this,IDC_COMBO_TYPE_CHANNEL,g_stChannelTitle.emOsdBlendType);
            ((CButton*)(GetDlgItem(arCoverChannelID[i])))->SetCheck(g_stChannelTitle.bEncodeBlend);
            ((CIPAddressCtrl*)(GetDlgItem(IDC_CHANNEL_FCOLOR)))->SetAddress(
                g_stChannelTitle.stuFrontColor.nRed,
                g_stChannelTitle.stuFrontColor.nGreen,
                g_stChannelTitle.stuFrontColor.nBlue,
                g_stChannelTitle.stuFrontColor.nAlpha);

            ((CIPAddressCtrl*)(GetDlgItem(IDC_CHANNEL_BCOLOR)))->SetAddress(
                g_stChannelTitle.stuBackColor.nRed,
                g_stChannelTitle.stuBackColor.nGreen,
                g_stChannelTitle.stuBackColor.nBlue,
                g_stChannelTitle.stuBackColor.nAlpha);

            SetDlgItemInt(IDC_EDIT_CHANNEL_TITLE_X,g_stChannelTitle.stuRect.nLeft);
            SetDlgItemInt(IDC_EDIT_CHANNEL_TITLE_Y,g_stChannelTitle.stuRect.nTop);
        }
        else
        {
            CString szText = "";
            CString szCover = "";
            GetDlgItemText(arCoverChannelID[i],szCover);
            szText.Format(ConvertString("%s:get channel title config failed!error code (0x%X)"),szCover,CLIENT_GetLastError());
            MessageBox(szText,ConvertString("Prompt"));        
        }
    }    

    return;
}

void CTestOSDDlg::UpdateTimeTitle()
{
    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL);
    if (pkCombo->GetCurSel() == -1)
    {
        MessageBox(ConvertString("please select channel!"),ConvertString("Prompt"));
        return;
    }

    SET_DWSIZE(g_stTimeTitle,true);

    for (int i = 0;i < _countof(arCoverTimeID);i++)
    {
        // emOsdBlendType字段必须要设置成合法的值，不然接口会调用失败
        g_stTimeTitle.emOsdBlendType = (NET_EM_OSD_BLEND_TYPE)(i + 1);
        BOOL bRet = CLIENT_GetConfig(g_lLogin,NET_EM_CFG_TIMETITLE,g_iChannelIndex,&g_stTimeTitle,sizeof(g_stTimeTitle));
        if (bRet)
        {
            //SetComboWithValue(this,IDC_COMBO_TYPE_TIME,g_stTimeTitle.emOsdBlendType);
            ((CButton*)(GetDlgItem(arCoverTimeID[i])))->SetCheck(g_stTimeTitle.bEncodeBlend);
            ((CButton*)(GetDlgItem(IDC_CHECK_SHOWTIME)))->SetCheck(g_stTimeTitle.bShowWeek);
            ((CIPAddressCtrl*)(GetDlgItem(IDC_TIME_FCOLOR)))->SetAddress(
                g_stTimeTitle.stuFrontColor.nRed,
                g_stTimeTitle.stuFrontColor.nGreen,
                g_stTimeTitle.stuFrontColor.nBlue,
                g_stTimeTitle.stuFrontColor.nAlpha);

            ((CIPAddressCtrl*)(GetDlgItem(IDC_TIME_BCOLOR)))->SetAddress(
                g_stTimeTitle.stuBackColor.nRed,
                g_stTimeTitle.stuBackColor.nGreen,
                g_stTimeTitle.stuBackColor.nBlue,
                g_stTimeTitle.stuBackColor.nAlpha);

            SetDlgItemInt(IDC_EDIT_TIME_TITLE_X,g_stTimeTitle.stuRect.nLeft);
            SetDlgItemInt(IDC_EDIT_TIME_TITLE_Y,g_stTimeTitle.stuRect.nTop);
        }
        else
        {
            CString szText = "";
            CString szCover = "";
            GetDlgItemText(arCoverTimeID[i],szCover);
            szText.Format(ConvertString("%s:get time title config failed!error code(0x%X)"),szCover,CLIENT_GetLastError());            
            MessageBox(szText,ConvertString("Prompt"));
        }
    }
}

void CTestOSDDlg::UpdateTitleCommonInfo()
{
    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL);
    if (pkCombo->GetCurSel() == -1)
    {
        MessageBox("please select channel!",ConvertString("Prompt"));
        return;
    }

    SET_DWSIZE(g_stCommonInfo,true);
   
    BOOL bRet = CLIENT_GetConfig(g_lLogin,NET_EM_CFG_OSDCOMMINFO,g_iChannelIndex,&g_stCommonInfo,sizeof(g_stCommonInfo));
    if (bRet)
    {
        CString szText = "";
        szText.Format("%.8f",g_stCommonInfo.fFontSizeScale);

        SetDlgItemText(IDC_EDIT_FONT_SCALE,szText);
        SetDlgItemInt(IDC_EDIT_FONT_SIZE,g_stCommonInfo.nFontSize);
        SetDlgItemInt(IDC_EDIT_FONT_SIZE_SUB1,g_stCommonInfo.nFontSizeExtra1);
        SetDlgItemInt(IDC_EDIT_FONT_SIZE_SUB2,g_stCommonInfo.nFontSizeExtra2);
        SetDlgItemInt(IDC_EDIT_FONT_SIZE_SUB3,g_stCommonInfo.nFontSizeExtra3);
        SetDlgItemInt(IDC_EDIT_FONT_SIZE_SNAP,g_stCommonInfo.nFontSizeSnapshot);
        SetDlgItemInt(IDC_EDIT_FONT_SIZE_SNAP2,g_stCommonInfo.nFontSizeMergeSnapshot); 
    }
    else
    {
        CString szText = "";
        szText.Format(ConvertString("get OSD common attributes failed!error code(0x%X)"),CLIENT_GetLastError());
        MessageBox(szText,ConvertString("Prompt"));
    }    

    return;
}

void CTestOSDDlg::OnBnClickedButtonSaveCommoninfo()
{
    // TODO: 在此添加控件通知处理程序代码

    SET_DWSIZE(g_stCommonInfo,false);

    CString szText = "";
    GetDlgItemText(IDC_EDIT_FONT_SCALE,szText);
    g_stCommonInfo.fFontSizeScale = atof(szText);
    g_stCommonInfo.nFontSize = GetDlgItemInt(IDC_EDIT_FONT_SIZE);
    g_stCommonInfo.nFontSizeExtra1 = GetDlgItemInt(IDC_EDIT_FONT_SIZE_SUB1);
    g_stCommonInfo.nFontSizeExtra2 = GetDlgItemInt(IDC_EDIT_FONT_SIZE_SUB2);
    g_stCommonInfo.nFontSizeExtra3 = GetDlgItemInt(IDC_EDIT_FONT_SIZE_SUB3);
    g_stCommonInfo.nFontSizeSnapshot = GetDlgItemInt(IDC_EDIT_FONT_SIZE_SNAP);
    g_stCommonInfo.nFontSizeMergeSnapshot = GetDlgItemInt(IDC_EDIT_FONT_SIZE_SNAP2);

    BOOL bRet = CLIENT_SetConfig(g_lLogin,NET_EM_CFG_OSDCOMMINFO,g_iChannelIndex,&g_stCommonInfo,sizeof(g_stCommonInfo));
    if (!bRet)
    {
        CString szText = "";
        szText.Format(ConvertString("set OSD common attributes failed!error code(0x%X)"),CLIENT_GetLastError());
        MessageBox(szText,ConvertString("Prompt"));
    }
    else
    {
        MessageBox(ConvertString("set OSD common attributes success!"),ConvertString("Prompt"));
    }
}

void CTestOSDDlg::OnBnClickedButtonSetCustomText()
{
    // TODO: 在此添加控件通知处理程序代码

    if (g_lLogin == 0)
    {
        MessageBox(ConvertString("please login first!"),ConvertString("Prompt"));
        return;
    }

    CSetCustomTextDlg Dlg;
    Dlg.DoModal();

    return;
}

void CTestOSDDlg::OnCbnSelchangeComboChannel()
{
    // TODO: 在此添加控件通知处理程序代码

    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_CHANNEL);
    if (g_lPlay)
    {
        CLIENT_StopRealPlay(g_lPlay);
        g_lPlay = 0;
    }

    g_iChannelIndex = pkCombo->GetItemData(pkCombo->GetCurSel());

    HWND hWnd = GetDlgItem(IDC_STATIC_PIC)->GetSafeHwnd();
    g_lPlay = CLIENT_StartRealPlay(g_lLogin,g_iChannelIndex,hWnd,DH_RType_Realplay,NULL,NULL,0);
    if (g_lPlay == 0)
    {
        CString str = "";
        str.Format(ConvertString("start RealPlay failed! error code(0x%X)"),CLIENT_GetLastError());
        MessageBox(str,ConvertString("Prompt"));

        return;
    }

    UpdateChannelTitle();
    UpdateTimeTitle();
    UpdateTitleCommonInfo();

    return;
}


void CTestOSDDlg::ShowLoginErrorReason(int nError)
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

HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  在此更改 DC 的任何属性

    // TODO:  如果默认的不是所需画笔，则返回另一个画笔
    return hbr;
}

void CTestOSDDlg::OnBnClickedButtonGetChannel()
{
    // TODO: 在此添加控件通知处理程序代码

    UpdateChannelTitle();
}

void CTestOSDDlg::OnBnClickedButtonGetTime()
{
    // TODO: 在此添加控件通知处理程序代码

    UpdateTimeTitle();
}

void CTestOSDDlg::OnBnClickedButtonGetCommoninfo()
{
    // TODO: 在此添加控件通知处理程序代码

    UpdateTitleCommonInfo();
}
void CTestOSDDlg::OnBnClickedBtnPtzzoom()
{
	CSetPTZZoom dlg;
	dlg.DoModal();
}


//Process when device disconnected 
LRESULT CTestOSDDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CTestOSDDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device reconnect
	SetWindowText(ConvertString("TestOSD"));
	return 0;
}

BOOL CTestOSDDlg::PreTranslateMessage(MSG* pMsg) 
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

void CTestOSDDlg::OnDestroy()
{
	CDialog::OnDestroy();

	if (0 != g_lPlay)
	{
		CLIENT_StopRealPlay(g_lPlay);
	}

	CLIENT_Cleanup();
}