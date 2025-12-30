   // NetSDKDemoDlg.cpp : implementation file

#include "StdAfx.h"
#include "NetSDKDemo.h"
#include "NetSDKDemoDlg.h"
#include "splitinfodlg.h"
#include "adddevicedlg.h"
#include "searchrecord.h"
#include "systemconfig.h"
#include "extptzctrl.h"
#include "ptzmenu.h"
#include "transcom.h"
#include "recordctrldlg.h"
#include "deviceworkstate.h"
#include "alarmctrldlg.h"
#include "cyclemonitor.h"
#include "direct.h"
#include "playbackbytime.h"
#include "downloadbytime.h"
#include "NetUpgrade.h"
#include "DDNS_QueryIP.h"
#include "DiskControl.h"
#include "usermanage.h"
#include "configmaindlg.h"
#include "PreviewParmsDlg.h"
#include "automaintenance.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// #pragma data_seg("sharesec")
// __declspec (allocate("sharesec")) HWND g_share_hWnd = NULL;
// #pragma comment(linker,"/SECTION:sharesec,RWS")

#define WM_DEVICE_DISCONNECT	(WM_USER+200)
#define WM_DEVICE_RECONNECT		(WM_USER+201)

#define ALARMLOG	0x1099
#define UPDATATREE	0x2000

BOOL	g_bUpdataTree = FALSE;
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About


inline void dbg_print_ex(int level, const char *msg, ...)
{
	char buf[256];
	
	va_list ap;
	va_start(ap, msg); // use variable arg list
	vsprintf(buf, msg, ap);
	va_end( ap );
}

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
	virtual BOOL OnInitDialog();
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetSDKDemoDlg dialog

CNetSDKDemoDlg::CNetSDKDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetSDKDemoDlg::IDD, pParent)
{
	m_connectwaittime = 3000;
	m_myBrush.CreateSolidBrush(RGB(255,255,255));
	//{{AFX_DATA_INIT(CNetSDKDemoDlg)
	m_play_frame = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = 0;//AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_curScreen = 0;
	m_curSoundSplit = 0;
	m_testvalue = 0;
	m_bPTZCtrl = FALSE;
	m_bRecord = FALSE;
	memset(&m_talkhandle, 0, sizeof(m_talkhandle));
	m_uRecordCount = 0;
	m_almLogFile_Comm = NULL;
	m_almLogFile_Shelter = NULL;
	m_almLogFile_dFull = NULL;
	m_almLogFile_dError = NULL;
	m_almLogFile_SoundDec = NULL;
	memset(&m_lastAlarm, 0, sizeof(m_lastAlarm));	
	m_lListenDevice = 0;
	m_lListenChannel = 0;

	memset(&m_mylsdata, 0, sizeof(m_mylsdata));
}

void CNetSDKDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetSDKDemoDlg)
	DDX_Control(pDX, IDC_TREE_DEVICELIST, m_devicelist);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNetSDKDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CNetSDKDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DEVICELIST, OnDblclkTreeDevicelist)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////初始化回调相关的函数////////////////////////////////////////////////
//设备断开时回调函数，可以用来处理断开后设备列表的更新及设备的删除操作
void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if (0 == dwUser)
	{
		return;
	}

	CNetSDKDemoDlg *pThis = (CNetSDKDemoDlg*)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_DISCONNECT, WPARAM(lLoginID), NULL);	
}

//设备断开处理
LRESULT CNetSDKDemoDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	LLONG lLoginID = (LLONG)wParam;
	CDevMgr::GetDevMgr().ModifyNode(lLoginID, FALSE);
	g_bUpdataTree = TRUE;
	return 0;	
}

void CALLBACK HaveReconnFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if (0 == dwUser)
	{
		return;
	}

	CNetSDKDemoDlg *pThis = (CNetSDKDemoDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, WPARAM(lLoginID), NULL);	
}

LRESULT CNetSDKDemoDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	LLONG lLoginID = (LLONG)wParam;
	CDevMgr::GetDevMgr().ModifyNode(lLoginID, TRUE);

	UpdateDeviceList();

	return 0;
}

//消息回调处理函数,是对整个sdk应用的回调
BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, 
						   char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(!dwUser) return FALSE;

	CNetSDKDemoDlg *dlg = (CNetSDKDemoDlg *)dwUser;
	return dlg ->ReceiveMessage(lLoginID, lCommand, pchDVRIP, nDVRPort,pBuf, dwBufLen);
}

//接收到设备消息的处理,目前只定义了报警消息回调
BOOL CNetSDKDemoDlg::ReceiveMessage(LLONG lLoginID, LONG lCommand, char *pchDVRIP, LONG nDVRPort, char *pBuf, DWORD dwBufLen)
{
	int nRet = CDevMgr::GetDevMgr().SetAlarmInfo(lLoginID, lCommand, pchDVRIP, nDVRPort, 
		pBuf, dwBufLen);
	return nRet<0?FALSE:TRUE;
}

//自定义画板回调,可以用来显示通道信息
void CALLBACK ChannelAutoDraw(LLONG lLoginID, LLONG lPlayHandle, HDC hDC, LDWORD dwUser)
{
	if(!dwUser) return;

	CNetSDKDemoDlg *dlg = (CNetSDKDemoDlg *)dwUser;
	dlg->AddDisplayInfo(lLoginID, lPlayHandle, hDC);
}

//叠加字符或图片
void CNetSDKDemoDlg::AddDisplayInfo(LLONG lLoginID, LLONG lPlayHandle, HDC hDC)
{
	DeviceNode *pDev;
	CString str;

	//取得窗口号
	int i = GetHandleSplit(lPlayHandle);
	if(i < 0)
	{
		return;
	}

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(i, &siNode);
	if (!ret)
	{
		return;
	}

	switch(siNode.Type) 
	{
	case SPLIT_TYPE_MONITOR:
		{
			SplitMonitorParam *mParam = (SplitMonitorParam *)siNode.Param;
			if (mParam)
			{
				pDev = mParam->pDevice;
				str.Format(" %s[%s] %02d", pDev->Name,pDev->IP, mParam->iChannel + 1);
				str = ConvertString(MSG_DEMODLG_MONITOR) + str;

				if(pDev->State.cState.motiondection[mParam->iChannel])
				{
					CString almstr;
					almstr.Format(ConvertString("! ALARM !"));
					SetTextColor(hDC, RGB(255,0,0));
					CRect rect;						
					CWnd* pWnd = m_screenPannel.GetPage(i);
					if (pWnd)
					{
						pWnd->GetClientRect(&rect);			

						if (::IsWindow(pWnd->GetSafeHwnd()))
						{
							TextOut(hDC, rect.right / 3, rect.bottom / 2, almstr.GetBuffer(0), almstr.GetLength());
						}	
					}									
				}			
			}
		}
		break;
	case SPLIT_TYPE_MULTIPLAY:
		pDev = (DeviceNode *)siNode.Param;
		str.Format(" %s[%s]", pDev->Name,  pDev->IP);
		str = ConvertString(MSG_DEMODLG_PREVIEW) + str;
		break;

	case SPLIT_TYPE_NETPLAY:
		{
			SplitNetPlayParam *nParam = (SplitNetPlayParam *)siNode.Param; 
			pDev = nParam->pFileInfo->pDevice;
			str.Format(" %s[%s]%02d", pDev->Name, pDev->IP, 
				nParam->pFileInfo->fileinfo.ch + 1);
			str = ConvertString(MSG_DEMODLG_PLAYBACKCHL) + str;
		}
		break;
	case SPLIT_TYPE_PBBYTIME:
		{
			break;
		}
	case SPLIT_TYPE_CYCLEMONITOR:
		{
			SplitCycleParam *cParam = (SplitCycleParam *)siNode.Param;
			CycleChannelInfo *cci = 0;
			POSITION pos = cParam->pChannelList->GetHeadPosition();
			for (int counter = 0; counter <= cParam->iCurPosition; counter++)
			{
				cci = (CycleChannelInfo *)cParam->pChannelList->GetNext(pos);
				if (cci)
				{
					pDev = (DeviceNode *)cci->dwDeviceID;
					str.Format(" %s[%s](%02d)", pDev->Name, pDev->IP, cci->iChannel + 1);
				}			
				str = ConvertString(MSG_DEMODLG_CYCLEMONITOR) + str;
			}			
			break;
		}
	default :
		return;
	}

	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255,255,0));
	TextOut(hDC, 0, 0, str.GetBuffer(0), str.GetLength());
	
}
// CNetSDKDemoDlg message handlers
BOOL CNetSDKDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	Getversion();
	char tmpDir[1000];
	_getcwd(tmpDir, 1000);
	g_strWorkDir.Format("%s", tmpDir);

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
	
	//初始化网络sdk,所有调用的开始
	BOOL ret = CLIENT_Init(DisConnectFunc, (LDWORD)this);
	if (!ret)
	{
		LastError();
	}
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	//设置断线重连
	CLIENT_SetAutoReconnect(HaveReconnFunc, (LDWORD)this);

	//设置信息回调函数,默认接收所有设备信息
	CLIENT_SetDVRMessCallBack(MessCallBack, (LDWORD)this);

	//初始化各子窗口

	m_selectPannel.Create(IDD_PANNEL_SELECT, this);	
	m_screenPannel.Create(
		NULL,
		NULL,
		WS_CHILD|WS_VISIBLE, 
		CRect(0,0,0,0), 
		this, 
		1981);		
	m_normalBtnPannel.Create(IDD_PANNEL_NORMAL_BTN, this);
	m_advanceBtnPannel.Create(IDD_PANNEL_ADVANCE_BTN, this);
	m_ptzPannel.Create(IDD_PANNEL_PTZ, this);
	m_runtimeMsgPannel.Create(IDD_PANNEL_RUNTIME_MSG, this);

	UpdatePannelPosition();

	m_selectPannel.ShowWindow(SW_SHOW);
	m_screenPannel.ShowWindow(SW_SHOW);	
	m_devicelist.ShowWindow(SW_SHOW);
	m_advanceBtnPannel.ShowWindow(SW_HIDE);
	m_ptzPannel.ShowWindow(SW_HIDE);
	m_runtimeMsgPannel.ShowWindow(SW_SHOW);
	
	//初始化画面分割模式选择项
	m_normalBtnPannel.InitSplit(CUR_SPLIT);	
	m_normalBtnPannel.ShowWindow(SW_SHOW);

	m_curScreen = 0;
	m_screenPannel.SetShowPlayWin(CUR_SPLIT, m_curScreen);

	//设置连接等待时间
	CLIENT_SetConnectTime(m_connectwaittime, 3);
	LastError();  
	
	CLIENT_RigisterDrawFun(ChannelAutoDraw, (LDWORD)this);
	LastError();  

	//初始化关闭声音
	m_curSoundSplit = -1;
	
	//创建状态页面和系统配置页面
	m_ClientStateDlg.Create(IDD_CLIENT_STATE);
	m_ClientStateDlg.CenterWindow();
	m_ClientStateDlg.m_isNoMoreShow = TRUE;
	m_ClientStateDlg.UpdateData(false);

	//初始化云台控制状态
	m_bPTZCtrl = FALSE;

	//刷新界面信息
	UpdateScreen(CUR_SPLIT+1);
	UpdateCurScreenInfo();

	/*Begin: Add by yehao(10857) For Task.NO.11071 2006-12-23*/
	m_broadcastDevList.clear();
	m_bRecord = FALSE;
	memset((void *)&m_talkhandle, 0, sizeof(TalkHandleInfo));
	m_uRecordCount = 0;

	//download test
	m_dbByTime.Create(IDD_DOWNLOADBYTIME, this);

	//specified alarm test
	ZeroMemory(&m_lastAlarm, sizeof(DEV_STATE));
	CString strAlmLogPath = g_strWorkDir + "\\AlarmLog_comm.txt";
	m_almLogFile_Comm = fopen(strAlmLogPath, "w+");
	strAlmLogPath = g_strWorkDir + "\\AlarmLog_shelter.txt";
	m_almLogFile_Shelter = fopen(strAlmLogPath, "w+");
	strAlmLogPath = g_strWorkDir + "\\AlarmLog_diskFull.txt";
	m_almLogFile_dFull = fopen(strAlmLogPath, "w+");
	strAlmLogPath = g_strWorkDir + "\\AlarmLog_diskError.txt";
	m_almLogFile_dError = fopen(strAlmLogPath, "w+");
	strAlmLogPath = g_strWorkDir + "\\AlarmLog_soundAlarm.txt";
	m_almLogFile_SoundDec = fopen(strAlmLogPath, "w+");
	if (m_almLogFile_Comm && m_almLogFile_Shelter && m_almLogFile_dFull
		&& m_almLogFile_dError && m_almLogFile_SoundDec)
	{
		SetTimer(ALARMLOG, 1000, NULL);
	}

	SetTimer(UPDATATREE, 1000, NULL);

	//listen device test
	m_lListenDevice = 0;
	m_lListenChannel = 0;
	memset(&m_mylsdata, 0 , sizeof(m_mylsdata));

	return TRUE;
}

void CNetSDKDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CNetSDKDemoDlg::OnPaint() 
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
HCURSOR CNetSDKDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

////////////////////////////////内部使用小接口函数///////////////////////////////////////////////////
//根据登录ID获取设备信息指针
void *CNetSDKDemoDlg::FindDeviceInfo(LLONG lLoginID, char *sDVRIP,LONG nDVRPort)
{	
	return NULL;	
}

//获取当前设备列表中选择的设备信息指针
void *CNetSDKDemoDlg::GetCurDeviceInfo()
{
	HTREEITEM node;
	DWORD nData;

	node = m_devicelist.GetSelectedItem();

	if(!node)
	{
		MessageBox(ConvertString(MSG_DEMODLG_CHECKSEL));
		return NULL;
	}
	
	nData = m_devicelist.GetItemData(node);
	if(nData < 2048/*256*/)    //通道选项，取得父项
	{
		node = m_devicelist.GetParentItem(node);
	}

	return (void *)m_devicelist.GetItemData(node);  //父项记录的数据为设备的信息指针值
}

//根据句柄获取播放窗口序号,其中句柄可以是预览通道Id,播放Id,预览id等
int CNetSDKDemoDlg::GetHandleSplit(LLONG lPlayHandle)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	for(int i = 0; i < DH_MAX_CHANNUM; i++)
	{
		if (!GetSplitInfo_Main(i, &siNode))
		{
			return -1;
		}

		if(siNode.Type != SPLIT_TYPE_NULL)
		{
			if(siNode.iHandle == (DWORD)lPlayHandle)
			{
				return i;
			}
		}
	}
	return -1;
} 

//获取当前画面分割模式的指定画面的起始画面序号
int  CNetSDKDemoDlg::GetCurSplitStart(int nScreen, int nSplit)
{
	return -1;
}

//检测当前画面状态并关闭当前状态
BOOL CNetSDKDemoDlg::CheckCurSplitAndClose()
{	
	ProcessCloseScreen(m_curScreen);
	UpdateCurScreenInfo();
	return TRUE;
}

//检测当前选择通道是否在某个轮循列表中
BOOL CNetSDKDemoDlg::IsCycling(DWORD deviceID, int channelNo)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));

	for (int i = 0; i < DH_MAX_CHANNUM; i++)
	{
		BOOL ret = GetSplitInfo_Main(i, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while getting split information"));
			return TRUE;
		}
		
		if (siNode.Type == SPLIT_TYPE_CYCLEMONITOR) 
		{
			POSITION pos = ((SplitCycleParam *)siNode.Param)->pChannelList->GetHeadPosition();
			for (int j = 0; j < ((SplitCycleParam *)siNode.Param)->pChannelList->GetCount(); j++)
			{
				CycleChannelInfo *tempnode;
				tempnode = (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetNext(pos);
				if ((tempnode->dwDeviceID == deviceID) && (tempnode->iChannel == channelNo)) 
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void CNetSDKDemoDlg::DeleteCycleParam(int nScreen)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(nScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	int count;
	count = ((SplitCycleParam *)siNode.Param)->pChannelList->GetCount();
	for (int i = 0; i < count; i++)
	{
		delete (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetTail();
		((SplitCycleParam *)siNode.Param)->pChannelList->RemoveTail();
	}
	delete ((SplitCycleParam *)siNode.Param)->pChannelList;
	delete (SplitCycleParam *)siNode.Param;
	siNode.Type = SPLIT_TYPE_NULL;
	siNode.Param = NULL;
	ret = SetSplitInfo_Main(nScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while setting split info"));
	}
	
	CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(nScreen);
	if (plWnd)
	{
		plWnd->PostMessage(VIDEO_REPAINT);
	}
}

//关闭画面的显示状态
BOOL CNetSDKDemoDlg::ProcessCloseScreen(int scrNo, BOOL bDis) 
{
	BOOL ret = TRUE;
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	ret = GetSplitInfo_Main(scrNo, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	switch(siNode.Type) 
	{
	case SPLIT_TYPE_MONITOR:
		{
			ret = CLIENT_StopRealPlayEx(siNode.iHandle);
			if (!ret) 
			{
				LastError();
				if (!bDis) 
				{
					MessageBox(ConvertString(MSG_CYCLE_STOPMONITORERROR));
				}
			}
			delete (SplitMonitorParam *)siNode.Param;
			siNode.Param = NULL;
			siNode.Type = SPLIT_TYPE_NULL;
			ret = SetSplitInfo_Main(scrNo, &siNode);
			if (!ret)
			{
				MessageBox(ConvertString("error while getting split info"));
			}
			CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(scrNo);
			if (plWnd)
			{
				plWnd->PostMessage(VIDEO_REPAINT);
			}
		break;
		}
		
	case SPLIT_TYPE_MULTIPLAY:
		{	
		ret = CLIENT_StopRealPlayEx(siNode.iHandle);
		if (!ret)
		{
			LastError();
			if (!bDis) 
			{
				MessageBox(ConvertString(MSG_CYCLE_STOPMULTIPLAYERROR));
			}
		}
		siNode.Param = NULL;
		siNode.Type = SPLIT_TYPE_NULL;
		ret = SetSplitInfo_Main(scrNo, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while getting split info"));
		}
		m_advanceBtnPannel.EnableTalk(TRUE);
		CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(scrNo);
		if (plWnd)
		{
			plWnd->PostMessage(VIDEO_REPAINT);
		}
		break;
		}
		
	case SPLIT_TYPE_NETPLAY:		
	case SPLIT_TYPE_PBBYTIME:
		{
			int nRet = PlayStop(scrNo, bDis);
			if (nRet < 0)
			{
				ret = FALSE;
			}
			else
			{
				ret = TRUE;
			}
			break;
		}
		
	case SPLIT_TYPE_CYCLEMONITOR:
		{
		//仅用于“关闭”指令，删除与断开设备有另外的代码。
		KillTimer(scrNo);
		ret = CLIENT_StopRealPlay(siNode.iHandle);
		if (!ret) 
		{
			LastError();
			MessageBox(ConvertString(MSG_CYCLE_STOPCYCLEERROR));
		}
		if (siNode.Param) 
		{
			DeleteCycleParam(scrNo);
		}
		break;
		}
	default:
		break;
	}
	return ret;
}


int UpdateDeviceListCallback(const DeviceNode& node, LDWORD dwUser)
{
	CNetSDKDemoDlg* pThis = (CNetSDKDemoDlg*)dwUser;
	if (!pThis)
	{
		return 1;
	}

	return pThis->UpdateDeviceListCallback_Imp(node);
}

int CNetSDKDemoDlg::UpdateDeviceListCallback_Imp(const DeviceNode& node)
{
	CString strDev, strCh;	
	HTREEITEM hnode, hnode1;
	CString strDevType;
	switch(node.Info.nDVRType)
	{
	case NET_DVR_MPEG4_SX2:
		strDevType.Format("LB");
		break;
	case NET_DVR_MEPG4_ST2:
		strDevType.Format("GB");
		break;
	case NET_DVR_MEPG4_SH2:
		strDevType.Format("HB");
		break;
	case NET_DVR_ATM:
		strDevType.Format("ATM");
		break;
	case NET_DVR_NONREALTIME:
		strDevType.Format("NRT");
		break;
	case NET_DVR_MPEG4_NVSII:
		strDevType.Format("NVS");
		break;
	case NET_NB_SERIAL:
		strDevType.Format("NB");
		break;
	case NET_LN_SERIAL:
		strDevType.Format("LN");
		break;
	case NET_BAV_SERIAL:
		strDevType.Format("BAV");
		break;
	case NET_SDIP_SERIAL:
		strDevType.Format("EIVS");
		break;
	case NET_IPC_SERIAL:
		strDevType.Format("IPC");
		break;
	case NET_NVS_B:
		strDevType.Format("NVS B");
		break;
	case NET_NVS_C:
		strDevType.Format("NVS H");
		break;
	case NET_NVS_S:
		strDevType.Format("NVS S");
		break;
	case NET_NVS_E:
		strDevType.Format("NVS E");
		break;
	case NET_DVR_NEW_PROTOCOL:
		// 此处从设备获取相应的名字，如果获取不到设为UnKnow
		{
			strDevType.Format("Unknown"); 

			char buf[1024] = {0};
			int retBufLen = 0;
			if (CLIENT_QueryDevState(node.LoginID, DH_DEVSTATE_TYPE, buf, sizeof(buf)-1, &retBufLen))
			{
				// 返回的内容有值
				if (strlen(buf) > 0)
				{
					strDevType.Format(buf);
				}
			}	
		}
		break;
	case NET_NVD_SERIAL:
		strDevType.Format("NVD");
		break;
	case NET_DVR_N5:
		strDevType.Format("N5");
		break;
	case NET_DVR_MIX_DVR:
		strDevType.Format("HDVR");
		break;
	case NET_SVR_SERIAL:
		strDevType.Format("SVR");
		break;
	case NET_SVR_BS:
		strDevType.Format("SVR-BS");
		break;
	case NET_NVR_SERIAL:
		strDevType.Format("NVR");
		break;
	case NET_DVR_N51:
		strDevType.Format("N51");
		break;
	case NET_DVR_N52:
		strDevType.Format("N52");
		break;
	case NET_ESS_SERIAL:
		strDevType.Format("ESS");
		break;
	case NET_IVS_PC:
		strDevType.Format("IVS-PC");
		break;
	case NET_PC_NVR:
		strDevType.Format("PC_NVR");
		break;
	case NET_DSCON:
		strDevType.Format("DSCON");
		break;
	case NET_EVS:
		strDevType.Format("EVS");
		break;
	case NET_EIVS:
		strDevType.Format("EIVS");
		break;
    case NET_MATRIX_SERIAL:
        strDevType.Format("MATRIX");
        break;
	case NET_ITC_SERIAL:
		strDevType.Format("ITC");
		break;
	case NET_BSC_SERIAL:
		strDevType.Format("BSC");
		break;
	case NET_DVR_DDNS:
		strDevType.Format("DDNS");
		break;
	case NET_DVR_MPEG1_2:
		strDevType.Format("MPEG1_2");
		break;
	case NET_DVR_MPEG1_8:
		strDevType.Format("MPEG1_8");
		break;
	case NET_DVR_MPEG4_16:
		strDevType.Format("MPEG4_16");
		break;
	case NET_DVR_MPEG4_8:
		strDevType.Format("MPEG4_8");
		break;
	case NET_DVR_N56:
		strDevType.Format("N56");
		break;
	case NET_DVR_N6:
		strDevType.Format("DVR_N6");
		break;
	case NET_DVR_NONREALTIME_MACE:
		strDevType.Format("NRT_MACE");
		break;
	case NET_DVR_STD_NEW:
		strDevType.Format("STD_NEW");
		break;
	case NET_HWS_SERIAL:
		strDevType.Format("HWS");
		break;
	case NET_ITSE_SERIAL:
		strDevType.Format("ITSE");
		break;
	case NET_IVS_B:
		strDevType.Format("IVS-B");
		break;
	case NET_IVS_F:
		strDevType.Format("IVS-F");
		break;
	case NET_IVS_SERIAL:
		strDevType.Format("IVS");
		break;
	case NET_IVS_V:
		strDevType.Format("IVS-V");
		break;
	case NET_NVS_MPEG1:
		strDevType.Format("NVS_MPEG1");
		break;
	case NET_NVS_SERIAL:
		strDevType.Format("NVS");
		break;
	case NET_PRODUCT_NONE:
		strDevType.Format("Unknown");
		break;
	case NET_PVR_SERIAL:
		strDevType.Format("PVR");
		break;
	case NET_TPC_SERIAL:
		strDevType.Format("TPC");
		break;
	case NET_UDS:
		strDevType.Format("UDS");
		break;
	case NET_VTNC_SERIAL:
		strDevType.Format("VTNC");
		break;
	case NET_VTO_SERIAL:
		strDevType.Format("VTO");
		break;
	case NET_IVSS:
		strDevType.Format("IVSS");
		break;
	default:
		strDevType.Format("Unknown");
		break;
	}

	strDev.Format(" %s (%s)<%s>",node.Name, node.IP, strDevType.GetBuffer(0));
	if (node.bIsOnline)
	{
		strDev = strDev + ConvertString("(on-line)");
	}
	else
	{
		strDev = strDev +ConvertString("(off-line)");
	}
	hnode = m_devicelist.InsertItem(strDev,0,0,TVI_ROOT);
	//设备项直接将设备信息指针作为列表Id
	m_devicelist.SetItemData(hnode,(DWORD)&node);		
    int nTotalChnNum = __max(node.nChnNum, node.Info.nChanNum);
	for(int j = 0; j < nTotalChnNum; j++)
	{
		strCh.Format(ConvertString("channel %02d"),j+1);

		hnode1 = m_devicelist.InsertItem(strCh,0,0,hnode);
		//通道项将通道序号作为列表ID
		m_devicelist.SetItemData(hnode1,j);
	}	

	return 0;
}


////////////////////////////////刷新显示相关的接口函数//////////////////////////////////////////
//刷新设备列表显示，直接根据g_ptrdevicelist重新刷新显示
void CNetSDKDemoDlg::UpdateDeviceList()
{
	m_devicelist.DeleteAllItems(); 

	CDevMgr::GetDevMgr().For_EachDev(UpdateDeviceListCallback, (LDWORD)this);

	if (m_devicelist.GetCount() > 0)
	{
		m_devicelist.SetFocus();
	}
}

//画面分割刷新显示
void CNetSDKDemoDlg::UpdateScreen(int nSplit)
{
	
}

//刷新当前显示画面的相关信息
void CNetSDKDemoDlg::UpdateCurScreenInfo()
{
	//判断是否打开了语音对讲
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	m_advanceBtnPannel.SetCheckSound(m_curSoundSplit == m_curScreen ? 1 : 0);

	//更新视频参数项
	UpdateVideoCtrl(VIDEO_TOTAL);	
}

//刷新视频参数控制区,nMode 0~3对应单项刷新
void CNetSDKDemoDlg::UpdateVideoCtrl(int nMode)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	BYTE bVideo[4];
	memset(bVideo,0, 4);
	if(siNode.Type == SPLIT_TYPE_MONITOR)
	{
		memcpy(bVideo, siNode.nVideoParam.bParam, 4);
		BOOL nRet = CLIENT_ClientSetVideoEffect(siNode.iHandle, 
									bVideo[0], bVideo[1], bVideo[2], bVideo[3]);
		if (!nRet)
		{
			LastError();
		}
	}
}  
    
/////////////for one////////////////
DWORD GetMonthDays(const DWORD& dwYear, const DWORD& dwMonth)
{
	DWORD dwMDays = 0;
	switch(dwMonth)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		dwMDays = 31;
	break;
	case 2:
	{
		if (((dwYear%4==0)&& dwYear%100) || (dwYear%400==0))
		{
			dwMDays = 29; 
		}
		else
		{
			dwMDays = 28;
		}
	}
	break;
	case 4:
	case 6:
	case 9:
	case 11:
	dwMDays = 30;
	break;
	default:
	break;
	} 

	return dwMDays;
}
 
/*
 * can't surpport year offset time
 */
NET_TIME GetSeekTimeByOffsetTime(const NET_TIME& bgtime, unsigned int offsettime)
{
	NET_TIME tmseek;
	memset(&tmseek, 0x00, sizeof(NET_TIME));
 
	DWORD dwNext = 0;
	//second
	tmseek.dwSecond = (bgtime.dwSecond+offsettime)%60;
	dwNext = (bgtime.dwSecond+offsettime)/60;
	//minute
	tmseek.dwMinute = (dwNext+bgtime.dwMinute)%60;
	dwNext = (dwNext+bgtime.dwMinute)/60;
	//hour
	tmseek.dwHour = (dwNext+bgtime.dwHour)%24;
	dwNext = (dwNext+bgtime.dwHour)/24;
 
	DWORD dwMDays = GetMonthDays(bgtime.dwYear, bgtime.dwMonth);
	// 如果年月的时间不对导致返回的一个月的天数为0则直接返回
	// 防止后面的除0崩溃
	if (0 == dwMDays)	
	{
		return tmseek;
	}
 
	//day
	tmseek.dwDay = (dwNext+bgtime.dwDay)%dwMDays;
	dwNext = (dwNext+bgtime.dwDay)/dwMDays;
	//month
	tmseek.dwMonth  = (dwNext+bgtime.dwMonth)%12;
	dwNext = (dwNext+bgtime.dwMonth)/12;
 
	tmseek.dwYear = dwNext+bgtime.dwYear;
 
	return tmseek;
}
 
/*
 * can't surpport year offset time
 */
DWORD GetOffsetTime(const NET_TIME& st, const NET_TIME& et)
{
	DWORD dwRet = -1; 
	if (et.dwYear != st.dwYear)
	{
		return dwRet;
	}

	DWORD dwDays = 0;

	for(int i=st.dwMonth+1; i < et.dwMonth; ++i)
	{
		dwDays += GetMonthDays(st.dwYear, i);
	}

	if (et.dwMonth == st.dwMonth)
	{
		dwDays +=et.dwDay - st.dwDay;
	}
	else
	{
		dwDays += et.dwDay;
		dwDays += GetMonthDays(st.dwYear, st.dwMonth) - st.dwDay;
	}

	dwRet = dwDays*24*60*60 + ((int)et.dwHour - (int)st.dwHour)*60*60 + 
		((int)et.dwMinute - (int)st.dwMinute)*60 + (int)et.dwSecond - (int)st.dwSecond;

	return dwRet;
}
/////////////for one////////////////
//播放进度条拖动的处理
BOOL CNetSDKDemoDlg::SeekPlayPositon(int nPos)
//void CNetSDKDemoDlg::OnReleasedcapturePlayPosition(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	ret = FALSE;
	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam * pbParam = (SplitNetPlayParam *)siNode.Param;
		if (pbParam->iStatus == STATUS_PAUSE) 
		{
			return FALSE;
		}

		pbParam->iPlayPos = nPos;
		DWORD total = pbParam->pFileInfo->fileinfo.size;
		ret = CLIENT_SeekPlayBack(siNode.iHandle, 0xFFFFFFFF, nPos * total /100);
		if (!ret)
		{
			LastError();
		}
	}
	else if (siNode.Type == SPLIT_TYPE_PBBYTIME) 
	{
		SplitPBByTimeParam * pbParam = (SplitPBByTimeParam *)siNode.Param;
		if (pbParam->iStatus == STATUS_PAUSE) 
		{
			return FALSE;
		}
		
		pbParam->npos = nPos;		
		DWORD time = GetOffsetTime(pbParam->starttime, pbParam->endtime);
		
		BOOL ret = CLIENT_SeekPlayBack(siNode.iHandle, nPos * time /100, 0xFFFFFFFF);
		if (!ret)
		{
			LastError();
		}
	}

	return ret;
}

////////////////////////////////////////////功能操作//////////////////////////////////////////////
//增加设备连接项
void CNetSDKDemoDlg::AddDevice() 
{
	AddDeviceDlg dlg;
		
	if(dlg.DoModal() == IDOK)
	{
		UpdateDeviceList();	   //刷新设备列表显示	
	}	
	// TODO: Add your control notification handler code here
	
}


//回放数据回调函数，demo里将数据保存成文件
int CALLBACK PBDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return -1;
	}
	CNetSDKDemoDlg *dlg = (CNetSDKDemoDlg *)dwUser;
	dlg ->ReceiveRealData(lRealHandle,dwDataType, pBuffer, dwBufSize);
	return 1;
}

void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize, LLONG lParam, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CNetSDKDemoDlg *dlg = (CNetSDKDemoDlg *)dwUser;
	dlg->ReceiveRealData(lRealHandle,dwDataType, pBuffer, dwBufSize, lParam);
}

//根据设备列表取得设备Id，根据通道选择通道号
void CNetSDKDemoDlg::OpenChannel() 
{
	HTREEITEM node_dev, tmpnode;
	DWORD nItem;
	CString strCh;
	DeviceNode *pInfo;
	
	tmpnode = m_devicelist.GetSelectedItem();
	if(!tmpnode)
	{
		MessageBox(ConvertString(MSG_DEMODLG_CHECKSEL));
		return;
	}
	
	DH_RealPlayType subtype;
	int subidx = m_normalBtnPannel.GetSubType();
	switch(subidx)
	{
	case 0:
		subtype = DH_RType_Realplay_0;
		break;
	case 1:
		subtype = DH_RType_Realplay_1;
		break;
	case 2:
		subtype = DH_RType_Realplay_2;
		break;
	case 3:
		subtype = DH_RType_Realplay_3;
		break;
	default:
		subtype = DH_RType_Realplay_0;
	}

	nItem = m_devicelist.GetItemData(tmpnode);
	if(!m_devicelist.GetChildItem(tmpnode))     //是通道项
	{
		node_dev = m_devicelist.GetParentItem(tmpnode);
		if (NULL != node_dev)
		{
			pInfo = (DeviceNode *)m_devicelist.GetItemData(node_dev);
			
			OpenSingleChannel(pInfo, nItem, m_curScreen, subtype);
		}
	}
	else              //设备项
	{
		OpenAllChannel((DeviceNode *)nItem, subtype);
	}
}

void CNetSDKDemoDlg::OpenSingleChannel(DeviceNode *pInfo, int nCh, int screenNo, DH_RealPlayType subtype)
{
	//查询是否被列入轮循列表
	if (IsCycling((DWORD)pInfo, nCh))
	{
		MessageBox(ConvertString(MSG_DEMODLG_ERROR_CYCLING));
		return;
	}
	
	//关闭当前窗口的播放
	ProcessCloseScreen(screenNo);

	CWnd* pWnd = m_screenPannel.GetPage(screenNo);
	if (!pWnd)
	{
		return ;
	}

	LLONG nID = pInfo->LoginID;

	LLONG nChannelID;
	
	nChannelID = CLIENT_RealPlayEx(nID, nCh, pWnd->m_hWnd, subtype);
	
	int nStreamType = subtype;
    if (nStreamType >= DH_RType_Realplay_0 && nStreamType <= DH_RType_Realplay_3)
    {
        nStreamType -= (int)DH_RType_Realplay_0;
    }
    else 
    {
        nStreamType = 0;
    }

	CLIENT_MakeKeyFrame(nID, nCh, nStreamType);
	if(!nChannelID)
	{
		LastError();
		MessageBox(ConvertString(MSG_DEMODLG_OPENCHLFAILED));
		return;
	}
	//获取视频参数
	BYTE bVideo[4];
	BOOL nRet = CLIENT_ClientGetVideoEffect(nChannelID, &bVideo[0],&bVideo[1],&bVideo[2],&bVideo[3]);
	if (!nRet)
	{
		LastError();
	}

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(screenNo, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
	}
	siNode.Type = SPLIT_TYPE_MONITOR;
	siNode.nVideoParam.dwParam = *(DWORD *)bVideo;
	siNode.iHandle = nChannelID;

	SplitMonitorParam *mparam	= new SplitMonitorParam;
	mparam->pDevice  = pInfo;
	mparam->iChannel  = nCh;
	
	siNode.Param  = mparam;

	ret = SetSplitInfo_Main(screenNo, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while setting split info"));	
	}

	//更新保存数据状态
	if(siNode.isSaveData)
	{
		CString strName ;
		strName.Format("savech%d.dav", nCh);
		BOOL ret = CLIENT_SaveRealData(nChannelID, strName.GetBuffer(0));
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_SAVEDATAFAILED));
		}
	}

	UpdateCurScreenInfo();

	//更新音频
	if(m_curSoundSplit == screenNo)
	{
		BOOL nRet = CLIENT_OpenSound(nChannelID);
		if (!nRet)
		{
			LastError();
		}
	}
	//设置数据回调
	BOOL cbRec = CLIENT_SetRealDataCallBackEx2(nChannelID, RealDataCallBackEx, (LDWORD)this, 0x0f);
	if (!cbRec)
	{
		LastError();	
	}
}

//接收播放进度状态处理
void CNetSDKDemoDlg::ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize)
{
	//取得窗口号
	int nScreen = GetHandleSplit(lPlayHandle);
	if(nScreen < 0)
	{
		return;
	}
	
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(nScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
	}
	
	//更新播放进度值
	if (siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *nParam = (SplitNetPlayParam *)siNode.Param;
		if (nParam && dwTotalSize > 0)
		{
			nParam->iPlayPos = dwDownLoadSize * 100 / dwTotalSize;
			if (nParam->iPlayPos > 100)
			{
					int xyz = 0;
			}
			if(((int)dwDownLoadSize == -1))
			{
				PlayStop(nScreen);				
			}
		}
	}
	else if (siNode.Type == SPLIT_TYPE_PBBYTIME)
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		if (pbParam && dwTotalSize > 0)
		{
			pbParam->npos = dwDownLoadSize * 100 / dwTotalSize;
			if(((int)dwDownLoadSize == -1) || (pbParam->npos >= 100))
			{
				PlayStop(nScreen);
			}
		}
	}
}

//播放进度状态回调
void CALLBACK PlayCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	CNetSDKDemoDlg *dlg = (CNetSDKDemoDlg *)dwUser;
	dlg->ReceivePlayPos(lPlayHandle, dwTotalSize, dwDownLoadSize);
}

//录像查询
void CNetSDKDemoDlg::SearchRecord() 
{
	CSearchRecord dlg;
	if(dlg.DoModal() == IDOK)   //打开回放时
	{	
		if (!dlg.m_playfile) 
		{
			return;
		}
		if(!CheckCurSplitAndClose())
		{
			return;
		}
		PlayRecordFile(dlg.m_playfile, m_curScreen);		
	}
}

void CNetSDKDemoDlg::PlayRecordFile(FileInfoNode* playfile, int scrNo)
{
	CWnd *plWnd = m_screenPannel.GetPage(scrNo);
	if (!plWnd)
	{
		MessageBox(ConvertString("unexpected error!!!!"));
		return;
	}

	LLONG lPlayHandle = CLIENT_PlayBackByRecordFile(playfile->pDevice->LoginID, &playfile->fileinfo, 
		plWnd->m_hWnd, PlayCallBack, (LDWORD)this);


	if(!lPlayHandle)
	{
		LastError();
		MessageBox(ConvertString(MSG_DEMODLG_PLAYFAILED));
		return;
	}
	else
	{  //如果其它通道没有打开音频，则打开音频
		if (m_curSoundSplit < 0)
		{
			if (FALSE == CLIENT_OpenSound(lPlayHandle)) 
			{
				LastError();
				MessageBox(ConvertString(MSG_OPENSOUNDFAILED));
				m_advanceBtnPannel.SetCheckSound(0);
			}
			else
			{
				m_advanceBtnPannel.SetCheckSound(1);
				m_curSoundSplit = scrNo;
			}
		}
	}
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(scrNo, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
	}
	
	siNode.Type = SPLIT_TYPE_NETPLAY;
	siNode.iHandle = lPlayHandle;
	
	SplitNetPlayParam *nParam	= new SplitNetPlayParam;
	nParam->pFileInfo = new FileInfoNode;
	memcpy(nParam->pFileInfo, playfile, sizeof(FileInfoNode));
	nParam->iPlayPos = 0;
	nParam->iStatus = STATUS_PLAY;
	
	siNode.Param = nParam;
	ret = SetSplitInfo_Main(scrNo, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
	}
		
	UpdateCurScreenInfo();
}

//停止播放当前播放画面
BOOL CNetSDKDemoDlg::PlayCtrl_Stop()
{
	int nRet = PlayStop(m_curScreen);
	if (nRet < 0)
	{
		return FALSE;
	}
	else
	{
	
	}
	return TRUE;
}

//快放
BOOL CNetSDKDemoDlg::PlayCtrl_Fast()
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *pParam = (SplitNetPlayParam *)siNode.Param;
		if(pParam->iStatus == STATUS_PAUSE)
		{
			goto e_exit;
		}
		if(pParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto e_exit;
			}
			pParam->iStatus = STATUS_STOP;
		}
		if (m_curSoundSplit == m_curScreen)
		{
			if (FALSE == CLIENT_CloseSound())
			{
				LastError();
				MessageBox(ConvertString(MSG_CLOSESOUNDFAILED));
				goto e_exit;
			}
			m_advanceBtnPannel.SetCheckSound(0);
			m_curSoundSplit = -1;
		}
		
		ret = CLIENT_FastPlayBack(siNode.iHandle);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
		}
		pParam->iStatus = STATUS_PLAY;
	}
	else if (siNode.Type == SPLIT_TYPE_PBBYTIME)
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		if(pbParam->iStatus == STATUS_PAUSE)
		{
			goto e_exit;
		}
		if(pbParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
			}
			else
			{
				pbParam->iStatus = STATUS_STOP;
			}
		}
		if (m_curSoundSplit == m_curScreen)
		{
			if (FALSE == CLIENT_CloseSound())
			{
				LastError();
				MessageBox(ConvertString(MSG_CLOSESOUNDFAILED));
				goto e_exit;
			}
			m_advanceBtnPannel.SetCheckSound(0);
			m_curSoundSplit = -1;
		}
		ret = CLIENT_FastPlayBack(siNode.iHandle);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto e_exit;
		}
		pbParam->iStatus = STATUS_PLAY;
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTPLAYING));
		goto e_exit;
	}

	return TRUE;
e_exit:
	return FALSE;

}

//慢放
BOOL CNetSDKDemoDlg::PlayCtrl_Slow() 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *pParam = (SplitNetPlayParam *)siNode.Param;
		if(pParam->iStatus == STATUS_PAUSE)
		{
			goto e_exit;
		}
		
		if(pParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto e_exit;
			}
			pParam->iStatus = STATUS_STOP;
		}

		ret = CLIENT_SlowPlayBack(siNode.iHandle);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto e_exit;
		}
		pParam->iStatus = STATUS_PLAY;	
	}
	else if (siNode.Type == SPLIT_TYPE_PBBYTIME)
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		if(pbParam->iStatus == STATUS_PAUSE)
		{
			goto e_exit;
		}
		
		if(pbParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto e_exit;
			}
			pbParam->iStatus = STATUS_STOP;
		}

		ret = CLIENT_SlowPlayBack(siNode.iHandle);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto e_exit;
		}
		pbParam->iStatus = STATUS_PLAY;	
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTPLAYING));
		goto e_exit;
	}

	return TRUE;
e_exit:
	return FALSE;
}

//单帧播放
BOOL CNetSDKDemoDlg::PlayCtrl_Step() 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *pParam = (SplitNetPlayParam *)siNode.Param;
		if(pParam->iStatus == STATUS_PAUSE)
		{
			goto e_exit;
		}
		ret = CLIENT_StepPlayBack(siNode.iHandle, FALSE);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto e_exit;
		}
		pParam->iStatus = STATUS_STEP;
	}
	else if (siNode.Type == SPLIT_TYPE_PBBYTIME) 
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		if(pbParam->iStatus == STATUS_PAUSE)
		{
			MessageBox(ConvertString("paused"));
			goto e_exit;
		}
		ret = CLIENT_StepPlayBack(siNode.iHandle, FALSE);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto e_exit;
		}
		pbParam->iStatus = STATUS_STEP;
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTPLAYING));
		goto e_exit;
	}

	return TRUE;
e_exit:
	return FALSE;
}

BOOL CNetSDKDemoDlg::PlayCtrl_Frame(int frame)
{
//	KillTimer(TIMER_KBPS);
	if(!UpdateData(true))
	{
		return FALSE;
	}

	if ((frame < 0) || (frame > 120))
	{
		MessageBox(ConvertString(MSG_SCHRECORD_ILLEGALFRAME));
		return FALSE;
	}

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}
	
	ret = FALSE;
	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *pParam = (SplitNetPlayParam *)siNode.Param;
		if(pParam->iStatus == STATUS_PAUSE)
		{
			goto out;
		}
		if(pParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto out;
			}
			pParam->iStatus = STATUS_STOP;
		}
		if (m_curSoundSplit == m_curScreen)
		{
			if (FALSE == CLIENT_CloseSound())
			{
				LastError();
				MessageBox(ConvertString(MSG_CLOSESOUNDFAILED));
				goto out;
			}
			m_advanceBtnPannel.SetCheckSound(0);
			m_curSoundSplit = -1;
		}
		ret = CLIENT_SetFramePlayBack(siNode.iHandle, frame);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto out;
		}
	}
	else if(siNode.Type == SPLIT_TYPE_PBBYTIME)
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		if(pbParam->iStatus == STATUS_PAUSE)
		{
			goto out;
		}
		if(pbParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto out;
			}
			pbParam->iStatus = STATUS_STOP;
		}
		if (m_curSoundSplit == m_curScreen)
		{
			if (FALSE == CLIENT_CloseSound())
			{
				LastError();
				MessageBox(ConvertString(MSG_CLOSESOUNDFAILED));
				goto out;
			}
			m_advanceBtnPannel.SetCheckSound(0);
			m_curSoundSplit = -1;
		}
		ret = CLIENT_SetFramePlayBack(siNode.iHandle, frame);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
			goto out;
		}
	}
	else
	{	
		MessageBox(ConvertString(MSG_DEMODLG_NOTPLAYING));
		goto out;
	}
out:
	return ret;
}


//播放/暂停切换
BOOL CNetSDKDemoDlg::PlayCtrl_Play()
{
	BOOL isPause = false;

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	ret = FALSE; 
	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *pParam = (SplitNetPlayParam *)siNode.Param;
		if(pParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto out;
			}
			pParam->iStatus = STATUS_STOP;
		}

		if(pParam->iStatus != STATUS_PAUSE)
		{
			isPause = true;	
		}

		ret = CLIENT_PausePlayBack(siNode.iHandle, isPause);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
		}
		else
		{
			if(pParam->iStatus == STATUS_PAUSE)
			{
				pParam->iStatus = STATUS_PLAY;
			}
			else
			{
				pParam->iStatus = STATUS_PAUSE;
			}
		}
	}
	else if(siNode.Type == SPLIT_TYPE_PBBYTIME)
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		if(pbParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto out;
			}
			pbParam->iStatus = STATUS_STOP;
		}

		if(pbParam->iStatus != STATUS_PAUSE)
		{
			isPause = true;	
		}
		
		ret = CLIENT_PausePlayBack(siNode.iHandle, isPause);
		if(!ret)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYCTRLFAILED));
		}	
		else
		{
			if(pbParam->iStatus == STATUS_PAUSE)
			{
				pbParam->iStatus = STATUS_PLAY;
			}
			else
			{
				pbParam->iStatus = STATUS_PAUSE;
			}
		}
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTPLAYING));
	}

out:
	return ret;
}

int CNetSDKDemoDlg::PlayStop(int iScreen, BOOL bDis)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(iScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		goto e_clear;
	}

	CPlayWnd *plWnd;
	if(siNode.Type == SPLIT_TYPE_NETPLAY)
	{
		SplitNetPlayParam *pParam = (SplitNetPlayParam *)siNode.Param;
		//如果处于单帧播放状态，需先恢复
		if(pParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto e_clear;
			}
		}	
		ret = CLIENT_StopPlayBack(siNode.iHandle);
		if (!ret)
		{
			LastError();
			if (!bDis)
			{
				MessageBox(ConvertString(MSG_DEMODLG_STOPPLAYFAILED));
				goto e_clear;
			}
		}
		FileInfoNode *pFile = ((SplitNetPlayParam *)siNode.Param)->pFileInfo;
		delete pFile;
		delete (SplitNetPlayParam *)siNode.Param;
		siNode.Param = NULL;
		siNode.Type = SPLIT_TYPE_NULL;
		ret = SetSplitInfo_Main(iScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
		}

		if (m_curSoundSplit == iScreen)
		{
			//m_advanceBtnPannel.SetCheckSound(0);
			m_curSoundSplit = -1;
		}
	}
	else if (siNode.Type == SPLIT_TYPE_PBBYTIME)
	{
		SplitPBByTimeParam *pbParam = (SplitPBByTimeParam *)siNode.Param;
		//如果处于单帧播放状态，需先恢复
		if(pbParam->iStatus == STATUS_STEP)
		{
			ret = CLIENT_StepPlayBack(siNode.iHandle, TRUE);
			if (!ret)
			{
				LastError();
				goto e_clear;
			}
		}
		
		ret = CLIENT_StopPlayBack(siNode.iHandle);
		if (!ret)
		{
			LastError();
			if (!bDis)
			{
				MessageBox(ConvertString(MSG_DEMODLG_STOPPLAYFAILED));
				goto e_clear;
			}
		}
		delete (SplitPBByTimeParam *)siNode.Param;
		//add by zhaojs 2006-12-06
		siNode.Param = NULL;
		siNode.Type = SPLIT_TYPE_NULL;
		ret = SetSplitInfo_Main(iScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
		}

		if (m_curSoundSplit == iScreen)
		{
			//m_advanceBtnPannel.SetCheckSound(0);
			m_curSoundSplit = -1;
		}
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTPLAYING));
		goto e_clear;
	}

	// repaint the play window
	plWnd = (CPlayWnd *)m_screenPannel.GetPage(iScreen);
	if (plWnd)
	{
		plWnd->PostMessage(VIDEO_REPAINT);
	}

	return 0;
e_clear:
	return -1;
}

//对当前预览通道进行抓图,对播放通道是否能抓图有待验证
void CNetSDKDemoDlg::CaptureScreen() 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	CString strName = "pct.bmp";
	
	//支持多种图像的抓图
	if(siNode.Type == SPLIT_TYPE_NULL)
	{
		MessageBox(ConvertString(MSG_DEMODLG_CANTCAPTURE));
		return;
	}

	//抓图保存文件名的输入
	CFileDialog dlg(FALSE,"*.bmp","pct.bmp",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"All File(*.bmp)|*.*||",this);

	if(dlg.DoModal() == IDOK)
	{
		strName = dlg.GetPathName();		
	}

	ret = CLIENT_CapturePicture(siNode.iHandle, strName.GetBuffer(0));
	if(!ret)
	{
		LastError();
		MessageBox(ConvertString(MSG_DEMODLG_CAPTUREFAILED));
	}
}

//强制I帧
void CNetSDKDemoDlg::SetIframe() 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	if(siNode.Type != SPLIT_TYPE_MONITOR)
	{
		MessageBox(ConvertString(MSG_DEMODLG_CANTFORCE_I));
		return;
	}
	
	SplitMonitorParam *mParam = (SplitMonitorParam *)siNode.Param;

	ret = CLIENT_MakeKeyFrame(mParam->pDevice->LoginID, mParam->iChannel);
	if(!ret)
	{
		LastError();
		MessageBox(ConvertString(MSG_DEMODLG_FAILEDFORCE_I));
	}
}

//是否打开声音
BOOL CNetSDKDemoDlg::OpenSound(BOOL bOpen) 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return FALSE;
	}

	if(bOpen)
	{
		if (m_curSoundSplit >= 0 && 
			(siNode.Type == SPLIT_TYPE_MONITOR || 
			siNode.Type == SPLIT_TYPE_NETPLAY || 
			siNode.Type == SPLIT_TYPE_PBBYTIME))
		{
			if (FALSE == CLIENT_CloseSound())
			{
				LastError();
				MessageBox(ConvertString(MSG_CLOSESOUNDFAILED));
				return FALSE;
			}
		}
		m_curSoundSplit = m_curScreen;
		if (siNode.Type == SPLIT_TYPE_MONITOR || 
			siNode.Type == SPLIT_TYPE_NETPLAY ||
			siNode.Type == SPLIT_TYPE_PBBYTIME)
		{
			if (FALSE == CLIENT_OpenSound(siNode.iHandle))
			{
				LastError();
				MessageBox(ConvertString(MSG_OPENSOUNDFAILED));
				return FALSE;
			}
		}
	}
	else
	{
		if (siNode.Type == SPLIT_TYPE_MONITOR || 
			siNode.Type == SPLIT_TYPE_NETPLAY ||
			siNode.Type == SPLIT_TYPE_PBBYTIME)
		{
			if (FALSE == CLIENT_CloseSound())
			{
				LastError();
				MessageBox(ConvertString(MSG_CLOSESOUNDFAILED));
				return FALSE;
			}
		}
		m_curSoundSplit = -1;
	}
	
	return TRUE;
}

//是否保存实时数据数据
void CNetSDKDemoDlg::SaveRealdata(int nCheck) 
{
	CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(m_curScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("we have a big error here!"));
		return;
	}
	
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	CString strName = "save.dav";
	if(nCheck > 0)
	{
		siNode.isSaveData = 1;
		if (siNode.Type == SPLIT_TYPE_MONITOR || siNode.Type == SPLIT_TYPE_MULTIPLAY)
		{
			//文件名的输入
			CFileDialog dlg(FALSE,"*.dav","save.dav",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
			"All File(*.dav)|*.*||",this);
			if(dlg.DoModal() == IDOK)
			{
				strName = dlg.GetPathName();		
			}
			else
			{
				return;
			}
			BOOL ret = CLIENT_SaveRealData(siNode.iHandle, strName.GetBuffer(0));
			if(!ret)
			{
				LastError();
				MessageBox (ConvertString(MSG_DEMODLG_SAVEDATAFAILED));
			}
			else
			{
				plWnd->SetSplitCBFlag_Real(1);
			}
		}
	}
	else
	{
		siNode.isSaveData = 0;
		if(siNode.Type == SPLIT_TYPE_MONITOR || siNode.Type == SPLIT_TYPE_MULTIPLAY)
		{
			BOOL stop = CLIENT_StopSaveRealData(siNode.iHandle);
			if (stop)
			{
				plWnd->SetSplitCBFlag_Real(0);
				MessageBox(ConvertString(MSG_DEMODLG_REALSAVE_STOPPED));
			}
			else
			{
				LastError();
				MessageBox(ConvertString("stop save file failed"));
			}
		}
	}
}

//用来打开显示画面分割各显示区域的信息
void CNetSDKDemoDlg::ShowFluxInfo() 
{
	
	CSplitInfoDlg dlg;
	m_runtimeMsgPannel.EnableShowFlux(FALSE);
	dlg.DoModal();
	m_runtimeMsgPannel.EnableShowFlux(TRUE);
}


//关闭预览图像
void CNetSDKDemoDlg::CloseScreen() 
{
	BOOL ret = ProcessCloseScreen(m_curScreen);
	UpdateCurScreenInfo();
}


void CNetSDKDemoDlg::PtzControl(int type, BOOL stop, int param)
{
	BOOL ret;
	SplitMonitorParam *nParam ;
	BOOL upRet;
	
	LLONG lHandle;
	int iChannel;

	if (stop) 
	{
		if (!m_bPTZCtrl)
		{
			goto exitPTZCtrl;
		}
	}

	switch(type) {
	//在主页面的控制
	case DH_PTZ_UP_CONTROL  :    //上           
	case DH_PTZ_DOWN_CONTROL:		//下
	case DH_PTZ_LEFT_CONTROL:	//左
	case DH_PTZ_RIGHT_CONTROL:		//右
	case DH_PTZ_ZOOM_ADD_CONTROL:		//变倍
	case DH_PTZ_ZOOM_DEC_CONTROL:		
	case DH_PTZ_FOCUS_ADD_CONTROL:	//调焦
	case DH_PTZ_FOCUS_DEC_CONTROL:	
	case DH_PTZ_APERTURE_ADD_CONTROL:	//光圈
	case DH_PTZ_APERTURE_DEC_CONTROL:
	case DH_EXTPTZ_LEFTTOP :
	case DH_EXTPTZ_RIGHTTOP :
	case DH_EXTPTZ_LEFTDOWN :
	case DH_EXTPTZ_RIGHTDOWN:
		upRet = UpdateData(true);
		if (!upRet) 
		{
			goto exitPTZCtrl;
		}
		break;
	case DH_PTZ_POINT_MOVE_CONTROL  :   //转至
	case DH_PTZ_POINT_SET_CONTROL   :   //设置
	case DH_PTZ_POINT_DEL_CONTROL   :   //删除
	case DH_PTZ_POINT_LOOP_CONTROL :   //点间轮循
	case DH_PTZ_LAMP_CONTROL:            //灯
	default:
		MessageBox(ConvertString(MSG_DEMODLG_PTZCMDFAILED));
		goto exitPTZCtrl;
	}

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));

	ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	if(siNode.Type != SPLIT_TYPE_MONITOR)
	{
		goto exitPTZCtrl;
	}
	nParam = (SplitMonitorParam *)siNode.Param;
	lHandle = nParam->pDevice->LoginID;
	iChannel = nParam->iChannel;
	if(type >= DH_EXTPTZ_LEFTTOP)
	{
		ret = CLIENT_DHPTZControl(lHandle,iChannel, type, (BYTE)param, (BYTE)param,0, stop);
		if (!ret)
		{
			LastError();
		}
		m_bPTZCtrl = !stop;
	}
	else
	{
		ret = CLIENT_DHPTZControl(lHandle,iChannel, type, 0, (BYTE)param,0, stop);
		if (!ret)
		{
			LastError();
		}
		m_bPTZCtrl = !stop;
	}
	if(!ret)
	{
		MessageBox(ConvertString(MSG_DEMODLG_PTZCTRLFAILED));
		goto exitPTZCtrl;
	}

	return;

exitPTZCtrl:
	m_bPTZCtrl = FALSE;
	return;
}


int CloseAllDevCallback(const DeviceNode& node, LDWORD dwUser)
{
	CNetSDKDemoDlg* pThis = (CNetSDKDemoDlg*)dwUser;
	if (!pThis)
	{
		return 1;
	}

	pThis->ProcessDeleteDevice(const_cast<DeviceNode*>(&node), false, true);

	return 0;
}

//窗口关闭时的处理，主要是需要释放动态内存
void CNetSDKDemoDlg::OnClose() 
{	
	//关闭打开的回调数据保存文件
	for(int i = 0; i < 16 ; i++)
	{
		SplitInfoNode siNode;
		memset(&siNode, 0, sizeof(siNode));
		BOOL ret = GetSplitInfo_Main(i, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while getting split info"));
		}
		if(siNode.SavecbFileRaw)
		{
			fclose(siNode.SavecbFileRaw);
		}
		if(siNode.SavecbFileStd)
		{
			fclose(siNode.SavecbFileStd);
		}
		if(siNode.SavecbFilePcm)
		{
			fclose(siNode.SavecbFilePcm);
		}
		if(siNode.SavecbFileYUV)
		{
			fclose(siNode.SavecbFileYUV);
		}
		ProcessCloseScreen(i);
	}
	
	//删除设备列表
	
	CDevMgr::GetDevMgr().For_EachDev(CloseAllDevCallback, (LDWORD)this);
	
	//listen device test
	if (m_lListenChannel)
	{
		BOOL b = CLIENT_StopRealPlayEx(m_lListenChannel);
	}
	if (m_lListenDevice) 
	{
		BOOL b = CLIENT_Logout(m_lListenDevice);
	}
	
    LastError();
	//关闭网络sdk
	
	char* pPath = g_GetIniPath();
	if(NULL != pPath)
	{
		delete []pPath;
	}

	CDialog::OnClose();
}

void CNetSDKDemoDlg::ProcessDeleteDevice(DeviceNode *pDevice, BOOL bDelList, BOOL bDis)
{
	int i;
	FileInfoNode *pInfo;
	if(!pDevice)
	{
		return;
	}

	/*Begin: Add by yehao(10857) For Task.NO.11071 2006-12-26*/
	{
		list<DeviceNode *>::iterator it = m_broadcastDevList.begin();
		for (; it != m_broadcastDevList.end(); it++)
		{
			if (LLONG(pDevice) == LLONG(*it))
			{
				CLIENT_AudioBroadcastDelDev(pDevice->LoginID);
				m_broadcastDevList.erase(it);
				break;
			}
		}
	}
	if (m_talkhandle.pDevice && pDevice->LoginID == m_talkhandle.pDevice->LoginID)
	{
		this->OpenTalk(FALSE);
	}
	/*End: yehao(10857) Task.NO.11071 */

	//检查是否正在刷新该设备的报警信息
	DeviceNode *alarmDev = m_ClientStateDlg.GetDevice();
	if (alarmDev && alarmDev->LoginID == pDevice->LoginID)
	{
		m_ClientStateDlg.StopRefresh();
		m_ClientStateDlg.SetDevice(0);
	}

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));

	DeviceNode *pDev = pDevice;
	//关闭该设备的预览通道和播放通道
	for(i = 0; i < 16; i++)
	{
		BOOL ret = GetSplitInfo_Main(i, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while getting split info"));
		}
		
		switch(siNode.Type)
		{
		case SPLIT_TYPE_MONITOR:
			if (pDev->LoginID == ((SplitMonitorParam *)siNode.Param)->pDevice->LoginID) 
			{
				ProcessCloseScreen(i, bDis);
			}
			break;	
			
		case SPLIT_TYPE_MULTIPLAY:
			if (pDev->LoginID == ((DeviceNode *)siNode.Param)->LoginID) 
			{
				ProcessCloseScreen(i, bDis);
			}
			break;
			
		case SPLIT_TYPE_NETPLAY:
			if (pDev->LoginID == ((SplitNetPlayParam *)siNode.Param)->pFileInfo->pDevice->LoginID) 
			{
				ProcessCloseScreen(i, bDis);
			}
			break;
		case SPLIT_TYPE_PBBYTIME:
			if (pDev->LoginID == ((SplitPBByTimeParam *)siNode.Param)->pDevice->LoginID) 
			{
				ProcessCloseScreen(i, bDis);
			}
			break;
			
		case SPLIT_TYPE_CYCLEMONITOR:
			{
				KillTimer(i);
				if (!siNode.Param) 
				{
					break;
				}
				POSITION pos1, pos2;
				pos1 = ((SplitCycleParam *)siNode.Param)->pChannelList->GetHeadPosition();
				int count = ((SplitCycleParam *)siNode.Param)->pChannelList->GetCount();
				for (int j = 0; j < count; j++)
				{
					pos2 = pos1;
					void *temp = ((SplitCycleParam *)siNode.Param)->pChannelList->GetNext(pos1);
					//比较轮循列表中节点的设备名
					if ((DWORD)pDev == ((CycleChannelInfo *)temp)->dwDeviceID) 
					{	
						//是否刚好是正在播放的节点
						if (j == ((SplitCycleParam *)siNode.Param)->iCurPosition)
						{
							BOOL ret = TRUE;
							if (0 != siNode.iHandle) 
							{
								ret = CLIENT_StopRealPlay(siNode.iHandle);
							}
							if(ret)
							{
								delete (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetAt(pos2);
								((SplitCycleParam *)siNode.Param)->pChannelList->RemoveAt(pos2);
								((SplitCycleParam *)siNode.Param)->iChannelCount--;
								siNode.iHandle = 0;
								ret = SetSplitInfo_Main(i, &siNode);
								if (!ret)
								{
									MessageBox(ConvertString("error while setting split info"));
								}
								//如果列表已空，则结束轮循
								if (((SplitCycleParam *)siNode.Param)->iChannelCount <= 0) 
								{
									DeleteCycleParam(i);
									siNode.Param = NULL;
									break;
								}
							}
							else
							{
								if (siNode.Param) 
								{
									if (!bDis) 
									{
										MessageBox(ConvertString(MSG_CYCLE_CLOSECHANNELFAILED));
									}
									DeleteCycleParam(i);
									siNode.Param = NULL;
								}
								break;
							}
						}
						//不是当前播放节点，直接删除
						else
						{
							delete (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetAt(pos2);
							((SplitCycleParam *)siNode.Param)->pChannelList->RemoveAt(pos2);
							((SplitCycleParam *)siNode.Param)->iChannelCount--;
							if (((SplitCycleParam *)siNode.Param)->iChannelCount <= 0) 
							{
								DeleteCycleParam(i);
								siNode.Param = NULL;
								break;
							}
						}
						//当前位置放回前一个，以便timer计时到了播放正确的下一个
						if (((SplitCycleParam *)siNode.Param)->iCurPosition != 0) 
						{
							((SplitCycleParam *)siNode.Param)->iCurPosition--;
						}
						else
						{
							((SplitCycleParam *)siNode.Param)->iCurPosition = ((SplitCycleParam *)siNode.Param)->iChannelCount - 1;
						}
					}
				}
				//如果列表中还有其它设备的通道，半秒后执行切换
				if (siNode.Param) 
				{
					SetTimer(i, 500, NULL);
				}
		//		LeaveCriticalSection(&g_csCycle);
			}
			
			break;
		default:
			break;
		}
	}

	//删除查询文件列表中该设备的文件项
	POSITION nPos1 = g_ptrfilelist->GetHeadPosition();
	POSITION nPos2;
	int filecount = g_ptrfilelist->GetCount();
	for(i = 0; i < filecount; i++)
	{
		nPos2 = nPos1;
		pInfo = (FileInfoNode *)g_ptrfilelist->GetNext(nPos1);
		if(pInfo && pInfo->pDevice == pDev)
		{
			g_ptrfilelist->RemoveAt(nPos2);
			delete pInfo;
		}
	}
	CLIENT_Logout(pDev->LoginID);	
	if(bDelList)
	{
		CDevMgr::GetDevMgr().DelNode(pDev->LoginID);
	}
}

//断开当前选择的设备
void CNetSDKDemoDlg::DeleteDevice() 
{
	DeviceNode *nDev=(DeviceNode *)GetCurDeviceInfo();
	if(nDev == NULL)
	{
		return;
	}
	ProcessDeleteDevice(nDev, TRUE);
	UpdateDeviceList();
	UpdateCurScreenInfo();
	UpdateScreen(m_normalBtnPannel.GetSplit());
}

//拖动亮度控制条
void CNetSDKDemoDlg::CtrlColor_Bright(int pos)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	if(siNode.Type == SPLIT_TYPE_MONITOR)
	{	
		siNode.nVideoParam.bParam[0] = pos;
		ret = SetSplitInfo_Main(m_curScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
			return;
		}
	}
	UpdateVideoCtrl(VIDEO_BRIGHT);
}

//拖动对比度控制条
void CNetSDKDemoDlg::CtrlColor_Contrast(int pos)

{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	
	if(siNode.Type == SPLIT_TYPE_MONITOR)
	{
		siNode.nVideoParam.bParam[1] = pos;
		ret = SetSplitInfo_Main(m_curScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
			return;
		}
	}
	UpdateVideoCtrl(VIDEO_CONTRAST);
}

//拖动色度控制条
void CNetSDKDemoDlg::CtrlColor_Hue(int pos)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	if(siNode.Type == SPLIT_TYPE_MONITOR)
	{
		siNode.nVideoParam.bParam[2] = pos;
		ret = SetSplitInfo_Main(m_curScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
			return;
		}
	}
	UpdateVideoCtrl(VIDEO_HUE);	
}

//拖动饱和度控制条
void CNetSDKDemoDlg::CtrlColor_Saturation(int pos)
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	
	if(siNode.Type == SPLIT_TYPE_MONITOR)
	{
		siNode.nVideoParam.bParam[3] = pos;
		ret = SetSplitInfo_Main(m_curScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
			return;
		}
	}
	UpdateVideoCtrl(VIDEO_SATURATION);	
}

//更新设备状态
void CNetSDKDemoDlg::UpdateDeviceState(void *pDevice,  char *pBuf, DWORD dwBufLen)
{
}

//接收到实时数后需要做的处理
void CNetSDKDemoDlg::ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LLONG lParam/* = 0*/)
{
	//取得窗口号
	int nScreen = GetHandleSplit(lRealHandle);
	if(nScreen < 0)
	{
		return;
	}
	
	//码流统计

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(nScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	
	//保存实时数据
	switch(dwDataType) {
	case 0:
		if(siNode.SavecbFileRaw)
		{

			fwrite(pBuffer, 1, dwBufSize, siNode.SavecbFileRaw);
		}
		break;
	case 1:
		if(siNode.SavecbFileStd)
		{
			fwrite(pBuffer, 1, dwBufSize, siNode.SavecbFileStd);
		}
		break;
	case 2:
		if(siNode.SavecbFileYUV)
		{
			fwrite(pBuffer, 1, dwBufSize, siNode.SavecbFileYUV);
		}
		break;
	case 3:   //音频
		if(siNode.SavecbFilePcm)
		{
			fwrite(pBuffer, 1, dwBufSize, siNode.SavecbFilePcm);
		}
		break;
	default: break;
	}
}

//显示设备列表中当前选项的详细信息
void CNetSDKDemoDlg::DeviceState() 
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if (pDev)
	{
		m_ClientStateDlg.SetDevice(pDev);
		m_ClientStateDlg.StartRefresh();
		m_ClientStateDlg.ShowWindow(SW_SHOW);	
	}
}

//打开网络预览
void CNetSDKDemoDlg::OpenMultiplay() 
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}
	
	CPreviewParmsDlg dlg(this, pDev);
	INT_PTR iRet = dlg.DoModal();
	int channelid = 0;
	DH_RealPlayType rType;
	if (IDOK == iRet)
	{
		channelid = dlg.GetStartChannel();
		rType = dlg.GetRealPlayType();
	}
	else
	{
		return;
	}

	if(!CheckCurSplitAndClose())
	{
		return;
	}
	
	CWnd *plWnd = m_screenPannel.GetPage(m_curScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("!error!"));
		return;
	}

	LLONG nPlayID = CLIENT_RealPlayEx(pDev->LoginID, channelid, plWnd->m_hWnd, rType);
	
	if(!nPlayID)
	{
		LastError();
		AfxMessageBox(ConvertString(MSG_DEMODLG_NOPREVIEW));
		return;
	}
	else
	{
		//禁止语音对讲
		m_advanceBtnPannel.EnableTalk(FALSE);
	}

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
	}

	siNode.Type = SPLIT_TYPE_MULTIPLAY;
	siNode.iHandle = nPlayID;
	siNode.Param  = pDev;

	ret = SetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
	}

	UpdateCurScreenInfo();
	BOOL cbRec = CLIENT_SetRealDataCallBackEx2(nPlayID, RealDataCallBackEx, (LDWORD)this, 0x0f);
	if (!cbRec)
	{
		LastError();
	}
}

void CNetSDKDemoDlg::Sysconfig() 
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}

	MessageBox(ConvertString("Please use the Parameter Config Demo in Sort Application folder , this part of code is stopped maintenance"), ConvertString("prompt"));
	return;	
}

void CNetSDKDemoDlg::ptzctrl() 
{
	CPtzMenu dlg;

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	
	if(siNode.Type != SPLIT_TYPE_MONITOR)
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTOPENNED));
		return;
	}
	SplitMonitorParam *nParam = (SplitMonitorParam *)siNode.Param;
	LLONG iHandle = nParam->pDevice->LoginID;
	int iChannel = nParam->iChannel;

	dlg.SetPtzParam(iHandle, iChannel);
	dlg.DoModal();
}

void CNetSDKDemoDlg::Extptzctrl() 
{
	CExtPtzCtrl dlg;

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	if(siNode.Type != SPLIT_TYPE_MONITOR)
	{
		MessageBox(ConvertString(MSG_DEMODLG_NOTOPENNED));
		return;
	}
	SplitMonitorParam *nParam = (SplitMonitorParam *)siNode.Param;
	LLONG iHandle = nParam->pDevice->LoginID;
	int iChannel = nParam->iChannel;

	dlg.SetExtPtzParam(iHandle, iChannel);
	dlg.DoModal();
	
}

//打开透明串口页面
void CNetSDKDemoDlg::Transcom() 
{
	CTransCom dlg;

	DeviceNode *nDev = (DeviceNode *)GetCurDeviceInfo();
	if(nDev == NULL)
	{
		return;
	}
	dlg.SetDeviceId(nDev->LoginID);
	dlg.DoModal();
}


void CNetSDKDemoDlg::UpdateDevice() 
{
	DeviceNode *nDev = (DeviceNode *)GetCurDeviceInfo();
	if(nDev == NULL)
	{
		return;
	}
	CNetUpgrade dlg;
	dlg.SetDevice(nDev);
	dlg.DoModal();
}

//录像控制
void CNetSDKDemoDlg::Recordstate() 
{
	DeviceNode *nDev = (DeviceNode *)GetCurDeviceInfo();
	if(nDev == NULL)
	{
		return;
	}

	CRecordCtrlDlg dlg;
	dlg.SetDeviceId(nDev->LoginID);
	dlg.DoModal();
}

//Io控制
void CNetSDKDemoDlg::AlarmIOctrl() 
{
	DeviceNode *nDev = (DeviceNode *)GetCurDeviceInfo();
	if(nDev == NULL)
	{
		return;
	}
	CAlarmCtrlDlg dlg;

	dlg.SetDeviceId(nDev->LoginID);

	dlg.DoModal();
}



//语音对讲功能
/* Modified by yehao(10857) 2006-12-20*/
BOOL CNetSDKDemoDlg::OpenTalk(BOOL bOpen)
{
	/*Begin: yehao(10857) 2006-12-20*/
	MessageBox(ConvertString("Please use the Talk Demo in Sort Application folder , this part of code is stopped maintenance"), ConvertString("prompt"));
	return FALSE;	
}

/*Begin: Add by yehao(10857) 2006-12-20*/
LLONG CNetSDKDemoDlg::GetTalkHandle()
{
	return m_talkhandle.lHandle;
}

BOOL CNetSDKDemoDlg::RecordStart()
{
	BOOL bRet = FALSE;

	if (0 == m_uRecordCount)
	{
		bRet = CLIENT_RecordStart();
	}
	else
	{
		bRet = TRUE;
	}
	if (TRUE == bRet)
	{
		m_uRecordCount++;
	}
	return bRet;
}

BOOL CNetSDKDemoDlg::RecordStop()
{
	BOOL bRet = FALSE;

	if (m_uRecordCount > 0)
	{
		bRet = TRUE;
		m_uRecordCount--;
	}
	if (0 == m_uRecordCount)
	{
		bRet = CLIENT_RecordStop();
		if (FALSE == bRet)
		{
			m_uRecordCount++;
		}
	}
	return bRet;
}

void CNetSDKDemoDlg::AudioDataCallBack(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag, LDWORD dwUser)
{
	CNetSDKDemoDlg *pdlg = (CNetSDKDemoDlg *)dwUser;
	if (lTalkHandle != pdlg->GetTalkHandle())
	{
		return;
	}
	static int num = 0;
	char buf[100] = {0};
	LONG lRet = -1;
	switch (byAudioFlag)
	{
	case 0:
		lRet = CLIENT_TalkSendData(lTalkHandle, pDataBuf, dwBufSize);
		_snprintf(buf, sizeof(buf), ConvertString("current num is %d \n"), num);
		num++;
		if (lRet != dwBufSize)
		{
		}
		break;
	case 1:
		CLIENT_AudioDec(pDataBuf, dwBufSize);
		break;
	default:
		break;
	}
}
/*End: yehao(10857) */
//是否保存回调原始数据
void CNetSDKDemoDlg::SavecbdataRaw(int nCheck) 
{
	CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(m_curScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("we have a big error here!"));
		return;
	}
	
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	CString strName ;
	if(nCheck > 0)
	{
		strName.Format("cbdata %d.bin",m_curScreen);
		CFileDialog dlg(FALSE,"*.bin",strName.GetBuffer(0),OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"All File(*.bin)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{
			strName = dlg.GetPathName();
			FILE *file = fopen(strName.GetBuffer(0),"wb");
			if(!file)
			{
				MessageBox(ConvertString(MSG_DEMODLG_OPENFILEFAILED));			
			}
			plWnd->SetSplitCBFile_Raw(file);
		}
		else
		{
			return;
		}
	}
	else
	{
		if(siNode.SavecbFileRaw)
		{
			fclose(siNode.SavecbFileRaw);
			plWnd->SetSplitCBFile_Raw(NULL);
		}
	}
}

void CNetSDKDemoDlg::SavecbdataStd(int nCheck) 
{
	CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(m_curScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("we have a big error here!"));
		return;
	}
	
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	CString strName ;
	if(nCheck > 0)
	{
		strName.Format("cbdata %d.dav",m_curScreen);
		CFileDialog dlg(FALSE,"*.dav",strName.GetBuffer(0),OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"All File(*.dav)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{
			strName = dlg.GetPathName();
			FILE *file = fopen(strName.GetBuffer(0),"wb");
			if(!file)
			{
				MessageBox(ConvertString(MSG_DEMODLG_OPENFILEFAILED));		
			}
			plWnd->SetSplitCBFile_Std(file);
		}
		else
		{
			return;
		}
	}
	else
	{
		if(siNode.SavecbFileStd)
		{
			fclose(siNode.SavecbFileStd);
			plWnd->SetSplitCBFile_Std(NULL);
		}
	}
}

void CNetSDKDemoDlg::Savecbdatapcm(int nCheck) 
{
	CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(m_curScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("we have a big error here!"));
		return;
	}
	
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}

	CString strName ;
	if(nCheck > 0)
	{
		strName.Format("cbdata %d.pcm",m_curScreen);
		CFileDialog dlg(FALSE,"*.pcm",strName.GetBuffer(0),OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"All File(*.pcm)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{
			strName = dlg.GetPathName();
			FILE *file = fopen(strName.GetBuffer(0),"wb");
			if(!file)
			{
				MessageBox(ConvertString(MSG_DEMODLG_OPENFILEFAILED));			
			}
			plWnd->SetSplitCBFile_Pcm(file);
		}
		else
		{
			return;
		}
	}
	else
	{
		if(siNode.SavecbFilePcm)
		{
			fclose(siNode.SavecbFilePcm);
			plWnd->SetSplitCBFile_Pcm(NULL);
		}
	}
}

void CNetSDKDemoDlg::Savecbdatayuv(int nCheck) 
{
	CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(m_curScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("we have a big error here!"));
		return;
	}
	
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	
	CString strName ;
	if(nCheck > 0)
	{
		strName.Format("cbdata %d.yuv",m_curScreen);
		CFileDialog dlg(FALSE,"*.yuv",strName.GetBuffer(0),OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"All File(*.yuv)|*.*||",this);
		if(dlg.DoModal() == IDOK)
		{
			strName = dlg.GetPathName();
			FILE *file = fopen(strName.GetBuffer(0),"wb");
			if(!file)
			{
				MessageBox(ConvertString(MSG_DEMODLG_OPENFILEFAILED));		
			}
			plWnd->SetSplitCBFile_Yuv(file);
		}
		else
		{
			return;
		}
	}
	else
	{
		if(siNode.SavecbFileYUV)
		{
			fclose(siNode.SavecbFileYUV);
			plWnd->SetSplitCBFile_Yuv(NULL);
		}
	}
}

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	return TRUE; 
}

void CNetSDKDemoDlg::CycleMonitor()
{
	CCycleMonitor dlg;

	if (dlg.DoModal() == IDOK) 
	{
		SplitInfoNode siNode;
		memset(&siNode, 0, sizeof(siNode));

		//刷新画面，如果有画面被关闭了可以正常显示
		CPlayWnd *plWnd;
		for (int curWin = 0; curWin < DH_MAX_CHANNUM; curWin++)
		{
			plWnd = (CPlayWnd *)m_screenPannel.GetPage(curWin);
			if (plWnd)
			{
				plWnd->PostMessage(VIDEO_REPAINT);
			}			
		}

		for (int screenNo = 0; screenNo < DH_MAX_CHANNUM; screenNo++)
		{
			KillTimer(screenNo);
			
			BOOL ret = GetSplitInfo_Main(screenNo, &siNode);
			if (!ret)
			{
				MessageBox(ConvertString("error while getting split info"));
				return;
			}			

			//判断是否轮循类型
			if (siNode.Type == SPLIT_TYPE_CYCLEMONITOR)
			{
				LLONG nID = 0;
				int nCh = 0;

				if (siNode.Param)
				{
					POSITION pos = ((SplitCycleParam *)siNode.Param)->pChannelList->GetHeadPosition();
					CycleChannelInfo *tempnode = (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetNext(pos);

					DeviceNode *pDev = (DeviceNode *)tempnode->dwDeviceID;
					((SplitCycleParam *)siNode.Param)->pDevice = pDev;
					nID = pDev->LoginID;
					nCh = tempnode->iChannel;
				}								
				
				CWnd *plWnd = m_screenPannel.GetPage(screenNo);
				if (!plWnd)
				{
					MessageBox(ConvertString("!!error!!"));
					return;
				}
				
				//用户选择1个通道，相当于打开1个预览通道
				//test，无论通道多少，一视同仁
				if (FALSE)
				{	
					LLONG nChannelID = CLIENT_RealPlay(nID, nCh, plWnd->m_hWnd);
					if(!nChannelID)
					{
						LastError();
						KillTimer(screenNo);
						MessageBox(ConvertString(MSG_CYCLE_OPENCHANNELFAILED));
						if (siNode.Param) 
						{
							DeleteCycleParam(screenNo);
						}
						continue;
					}
					//设置轮循位置为第一个
					((SplitCycleParam *)siNode.Param)->iCurPosition = 0;
					//获取视频参数
					byte bVideo[4];
					BOOL nRet = CLIENT_ClientGetVideoEffect(nChannelID, &bVideo[0],&bVideo[1],&bVideo[2],&bVideo[3]);
					if (!nRet)
					{
						LastError();
					}
					siNode.nVideoParam.dwParam = *(DWORD *)bVideo;
					((CPlayWnd *)plWnd)->SetSplitHandle(nChannelID);
					continue;
				}
				//用户选择了2个或2个以上通道，开始轮循
				else
				{
					LLONG nChannelID = CLIENT_RealPlay(nID, nCh, plWnd->m_hWnd/*m_playWnd[screenNo].m_hWnd*/);
					if(!nChannelID)
					{
						LastError();
						KillTimer(screenNo);
						MessageBox(ConvertString(MSG_CYCLE_OPENCHANNELFAILED));
						if (siNode.Param) 
						{
							DeleteCycleParam(screenNo);
						}
						continue;
					}
					
					//设置轮循位置为第一个
					if (siNode.Param) 
					{
						((SplitCycleParam *)siNode.Param)->iCurPosition = 0;
					}					

					//获取视频参数
					byte bVideo[4];
					BOOL nRet = CLIENT_ClientGetVideoEffect(nChannelID, &bVideo[0],&bVideo[1],&bVideo[2],&bVideo[3]);
					if (!nRet)
					{
						LastError();
					}
					siNode.nVideoParam.dwParam = *(DWORD *)bVideo;
					((CPlayWnd *)plWnd)->SetSplitHandle(nChannelID);
					
					//设置轮循计时器，将秒转换为毫秒
					if (siNode.Param) 
					{
						UINT interval = ((SplitCycleParam *)siNode.Param)->iInterval;
						SetTimer((UINT)screenNo, interval * 1000, NULL);
					}
					
					UpdateCurScreenInfo();
				}
			}
		}//end of for (int screenNo = 0; screenNo < DH_MAX_CHANNUM; screenNo++)
	//LeaveCriticalSection(&g_csCycle);
	}
}

int WriteAlarmLogFunc(const AlarmNode& node, LDWORD dwUser)
{
	CNetSDKDemoDlg* pThis = (CNetSDKDemoDlg*)dwUser;
	if (!pThis)
	{
		return 1;
	}
	
	return pThis->WriteAlarmLogFunc_Imp(node);
}

int CNetSDKDemoDlg::WriteAlarmLogFunc_Imp(const AlarmNode& node)
{
	if (!m_almLogFile_Comm || !m_almLogFile_Shelter || !m_almLogFile_dFull
		|| !m_almLogFile_dError || !m_almLogFile_SoundDec) 
	{
		return 1;
	}

	CString strNewLog = "";
	CString strAlmTime;
	strAlmTime.Format(ConvertString("upgrade time:%d-%d-%d,%d:%d:%d\n"),
		node.timeStamp.GetYear(), node.timeStamp.GetMonth(), node.timeStamp.GetDay(),
		node.timeStamp.GetHour(), node.timeStamp.GetMinute(), node.timeStamp.GetSecond());
	strNewLog += strAlmTime;
	CString strAlmContent = (ConvertString("information:\n"));
	CString strTemp;
	BOOL isAlarm = FALSE; //确定是否有新的报警
	switch(node.alarmType)
	{
	case DH_COMM_ALARM:
		{
			//strAlmType.Format("报警类型：常规\n");
			//alarm input
			int i = 0;
			for(i = 0; i < 16; i++)
			{			
				if (node.state.cState.alarm[i] != m_lastAlarm.cState.alarm[i])
				{				
					if (m_lastAlarm.cState.alarm[i]) //alarm i is gone
					{
						strTemp.Format(ConvertString("Alarm end, alarm:  channel No.:%d\n"), i+1);
					}
					else //new alarm 
					{
						strTemp.Format(ConvertString("Alarm start, alram: channel No.:%d\n"), i+1);
					}
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
			}
			if (m_almLogFile_Comm && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_Comm);
				fflush(m_almLogFile_Comm);
			}

			//motion detect
			if (isAlarm)
			{
				strNewLog = "";
			}
			CString strAlmContent = "";
			isAlarm = FALSE;
			for (i = 0; i < 16; i++)
			{		
				if (node.state.cState.motiondection[i] != m_lastAlarm.cState.motiondection[i])
				{			
					if (m_lastAlarm.cState.motiondection[i]) //alarm i is gone
					{
						strTemp.Format(ConvertString("Alarm end, motion alarm: channel No.:%d\n"), i+1);
					}
					else //new alarm 
					{
						strTemp.Format(ConvertString("Alarm start, motion alarm: channel No.:%d\n"), i+1);
					}
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
			}
			if (m_almLogFile_Comm && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_Comm);
				fflush(m_almLogFile_Comm);
			}

			//video lost
			if (isAlarm)
			{
				strNewLog = "";
			}
			strAlmContent = "";
			isAlarm = FALSE;
			for (i = 0; i < 16; i++)
			{		
				if (node.state.cState.videolost[i] != m_lastAlarm.cState.videolost[i])
				{			
					if (m_lastAlarm.cState.videolost[i]) //alarm i is gone
					{
						strTemp.Format(ConvertString("Alarm end, video lost alarm: channel No.:%d\n"), i+1);
					}
					else //new alarm 
					{
						strTemp.Format(ConvertString("Alarm start, video lost alarm: channel No.:%d\n"), i+1);
					}
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
			}
			if (m_almLogFile_Comm && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_Comm);
				fflush(m_almLogFile_Comm);
			}
		}
		break;
	case DH_SHELTER_ALARM:
		{
			for (int i = 0; i < 16; i++)
			{
				if (node.state.shelter[i] != m_lastAlarm.shelter[i])
				{
					if (m_lastAlarm.shelter[i]) //alarm i is gone
					{
						strTemp.Format(ConvertString("Alarm end, shelter alarm: channel No.:%d\n"), i+1);
					}
					else //new alarm 
					{
						strTemp.Format(ConvertString("Alarm start, shelter alarm: channel No.:%d\n"), i+1);
					}
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
			}
			if (m_almLogFile_Shelter && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_Shelter);
				fflush(m_almLogFile_Shelter);
			}
		}
		break;
	case DH_DISK_FULL_ALARM:
		{		
			{			
				if (node.state.dFull) //disk full
				{
					strTemp.Format(ConvertString("Alarm, Disk full\n"));
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
				else //disk full alarm is gone
				{
				
				}
			}
			if (m_almLogFile_dFull && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_dFull);
				fflush(m_almLogFile_dFull);
			}
		}
		break;
	case DH_DISK_ERROR_ALARM:
		{
			for (int i = 0; i < 32; i++)
			{
				DWORD dwDE = node.state.dError ^ m_lastAlarm.dError;
				if (dwDE & (0x01<<i))
				{
					if (m_lastAlarm.dError & (0x01<<i)) //disk error alarm is gone
					{
						strTemp.Format(ConvertString("Alarm end, Disk error alarm %d\n"), i+1);
					}
					else
					{
						strTemp.Format(ConvertString("Alarm start, Disk error alarm %d\n"), i+1);
					}
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
			}
			if (m_almLogFile_dError && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_dError);
				fflush(m_almLogFile_dError);
			}
		}
		break;
	case DH_SOUND_DETECT_ALARM:
		{
			for (int i = 0; i < 16; i++)
			{
				if (node.state.soundalarm[i] != m_lastAlarm.soundalarm[i])
				{
					if (m_lastAlarm.soundalarm[i]) //alarm i is gone
					{
						strTemp.Format(ConvertString("Alarm end, sound detect alarm: channel No.:%d\n"), i+1);
					}
					else //new alarm 
					{
						strTemp.Format(ConvertString("Alarm start, sound detect alarm:channel No.:%d\n"), i+1);
					}
					isAlarm = TRUE;
					strAlmContent += strTemp;
				}
			}
			if (m_almLogFile_SoundDec && isAlarm)
			{
				strNewLog += strAlmContent + "\n";
				fwrite(strNewLog.GetBuffer(0), strNewLog.GetLength(), 1, m_almLogFile_SoundDec);
				fflush(m_almLogFile_SoundDec);
			}
		}
		break;
	default:
		return 1;
	}
	
	memcpy(&m_lastAlarm, &node.state, sizeof(DEV_STATE));
	return 0;
}

void CNetSDKDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	//刷新播放放进度条
	CDialog::OnTimer(nIDEvent);
	
	if (nIDEvent <= CUR_MAXCHAN)
	{
		NextCycleMonitor(nIDEvent);
	}
	else if (nIDEvent == ALARMLOG) 
	{
		CDevMgr::GetDevMgr().For_EachAlmNode(WriteAlarmLogFunc, (LDWORD)this);	
	}
	else if (nIDEvent == UPDATATREE)
	{
		if (g_bUpdataTree)
		{
			g_bUpdataTree = FALSE;

			UpdateDeviceList();
		}
	}
}


void CNetSDKDemoDlg::Getversion() 
{
	CString strCptn;
	GetWindowText(strCptn);

	LONG lVer = CLIENT_GetSDKVersion();

	CString strVerTmp, strVer;
	strVerTmp.Format("%ld", lVer);
	strVer = strVerTmp.Left(1) + "." + strVerTmp.Right(strVerTmp.GetLength()-1).Left(2)+"."+strVerTmp.Right(strVerTmp.GetLength()-3);
	strVer = ConvertString(MSG_VERSION) + strVer;
	strCptn += strVer;
	
	SetWindowText(strCptn);
}

/*Begin: Add by yehao(10857) For Task.NO.11071 2006-12-23*/
void CNetSDKDemoDlg::OpenAudioBroadcastDlg()
{
	DeviceNode *ptmpDevNode = NULL;
	HTREEITEM tmphandle = m_devicelist.GetRootItem();
	int num = 0;
	m_audioBroadcastDlg.m_AllDevList.clear();
	m_audioBroadcastDlg.m_BroadcastDevList.clear();
	while (tmphandle)
	{
		ptmpDevNode = (DeviceNode *)m_devicelist.GetItemData(tmphandle);
		m_audioBroadcastDlg.m_AllDevList.push_back(ptmpDevNode);
		tmphandle = m_devicelist.GetNextItem(tmphandle, TVGN_NEXT);
	}
	
	list<DeviceNode *>::iterator it = m_broadcastDevList.begin();
	for (; it != m_broadcastDevList.end(); it++)
	{
		list<DeviceNode *>::iterator itt = m_audioBroadcastDlg.m_AllDevList.begin();
		for (; itt != m_audioBroadcastDlg.m_AllDevList.end(); itt++)
		{
			if (LLONG(*it) == LLONG(*itt))
			{
				m_audioBroadcastDlg.m_BroadcastDevList.push_back((*it));
				m_audioBroadcastDlg.m_AllDevList.erase(itt);
				break;
			}
		}
	}
	m_audioBroadcastDlg.m_bRecord = m_bRecord;
	int iRet = m_audioBroadcastDlg.DoModal();

	if (IDOK == iRet)
	{
		if (m_bRecord != m_audioBroadcastDlg.m_bRecord)
		{
			m_bRecord = m_audioBroadcastDlg.m_bRecord;
			if (m_bRecord)
			{
				RecordStart();
			}
			else
			{
				RecordStop();
			}
		}
		BOOL find = FALSE;
		{
			list<DeviceNode *>::iterator it = m_broadcastDevList.begin();
			for (; it != m_broadcastDevList.end(); it++)
			{
				find = FALSE;
				list<DeviceNode *>::iterator itt = m_audioBroadcastDlg.m_BroadcastDevList.begin();
				for (; itt != m_audioBroadcastDlg.m_BroadcastDevList.end(); itt++)
				{
					if (LLONG(*it) == LLONG(*itt))
					{
						find = TRUE;
						break;
					}
				}
				if (FALSE == find)
				{
					CLIENT_AudioBroadcastDelDev((*it)->LoginID);
				}
			}
		}

		{
			list<DeviceNode *>::iterator it = m_audioBroadcastDlg.m_BroadcastDevList.begin();
			for (; it != m_audioBroadcastDlg.m_BroadcastDevList.end(); it++)
			{
				find = FALSE;
				list<DeviceNode *>::iterator itt = m_broadcastDevList.begin();
				for (; itt != m_broadcastDevList.end(); itt++)
				{
					if (LLONG(*it) == LLONG(*itt))
					{
						find = TRUE;
						break;
					}
				}
				if (FALSE == find)
				{
					CLIENT_AudioBroadcastAddDev((*it)->LoginID);
				}
			}
		}

		{
			m_broadcastDevList.clear();
			list<DeviceNode *>::iterator it = m_audioBroadcastDlg.m_BroadcastDevList.begin();
			for (; it != m_audioBroadcastDlg.m_BroadcastDevList.end(); it++)
			{
				m_broadcastDevList.push_back(*it);
			}
		}
	}
	else
	{
	}

	return;
}
/*End: yehao(10857) Task.NO.11071 */
HBRUSH CNetSDKDemoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (pWnd->GetDlgCtrlID() == IDC_TREE_DEVICELIST)
	{
		return m_myBrush;
	}
	return hbr;
}

void CNetSDKDemoDlg::Sysconfig2() 
{
	DeviceNode *pDev;
	In_DeviceInfo m_di;
	CSystemConfig nSyscfgdlg;
	
	pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}

	m_di.lDeviceID = pDev->LoginID;
	memcpy(m_di.cDeviceName , pDev->Name, 20);
	m_di.unAlarmInNum = pDev->Info.nAlarmInPortNum;
	m_di.unAlarmOutNum = pDev->Info.nAlarmOutPortNum;
	m_di.unChannelNum = pDev->Info.nChanNum;
	m_di.unTypeIndex = (pDev->Info.nDVRType == NET_DVR_MPEG4_NVSII) ? 2 : 1;
	m_di.unVideoCodeType = 8;
	m_di.unNetType = 1;
	
	nSyscfgdlg.setDeviceId(pDev->LoginID);
	nSyscfgdlg.ShowDefModal(m_di);
}

void CNetSDKDemoDlg::Playbackbytime() 
{
	CPlayBackByTime dlg;
showBTwindow:
	if (dlg.DoModal() == IDOK) 
	{
		if(!CheckCurSplitAndClose())
		{
			MessageBox(ConvertString("Error: Cannot close current screen!"));
			return;
		}		
		PlayBackByTimeInfo *tmpinfo = dlg.GetPlayBackInfo();
		
		NET_RECORDFILE_INFO ri;
		ZeroMemory(&ri, sizeof(ri));
		ri.starttime = tmpinfo->starttime;
		ri.endtime   = tmpinfo->endtime;	
		
		CWnd *plWnd = m_screenPannel.GetPage(m_curScreen);
		if (!plWnd)
		{
			MessageBox(ConvertString("!!error!!"));
			return;
		}

		LLONG lPlayID = CLIENT_PlayBackByTimeEx(tmpinfo->pDevice->LoginID, tmpinfo->nChannel, 
			&tmpinfo->starttime, &tmpinfo->endtime, plWnd->m_hWnd, 
			PlayCallBack, (LDWORD)this,PBDataCallBack, (LDWORD)this);
		
		if(!lPlayID)
		{
			LastError();
			MessageBox(ConvertString(MSG_DEMODLG_PLAYFAILED));
			goto showBTwindow;
		}
		else
		{  //如果其它通道没有打开音频，则打开音频
			if (m_curSoundSplit < 0)
			{
				if (FALSE == CLIENT_OpenSound(lPlayID)) 
				{
					LastError();
					MessageBox(ConvertString(MSG_OPENSOUNDFAILED));
					m_advanceBtnPannel.SetCheckSound(0);
				}
				else
				{
					m_advanceBtnPannel.SetCheckSound(1);
					m_curSoundSplit = m_curScreen;
				}
			}
		}

		SplitInfoNode siNode;
		memset(&siNode, 0, sizeof(siNode));
		BOOL ret = GetSplitInfo_Main(m_curScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while getting split info"));
		}

		siNode.Type = SPLIT_TYPE_PBBYTIME;
		siNode.iHandle = lPlayID;

		SplitPBByTimeParam *nparam	= new SplitPBByTimeParam;
		nparam->pDevice = tmpinfo->pDevice;
		nparam->nChannel = tmpinfo->nChannel;
		nparam->starttime = tmpinfo->starttime;
		nparam->endtime = tmpinfo->endtime;

		siNode.Param  = nparam;

		ret = SetSplitInfo_Main(m_curScreen, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while setting split info"));
		}

		UpdateCurScreenInfo();
	}
}

void CNetSDKDemoDlg::Downloadbytime() 
{
	m_dbByTime.CenterWindow();
	m_dbByTime.ShowWindow(SW_SHOW);
}

void CNetSDKDemoDlg::RebootDevice() 
{
	DeviceNode *nDev = (DeviceNode *)GetCurDeviceInfo();
	if (NULL == nDev)
	{
		return;
	}
	BOOL bRet = CLIENT_RebootDev(nDev->LoginID);
	if (!bRet)
	{
		LastError();
		MessageBox(ConvertString(MSG_DEMODLG_REBOOTFAILED));
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_REBOOTDONE));
	}
}

void CNetSDKDemoDlg::ShutdownDevice() 
{
	DeviceNode *nDev = (DeviceNode *)GetCurDeviceInfo();
	if (NULL == nDev)
	{
		return;
	}
	BOOL bRet = CLIENT_ShutDownDev(nDev->LoginID);
	if (!bRet)
	{
		LastError();
		MessageBox(ConvertString(MSG_DEMODLG_SHUTDOWNFAILED));
	}
	else
	{
		MessageBox(ConvertString(MSG_DEMODLG_SHUTDOWNDONE));
	}
}


void CNetSDKDemoDlg::DDNS_QueryIP() 
{
	CDDNS_QueryIP dlg;
	dlg.DoModal();
}

void CNetSDKDemoDlg::OnDblclkTreeDevicelist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OpenChannel();
	*pResult = 0;
}

void CNetSDKDemoDlg::OpenAllChannel(DeviceNode *pInfo, DH_RealPlayType subtype) 
{
	LONG nChannelID = 0;

	ProcessCloseAllChannel(pInfo);
	
	LLONG nID = pInfo->LoginID;
	int chNum = pInfo->Info.nChanNum;
	
	int ScreenNum = CUR_MAXCHAN;//本SDK现在所能显示的最大画面数
	int FreeScreen = 0;
	DWORD freeFlag = 0;

	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));

	if(chNum > CUR_MAXCHAN)//设备返回通道数>SDK显示通道数
	{
		for(int ii = 0 ; ii < CUR_MAXCHAN ; ii++)
    	{
			if (ProcessCloseScreen(ii)) 
			{
				freeFlag |= (0x00000001 << ii);
			}
		}
		FreeScreen = CUR_MAXCHAN;
		UpdateCurScreenInfo();
	}
	else 
	{
		for(int ii = 0 ; ii< CUR_MAXCHAN; ii++)
		{
			BOOL ret = GetSplitInfo_Main(ii, &siNode);
			if (!ret)
			{
				MessageBox(ConvertString("error while getting split info"));
			}
			if(SPLIT_TYPE_NULL == siNode.Type)
			{
				freeFlag |= (0x00000001 << ii);
				FreeScreen ++;
			}
		}
		if((chNum > FreeScreen))//空余画面数<设备返回通道数
		{
			int killNum = chNum - FreeScreen;
			for (int ii = 0 ; ii < CUR_MAXCHAN; ii ++)
			{		
				if(killNum && !(freeFlag & (0x00000001 << ii)))
				{
					if (ProcessCloseScreen(ii))
					{
						freeFlag |= (0x00000001 << ii);
						FreeScreen ++;
					}
					killNum --;
				}
			}
			UpdateCurScreenInfo();
		}
	}

	int nCh = 0;
	for (int nScrn = 0; nScrn < CUR_MAXCHAN; nScrn++)
	{
		if (((freeFlag >> nScrn)& 0x00000001) && (nCh < FreeScreen) && (nCh < chNum))
		{
			OpenSingleChannel(pInfo, nCh, nScrn, subtype);
			nCh++;
		}
	}
}

BOOL CNetSDKDemoDlg::ProcessCloseAllChannel(DeviceNode *pInfo) 
{
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));

	LLONG nID = pInfo->LoginID;
	LLONG curID = 0;
	BOOL ret = TRUE;
	for(int i = 0 ; i < CUR_MAXCHAN; i++)
    {
		BOOL ret = GetSplitInfo_Main(i, &siNode);
		if (!ret)
		{
			MessageBox(ConvertString("error while getting split info"));
			return FALSE;
		}

		if(SPLIT_TYPE_NULL == siNode.Type)
		{
			continue;
		}
		else if(SPLIT_TYPE_MONITOR == siNode.Type)
		{
			SplitMonitorParam *mParam = (SplitMonitorParam *)siNode.Param;
			curID = mParam->pDevice->LoginID;
		}
		else if (SPLIT_TYPE_NETPLAY == siNode.Type)
		{
			SplitNetPlayParam *nParam = (SplitNetPlayParam *)siNode.Param; 
			curID = nParam->pFileInfo->pDevice->LoginID;
		}
		else if (SPLIT_TYPE_MULTIPLAY == siNode.Type)
		{
			curID = ((DeviceNode *)siNode.Param)->LoginID;
		}
		else if (SPLIT_TYPE_CYCLEMONITOR == siNode.Type)
		{
			SplitCycleParam *mParam = (SplitCycleParam *)siNode.Param;
			curID = mParam->pDevice->LoginID;
		}
		else if (SPLIT_TYPE_PBBYTIME == siNode.Type)
		{
			SplitPBByTimeParam *mParam = (SplitPBByTimeParam *)siNode.Param;
			curID = mParam->pDevice->LoginID;
		}
		
		if (nID == curID)
		{
			if (!ProcessCloseScreen(i))
			{
				ret = FALSE;
			}
			else
			{
				CPlayWnd *plWnd = (CPlayWnd *)m_screenPannel.GetPage(i);
				if (!plWnd)
				{
					MessageBox(ConvertString("!!big error!!"));
					return FALSE;
				}
				plWnd->PostMessage(VIDEO_REPAINT);
			}
		}
		curID = 0;
	}
	UpdateCurScreenInfo();
	
	return ret;
}

void CNetSDKDemoDlg::SwitchFullScreen() 
{
	m_screenPannel.SetFullScreen(!m_screenPannel.GetFullScreen());
}



//打开轮循列表中下一个通道
void CNetSDKDemoDlg::NextCycleMonitor(UINT nID)	
{
	UINT nScreen = nID;
	SplitInfoNode siNode;
	memset(&siNode, 0, sizeof(siNode));
	BOOL ret = GetSplitInfo_Main(nScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while getting split info"));
		return;
	}
	if (siNode.Type != SPLIT_TYPE_CYCLEMONITOR || NULL == siNode.Param)
	{
		return;
	}

	//取轮循列表的下一个通道值
	CycleChannelInfo *tempnode;
	POSITION nPos = ((SplitCycleParam *)siNode.Param)->pChannelList->GetHeadPosition();
	tempnode = (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetNext(nPos);
	int nextPosition = ((SplitCycleParam *)siNode.Param)->iCurPosition + 1;
	//如果已经到了列表末尾，则返回列表头
	if (nextPosition >= ((SplitCycleParam *)siNode.Param)->iChannelCount)
	{
		nextPosition = 0;
	}
	else	
	{
		//否则取下一个
		for (int x = 0; x < nextPosition; x++)
		{
			tempnode = (CycleChannelInfo *)((SplitCycleParam *)siNode.Param)->pChannelList->GetNext(nPos);
		}
	}
		
	if (siNode.iHandle != 0) 
	{
		CLIENT_StopRealPlay(siNode.iHandle);
	}

	DeviceNode *pDev = (DeviceNode *)(tempnode->dwDeviceID);
	((SplitCycleParam *)siNode.Param)->pDevice = pDev;	
	int nCh = tempnode->iChannel;

	CWnd *plWnd = m_screenPannel.GetPage(nScreen);
	if (!plWnd)
	{
		MessageBox(ConvertString("fatal error!"));
		return;
	}

	LLONG nChannelID = CLIENT_RealPlay(pDev->LoginID, nCh, plWnd->m_hWnd);

	//设置当前轮循位置
	((SplitCycleParam *)siNode.Param)->iCurPosition = nextPosition;

	//获取视频参数
	byte bVideo[4];
	BOOL nRet = CLIENT_ClientGetVideoEffect(nChannelID, &bVideo[0],&bVideo[1],&bVideo[2],&bVideo[3]);
	if (!nRet)
	{
		LastError();
	}
	siNode.nVideoParam.dwParam = *(DWORD *)bVideo;
	siNode.iHandle = nChannelID;

	ret = SetSplitInfo_Main(nScreen, &siNode);
	if (!ret)
	{
		MessageBox(ConvertString("error while setting split info"));
	}

	//设置轮循计时器
	UINT interval = ((SplitCycleParam *)siNode.Param)->iInterval;
	SetTimer((UINT)nScreen, interval * 1000, NULL);

	return;
}

void CNetSDKDemoDlg::CloseAllChannel() 
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if (NULL == pDev)
	{
		return;
	}
	ProcessCloseAllChannel(pDev);
}

void CNetSDKDemoDlg::SeleteNormalPannel()			//显示“常规功能”页面
{
	m_normalBtnPannel.ShowWindow(SW_SHOW);
	m_advanceBtnPannel.ShowWindow(SW_HIDE);
	m_ptzPannel.ShowWindow(SW_HIDE);
}

void CNetSDKDemoDlg::SeleteAdvancePannel()			//显示“高级”功能页面
{
	m_advanceBtnPannel.ShowWindow(SW_SHOW);
	m_normalBtnPannel.ShowWindow(SW_HIDE);
	m_ptzPannel.ShowWindow(SW_HIDE);
}

void CNetSDKDemoDlg::SeletePTZPannel()				//显示“云台控制”功能页面
{
	m_ptzPannel.ShowWindow(SW_SHOW);
	m_advanceBtnPannel.ShowWindow(SW_HIDE);
	m_normalBtnPannel.ShowWindow(SW_HIDE);
}

void CNetSDKDemoDlg::SeleteSaveDataPannel()		//显示“数据保存”功能页面
{
	m_devicelist.ShowWindow(SW_HIDE);
}

void CNetSDKDemoDlg::SeleteColorPannel()			//显示“颜色调整”功能页面
{
	m_devicelist.ShowWindow(SW_HIDE);
}

void CNetSDKDemoDlg::SeletePlayCtrlPannel()		//显示“播放控制”功能页面
{
	m_devicelist.ShowWindow(SW_HIDE);	
}

void CNetSDKDemoDlg::SeleteDevListPannel()			//显示“设备列表”功能页面
{
	m_devicelist.ShowWindow(SW_SHOW);	
}

//窗口改变时的处理
void CNetSDKDemoDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	//窗口最小化与窗口大小无变化不处理
	if ((cx ==0 && cy == 0) || 
		(cx == m_clientRect.Width() && cy == m_clientRect.Height())) 
	{
		return;
	}
	else
	{
		UpdatePannelPosition();
		UpdateScreen(m_normalBtnPannel.GetSplit());
		Invalidate();
	}
}

void CNetSDKDemoDlg::UpdatePannelPosition()
{
	GetClientRect(&m_clientRect);

	m_screenRect.top = m_clientRect.top/* + 5*/;
	m_screenRect.bottom = m_clientRect.bottom/* - 20*/;
	m_screenRect.left = m_clientRect.left/* + 10*/;
	m_screenRect.right = m_clientRect.right - 210;
	GetDlgItem(IDC_SCREEN_WINDOW)->MoveWindow(m_screenRect);
	
	m_pannelRect.top = m_clientRect.top + 100;
	m_pannelRect.bottom = m_clientRect.bottom - 263;
	m_pannelRect.right = m_clientRect.right - 2;
	m_pannelRect.left = m_pannelRect.right - 197;
	GetDlgItem(IDC_FRAME_PANNEL)->MoveWindow(m_pannelRect);	
	
	m_selectRect.left = m_pannelRect.left;
	m_selectRect.right = m_pannelRect.right;
	m_selectRect.top = m_pannelRect.bottom;
	m_selectRect.bottom = m_selectRect.top + 35;
	
	m_btnAreaRect.left = m_selectRect.left;
	m_btnAreaRect.right = m_selectRect.right;
	m_btnAreaRect.top = m_selectRect.bottom;
	m_btnAreaRect.bottom = m_btnAreaRect.top + 216;
	GetDlgItem(IDC_FRAME_BTN_AREA)->MoveWindow(m_btnAreaRect);	
	
	m_runtimeMsgRect.left = m_pannelRect.left;
	m_runtimeMsgRect.right = m_pannelRect.right;
	m_runtimeMsgRect.top = m_pannelRect.top - 95;
	m_runtimeMsgRect.bottom = m_runtimeMsgRect.top + 90;
	
	m_selectPannel.MoveWindow(m_selectRect);	
	m_devicelist.MoveWindow(m_pannelRect); //设备列表，非子窗口
	m_normalBtnPannel.MoveWindow(m_btnAreaRect);
	m_advanceBtnPannel.MoveWindow(m_btnAreaRect);
	m_ptzPannel.MoveWindow(m_btnAreaRect);
	m_runtimeMsgPannel.MoveWindow(m_runtimeMsgRect);
	m_screenPannel.MoveWindow(m_screenRect);
}

void CNetSDKDemoDlg::LastError()
{
	m_runtimeMsgPannel.ShowLastError();
}

BOOL CNetSDKDemoDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (WM_KEYDOWN == pMsg->message && 
		(VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam))
	{
		if (VK_ESCAPE == pMsg->wParam && m_screenPannel.GetFullScreen())
		{
			m_screenPannel.SetFullScreen(FALSE);
		}
		return TRUE;
	}

	if (WM_SIZING == pMsg->message)
	{
		RECT* rc = (RECT*)(pMsg->lParam);
		if (rc)
		{		
			CRect rect(rc);
		}

	}



	return CDialog::PreTranslateMessage(pMsg);
}

void CNetSDKDemoDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	m_selectPannel.DestroyWindow();
	m_screenPannel.DestroyWindow();
	m_devicelist.DestroyWindow();
	m_advanceBtnPannel.DestroyWindow();
	m_ptzPannel.DestroyWindow();
	m_runtimeMsgPannel.DestroyWindow();

	CLIENT_Cleanup();
}

void CNetSDKDemoDlg::SwitchMultiWnd(int nSplit)
{
	if (SPLIT1 == nSplit)
	{
		m_screenPannel.SetMultiScreen(FALSE);
		return ;
	}
	else
	{
		m_screenPannel.SetMultiScreen(TRUE);
		m_screenPannel.SetShowPlayWin(nSplit, m_curScreen);
	}	
}


int	 CNetSDKDemoDlg::GetCurWinID(void)
{
	CPlayWnd* pWnd = (CPlayWnd*)m_screenPannel.GetActivePage();
	if (pWnd && ::IsWindow(pWnd->GetSafeHwnd()))
	{
		return pWnd->GetWinID();
	}
	
	return -1;
}

LRESULT CNetSDKDemoDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DefWindowProc(message, wParam, lParam);
}

BOOL CNetSDKDemoDlg::GetSplitInfo_Main(int nIndex, SplitInfoNode* info)
{
	if (nIndex < 0 || nIndex > DH_MAX_CHANNUM)
	{
		return FALSE;
	}
	CPlayWnd *plWnd = (CPlayWnd*)m_screenPannel.GetPage(nIndex);
	if (!plWnd)
	{
		MessageBox(ConvertString("oops!"));
		return FALSE;
	}

	return plWnd->GetSplitInfo(info);
}

BOOL CNetSDKDemoDlg::SetSplitInfo_Main(int nIndex, SplitInfoNode* info)
{
	if (nIndex < 0 || nIndex > DH_MAX_CHANNUM)
	{
		return FALSE;
	}
	
	CPlayWnd *plWnd = (CPlayWnd*)m_screenPannel.GetPage(nIndex);
	if (!plWnd)
	{
		MessageBox(ConvertString("oops!"));
		return FALSE;
	}
	
	return plWnd->SetSplitInfo(info);
}

void CNetSDKDemoDlg::SetSplit(int split)
{
	m_normalBtnPannel.SetSplit(split);
}

int CNetSDKDemoDlg::GetSplit()
{
	return m_normalBtnPannel.GetSplit();
}

void CNetSDKDemoDlg::GetDeviceWorkstate()			//"设备工作状态"按键响应函数
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}

	CDeviceWorkState dlg;
	
	dlg.SetDevice(pDev);
	
	dlg.DoModal();

}

void CNetSDKDemoDlg::DeviceControldisk()			//"硬盘控制"按键响应函数
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}
	
	CDiskControl dlg;
	dlg.SetDeviceId(pDev);
	dlg.DoModal();	
}

void CNetSDKDemoDlg::DeviceUserinfo()				//"用户信息"按键响应函数
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}

	CUserManage dlg;
	dlg.SetDeviceId(pDev);
	dlg.DoModal();
}


void CNetSDKDemoDlg::TestProc()
{
	DeviceNode *pDev = (DeviceNode *)GetCurDeviceInfo();
	if(pDev == NULL)
	{
		return;
	}

	DHDEV_CHANNEL_CFG stuInfo = {0};
	DWORD nRetLen = 0;
	BOOL bret = CLIENT_GetDevConfig(pDev->LoginID, DH_DEV_CHANNELCFG, -1, &stuInfo, sizeof(DHDEV_CHANNEL_CFG), &nRetLen);
	if (bret)
	{
		stuInfo.stMainVideoEncOpt->byEncodeMode = 7;
		stuInfo.stMainVideoEncOpt->bH264ProfileRank = 2;
		bret = CLIENT_SetDevConfig(pDev->LoginID, DH_DEV_CHANNELCFG, 0, &stuInfo, sizeof(DHDEV_CHANNEL_CFG));
		if (bret)
		{
			int i =0;
		}
	}
}
