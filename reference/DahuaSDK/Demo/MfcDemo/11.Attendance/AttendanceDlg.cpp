// AttendanceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Attendance.h"
#include "AttendanceDlg.h"
#include "Util.h"
#include "AttendanceManager.h"
#include "UserDetailDlg.h"
#include "FingerprintByUserIDDlg.h"
#include "FingerprintByFPID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ACCESS_CONTROL	(WM_USER+200)
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


// CAttendanceDlg 对话框


CAttendanceDlg::CAttendanceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAttendanceDlg::IDD, pParent)
	, m_nPort(0)
	, m_strLoginUser(_T(""))
	, m_strLoginPasswd(_T(""))
	, m_bLogined(false)
	, m_bSubscribed(false)
	, m_nOffset(0)
	, m_nQueryCount(100)
	, m_nTotal(0)
	, m_strGetUser_USERID(_T(""))
	, m_nFindUserCount(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAttendanceManager = new CAttendanceManager;
	m_nEventIndex = 1;
}

void CAttendanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER_INFO, m_listCtlUserInfo);
	DDX_Control(pDX, IDC_LIST_EVENT, m_EventListCtl);
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_ipCtrl);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_USER, m_strLoginUser);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strLoginPasswd);
	DDX_Text(pDX, IDC_EDIT_OFFSET, m_nOffset);
	DDX_Text(pDX, IDC_EDIT_QUERYCOUNT, m_nQueryCount);
	//	DDV_MinMaxInt(pDX, m_nQueryCount, 1, 100);
	DDX_Text(pDX, IDC_EDIT_TOTAL, m_nTotal);
	DDX_Text(pDX, IDC_EDIT_GETUSER_USERID, m_strGetUser_USERID);
	DDX_Text(pDX, IDC_EDIT_USERCOUNT, m_nFindUserCount);
}

BEGIN_MESSAGE_MAP(CAttendanceDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_ACCESS_CONTROL, OnAccessCtlEvent)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CAttendanceDlg::OnBnClickedButtonLogin)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_ADDUSER, &CAttendanceDlg::OnBnClickedButtonAdduser)
	ON_BN_CLICKED(IDC_BUTTON_MODIFYUSER, &CAttendanceDlg::OnBnClickedButtonModifyuser)
	ON_BN_CLICKED(IDC_BUTTON_DELETEUSER, &CAttendanceDlg::OnBnClickedButtonDeleteuser)
	ON_BN_CLICKED(IDC_BUTTON_GETUSER, &CAttendanceDlg::OnBnClickedButtonGetuser)
	ON_BN_CLICKED(IDC_EDIT_SUBSCRIBE, &CAttendanceDlg::OnBnClickedEditSubscribe)
	ON_BN_CLICKED(IDC_BUTTON_FINDUSER, &CAttendanceDlg::OnBnClickedButtonFinduser)
	ON_BN_CLICKED(IDC_BUTTON_OPERATE_FINGERPRINT_BYUSERID, &CAttendanceDlg::OnBnClickedButtonOperateFingerprintByuserid)
	ON_BN_CLICKED(IDC_BUTTON_OPERATE_FINGERPRINT_BYFPID, &CAttendanceDlg::OnBnClickedButtonOperateFingerprintByfpid)
END_MESSAGE_MAP()


// CAttendanceDlg 消息处理程序

BOOL CAttendanceDlg::OnInitDialog()
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
	//init userinfo list control
	InitUserListCtl();

	InitEventListCtl();

	initLoginParam();

	EnableButtons(false);

	UpdateData(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


CString GetOpenMethodStr(NET_ACCESS_DOOROPEN_METHOD emMethod)
{
	CString pcMethodRet = NULL;
	switch(emMethod)
	{
	case NET_ACCESS_DOOROPEN_METHOD_CARD:                  
		{
			pcMethodRet = ConvertString("Card");	
		}
		break;
	case NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT:
		{
			pcMethodRet = ConvertString("FingerPrint");	
		}
		break;
	default:
		{
			pcMethodRet = ConvertString("UnKnown");	
		}
		break;
	}

	return pcMethodRet;
}

LRESULT CAttendanceDlg::OnAccessCtlEvent(WPARAM wParam, LPARAM lParam)
{
	DEV_EVENT_ACCESS_CTL_INFO *pEventInfo = (DEV_EVENT_ACCESS_CTL_INFO *)wParam;
	if (NULL == pEventInfo)
	{
		return -1;
	}
	
	if (m_nEventIndex > 50)
	{
		int nTmpIndex = m_EventListCtl.GetItemCount();
		m_EventListCtl.DeleteItem(nTmpIndex-1);
	}

	int nIndex = m_EventListCtl.InsertItem(0, NULL);
	CString strIndex;
	strIndex.Format("%d", m_nEventIndex);
	m_EventListCtl.SetItemText(nIndex, 0, strIndex);
	m_EventListCtl.SetItemText(nIndex, 1, pEventInfo->szUserID);
	m_EventListCtl.SetItemText(nIndex, 2, pEventInfo->szCardNo);
	CString strTime;
	strTime.Format("%.4d-%.2d-%.2d %.2d:%.2d:%.2d", pEventInfo->UTC.dwYear, pEventInfo->UTC.dwMonth, pEventInfo->UTC.dwDay,
		pEventInfo->UTC.dwHour, pEventInfo->UTC.dwMinute, pEventInfo->UTC.dwSecond);
	m_EventListCtl.SetItemText(nIndex, 3, strTime);
	m_EventListCtl.SetItemText(nIndex, 4, GetOpenMethodStr(pEventInfo->emOpenMethod));
	m_nEventIndex++;

	delete pEventInfo;
	pEventInfo = NULL;
	return 0;
}

void CAttendanceDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CAttendanceDlg::OnPaint()
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
HCURSOR CAttendanceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CAttendanceDlg::InitUserListCtl(void)
{
	m_listCtlUserInfo.SetExtendedStyle(m_listCtlUserInfo.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_listCtlUserInfo.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 100, -1);
	m_listCtlUserInfo.InsertColumn(1, ConvertString("UserID"), LVCFMT_LEFT, 100, -1);
	m_listCtlUserInfo.InsertColumn(2, ConvertString("UserName"), LVCFMT_LEFT, 100, -1);
	m_listCtlUserInfo.InsertColumn(3, ConvertString("CardNo"), LVCFMT_LEFT, 100, -1);

	return 0;
}

int CAttendanceDlg::InitEventListCtl(void)
{
	m_EventListCtl.SetExtendedStyle(m_EventListCtl.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_EventListCtl.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 100, -1);
	m_EventListCtl.InsertColumn(1, ConvertString("UserID"), LVCFMT_LEFT, 100, -1);
	m_EventListCtl.InsertColumn(2, ConvertString("CardNo"), LVCFMT_LEFT, 100, -1);
	m_EventListCtl.InsertColumn(3, ConvertString("Time"), LVCFMT_LEFT, 150, -1);
	m_EventListCtl.InsertColumn(4, ConvertString("OpenMethod"), LVCFMT_LEFT, 100, -1);
	return 0;
}

void CAttendanceDlg::OnBnClickedButtonLogin()
{
	// TODO: 在此添加控件通知处理程序代码

	
	if (!m_bLogined)
	{
		UpdateData(TRUE);

		CString csIp;
		m_ipCtrl.GetWindowText(csIp);

		char *pcIP = (LPSTR)(LPCSTR)csIp;
		char *pcUser = (LPSTR)(LPCSTR)m_strLoginUser;
		char *pcPwssd = (LPSTR)(LPCSTR)m_strLoginPasswd;

		int nError = 0;
		m_bLogined = m_pAttendanceManager->LoginDevice(pcIP, m_nPort, pcUser, pcPwssd, &nError);
		if (m_bLogined)
		{
			SetDlgItemText(IDC_BUTTON_LOGIN, ConvertString("LogOut"));
			EnableButtons(true);
		}
		else
		{
			m_pAttendanceManager->GetLastError();
			ShowLoginErrorReason(nError);
		}
	}
	else
	{
		// 如果订阅了则停止订阅
		if (m_bSubscribed)
		{
			OnBnClickedEditSubscribe();
		}
		bool bRet = m_pAttendanceManager->LogoutDevice();
		if (false == bRet)
		{
			m_pAttendanceManager->GetLastError();	
		}
		m_bLogined = false;
		SetDlgItemText(IDC_BUTTON_LOGIN, ConvertString("Login"));
		RefreshUI();
	}

}

int CAttendanceDlg::initLoginParam(void)
{
	m_ipCtrl.SetWindowText("172.23.29.60");
	m_nPort = 37777;
	m_strLoginUser = "admin";
	m_strLoginPasswd = "admin123";
	return 0;
}

void CAttendanceDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	if (m_pAttendanceManager)
	{
		delete m_pAttendanceManager;
		m_pAttendanceManager = NULL;
	}
}

void CAttendanceDlg::ShowLoginErrorReason(int nError)
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

void CAttendanceDlg::OnBnClickedButtonAdduser()
{
	// TODO: 在此添加控件通知处理程序代码
	CUserDetailDlg AddUserDlg(m_pAttendanceManager, "","","",EM_USER_OPERATOR_TYPE_ADD);
	AddUserDlg.DoModal();
}

int CAttendanceDlg::EnableButtons(bool bEnable)
{
	BOOL bFlag = TRUE;
	if (false == bEnable)
	{
		bFlag = FALSE;	
	}
	
	GetDlgItem(IDC_BUTTON_ADDUSER)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_MODIFYUSER)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_DELETEUSER)->EnableWindow(bFlag);
	GetDlgItem(IDC_EDIT_SUBSCRIBE)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_GETUSER)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_FINDUSER)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_OPERATE_FINGERPRINT_BYUSERID)->EnableWindow(bFlag);
	GetDlgItem(IDC_BUTTON_OPERATE_FINGERPRINT_BYFPID)->EnableWindow(bFlag);
	
	return 0;
}

void CAttendanceDlg::OnBnClickedButtonModifyuser()
{
	// TODO: 在此添加控件通知处理程序代码
	int nSelectCount = m_listCtlUserInfo.GetSelectedCount();
	if (0 == nSelectCount)
	{
		MessageBox(ConvertString("Please select one user to modify"), ConvertString("Prompt"));
		return;
	}
	
	int nIndex = m_listCtlUserInfo.GetSelectionMark();
	char szUserID[MAX_COMMON_STRING_32]={0};
	m_listCtlUserInfo.GetItemText(nIndex, 1, szUserID, sizeof(szUserID));
	char szUserName[MAX_ATTENDANCE_USERNAME_LEN]={0};
	m_listCtlUserInfo.GetItemText(nIndex, 2, szUserName, sizeof(szUserName));
	char szCardNo[MAX_COMMON_STRING_32]={0};
	m_listCtlUserInfo.GetItemText(nIndex, 3, szCardNo, sizeof(szCardNo));

	CUserDetailDlg ModifyUserDlg(m_pAttendanceManager, szUserID, szUserName, szCardNo,EM_USER_OPERATOR_TYPE_MODIFY);
	ModifyUserDlg.DoModal();
}

void CAttendanceDlg::OnBnClickedButtonDeleteuser()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDelUserDlg delUserDlg(m_pAttendanceManager);
	//delUserDlg.DoModal();
	// TODO: 在此添加控件通知处理程序代码
	int nSelectCount = m_listCtlUserInfo.GetSelectedCount();
	if (0 == nSelectCount)
	{
		MessageBox(ConvertString("Please select one user to delete"), ConvertString("Prompt"));
		return;
	}

	int nIndex = m_listCtlUserInfo.GetSelectionMark();
	char szUserID[256]={0};
	m_listCtlUserInfo.GetItemText(nIndex, 1, szUserID, sizeof(szUserID));

	NET_CTRL_IN_FINGERPRINT_REMOVE_BY_USERID stuInRemove = {sizeof(stuInRemove)};
	strncpy(stuInRemove.szUserID, szUserID, sizeof(stuInRemove.szUserID)-1);
	NET_CTRL_OUT_FINGERPRINT_REMOVE_BY_USERID stuOutRemove = {sizeof(stuOutRemove)};
	m_pAttendanceManager->RemoveFingerByUserID(&stuInRemove, &stuOutRemove);

	NET_IN_ATTENDANCE_DELUSER stuInDelUser = { sizeof(stuInDelUser)};
	strncpy(stuInDelUser.szUserID, szUserID, sizeof(stuInDelUser.szUserID)-1);
	bool bRet = m_pAttendanceManager->DelAttendanceUser(&stuInDelUser);
	if (true == bRet)
	{
		MessageBox(ConvertString("delete user success"), ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("delete user failed"), ConvertString("Prompt"));
	}
}

void CAttendanceDlg::OnBnClickedButtonGetuser()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	NET_IN_ATTENDANCE_GetUSER stuInGetUser = { sizeof(stuInGetUser)};
	char *pcUserID = (LPSTR)(LPCSTR)m_strGetUser_USERID;
	strncpy(stuInGetUser.szUserID, pcUserID, sizeof(stuInGetUser.szUserID)-1);
	NET_OUT_ATTENDANCE_GetUSER stuOuTGetUser = { sizeof(stuOuTGetUser)};
	stuOuTGetUser.nMaxLength = 1024*100;
	stuOuTGetUser.pbyPhotoData = (BYTE *)new char [stuOuTGetUser.nMaxLength];
	bool bRet = m_pAttendanceManager->GetAttendanceUser(&stuInGetUser, &stuOuTGetUser);
	if (true == bRet)
	{
		UpdateUserList(&stuOuTGetUser.stuUserInfo, 1, 1, false);
	}
	else
	{
		m_pAttendanceManager->GetLastError();
		UpdateUserList(NULL, 0, 1, false);
		MessageBox(ConvertString("get attendance user failed"), ConvertString("Prompt"));
	}
	delete []stuOuTGetUser.pbyPhotoData ;
	stuOuTGetUser.pbyPhotoData  = NULL;
}

void CAttendanceDlg::OnBnClickedEditSubscribe()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_bSubscribed)
	{
		bool bRet = m_pAttendanceManager->RealLoadPicture(-1, EVENT_IVS_ACCESS_CTL, this);
		if (true == bRet)
		{
			m_bSubscribed = true;
			m_nEventIndex = 1;
			SetDlgItemText(IDC_EDIT_SUBSCRIBE, ConvertString("Stop Subscribe"));
		}
		else
		{
			m_pAttendanceManager->GetLastError();
			MessageBox(ConvertString("Subscribe failed"), ConvertString("Prompt"));
		}
	}
	else
	{
		bool bRet = m_pAttendanceManager->StopLoadPicture();
		if (false == bRet)
		{
			m_pAttendanceManager->GetLastError();	
		}
		m_bSubscribed = false;
		m_EventListCtl.DeleteAllItems();
		SetDlgItemText(IDC_EDIT_SUBSCRIBE, ConvertString("Subscribe"));
	}
	
}

int CAttendanceDlg::updateAttendanceInfo(ATTENDANCE_MSG_TYPE msgType, void *pMsgInfo, BYTE *pBuffer, DWORD dwBufSize)
{
	if (ATTENDANCE_MSG_TYPE_IVS_ACCESS_CTL != msgType || NULL == pMsgInfo)
	{
		return -1;
	}

	DEV_EVENT_ACCESS_CTL_INFO *pAccessCtl = new DEV_EVENT_ACCESS_CTL_INFO;
	if (NULL != pAccessCtl)
	{
		memcpy(pAccessCtl, pMsgInfo, sizeof(*pAccessCtl));
		PostMessage(WM_ACCESS_CONTROL, (WPARAM)pAccessCtl, NULL);
	}
	
	return 0;
}

void CAttendanceDlg::UpdateUserList(void *pUserList, int nUserCount, int nTotalCount, bool bUpdateTotalCount)
{
	m_nFindUserCount = nUserCount;
	if (true == bUpdateTotalCount)
	{
		m_nTotal = nTotalCount;
	}
	UpdateData(FALSE);

	m_listCtlUserInfo.DeleteAllItems();

	if (pUserList== NULL || 0 >= nUserCount)
	{
		return ;
	}
	
	NET_ATTENDANCE_USERINFO *pstuUserList = (NET_ATTENDANCE_USERINFO*)pUserList;
	for (int i = 0; i < nUserCount; i++)
	{
		int nIndex = m_listCtlUserInfo.InsertItem(i, NULL);
		CString strIndex;
		strIndex.Format("%d", i);
		m_listCtlUserInfo.SetItemText(nIndex, 0, strIndex);
		m_listCtlUserInfo.SetItemText(nIndex, 1, pstuUserList->szUserID);
		m_listCtlUserInfo.SetItemText(nIndex, 2, pstuUserList->szUserName);
		m_listCtlUserInfo.SetItemText(nIndex, 3, pstuUserList->szCardNo);
		pstuUserList++;
	}
}

void CAttendanceDlg::OnBnClickedButtonFinduser()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (0 > m_nOffset)
	{
		UpdateUserList(NULL, 0,0, true);
		MessageBox(ConvertString("offset invalied"), ConvertString("Prompt"));
		return ;
	}

	if (m_nQueryCount > 100 || m_nQueryCount < 1)
	{
		UpdateUserList(NULL, 0,0, true);
		MessageBox(ConvertString("querycount invalied"), ConvertString("Prompt"));
		return ;
	}

	
	NET_IN_ATTENDANCE_FINDUSER stuInFindUser = { sizeof(stuInFindUser)};
	stuInFindUser.nOffset = m_nOffset;
	stuInFindUser.nPagedQueryCount = m_nQueryCount;

	NET_OUT_ATTENDANCE_FINDUSER stuOutFindUser = { sizeof(stuOutFindUser)};
	stuOutFindUser.nMaxUserCount = 100;
	stuOutFindUser.stuUserInfo = new NET_ATTENDANCE_USERINFO[stuOutFindUser.nMaxUserCount];
	stuOutFindUser.nMaxPhotoDataLength = 1024*1024;
	stuOutFindUser.pbyPhotoData = (BYTE *)new char[stuOutFindUser.nMaxPhotoDataLength];
	bool bRet = m_pAttendanceManager->FindAttendanceUser(&stuInFindUser, &stuOutFindUser);
	if (true == bRet)
	{
		UpdateUserList(stuOutFindUser.stuUserInfo, stuOutFindUser.nRetUserCount, stuOutFindUser.nTotalUser, true);
	}
	else
	{
		m_pAttendanceManager->GetLastError();
		UpdateUserList(NULL, 0,0, true);
		MessageBox(ConvertString("find attendance user failed"), ConvertString("Prompt"));
	}

	delete []stuOutFindUser.stuUserInfo;
	stuOutFindUser.stuUserInfo = NULL;
	delete []stuOutFindUser.pbyPhotoData;
	stuOutFindUser.pbyPhotoData = NULL;
}

//登出后刷新UI
void CAttendanceDlg::RefreshUI(void)
{
	UpdateUserList(NULL, 0, 0, true);
	EnableButtons(false);
}

BOOL CAttendanceDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (WM_KEYDOWN == pMsg->message)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			return true;
		}
	}
	return __super::PreTranslateMessage(pMsg);
}

void CAttendanceDlg::OnBnClickedButtonOperateFingerprintByuserid()
{
	// TODO: 在此添加控件通知处理程序代码
	int nSelectCount = m_listCtlUserInfo.GetSelectedCount();
	if (0 == nSelectCount)
	{
		MessageBox(ConvertString("Please select one user to operate fingerprint"), ConvertString("Prompt"));
		return;
	}

	int nIndex = m_listCtlUserInfo.GetSelectionMark();
	char szUserID[256]={0};
	m_listCtlUserInfo.GetItemText(nIndex, 1, szUserID, sizeof(szUserID));
	CFingerprintByUserIDDlg dlg(m_pAttendanceManager, szUserID);
	dlg.DoModal();
}

void CAttendanceDlg::OnBnClickedButtonOperateFingerprintByfpid()
{
	// TODO: 在此添加控件通知处理程序代码
	CFingerprintByFPID dlg(m_pAttendanceManager);
	dlg.DoModal();
}
