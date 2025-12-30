// ThermalCameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "ThermalCameraDlg.h"
#include "control.h"
#include "AlarmDlg.h"
#include "Info.h"
#include "QueryDlg.h"
#include "Property.h"
#include "RuleDlg.h"
#include "GlobalDlg.h"
#include "StatistcisDlg.h"
#include "AlarmConfigDlg.h"
#include "LocalParameter.h"
#include "MessageText.h"
#include "QueryPointInfoDlg.h"
#include "QueryItemInfoDlg.h"
#include "heatmap.h"
#include "ThermalSetDlg.h"
#include "FireWarnSetDlg.h"
#include "FireCalibrateDlg.h"
#include "PTZDlg.h"
#include "GetTemper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)
/////////////////////////////////////////////////////////////////////////////
// CThermalCameraDlg dialog

CThermalCameraDlg::CThermalCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CThermalCameraDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThermalCameraDlg)
	m_DvrUserName = _T("admin");
	m_DvrPassword = _T("admin123");
	m_DvrPort = 37777;
	m_bOriginal = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_LoginID = 0;
    m_nChannelCount = 0;
    m_nChannelNum = 0;
    for (int i = 0;i<2;++i)
    {
        m_DispHanle[i] =0;
    }
}

void CThermalCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThermalCameraDlg)
	DDX_Control(pDX, IDC_BUTTON_STOP, m_BtnStop);
	DDX_Control(pDX, IDC_BUTTON_Play, m_BtnPlay);
	DDX_Control(pDX, IDC_COMBO_PLAYMODE, m_comboPlayMode);
	DDX_Control(pDX, IDC_IPADDRESS, m_DvrIPAddr);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_DvrUserName);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_DvrPassword);
	DDX_Text(pDX, IDC_EDIT_PORT, m_DvrPort);
	DDX_Check(pDX, IDC_CHECK_ORIGINAL, m_bOriginal);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CThermalCameraDlg, CDialog)
	//{{AFX_MSG_MAP(CThermalCameraDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_ALARM, OnAlarm)
	ON_COMMAND(ID_QUERY, OnQuery)
	ON_COMMAND(ID_Preperty, OnPreperty)
	ON_COMMAND(ID_GLOBAL, OnGlobal)
	ON_COMMAND(ID_STATISTCIS, OnStatistcis)
	ON_BN_CLICKED(IDC_BTN_Login, OnBTNLogin)
	ON_BN_CLICKED(IDC_BTN_LEAVE, OnBtnLeave)
	ON_BN_CLICKED(IDC_BUTTON_Play, OnBUTTONPlay)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnButtonStop)
	ON_COMMAND(ID_PRESETINFO, OnPresetinfo)
	ON_COMMAND(ID_POINTINFO, OnPointinfo)
	ON_COMMAND(ID_ITEMINFO, OnIteminfo)
	ON_COMMAND(ID_TEMPERINFO, OnGetRandomRegionTemper)
	ON_COMMAND(ID_RULE, OnRule)
	ON_BN_CLICKED(IDC_BTN_PTZMENU, OnBtnPtzmenu)
	ON_COMMAND(ID_HEATMAP, OnHeatmap)
	ON_COMMAND(ID_THERMALSET, OnThermalSet)
	ON_COMMAND(ID_FIREWARN_FIREWARNSET, OnFireWarnSet)
	ON_COMMAND(ID_FIREWARN_CALIBRATE, OnCalibrate)
	ON_COMMAND(ID_PTZ, OnPTZ)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThermalCameraDlg message handlers

BOOL CThermalCameraDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_Menu.LoadMenu(IDR_MENU);
    m_Menu.GetSubMenu(0)->ModifyMenu(0,MF_BYPOSITION,ID_PRESETINFO,ConvertString("presetinfo"));
    m_Menu.GetSubMenu(0)->ModifyMenu(1,MF_BYPOSITION,ID_POINTINFO,ConvertString("pointinfo"));
    m_Menu.GetSubMenu(0)->ModifyMenu(2,MF_BYPOSITION,ID_ITEMINFO,ConvertString("iteminfo"));
	m_Menu.GetSubMenu(0)->ModifyMenu(3,MF_BYPOSITION,ID_TEMPERINFO,ConvertString("regiontemperinfo"));
    m_Menu.ModifyMenu(1,MF_BYPOSITION,ID_ALARM,ConvertString("alarm"));
//    m_Menu.ModifyMenu(0,MF_BYPOSITION,ID_CONTROL,ConvertString("control"));
    m_Menu.ModifyMenu(0,MF_BYPOSITION,ID_INFO,ConvertString("info"));
    m_Menu.ModifyMenu(2,MF_BYPOSITION,ID_HEATMAP,ConvertString("heatmap"));
	m_Menu.ModifyMenu(3,MF_BYPOSITION,ID_THERMALSET,ConvertString("ThermalSet"));
	m_Menu.ModifyMenu(4,MF_BYPOSITION,ID_INFO,ConvertString("FireWarn"));
	m_Menu.GetSubMenu(4)->ModifyMenu(0,MF_BYPOSITION,ID_FIREWARN_FIREWARNSET,ConvertString("FireWarnSet"));
	m_Menu.GetSubMenu(4)->ModifyMenu(1,MF_BYPOSITION,ID_FIREWARN_CALIBRATE,ConvertString("Calibrate"));
	m_Menu.ModifyMenu(5,MF_BYPOSITION,ID_PTZ,ConvertString("PTZ"));
    SetMenu(&m_Menu);
    g_SetWndStaticText(this);
    //m_DvrIPAddr.SetAddress(172,12,4,205);
	m_DvrIPAddr.SetAddress(10,35,248,156);
	//m_DvrIPAddr.SetAddress(10,18,128,97);
	int nIndex = 0;
    CString strPlayMode[2]={ConvertString("Direct-play"), ConvertString("Data-callback")};
    m_comboPlayMode.ResetContent();
    nIndex = m_comboPlayMode.AddString(strPlayMode[0]);
    m_comboPlayMode.SetItemData(nIndex,DirectMode);
    nIndex = m_comboPlayMode.AddString(strPlayMode[1]);
    m_comboPlayMode.SetItemData(nIndex,ServerMode);
	m_comboPlayMode.SetCurSel(0);
	InitNetSDK();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CThermalCameraDlg::OnPaint() 
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

//Callback function when device disconnected
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{    
	if(dwUser == 0)
	{
		return;
	}

	CThermalCameraDlg *pThis = (CThermalCameraDlg *)dwUser;
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

	CThermalCameraDlg *pThis = (CThermalCameraDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}

//Initialize net SDK
void CThermalCameraDlg::InitNetSDK()
{
    //Initialize net sdk, All callback begins here.
    BOOL bSuccess = CLIENT_Init(DisConnectFunc, (LDWORD)this);
    if (!bSuccess)
    {
        //Display function error occurrs reason.
//        LastError();
    }
	else
	{
		LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
		CLIENT_LogOpen(&stLogPrintInfo);
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
}


//Process when device disconnected 
LRESULT CThermalCameraDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
    //The codes need to be processed when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device disconnected 
LRESULT CThermalCameraDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device reconnect
	SetWindowText(ConvertString("ThermalCamera"));
	return 0;
}


// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CThermalCameraDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// void CThermalCameraDlg::OnControl() 
// {
//     if (0 != m_LoginID)
//     {
//         CControl Dlg(this,m_LoginID);
//         Dlg.DoModal();
//     }
// }

void CThermalCameraDlg::OnAlarm() 
{
	
	CAlarmDlg dlg(this,m_LoginID,m_nChannelCount);
	dlg.DoModal();
}

void CThermalCameraDlg::OnPresetinfo() 
{
    CInfo Dlg(this, m_LoginID,m_nChannelCount-1);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnQuery() 
{
    CQueryDlg Dlg(this,m_LoginID);
	Dlg.DoModal();
	
}

void CThermalCameraDlg::OnPreperty() 
{
	CProperty Dlg(this,m_LoginID);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnGlobal() 
{
	CGlobalDlg Dlg(this,m_LoginID);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnStatistcis() 
{
	CStatistcisDlg Dlg(this,m_LoginID);
	Dlg.DoModal();
	
}

void CThermalCameraDlg::OnBTNLogin() 
{
    BOOL bValid = UpdateData(TRUE);	//Get interface input 
    if(bValid)
    {
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
            m_nChannelCount = min(stOutparam.stuDeviceInfo.nChanNum, 2); // thermal camera max channel is two
            m_LoginID = lRet;
            GetDlgItem(IDC_BTN_Login)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_LEAVE)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_Play)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
        }
        else
        {
            ShowLoginErrorReason(stOutparam.nError);
        }
    }
	SetWindowText(ConvertString("ThermalCamera"));
}
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
        strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
        if(strClassName == "CEdit")
        {
            //Nex sub-window 
            pChild=pChild->GetWindow(GW_HWNDNEXT);
            continue;
        }
        
        //////////////////////////////////////////////////////////////////////////	
        
        //Set sub-window current language text
        pChild->GetWindowText(strCaption);
        strText=ConvertString(strCaption);
        pChild->SetWindowText(strText);
        
        //Next sub-window 
        pChild=pChild->GetWindow(GW_HWNDNEXT);
    }
}
CString ConvertString(CString strText, const char* pszSeg /* = NULL */)
{
    CString strIniPath, strRet;
    char szVal[256] = {0};
    
    if (!pszSeg)
    {
        pszSeg = DLG_MAIN;
    }
    
    GetPrivateProfileString(pszSeg, strText, "", szVal, sizeof(szVal), "./langchn.ini");
    strRet = szVal;
    if(strRet.GetLength()==0)
    {
        //If there is no corresponding string in ini file then set it to be default value(English).
        strRet=strText;
	}
    return strRet;
}
//Display log in failure reason 
void CThermalCameraDlg::ShowLoginErrorReason(int nError)
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
CString CThermalCameraDlg::GetDvrIP()
{
    CString strRet="";
    BYTE nField0,nField1,nField2,nField3;
    m_DvrIPAddr.GetAddress(nField0,nField1,nField2,nField3);
    strRet.Format("%d.%d.%d.%d",nField0,nField1,nField2,nField3);
    return strRet;
}


void CThermalCameraDlg::OnBtnLeave() 
{
    for (int i = 0; i<2; ++i)
    {
        CloseDispVideo(i);
    }
	if (0 != m_LoginID)
	{
        BOOL bSuccess = CLIENT_Logout(m_LoginID);
        if (bSuccess)
        {
            m_LoginID = 0;
            m_DispHanle[0] = 0;
            m_DispHanle[1] = 0;
            GetDlgItem(IDC_BTN_Login)->EnableWindow(TRUE);
            GetDlgItem(IDC_BTN_LEAVE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_Play)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
            Invalidate();
        }
        else
        {
            MessageBox(ConvertString("Fail to Logout!"), ConvertString("Prompt"));
		}
	}
    m_comboPlayMode.EnableWindow(TRUE);
	SetWindowText(ConvertString("ThermalCamera"));
}

//Play video directly 
void CThermalCameraDlg::DirectPlayMode(int iDispNum,int iChannel,HWND hWnd)
{
    //Close current video 
    CloseDispVideo(iDispNum);
    
    LLONG lRet = CLIENT_RealPlay(m_LoginID,iChannel,hWnd);
    if(0 != lRet)
    {
        m_DispHanle[iDispNum] = lRet;
    }
    else
    {
        MessageBox(ConvertString("Fail to play"), ConvertString("Prompt"));
    }
}

void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize, LLONG lParam, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CThermalCameraDlg *dlg = (CThermalCameraDlg *)dwUser;
	dlg->ReceiveRealData(lRealHandle,dwDataType, pBuffer, dwBufSize, lParam);
}


//Play video in data callback mode 
void CThermalCameraDlg::ServerPlayMode(int iDispNum,int iChannel, HWND hWnd)
{
    //Close current video 
    CloseDispVideo(iDispNum);
    
    //Enable stream
    BOOL bOpenRet = PLAY_OpenStream(iDispNum,0,0,1024*512*6);
    if(bOpenRet)
    {
        //Begin play 
        BOOL bPlayRet = PLAY_Play(iDispNum,hWnd);
        if(bPlayRet)
        {
            //Real-time play 
            LLONG lRet = CLIENT_RealPlayEx(m_LoginID,iChannel,0);
            if(0 != lRet)
            {
                m_DispHanle[iDispNum] =lRet;
                //Callback monitor data and then save 
                CLIENT_SetRealDataCallBackEx2(lRet, RealDataCallBackEx, (LDWORD)this, 0x0f);
            }
            else
            {
                MessageBox(ConvertString("Fail to play"), ConvertString("Prompt"));
                PLAY_Stop(iDispNum);
                PLAY_CloseStream(iDispNum);
            }
        }
        else
        {
            PLAY_CloseStream(iDispNum);
        }
    }
}

//Close video 
void CThermalCameraDlg::CloseDispVideo(int iDispNum)
{
    //Close current video 
    int ePlayMode = m_comboPlayMode.GetCurSel();
    if(ePlayMode == DirectMode)
    {
        if(0 != m_DispHanle[iDispNum])
        {
            BOOL bRet = CLIENT_StopRealPlay(m_DispHanle[iDispNum]);
            m_DispHanle[iDispNum] = 0;
    	}
    }
    else if(ePlayMode == ServerMode)
    {
        if(0 != m_DispHanle[iDispNum])
        {
            BOOL bRealPlay = CLIENT_StopRealPlay(m_DispHanle[iDispNum]);
            if(bRealPlay)
            {
                //And then close PLAY_Play
                BOOL bPlay = PLAY_Stop(iDispNum);
                if(bPlay)
                {
                    //At last close PLAY_OpenStream
                    BOOL bStream = PLAY_CloseStream(iDispNum);
					m_DispHanle[iDispNum] = 0;
                }
            }
    	}
    }
}


//Process after receiving real-time data 
void CThermalCameraDlg::ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LLONG lParam)
{
	int nPort = 0;
	for (int i=0; i<m_nChannelCount; i++)
	{
		if (m_DispHanle[i] == lRealHandle)
		{
			nPort = i;
			break;
		}
	}
	
    BOOL bInput=FALSE;
    if(0 != lRealHandle)
    {
        switch(dwDataType) 
        {
        case 0:
            //Original data 
            bInput = PLAY_InputData(nPort,pBuffer,dwBufSize);
            break;
        case 1:
            //Standard video data 
            
            break;
        case 2:
            //yuv data 
            
            break;
        case 3:
            //pcm audio data 
            
            break;       
        default:
            break;
        }	
    }
}

void CThermalCameraDlg::OnBUTTONPlay() 
{
    for (int i = 0;i < m_nChannelCount;i++)
    {
        HWND hWnd = NULL;
		int nPort = 0;
        int iDispNum = i;
	    int ePlayMode = m_comboPlayMode.GetCurSel();
        m_nChannelNum = i;                              
        if (0 == i)
        {
            hWnd = GetDlgItem(IDC_REALPALY)->m_hWnd;
			nPort = 1;
        }
        else if(1 == i)
        {
            hWnd = GetDlgItem(IDC_HEATPALY)->m_hWnd;
			nPort = 2;
        }
        if (-1 != ePlayMode)
        {
            if(ePlayMode == DirectMode)
            {
                //Play directly 
                DirectPlayMode(iDispNum,m_nChannelNum,hWnd);
            }
            if(ePlayMode == ServerMode)
            {
                //Play in data callback mode 
                ServerPlayMode(iDispNum,m_nChannelNum,hWnd);
	        }
        }
    }
    m_comboPlayMode.EnableWindow(FALSE);
    m_BtnPlay.EnableWindow(FALSE);
    m_BtnStop.EnableWindow(TRUE);
}

void CThermalCameraDlg::OnButtonStop() 
{
    for (int i = 0;i<m_nChannelCount;++i)
    {
        CloseDispVideo(i);
        //Refresh page 
    }
    Invalidate();
    m_comboPlayMode.EnableWindow(TRUE);
    m_BtnPlay.EnableWindow(TRUE);
    m_BtnStop.EnableWindow(FALSE);
}


void CThermalCameraDlg::OnPointinfo() 
{
	CQueryPointInfoDlg Dlg(this,m_LoginID,m_nChannelCount-1);
    Dlg.DoModal();
}

void CThermalCameraDlg::OnIteminfo() 
{
	CQueryItemInfoDlg Dlg(this,m_LoginID,m_nChannelCount-1);
    Dlg.DoModal();
}

void CThermalCameraDlg::OnGetRandomRegionTemper() 
{
	CGetTemper Dlg(this,m_LoginID,m_nChannelCount);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnRule() 
{
    CRuleDlg Dlg(this,m_LoginID,m_nChannelCount -1);
 	Dlg.DoModal();
}


void CThermalCameraDlg::OnBtnPtzmenu() 
{
    DWORD bytesReturned = 0;
    DH_ALARMIN_CFG_EX tmpIPCAlarmCfg;
    memset(&tmpIPCAlarmCfg, 0 ,sizeof(DH_ALARMIN_CFG_EX));
    BOOL bResult = CLIENT_GetDevConfig(m_LoginID, DH_DEV_LOCALALARM_CFG, 0, &tmpIPCAlarmCfg, sizeof(DH_ALARMIN_CFG_EX), &bytesReturned, 1000);

}

void CThermalCameraDlg::OnHeatmap() 
{
	CHeatmap Dlg(this,m_LoginID,m_nChannelCount-1);
    Dlg.DoModal();
}

void CThermalCameraDlg::OnThermalSet()
{
	ThermalSetDlg Dlg(this,m_LoginID, m_nChannelCount);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnFireWarnSet()
{
	FireWarnSetDlg Dlg(this,m_LoginID, m_nChannelCount);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnCalibrate()
{
	FireCalibrateDlg Dlg(this,m_LoginID, m_nChannelCount);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnPTZ()
{
	PTZDlg Dlg(this,m_LoginID, m_nChannelCount);
	Dlg.DoModal();
}

void CThermalCameraDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	for (int i = 0;i < m_nChannelCount; ++i)
	{
		CloseDispVideo(i);		
	}

	if (m_LoginID != 0)
	{
		CLIENT_Logout(m_LoginID);
	}
	
	CLIENT_Cleanup();	
}

void CThermalCameraDlg::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();
}


BOOL CThermalCameraDlg::PreTranslateMessage(MSG* pMsg) 
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