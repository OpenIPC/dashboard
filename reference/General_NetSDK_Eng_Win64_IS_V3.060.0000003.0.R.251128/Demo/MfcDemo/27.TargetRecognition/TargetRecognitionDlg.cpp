// TargetRecognitionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "TargetRecognitionDlg.h"
#include "FaceDbOpreate.h"
#include <atlconv.h>
#include <assert.h>
#include "SearchByPic.h"
#include "QueryDownLoad.h"
// #include <winuser.h>
// #include <crtdbg.h>
// #include <OLE2.H>
// #include <ocidl.h>
// #include <olectl.h>
// #include <shlwapi.h>
// 
// #pragma comment(lib, "Shlwapi.lib");

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct strAnalyInfo
{
	LLONG lAnalyzerHandle;
	DWORD dwAlarmType;
	void* pAlarmInfo;
	BYTE *pBuffer;
	DWORD dwBufSize;
};

#define WM_DEVICE_DOATTACH		(WM_USER + 110)
#define WM_DEVICE_DISCONNECT	(WM_USER + 111)
#define WM_DEVICE_RECONNECT		(WM_USER + 112)

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
// CTargetRecognitionDlg dialog

CTargetRecognitionDlg::CTargetRecognitionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTargetRecognitionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTargetRecognitionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nChnCnt = 0;
	m_lLoginID = 0;
	m_lRealPicHandle = 0;
	m_lRealHandle = 0;
	m_nFaceDetectGroupId = 0;

	m_pszSoftPath = NULL;
	m_pszSoftPath= new char[1024];
	if (m_pszSoftPath)
	{
		memset(m_pszSoftPath, 0, 1024);
	}
}

CTargetRecognitionDlg::~CTargetRecognitionDlg()
{
	if (m_pszSoftPath)
	{
		delete[] m_pszSoftPath;
		m_pszSoftPath = NULL;
	}

	if (0 != m_lRealPicHandle)
	{
		BOOL bRet = CLIENT_StopLoadPic(m_lRealPicHandle);
		if (!bRet)
		{
			MessageBox(ConvertString("Failed to stop load picture!"), "");
		}
		m_lRealPicHandle = 0;
	}
	
	if (0 != m_lRealHandle) 
	{
		CLIENT_StopRealPlayEx(m_lRealHandle);
		m_lRealHandle = 0;
	}

	if (0 != m_lLoginID)
	{
		BOOL bRet = CLIENT_Logout(m_lLoginID);
		if (!bRet)
		{
			MessageBox(ConvertString("Failed to login out!"), "");
		}
		else
		{
			m_lRealHandle = 0;
		}
	}
	
	CLIENT_Cleanup();
}

void CTargetRecognitionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTargetRecognitionDlg)
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_comboChannel);
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_BIG_PIC, m_pBicPic);
	DDX_Control(pDX, IDC_STATIC_CANDI1, m_pCandiPic);
	DDX_Control(pDX, IDC_STATIC_PERSON1, m_pFacePic);
}

BEGIN_MESSAGE_MAP(CTargetRecognitionDlg, CDialog)
	//{{AFX_MSG_MAP(CTargetRecognitionDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, OnButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_ATTACH, OnButtonAttachOrDeteach)
	ON_BN_CLICKED(IDC_BUTTON_DETEACH, OnButtonDeteach)
	ON_BN_CLICKED(IDC_BUTTON_FACE_DB, OnButtonFaceDb)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_REALPLAY, &CTargetRecognitionDlg::OnBnClickedButtonRealplay)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_DOWNLOAD, &CTargetRecognitionDlg::OnBnClickedButtonQueryDownload)
	ON_BN_CLICKED(IDC_BUTTON_SearchByPic, &CTargetRecognitionDlg::OnBnClickedButtonSearchbypic)

	ON_MESSAGE(WM_DEVICE_DOATTACH, &CTargetRecognitionDlg::DoDeviceAttach)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, &CTargetRecognitionDlg::OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, &CTargetRecognitionDlg::OnReConnect)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTargetRecognitionDlg message handlers

void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CTargetRecognitionDlg *pThis = (CTargetRecognitionDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_DISCONNECT, NULL, NULL);
}

void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CTargetRecognitionDlg *pThis = (CTargetRecognitionDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);	
}

void CTargetRecognitionDlg::ShowLoginErrorReason(int nError)
{
	if(1 == nError)		MessageBox(ConvertString("incorrect password!"), ConvertString("Prompt"));
	else if(2 == nError)	MessageBox(ConvertString("no this account!"), ConvertString("Prompt"));
	else if(3 == nError)	MessageBox(ConvertString("time out!"), ConvertString("Prompt"));
	else if(4 == nError)	MessageBox(ConvertString("account have been logined!"), ConvertString("Prompt"));
	else if(5 == nError)	MessageBox(ConvertString("account have been locked!"), ConvertString("Prompt"));
	else if(6 == nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("Prompt"));
	else if(7 == nError)	MessageBox(ConvertString("The system is busy!"), ConvertString("Prompt"));
	else if(9 == nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("Prompt"));
	else if(10 == nError)	MessageBox(ConvertString("More than the maximum number of connections!"), ConvertString("Prompt"));
	else	MessageBox(ConvertString("Login falied!"), ConvertString("Prompt"));
}


BOOL CTargetRecognitionDlg::OnInitDialog()
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
	SetDlgItemText(IDC_IPADDRESS, "172.23.12.30");
	SetDlgItemText(IDC_EDIT_PORT, "37777");
	SetDlgItemText(IDC_EDIT_USER, "admin");
	SetDlgItemText(IDC_EDIT_PASS, "admin123");
	
	GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);

	GetDlgItem(IDC_BUTTON_ATTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_FACE_DB)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_REALPLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SearchByPic)->EnableWindow(FALSE);

	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_BIG_PIC));
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PERSON1));
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_CANDI1));

	InitSdk();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTargetRecognitionDlg::InitSdk()
{
	BOOL ret = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	if (!ret)
	{
		MessageBox(ConvertString("Initialize SDK failed!"), ConvertString("Prompt"));
	}
	else
	{
		LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
		CLIENT_LogOpen(&stLogPrintInfo);
		CLIENT_SetAutoReconnect(ReConnectFunc, (LDWORD)this);
	}
}

//Process when device disconnected 
LRESULT CTargetRecognitionDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device disconnected
	SetWindowText(ConvertString("Network disconnected!"));
	return 0;
}

//Process when device reconnect
LRESULT CTargetRecognitionDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	//The codes need to be processed when device reconnect
	SetWindowText(ConvertString("TargetRecognition"));
	return 0;
}

void CTargetRecognitionDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTargetRecognitionDlg::OnPaint() 
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
HCURSOR CTargetRecognitionDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTargetRecognitionDlg::OnButtonLoginIn()
{
	CString strIp;
	CString strUser;
	CString strPwd;
	int		nPort;

	GetDlgItemText(IDC_IPADDRESS, strIp);
	GetDlgItemText(IDC_EDIT_USER, strUser);
	GetDlgItemText(IDC_EDIT_PASS, strPwd);
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
	m_lLoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (0 == m_lLoginID)
	{
		ShowLoginErrorReason(stOutparam.nError);
	}
	else
	{

		SetDlgItemText(IDC_BUTTON_LOGIN, ConvertString("Logout"));

		// Get the program path
		int filelen = GetModuleFileName(NULL, m_pszSoftPath, 1024);
		int k = filelen;
		while (*(m_pszSoftPath+k) != '\\')
		{
			k--;
		}
		*(m_pszSoftPath+(k+1)) = '\0';

		// create the floder to save face picture
		CString str = "\\Face";
		CString DownLoad = "\\DownLoad";
		DownLoad = m_pszSoftPath + DownLoad;		// DwonLoad path
		str = m_pszSoftPath + str;
		BOOL bCreate = CreateDirectory(str, NULL);
		BOOL bCreateDownLoad = CreateDirectory(DownLoad, NULL);
		if (bCreate && bCreateDownLoad)
		{
			CString Candidate = str + "\\Candidate"; // Save Contrast diagram path
			CString Global = str + "\\Global";		 // Save Global picture path
			CString Person = str + "\\Person";		 // Save Face Picture path
			CString RemoteFace = str + "\\RemoteFace"; // Save picture which search by picture path

			CString DwonLoadVideo = DownLoad + "\\FaceVideo";		//FaveVideo Download path
			CString DwonLoadFaceDetection = DownLoad + "\\FaceDetection";		//FaceDetection Download path
			CString DwonLoadTargetRecognition = DownLoad + "\\TargetRecognition";		//TargetRecognition Download path

			CreateDirectory(Candidate, NULL);
			CreateDirectory(Global, NULL);
			CreateDirectory(Person, NULL);
			CreateDirectory(RemoteFace, NULL);
			CreateDirectory(DwonLoadVideo, NULL);
			CreateDirectory(DwonLoadFaceDetection, NULL);
			CreateDirectory(DwonLoadTargetRecognition, NULL);
		}

		GetDlgItem(IDC_BUTTON_ATTACH)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_FACE_DB)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_REALPLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SearchByPic)->EnableWindow(TRUE);

		m_nChnCnt = stOutparam.stuDeviceInfo.nChanNum; // Total number of channels
		int nIndex = 0;
		m_comboChannel.ResetContent();
		for(int i = 1;i <= m_nChnCnt;i++)
		{
			CString str;
			str.Format("%d",i);
			nIndex = m_comboChannel.AddString(str);
			m_comboChannel.SetItemData(nIndex,i);
		}
		m_comboChannel.SetCurSel(0);
	}
}
void CTargetRecognitionDlg::OnButtonLogin() 
{
	BOOL bValid = UpdateData(TRUE);
	if (bValid)
	{
		CString strButton;
		GetDlgItemText(IDC_BUTTON_LOGIN, strButton);
		if (strButton == ConvertString("Login"))
		{
			OnButtonLoginIn();
		}
		else if (strButton == ConvertString("Logout"))
		{
			OnButtonLoginOut();
		}
	}
}

void CTargetRecognitionDlg::FillCWndWithDefaultColor(CWnd* cwnd) 
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
	CBrush brush(RGB(105,105,105));
	cdc->FillRect(&rect, &brush);
	cwnd->ReleaseDC(cdc);
}

void CTargetRecognitionDlg::ClearShowEventInfo()
{
	SetDlgItemText(IDC_EVENT_TYPE, ConvertString("(Event Type)"));
	SetDlgItemText(IDC_EDIT_EVENT_TIME, ConvertString(""));
	SetDlgItemText(IDC_EDIT_SEX, ConvertString(""));
	SetDlgItemText(IDC_EDIT_AGE, ConvertString(""));
	SetDlgItemText(IDC_EDIT_EYE, ConvertString(""));
	SetDlgItemText(IDC_EDIT_MOUTH, ConvertString(""));
	SetDlgItemText(IDC_EDIT_MASK, ConvertString(""));
	SetDlgItemText(IDC_EDIT_BEARD, ConvertString(""));
	SetDlgItemText(IDC_EDIT_PERSON_NAME, ConvertString(""));
	SetDlgItemText(IDC_EDIT_SEX2, ConvertString(""));
	SetDlgItemText(IDC_EDIT_BIRTHDAY, ConvertString(""));
	SetDlgItemText(IDC_EDIT_GROUP_ID, ConvertString(""));
	SetDlgItemText(IDC_EDIT_GROUP_NAME, ConvertString(""));
	SetDlgItemText(IDC_EDIT_SIMILARITY, ConvertString(""));
}

void CTargetRecognitionDlg::ShowFaceDataInfo(NET_FACE_DATA *pstDataInfo)
{
	if (NULL == pstDataInfo)
	{
		return;
	}
	if (pstDataInfo->emSex == EM_DEV_EVENT_FACEDETECT_SEX_TYPE_UNKNOWN)
	{
		SetDlgItemText(IDC_EDIT_SEX, ConvertString("Unknown"));
	}
	else if (pstDataInfo->emSex == EM_DEV_EVENT_FACEDETECT_SEX_TYPE_MAN)
	{
		SetDlgItemText(IDC_EDIT_SEX, ConvertString("Male"));
	}
	if (pstDataInfo->emSex == EM_DEV_EVENT_FACEDETECT_SEX_TYPE_WOMAN)
	{
		SetDlgItemText(IDC_EDIT_SEX, ConvertString("Female"));
	}

	if (pstDataInfo->nAge > 0)
	{
		SetDlgItemInt(IDC_EDIT_AGE, pstDataInfo->nAge);
	}
	else
	{
		SetDlgItemText(IDC_EDIT_AGE, ConvertString("Unknown"));
	}

	if (pstDataInfo->emEye == EM_EYE_STATE_CLOSE)
	{
		SetDlgItemText(IDC_EDIT_EYE, ConvertString("Close Eyes"));
	}
	else if (pstDataInfo->emEye == EM_EYE_STATE_OPEN)
	{
		SetDlgItemText(IDC_EDIT_EYE, ConvertString("Open Eyes"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_EYE, ConvertString("Unknown"));
	}

	if (pstDataInfo->emMouth == EM_MOUTH_STATE_CLOSE)
	{
		SetDlgItemText(IDC_EDIT_MOUTH, ConvertString("Close Mouth"));
	}
	else if (pstDataInfo->emMouth == EM_MOUTH_STATE_OPEN)
	{
		SetDlgItemText(IDC_EDIT_MOUTH, ConvertString("Open Mouth"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_MOUTH, ConvertString("Unknown"));
	}

	if (pstDataInfo->emMask == EM_MASK_STATE_NOMASK)
	{
		SetDlgItemText(IDC_EDIT_MASK, ConvertString("Not Wear"));
	}
	else if (pstDataInfo->emMask == EM_MASK_STATE_WEAR)
	{
		SetDlgItemText(IDC_EDIT_MASK, ConvertString("Wear"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_MASK, ConvertString("Unknown"));
	}

	if (pstDataInfo->emBeard == EM_BEARD_STATE_NOBEARD)
	{
		SetDlgItemText(IDC_EDIT_BEARD, ConvertString("No Beard"));
	}
	else if (pstDataInfo->emBeard == EM_BEARD_STATE_HAVEBEARD)
	{
		SetDlgItemText(IDC_EDIT_BEARD, ConvertString("Have Beard"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_BEARD, ConvertString("Unknown"));
	}
}

void CTargetRecognitionDlg::ShowFaceDataInfo(DEV_EVENT_FACEDETECT_INFO *pstDataInfo)
{
	if (NULL == pstDataInfo)
	{
		return;
	}
	if (pstDataInfo->emSex == EM_DEV_EVENT_FACEDETECT_SEX_TYPE_UNKNOWN)
	{
		SetDlgItemText(IDC_EDIT_SEX, ConvertString("Unknown"));
	}
	else if (pstDataInfo->emSex == EM_DEV_EVENT_FACEDETECT_SEX_TYPE_MAN)
	{
		SetDlgItemText(IDC_EDIT_SEX, ConvertString("Male"));
	}
	if (pstDataInfo->emSex == EM_DEV_EVENT_FACEDETECT_SEX_TYPE_WOMAN)
	{
		SetDlgItemText(IDC_EDIT_SEX, ConvertString("Female"));
	}

	if (pstDataInfo->nAge > 0)
	{
		SetDlgItemInt(IDC_EDIT_AGE, pstDataInfo->nAge);
	}
	else
	{
		SetDlgItemText(IDC_EDIT_AGE, ConvertString("Unknown"));
	}

	if (pstDataInfo->emEye == EM_EYE_STATE_CLOSE)
	{
		SetDlgItemText(IDC_EDIT_EYE, ConvertString("Close Eyes"));
	}
	else if (pstDataInfo->emEye == EM_EYE_STATE_OPEN)
	{
		SetDlgItemText(IDC_EDIT_EYE, ConvertString("Open Eyes"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_EYE, ConvertString("Unknown"));
	}

	if (pstDataInfo->emMouth == EM_MOUTH_STATE_CLOSE)
	{
		SetDlgItemText(IDC_EDIT_MOUTH, ConvertString("Close Mouth"));
	}
	else if (pstDataInfo->emMouth == EM_MOUTH_STATE_OPEN)
	{
		SetDlgItemText(IDC_EDIT_MOUTH, ConvertString("Open Mouth"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_MOUTH, ConvertString("Unknown"));
	}

	if (pstDataInfo->emMask == EM_MASK_STATE_NOMASK)
	{
		SetDlgItemText(IDC_EDIT_MASK, ConvertString("Not Wear"));
	}
	else if (pstDataInfo->emMask == EM_MASK_STATE_WEAR)
	{
		SetDlgItemText(IDC_EDIT_MASK, ConvertString("Wear"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_MASK, ConvertString("Unknown"));
	}

	if (pstDataInfo->emBeard == EM_BEARD_STATE_NOBEARD)
	{
		SetDlgItemText(IDC_EDIT_BEARD, ConvertString("No Beard"));
	}
	else if (pstDataInfo->emBeard == EM_BEARD_STATE_HAVEBEARD)
	{
		SetDlgItemText(IDC_EDIT_BEARD, ConvertString("Have Beard"));
	}
	else
	{
		SetDlgItemText(IDC_EDIT_BEARD, ConvertString("Unknown"));
	}
}

void CTargetRecognitionDlg::ShowFaceDetectPicture(DEV_EVENT_FACEDETECT_INFO *pstInfo, BYTE *pBuffer, DWORD dwBufSize)
{
	if (NULL == pstInfo)
	{
		return;
	}

	char szPicName[128] = {0};

	if (m_nFaceDetectGroupId != pstInfo->stuObject.nRelativeID) // 
	//if (m_nFaceDetectGroupId != pstInfo->stuFileInfo.nGroupId)
	{
		m_nFaceDetectGroupId = pstInfo->stuObject.nObjectID;
		if ((dwBufSize > 0) && pBuffer)
		{
			std::string filepath(m_pszSoftPath);
			filepath += "Face\\Global\\GlobalFace.jpg";

			FILE *fPic = fopen(filepath.c_str(), "wb+");
			if (fPic)
			{
				fwrite(pBuffer, 1, dwBufSize, fPic);
				fclose(fPic);

				// Display global picture
				m_pBicPic.SetImageFile(filepath.c_str());
			}
		}
	}
	else if (m_nFaceDetectGroupId == pstInfo->stuObject.nRelativeID) // First face 
	//else if (m_nFaceDetectGroupId == pstInfo->stuFileInfo.nGroupId) // First face picture
	{
		if ((dwBufSize > 0) && pBuffer)
		{
			std::string filepath(m_pszSoftPath);
			filepath += "Face\\Person\\PersonFace.jpg";

			FILE *fPic = fopen(filepath.c_str(), "wb+");
			if (fPic)
			{
				fwrite(pBuffer, 1, dwBufSize, fPic);
				fclose(fPic);

				// Display small picture
				m_pFacePic.SetImageFile(filepath.c_str());
				//ShowFacePicture(pFileName, IDC_STATIC_PERSON1, &m_pPersonPicture);
			}
			ShowFaceDataInfo(pstInfo);
		}
	}
}

// Display global picture
void CTargetRecognitionDlg::ShowGlobalPicture(DEV_EVENT_FACERECOGNITION_INFO *pstInfo, BYTE *pBuffer)
{
	char szPicName[128] = {0};
	char *pFileName = NULL;

	if (FALSE == pstInfo->bGlobalScenePic) // No global picture
	{
		FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_BIG_PIC));
		MessageBox(ConvertString("No global scene picture!"), "");
	}
	else
	{
		DH_PIC_INFO *pGlobalPic = &(pstInfo->stuGlobalScenePicInfo);
		if ((pGlobalPic->dwFileLenth > 0) && pBuffer)
		{
			CString filepath(m_pszSoftPath);
			CString filename("Face\\Global\\GlobalFace.jpg");
			CString strFile = filepath + filename;
			pFileName = strFile.GetBuffer(200);
			FILE *fPic = fopen(pFileName, "wb+");
			if (fPic)
			{
				fwrite(pBuffer + pGlobalPic->dwOffSet, 1, pGlobalPic->dwFileLenth, fPic);
				fclose(fPic);
				
				// show global picture
				m_pBicPic.SetImageFile(pFileName);
			}
		}
		else
		{
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_BIG_PIC));

			CString str1 = ConvertString("The path of global scene picture on the device is:");
			memset(szPicName, 0, sizeof(szPicName));
			_snprintf(szPicName, sizeof(szPicName)-1, "%s!", pGlobalPic->pszFilePath);
			CString str2(szPicName);
			CString str = str1+str2;
			MessageBox(str, "");
		}
	}
}

// Display face picture
void CTargetRecognitionDlg::ShowPersonPicture(DEV_EVENT_FACERECOGNITION_INFO *pstInfo, BYTE *pBuffer)
{
	char szPicName[128] = {0};
	char *pFileName = NULL;

	DH_MSG_OBJECT *pstFaceInfo = &(pstInfo->stuObject);
	if ((pstFaceInfo->stPicInfo.dwFileLenth >0) && pBuffer)
	{
		CString filepath(m_pszSoftPath);
		CString filename("Face\\Person\\PersonFace.jpg");
		CString strFile = filepath + filename;
		pFileName = strFile.GetBuffer(200);
		FILE *fPic = fopen(pFileName, "wb+");
		if (fPic)
		{
			fwrite(pBuffer + pstFaceInfo->stPicInfo.dwOffSet, 1, pstFaceInfo->stPicInfo.dwFileLenth, fPic);
			fclose(fPic);
			
			// Display small picture
			m_pFacePic.SetImageFile(pFileName);
			//ShowFacePicture(pFileName, IDC_STATIC_PERSON1, &m_pPersonPicture);
		}
	}
	else
	{
		FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PERSON1));

		CString str1 = ConvertString("The path of person picture on the device is:");
		memset(szPicName, 0, sizeof(szPicName));
		_snprintf(szPicName, sizeof(szPicName)-1, "%s!", pstFaceInfo->stPicInfo.pszFilePath);
		CString str2(szPicName);
		CString str = str1+str2;
		MessageBox(str, "");
	}
}

// Display Contrast diagram
void CTargetRecognitionDlg::ShowCandidatePicture(DEV_EVENT_FACERECOGNITION_INFO *pstInfo, BYTE *pBuffer)
{
	char szPicName[128] = {0};
	char *pFileName = NULL;
	int i = 0;

	SetDlgItemInt(IDC_EDIT_SIMILARITY, 0);
	SetDlgItemText(IDC_EDIT_GROUP_ID, ConvertString("Unknown"));
	SetDlgItemText(IDC_EDIT_GROUP_NAME, ConvertString("Unknown"));
	SetDlgItemText(IDC_EDIT_PERSON_NAME, ConvertString("Unknown"));
	SetDlgItemText(IDC_EDIT_BIRTHDAY, ConvertString("Unknown"));
	SetDlgItemText(IDC_EDIT_SEX2, ConvertString("Unknown"));
	if (pstInfo->nCandidateNum <= 0)
	{
		SetDlgItemText(IDC_EDIT_SIMILARITY, ConvertString("Stranger"));
		FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_CANDI1));
	}
	else
	{
		CANDIDATE_INFOEX *pstCandidate = &(pstInfo->stuCandidatesEx[0]);
		for (i = 0; i < pstInfo->nCandidateNum; i++)
		{
			if (NULL != pstCandidate)
			{
				if (pstInfo->stuCandidatesEx[i].bySimilarity > pstCandidate->bySimilarity)
				{
					pstCandidate = &(pstInfo->stuCandidatesEx[i]);
				}
			}
		}
			
		SetDlgItemInt(IDC_EDIT_SIMILARITY, (int)pstCandidate->bySimilarity);
			
		//Person info
		FACERECOGNITION_PERSON_INFOEX  *pstPersonInfo = &(pstCandidate->stPersonInfo);
		if (strlen(pstPersonInfo->szGroupID) > 0)
		{
			SetDlgItemText(IDC_EDIT_GROUP_ID, pstPersonInfo->szGroupID);
		}
		if (strlen(pstPersonInfo->szGroupName) > 0)
		{
			SetDlgItemText(IDC_EDIT_GROUP_NAME, pstPersonInfo->szGroupName);
		}
		if (strlen(pstPersonInfo->szPersonName) > 0)
		{
			SetDlgItemText(IDC_EDIT_PERSON_NAME, pstPersonInfo->szPersonName);
		}
		char szBirthDay[11] = {0};
		_snprintf(szBirthDay, sizeof(szBirthDay)-1, "%04d-%02d-%02d", pstPersonInfo->wYear, pstPersonInfo->byMonth, pstPersonInfo->byDay);
		SetDlgItemText(IDC_EDIT_BIRTHDAY, szBirthDay);

		if (1 == pstPersonInfo->bySex)
		{
			SetDlgItemText(IDC_EDIT_SEX2, ConvertString("Male"));
		}
		else if (2 == pstPersonInfo->bySex)
		{
			SetDlgItemText(IDC_EDIT_SEX2, ConvertString("Female"));
		}
			
		for (int j = 0; j < pstPersonInfo->wFacePicNum && j < 1; j++) // Only report one picture
		{
			DH_PIC_INFO *pstFacePicInfo = &(pstPersonInfo->szFacePicInfo[j]);
			if ((pstFacePicInfo->dwFileLenth > 0) && pBuffer)
			{	
				CString filepath(m_pszSoftPath);
				CString filename("Face\\Candidate\\Candidate.jpg");
				CString strFile = filepath + filename;
				pFileName = strFile.GetBuffer(200);
				FILE *fPic = fopen(pFileName, "wb+");
				if (fPic)
				{
					fwrite(pBuffer + pstFacePicInfo->dwOffSet, 1, pstFacePicInfo->dwFileLenth, fPic);
					fclose(fPic);
					// Display Contrast diagram
					m_pCandiPic.SetImageFile(pFileName);
				}
			}
			else
			{
				FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_CANDI1+j));

				CString str1 = ConvertString("The path of candidate picture on the device is:");
				memset(szPicName, 0, sizeof(szPicName));
				_snprintf(szPicName, sizeof(szPicName), "%s!", pstCandidate->stuSceneImage.szFilePath);
				CString str2(szPicName);
				CString str = str1+str2;
				MessageBox(str, "");
			}
		}
	}
}

void CTargetRecognitionDlg::ClearCandidateShowInfo()
{
	SetDlgItemText(IDC_EDIT_SIMILARITY, "");
	SetDlgItemText(IDC_EDIT_GROUP_ID, "");
	SetDlgItemText(IDC_EDIT_GROUP_NAME, "");
	SetDlgItemText(IDC_EDIT_PERSON_NAME, "");
	SetDlgItemText(IDC_EDIT_BIRTHDAY, "");
	SetDlgItemText(IDC_EDIT_SEX2, "");

	m_pCandiPic.SetImageFile(NULL);	
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_CANDI1));
}

void CTargetRecognitionDlg::DoFaceData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize)
{
	if (0 == lAnalyzerHandle || NULL == pAlarmInfo)
	{
		MessageBox(ConvertString("Invalid param!"), ConvertString("Prompt"));
		return;
	}

	switch(dwAlarmType)
	{
	case EVENT_IVS_FACERECOGNITION: // TargetRecognition event
		{
			SetDlgItemText(IDC_EVENT_TYPE, ConvertString("(Target Recognition)"));
			DEV_EVENT_FACERECOGNITION_INFO *pstInfo = (DEV_EVENT_FACERECOGNITION_INFO *)pAlarmInfo;

			char szEventTime[20] = {0};
			_snprintf(szEventTime, sizeof(szEventTime)-1, "%04d-%02d-%02d %02d:%02d:%02d", pstInfo->UTC.dwYear, pstInfo->UTC.dwMonth, pstInfo->UTC.dwDay, pstInfo->UTC.dwHour,pstInfo->UTC.dwMinute,pstInfo->UTC.dwSecond);
			SetDlgItemText(IDC_EDIT_EVENT_TIME, szEventTime);

			ShowGlobalPicture(pstInfo, pBuffer); // Global picture

			/**************Face attributes info ******************************/
			NET_FACE_DATA *pstDataInfo = &(pstInfo->stuFaceData);
			ShowFaceDataInfo(pstDataInfo);

			/*************Face picture**********************************************/
			DH_MSG_OBJECT *pstFaceInfo = &(pstInfo->stuObject);
			ShowPersonPicture(pstInfo, pBuffer);

			/**************Candidate info (Contrast diagram)*******************************/
			ShowCandidatePicture(pstInfo, pBuffer);
			break;
		}
	case  EVENT_IVS_FACEDETECT:			//FACEDETECTION event
		{
			SetDlgItemText(IDC_EVENT_TYPE, ConvertString("(Face Detect)"));
			DEV_EVENT_FACEDETECT_INFO *pstInfo = (DEV_EVENT_FACEDETECT_INFO*)pAlarmInfo;

			char szEventTime[20] = {0};
			_snprintf(szEventTime, sizeof(szEventTime)-1, "%04d-%02d-%02d %02d:%02d:%02d", pstInfo->UTC.dwYear, pstInfo->UTC.dwMonth, pstInfo->UTC.dwDay, pstInfo->UTC.dwHour,pstInfo->UTC.dwMinute,pstInfo->UTC.dwSecond);
			SetDlgItemText(IDC_EDIT_EVENT_TIME, szEventTime);
			ClearCandidateShowInfo();
			ShowFaceDetectPicture(pstInfo, pBuffer, dwBufSize);
			break;
		}
	}
}

int CALLBACK AnalyzerData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
	if(dwUser == 0)
	{
		return 0;
	}

	CTargetRecognitionDlg *dlg = (CTargetRecognitionDlg*)dwUser;

	HWND hWnd = dlg->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return 0;
	}
	strAnalyInfo *info = new strAnalyInfo;
	memset(info, 0, sizeof(strAnalyInfo));
	info->lAnalyzerHandle = lAnalyzerHandle;
	info->dwAlarmType = dwAlarmType;
	if (dwAlarmType == EVENT_IVS_FACERECOGNITION)
	{
		info->pAlarmInfo = new DEV_EVENT_FACERECOGNITION_INFO;
		memcpy(info->pAlarmInfo, (DEV_EVENT_FACERECOGNITION_INFO*)pAlarmInfo, sizeof(DEV_EVENT_FACERECOGNITION_INFO));
	}
	else if (dwAlarmType == EVENT_IVS_FACEDETECT)
	{
		info->pAlarmInfo = new DEV_EVENT_FACEDETECT_INFO;
		memcpy(info->pAlarmInfo, (DEV_EVENT_FACEDETECT_INFO*)pAlarmInfo, sizeof(DEV_EVENT_FACEDETECT_INFO));
	}
	else
	{
		delete info;
		info = NULL;
		return 0;
	}
	info->dwBufSize=dwBufSize;
	if (dwBufSize > 0)
	{
		info->pBuffer = new BYTE[dwBufSize];
		memset(info->pBuffer, 0, dwBufSize);
		memcpy(info->pBuffer, pBuffer, dwBufSize);
	}

	::PostMessage(hWnd, WM_DEVICE_DOATTACH, WPARAM(info), NULL);

	return 1;
}

void CTargetRecognitionDlg::OnButtonAttachOrDeteach() 
{
	CString strButton;
	GetDlgItemText(IDC_BUTTON_ATTACH, strButton);
	if (strButton == ConvertString("Attach"))
	{
		int nIndex = m_comboChannel.GetCurSel();
		if(CB_ERR != nIndex)
		{
			int nChannel = m_comboChannel.GetItemData(nIndex) - 1;
			m_lRealPicHandle = CLIENT_RealLoadPictureEx(m_lLoginID, nChannel, EVENT_IVS_ALL, TRUE, AnalyzerData, (DWORD)this, NULL);
			if (0 == m_lRealPicHandle)
			{
				MessageBox(ConvertString("Failed to attach face event!"), "");
			}
			else
			{	
				SetDlgItemText(IDC_BUTTON_ATTACH, ConvertString("Detach"));
			}
		}
	}
	else if (strButton == ConvertString("Detach"))
	{
		OnButtonDeteach();
	}

	return;
}

void CTargetRecognitionDlg::OnButtonDeteach() 
{
	m_nFaceDetectGroupId = 0;
	if (0 != m_lRealPicHandle)
	{
		BOOL bRet = CLIENT_StopLoadPic(m_lRealPicHandle);
		if (!bRet)
		{
			MessageBox(ConvertString("Failed to stop load picture!"), "");
		}
		else
		{
			ClearShowEventInfo();
			SetDlgItemText(IDC_BUTTON_ATTACH, ConvertString("Attach"));
			m_lRealPicHandle = 0;
			
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_BIG_PIC));
			m_pBicPic.SetImageFile(NULL);
			
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PERSON1));
			m_pFacePic.SetImageFile(NULL);

			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_CANDI1));
			m_pCandiPic.SetImageFile(NULL);	
		}
	}

	return;
}

void CTargetRecognitionDlg::OnButtonLoginOut() 
{
	SetWindowText(ConvertString("TargetRecognition"));
	OnButtonDeteach(); // Stop attach

	if (0 != m_lRealHandle) //Stop realplay
	{
		CLIENT_StopRealPlayEx(m_lRealHandle);
		m_lRealHandle = 0;
		SetDlgItemText(IDC_BUTTON_REALPLAY, ConvertString("StartRealPlay"));
		FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_VIDEO));
	}

	if (0 != m_lLoginID)
	{
		// logout device
		BOOL bRet = CLIENT_Logout(m_lLoginID);
		if (!bRet)
		{
			MessageBox(ConvertString("Failed to login out!"), "");
		}
		else
		{
			m_lRealHandle = 0;
			SetDlgItemText(IDC_BUTTON_LOGIN, ConvertString("Login"));
			GetDlgItem(IDC_BUTTON_LOGIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_ATTACH)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_FACE_DB)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_REALPLAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_SearchByPic)->EnableWindow(FALSE);
		}
	}
}


void CTargetRecognitionDlg::OnButtonFaceDb() 
{
	CFaceDbOpreate FaceDbOpreateDlg(m_lLoginID, m_nChnCnt, m_pszSoftPath);
	FaceDbOpreateDlg.DoModal();
}

void CTargetRecognitionDlg::OnBnClickedButtonRealplay()
{
	CString strButton;
	GetDlgItemText(IDC_BUTTON_REALPLAY, strButton);
	if (strButton == ConvertString("StartRealPlay"))
	{
		int nIndex = m_comboChannel.GetCurSel();
		if(CB_ERR != nIndex)
		{
			int nChannel = m_comboChannel.GetItemData(nIndex) - 1;
			HWND hWnd = GetDlgItem(IDC_STATIC_VIDEO)->m_hWnd;
			if (NULL == hWnd)
			{
				MessageBox(ConvertString("Failed to get window handle!"), "");
			}
			else
			{
				LLONG lRet = CLIENT_RealPlayEx(m_lLoginID, nChannel, hWnd, DH_RType_Realplay_0);
				if (0 == lRet)
				{
					MessageBox(ConvertString("Failed to real play!"), "");
				}
				else
				{
					SetDlgItemText(IDC_BUTTON_REALPLAY, ConvertString("StopRealPlay"));

					m_lRealHandle = lRet;
					BOOL bRet = CLIENT_RenderPrivateData(m_lRealHandle, TRUE);
					if (!bRet)
					{
						MessageBox(ConvertString("Failed to render private data failed!"), "");
					}
				}
			}
		}
	}
	else if (strButton == ConvertString("StopRealPlay"))
	{
		if (0 != m_lRealHandle)
		{
			BOOL bRet = CLIENT_RenderPrivateData(m_lRealHandle, FALSE);
			if (!bRet)
			{
				MessageBox(ConvertString("Failed to stop render private data!"), "");
			}
			CLIENT_StopRealPlayEx(m_lRealHandle);
			SetDlgItemText(IDC_BUTTON_REALPLAY, ConvertString("StartRealPlay"));
			m_lRealHandle = 0;
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_VIDEO));
		}
	}
}

void CTargetRecognitionDlg::OnBnClickedButtonQueryDownload()
{
	CQueryDownLoad dlg(m_nChnCnt, m_lLoginID, m_pszSoftPath);
	dlg.DoModal();
}

void CTargetRecognitionDlg::OnBnClickedButtonSearchbypic()
{
	CSearchByPic SearchByPicDlg(m_lLoginID, m_nChnCnt, m_pszSoftPath);
	SearchByPicDlg.DoModal();
}

BOOL CTargetRecognitionDlg::PreTranslateMessage(MSG* pMsg)
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

LRESULT CTargetRecognitionDlg::DoDeviceAttach(WPARAM wParam, LPARAM lParam)
{
	strAnalyInfo *pInfo = (strAnalyInfo *)wParam;
	if (pInfo == NULL)
	{
		return 0;
	}

	DoFaceData(pInfo->lAnalyzerHandle, pInfo->dwAlarmType, pInfo->pAlarmInfo, pInfo->pBuffer, pInfo->dwBufSize);

	if (pInfo->pBuffer)
	{
		delete[] pInfo->pBuffer;
		pInfo->pBuffer = NULL;
	}
	if (pInfo->pAlarmInfo)
	{
		delete pInfo->pAlarmInfo;
		pInfo->pAlarmInfo = NULL;
	}

	if (pInfo)
	{
		delete pInfo;
		pInfo = NULL;
	}
	return 0;
}