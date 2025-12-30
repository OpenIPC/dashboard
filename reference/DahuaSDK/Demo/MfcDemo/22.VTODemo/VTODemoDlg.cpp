// VTODemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "time.h"

#include "VTODemo.h"
#include "VTODemoDlg.h"
#include "AddCardDlg.h"
#include "CardManagerDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_ALARM_MESSAGE	(WM_USER + 112)

static const unsigned int nEventDisplayNum = 8;						
static const unsigned int nMaxEventToStoreInEventList = 1000;	

struct AlarmEvent
{
	LONG lCommand;
	char *pBuf;
	DWORD dwBufLen;

	AlarmEvent()
	{
		lCommand	= 0;
		pBuf		= NULL;
		dwBufLen	= NULL;
	}

	~AlarmEvent()
	{
		if (pBuf != NULL)
		{
			delete[] pBuf;
			pBuf = NULL;
		}
	}
};

static void CALLBACK AudioDataCallBack(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag, LDWORD dwUser)
{
	if(0 == dwUser)
	{
		return;
	}
	CVTODemoDlg* pThis = (CVTODemoDlg *)dwUser;
	pThis->DealAudioData(lTalkHandle, pDataBuf,dwBufSize,byAudioFlag);
}

static int CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char* pBuf, DWORD dwBufLen, char* pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if (0 == dwUser || dwBufLen <= 0)
	{
		return -1;
	}
	CVTODemoDlg* pThis = (CVTODemoDlg*)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == pThis)
	{
		return -1; 
	}

	AlarmEvent* pAlarmEvent = new AlarmEvent;
	if (NULL == pAlarmEvent)
	{
		return -1;
	}

	pAlarmEvent->pBuf = new char[dwBufLen + 1];
	if (NULL == pAlarmEvent->pBuf)
	{
		delete pAlarmEvent;
		pAlarmEvent = NULL;
		return -1;
	}

	memset(pAlarmEvent->pBuf, 0, dwBufLen + 1);
	memcpy(pAlarmEvent->pBuf, pBuf, dwBufLen);
	pAlarmEvent->lCommand	= lCommand;	
	pAlarmEvent->dwBufLen	= dwBufLen;

	PostMessage(hWnd, WM_ALARM_MESSAGE, (WPARAM)(pAlarmEvent), NULL);
	return 0;
}

// CVTODemoDlg 对话框


CVTODemoDlg::CVTODemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVTODemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_lLoginID = 0;
	m_lTalkID = 0;
	m_lPreviewID = 0;
	m_nChannelNum = 0;
	m_nChannelIndex = -1;

	InitSDK();
}

void CVTODemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST_ALARM, m_listCtrlAlarm);
}

BEGIN_MESSAGE_MAP(CVTODemoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_LOGIN, &CVTODemoDlg::OnBnClickedBtnLogin)
	ON_BN_CLICKED(IDC_BTN_LOGOUT, &CVTODemoDlg::OnBnClickedBtnLogout)
	ON_BN_CLICKED(IDC_BTN_STARTPREVIEW, &CVTODemoDlg::OnBnClickedBtnStartpreview)
	ON_BN_CLICKED(IDC_BTN_STOPPREVIEW, &CVTODemoDlg::OnBnClickedBtnStoppreview)
	ON_BN_CLICKED(IDC_BTN_ATTACH, &CVTODemoDlg::OnBnClickedBtnAttach)
	ON_BN_CLICKED(IDC_BTN_DETTACH, &CVTODemoDlg::OnBnClickedBtnDettach)
	ON_BN_CLICKED(IDC_BTN_STARTTALK, &CVTODemoDlg::OnBnClickedBtnStarttalk)
	ON_BN_CLICKED(IDC_BTN_STOPTALK, &CVTODemoDlg::OnBnClickedBtnStoptalk)
	ON_BN_CLICKED(IDC_BTN_OPENDOOR, &CVTODemoDlg::OnBnClickedBtnOpendoor)
	ON_BN_CLICKED(IDC_BTN_CARDMANAGER, &CVTODemoDlg::OnBnClickedBtnCardmanager)

	ON_MESSAGE(WM_ALARM_MESSAGE, OnMessCallback)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CVTODemoDlg 消息处理程序

BOOL CVTODemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	SetDlgItemText(IDC_IPADDRESS, ConvertString("172.23.29.61"));
	SetDlgItemText(IDC_EDIT_PORT, ConvertString("37777"));
	SetDlgItemText(IDC_EDIT_USERNAME, ConvertString("admin"));
	SetDlgItemText(IDC_EDIT_PASSWORD, ConvertString("admin123"));

	CRect rect;
	GetClientRect(&rect);
	unsigned int nWidth = rect.Width();

	m_listCtrlAlarm.SetExtendedStyle(m_listCtrlAlarm.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_listCtrlAlarm.InsertColumn(0, ConvertString("Time"), LVCFMT_LEFT, 200, -1);
	m_listCtrlAlarm.InsertColumn(1, ConvertString("Status"), LVCFMT_LEFT, 110, 0);
	m_listCtrlAlarm.InsertColumn(2, ConvertString("Method"), LVCFMT_LEFT, 110, 1);
	m_listCtrlAlarm.InsertColumn(3, ConvertString("CardNo"), LVCFMT_LEFT, 150, 2);
	m_listCtrlAlarm.InsertColumn(4, ConvertString("RoomNo"), LVCFMT_LEFT, 100, 3);

	ResetDlg();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVTODemoDlg::OnPaint()
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
HCURSOR CVTODemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CVTODemoDlg::InitSDK()
{
	BOOL ret = CLIENT_Init(NULL, (LDWORD)this);
	if(!ret)
	{
		MessageBox(ConvertString("Initialize SDK failed!"),ConvertString("Prompt"));
		return FALSE;
	}
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetDVRMessCallBack(MessCallBack, (LDWORD)this);

	m_lLoginID = 0;
	m_lTalkID = 0;
	m_lPreviewID = 0;

	return TRUE;
}
void CVTODemoDlg::OnBnClickedBtnLogin()
{
	// TODO: 在此添加控件通知处理程序代码

	CString strIp;
	CString strUser;
	CString strPwd;
	int nPort;
	
	nPort = GetDlgItemInt(IDC_EDIT_PORT);
	GetDlgItemText(IDC_IPADDRESS, strIp);
	GetDlgItemText(IDC_EDIT_USERNAME, strUser);
	GetDlgItemText(IDC_EDIT_PASSWORD, strPwd);

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
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if(lRet != 0)
	{
		m_lLoginID = lRet;
		m_nChannelNum = stOutparam.stuDeviceInfo.nChanNum;

		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STARTPREVIEW)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOPPREVIEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_ATTACH)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_DETTACH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_OPENDOOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_CARDMANAGER)->EnableWindow(TRUE);
	}
	else
	{
		if(1 == stOutparam.nError)			MessageBox(ConvertString("Invalid password!"), ConvertString("Prompt"));
		else if(2 == stOutparam.nError)	MessageBox(ConvertString("Invalid account!"),ConvertString("Prompt"));
		else if(3 == stOutparam.nError)	MessageBox(ConvertString("Timeout!"), ConvertString("Prompt"));
		else if(4 == stOutparam.nError)	MessageBox(ConvertString("The user has logged in!"),ConvertString("Prompt"));
		else if(5 == stOutparam.nError)	MessageBox(ConvertString("The user has been locked!"), ConvertString("Prompt"));
		else if(6 == stOutparam.nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("Prompt"));
		else if(7 == stOutparam.nError)	MessageBox(ConvertString("The system is busy!"),ConvertString( "Prompt"));
		else if(9 == stOutparam.nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("Prompt"));
		else	MessageBox(ConvertString("Login failed!"), ConvertString("Prompt"));
	}
}

void CVTODemoDlg::OnBnClickedBtnLogout()
{
	// TODO: 在此添加控件通知处理程序代码
	ResetDlg();
	Invalidate(TRUE);
	CloseAttchHandle();
	ClosePreviewHandle();
	CloseTalkHandle();
	CloseLoginHandle();
}

void CVTODemoDlg::OnBnClickedBtnStartpreview()
{
	// TODO: 在此添加控件通知处理程序代码
	HWND hWnd = GetDlgItem(IDC_STATIC_PREVIEW)->GetSafeHwnd();
	if(NULL == hWnd)
	{
		return;
	}

	m_lPreviewID = StartPreview(hWnd);
	if(0 == m_lPreviewID)
	{
		MessageBox(ConvertString("Start RealPlay failed!"), ConvertString("Prompt"));		
		return;	
	}

	GetDlgItem(IDC_BTN_STARTPREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOPPREVIEW)->EnableWindow(TRUE);
}

void CVTODemoDlg::OnBnClickedBtnStoppreview()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet = StopPreview();
	if(!bRet)
	{
		 MessageBox(ConvertString("Stop RealPlay failed!"), ConvertString("Prompt"));	
		 return;
	}

	GetDlgItem(IDC_BTN_STARTPREVIEW)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOPPREVIEW)->EnableWindow(FALSE);
	Invalidate(TRUE);
}

void CVTODemoDlg::OnBnClickedBtnAttach()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet = StartAttach();
	if(!bRet)
	{
		MessageBox(ConvertString("Start Listen failed!"), ConvertString("Prompt"));	
		return;
	}

	GetDlgItem(IDC_BTN_ATTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETTACH)->EnableWindow(TRUE);
}

void CVTODemoDlg::OnBnClickedBtnDettach()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet = StopAttach();
	if(!bRet)
	{
		MessageBox(ConvertString("Stop Listen failed!"), ConvertString("Prompt"));	
		return;
	}

	GetDlgItem(IDC_BTN_ATTACH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_DETTACH)->EnableWindow(FALSE);
}

void CVTODemoDlg::OnBnClickedBtnStarttalk()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet = StartTalk();

	if(!bRet)
	{
		MessageBox(ConvertString("Start talk failed"), ConvertString("Prompt"));
		return;
	}

	GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(TRUE);
}

void CVTODemoDlg::OnBnClickedBtnStoptalk()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet = StopTalk();

	if(!bRet)
	{
		MessageBox(ConvertString("Stop talk failed"), ConvertString("Prompt"));
		return;
	}

	GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(FALSE);
}

void CVTODemoDlg::OnBnClickedBtnOpendoor()
{
	// TODO: 在此添加控件通知处理程序代码
	NET_CTRL_ACCESS_OPEN stuParam = {sizeof(stuParam)};
	stuParam.nChannelID = 0;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_ACCESS_OPEN, &stuParam, 3000);
	if(bRet)
	{
		MessageBox(ConvertString("Open door succeed!"), ConvertString("Prompt"));	
	}
	else
	{
		MessageBox(ConvertString("Open door failed!"), ConvertString("Prompt"));
	}
}

void CVTODemoDlg::OnBnClickedBtnCardmanager()
{
	// TODO: 在此添加控件通知处理程序代码
	CCardManagerDlg dlg(m_lLoginID);
	dlg.DoModal();
}

void CVTODemoDlg::ResetDlg()
{
	GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_LOGOUT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STARTPREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOPPREVIEW)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STARTTALK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOPTALK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_ATTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_DETTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_OPENDOOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_CARDMANAGER)->EnableWindow(FALSE);

	m_listCtrlAlarm.SetExtendedStyle(m_listCtrlAlarm.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
}

BOOL CVTODemoDlg::StartAttach()
{
	if(0 == m_lLoginID)
	{
		return FALSE;
	}

	return CLIENT_StartListenEx(m_lLoginID);
}
BOOL CVTODemoDlg::StopAttach()
{
	m_listCtrlAlarm.DeleteAllItems();
	m_listAccessControlInfo.RemoveAll();
	if(0 == m_lLoginID)
	{
		return FALSE;
	}
	
	return CLIENT_StopListen(m_lLoginID);
}
void CVTODemoDlg::CloseAttchHandle()
{
	if(0 != m_lLoginID)
	{
		StopAttach();
	}
}

LLONG CVTODemoDlg::StartPreview(const HWND& hWnd)
{
	if(0 == m_lLoginID)
	{
		return 0;
	}
	return CLIENT_RealPlayEx(m_lLoginID, 0, hWnd, DH_RType_Realplay);
}
BOOL CVTODemoDlg::StopPreview()
{
	BOOL bRet = CLIENT_StopRealPlay(m_lPreviewID);
	if(bRet)
	{
		m_lPreviewID = 0;
	}
	return bRet;
}
void CVTODemoDlg::ClosePreviewHandle()
{
	if(0 != m_lPreviewID)
	{
		/*CLIENT_StopRealPlay(m_lPreviewID);
		m_lPreviewID = 0;*/
		StopPreview();
	}
}

BOOL CVTODemoDlg::StartTalk()
{
	DHDEV_TALKDECODE_INFO talkInfo;
	memset(&talkInfo, 0, sizeof(talkInfo));
	talkInfo.encodeType = DH_TALK_PCM;
	talkInfo.dwSampleRate = 8000;
	talkInfo.nAudioBit = 16;
	talkInfo.nPacketPeriod = 25;

	CString strMessage;

	BOOL bRet = CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_ENCODE_TYPE, &talkInfo);
	if(!bRet)
	{
		MessageBox(ConvertString("Set encode type failed!"), ConvertString("Prompt"));
		return FALSE;
	}

	bRet = CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_CLIENT_MODE, NULL);
	if(!bRet)
	{
		MessageBox(ConvertString("Set client type failed!"), ConvertString("Prompt"));
		return FALSE;
	}

	NET_SPEAK_PARAM speakParam = {sizeof(speakParam)};
	speakParam.nMode = 0;
	bRet = CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_SPEAK_PARAM, &speakParam);
	if(!bRet)
	{
		MessageBox(ConvertString("Set speak param failed!"),ConvertString("Prompt"));
		return FALSE;
	}

	LLONG lTalkHandle = CLIENT_StartTalkEx(m_lLoginID, AudioDataCallBack, (LDWORD)this);
	if(0 == lTalkHandle)
	{
		return FALSE;
	}

	m_lTalkID = lTalkHandle;

	bRet = CLIENT_RecordStartEx(m_lLoginID);
	if(!bRet)
	{
		CLIENT_StopTalkEx(m_lTalkID);
		m_lTalkID = 0;
		MessageBox(ConvertString("Start record failed!"),ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}
BOOL CVTODemoDlg::StopTalk()
{
	BOOL bRet = FALSE;
	
	bRet = CLIENT_RecordStopEx(m_lLoginID);
	if(!bRet)
	{
		MessageBox(ConvertString("Stop record failed!"),ConvertString("Prompt"));
	}
	else
	{
		bRet = CLIENT_StopTalkEx(m_lTalkID);
	}
	m_lTalkID = 0;

	return bRet;
}
void CVTODemoDlg::DealAudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag)
{
	if (m_lTalkID != lTalkHandle)
	{
		return ;
	}

	if(NET_TALK_DATA_LOCAL_AUDIO == byAudioFlag)
	{
		//It means it is the audio data from the local record library		
		long lSendLen = CLIENT_TalkSendData(lTalkHandle, pDataBuf, dwBufSize);
		if(lSendLen != (long)dwBufSize)
		{			
			//Error occurred when sending the user audio data to the device			
		}				
	}
	else if(NET_TALK_DATA_RECV_AUDIO == byAudioFlag || NET_TALK_DATA_RECV_VIDEO == byAudioFlag)
	{
		//It means it has received the audio data from the device.
		CLIENT_AudioDecEx(lTalkHandle, pDataBuf, dwBufSize);
	}
	else if (NET_TALK_DATA_RESPOND == byAudioFlag)
	{
		OutputDebugString(ConvertString("device responded..."));
	}
}
void CVTODemoDlg::CloseTalkHandle()
{
	if(0 != m_lTalkID)
	{
		StopTalk();
	}

}

void CVTODemoDlg::CloseLoginHandle()
{
	if(0 != m_lLoginID)
	{
		CLIENT_Logout(m_lLoginID);
		m_lLoginID = 0;
	}
}

LRESULT CVTODemoDlg::OnMessCallback(WPARAM wParam, LPARAM lParam)
{
	AlarmEvent* pEvent = (AlarmEvent*)wParam;
	if (NULL == pEvent || NULL == pEvent->pBuf)
	{
		return -1;
	}

	DealWithAlarmEvent(pEvent->lCommand, pEvent->pBuf, pEvent->dwBufLen);	

	delete pEvent;
	pEvent = NULL;

	return 0;
}
BOOL CVTODemoDlg::DealWithAlarmEvent(LONG lCommand, char *pBuf, DWORD dwBufLen)
{	
	ALARM_ACCESS_CTL_EVENT_INFO accessCtrlInfo = {sizeof(ALARM_ACCESS_CTL_EVENT_INFO)};
	switch(lCommand)
	{		
	case DH_ALARM_ACCESS_CTL_EVENT:
		{
			memcpy(&accessCtrlInfo, pBuf, min(sizeof(ALARM_ACCESS_CTL_EVENT_INFO), dwBufLen));			
			//GetZoneTime(accessCtrlInfo.stuTime);
			BOOL bret = m_csAccessControl.Lock();
			if (bret)
			{
				m_listAccessControlInfo.AddHead(accessCtrlInfo);
			}
			m_csAccessControl.Unlock();

			UpdateAccessControlListCtrl();
		}
		break;		
	default:
		break;
	}

	return TRUE;
}
void CVTODemoDlg::UpdateAccessControlListCtrl()
{
	BOOL bret = m_csAccessControl.Lock();
	if (bret)
	{
		int nCount = m_listAccessControlInfo.GetCount();
		if (nCount > nMaxEventToStoreInEventList)
		{
			ALARM_ACCESS_CTL_EVENT_INFO alarmInfo = m_listAccessControlInfo.GetHead();
			m_listAccessControlInfo.RemoveAll();
			m_listAccessControlInfo.AddHead(alarmInfo);		
		}

		//m_nAllAccessControlEventCount += 1;
		//CString strAccessControlInfo;
		//strAccessControlInfo.Format("Access Control Event Num Since Login:%d", m_nAllAccessControlEventCount);
		//GetDlgItem(IDC_GROUP_ACCESS_CONTROL)->SetWindowText(strAccessControlInfo);

		m_listCtrlAlarm.DeleteAllItems();	

		nCount = m_listAccessControlInfo.GetCount();
		int nNum = min(nEventDisplayNum, nCount);
		POSITION pos = m_listAccessControlInfo.GetHeadPosition();
		for (int i = 0; i < nNum; i++)
		{
			ALARM_ACCESS_CTL_EVENT_INFO alarmInfo = {sizeof(ALARM_ACCESS_CTL_EVENT_INFO)};
			alarmInfo = m_listAccessControlInfo.GetAt(pos);

			m_listCtrlAlarm.InsertItem(i, NULL);

			CString strTime;
			strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),
				alarmInfo.stuTime.dwYear,
				alarmInfo.stuTime.dwMonth,
				alarmInfo.stuTime.dwDay,
				alarmInfo.stuTime.dwHour,
				alarmInfo.stuTime.dwMinute,
				alarmInfo.stuTime.dwSecond);
			m_listCtrlAlarm.SetItemText(i, 0, strTime);

			// status
			CString strStatus = alarmInfo.bStatus ? ConvertString("Success") : ConvertString("Fail");
			m_listCtrlAlarm.SetItemText(i, 1, strStatus);

			// method
			CString strMethod = EmOpenMethodToCString(alarmInfo.emOpenMethod);
			m_listCtrlAlarm.SetItemText(i, 2, strMethod);

			// cardNo	
			m_listCtrlAlarm.SetItemText(i, 3, (CString)alarmInfo.szCardNo);

			//RoomNo
			m_listCtrlAlarm.SetItemText(i, 4, (CString)alarmInfo.szUserID);		

			m_listAccessControlInfo.GetNext(pos);
		}
	}
	m_csAccessControl.Unlock();
}
CString CVTODemoDlg::EmOpenMethodToCString(NET_ACCESS_DOOROPEN_METHOD  emOpenMethod)
{
	CString strOpenMethod = _T("");
	switch (emOpenMethod)
	{
	case NET_ACCESS_DOOROPEN_METHOD_UNKNOWN:
		strOpenMethod = ConvertString("UnKnown");
		break;
	case NET_ACCESS_DOOROPEN_METHOD_CARD:
		strOpenMethod = ConvertString("Card");
		break;
	case NET_ACCESS_DOOROPEN_METHOD_REMOTE:
		strOpenMethod = ConvertString("Remote");
		break;
	case NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT:
		strOpenMethod = ConvertString("FingerPrint");
		break;
	case NET_ACCESS_DOOROPEN_METHOD_FACE_RECOGNITION:
		strOpenMethod = ConvertString("Targ Recognition");
		break;	
	default:
		strOpenMethod = ConvertString("UnKnown");
		break;
	}
	return strOpenMethod;
}

void CVTODemoDlg::GetZoneTime(NET_TIME& netTime)
{
	time_t t_second = 0;
	struct tm tmTime;
	tmTime.tm_year = netTime.dwYear - 1900;
	tmTime.tm_mon = netTime.dwMonth;
	tmTime.tm_mday = netTime.dwDay;
	tmTime.tm_hour = netTime.dwHour;
	tmTime.tm_min = netTime.dwMinute;
	tmTime.tm_sec = netTime.dwSecond;
	tmTime.tm_isdst = 0;

	t_second = mktime(&tmTime);

	DHDEV_NTP_CFG stuCFGParam;
	memset(&stuCFGParam, 0, sizeof(stuCFGParam));
	DWORD nError = 0;
	bool bRet = CLIENT_GetDevConfig(m_lLoginID, DH_DEV_NTP_CFG, -1, (void*)&stuCFGParam, sizeof(stuCFGParam), &nError, 3000);

	if(bRet)
	{
		switch(stuCFGParam.nTimeZone)
		{
		case DH_TIME_ZONE_0:
			t_second = t_second + 0 * 3600;
			break;
		case DH_TIME_ZONE_1:
			t_second = t_second + 1 * 3600;
			break;
		case DH_TIME_ZONE_2:
			t_second = t_second + 2 * 3600;
			break;
		case DH_TIME_ZONE_3:
			t_second = t_second + 3 * 3600;
			break;
		case DH_TIME_ZONE_4:
			t_second = t_second + 3 * 3600 + 1800;
			break;
		case DH_TIME_ZONE_5:
			t_second = t_second + 4 * 3600;
			break;
		case DH_TIME_ZONE_6:
			t_second = t_second + 4 * 3600 + 1800;
			break;
		case DH_TIME_ZONE_7:
			t_second = t_second + 5 * 3600;
			break;
		case DH_TIME_ZONE_8:
			t_second = t_second + 5 * 3600 + 1800;
			break;
		case DH_TIME_ZONE_9:
			t_second = t_second + 5 * 3600 + 1800 + 900;
			break;
		case DH_TIME_ZONE_10:
			t_second = t_second + 6 * 3600;
			break;
		case DH_TIME_ZONE_11:
			t_second = t_second + 6 * 3600 + 1800;
			break;
		case DH_TIME_ZONE_12:
			t_second = t_second + 7 * 3600;
			break;
		case DH_TIME_ZONE_13:
			t_second = t_second + 8 * 3600;
			break;
		case DH_TIME_ZONE_14:
			t_second = t_second + 9 * 3600;
			break;
		case DH_TIME_ZONE_15:
			t_second = t_second + 9 * 3600 + 1800;
			break;
		case DH_TIME_ZONE_16:
			t_second = t_second + 10 * 3600;
			break;
		case DH_TIME_ZONE_17:
			t_second = t_second + 11 * 3600;
			break;
		case DH_TIME_ZONE_18:
			t_second = t_second + 12 * 3600;
			break;
		case DH_TIME_ZONE_19:
			t_second = t_second + 13 * 3600;
			break;
		case DH_TIME_ZONE_20:
			t_second = t_second - 1 * 3600;
			break;
		case DH_TIME_ZONE_21:
			t_second = t_second - 2 * 3600;
			break;
		case DH_TIME_ZONE_22:
			t_second = t_second - 3 * 3600;
			break;
		case DH_TIME_ZONE_23:
			t_second = t_second - 3 * 3600 - 1800;
			break;
		case DH_TIME_ZONE_24:
			t_second = t_second - 4 * 3600;
			break;
		case DH_TIME_ZONE_25:
			t_second = t_second - 5 * 3600;
			break;
		case DH_TIME_ZONE_26:
			t_second = t_second - 6 * 3600;
			break;
		case DH_TIME_ZONE_27:
			t_second = t_second - 7 * 3600;
			break;
		case DH_TIME_ZONE_28:
			t_second = t_second - 8 * 3600;
			break;
		case DH_TIME_ZONE_29:
			t_second = t_second - 9 * 3600;
			break;
		case DH_TIME_ZONE_30:
			t_second = t_second - 10 * 3600;
			break;
		case DH_TIME_ZONE_31:
			t_second = t_second - 11 * 3600;
			break;
		case DH_TIME_ZONE_32:
			t_second = t_second - 12 * 3600;
			break;
		default:
			break;
		}
		struct tm* pTm = localtime(&t_second);
		netTime.dwYear = pTm->tm_year + 1900;
		netTime.dwMonth = pTm->tm_mon;
		netTime.dwDay = pTm->tm_mday;
		netTime.dwHour = pTm->tm_hour;
		netTime.dwMinute = pTm->tm_min;
		netTime.dwSecond = pTm->tm_sec;

		if (0 == netTime.dwMonth)
		{
			netTime.dwYear--;
			netTime.dwMonth = 12;
		}
	}
}
void CVTODemoDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	CloseAttchHandle();
	ClosePreviewHandle();
	CloseTalkHandle();
	CloseLoginHandle();

	CLIENT_Cleanup();
}
