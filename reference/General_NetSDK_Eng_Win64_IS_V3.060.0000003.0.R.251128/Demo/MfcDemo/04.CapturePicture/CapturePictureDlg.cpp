// CapturePictureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CapturePicture.h"
#include "CapturePictureDlg.h"
#include "../../../Include/Common/dhnetsdk.h"
#include <atlconv.h>
#include <assert.h>
#include <windows.h>
#include <stdio.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEMO_SDK_WAITTIME 3000

#define	PICTURE_BACK_COLOR	RGB(105,105,105)
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
// CCapturePictureDlg dialog

CCapturePictureDlg::CCapturePictureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCapturePictureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCapturePictureDlg)
	
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_LoginID = 0;
	m_nChannelCount = 0;	

	memset(&m_stuDevAttrInfo, 0, sizeof(m_stuDevAttrInfo));
	memset(&m_bDataError,0,32 * sizeof(BOOL));
	memset(&m_stuSnapAttr, 0, sizeof(m_stuSnapAttr));
	m_pPicture = NULL;
}

void CCapturePictureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCapturePictureDlg)	
	DDX_Control(pDX, IDC_SHOWPICTURE, m_showbmp);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_ctlChannel);		
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCapturePictureDlg, CDialog)
	//{{AFX_MSG_MAP(CCapturePictureDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, OnButtonLogout)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_START, &CCapturePictureDlg::OnBnClickedButtonStart)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapturePictureDlg message handlers

BOOL CCapturePictureDlg::OnInitDialog()
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
	SetDlgItemText(IDC_IPADDRESS_IP, "172.28.2.41");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USER, "admin");
	SetDlgItemText(IDC_EDIT_PWD, "admin");		
	m_pPicture = NULL;
	
	InitNetSDK();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCapturePictureDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CCapturePictureDlg::OnPaint() 
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
       if (m_LoginID != 0 && (NULL != m_pPicture))
       {
			CPaintDC dc(this->GetDlgItem(IDC_SHOWPICTURE));
			CWnd* fullImageWindow = GetDlgItem(IDC_SHOWPICTURE);
			if (NULL == fullImageWindow)
			{
				return;
			}

			RECT rect;
			fullImageWindow->GetWindowRect(&rect);
			OLE_XSIZE_HIMETRIC hmWidth; 
			OLE_YSIZE_HIMETRIC hmHeight; 
			m_pPicture->get_Width(&hmWidth); 
			m_pPicture->get_Height(&hmHeight); 

			m_pPicture->Render(dc.m_hDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);	
       }
       else
       {
            FillCWndWithDefaultColor(GetDlgItem(IDC_SHOWPICTURE));
       }
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCapturePictureDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//Process when device disconnected 
LRESULT CCapturePictureDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("Network disconnected"));
	return 0;
}

//Process when device reconnect 
LRESULT CCapturePictureDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//Add process code when device disconnected
	SetWindowText(ConvertString("CapturePicture"));
	return 0;
}

void CCapturePictureDlg::OnButtonLogin() 
{
	// TODO: Add your control notification handler code here
	int nPort = 0;
	char szIp[32] = {0};
	char szUser[32] = {0};
	char szPwd[32] = {0};

	nPort = GetDlgItemInt(IDC_EDIT_PORT);
	GetDlgItemText(IDC_IPADDRESS_IP, szIp, 32);
	GetDlgItemText(IDC_EDIT_USER, szUser, 32);
	GetDlgItemText(IDC_EDIT_PWD, szPwd, 32);

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, szIp, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, szPwd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, szUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if(0 == lRet)
	{
		//Display log in failure reason
		ShowLoginErrorReason(stOutparam.nError);
	}
	else
	{
		m_LoginID = lRet;
		int nRetLen = 0;
		NET_DEV_CHN_COUNT_INFO stuChn = { sizeof(NET_DEV_CHN_COUNT_INFO) };
		stuChn.stuVideoIn.dwSize = sizeof(stuChn.stuVideoIn);
		stuChn.stuVideoOut.dwSize = sizeof(stuChn.stuVideoOut);
		BOOL bRet = CLIENT_QueryDevState(m_LoginID, DH_DEVSTATE_DEV_CHN_COUNT, (char*)&stuChn, stuChn.dwSize, &nRetLen);
		if (!bRet)
		{
			DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
		}
		m_nChannelCount = __max(stOutparam.stuDeviceInfo.nChanNum, stuChn.stuVideoIn.nMaxTotal);

		int nIndex = -1;
		m_ctlChannel.ResetContent();
		for (int i=0; i<m_nChannelCount; i++)
		{
			CString str;
			str.Format("%d",i+1);
			nIndex = m_ctlChannel.AddString(str);
			m_ctlChannel.SetItemData(nIndex,i);
		}
		m_ctlChannel.SetCurSel(0);		

		GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(TRUE);
	}
	SetWindowText(ConvertString("CapturePicture"));
}

void CCapturePictureDlg::OnButtonLogout() 
{
	// TODO: Add your control notification handler code here
	if(0 != m_LoginID)
	{
		BOOL bSuccess = CLIENT_Logout(m_LoginID);
		if(bSuccess)
		{			
			m_ctlChannel.ResetContent();				
			GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_LOGOUT)->EnableWindow(FALSE);			
		}
		else
		{
			MessageBox(ConvertString("Logout failed!"), ConvertString("prompt"));
		}
	}

	m_LoginID = 0;			

	SetWindowText(ConvertString("CapturePicture"));   

	if(m_pPicture != NULL)
	{
		m_pPicture->Release();
		m_pPicture = NULL;
	}
	
	FillCWndWithDefaultColor(GetDlgItem(IDC_SHOWPICTURE));
}



void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	
	CCapturePictureDlg *pThis = (CCapturePictureDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
}

void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CCapturePictureDlg *pThis = (CCapturePictureDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);
}

void CALLBACK SnapPicRet(LLONG ILoginID, BYTE *pBuf, UINT RevLen, UINT EncodeType, DWORD CmdSerial, LDWORD dwUser)
{
    CCapturePictureDlg *pThis = (CCapturePictureDlg*)dwUser;
	pThis->OnOnePicture(ILoginID,pBuf,RevLen,EncodeType,CmdSerial);
}

void CCapturePictureDlg::OnOnePicture(LLONG ILoginID, BYTE *pBuf, UINT RevLen, UINT EncodeType, UINT CmdSerial)
{
	//Get file path
	char path[1000];
	int filelen = GetModuleFileName(NULL,path,1000);
	int i = filelen;
	while(path[i]!='\\')
	{
        i--;
	}
	path[i + 1] = '\0';

	//Get file name
    CString filepath(path);
	CString filename = "mpeg4.JPG";

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char szFileName[128] = {0};
	sprintf_s(szFileName, "Ch%02d_%04d_%02d_%02d_%02d_%02d_%02d.%03d_mpeg4.jpg",
		m_ctlChannel.GetCurSel(),
		sys.wYear, 
		sys.wMonth, 
		sys.wDay, 
		sys.wHour, 
		sys.wMinute, 
		sys.wSecond,
		sys.wMilliseconds);
	CString strFileName(szFileName);
	CString strfile = filepath + strFileName;
	char *pFileName = strfile.GetBuffer(200);

	/* Save image original file */
	FILE *stream;
	if( (stream = fopen((const char*) pFileName, "wb")) != NULL )
	{
		int numwritten = fwrite( pBuf, sizeof( char ), RevLen, stream );
		fclose( stream );
	}
	
	/*Veirfy image encode type. If it is an I frame of mpeg4,then call I frame to decode to BMP to display.*/
	if ( EncodeType == 0) 
	{
		//int iRet = IFrameToBmp("tmp.bmp",pFileName);
		//if (iRet == 1)
		//{
		//	ShowBitmap("tmp.bmp");
		//}
	}
	else if (EncodeType == 10)
	{
		//ShowBitmap(pFileName);
		ShowSnapImage(pFileName);
	}
}

const COLORREF RESERVED_COLOR = RGB(255, 255, 255);
 
const int   HIMETRIC_INCH = 2540;

HBITMAP CCapturePictureDlg::LoadImageFile(LPCSTR szImagePath, int &nWidth, int &nHeight)
{
	USES_CONVERSION;
	IPicture* pPicture = NULL;
	
	HBITMAP hbm = NULL;
	HRESULT hr = OleLoadPicturePath(T2OLE(szImagePath), NULL, 0, RESERVED_COLOR, IID_IPicture, (LPVOID*)&pPicture);
	
	if (pPicture)
	{
		hbm = ExtractBitmap(pPicture,nWidth, nHeight);
		pPicture->Release();
	}
	
	return hbm;
}

void CCapturePictureDlg::ShowSnapImage(LPCSTR szImagePath)
{
	USES_CONVERSION;
	// 获取展示窗口
	CWnd* fullImageWindow = GetDlgItem(IDC_SHOWPICTURE);
	if (NULL == fullImageWindow)
	{
		return;
	}
	RECT rect;
	fullImageWindow->GetWindowRect(&rect);
	CDC* pDC = NULL;
	pDC = fullImageWindow->GetWindowDC();
	if (NULL == pDC)
	{
        return;
	}
	// 加载图片
	HRESULT hr = S_FALSE;
    hr = OleLoadPicturePath(T2OLE(szImagePath), NULL, 0, RESERVED_COLOR, IID_IPicture, (LPVOID*)&m_pPicture);
	if (NULL == m_pPicture)
	{
		fullImageWindow->ReleaseDC(pDC);
		return;
	}

	//显示全图
	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	m_pPicture->get_Width(&hmWidth); 
	m_pPicture->get_Height(&hmHeight); 

	hr = m_pPicture->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	fullImageWindow->ReleaseDC(pDC);
}
HBITMAP CCapturePictureDlg::ExtractBitmap(IPicture *pPicture, int &nWidth, int &nHeight)
{
	ASSERT(pPicture);
	
	if (!pPicture)
		return NULL;
	
	CBitmap bmMem;
	CDC dcMem;
	CDC* pDC = CWnd::GetDesktopWindow()->GetDC();
	
	if (dcMem.CreateCompatibleDC(pDC))
	{
		long hmWidth;
		long hmHeight;
		
		pPicture->get_Width(&hmWidth);
		pPicture->get_Height(&hmHeight);
		
		nWidth = MulDiv(hmWidth, pDC->GetDeviceCaps(LOGPIXELSX), HIMETRIC_INCH);
		nHeight = MulDiv(hmHeight, pDC->GetDeviceCaps(LOGPIXELSY), HIMETRIC_INCH);
		
		if (bmMem.CreateCompatibleBitmap(pDC, nWidth, nHeight))
		{
			CBitmap* pOldBM = dcMem.SelectObject(&bmMem);
			
			HRESULT hr = pPicture->Render(dcMem, 0, 0, nWidth, nHeight, 0, hmHeight, hmWidth, -hmHeight, NULL);
			dcMem.SelectObject(pOldBM);
		}
	}
	
	CWnd::GetDesktopWindow()->ReleaseDC(pDC);
	
	return (HBITMAP)bmMem.Detach();
}


void CCapturePictureDlg::InitNetSDK()
{
	BOOL ret = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	if (ret)
	{
		LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
		CLIENT_LogOpen(&stLogPrintInfo);
		CLIENT_SetSnapRevCallBack(SnapPicRet,(LDWORD)this);
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
	else
	{
		MessageBox(ConvertString("initialize SDK failed!"), ConvertString("prompt"));
	}

	// 初始化成功后设置网络参数
	NET_PARAM stuNetParam = {0};
	// 目前仅单独设置获取设备信息时间（部分设备因性能问题，没法在默认时间内完成，其他参数暂时保持默认）
	stuNetParam.nGetDevInfoTime = 3000;
	stuNetParam.nPicBufSize = 10*1024*1024;
	CLIENT_SetNetworkParam(&stuNetParam);
}

void CCapturePictureDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CCapturePictureDlg::ShowLoginErrorReason(int nError)
{
	if(1 == nError)		MessageBox(ConvertString("incorrect password!"), ConvertString("prompt"));
	else if(2 == nError)	MessageBox(ConvertString("no this account! "), ConvertString("prompt"));
	else if(3 == nError)	MessageBox(ConvertString("time out!"), ConvertString("prompt"));
	else if(4 == nError)	MessageBox(ConvertString("account have been logined!"), ConvertString("prompt"));
	else if(5 == nError)	MessageBox(ConvertString("account have been locked!"), ConvertString("prompt"));
	else if(6 == nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("prompt"));
	else if(7 == nError)	MessageBox(ConvertString("The system is busy!"), ConvertString("prompt"));
	else if(9 == nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("prompt"));
	else	MessageBox(ConvertString("Login falied!"), ConvertString("prompt"));
}


void CCapturePictureDlg::OnDestroy() 
{
	CDialog::OnDestroy();	

	if(m_pPicture != NULL)
	{
		m_pPicture->Release();
	}	

	CLIENT_Cleanup();
}

void CCapturePictureDlg::OnBnClickedButtonStart()
{
	if (0 != m_LoginID)
	{
		UpdateData(TRUE);
		//Fill in request structure 
		SNAP_PARAMS snapparams = {0};
		snapparams.Channel = m_ctlChannel.GetCurSel();
		snapparams.mode = 0;
		snapparams.CmdSerial = 0;		
		//Send out request.
		BOOL b = CLIENT_SnapPicture(m_LoginID, snapparams);
		if (!b)
		{
			MessageBox(ConvertString("begin to snap failed!"), ConvertString("prompt"));
		}		
	}
}


BOOL CCapturePictureDlg::PreTranslateMessage(MSG* pMsg) 
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


void CCapturePictureDlg::FillCWndWithDefaultColor(CWnd* cwnd) 
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