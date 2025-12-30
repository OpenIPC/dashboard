// ActiveLoginDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveLoginDemo.h"
#include "ActiveLoginDemoDlg.h"
#include "NameDlg.h"
#include "DeviceInfo.h"
#include <process.h>
#include <algorithm>
#include "stdlib.h"

// #ifdef _DEBUG
// #define new DEBUG_NEW
// #undef THIS_FILE
// static char THIS_FILE[] = __FILE__;
// #endif

DWORD  WINAPI LoginThreadProc(LPVOID lpParam)
{
    CActiveLoginDemoDlg *pThis = (CActiveLoginDemoDlg*)lpParam;
    if (pThis == NULL)
    {
        return NULL;
    }
    if(CoInitialize(NULL)!= 0)
    {
        AfxMessageBox(ConvertString("Initialization Com Fail!",DLG_MAIN));
        return 0;
    }
    BOOL bSignal = TRUE;
    int nAsynSpanTime = 0;
    int nTimeCount = 0;
    while (TRUE)
    {
        if (pThis->m_bExit)
        {
            break;
        }
        if (bSignal)
        {
            pThis->InitExcel();
            bSignal = FALSE;
			pThis->m_bInitExcel = TRUE;
			pThis->GetDlgItem(IDC_STARTSERVER)->EnableWindow(TRUE);
        }
		else if (pThis->m_bDeleteDev)
		{
			pThis->DeleteDev();
		}
        else if (pThis->m_bSort)
        {
            pThis->Sort();
        }
		else if (pThis->m_bDirSort)
		{
			pThis->DirSort();
		}
		else if (pThis->m_bSearch)
		{
			pThis->Search();
		}
        else
        {
            pThis->LoginThread();
        }
        Sleep(1);
    }
	SetEvent(pThis->m_hEvent);
//	pThis->ClearRecourse();
    CoUninitialize();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoDlg dialog

CActiveLoginDemoDlg::CActiveLoginDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CActiveLoginDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CActiveLoginDemoDlg)
	m_LocalPort = 9500;
	m_strSearch = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_hThread = 0;
    m_bExit = FALSE;
    m_bSort = FALSE;
	m_bDirSort = FALSE;
	m_bDeleteDev = FALSE;
	m_bSearch =FALSE;
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_lListenHdl = 0;
    m_CurScreen = 0;
    m_lLoginID = 0;
    m_nChannel = 0;
	m_bInitExcel = FALSE;
    for(int i=0;i<9;i++)
    {
        m_DispHanle[i]=0;
        memset(&m_stuVideoInfo[i], 0, sizeof(m_stuVideoInfo[i]));
        memset(&m_stuPlayInfo[i], 0, sizeof(m_stuPlayInfo[i]));
	}
	dwThreadID = 0;
	m_bSorted = FALSE;
	m_nShowType = 0;
}

void CActiveLoginDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CActiveLoginDemoDlg)
	DDX_Control(pDX, IDC_STATIC_INFO, m_StaticInfo);
	DDX_Control(pDX, IDC_CMB_SPLIT, m_Cmb_Split);
	DDX_Control(pDX, IDC_COMBO_SHOWTYPE, m_Cmb_ShowType);
	DDX_Control(pDX, IDC_TAB_DLG, m_CtlTab);
	DDX_Control(pDX, IDC_LOCALIPADDRESS, m_LocalAddr);
	DDX_Control(pDX, IDC_DEVTREE, m_DevTree);
	DDX_Text(pDX, IDC_EDIT_LOCALPORT, m_LocalPort);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_strSearch);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CActiveLoginDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CActiveLoginDemoDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_STARTSERVER, OnStartserver)
	ON_BN_CLICKED(IDC_STOPSERVER, OnStopserver)
    ON_MESSAGE(WM_DEVICE_DISCONNECTED, OnDeviceDisconnected)
	ON_MESSAGE(WM_LOGINSUCCESS, OnLoginSuccess)
	ON_MESSAGE(WM_LOGIN_DISCONNECTED, OnLoginDisconnected)
	ON_NOTIFY(NM_CLICK, IDC_DEVTREE, OnClickDevtree)
	ON_NOTIFY(NM_DBLCLK, IDC_DEVTREE, OnDblclkDevtree)
	ON_NOTIFY(NM_RCLICK, IDC_DEVTREE, OnRclickDevtree)
	ON_CBN_SELCHANGE(IDC_COMBO_SHOWTYPE, OnSelchangeComboShowtype)
	ON_BN_CLICKED(IDC_BTN_SEARCH, OnBtnSearch)
	ON_BN_CLICKED(IDC_BTN_STOPALL, OnBtnStopall)
	ON_CBN_SELCHANGE(IDC_CMB_SPLIT, OnSelchangeCmbSplit)
	//}}AFX_MSG_MAP
    ON_COMMAND_RANGE(VIDEO_MENU_BASE, VIDEO_MENU_END, OnTreeMenu)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_HELP, &CActiveLoginDemoDlg::OnBnClickedHelp)
END_MESSAGE_MAP()

CActiveLoginDemoDlg::~CActiveLoginDemoDlg()
{

}
/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoDlg message handlers

void CALLBACK DemoDisConnectCallBack(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    if(0 != dwUser)
    {
		 CActiveLoginDemoDlg *dlg = (CActiveLoginDemoDlg *)dwUser;
	     
		 // 在该线程由SDK内部维护，弹窗将阻塞线程，应当异步传递消息
		 DisConnectionInfo* stuDisInfo = NEW DisConnectionInfo;
		 stuDisInfo->lLoginID = lLoginID;
		 stuDisInfo->nPort = nDVRPort;
		 stuDisInfo->strIp = pchDVRIP;
		 dlg->PostMessage(WM_DEVICE_DISCONNECTED, (WPARAM)stuDisInfo, 0);
    }
}

void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser)
{
//  	CString str;
//  	str.Format("PlayHand:%ld", lRealHandle);
//  	PRINT_OPERATE(str);
//  	str.Format("BufSize:%ld", dwBufSize);
//  	PRINT_OPERATE(str);
}
LRESULT CActiveLoginDemoDlg::OnDeviceDisconnected(WPARAM wParam, LPARAM lParam)
{
    if (NULL != wParam)
    {
		DisConnectionInfo* stuInfo = (DisConnectionInfo*)wParam;
        CString strInfo;
//        strInfo.Format("Device DisConnected! ip:%s, port:%d",szIp, nPort);
		strInfo.Format(ConvertString("Device DisConnected! Ip:%s, Port:%d"),stuInfo->strIp, stuInfo->nPort);
        PRINT_OPERATE("Device DisConnected! Ip:%s, Port:%d",stuInfo->strIp, stuInfo->nPort);
        SetDlgItemText(IDC_STATIC_INFO,strInfo);	
        std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
        for( ; Device != m_lstDeviceInfos.end(); ++Device)
        {
            
            DH_DEVICE_INFO* pInfo = *Device;
            if (pInfo != NULL && pInfo->lLoginID == stuInfo->lLoginID && pInfo->nPort == stuInfo->nPort &&pInfo->szIP  == stuInfo->strIp)
            {
                m_DevTree.SetItemColor(pInfo->DstRoot,RGB_OFFLine);
                if (pInfo->nLoginType == 1)
                {
                    CLIENT_Logout(pInfo->lLoginID);
                    m_csDeviceInfos.Lock();
                    pInfo->szIP = ConvertString("UnRegister",DLG_MAIN);
                    pInfo->nPort = 0;
                    pInfo->nLoginStatus = 0;
                    pInfo->lLoginID = 0;
                    m_csDeviceInfos.UnLock();
                }
				else if(pInfo->nLoginType == 0)
				{
					CLIENT_Logout(pInfo->lLoginID);
					m_csDeviceInfos.Lock();
					pInfo->nLoginStatus = 0;
					pInfo->lLoginID = 0;
					m_csDeviceInfos.UnLock();
				}
				for(int h =0; h<9 ; h++)
				{
					if (m_stuPlayInfo[h].hParent == pInfo->DstRoot)
					{
						m_stuPlayInfo[h].root = 0;
						m_stuPlayInfo[h].bPlay = FALSE;
						m_stuPlayInfo[h].hParent = 0;
						m_stuPlayInfo[h].nChannel = -1;
						m_stuPlayInfo[h].bFrameType = 0;
						m_DispHanle[h] = 0;
						m_ptzScreen.Invalidate();
					}
				}
				m_DevTree.Invalidate();
                break;
            }
        }
		delete stuInfo;
		stuInfo = NULL;
    }
    return 0;
}

LRESULT CActiveLoginDemoDlg::OnLoginSuccess(WPARAM wParam, LPARAM lParam)
{
	CString strInfo;
	DH_DEVICE_INFO* pInfo = (DH_DEVICE_INFO*)wParam;
	if (NULL == pInfo)
	{
		return 0;
	}
	int nChannelNum = pInfo->nChannelNum;
	if (1 == lParam)
	{
		PRINT_OPERATE("Login Success! Ip:%s, Port:%d",pInfo->szIP, pInfo->nPort);
		strInfo.Format(ConvertString("Login Success! Ip:%s, Port:%d"),pInfo->szIP, pInfo->nPort);
		SetDlgItemText(IDC_STATIC_INFO,strInfo);
		if (pInfo->bOnTree)
		{
			m_DevTree.SetItemColor(pInfo->DstRoot,RGB_OnLine);
			if (1 == pInfo->nLoginType && 1 == m_nShowType)
			{
				m_DevTree.SetItemText(pInfo->DstRoot, pInfo->szIP);
			}
			CString str;
			CString strChannel = CHANNEL;
			HTREEITEM ChannelRoot;
			while(TRUE)                                                                 // 添加通道节点前先删除一次，以免重复添加
			{
				ChannelRoot = m_DevTree.GetChildItem(pInfo->DstRoot);
				if (NULL != ChannelRoot)
				{
					m_DevTree.DeleteItem(ChannelRoot);
				}
				else
				{
					break;
				}
			}
			for (int i= 0; i<nChannelNum; i++)
			{
				str.Format("%d",i+1);
				str = strChannel + str;
				HTREEITEM ChannelRoot = Initdirectory(str,pInfo->DstRoot);
				m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
				m_DevTree.SetItemData(ChannelRoot,i);
			}
			m_DevTree.Invalidate(FALSE);
		}
		
	}
	else if (0 == lParam)
	{	
		m_DevTree.SetItemColor(pInfo->DstRoot,RGB_OFFLine);	
		PRINT_OPERATE("Login Fail! ip:%s, port:%d",pInfo->szIP, pInfo->nPort);
		strInfo.Format(ConvertString("Login Fail! ip:%s, port:%d"),pInfo->szIP, pInfo->nPort);
		SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
	}
	return 0;
}

LRESULT CActiveLoginDemoDlg::OnLoginDisconnected(WPARAM wParam, LPARAM lParam)
 {
	DH_DEVICE_INFO* pInfo = (DH_DEVICE_INFO*)wParam;
	PRINT_OPERATE("Logining DisConnect! IP:%s,Port:%d",pInfo->szIP,pInfo->nPort);
	CString strInfo;
	strInfo.Format(ConvertString("Logining DisConnect! IP:%s,Port:%d"),pInfo->szIP,pInfo->nPort);
	SetDlgItemText(IDC_STATIC_INFO,strInfo);
	return 0;
}

BOOL CActiveLoginDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
    char tmpDir[1000]={0};
    GetCurrentDirectory(1000,tmpDir);
    CString strWorkDir;
    strWorkDir.Format("%s", tmpDir);
    m_strPath = strWorkDir + "\\deviceinfo.xlsx";
	m_strPDFPath = strWorkDir + "\\NetSDK_AutoRegister_Manual_V1.0.0.pdf";
    m_ptzScreen.Create(NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,1981);
    UpdataScreenPos();
    m_ptzScreen.ShowWindow(SW_SHOW);
    m_ptzScreen.SetCallBack(MessageProcFunc,(DWORD)this,
        GetParamsFunc,(DWORD)this,
        SetParamsFunc,(DWORD)this,
        RectEventFunc,(DWORD)this);
	m_ptzScreen.SetShowPlayWin(SPLIT9,0);
	GetDlgItem(IDC_STARTSERVER)->EnableWindow(FALSE);
    m_LocalAddr.SetAddress(127,0,0,1);
    InitTabCtrl();
    CLIENT_Init(DemoDisConnectCallBack, (LDWORD)this);
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
    m_hThread = CreateThread(NULL,0,LoginThreadProc,(void*)this,0,0);
	CString strShowType[]= {"ID","IP","Name"};
	m_Cmb_ShowType.ResetContent();
	for (int i= 0; i<3;i++)
	{
		m_Cmb_ShowType.InsertString(-1, ConvertString(strShowType[i], DLG_MAIN));
	}
    m_Cmb_ShowType.SetCurSel(0);
	m_nShowType = 0;

	CString strSplit[]= {"Single","Split-4","Split-9"};
	m_Cmb_Split.ResetContent();
	for (int i= 0; i<3;i++)
	{
		m_Cmb_Split.InsertString(-1, ConvertString(strSplit[i], DLG_MAIN));
	}
    m_Cmb_Split.SetCurSel(2);
    
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CALLBACK  MessageProcFunc(int nWndID, UINT message, DWORD dwUser)
{
    if(dwUser == 0)
    {
        return;
    }
    
    CActiveLoginDemoDlg *dlg = (CActiveLoginDemoDlg *)dwUser;
    dlg->MessageProc(nWndID, message);
    
}
void CActiveLoginDemoDlg::MessageProc(int nWndID, UINT message)
{
    switch(message)
    {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        {
            m_CurScreen =nWndID;
            m_PtzControlDlg.m_nChannel = m_stuVideoInfo[m_CurScreen].nChannel;
            m_PtzControlDlg.m_lLoginID = m_stuVideoInfo[m_CurScreen].lLoginID;
        }
        break;
    default:
        break;
    }
    
}
BOOL CALLBACK  GetParamsFunc(int nWndID, int type, DWORD dwUser)
{
    if(dwUser == 0)
    {
        return FALSE;
    }
    CActiveLoginDemoDlg *dlg = (CActiveLoginDemoDlg *)dwUser;
    return dlg->GetParams(nWndID, type);
}

BOOL CActiveLoginDemoDlg::GetParams(int nWndID, int type)
{
    BOOL bRet = FALSE;
    
    if (type == 0)
    {
        bRet = GetExitDecode(nWndID);
    }
    else if (type == 1)
    {
        bRet = GetExitCycle(nWndID);
    }
    
    return bRet;
}

BOOL CActiveLoginDemoDlg::GetExitDecode(int nCurWndID)
{
    BOOL bRet = FALSE;
    if (nCurWndID<0 || nCurWndID>=16)
    {
        return bRet;
    }
    
    return bRet;
}

BOOL CActiveLoginDemoDlg::GetExitCycle(int nCurWndID)
{
    BOOL bRet = FALSE;
    if (nCurWndID<0 || nCurWndID>=16)
    {
        return bRet;
    }
    
    bRet = m_bWndExitCycle[nCurWndID];
    
    return bRet;
}

void CALLBACK SetParamsFunc(int nWndID, int type, DWORD dwUser)
{
    if(dwUser == 0)
    {
        return;
    }
    
    CActiveLoginDemoDlg *dlg = (CActiveLoginDemoDlg *)dwUser;
    dlg->SetParams(nWndID, type);
}
void CActiveLoginDemoDlg::SetParams(int nWndID, int type)
{
    if (type == 0)
    {
        SetExitDecode(nWndID);
    }
    else if (type == 1)
    {
        SetExitCycle(nWndID);
    }
    
}
void CActiveLoginDemoDlg::SetExitDecode(int nCurWndID)
{
    if (nCurWndID<0 || nCurWndID>=9)
    {
        return;
    }
    if (m_DispHanle[nCurWndID] != 0)
    {
        BOOL bRet = CLIENT_StopRealPlayEx(m_DispHanle[nCurWndID]); 
    }
    m_DispHanle[nCurWndID] = 0;
    m_stuPlayInfo[nCurWndID].root = 0;
    m_stuPlayInfo[nCurWndID].bPlay = FALSE;
    m_stuPlayInfo[nCurWndID].hParent = 0;
    m_stuPlayInfo[nCurWndID].nChannel = -1;
	m_stuPlayInfo[nCurWndID].bFrameType = 0;
	if (nCurWndID ==8)
	{
		m_ptzScreen.SetActiveWnd(0,TRUE);
		m_CurScreen = 0;
	}
	else
	{
		m_ptzScreen.SetActiveWnd(m_CurScreen+1,TRUE);
		m_CurScreen ++;
	}
	m_ptzScreen.Invalidate();
}

void CActiveLoginDemoDlg::SetExitCycle(int nCurWndID)
{
    if (nCurWndID<0 || nCurWndID>=9)
    {
        return;
    }
    
    m_bWndExitCycle[nCurWndID] = !m_bWndExitCycle[nCurWndID];
}

void CALLBACK RectEventFunc(RECT WinRect,CPoint &pointStart,CPoint &pointEnd,DWORD dwUser)
{
    CActiveLoginDemoDlg* dlg = (CActiveLoginDemoDlg*) dwUser;
    CPoint Origin;
    CPoint SendPoint;
    
    Origin.x = ( WinRect.left + WinRect.right ) / 2;
    Origin.y = ( WinRect.top + WinRect.bottom ) / 2;
    
    int dx = (pointStart.x + pointEnd.x)/2;
    int dy = (pointStart.y + pointEnd.y)/2;
    
    int width  = WinRect.right - WinRect.left;
    int height = WinRect.bottom - WinRect.top; 
    
    SendPoint.x = ( dx - Origin.x) * 8192 * 2 / width;
    SendPoint.y = ( dy - Origin.y) * 8192 * 2  / height ;
    
    int width2 = pointEnd.x - pointStart.x;
    int height2 = pointEnd.y - pointStart.y;
    int multiple = 0;
    
    if ( height2 !=0 && width2!=0)
    {
        if (pointEnd.y >= pointStart.y)
        {
            multiple = (width * height) / (width2 * height2);
        }
        else
        {
            multiple = -(width * height) / (width2 * height2);
        }
    }
    
    dlg->SetFastGOTO(SendPoint.x, SendPoint.y, multiple);
}

void CActiveLoginDemoDlg::SetFastGOTO(int nposX, int nposY, int nZoom)
{
    m_PtzControlDlg.m_posX = nposX;
    m_PtzControlDlg.m_posY = nposY;
    m_PtzControlDlg.m_posZoom = nZoom;
    m_PtzControlDlg.UpdateData(FALSE);
    m_PtzControlDlg.PtzExtControl(DH_EXTPTZ_FASTGOTO);
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CActiveLoginDemoDlg::OnPaint() 
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
HCURSOR CActiveLoginDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CActiveLoginDemoDlg::PreTranslateMessage(MSG* pMsg) 
{
	
	if (WM_KEYDOWN == pMsg->message && 
		(VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam))
	{
		if (VK_ESCAPE == pMsg->wParam && m_ptzScreen.GetFullScreen())
		{
			m_ptzScreen.SetFullScreen(FALSE);
		}
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CActiveLoginDemoDlg::UpdataScreenPos()
{
    GetDlgItem(IDC_SCREEN_WINDOW)->GetWindowRect(&m_clientRect);
    ScreenToClient(&m_clientRect);
    m_screenRect = m_clientRect;
	m_ptzScreen.MoveWindow(m_screenRect);
}

HTREEITEM CActiveLoginDemoDlg::Initdirectory(CString strSecDir, HTREEITEM root)
{
   return m_DevTree.InsertItem(strSecDir,0,0,root,TVI_LAST);
}

void CActiveLoginDemoDlg::InitExcel()
{
    DWORD  iTIme = ::GetTickCount();
    CReadWriteExcel m_ReadWriteExcel;
    if (!m_ReadWriteExcel.SetFilePath(m_strPath))
    {
		GetDlgItem(IDC_BTN_SEARCH)->EnableWindow(FALSE);
		MessageBox(ConvertString("please Check the deviceinfo.xlsx is right!",DLG_MAIN), ConvertString("Prompt"));
        return;
    }
	m_ReadWriteExcel.SetUseRange(1);  
	m_ReadWriteExcel.SetCellString(1,1,ConvertString("Parents Dir",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,2,ConvertString("Device ID",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,3,ConvertString("Device Name",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,4,ConvertString("Device IP",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,5,ConvertString("Port",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,6,ConvertString("UserName",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,7,ConvertString("PassWord",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,8,ConvertString("LoginType",DLG_MAIN));
    m_ReadWriteExcel.SetUseRange(2);                // 先读sheet2，构造好设备树目录
	m_ReadWriteExcel.SetCellString(1,1,ConvertString("First Directory",DLG_MAIN));
	m_ReadWriteExcel.SetCellString(1,2,ConvertString("Second Directory",DLG_MAIN));
    int iRowNum = m_ReadWriteExcel.GetRowCount();  //取得已经使用的行数
    iRowNum = iRowNum <= MAX_ROW ? iRowNum : MAX_ROW;
    int iColNum = m_ReadWriteExcel.GetColCount();  //取得已经使用的列数

	m_strRootName = m_ReadWriteExcel.GetCellString(2,1);     // 获取第二行，第一列数据，作为根目录名称
    if (m_strRootName.IsEmpty())
    {
        m_strRootName = ConvertString("root directory");
    }
    HTREEITEM root = m_DevTree.InsertItem(m_strRootName);    // 首先构建一级根目录
    m_DevTree.SetItemData(root, ROOT_DIR);                   // 根目录值设置为-100

    for (int i = 2 ;i<= iRowNum; i++)                        // 读取二级目录并添加到目录链表中            
    {
        m_DevTree.Expand(root, TVE_EXPAND); 
        CString SecDirName;
		CString SecName;
        BOOL bRepeat = FALSE;
        SecDirName = m_ReadWriteExcel.GetCellString(i,2);
        if (SecDirName.IsEmpty())
        {
            PRINT_WARNING("Excel Cell is Empty! Row = %d, Col = %d", i, 2);
            bRepeat = TRUE;
        }
        else
        {
            map<CString,char>::iterator it = m_mapDirName.find(SecDirName);
            if (it != m_mapDirName.end())
            {
                PRINT_WARNING("Excel Cell is Repeat! Row = %d, Col = %d", i, 2);
                bRepeat = TRUE;
            }
        }

        if (!bRepeat)
        {
            m_mapDirName.insert(make_pair<CString,char>(SecDirName,'0'));
            DIR_NAME_NUMBER* s_DirName_Number = NEW DIR_NAME_NUMBER;
            s_DirName_Number->strSecDirName = SecDirName;
            s_DirName_Number->nSecDirNumber = i;
            s_DirName_Number->root = Initdirectory(SecDirName, root);    // 构建二级目录，从第二行，第二列开始
            m_DevTree.Invalidate();
            m_DevTree.SetItemData(s_DirName_Number->root,SECOND_DIR);
            m_lstSecDirInfos.push_back(s_DirName_Number);                // 保存目录链表 
        }
    }
    m_ReadWriteExcel.SetUseRange(1);                                     // 读sheet1，读取设备信息
    iRowNum = m_ReadWriteExcel.GetRowCount();                            // 取得已经使用的行数
    iRowNum = iRowNum <= MAX_ROW ? iRowNum : MAX_ROW;
    iColNum = m_ReadWriteExcel.GetColCount();                            // 取得已经使用的列数
    if (iColNum <2)
    {
        return;
    }

    for (int h = 2; h<=iRowNum; h++)                                      // 从第二行第一列开始读取
    {
        BOOL bFindID = TRUE;
        DH_DEVICE_INFO* s_Device_Info = NEW DH_DEVICE_INFO;              // 每一行一个设备
        if (NULL == s_Device_Info)
        {
            PRINT_ERROR("malloc s_Device_Info failed!");
            break;
        }

        s_Device_Info->lLoginID = 0;
        CString str;
        for (int l = 1; l<=iColNum; l++)
        {
            str = m_ReadWriteExcel.GetCellString(h,l);
            if (2 == l)                                                  // 如果设备ID号为空，直接跳过
            {
                if (str.IsEmpty())
                { 
                    PRINT_ERROR("Device ID is Empty! Row:%d,Col:%d", h,l);       
                    bFindID = FALSE;
                    break;
                }
                map<CString,char>::iterator it = m_mapDeviceID.find(str); // ID重复,跳过
                if (it != m_mapDeviceID.end())
                {
                    PRINT_ERROR("Device ID is Repeat! Row = %d, Col = %d", h, l);
                    bFindID = FALSE;
                    break;
                }
            }
            InitDeviceInfo(l, s_Device_Info, str);   
        }

        if (!bFindID)
        {
            if (NULL != s_Device_Info)
            {
                delete s_Device_Info;
                s_Device_Info = NULL;
            }
            continue;
        }

        m_mapDeviceID.insert(pair<CString,char>(s_Device_Info->szDevSerial,'0'));
        std::list<DIR_NAME_NUMBER*>::iterator it = m_lstSecDirInfos.begin();            // 根据第一列目录名称，查找设备所属的目录等级和目录序号
        BOOL bFindName = FALSE;
        while( it != m_lstSecDirInfos.end())
        {
            DIR_NAME_NUMBER* pInfo = *it;
            ++it;
            if (pInfo->strSecDirName == s_Device_Info->strSecDirName)
            {
                bFindName = TRUE;
                s_Device_Info->srcRoot = pInfo->root;
                break;
            }
        }
        if (!bFindName)
        {
            s_Device_Info->srcRoot = root;
        }
        if (0 == s_Device_Info->nLoginType)
        {
            s_Device_Info->nLoginStatus = 1;
			s_Device_Info->nLoginTimes = 1;
        }
        else if (1 == s_Device_Info->nLoginType)
        {
            s_Device_Info->nLoginStatus = 0;
            s_Device_Info->nLoginTimes = 0;
        }
        s_Device_Info->nDeviceRow = h;
        s_Device_Info->DstRoot = Initdirectory(s_Device_Info->szDevSerial,s_Device_Info->srcRoot); 
		s_Device_Info->bOnTree = TRUE;
        m_DevTree.Invalidate();
        m_DevTree.SetItemData(s_Device_Info->DstRoot,DEVICE_DIR);
        m_lstDeviceInfos.push_back(s_Device_Info);                     // 保存设备信息链表                         
    }

    std::sort(m_vecIDInfo.begin(),m_vecIDInfo.end());
    std::sort(m_vecIPInfo.begin(),m_vecIPInfo.end());
    std::sort(m_vecNameInfo.begin(),m_vecNameInfo.end());
}

void CActiveLoginDemoDlg::DoEvent()
{
	MSG Msg;
	if (::PeekMessage(&Msg,NULL,0,0,PM_REMOVE))
	{
		::TranslateMessage(&Msg);
		::DispatchMessage(&Msg);
	}
}
void CActiveLoginDemoDlg::ClearRecourse()
{
	m_csSecDirInfos.Lock();
	std::list<DIR_NAME_NUMBER*>::iterator it = m_lstSecDirInfos.begin();
	while( it != m_lstSecDirInfos.end())
	{
		DIR_NAME_NUMBER* pInfo = *it;
		++it;
		if (pInfo != NULL)
		{
			delete pInfo;
			pInfo = NULL;
		}
	}
	m_lstSecDirInfos.clear();
	m_csSecDirInfos.UnLock();

	m_csDeviceInfos.Lock();
	std::list<DH_DEVICE_INFO*>::iterator lstDevice = m_lstDeviceInfos.begin();
	while( lstDevice != m_lstDeviceInfos.end())
	{
		DH_DEVICE_INFO* pInfo = *lstDevice;
		++lstDevice;
		if (pInfo != NULL)
		{
			delete pInfo;
			pInfo = NULL;
		}
	}
	m_lstDeviceInfos.clear();
	m_csDeviceInfos.UnLock();
	
  	m_mapDeviceID.clear();
  	m_mapDirName.clear();
	m_vecIDInfo.clear();
	m_vecIPInfo.clear();
	m_vecNameInfo.clear();

}
static DWORD WaitForObjectEx( HANDLE hHandle, DWORD dwMilliseconds )
{
	BOOL bRet;
	MSG msg;
	int iWaitRet = 0;
	int nTimeOut = 0;
	while( (bRet = ::GetMessage( &msg, NULL, 0, 0 )) != 0)
	{
		if(nTimeOut++ * 20 >= dwMilliseconds)
			break;
		iWaitRet = WaitForSingleObject(hHandle, 20);
		if(iWaitRet != WAIT_TIMEOUT)
		{
			break;
		}
		if (bRet == -1)
		{
			break;
		}
		else
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	return iWaitRet;
}

void CActiveLoginDemoDlg::OnClose() 
{
	if (!m_bInitExcel)
	{
		MessageBox(ConvertString("Initializationing，Please wait!",DLG_MAIN), ConvertString("Prompt"));
		return;
	}
	m_bExit = TRUE;
	if (0 != m_lListenHdl)
	{
		BOOL bRet = CLIENT_StopListenServer(m_lListenHdl);
		m_lListenHdl = 0;
	}
	OnBtnStopall();
	DWORD dwRet = 0;
	dwRet = WaitForObjectEx(m_hThread,INFINITE);

	if (0 != m_lListenHdl)
	{
		BOOL bRet = CLIENT_StopListenServer(m_lListenHdl);
		m_lListenHdl = 0;
	}
	CLIENT_Cleanup();
	ClearRecourse();
	CDialog::OnClose();
}

void CActiveLoginDemoDlg::InitDeviceInfo(int iCol, DH_DEVICE_INFO* stuDevice_Info, CString str)
{
    char szBuf[100];
    memset(&szBuf,0,100);
	strncpy(szBuf, str, sizeof(szBuf) - 1);
    switch(iCol)
    {
    case 1:
        stuDevice_Info->strSecDirName = str;
        break;
    case 2:
        stuDevice_Info->szDevSerial = str;
        m_vecIDInfo.push_back(szBuf);
        break;
    case 3:
		if(str.IsEmpty())
		{
			str = NONAME;
		}
		else if( str != NONAME)
		{
			m_vecNameInfo.push_back(szBuf);
		}
        stuDevice_Info->szDevName = str;
        break;
    case 4:
        if (str.IsEmpty())
        {
            str = UNREGISTER;
        }
        else if ( str != UNREGISTER)
        {
			m_vecIPInfo.push_back(szBuf);
        }
        stuDevice_Info->szIP = str;
        break;
    case 5:
        stuDevice_Info->nPort = atoi(str);
        break;
    case 6:
        stuDevice_Info->szUseName = str;
        break;
    case 7:
        stuDevice_Info->szPassWord = str;
        break;
    case 8:
        stuDevice_Info->nLoginType = atoi(str);
        break;
    default:
        break;
    }  
}

void CActiveLoginDemoDlg::LoginThread()
{
    std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
    for( ; Device != m_lstDeviceInfos.end(); ++Device)
    {
			if (m_bSort || m_bExit || m_bDeleteDev || m_bSearch || m_bDirSort)
			{
				break;
			}
			else
			{
				DH_DEVICE_INFO* pInfo = *Device;
				if (pInfo != NULL)
				{
					login(pInfo);
				}
			}
    }
}

void CActiveLoginDemoDlg::login(DH_DEVICE_INFO* pInfo)
{
    CString strInfo;
    if (NULL !=pInfo)
    {
        if (1 == pInfo->nLoginStatus || pInfo->nLoginTimes >0)
        {
			NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
			memset(&stInparam, 0, sizeof(stInparam));
			stInparam.dwSize = sizeof(stInparam);
			strncpy(stInparam.szIP, pInfo->szIP, sizeof(stInparam.szIP) - 1);
			strncpy(stInparam.szPassword, pInfo->szPassWord, sizeof(stInparam.szPassword) - 1);
			strncpy(stInparam.szUserName, pInfo->szUseName, sizeof(stInparam.szUserName) - 1);
			stInparam.nPort = pInfo->nPort;

			NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
			memset(&stOutparam, 0, sizeof(stOutparam));
			stOutparam.dwSize = sizeof(stOutparam);

            LLONG loginID = 0;
			if (HAND == pInfo->nLoginType)
			{
				stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;
				loginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
			}
			else if (AUTO == pInfo->nLoginType)
			{
				char szSerial[64] = {0};
				strncpy(szSerial, pInfo->szDevSerial, sizeof(szSerial) - 1);

				stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_SERVER_CONN;
				stInparam.pCapParam = (void*)szSerial;
				loginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);
			}
			if (loginID != 0)
            {
                m_csDeviceInfos.Lock();
				pInfo->lLoginID = loginID;
				pInfo->nChannelNum = stOutparam.stuDeviceInfo.nChanNum;
                pInfo->nLoginStatus = 2;
				pInfo->nLoginTimes = 0;
				m_csDeviceInfos.UnLock();
				SendMessage(WM_LOGINSUCCESS,(WPARAM)pInfo,1); 
            }
            else
            {
                m_csDeviceInfos.Lock();
                pInfo->nLoginStatus = 0;
				pInfo->nLoginTimes = 0;
				pInfo->lLoginID = 0;
				pInfo->nChannelNum = 0;
				m_csDeviceInfos.UnLock();
				SendMessage(WM_LOGINSUCCESS,(WPARAM)pInfo,0);
            }
        }
        
    }
}
void CActiveLoginDemoDlg::GetLocalIPAndPort(char *pcIPAddr, int *pnPort)
{
    if (NULL == pcIPAddr)
    {
        PRINT_ERROR("IP is Error!");
        return;
    }
    
    if (NULL == pnPort)
    {
        PRINT_ERROR("Port is Error!");
        return;
    }
    
    BOOL bValid = UpdateData(TRUE);
    if (TRUE == bValid)
    {
        BYTE nField0,nField1,nField2,nField3;
        m_LocalAddr.GetAddress(nField0,nField1,nField2,nField3);
        sprintf(pcIPAddr, "%d.%d.%d.%d",nField0,nField1,nField2,nField3);
        *pnPort = (WORD)m_LocalPort;
    }
    return;
}

//Callback function when device disconnected
int CALLBACK ListenCallback(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen, LDWORD dwUserData)
{
    if(0 != dwUserData)
    {
        CString strInfo;
        CActiveLoginDemoDlg *dlg = (CActiveLoginDemoDlg *)dwUserData;
//         strInfo.Format("Call Back! IP:%s,Port:%d,Command:%d",pIp,wPort,lCommand);
//         PRINT_OPERATE(strInfo);
//         dlg->SetDlgItemText(IDC_STATIC_INFO,strInfo);
		dlg->DealListenCB(lHandle, pIp,wPort, lCommand, pParam, dwParamLen);
    }
    
    return 0;
}

void CActiveLoginDemoDlg::DealListenCB(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen)
{
    if (DH_DVR_DISCONNECT == lCommand)
    {
        CString strIp = pIp;
        std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
        for( ; Device != m_lstDeviceInfos.end(); ++Device)
        {
            DH_DEVICE_INFO* pInfo = *Device;
            if (pInfo != NULL && strIp == pInfo->szIP && wPort == pInfo->nPort)
            {
				SendMessage(WM_LOGIN_DISCONNECTED,(WPARAM)pInfo,0);
                m_csDeviceInfos.Lock();
                pInfo->szIP = ConvertString("UnRegister");
                pInfo->nPort = 0;
                pInfo->nLoginStatus = 0;
                pInfo->nLoginTimes--;
                m_csDeviceInfos.UnLock();
                break;
            }
        }
	}
    else if (DH_DVR_SERIAL_RETURN == lCommand)
    {
        char* szDevSerial =(char*)pParam;
        CString strSerial = szDevSerial;
        std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
        for( ; Device != m_lstDeviceInfos.end(); ++Device)
        {
            DH_DEVICE_INFO* pInfo = *Device;
            if (pInfo != NULL && strSerial == pInfo->szDevSerial && 1 == pInfo->nLoginType)
            {
                m_csDeviceInfos.Lock();
                pInfo->szIP = pIp;
                pInfo->nPort = wPort;
                pInfo->nLoginStatus = 1;
                pInfo->nLoginTimes++;
				DeleteVector(m_vecIPInfo,pIp);
                m_vecIPInfo.push_back(pIp);
                m_csDeviceInfos.UnLock();
                break;
            }
        }
    }
}
void CActiveLoginDemoDlg::OnStartserver() 
{
    char szIP[32] = {0};
    int nPort = 0;
    CString strInfo;
	GetLocalIPAndPort(szIP, &nPort);
    m_lListenHdl = CLIENT_ListenServer(szIP, nPort, 1000, ListenCallback, (LDWORD)this);
    if (0 == m_lListenHdl)
    {
		PRINT_OPERATE("Listen Fail! ip:%s, port:%d",szIP, nPort);
        strInfo.Format(ConvertString("Listen Fail! ip:%s, port:%d"),szIP, nPort);
    }
    else
    {
		PRINT_OPERATE("Listen Success! ip:%s, port:%d",szIP, nPort);
        strInfo.Format(ConvertString("Listen Success! ip:%s, port:%d"),szIP, nPort);
        GetDlgItem(IDC_STARTSERVER)->EnableWindow(FALSE);
        GetDlgItem(IDC_STOPSERVER)->EnableWindow(TRUE);
	}
	SetDlgItemText(IDC_STATIC_INFO,strInfo);
}

void CActiveLoginDemoDlg::OnStopserver() 
{
    if (0 != m_lListenHdl)
    {
        CString strInfo;
        BOOL bRet = CLIENT_StopListenServer(m_lListenHdl);
        if (TRUE != bRet)
        {
            strInfo = "Stop Listen Fail!";
        }
        else
        {
            strInfo = "Stop listen Succeed!";
            GetDlgItem(IDC_STARTSERVER)->EnableWindow(TRUE);
            GetDlgItem(IDC_STOPSERVER)->EnableWindow(FALSE);
        }
		PRINT_OPERATE(strInfo);
        SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
        m_lListenHdl = 0;
	}
}

void CActiveLoginDemoDlg::OnClickDevtree(NMHDR* pNMHDR, LRESULT* pResult)		// 左键单击，查找当前设备登录句柄和通道号
{
	CPoint point;
    GetCursorPos(&point);
    m_DevTree.ScreenToClient(&point);
    UINT uFlags;
    HTREEITEM CurrentItem;
    CurrentItem = m_DevTree.HitTest(point,&uFlags);
    if (NULL == CurrentItem)
    {
        return;
    }
    int i =  m_DevTree.GetItemData(CurrentItem);
    if (-1 != i)
    {
        m_nChannel = i;
    }
    if ((NULL != CurrentItem) && (TVHT_ONITEM & uFlags))
    {
        DH_DEVICE_INFO* pInfo = Find(CurrentItem);
        if (NULL != pInfo)
        {
            m_lLoginID = pInfo->lLoginID;
        }
    }
    else
    {
        return;
    }
	*pResult = 0;
}

DH_DEVICE_INFO* CActiveLoginDemoDlg::Find(HTREEITEM hRoot)							// 通过树句柄查找具体设备信息
{

    DH_DEVICE_INFO* pInfo;
    std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
    for( ; Device != m_lstDeviceInfos.end(); ++Device)
    {
        pInfo = *Device;
        if (hRoot == pInfo->DstRoot && pInfo->bOnTree)
        {
            return pInfo;
        }
    }
    return NULL;
}

void CActiveLoginDemoDlg::OnDblclkDevtree(NMHDR* pNMHDR, LRESULT* pResult)			// 左键双击，默认开启主码流预览
{
	
    CPoint point;
    GetCursorPos(&point);
    m_DevTree.ScreenToClient(&point);
    UINT uFlags;
    HTREEITEM CurrentItem;
    HTREEITEM ParentItem;
    CurrentItem = m_DevTree.HitTest(point,&uFlags);
    ParentItem = m_DevTree.GetParentItem(CurrentItem);
    if ((NULL != ParentItem) && (TVHT_ONITEM & uFlags))
    {
        DH_DEVICE_INFO* pInfo = Find(ParentItem);
        if (NULL != pInfo)
        {
            m_lLoginID = pInfo->lLoginID;
            m_nChannel = m_DevTree.GetItemData(CurrentItem);
            HWND hWnd = GetDispHandle();
            if (0 != hWnd)
            {
                if (0 !=m_DispHanle[m_CurScreen])
                {
                    CLIENT_StopRealPlayEx(m_DispHanle[m_CurScreen]);
                    m_DispHanle[m_CurScreen] = 0;
                    m_stuVideoInfo[m_CurScreen].nChannel = 0;
                    m_stuVideoInfo[m_CurScreen].lLoginID = 0;
                    m_stuPlayInfo[m_CurScreen].root = 0;
                    m_stuPlayInfo[m_CurScreen].bPlay = FALSE;
                    m_stuPlayInfo[m_CurScreen].hParent = 0;
                    m_stuPlayInfo[m_CurScreen].nChannel = -1;
					m_stuPlayInfo[m_CurScreen].bFrameType = 0;
                    Invalidate();
                }
                for (int i = 0; i<9;i++)
                {
                    if (m_stuPlayInfo[i].root == CurrentItem)
                    {
                        int nDisHandle = i;
                        if (m_stuPlayInfo[i].bPlay)
                        {
                            CLIENT_StopRealPlayEx(m_DispHanle[nDisHandle]);
                            m_DispHanle[nDisHandle] = 0;
                            m_stuVideoInfo[nDisHandle].nChannel = 0;
                            m_stuVideoInfo[nDisHandle].lLoginID = 0;
                            m_stuPlayInfo[i].root = 0;
                            m_stuPlayInfo[i].bPlay = FALSE;
                            m_stuPlayInfo[i].hParent = 0;
                            m_stuPlayInfo[i].nChannel = -1;
							m_stuPlayInfo[i].bFrameType = 0;
                            Invalidate();
                        }
                    }
                }
                long lRet = CLIENT_RealPlayEx(m_lLoginID,m_nChannel,hWnd);
                if(0 != lRet)
                {
/*					CLIENT_SetRealDataCallBack(lRet,RealDataCallBackEx,(LDWORD)this);*/
                    m_DispHanle[m_CurScreen]=lRet;
                    m_stuVideoInfo[m_CurScreen].lLoginID = m_lLoginID;				
                    m_stuVideoInfo[m_CurScreen].nChannel = m_nChannel;
                    if (!m_stuPlayInfo[m_CurScreen].bPlay)
                    {
                        m_stuPlayInfo[m_CurScreen].bPlay = TRUE;
                        m_stuPlayInfo[m_CurScreen].root = CurrentItem;
                        m_stuPlayInfo[m_CurScreen].hParent = ParentItem;
                        m_stuPlayInfo[m_CurScreen].nChannel = m_nChannel;
						m_stuPlayInfo[m_CurScreen].bFrameType = DH_RType_Realplay_0;
                    }
					if (m_CurScreen ==8)
					{
						m_ptzScreen.SetActiveWnd(0,TRUE);
						m_CurScreen = 0;
					}
					else
					{
						m_ptzScreen.SetActiveWnd(m_CurScreen+1,TRUE);
						m_CurScreen ++;
					}
                }
                else
                {
                    CString strInfo;
                    strInfo = "Fail to play!";
                    PRINT_OPERATE(strInfo);
                    SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
	            }
            }
        }
    }
    else
    {
        return;
    }
	*pResult = 0;
}

HWND CActiveLoginDemoDlg::GetDispHandle()
{
    HWND hWnd=0;
    hWnd = ((CWnd *)(m_ptzScreen.GetPage(m_CurScreen)))->m_hWnd;
	return hWnd;
}

void CActiveLoginDemoDlg::InitTabCtrl()
{
    CRect childRect;
    m_CtlTab.GetClientRect(childRect);
    childRect.top += 20;
    childRect.bottom -= 4;
    childRect.left += 2;
	childRect.right -= 2;

    m_PtzControlDlg.Create(IDD_DIALOG_PTZCONTROL, &m_CtlTab);
	m_PtzControlDlg.MoveWindow(childRect);
    m_CtlTab.InsertItem(0, ConvertString("PTZ CONTROL",DLG_MAIN));
    m_CtlTab.SetCurSel(0);
    m_PtzControlDlg.ShowWindow(SW_SHOW);

}

void CActiveLoginDemoDlg::OnRclickDevtree(NMHDR* pNMHDR, LRESULT* pResult)				// 右键单击，通过不同节点值判断点击的哪一层目录
{
    CPoint point;
    GetCursorPos(&point);
    m_DevTree.ScreenToClient(&point);
    //    m_DevTree.ScreenToClient(&point);
    UINT uFlags;
    HTREEITEM CurrentItem = m_DevTree.HitTest(point,&uFlags);
    if ((NULL != CurrentItem) && (TVHT_ONITEM & uFlags))
    {
        m_DevTree.SelectItem(CurrentItem);
        if (ROOT_DIR == m_DevTree.GetItemData(CurrentItem))
        {
            CMenu menu;
            menu.CreatePopupMenu();
			menu.AppendMenu(MF_STRING , VIDEO_MENU_ADDDIR, ConvertString(NAME_MENU_ADDDIR));
            menu.AppendMenu(MF_STRING , VIDEO_MEMU_ADDDEVICE, ConvertString(NAME_MENU_ADDDEVICE));
			menu.AppendMenu(MF_STRING , VIDEO_MENU_SORT, ConvertString(NAME_MENU_SORT));
            menu.AppendMenu(MF_STRING , VIDEO_MENU_RENAME, ConvertString(NAME_MENU_RENAME));
            GetCursorPos(&point);
            TrackPopupMenu( 
                menu.m_hMenu, 
                TPM_LEFTALIGN, 
                point.x, 
                point.y,
                0,
                this->m_hWnd,
            NULL);
        }
        else if (SECOND_DIR == m_DevTree.GetItemData(CurrentItem))
        {
            CMenu menu;
            menu.CreatePopupMenu();
            HTREEITEM ChildItem = m_DevTree.GetChildItem(CurrentItem);
			BOOL bFind = FALSE;
			std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
			for ( ; Device != m_lstDeviceInfos.end(); ++Device)
			{
				DH_DEVICE_INFO* pInfo = *Device;
				if (pInfo != NULL)
				{
					if (CurrentItem == pInfo->srcRoot)
					{
						bFind = TRUE;
						break;
					}
				}
			}
			menu.AppendMenu(MF_STRING , VIDEO_MEMU_ADDDEVICE, ConvertString(NAME_MENU_ADDDEVICE));
			menu.AppendMenu(MF_STRING, VIDEO_MENU_SORT,ConvertString(NAME_MENU_SORT));
            if (NULL == ChildItem && !bFind)
            {
                menu.AppendMenu(MF_STRING , VIDEO_MENU_DELETEDIR, ConvertString(NAME_MENU_DELETE_DIR));
            }
            menu.AppendMenu(MF_STRING , VIDEO_MENU_RENAME, ConvertString(NAME_MENU_RENAME));

            GetCursorPos(&point);
            TrackPopupMenu( 
                menu.m_hMenu, 
                TPM_LEFTALIGN, 
                point.x, 
                point.y,
                0,
                this->m_hWnd,
				NULL);
        }
        else if (DEVICE_DIR == m_DevTree.GetItemData(CurrentItem))
        {
            CMenu menu;
            menu.CreatePopupMenu();
			DH_DEVICE_INFO* pInfo = Find(CurrentItem);
			if (pInfo != NULL)
			{
				if (pInfo->nLoginType == 0)
				{
					if (2 == pInfo->nLoginStatus)
					{	
						menu.AppendMenu(MF_STRING , VIDEO_MENU_LOGOUT, ConvertString(NAME_MENU_LOGOUT));
					}
					else if(0 == pInfo->nLoginStatus)
					{
						menu.AppendMenu(MF_STRING , VIDEO_MENU_LOGIN, ConvertString(NAME_MENU_LOGIN));
					}
				}
			}
            menu.AppendMenu(MF_STRING , VIDEO_MENU_DELETEDEV, ConvertString(NAME_MENU_DELETE_DEV));
			menu.AppendMenu(MF_STRING , VIDEO_MENU_ATTRIBUTE, ConvertString(NAME_MENU_ATTRIBUTE));
            GetCursorPos(&point);
            TrackPopupMenu( 
                menu.m_hMenu, 
                TPM_LEFTALIGN, 
                point.x, 
                point.y,
                0,
                this->m_hWnd,
            NULL);
        }
        else if (CHANNEL_DIR <= m_DevTree.GetItemData(CurrentItem))
        {
            CMenu menu;
            menu.CreatePopupMenu();
			BOOL bFind = FALSE;
			HTREEITEM hParent = m_DevTree.GetParentItem(CurrentItem);
			DH_DEVICE_INFO* pInfo = Find(hParent);
			if (pInfo == NULL)
			{
				return;
			}
			for(int i = 0 ; i<9; i++)
			{
				if (2 == pInfo->nLoginStatus && CurrentItem == m_stuPlayInfo[i].root && m_stuPlayInfo[i].bPlay)
				{
					if (m_stuPlayInfo[i].bFrameType == DH_RType_Realplay_0)
					{
						menu.AppendMenu(MF_STRING | MF_CHECKED, VIDEO_MENU_STARTMAINPLAY, ConvertString(NAME_MENU_STARTMAINPLAY));
						menu.AppendMenu(MF_STRING | MF_UNCHECKED, VIDEO_MENU_STARTEXTRAPLAY, ConvertString(NAME_MENU_STARTEXTRAPLAY));
						menu.AppendMenu(MF_STRING , VIDEO_MENU_STOPPLAY, ConvertString(NAME_MENU_STOPPLAY));
					}
					else if (m_stuPlayInfo[i].bFrameType == DH_RType_Realplay_1)
					{
						menu.AppendMenu(MF_STRING | MF_UNCHECKED, VIDEO_MENU_STARTMAINPLAY, ConvertString(NAME_MENU_STARTMAINPLAY));
						menu.AppendMenu(MF_STRING | MF_CHECKED, VIDEO_MENU_STARTEXTRAPLAY, ConvertString(NAME_MENU_STARTEXTRAPLAY));
						menu.AppendMenu(MF_STRING , VIDEO_MENU_STOPPLAY, ConvertString(NAME_MENU_STOPPLAY));
					}
					bFind = TRUE;
					break;
				}
			}
			if (!bFind)
			{
				menu.AppendMenu(MF_STRING , VIDEO_MENU_STARTMAINPLAY, ConvertString(NAME_MENU_STARTMAINPLAY));
				menu.AppendMenu(MF_STRING , VIDEO_MENU_STARTEXTRAPLAY, ConvertString(NAME_MENU_STARTEXTRAPLAY));
			}
            GetCursorPos(&point);
            TrackPopupMenu( 
                menu.m_hMenu, 
                TPM_LEFTALIGN, 
                point.x, 
                point.y,
                0,
                this->m_hWnd,
            NULL);
        }
    }

	*pResult = 0;
}

void CActiveLoginDemoDlg::DeleteVector(std::vector<std::string>& vec,char* string)
{
    std::vector<std::string>::iterator iter = std::find(vec.begin(), vec.end(),string);
    
    if(iter != vec.end())
    {
        vec.erase(iter);
    }
}
void CActiveLoginDemoDlg::OnTreeMenu(UINT nID)
{
	if (!m_bInitExcel)
	{
		MessageBox(ConvertString("Initializationing，Please wait!",DLG_MAIN), ConvertString("Prompt"));
		return;
	}
	CString strInfo;
    switch(nID)
    {
    case VIDEO_MEMU_ADDDEVICE:																		// 新增设备
        {
           CDeviceInfo Dlg;
           if (IDOK == Dlg.DoModal())
           {
                if (Dlg.m_strID.IsEmpty())
                {
				    PRINT_OPERATE("Add Fail! ID  is NULL, ID:%s",Dlg.m_strID);
                    strInfo.Format(ConvertString("Add Fail! ID  is NULL, ID:%s"),Dlg.m_strID);
                    SetDlgItemText(IDC_STATIC_INFO,strInfo);
				    MessageBox(strInfo);
                    return;
                }
                if (!Dlg.m_bRadio)                                      // 非主动注册设备，需要判断IP和端口是否填写
                {
                    if ((Dlg.m_strIP.IsEmpty()) || 0 == Dlg.m_nPort )
                    {
						PRINT_OPERATE("Add Fail! IP or Port is NULL, IP:%s, port:%d",Dlg.m_strIP, Dlg.m_nPort);
                        strInfo.Format(ConvertString("Add Fail! IP or Port is NULL, IP:%s, port:%d"),Dlg.m_strIP, Dlg.m_nPort);
                        SetDlgItemText(IDC_STATIC_INFO,strInfo);
						MessageBox(strInfo);
                        return;
                    }
                }
				else
				{
					if (Dlg.m_strIP.IsEmpty())
					{
						Dlg.m_strIP = UNREGISTER;
					}
				}
				CReadWriteExcel Excel;
				if (!Excel.SetFilePath(m_strPath))
				{
					return;
				}
				Excel.SetUseRange(1);
				for (int i = 2; i<= MAX_ROW; i++)
				{
					CString str = Excel.GetCellString(i,2);
					if (str == Dlg.m_strID)
					{
						strInfo.Format("Add Fail! ID is Repeat!");
						PRINT_OPERATE(strInfo);
						SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
						MessageBox(ConvertString(strInfo,DLG_MAIN), ConvertString("Prompt"));
						return;
					}
				}
				BOOL bEmpty = FALSE;
				int nRow = 0;
				for (int i = 2; i<= MAX_ROW; i++)
				{
					CString str = Excel.GetCellString(i,2);
					if (str.IsEmpty())
					{
						nRow = i;
						bEmpty = TRUE;
						break;
					}
				}
				if (!bEmpty)
				{
					strInfo = "Add Fail! All of 1001 Row are Full!";
					PRINT_OPERATE(strInfo);
					SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
					MessageBox(ConvertString(strInfo,DLG_MAIN), ConvertString("Prompt"));
					return;
				}
                DH_DEVICE_INFO* s_Device_Info = NEW DH_DEVICE_INFO;             // 每一行一个设备
                HTREEITEM hRoot = m_DevTree.GetSelectedItem();
                s_Device_Info->srcRoot = hRoot; 
                s_Device_Info->strSecDirName = m_DevTree.GetItemText(hRoot);
                s_Device_Info->nLoginType = Dlg.m_bRadio;

                s_Device_Info->nLoginStatus = 0;                                // 新添加设备全都需要手动登录

                s_Device_Info->szDevName = Dlg.m_strDeviceName;
                s_Device_Info->szIP = Dlg.m_strIP;
                s_Device_Info->szDevSerial = Dlg.m_strID;
                s_Device_Info->szUseName = Dlg.m_strUserName;
                s_Device_Info->szPassWord = Dlg.m_strPsw;
                s_Device_Info->nPort = Dlg.m_nPort;
                s_Device_Info->DstRoot = Initdirectory(Dlg.m_strID, hRoot);
				s_Device_Info->bOnTree = TRUE;
                m_DevTree.Expand(hRoot, TVE_EXPAND);
                char szBuf[100];
				memset(szBuf,0,100);
				strncpy(szBuf,s_Device_Info->szDevSerial,__min(strlen(s_Device_Info->szDevSerial),99));
				m_vecIDInfo.push_back(szBuf);
				memset(szBuf,0,100);
				strncpy(szBuf,s_Device_Info->szIP,__min(strlen(s_Device_Info->szIP),99));
				m_vecIPInfo.push_back(szBuf);
				memset(szBuf,0,100);
				strncpy(szBuf,s_Device_Info->szDevName,__min(strlen(s_Device_Info->szDevName),99));
				m_vecNameInfo.push_back(szBuf);
                
                if (0 == m_nShowType)
                {
                    m_DevTree.SetItemText(s_Device_Info->DstRoot,s_Device_Info->szDevSerial);
                }
                else if (1 == m_nShowType)
                {
                    m_DevTree.SetItemText(s_Device_Info->DstRoot, s_Device_Info->szIP);
                }
                else if (2 == m_nShowType)
                {
                    m_DevTree.SetItemText(s_Device_Info->DstRoot,s_Device_Info->szDevName);
                }
                m_DevTree.SetItemData(s_Device_Info->DstRoot,DEVICE_DIR);
                m_DevTree.SetItemColor(s_Device_Info->DstRoot,RGB_OFFLine);
                CString str;
                Excel.SetCellString(nRow,1,s_Device_Info->strSecDirName);
                Excel.SetCellString(nRow,2,s_Device_Info->szDevSerial);
                Excel.SetCellString(nRow,3,s_Device_Info->szDevName);
                Excel.SetCellString(nRow,4,s_Device_Info->szIP);
                str.Format("%d",s_Device_Info->nPort);
                Excel.SetCellString(nRow,5,str);
                Excel.SetCellString(nRow,6,s_Device_Info->szUseName);
                Excel.SetCellString(nRow,7,s_Device_Info->szPassWord);
                str.Format("%d",s_Device_Info->nLoginType);
                Excel.SetCellString(nRow,8,str);
                s_Device_Info->nDeviceRow = nRow;
				m_csDeviceInfos.Lock();
                m_lstDeviceInfos.push_back(s_Device_Info);
				m_csDeviceInfos.UnLock();
				std::sort(m_vecIDInfo.begin(),m_vecIDInfo.end());
				std::sort(m_vecIPInfo.begin(),m_vecIPInfo.end());
				std::sort(m_vecNameInfo.begin(),m_vecNameInfo.end());
				if (m_bSorted)
				{
					m_bSort = TRUE;
				}
           }  
           break;
        }
    case VIDEO_MENU_RENAME:																	// 重命名
        {
            CNameDlg Dlg;
            if(Dlg.DoModal() == IDOK)
            {
				Dlg.m_strName = Dlg.m_strName.Left(32);
                HTREEITEM hRoot = m_DevTree.GetSelectedItem();
                ReName(hRoot, Dlg.m_strName);
            }
            break;
        }
    case VIDEO_MENU_ADDDIR:																	// 添加目录
        {
            CNameDlg Dlg;
            if(Dlg.DoModal() == IDOK)
            { 
				Dlg.m_strName = Dlg.m_strName.Left(32);
                map<CString,char>::iterator SecondDir = m_mapDirName.find (Dlg.m_strName);      // 首先修改sheet2中的目录名称
                if (SecondDir != m_mapDirName.end())
                {
					strInfo = "Add Fail! Second Directory name is Repeat!";
                    PRINT_WARNING(strInfo);
					SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
					MessageBox(ConvertString(strInfo,DLG_MAIN), ConvertString("Prompt"));
                    return;
                }
                CReadWriteExcel RWExcel;
                if (!RWExcel.SetFilePath(m_strPath))
                {
                    return;
                }
                RWExcel.SetUseRange(2);
                DIR_NAME_NUMBER* s_DirName_Number = NEW DIR_NAME_NUMBER;
                s_DirName_Number->strSecDirName = Dlg.m_strName;
				BOOL bFind = FALSE;
                for (int i= 2; i<=MAX_ROW; i++)                                     //遍历Excel第二列，寻找第一个空行
                {
                    CString str = RWExcel.GetCellString(i,2);
                    if (str.IsEmpty())
                    {
                        s_DirName_Number->nSecDirNumber = i;
						bFind = TRUE;
                        break;
                    }
                }
                if (!bFind)
                {
                    delete s_DirName_Number;
                    s_DirName_Number = NULL;
					strInfo = "Add Fail! All of 1001 Row are Full!";
                    PRINT_WARNING(strInfo);
					SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
					MessageBox(ConvertString(strInfo,DLG_MAIN), ConvertString("Prompt"));
                    return;
                }
				m_mapDirName.insert(pair<CString,char>(Dlg.m_strName,'0'));
                HTREEITEM hSecRoot = m_DevTree.GetRootItem();
                s_DirName_Number->root = Initdirectory(Dlg.m_strName, hSecRoot);    // 构建二级目录，从第二行，第二列开始
                m_DevTree.SetItemData(s_DirName_Number->root, SECOND_DIR);
				m_DevTree.SetItemColor(s_DirName_Number->root, RGB_OFFLine);
                m_lstSecDirInfos.push_back(s_DirName_Number);                       // 保存目录链表 
                RWExcel.SetCellString(s_DirName_Number->nSecDirNumber,2,Dlg.m_strName);
				m_DevTree.Invalidate(FALSE);
           }
           break;
        }
    case VIDEO_MENU_STARTMAINPLAY:
    case VIDEO_MENU_STARTEXTRAPLAY:													// 开始预览
        {
            HTREEITEM CurrentItem;
            HTREEITEM ParentItem;
            CurrentItem = m_DevTree.GetSelectedItem();
            ParentItem = m_DevTree.GetParentItem(CurrentItem);
            if (NULL != ParentItem)
            {
                DH_DEVICE_INFO* pInfo = Find(ParentItem);
                if (NULL != pInfo)
                {
                    m_lLoginID = pInfo->lLoginID;
                    m_nChannel = m_DevTree.GetItemData(CurrentItem);
                    HWND hWnd = GetDispHandle();
                    if (0 != hWnd)																
                    {
                        if (0 !=m_DispHanle[m_CurScreen])						   // 当前窗口有正在播放，先关闭原有通道
                        {
                            CLIENT_StopRealPlayEx(m_DispHanle[m_CurScreen]);
                            m_DispHanle[m_CurScreen] = 0;
                            m_stuVideoInfo[m_CurScreen].nChannel = 0;
                            m_stuVideoInfo[m_CurScreen].lLoginID = 0;
                            m_stuPlayInfo[m_CurScreen].root = 0;
                            m_stuPlayInfo[m_CurScreen].bPlay = FALSE;
                            m_stuPlayInfo[m_CurScreen].hParent = 0;
                            m_stuPlayInfo[m_CurScreen].nChannel = -1;
							m_stuPlayInfo[m_CurScreen].bFrameType = 0;

                            Invalidate();
                        }
                        for (int i = 0; i<9;i++)								   // 查找等到播放的通道是否在其他窗口上播放，有则先关闭
                        {
                            if (m_stuPlayInfo[i].root == CurrentItem)
                            {
                                int nDisHandle = i;
                                if (m_stuPlayInfo[i].bPlay)
                                {
                                    CLIENT_StopRealPlayEx(m_DispHanle[nDisHandle]);
                                    m_DispHanle[nDisHandle] = 0;
                                    m_stuVideoInfo[nDisHandle].nChannel = 0;
                                    m_stuVideoInfo[nDisHandle].lLoginID = 0;
                                    m_stuPlayInfo[i].root = 0;
                                    m_stuPlayInfo[i].bPlay = FALSE;
                                    m_stuPlayInfo[i].hParent = 0;
                                    m_stuPlayInfo[i].nChannel = -1;
									m_stuPlayInfo[i].bFrameType = 0;
                                    Invalidate();
                                }
                            }
                        }
                        long lRet = 0;
                        if(VIDEO_MENU_STARTMAINPLAY == nID)
                        {
                            lRet = CLIENT_RealPlayEx(m_lLoginID,m_nChannel,hWnd,DH_RType_Realplay_0);
                        }
                        else
                        {
                            lRet = CLIENT_RealPlayEx(m_lLoginID,m_nChannel,hWnd,DH_RType_Realplay_1);
                        }
                        if(0 != lRet)
                        {
/*							CLIENT_SetRealDataCallBack(lRet,RealDataCallBackEx,(LDWORD)this);*/
                            m_DispHanle[m_CurScreen]=lRet;
                            m_stuVideoInfo[m_CurScreen].lLoginID = m_lLoginID;
                            m_stuVideoInfo[m_CurScreen].nChannel = m_nChannel;
                            if (!m_stuPlayInfo[m_CurScreen].bPlay)
                            {
                                m_stuPlayInfo[m_CurScreen].root = CurrentItem;
                                m_stuPlayInfo[m_CurScreen].bPlay = TRUE;
                                m_stuPlayInfo[m_CurScreen].hParent = ParentItem;
                                m_stuPlayInfo[m_CurScreen].nChannel = m_nChannel;
								if (VIDEO_MENU_STARTMAINPLAY == nID)
								{
									m_stuPlayInfo[m_CurScreen].bFrameType = DH_RType_Realplay_0;
								}
								else
								{
									m_stuPlayInfo[m_CurScreen].bFrameType = DH_RType_Realplay_1;
								}
								if (m_CurScreen ==8)
								{
									m_ptzScreen.SetActiveWnd(0,TRUE);
									m_CurScreen = 0;
								}
								else
								{
									m_ptzScreen.SetActiveWnd(m_CurScreen+1,TRUE);
									m_CurScreen ++;
								}
                            }
                        }
                        else
                        {
                            strInfo = "Fail to play!";
                            PRINT_OPERATE(strInfo);
                            SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
                        }
                    }
                }
            }
            else
            {
                return;
            }
            break;
        }
    case VIDEO_MENU_STOPPLAY:													// 停止预览
        {
            HTREEITEM CurrentItem = m_DevTree.GetSelectedItem();
            for (int i = 0; i<9;i++)
            {
                if (m_stuPlayInfo[i].root == CurrentItem)
                {
                    int nDisHandle = i;
                    if (0 != m_DispHanle[nDisHandle])
                    {
                        CLIENT_StopRealPlayEx(m_DispHanle[nDisHandle]);
                        m_DispHanle[nDisHandle] = 0;
                        m_stuPlayInfo[i].root = 0;
                        m_stuPlayInfo[i].bPlay = FALSE;
                        m_stuPlayInfo[i].hParent = 0;
                        m_stuPlayInfo[i].nChannel = -1;
						m_stuPlayInfo[i].bFrameType = 0;
                        m_stuVideoInfo[nDisHandle].nChannel = 0;
                        m_stuVideoInfo[nDisHandle].lLoginID = 0;
                        Invalidate();
                    }
                }
            }
            break;
        }
    case VIDEO_MENU_ATTRIBUTE:													// 设备属性
        {
            HTREEITEM CurrentItem = m_DevTree.GetSelectedItem();
            DH_DEVICE_INFO* pInfo = Find(CurrentItem);
            if (pInfo != NULL)
            {
				m_lLoginID = pInfo->lLoginID;
                CDeviceInfo Dlg(NULL,m_lLoginID);
                if (2 == pInfo->nLoginStatus)
                {
                    Dlg.m_bStatus = TRUE;
                }
                Dlg.m_strDeviceName = pInfo->szDevName;
                Dlg.m_strID = pInfo->szDevSerial;
                Dlg.m_strIP = pInfo->szIP;
                Dlg.m_strPsw = pInfo->szPassWord;
                Dlg.m_nPort = pInfo->nPort;
                Dlg.m_strUserName = pInfo->szUseName;
                Dlg.m_bRadio = pInfo->nLoginType;
                if (Dlg.DoModal() == IDOK)														// 先删除vector中所存数据
                {
					CReadWriteExcel Excel;
					if (!Excel.SetFilePath(m_strPath))
					{
						return;
					}
					Excel.SetUseRange(1);
					for (int i = 2; i<= MAX_ROW; i++)
					{
						CString str = Excel.GetCellString(i,2);
						if (Dlg.m_strID != pInfo->szDevSerial && str == Dlg.m_strID)
						{
							strInfo.Format("Modify Fail! ID is Repeat!");
							PRINT_OPERATE(strInfo);
							SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
							MessageBox(ConvertString(strInfo,DLG_MAIN), ConvertString("Prompt"));
							return;
						}
					}
                    char szBuf[100];
                    memset(szBuf,0,100);
                    strncpy(szBuf,pInfo->szDevSerial,__min(strlen(pInfo->szDevSerial),99));
                    DeleteVector(m_vecIDInfo,szBuf);
                    memset(szBuf,0,100);
					strncpy(szBuf,pInfo->szIP,__min(strlen(pInfo->szIP),99));
                    DeleteVector(m_vecIPInfo,szBuf);
                    memset(szBuf,0,100);
					strncpy(szBuf,pInfo->szDevName,__min(strlen(pInfo->szDevName),99));
                    DeleteVector(m_vecNameInfo,szBuf);
                    m_csDeviceInfos.Lock();
                    pInfo->szDevName = Dlg.m_strDeviceName;
					pInfo->szDevSerial = Dlg.m_strID;
					if(Dlg.m_strIP.IsEmpty())
					{
						pInfo->szIP = UNREGISTER;
					}
					else
					{
						pInfo->szIP = Dlg.m_strIP;
					}
                    pInfo->szPassWord = Dlg.m_strPsw;
                    pInfo->nPort = Dlg.m_nPort;
                    pInfo->szUseName = Dlg.m_strUserName;
                    pInfo->nLoginType = Dlg.m_bRadio;
                    m_csDeviceInfos.UnLock();
                    memset(szBuf,0,100);														// 更新vector中所存数据									
                    strncpy(szBuf,pInfo->szDevSerial,__min(strlen(pInfo->szDevSerial),99));
                    m_vecIDInfo.push_back(szBuf);
                    memset(szBuf,0,100);
                    strncpy(szBuf,pInfo->szIP,__min(strlen(pInfo->szIP),99));
                    m_vecIPInfo.push_back(szBuf);
                    memset(szBuf,0,100);
                    strncpy(szBuf,pInfo->szDevName,__min(strlen(pInfo->szDevName),99));
                    m_vecNameInfo.push_back(szBuf);
                    
                    if (0 == m_nShowType)
                    {
                        m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szDevSerial);
                    }
                    else if (1 == m_nShowType)
                    {
                        m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szIP);
                    }
                    else if (2 == m_nShowType)
                    {
                        m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szDevName);
                    }
                    CString str;
                    Excel.SetCellString(pInfo->nDeviceRow,1,pInfo->strSecDirName);
                    Excel.SetCellString(pInfo->nDeviceRow,2,pInfo->szDevSerial);
                    Excel.SetCellString(pInfo->nDeviceRow,3,pInfo->szDevName);
                    Excel.SetCellString(pInfo->nDeviceRow,4,pInfo->szIP);
                    str.Format("%d",pInfo->nPort);
                    Excel.SetCellString(pInfo->nDeviceRow,5,str);
                    Excel.SetCellString(pInfo->nDeviceRow,6,pInfo->szUseName);
                    Excel.SetCellString(pInfo->nDeviceRow,7,pInfo->szPassWord);
                    str.Format("%d",pInfo->nLoginType);
                    Excel.SetCellString(pInfo->nDeviceRow,8,str);
                }
            }
            break;
        }
    case VIDEO_MENU_DELETEDEV:																// 删除设备
        {
			m_bDeleteDev = TRUE;
            break;  
        }
    case VIDEO_MENU_DELETEDIR:																// 删除目录
        {
            HTREEITEM CurrentItem = m_DevTree.GetSelectedItem();
            if (NULL != CurrentItem)
            {
                HTREEITEM hChildItem = m_DevTree.GetChildItem(CurrentItem);
                if (NULL != hChildItem)
                {
                    PRINT_WARNING("Delete Fail! Second Directory has a Child Item!");
                    return;
                }
            }
            m_DevTree.DeleteItem(CurrentItem);
            CReadWriteExcel RWExcel;
            if (!RWExcel.SetFilePath(m_strPath))
            {
                return;
            }
            std::list<DIR_NAME_NUMBER*>::iterator SecondDir = m_lstSecDirInfos.begin();      // 首先修改sheet2中的目录名称
            for( ; SecondDir != m_lstSecDirInfos.end(); ++SecondDir)
            {
                DIR_NAME_NUMBER* pInfo = *SecondDir;
                if (pInfo != NULL)
                {
                    if (pInfo->root == CurrentItem)                                     
                    {
                        m_mapDirName.erase(pInfo->strSecDirName);
                        RWExcel.SetUseRange(2);
                        RWExcel.SetCellString(pInfo->nSecDirNumber,2,SET_NULL);
						m_lstSecDirInfos.erase(SecondDir);
                        break;
                    }
                }
            }
            break;
        }
    case VIDEO_MENU_LOGIN:																	// 登录设备
        {
            HTREEITEM CurrentItem = m_DevTree.GetSelectedItem();
            DH_DEVICE_INFO* pInfo = Find(CurrentItem);
            if (NULL != pInfo)
            {
				m_csDeviceInfos.Lock();
				pInfo->nLoginStatus = 1;
				pInfo->nLoginTimes = 1;
				m_csDeviceInfos.UnLock();
			}
			break;
		}
    case VIDEO_MENU_LOGOUT:																	// 登出
        {
            HTREEITEM CurrentItem = m_DevTree.GetSelectedItem();
            DH_DEVICE_INFO* pInfo = Find(CurrentItem);
            if (NULL != pInfo)
            {
                CLIENT_Logout(pInfo->lLoginID);
                m_DevTree.SetItemColor(pInfo->DstRoot, RGB_OFFLine);
                m_csDeviceInfos.Lock();
                pInfo->lLoginID = 0;
                pInfo->nLoginStatus = 0;
				pInfo->nLoginTimes = 0;
                m_csDeviceInfos.UnLock();
				for(int i = 0 ; i<9; i++)
				{
					if (CurrentItem == m_stuPlayInfo[i].hParent)
					{
						m_stuPlayInfo[i].root = 0;
						m_stuPlayInfo[i].bPlay = FALSE;
						m_stuPlayInfo[i].hParent = 0;
						m_stuPlayInfo[i].nChannel = -1;
						m_stuPlayInfo[i].bFrameType = 0;
						
						break;
					}
				}
            }
            Invalidate(FALSE);
            break;
        }
    case VIDEO_MENU_SORT:																	// 设备排序
        {
			HTREEITEM hRoot = m_DevTree.GetSelectedItem();
			if (ROOT_DIR == m_DevTree.GetItemData(hRoot))
			{
				m_bSort = TRUE;
				m_bSorted = TRUE;
			}
			else if (SECOND_DIR == m_DevTree.GetItemData(hRoot))
			{
				m_bDirSort = TRUE;
			}
            break;
        }
    default:
        break;
    }
}

void CActiveLoginDemoDlg::DirSort()
{
	HTREEITEM hRoot = m_DevTree.GetSelectedItem();
	HTREEITEM hDstRoot = 0;
	DeleteDeviceDir(m_DevTree.GetChildItem(hRoot));
	std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();						// 遍历链表，所有设备排序状态置false
	for (; Device != m_lstDeviceInfos.end(); ++ Device)
	{
		DH_DEVICE_INFO* pDevice = *Device;
		if (NULL != pDevice)
		{
			m_csDeviceInfos.Lock();
			pDevice->bSort = FALSE;
			m_csDeviceInfos.UnLock();
		}

	}
	std::sort(m_vecIDInfo.begin(),m_vecIDInfo.end());
	std::sort(m_vecIPInfo.begin(),m_vecIPInfo.end());
	std::sort(m_vecNameInfo.begin(),m_vecNameInfo.end());
	switch(m_nShowType)														    // 0按ID排序。1按IP排序。2按名称排序
	{
	case 0:
		{
			std::vector<std::string>::iterator st = m_vecIDInfo.begin();						// 遍历排序vector
			for(; st != m_vecIDInfo.end(); st++)
			{
				std::string str = *st;

				std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();			// 遍历链表.查找符合vector顺序的设备
				for (; Device != m_lstDeviceInfos.end(); ++ Device)
				{
					DH_DEVICE_INFO* pDevice = *Device;
					if (NULL != pDevice)
					{
						// 判断条件1.ID和vector中元素相同，2.为避免设备重复ID情况下重复添加，需要判断是否已排过序，3.只排此目录下设备 4.只排在设备树上的
						if (pDevice->szDevSerial == str.c_str() && !pDevice->bSort && pDevice->srcRoot == hRoot)		
						{
							hDstRoot = pDevice->DstRoot;
							pDevice->bSort = TRUE;
							pDevice->DstRoot = Initdirectory(pDevice->szDevSerial,pDevice->srcRoot);
							pDevice->bOnTree = TRUE;
							if (pDevice->nLoginStatus == 2)
							{
								m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
								m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
								CString str;
								CString strChannel = CHANNEL;

								for (int i= 0; i<pDevice->nChannelNum; i++)
								{
									str.Format("%d",i+1);
									str = strChannel + str;
									HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
									m_DevTree.SetItemData(ChannelRoot,i);
									m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
									for (int h = 0; h<9 ; h++)
									{
										if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hDstRoot))
										{
											m_stuPlayInfo[h].hParent = pDevice->DstRoot;
											m_stuPlayInfo[h].root = ChannelRoot;
											break;
										}
									}
								}
							}
							else
							{
								m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
								m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
							}
						}
					}
				}
			}
			break;
		}
	case 1:
		{
			std::vector<std::string>::iterator st = m_vecIPInfo.begin();										// 遍历排序vector
			for(; st != m_vecIPInfo.end(); st++)
			{
				std::string str = *st;
				std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();							// // 遍历链表.查找符合vector顺序的设备
				for (; Device != m_lstDeviceInfos.end(); ++ Device)
				{
					DH_DEVICE_INFO* pDevice = *Device;
					if (NULL != pDevice)
					{
						// 判断条件1.IP和vector中元素相同，2.为避免设备重复IP情况下重复添加，需要判断是否已排过序，3.只排此目录下设备
						if ((!pDevice->bSort) && (pDevice->srcRoot == hRoot) && (pDevice->szIP!= UNREGISTER) && (pDevice->szIP == str.c_str()) )
						{
							hDstRoot = pDevice->DstRoot;
							pDevice->bSort = TRUE;
							pDevice->DstRoot = Initdirectory(pDevice->szIP,pDevice->srcRoot);
							pDevice->bOnTree = TRUE;
							if (pDevice->nLoginStatus == 2)
							{
								m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
								m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
								CString str;
								CString strChannel = CHANNEL;
								HTREEITEM ChannelRoot = NULL;

								for (int i= 0; i<pDevice->nChannelNum; i++)
								{
									str.Format("%d",i+1);
									str = strChannel + str;
									HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
									m_DevTree.SetItemData(ChannelRoot,i);
									m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
									for (int h = 0; h<9 ; h++)
									{
										if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hDstRoot))
										{
											m_stuPlayInfo[h].hParent = pDevice->DstRoot;
											m_stuPlayInfo[h].root = ChannelRoot;
											break;
										}
									}
								}
							}
							else
							{
								m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
								m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
							}
						}
					}
				}
			}
			std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
			for (; Device != m_lstDeviceInfos.end(); ++ Device)
			{
				DH_DEVICE_INFO* pDevice = *Device;
				if (NULL != pDevice)
				{
					if (pDevice->szIP == UNREGISTER && (pDevice->srcRoot == hRoot))
					{
						pDevice->DstRoot = Initdirectory(pDevice->szIP,pDevice->srcRoot);
						pDevice->bOnTree = TRUE;
						m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
						m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
					}
				}

			}
			break;
		}
	case 2:
		{
			std::vector<std::string>::iterator st = m_vecNameInfo.begin();
			for(; st != m_vecNameInfo.end(); st++)
			{
				std::string str = *st;

				std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
				for (; Device != m_lstDeviceInfos.end(); ++ Device)
				{
					DH_DEVICE_INFO* pDevice = *Device;
					if (NULL != pDevice)
					{
						// 判断条件1.ID和vector中元素相同，2.为避免设备重复ID情况下重复添加，需要判断是否已排过序，3.只排此目录下设备
						if ((!pDevice->bSort) && (pDevice->srcRoot == hRoot) && (pDevice->szDevName == str.c_str()) && (pDevice->szDevName != NONAME))
						{
							hDstRoot = pDevice->DstRoot;
							PRINT_WARNING("Name :%s",pDevice->szDevName);
							pDevice->bSort = TRUE;
							pDevice->DstRoot = Initdirectory(pDevice->szDevName,pDevice->srcRoot);
							pDevice->bOnTree = TRUE;
							if (pDevice->nLoginStatus == 2)
							{
								m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
								m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
								CString str;
								CString strChannel = CHANNEL;
								HTREEITEM ChannelRoot = NULL;

								for (int i= 0; i<pDevice->nChannelNum; i++)
								{
									str.Format("%d",i+1);
									str = strChannel + str;
									HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
									m_DevTree.SetItemData(ChannelRoot,i);
									m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
									for (int h = 0; h<9 ; h++)
									{
										if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hDstRoot))
										{
											m_stuPlayInfo[h].hParent = pDevice->DstRoot;
											m_stuPlayInfo[h].root = ChannelRoot;
											break;
										}
									}
								}
							}
							else
							{
								m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
								m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
							}
						}
					}
				}
			}
			std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
			for (; Device != m_lstDeviceInfos.end(); ++ Device)
			{
				DH_DEVICE_INFO* pDevice = *Device;
				if (NULL != pDevice)
				{
					if (pDevice->szDevName == NONAME && (pDevice->srcRoot == hRoot))
					{
						hDstRoot = pDevice->DstRoot;
						pDevice->DstRoot = Initdirectory(pDevice->szDevName,pDevice->srcRoot);
						pDevice->bOnTree = TRUE;
						if (pDevice->nLoginStatus == 2)
						{
							m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
							m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
							CString str;
							CString strChannel = CHANNEL;
							HTREEITEM ChannelRoot = NULL;

							for (int i= 0; i<pDevice->nChannelNum; i++)
							{
								str.Format("%d",i+1);
								str = strChannel + str;
								HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
								m_DevTree.SetItemData(ChannelRoot,i);
								m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
								for (int h = 0; h<9 ; h++)
								{
									if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hDstRoot))
									{
										m_stuPlayInfo[h].hParent = pDevice->DstRoot;
										m_stuPlayInfo[h].root = ChannelRoot;
										break;
									}
								}
							}
						}
						else
						{
							m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
							m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
						}
					}
				}

			}
			break;
		}
	default:
		break;
	}
	 OnSelchangeComboShowtype();
	m_DevTree.Invalidate();
	m_bDirSort = FALSE;
}
void CActiveLoginDemoDlg::Sort()
{
    m_DevTree.DeleteAllItems();								 // 清空整颗树
    HTREEITEM root = m_DevTree.InsertItem(m_strRootName);    // 首先重新构建一级根目录
    m_DevTree.SetItemData(root, ROOT_DIR);                   // 根目录值设置为-100
    std::list<DIR_NAME_NUMBER*>::iterator it = m_lstSecDirInfos.begin();							// 构建二级目录
    int count = 0;
    for( ; it != m_lstSecDirInfos.end(); ++it)
    {
        DIR_NAME_NUMBER* pInfo = *it;
        if (NULL != pInfo)
        {
            pInfo->root = Initdirectory(pInfo->strSecDirName, root);
            m_DevTree.SetItemData(pInfo->root,SECOND_DIR);
            m_DevTree.SetItemColor(pInfo->root,RGB_OFFLine);
        }
    }
    m_DevTree.Expand(root,TVE_EXPAND);
    std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();					
    for ( ; Device != m_lstDeviceInfos.end(); ++Device)
    {
        DH_DEVICE_INFO* pInfo = *Device;
        if (pInfo != NULL)
        {
            std::list<DIR_NAME_NUMBER*>::iterator it = m_lstSecDirInfos.begin();            // 根据第一列目录名称，查找设备所属的目录等级和目录序号
            BOOL bFindName = FALSE;
            while( it != m_lstSecDirInfos.end())
            {
                DIR_NAME_NUMBER* pName = *it;
                ++it;
                if (pName->strSecDirName == pInfo->strSecDirName)
                {
                    bFindName = TRUE;
                    pInfo->srcRoot = pName->root;
                    break;
                }
            }
            if (!bFindName)
            {
                pInfo->srcRoot = root;
            }
            pInfo->bSort = FALSE;
        }
    }
	std::sort(m_vecIDInfo.begin(),m_vecIDInfo.end());
	std::sort(m_vecIPInfo.begin(),m_vecIPInfo.end());
	std::sort(m_vecNameInfo.begin(),m_vecNameInfo.end());
    switch(m_nShowType)
    {
    case 0:
        {
            std::vector<std::string>::iterator st = m_vecIDInfo.begin();
            for(; st != m_vecIDInfo.end(); st++)
            {
                std::string str = *st;
                
                std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
                for (; Device != m_lstDeviceInfos.end(); ++ Device)
                {
                    DH_DEVICE_INFO* pDevice = *Device;
                    if (NULL != pDevice)
                    {
                        if (pDevice->szDevSerial == str.c_str() && !pDevice->bSort)
                        {
                            HTREEITEM hRoot = pDevice->DstRoot;
                            pDevice->bSort = TRUE;
                            pDevice->DstRoot = Initdirectory(pDevice->szDevSerial,pDevice->srcRoot);
							pDevice->bOnTree = TRUE;
                            if (pDevice->nLoginStatus == 2)
                            {
                                m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
                                m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                                CString str;
                                CString strChannel = CHANNEL;
                                HTREEITEM ChannelRoot = NULL;
                                
                                for (int i= 0; i<pDevice->nChannelNum; i++)
                                {
                                    str.Format("%d",i+1);
                                    str = strChannel + str;
                                    HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
                                    m_DevTree.SetItemData(ChannelRoot,i);
                                    m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
                                    for (int h = 0; h<9 ; h++)
                                    {
                                        if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hRoot))
                                        {
                                            m_stuPlayInfo[h].hParent = pDevice->DstRoot;
                                            m_stuPlayInfo[h].root = ChannelRoot;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
                                m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                            }
                        }
                    }
                }
            }
            break;
        }
    case 1:
        {
            std::vector<std::string>::iterator st = m_vecIPInfo.begin();
            for(; st != m_vecIPInfo.end(); st++)
            {
                std::string str = *st;
                std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
                for (; Device != m_lstDeviceInfos.end(); ++ Device)
                {
                    DH_DEVICE_INFO* pDevice = *Device;
                    if (NULL != pDevice)
                    {
                      
                        if ( (pDevice->szIP == str.c_str()) && (!pDevice->bSort) && (pDevice->szIP!= UNREGISTER))
                        {
                            HTREEITEM hRoot = pDevice->DstRoot;
                            pDevice->bSort = TRUE;
                            pDevice->DstRoot = Initdirectory(pDevice->szIP,pDevice->srcRoot);
							pDevice->bOnTree = TRUE;
                            if (pDevice->nLoginStatus == 2)
                            {
                                m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
                                m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                                CString str;
                                CString strChannel = CHANNEL;
                                HTREEITEM ChannelRoot = NULL;
                                
                                for (int i= 0; i<pDevice->nChannelNum; i++)
                                {
                                    str.Format("%d",i+1);
                                    str = strChannel + str;
                                    HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
                                    m_DevTree.SetItemData(ChannelRoot,i);
                                    m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
                                    for (int h = 0; h<9 ; h++)
                                    {
                                        if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hRoot))
                                        {
                                            m_stuPlayInfo[h].hParent = pDevice->DstRoot;
                                            m_stuPlayInfo[h].root = ChannelRoot;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
                                m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                            }
                        }
                    }
                }
            }
            std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
            for (; Device != m_lstDeviceInfos.end(); ++ Device)
            {
                DH_DEVICE_INFO* pDevice = *Device;
                if (NULL != pDevice)
                {
                    if (pDevice->szIP == UNREGISTER)
                    {
                        pDevice->DstRoot = Initdirectory(pDevice->szIP,pDevice->srcRoot);
						pDevice->bOnTree = TRUE;
                        m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
                        m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                    }
                }
                
            }
            break;
        }
    case 2:
        {
            std::vector<std::string>::iterator st = m_vecNameInfo.begin();
            for(; st != m_vecNameInfo.end(); st++)
            {
                std::string str = *st;
                
                std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
                for (; Device != m_lstDeviceInfos.end(); ++ Device)
                {
                    DH_DEVICE_INFO* pDevice = *Device;
                    if (NULL != pDevice)
                    {
                        if ( (pDevice->szDevName == str.c_str())&& (!pDevice->bSort) && (pDevice->szDevName != NONAME))
                        {
                            HTREEITEM hRoot = pDevice->DstRoot;
                            pDevice->bSort = TRUE;
                            pDevice->DstRoot = Initdirectory(pDevice->szDevName,pDevice->srcRoot);
							pDevice->bOnTree = TRUE;
                            if (pDevice->nLoginStatus == 2)
                            {
                                m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
                                m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                                CString str;
                                CString strChannel = CHANNEL;
                                HTREEITEM ChannelRoot = NULL;
                                
                                for (int i= 0; i<pDevice->nChannelNum; i++)
                                {
                                    str.Format("%d",i+1);
                                    str = strChannel + str;
                                    HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
                                    m_DevTree.SetItemData(ChannelRoot,i);
                                    m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
                                    for (int h = 0; h<9 ; h++)
                                    {
                                        if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hRoot))
                                        {
                                            m_stuPlayInfo[h].hParent = pDevice->DstRoot;
                                            m_stuPlayInfo[h].root = ChannelRoot;
                                            break;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
                                m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
                            }
                        }
                    }
                }
            }
			std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
			for (; Device != m_lstDeviceInfos.end(); ++ Device)
			{
				DH_DEVICE_INFO* pDevice = *Device;
				if (NULL != pDevice)
				{
					if (pDevice->szDevName == NONAME)
					{
						HTREEITEM hRoot = pDevice->DstRoot;
						pDevice->DstRoot = Initdirectory(pDevice->szDevName,pDevice->srcRoot);
						pDevice->bOnTree = TRUE;
						if (pDevice->nLoginStatus == 2)
						{
							m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OnLine);
							m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
							CString str;
							CString strChannel = CHANNEL;
							HTREEITEM ChannelRoot = NULL;

							for (int i= 0; i<pDevice->nChannelNum; i++)
							{
								str.Format("%d",i+1);
								str = strChannel + str;
								HTREEITEM ChannelRoot = Initdirectory(str,pDevice->DstRoot);
								m_DevTree.SetItemData(ChannelRoot,i);
								m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
								for (int h = 0; h<9 ; h++)
								{
									if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hRoot))
									{
										m_stuPlayInfo[h].hParent = pDevice->DstRoot;
										m_stuPlayInfo[h].root = ChannelRoot;
										break;
									}
								}
							}
						}
						else
						{
							m_DevTree.SetItemColor(pDevice->DstRoot,RGB_OFFLine);
							m_DevTree.SetItemData(pDevice->DstRoot,DEVICE_DIR);
						}
					}
				}

			}
            break;
        }
    default:
        break;
    }
    m_bSort = FALSE;
    OnSelchangeComboShowtype();
    m_DevTree.Invalidate();
}

void CActiveLoginDemoDlg::DeleteDev()
{
	HTREEITEM CurrentItem = m_DevTree.GetSelectedItem();
	DH_DEVICE_INFO* pInfo = NULL;
	std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
	for( ; Device != m_lstDeviceInfos.end(); ++Device)
	{
		pInfo = *Device;
		if (NULL != pInfo && CurrentItem == pInfo->DstRoot)
		{
			break;
		}
	}
	if (NULL == pInfo || Device == m_lstDeviceInfos.end())
	{
		m_bDeleteDev = FALSE;
		return;
	}
	int iRow = 0;
	iRow = pInfo->nDeviceRow;
	CReadWriteExcel Excel;
	if (!Excel.SetFilePath(m_strPath))
	{
		PRINT_WARNING("SetFilePath Error!");
		m_bDeleteDev = FALSE;
		return;
	}
	Excel.SetUseRange(1);
	Excel.SetCellString(iRow,1,SET_NULL);
	Excel.SetCellString(iRow,2,SET_NULL);
	Excel.SetCellString(iRow,3,SET_NULL);
	Excel.SetCellString(iRow,4,SET_NULL);
	Excel.SetCellString(iRow,5,SET_NULL);
	Excel.SetCellString(iRow,6,SET_NULL);
	Excel.SetCellString(iRow,7,SET_NULL);
	Excel.SetCellString(iRow,8,SET_NULL);
	char szBuf[100];
	memset(szBuf,0,100);
	strncpy(szBuf,pInfo->szDevSerial,__min(strlen(pInfo->szDevSerial),99));
	DeleteVector(m_vecIDInfo,szBuf);
	memset(szBuf,0,100);
	strncpy(szBuf,pInfo->szIP,__min(strlen(pInfo->szIP),99));
	DeleteVector(m_vecIPInfo,szBuf);
	memset(szBuf,0,100);
	strncpy(szBuf,pInfo->szDevName,__min(strlen(pInfo->szDevName),99));
	DeleteVector(m_vecNameInfo,szBuf);

	CLIENT_Logout(pInfo->lLoginID);
	Invalidate(FALSE);
	m_csDeviceInfos.Lock();
	delete pInfo;
	pInfo = NULL;
	m_lstDeviceInfos.erase(Device);
	m_csDeviceInfos.UnLock();
	CString str;
	m_DevTree.DeleteItem(CurrentItem);
	m_bDeleteDev = FALSE;
}
void CActiveLoginDemoDlg::ReName(HTREEITEM root, CString strName)
{
    CString str = m_DevTree.GetItemText(root);
	if (str == strName)
	{
		return;
	}
	int nDir = m_DevTree.GetItemData(root);
    CReadWriteExcel RWExcel;
    if (!RWExcel.SetFilePath(m_strPath))
    {
        return;
    }
    switch(nDir)
    {
    case ROOT_DIR:
        {
			m_strRootName = strName;
            RWExcel.SetUseRange(2);
            RWExcel.SetCellString(2,1,strName);
			m_DevTree.SetItemText(root,strName);
            break;
        }
    case SECOND_DIR:
        {
			map<CString,char>::iterator it = m_mapDirName.find(strName);
			if (it != m_mapDirName.end())
			{
				CString strInfo = "Modify Fail! Name is Repeat!";
				PRINT_OPERATE(strInfo);
				SetDlgItemText(IDC_STATIC_INFO,ConvertString(strInfo,DLG_MAIN));
				MessageBox(ConvertString(strInfo,DLG_MAIN), ConvertString("Prompt"));
				return;
			}
            m_mapDirName.erase(str);
            m_mapDirName.insert(pair<CString,char>(strName,'0'));
			m_DevTree.SetItemText(root,strName);
            std::list<DIR_NAME_NUMBER*>::iterator SecondDir = m_lstSecDirInfos.begin();      // 首先修改sheet2中的目录名称
            for( ; SecondDir != m_lstSecDirInfos.end(); ++SecondDir)
            {
                DIR_NAME_NUMBER* pInfo = *SecondDir;
                if (pInfo != NULL)
                {
                    if (pInfo->root == root)                                     
                    {
                        pInfo->strSecDirName = strName;
                        RWExcel.SetUseRange(2);
                        RWExcel.SetCellString(pInfo->nSecDirNumber,2,strName);
                        break;
                    }
                }
            }
            std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();             // 首先修改sheet1设备关联的目录名称
            for( ; Device != m_lstDeviceInfos.end(); ++Device)
            {
                
                DH_DEVICE_INFO* pInfo = *Device;
                if (pInfo != NULL)
                {
                    if (pInfo->srcRoot == root)
                    {
						m_csDeviceInfos.Lock();
                        pInfo->strSecDirName = strName;
						m_csDeviceInfos.UnLock();
                        RWExcel.SetUseRange(1);
                        RWExcel.SetCellString(pInfo->nDeviceRow,1,strName);
                    }
                }
            }  
        }
    default:
        break;
    }
}

void CActiveLoginDemoDlg::FindItem(HTREEITEM hRoot,int ShowType)
{
	if (NULL != hRoot)
	{
		if (m_DevTree.GetItemData(hRoot) == DEVICE_DIR)
		{
			DH_DEVICE_INFO* pInfo = Find(hRoot);
			if (NULL != pInfo)
			{
				if (0 == ShowType)
				{
					m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szDevSerial);
				}
				else if (1 == ShowType)
				{
					m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szIP);
				}
				else if (2 == m_nShowType)
				{
					m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szDevName);
				}
			}
		}
		else
		{
			m_DevTree.SetItemColor(hRoot, RGB_OFFLine);
		}
	}
	HTREEITEM hNextRoot = m_DevTree.GetChildItem(hRoot);
	while(hNextRoot != NULL)
	{
		FindItem(hNextRoot,ShowType);
		hNextRoot = m_DevTree.GetNextSiblingItem(hNextRoot);
	}
}

void CActiveLoginDemoDlg::OnSelchangeComboShowtype() 
{
	m_nShowType = m_Cmb_ShowType.GetCurSel();
	CString str;

	if (0 == m_nShowType)
    {
        str = ConvertString("SearchByID");
        SetDlgItemText(IDC_STATIC1,str);
    }
    else if (1 == m_nShowType)
    {
		str = ConvertString("SearchByIP");
        SetDlgItemText(IDC_STATIC1,str);
    }
    else if (2 == m_nShowType)
    {
		str = ConvertString("SearchByName");
        SetDlgItemText(IDC_STATIC1,str);
    }
	HTREEITEM hRoot = m_DevTree.GetRootItem();
	FindItem(hRoot,m_nShowType);
	m_DevTree.SetItemText(hRoot,m_strRootName);
}

void CActiveLoginDemoDlg::DeleteEmptyDir(HTREEITEM root)
{
	HTREEITEM hNextRoot = m_DevTree.GetNextSiblingItem(root);
	if (NULL != m_DevTree.GetChildItem(root))
	{
		m_DevTree.Expand(root,TVE_EXPAND);
	}
	else if (SECOND_DIR  == m_DevTree.GetItemData(root))
	{
		m_DevTree.DeleteItem(root);
	}
	if (NULL != hNextRoot)
	{
		DeleteEmptyDir(hNextRoot);
	}
}

void CActiveLoginDemoDlg::DeleteDeviceDir(HTREEITEM root)
{
	HTREEITEM hNextRoot = m_DevTree.GetNextSiblingItem(root);

	m_DevTree.DeleteItem(root);

	if (NULL != hNextRoot)
	{
		DeleteDeviceDir(hNextRoot);
	}
}

void CActiveLoginDemoDlg::Search()
{
	m_DevTree.DeleteAllItems();

	HTREEITEM root = m_DevTree.InsertItem(m_strRootName);    // 首先构建一级根目录
	m_DevTree.SetItemData(root, ROOT_DIR);                   // 根目录值设置为-100
	std::list<DIR_NAME_NUMBER*>::iterator it = m_lstSecDirInfos.begin();
	int count = 0;
	for( ; it != m_lstSecDirInfos.end(); ++it)
	{
		DIR_NAME_NUMBER* pInfo = *it;
		if (NULL != pInfo)
		{
			pInfo->root = Initdirectory(pInfo->strSecDirName, root);
			m_DevTree.SetItemData(pInfo->root,SECOND_DIR);
			m_DevTree.SetItemColor(pInfo->root,RGB_OFFLine);
		}
	}
	m_DevTree.Expand(root,TVE_EXPAND);
	std::list<DH_DEVICE_INFO*>::iterator Device = m_lstDeviceInfos.begin();
	for ( ; Device != m_lstDeviceInfos.end(); ++Device)
	{
		DH_DEVICE_INFO* pInfo = *Device;
		if (pInfo != NULL)
		{
			std::list<DIR_NAME_NUMBER*>::iterator it = m_lstSecDirInfos.begin();            // 根据第一列目录名称，查找设备所属的目录等级和目录序号
			BOOL bFindName = FALSE;
			while( it != m_lstSecDirInfos.end())
			{
				DIR_NAME_NUMBER* pName = *it;
				++it;
				if (pName->strSecDirName == pInfo->strSecDirName)
				{
					bFindName = TRUE;
					m_csDeviceInfos.Lock();
					pInfo->srcRoot = pName->root;
					m_csDeviceInfos.UnLock();
					break;
				}
			}
			if (!bFindName)
			{
				pInfo->srcRoot = root;
			}
			CString str;
			if (0 == m_nShowType)
			{
				str = pInfo->szDevSerial;
			}
			else if (1 == m_nShowType)
			{
				str = pInfo->szIP;
			}
			else if (2 == m_nShowType)
			{
				str = pInfo->szDevName;
			}

			int nFind = str.Find(m_strSearch);
			if (m_strSearch.IsEmpty() || (-1 != nFind))
			{
				HTREEITEM hRoot = pInfo->DstRoot;
				pInfo->DstRoot = Initdirectory(pInfo->szDevSerial, pInfo->srcRoot);
				pInfo->bOnTree = TRUE;
				if (0 == m_nShowType)
				{
					m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szDevSerial);
				}
				else if (1 == m_nShowType)
				{
					m_DevTree.SetItemText(pInfo->DstRoot, pInfo->szIP);
				}
				else if (2 == m_nShowType)
				{
					m_DevTree.SetItemText(pInfo->DstRoot,pInfo->szDevName);
				}
				if (pInfo->nLoginStatus == 2)
				{
					m_DevTree.SetItemColor(pInfo->DstRoot,RGB_OnLine);
					CString str;
					CString strChannel = CHANNEL;
					HTREEITEM ChannelRoot = NULL;
					while(TRUE)                                                                 // 添加通道节点前先删除一次，以免重复添加
					{
						ChannelRoot = m_DevTree.GetChildItem(pInfo->DstRoot);
						if (NULL != ChannelRoot)
						{
							m_DevTree.DeleteItem(ChannelRoot);
						}
						else
						{
							break;
						}
					}
					for (int i= 0; i<pInfo->nChannelNum; i++)
					{
						str.Format("%d",i+1);
						str = strChannel + str;
						HTREEITEM ChannelRoot = Initdirectory(str,pInfo->DstRoot);
						m_DevTree.SetItemData(ChannelRoot,i);
						m_DevTree.SetItemColor(ChannelRoot,RGB_OFFLine);
						for (int h = 0; h<9 ; h++)
						{
							if ((m_stuPlayInfo[h].nChannel == i) && (m_stuPlayInfo[h].hParent == hRoot))
							{
								m_stuPlayInfo[h].hParent = pInfo->DstRoot;
								m_stuPlayInfo[h].root = ChannelRoot;
								break;
							}
						}
					}
				}
				else
				{
					m_DevTree.SetItemColor(pInfo->DstRoot,RGB_OFFLine);
				}
				m_DevTree.SetItemData(pInfo->DstRoot,DEVICE_DIR);
			}
			else
			{
				m_csDeviceInfos.Lock();
				pInfo->bOnTree = FALSE;
				m_csDeviceInfos.UnLock();
			}
		}
	}
	if (!m_strSearch.IsEmpty())
	{
		HTREEITEM hRoot = m_DevTree.GetChildItem(root);
		DeleteEmptyDir(hRoot);
	}
	m_bSearch = FALSE;
	m_DevTree.Invalidate();
}
void CActiveLoginDemoDlg::OnBtnSearch() 
{
	BOOL bValue = UpdateData(TRUE);
    if (bValue)
    {
		m_bSearch = TRUE;
    }
}

void CActiveLoginDemoDlg::OnBtnStopall() 
{
	for (int i= 0; i< 9; i++)
	{
        if (0 != m_DispHanle[i])
        {
            BOOL bRet = CLIENT_StopRealPlayEx(m_DispHanle[i]);
            if (bRet)
            {
                m_stuPlayInfo[i].root = 0;
                m_stuPlayInfo[i].bPlay = FALSE;
                m_stuPlayInfo[i].hParent = 0;
                m_stuPlayInfo[i].nChannel = -1;
				m_stuPlayInfo[i].bFrameType = 0;
                m_DispHanle[i] = 0;
            }
        }
	}
    Invalidate();
}

void CActiveLoginDemoDlg::OnSelchangeCmbSplit() 
{
    int nSplit = m_Cmb_Split.GetCurSel();
    m_ptzScreen.SetShowPlayWin(nSplit,0);
}


void CActiveLoginDemoDlg::OnDestroy()
{
	CDialog::OnDestroy();
}

void CActiveLoginDemoDlg::OnBnClickedHelp()
{
	HINSTANCE hRet = ShellExecute(NULL,"open",m_strPDFPath,NULL,NULL,SW_SHOWMAXIMIZED);
	if ((int)hRet == 2)
	{
		MessageBox(ConvertString("Open Fail,Please Checked!"), ConvertString("Prompt"));
	}
}
