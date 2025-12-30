// DemoMonitorWallDlg.cpp : implementation file
//

#include "stdafx.h"
#include <cmath>
#include "DemoMonitorWall.h"
#include "DemoMonitorWallDlg.h"
#include "LoginDlg.h"
#include "SpliceScreenConfigDlg.h"
#include "PowerDlg.h"
#include "CollectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	IDC_COMBOBOX_TV_OUT		5670
#define WM_DEVICE_DISCONNECT	(WM_USER + 100)
#define WM_DEVICE_RECONNECT		(WM_USER + 101)
// 工具栏分割按钮信息
struct SplitToolBarButtonInfo
{
	UINT		nSplit;		// 分割模式
	UINT		nID;		// 工具栏ID
};
static SplitToolBarButtonInfo g_ToolBarBtnSplit[] = 
{
	{1, ID_TOOLBAR_SPLIT1}, 
	{4, ID_TOOLBAR_SPLIT4}, 
	{8, ID_TOOLBAR_SPLIT8}, 
	{9, ID_TOOLBAR_SPLIT9}, 
	{16, ID_TOOLBAR_SPLIT16},
    {25, ID_TOOLBAR_SPLIT25},
    {36, ID_TOOLBAR_SPLIT36}, 
	{64, ID_TOOLBAR_SPLIT64}, 
	{DH_SPLIT_FREE, ID_TOOLBAR_SPLIT_FREE}, 
};

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallDlg dialog

CDemoMonitorWallDlg::CDemoMonitorWallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDemoMonitorWallDlg::IDD, pParent)
	, m_pDecoder(NULL)
{
	//{{AFX_DATA_INIT(CDemoMonitorWallDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDemoMonitorWallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDemoMonitorWallDlg)
	DDX_Control(pDX, IDC_TREE_DEVICE, m_treeDevice);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDemoMonitorWallDlg, CDialog)
	//{{AFX_MSG_MAP(CDemoMonitorWallDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_ADD_DEVICE, OnBtnAddDevice)
	ON_BN_CLICKED(IDC_BTN_REMOVE_DEVICE, OnBtnRemoveDevice)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DEVICE, OnDblclkTreeDevice)
	ON_CBN_SELCHANGE(IDC_COMBOBOX_TV_OUT, OnSelchangeComboTVOut)
	ON_MESSAGE(WM_DEVICE_DISCONNECT, OnDisConnect)
	ON_MESSAGE(WM_DEVICE_RECONNECT, OnReConnect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallDlg message handlers

BOOL CDemoMonitorWallDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (!CLIENT_Init(DisConnect, (LDWORD)this))
	{
		MsgBox0("NetSDK init error.");
	}
	LOG_SET_PRINT_INFO  stLogPrintInfo = {sizeof(stLogPrintInfo)};
	CLIENT_LogOpen(&stLogPrintInfo);
	CLIENT_SetAutoReconnect(HaveReConnect, (LDWORD)this);
	
	InitUI();

	LMIU(this);

	PostMessage(WM_COMMAND, ID_TOOLBAR_LOGIN, 0);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDemoMonitorWallDlg::InitUI()
{
	const int nSpace = 2;
	const int nWidthDeviceTree = 160;
	const int nDevBtnHeight = 20;
	
	// 工具栏
	m_wndToolBar.CreateEx(this);
	m_wndToolBar.LoadToolBar(IDR_TOOLBAR1);
	// RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_FIRST, 0, reposDefault, NULL, NULL, FALSE);
	
	m_wndToolBar.EnableButton(ID_TOOLBAR_LOGIN, TRUE);
	m_wndToolBar.EnableButton(ID_TOOLBAR_LOGOUT, FALSE);

	CRect rcClientStart;
	CRect rcClientNow;
	CRect rcTree, rcVideo, rcBtn;
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);
	CPoint ptOffset(rcClientNow.left - rcClientStart.left, rcClientNow.top - rcClientStart.top);

	rcTree.right = rcClientNow.right - nSpace;
	rcTree.left = rcTree.right - nWidthDeviceTree;
	rcTree.top = rcClientNow.top + nSpace;
	rcTree.bottom = rcClientNow.bottom - nDevBtnHeight - nSpace * 2;
	m_treeDevice.MoveWindow(rcTree);
	
	rcVideo.left = rcClientNow.left;
	rcVideo.top = rcTree.top;
	rcVideo.right = rcTree.left - 2;
	rcVideo.bottom = rcTree.bottom;
 	m_wndScreen.Create(NULL, NULL, WS_VISIBLE | WS_CHILD, rcVideo, this, 30000);

	rcBtn.left = rcTree.left;
	rcBtn.right = rcTree.left + (rcTree.Width() - nSpace) / 2;
	rcBtn.top = rcTree.bottom + nSpace;
	rcBtn.bottom = rcBtn.top + nDevBtnHeight;
	GetDlgItem(IDC_BTN_ADD_DEVICE)->MoveWindow(rcBtn);

	rcBtn.OffsetRect(rcBtn.Width() + nSpace, 0);
	GetDlgItem(IDC_BTN_REMOVE_DEVICE)->MoveWindow(rcBtn);

	m_wndSplit.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_BOTTOM);
	m_wndSplit.LoadToolBar(IDR_TOOLBAR2);

	int nIndex = m_wndSplit.CommandToIndex(ID_TOOLBAR_TV_OUT);
	m_wndSplit.SetButtonInfo(nIndex, ID_TOOLBAR_TV_OUT, TBBS_SEPARATOR, 200);
	m_cbTVOut.Create(WS_CHILD|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWNLIST, CRect(0,0,200,300), &m_wndSplit, IDC_COMBOBOX_TV_OUT);
	m_cbTVOut.SetFont(GetFont());
	
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_FIRST, 0);

	m_wndToolBar.SetButtonTitle(ID_TOOLBAR_LOGIN, LMIC("Login"));
	m_wndToolBar.SetButtonTitle(ID_TOOLBAR_LOGOUT, LMIC("Logout"));
	m_wndToolBar.SetButtonTitle(ID_TOOLBAR_SPLICE, LMIC("Config"));
	m_wndToolBar.SetButtonTitle(IDCANCEL, LMIC("Quit"));

	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT1, "1");
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT4, "4");
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT8, "8");
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT9, "9");
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT16, "16");
    m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT25, "25");
    m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT36, "36");
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT64, "64");

	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SPLIT_FREE, LMIC("Free"));
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_OPEN_WINDOW, LMIC("Open"));
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_CLOSE_WINDOW, LMIC("Close"));
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SET_LEVEL, LMIC("Top"));		
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_POWER, LMIC("Power"));	
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_COLLECTION, LMIC("Coll"));	
	m_wndSplit.SetButtonTitle(ID_TOOLBAR_SET_PUSH, LMIC("Push"));	

	// 隐藏电源控制, 预案管理
 	m_wndSplit.ShowButton(ID_TOOLBAR_POWER, FALSE);
 	m_wndSplit.ShowButton(ID_TOOLBAR_COLLECTION, FALSE);
	//m_wndSplit.GetToolBarCtrl().DeleteButton(m_wndSplit.GetToolBarCtrl().CommandToIndex(ID_TOOLBAR_SET_LEVEL) + 1);
}

BOOL CDemoMonitorWallDlg::DestroyWindow() 
{
	ClearDecoder();
	ClearEncoderTree();
	ClearEncoder();

	CLIENT_Cleanup();
	
	return CDialog::DestroyWindow();
}
// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDemoMonitorWallDlg::OnPaint() 
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
HCURSOR CDemoMonitorWallDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// 断线重连回调函数
void CALLBACK CDemoMonitorWallDlg::HaveReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if (0 == dwUser)
	{
		return;
	}	

	CDemoMonitorWallDlg *pThis = (CDemoMonitorWallDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_RECONNECT, (WPARAM)(lLoginID), NULL);	
}

LRESULT CDemoMonitorWallDlg::OnReConnect(WPARAM wParam, LPARAM lParam)
{
	LLONG lLoginID = (LLONG)wParam;
	if (m_pDecoder && lLoginID == m_pDecoder->GetHandle())
	{

	}
	else
	{
		CEncoderMap::const_iterator it = m_mapEncoder.find(lLoginID);
		if (it != m_mapEncoder.end())
		{
			CEncoder* pDev = it->second;
			pDev->Online(TRUE);
			
			HTREEITEM hItem = FindTreeNode(m_treeDevice, m_treeDevice.GetRootItem(), TreeItemData(TREE_ITEM_DEVICE, pDev));
			if (hItem)
			{
				m_treeDevice.SetItemText(hItem, pDev->GetIP() + (pDev->IsOnline() ? LMIC("(Online)") : LMIC("(Offline)")));
			}			
		}
	}
	return 0;
}

// 断线回调函数
void CALLBACK CDemoMonitorWallDlg::DisConnect( LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser )
{
	if (0 == dwUser)
	{
		return;
	}

	CDemoMonitorWallDlg *pThis = (CDemoMonitorWallDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	::PostMessage(hWnd, WM_DEVICE_DISCONNECT, WPARAM(lLoginID), LPARAM(pchDVRIP));		
}


//Callback function when device reconnect 
void CALLBACK ReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}

	CDemoMonitorWallDlg *pThis = (CDemoMonitorWallDlg *)dwUser;
	HWND hWnd = pThis->GetSafeHwnd();
	if (NULL == hWnd)
	{
		return;
	}
	PostMessage(hWnd, WM_DEVICE_RECONNECT, NULL, NULL);		
}


LRESULT CDemoMonitorWallDlg::OnDisConnect(WPARAM wParam, LPARAM lParam)
{
	LLONG lLoginID = (LLONG)wParam;
	char* szIp = (char*)lParam;
	if (m_pDecoder && lLoginID == m_pDecoder->GetHandle() &&(NULL != szIp))
	{
	
	}
	else
	{		
		CEncoderMap::const_iterator it = m_mapEncoder.find(lLoginID);
		if (it != m_mapEncoder.end())
		{
			CEncoder* pDev = it->second;
			pDev->Online(FALSE);

			HTREEITEM hItem = FindTreeNode(m_treeDevice, m_treeDevice.GetRootItem(), TreeItemData(TREE_ITEM_DEVICE, pDev));
			if (hItem)
			{
				m_treeDevice.SetItemText(hItem, pDev->GetIP() + (pDev->IsOnline() ? LMIC("(Online])") : LMIC("(Offline)")));
			}			
		}
	}
	return 0;
}


LRESULT CDemoMonitorWallDlg::DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
	{
	case WM_CLOSE_VIDEOWINDOW:
		{
			int nWindow = wParam;			
			int nTVSel = m_cbTVOut.GetCurSel();
			if (nTVSel != CB_ERR)
			{
				int nTVID = m_cbTVOut.GetItemData(nTVSel);	
				
				if (MsgBoxFmt(m_hWnd, MB_YESNO | MB_ICONQUESTION, LMIC("Prompt"), LMIC("Close window %d ?"), nWindow) == IDYES)
				{
					DH_IN_SPLIT_CLOSE_WINDOW stuInParam = { sizeof(DH_IN_SPLIT_CLOSE_WINDOW) };
					stuInParam.nChannel = nTVID;
					stuInParam.nWindowID = nWindow;

					DH_OUT_SPLIT_CLOSE_WINDOW stuOutParam = { sizeof(DH_OUT_SPLIT_CLOSE_WINDOW) };
					
					if(!CLIENT_CloseSplitWindow(m_pDecoder->GetHandle(), &stuInParam, &stuOutParam, WAIT_TIME))
					{
						MsgBoxErr("Close window error.");
					}
					else
					{
						m_wndScreen.RemoveWindow(nWindow);
					}
				}
			}
		}
		break;
	case WM_CLEAR_VIDEOWINDOW:
		{
			int nWindow = wParam;
			if (m_wndScreen.GetWindowText(nWindow).IsEmpty())
			{
				break;
			}
			
			int nTVSel = m_cbTVOut.GetCurSel();
			if (nTVSel != CB_ERR)
			{
				int nTVID = m_cbTVOut.GetItemData(nTVSel);
				
				DH_SPLIT_SOURCE stuSrc = { sizeof(DH_SPLIT_SOURCE) };
				if (CLIENT_SetSplitSource(m_pDecoder->GetHandle(), nTVID, nWindow, &stuSrc, 1, WAIT_TIME))
				{
					m_wndScreen.SetWindowText(nWindow, NULL);
				}
			}
		}
		break;
	case WM_SET_VIDEOWINDOW_RECT:
		{
			int nWindow = wParam;
			int nTVSel = m_cbTVOut.GetCurSel();
			if (nTVSel != CB_ERR)
			{
				int nTVID = m_cbTVOut.GetItemData(nTVSel);
				
				DH_IN_SPLIT_SET_RECT stuInParam = { sizeof(DH_IN_SPLIT_SET_RECT) };
				stuInParam.nChannel = nTVID;
				stuInParam.nWindowID = nWindow;
				stuInParam.stuRect = RectUniversalize(*(CRect*)lParam);
				if (!CLIENT_SetSplitWindowRect(m_pDecoder->GetHandle(), &stuInParam, NULL, WAIT_TIME))
				{
					MsgBoxErr("Set rect error.");
				}
			}
		}
		break;
	default:
		return CDialog::DefWindowProc(message, wParam, lParam);
	}
	return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarLogin(WPARAM wParam, LPARAM lParam)
{
    CLoginDlg dlg;
    if (dlg.DoModal() == IDOK)
    {
        ClearDecoder();
        m_pDecoder = (CDecoder*)CDeviceFactory::Create(dlg.m_lLoginID, dlg.m_strAddr, dlg.m_nPort, dlg.m_strUser, dlg.m_strPwd, dlg.DCID());
        //	m_pDecoder = (CDecoder*)CDeviceFactory::Create(dlg.m_lLoginID, dlg.m_strAddr, dlg.m_nPort, dlg.m_strUser, dlg.m_strPwd, CDecoderDSCON::DCID()/*dlg.DCID()*/);
        
        if (!m_pDecoder->Update())
        {
            MsgBoxErr("Query device info error");
            ClearDecoder();
            return FALSE;
        }
        
        int i = 0;
        int nTVID = 0;
        CString strCaption;
        strCaption.Format("%s - %s [%s]", LMIC(APP_NAME), dlg.m_strAddr, m_pDecoder->GetDeviceClass());
        SetWindowText(strCaption);
        
        m_wndToolBar.EnableButton(ID_TOOLBAR_LOGIN, FALSE);			
        m_wndToolBar.EnableButton(ID_TOOLBAR_LOGOUT, TRUE);
        
        if (m_pDecoder->GetDecChnMap().size() == 0)
        {
            MsgBox0("This device has no video out channels.");
            return FALSE;
        }
        
        m_cbTVOut.ResetContent();
        CDecChnMap::const_iterator it = m_pDecoder->GetDecChnMap().begin();
        for (; it != m_pDecoder->GetDecChnMap().end(); ++it)
        {
            // 已经是融合屏的组成部分, 不显示
            if (it->second.m_bPartOfSpliceScreen)	continue;
            
            // 大屏控制器不显示单个物理屏
            if (_stricmp(m_pDecoder->GetDeviceClass(), CDecoderDSCON::DCID()) == 0 && !it->second.m_bSpliceScreen) 
                continue;
            
            int nIndex = m_cbTVOut.AddString(it->second.ToString());
            m_cbTVOut.SetItemData(nIndex, it->first);
        }
        
        m_cbTVOut.SetCurSel(0);
        nTVID = m_cbTVOut.GetItemData(0);
        if (nTVID >= 0)
        {					
            // 获取当前TV的分割模式
            DH_SPLIT_MODE_INFO stuInfo = { sizeof(DH_SPLIT_MODE_INFO) };
            if (!CLIENT_GetSplitMode(m_pDecoder->GetHandle(), nTVID, &stuInfo, WAIT_TIME))
            {
                MsgBoxErr("Get split mode error.");
                //	return FALSE;
            }
            if (stuInfo.emSplitMode == DH_SPLIT_FREE)
            {
                m_wndScreen.SetWindowControlType(CVideoScreen::SLIP_WINDOW);
            }
            else
            {
                m_wndScreen.SetWindowControlType(CVideoScreen::FIXED_WINDOW);
                m_wndScreen.SetSplitMode(stuInfo.emSplitMode);
            }
            EnableFreeStyleButton(stuInfo.emSplitMode == DH_SPLIT_FREE);
            
            // 获取当前分组中所有窗口的显示源
            UpdateSplitSource();		
            
            // 分割按钮根据能力使能
            EnableSplitButton(-1, FALSE);
            const CDecChannel* pTVOut = m_pDecoder->GetDecodeChannel(nTVID);
            if (pTVOut)
            {	
                const UINT* pnMode = pTVOut->m_nSplitModes;
                while (*pnMode)
                {
                    EnableSplitButton(*pnMode, TRUE, SBF_BYVALUE);
                    ++pnMode;
                }
            }
        }
        
        AddDefaultEncoder();

		// 添加设备按钮使能
		BOOL bEnableAddDevice = TRUE;
		LPCTSTR lpszDevClass = m_pDecoder->GetDeviceClass();
		if (_stricmp(lpszDevClass, CDecoderNVR::DCID()) == 0 || _stricmp(lpszDevClass, CDecoderDSCON::DCID()) == 0)
		{
			bEnableAddDevice = FALSE;
		}
		GetDlgItem(IDC_BTN_ADD_DEVICE)->EnableWindow(bEnableAddDevice);
		GetDlgItem(IDC_BTN_REMOVE_DEVICE)->EnableWindow(bEnableAddDevice);
	}

    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarLogout(WPARAM wParam, LPARAM lParam)
{
    if (m_pDecoder->GetHandle())
    {
        ClearDecoder();		
        ClearEncoder();
        ClearEncoderTree();
        SetWindowText(LMIC(APP_NAME));
        m_wndToolBar.EnableButton(ID_TOOLBAR_LOGIN, TRUE);
        m_wndToolBar.EnableButton(ID_TOOLBAR_LOGOUT, FALSE);
        //m_wndScreen.ClearWindowText();
        m_wndScreen.SetWindowControlType(CVideoScreen::SLIP_WINDOW);
        m_cbTVOut.ResetContent();
	}

    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarSplice(WPARAM wParam, LPARAM lParam)
{
    if (m_pDecoder == NULL) return FALSE;
    CSpliceScreenConfigDlg dlg(m_pDecoder->GetHandle());
    if (dlg.DoModal() == IDOK)
    {
        MsgBox0("Please logout and login again after monitor wall setting.");
        SendMessage(WM_COMMAND, ID_TOOLBAR_LOGOUT);
        SendMessage(WM_COMMAND, ID_TOOLBAR_LOGIN);
	}
    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarSplitN(WPARAM wParam, LPARAM lParam)
{
    int nTVSel = m_cbTVOut.GetCurSel();
    if (nTVSel == CB_ERR)
    {
        return FALSE;
    }
    int nTVID = m_cbTVOut.GetItemData(nTVSel);
    const CDecChannel* pTVOut = m_pDecoder->GetDecodeChannel(nTVID);
    if (pTVOut == NULL)
    {
        return FALSE;
    }
    
    int i = 0;
    int nSplit = 0;
    for (i = 0; i < _countof(g_ToolBarBtnSplit); ++i)
    {
        if (g_ToolBarBtnSplit[i].nID == wParam)
        {
            nSplit = g_ToolBarBtnSplit[i].nSplit;
            break;
        }
    }
    
    if (nSplit > 0)
    {
        int nGroupCount = ceil((float)pTVOut->m_nMaxSources / nSplit);
        
        // 8分割比较特殊, 分组数就是总的显示源数
        // 默认情况下, 主窗口对应第一视频源, 当把第二视频源作为主窗口时是组2, 以此类推
        if (nSplit == 8) 
        {
            nGroupCount = pTVOut->m_nMaxSources;
        }
        
        CString strTemp;
        CMenu contextMenu;
        
        contextMenu.CreatePopupMenu();
        for (i = 0; i < nGroupCount; ++i)
        {
            strTemp.Format(LMIC("Group %d"), i);
            contextMenu.AppendMenu(MF_STRING, ID_MENU_GROUP_ID + i, strTemp);
        }
        
        POINT pt;
        GetCursorPos(&pt);
        int nCmd = contextMenu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, this);
        
        int nGroup = nCmd - ID_MENU_GROUP_ID;
        if (nGroup >= 0 && nGroup < nGroupCount)
        {
            // 切换分割
            DH_SPLIT_MODE_INFO stuMode = { sizeof(DH_SPLIT_MODE_INFO) };
            stuMode.emSplitMode = (DH_SPLIT_MODE)nSplit;
            stuMode.nGroupID = nGroup;
            if (!CLIENT_SetSplitMode(m_pDecoder->GetHandle(), nTVID, &stuMode, WAIT_TIME))
            {
                MsgBoxErr("Set split mode error.");
                return FALSE;
            }
            m_wndScreen.SetWindowControlType(CVideoScreen::FIXED_WINDOW);
            m_wndScreen.SetSplitMode(nSplit);
            EnableFreeStyleButton(FALSE);
            
            // 显示个窗口信息
            UpdateSplitSource();
        }
	}

    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarSplitFree(WPARAM wParam, LPARAM lParam)
{
    int nTVSel = m_cbTVOut.GetCurSel();
    if (nTVSel == CB_ERR)
    {
        return FALSE;
    }
    int nTVID = m_cbTVOut.GetItemData(nTVSel);
    
    // 切换分割
    DH_SPLIT_MODE_INFO stuMode = { sizeof(DH_SPLIT_MODE_INFO) };
    stuMode.emSplitMode = DH_SPLIT_FREE;
    if (!CLIENT_SetSplitMode(m_pDecoder->GetHandle(), nTVID, &stuMode, WAIT_TIME))
    {
        MsgBoxErr("Set split mode error.");
        return FALSE;
    }
    m_wndScreen.SetWindowControlType(CVideoScreen::SLIP_WINDOW);
    EnableFreeStyleButton(TRUE);
    
    // 显示窗口信息
	UpdateSplitSource();

    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarOpenWindow(WPARAM wParam, LPARAM lParam)
{
    if (m_wndScreen.GetWindowControlType() == CVideoScreen::FIXED_WINDOW)
    {
        return FALSE;
    }
    // tv id
    int nTVSel = m_cbTVOut.GetCurSel(); 
    if (nTVSel == CB_ERR)
    {
        return FALSE;
    }
    int nTVID = m_cbTVOut.GetItemData(nTVSel);
    
    CRect rcWnd(0, 0, 200, 200);
    
    DH_IN_SPLIT_OPEN_WINDOW stuInParam = { sizeof(DH_IN_SPLIT_OPEN_WINDOW) };
    DH_OUT_SPLIT_OPEN_WINDOW stuOutParam = { sizeof(DH_OUT_SPLIT_OPEN_WINDOW) };
    stuInParam.nChannel = nTVID;
    stuInParam.stuRect = RectUniversalize(rcWnd);
    if (!CLIENT_OpenSplitWindow(m_pDecoder->GetHandle(), &stuInParam, &stuOutParam, WAIT_TIME))
    {
        MsgBoxErr("Open window error.");
    }
    else
    {
        m_wndScreen.AddWindow(rcWnd, stuOutParam.nWindowID);
	}

    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarCloseWindow(WPARAM wParam, LPARAM lParam)
{
    if (m_wndScreen.GetWindowControlType() == CVideoScreen::FIXED_WINDOW)
    {
        return FALSE;
    }
    // tv id
    int nTVSel = m_cbTVOut.GetCurSel();
    if (nTVSel == CB_ERR)
    {
        return FALSE;
    }
    int nTVID = m_cbTVOut.GetItemData(nTVSel);
    
    // window id
    int nWindow = m_wndScreen.GetActiveWindowID();
    
    if (MsgBoxFmt(m_hWnd, MB_YESNO | MB_ICONQUESTION, LMIC("Prompt"), LMIC("Close window %d ?"), nWindow) == IDYES)
    {
        DH_IN_SPLIT_CLOSE_WINDOW stuInParam = { sizeof(DH_IN_SPLIT_CLOSE_WINDOW) };
        stuInParam.nChannel = nTVID;
        stuInParam.nWindowID = nWindow;
        
		DH_OUT_SPLIT_CLOSE_WINDOW stuOutParam = { sizeof(DH_OUT_SPLIT_CLOSE_WINDOW) };

        if(!CLIENT_CloseSplitWindow(m_pDecoder->GetHandle(), &stuInParam, &stuOutParam, WAIT_TIME))
        {
            MsgBoxErr("Close window error.");
        }
        else
        {
            m_wndScreen.RemoveWindow(nWindow);
        }
	}
    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarSetLevel(WPARAM wParam, LPARAM lParam)
{
    if (m_wndScreen.GetWindowControlType() == CVideoScreen::FIXED_WINDOW)
    {
        return FALSE;    
    }
    
    // tv id
    int nTVSel = m_cbTVOut.GetCurSel();
    if (nTVSel == CB_ERR)
    {
        return FALSE;
    }
    int nTVID = m_cbTVOut.GetItemData(nTVSel);
    
    // window id
    int nWindow = m_wndScreen.GetActiveWindowID();
    
    DH_IN_SPLIT_SET_TOP_WINDOW stuInParam = { sizeof(DH_IN_SPLIT_SET_TOP_WINDOW), nTVID, nWindow };
    DH_OUT_SPLIT_SET_TOP_WINDOW stuOutParam = { sizeof(DH_OUT_SPLIT_SET_TOP_WINDOW) };
    stuOutParam.nMaxWndCount = 64;
    stuOutParam.pZOders = new DH_WND_ZORDER[stuOutParam.nMaxWndCount];
    memset(stuOutParam.pZOders, 0, sizeof(DH_WND_ZORDER) * stuOutParam.nMaxWndCount);
    for (int i = 0; i < stuOutParam.nMaxWndCount; ++i)
    {
        stuOutParam.pZOders[i].dwSize = sizeof(DH_WND_ZORDER);
    }
    if (!CLIENT_SetSplitTopWindow(m_pDecoder->GetHandle(), &stuInParam, &stuOutParam, WAIT_TIME))
    {
        MsgBoxErr("Set top window error.");
    }
    else
    {
        CString strMsg, strTemp;
        for (int i = 0; i < stuOutParam.nWndCount; ++i)
        {
            strTemp.Format("%d\t%d\n", stuOutParam.pZOders[i].nWindowID, stuOutParam.pZOders[i].nZOrder);
            strMsg += strTemp;
        }
        MsgBox(strMsg);
	}
    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarPower(WPARAM wParam, LPARAM lParam)
{
    if (m_pDecoder && m_pDecoder->GetHandle())
    {
        CPowerDlg dlg(m_pDecoder->GetHandle());
        dlg.DoModal();
	}

    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarCollection(WPARAM wParam, LPARAM lParam)
{
    if (m_pDecoder && m_pDecoder->GetHandle())
    {
        CCollectionDlg dlg(m_pDecoder->GetHandle());
        dlg.DoModal();
	}
    return TRUE;
}

BOOL CDemoMonitorWallDlg::ToolBarSetPush(WPARAM wParam, LPARAM lParam)
{
    int nTVSel = m_cbTVOut.GetCurSel();
    if (nTVSel == CB_ERR)
    {
        return FALSE;
    }
    int nTVID = m_cbTVOut.GetItemData(nTVSel);
    // window id
    int nWindow = m_wndScreen.GetActiveWindowID();
    
    // 切换成推流方式
    NET_IN_SET_SPLIT_SOURCE stuInParam = { sizeof(NET_IN_SET_SPLIT_SOURCE) };
    stuInParam.nChannel = nTVID;
    stuInParam.nSourceCount = 1;
    stuInParam.nWindow = nWindow;
    stuInParam.pstuSources = new DH_SPLIT_SOURCE;
    memset(stuInParam.pstuSources, 0, sizeof(DH_SPLIT_SOURCE));
    stuInParam.pstuSources->dwSize = sizeof(DH_SPLIT_SOURCE);
    stuInParam.pstuSources->bEnable = TRUE;
    stuInParam.pstuSources->byWorkMode = 1;
    stuInParam.pstuSources->byConnType = 0;
    
    NET_OUT_SET_SPLIT_SOURCE stuOutParam = { sizeof(NET_OUT_SET_SPLIT_SOURCE) };
    stuOutParam.nMaxResultCount = 1;
    stuOutParam.pstuResults = new NET_SET_SPLIT_SOURCE_RESULT;
    memset(stuOutParam.pstuResults, 0, sizeof(NET_SET_SPLIT_SOURCE_RESULT));
    stuOutParam.pstuResults->dwSize = sizeof(NET_SET_SPLIT_SOURCE_RESULT);
    
    if (CLIENT_SetSplitSourceEx(m_pDecoder->GetHandle(), &stuInParam, &stuOutParam))
    {
        stuInParam.pstuSources->wListenPort = stuOutParam.pstuResults->nPushPort;
        m_wndScreen.SetWindowText(nWindow, GetSplitSourceCaption(stuInParam.pstuSources));
    }
    else
    {
        MsgBoxErr("Switch to push mode failed.");
    }
    
    delete stuInParam.pstuSources;
	delete stuOutParam.pstuResults;
    return TRUE;
}

BOOL CDemoMonitorWallDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam)
	{
	case ID_TOOLBAR_LOGIN:
		{
		    return ToolBarLogin(wParam, lParam);
		}
		break;
	case ID_TOOLBAR_LOGOUT:
		{
		    return ToolBarLogout(wParam, lParam);
		}
		break;
	case ID_TOOLBAR_SPLICE:
		{
			return ToolBarSplice(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_SPLIT1:
	case ID_TOOLBAR_SPLIT4:	
	case ID_TOOLBAR_SPLIT8:
	case ID_TOOLBAR_SPLIT9:
	case ID_TOOLBAR_SPLIT16:
    case ID_TOOLBAR_SPLIT25:
    case ID_TOOLBAR_SPLIT36:
	case ID_TOOLBAR_SPLIT64:
		{
			return ToolBarSplitN(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_SPLIT_FREE:
		{
            return ToolBarSplitFree(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_OPEN_WINDOW:
		{
            return ToolBarOpenWindow(wParam ,lParam);
		}
		break;
	case ID_TOOLBAR_CLOSE_WINDOW:
		{		
            return ToolBarCloseWindow(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_SET_LEVEL:
		{			
            return ToolBarSetLevel(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_POWER:
		{
            return ToolBarPower(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_COLLECTION:
		{
            return ToolBarCollection(wParam , lParam);
		}
		break;
	case ID_TOOLBAR_SET_PUSH:
		{
            return ToolBarSetPush(wParam , lParam);
        }
	default:
		return CDialog::OnCommand(wParam, lParam);
	}
	return TRUE;
}

void CDemoMonitorWallDlg::OnBtnAddDevice() 
{
	CLoginDlg dlg(CLoginDlg::LOGIN_ENCODER);
	if (dlg.DoModal() == IDOK)
	{
		CEncoder* pDev = new CEncoder(dlg.m_lLoginID, dlg.m_strAddr, dlg.m_nPort, dlg.m_strUser, dlg.m_strPwd);
		m_mapEncoder[pDev->GetHandle()] = pDev;
		pDev->Update();
		pDev->Online(TRUE);
		AddDevice2Tree(pDev);

		pDev->m_strDevClass = dlg.m_strDevClass;
		pDev->m_strDevModel = dlg.m_strDevType;

		int nRetLen = 0;
		if (pDev->m_strDevClass.IsEmpty())
		{
			BOOL bRet = CLIENT_QueryDevState(pDev->GetHandle(), DH_DEVSTATE_TYPE, pDev->m_strDevClass.GetBuffer(256), 256, &nRetLen);
			if (!bRet)			
			{
				DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
			}
			pDev->m_strDevClass.ReleaseBuffer();
			pDev->m_strDevModel = pDev->m_strDevClass;
		}
	}	
}

void CDemoMonitorWallDlg::OnBtnRemoveDevice() 
{
	HTREEITEM hItem = m_treeDevice.GetSelectedItem();
	if (hItem)
	{
		TreeItemData* pData = (TreeItemData*)m_treeDevice.GetItemData(hItem);
		if (pData)
		{
			if (pData->uItemType == TREE_ITEM_DEVICE)
			{
				ClearTreeNode(m_treeDevice, hItem);
			}
			else if (pData->uItemType == TREE_ITEM_CHANNEL)
			{
				ClearTreeNode(m_treeDevice, m_treeDevice.GetParentItem(hItem));
			}
		}
	}
}

void CDemoMonitorWallDlg::ClearEncoder()
{	
	for (CEncoderMap::iterator it = m_mapEncoder.begin(); it != m_mapEncoder.end(); ++it)
	{
		CEncoder* pDev = it->second;
		if (pDev)
		{
			CLIENT_Logout(pDev->GetHandle());
			delete pDev;
			pDev = NULL;
		}
	}
	m_mapEncoder.clear();
}

void CDemoMonitorWallDlg::ClearDecoder()
{
	if (m_pDecoder)
	{
		CLIENT_Logout(m_pDecoder->GetHandle());
		delete m_pDecoder;
		m_pDecoder = NULL;
	}
}

void CDemoMonitorWallDlg::AddDevice2Tree( CEncoder* pDev )
{
	CString strText;
	HTREEITEM hDevice = m_treeDevice.InsertItem(pDev->GetIP() + (pDev->IsOnline() ? LMIC("(Online)") : LMIC("(Offline)")));	
	m_treeDevice.SetItemData(hDevice, (DWORD)new TreeItemData(TREE_ITEM_DEVICE, pDev));

	CEncChnMap::const_iterator it = pDev->GetEncChnMap().begin();
	for (; it != pDev->GetEncChnMap().end(); ++it)
	{
		HTREEITEM hChannel = m_treeDevice.InsertItem(it->second.ToString(), hDevice);
		m_treeDevice.SetItemData(hChannel, (DWORD)new TreeItemData(TREE_ITEM_CHANNEL, pDev, it->second.m_nChannelID));		
	}
	m_treeDevice.Expand(hDevice, TVE_EXPAND);
}

void CDemoMonitorWallDlg::ClearEncoderTree()
{
	HTREEITEM hRoot = m_treeDevice.GetRootItem();
	while (hRoot)
	{
		HTREEITEM hRootNext = m_treeDevice.GetNextItem(hRoot, TVGN_NEXT);
		ClearTreeNode(m_treeDevice, hRoot);
		hRoot = hRootNext;
	}
}

void CDemoMonitorWallDlg::ClearTreeNode( CTreeCtrl& treeCtrl, HTREEITEM hItem )
{
	if (hItem)
	{
		HTREEITEM hChild = treeCtrl.GetNextItem(hItem, TVGN_CHILD);
		while (hChild)
		{
			HTREEITEM hChildNext = treeCtrl.GetNextItem(hChild, TVGN_NEXT);
			ClearTreeNode(treeCtrl, hChild);
			hChild = hChildNext;
		}

		TreeItemData* pData = (TreeItemData*)treeCtrl.GetItemData(hItem);
		if (pData)
		{
			delete pData;
			pData = NULL;
		}
		treeCtrl.DeleteItem(hItem);
	}
}

HTREEITEM CDemoMonitorWallDlg::FindTreeNode( const CTreeCtrl& treeCtrl, HTREEITEM hItem, const TreeItemData& data )
{
	HTREEITEM hTreeNode = NULL;
	if (hItem)
	{
		TreeItemData* pData = (TreeItemData*)treeCtrl.GetItemData(hItem);
		if (pData)
		{
			if (*pData == data)
			{
				return hItem;
			}
		}

		HTREEITEM hChild = treeCtrl.GetNextItem(hItem, TVGN_CHILD);
		while (hChild)
		{
			hTreeNode = FindTreeNode(treeCtrl, hChild, data);
			if (hTreeNode)
			{
				return hTreeNode;
			}
			hChild = treeCtrl.GetNextItem(hChild, TVGN_NEXT);
		}

		HTREEITEM hBrother = treeCtrl.GetNextItem(hItem, TVGN_NEXT);
		if (hBrother)
		{
			hTreeNode = FindTreeNode(treeCtrl, hBrother, data);
			if (hTreeNode)
			{
				return hTreeNode;
			}
			hBrother = treeCtrl.GetNextItem(hBrother, TVGN_NEXT);
		}
	}
	return NULL;
}

void CDemoMonitorWallDlg::OnDblclkTreeDevice(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HTREEITEM hItem = m_treeDevice.GetSelectedItem();
    if (hItem == NULL)
    {
        return;
    }
	TreeItemData* pData = (TreeItemData*)m_treeDevice.GetItemData(hItem);

	if (pData && pData->pDevice && pData->uItemType == TREE_ITEM_CHANNEL && pData->nChannel >= 0)
	{		
		int nTVSel = m_cbTVOut.GetCurSel();
		if (nTVSel == CB_ERR)
		{
			return;
		}
		int nTVID = m_cbTVOut.GetItemData(nTVSel);

		int nWindow = m_wndScreen.GetActiveWindowID();

		DH_SPLIT_SOURCE stuSrc = { sizeof(DH_SPLIT_SOURCE) };
		stuSrc.bEnable = TRUE;
#if 1	// 使用ip/端口设置视频源
		strncpy(stuSrc.szIp, pData->pDevice->GetIP(), sizeof(stuSrc.szIp) - 1);
		stuSrc.nPort = pData->pDevice->GetPort();
		strncpy(stuSrc.szUser, pData->pDevice->GetUser(), sizeof(stuSrc.szUser) - 1);
		strncpy(stuSrc.szPwd, pData->pDevice->GetPwd(), sizeof(stuSrc.szPwd) - 1);
		stuSrc.nChannelID = pData->nChannel;
		strncpy(stuSrc.szDevName, pData->pDevice->GetIP(), sizeof(stuSrc.szDevName) - 1);
		stuSrc.nVideoChannel = pData->pDevice->GetEncChnMap().size();
		strncpy(stuSrc.szDevClass, pData->pDevice->m_strDevClass, sizeof(stuSrc.szDevClass) - 1);
		strncpy(stuSrc.szDevType, pData->pDevice->m_strDevModel, sizeof(stuSrc.szDevType) - 1);
#else // 使用逻辑通道号设置视频源, 逻辑通道号通过CLIENT_MatrixGetCameras得到
		strncpy(stuSrc.szDeviceID, DH_DEVICE_ID_UNIQUE, sizeof(stuSrc.szDeviceID) - 1);
		stuSrc.nChannelID = pData->nChannel;
#endif

		if (!CLIENT_SetSplitSource(m_pDecoder->GetHandle(), nTVID, nWindow, &stuSrc, 1, WAIT_TIME))
		{
			MsgBoxErr("Set split source error.");
			return;
		}
		m_wndScreen.SetWindowText(nWindow, GetSplitSourceCaption(&stuSrc));
	}
	
	*pResult = 0;
}

CString CDemoMonitorWallDlg::GetSplitSourceCaption( const DH_SPLIT_SOURCE* pstuSrc )
{
	CString str;
	if (pstuSrc->bEnable || *pstuSrc->szDeviceID || *pstuSrc->szDevIpEx || *pstuSrc->szIp)
	{
		if (pstuSrc->byWorkMode == 1) // 推流方式
		{
			str.Format("%s\r\n%s\r\n%s\r\n%d", 
				pstuSrc->bEnable ? LMIC("Enable") : LMIC("Disable"), 
				pstuSrc->szDeviceID, 
				pstuSrc->byConnType == 1 ? "UDP-Push" : "TCP-Push", 
				pstuSrc->wListenPort);
		}
		else // 拉流方式
		{
			str.Format("%s\r\n%s\r\n%s\r\n%d", 
				pstuSrc->bEnable ? LMIC("Enable") : LMIC("Disable"), 
				pstuSrc->szDeviceID, 
				pstuSrc->szDevIpEx[0] == 0 ? pstuSrc->szIp : pstuSrc->szDevIpEx, 
			pstuSrc->nChannelID);
		}
	}
	return str;
}

CString CDemoMonitorWallDlg::GetSplitSourceCaption( const DH_WINDOW_COLLECTION* pstuSrc )
{
	CString str;
	if (pstuSrc->bSrcEnable || *pstuSrc->szDeviceID)
	{
		str.Format("%s\r\n%s\r\n%d", 
			pstuSrc->bSrcEnable ? LMIC("Enable") : LMIC("Disable"), 
			pstuSrc->szDeviceID, 
			pstuSrc->nVideoChannel);
	}
	return str;
}

void CDemoMonitorWallDlg::UpdateSplitSource()
{
	int nTVSel = m_cbTVOut.GetCurSel();
	if (nTVSel != CB_ERR)
	{
		int nTVID = m_cbTVOut.GetItemData(nTVSel);
		int i = 0;

		if (m_wndScreen.GetWindowControlType() == CVideoScreen::FIXED_WINDOW)
		{
			int nRealWindow = 0;
			int nMaxSplitWindow = 64;
			DH_SPLIT_SOURCE* pstuSrc = new DH_SPLIT_SOURCE[nMaxSplitWindow];
			memset(pstuSrc, 0, sizeof(DH_SPLIT_SOURCE) * nMaxSplitWindow);
			for (i = 0; i < nMaxSplitWindow; ++i)
			{
				pstuSrc[i].dwSize = sizeof(DH_SPLIT_SOURCE);
			}
			
			if (!CLIENT_GetSplitSource(m_pDecoder->GetHandle(), nTVID, -1, pstuSrc, nMaxSplitWindow, &nRealWindow, WAIT_TIME))
			{
				MsgBoxErr("Get split source error.");
			}
			m_wndScreen.ClearWindowText();
			for (i = 0; i < nRealWindow; ++i)
			{
				m_wndScreen.SetWindowText(i, GetSplitSourceCaption(pstuSrc + i));
			}
			delete[] pstuSrc;
		}
		else
		{
			DH_IN_SPLIT_GET_WINDOWS stuInParam = { sizeof(DH_IN_SPLIT_GET_WINDOWS), nTVID };
			DH_OUT_SPLIT_GET_WINDOWS stuOutParam = { sizeof(DH_OUT_SPLIT_GET_WINDOWS) };
			stuOutParam.stuWindows.dwSize = sizeof(DH_BLOCK_COLLECTION);
			for (i = 0; i < DH_MAX_SPLIT_WINDOW; ++i)
			{
				stuOutParam.stuWindows.stuWnds[i].dwSize = sizeof(DH_WINDOW_COLLECTION);
			}

			if (!CLIENT_GetSplitWindowsInfo(m_pDecoder->GetHandle(), &stuInParam, &stuOutParam, WAIT_TIME))
			{
				MsgBoxErr("Get windows info error.");
			}

			m_wndScreen.RemoveWindow(-1);
			for (i = 0; i < stuOutParam.stuWindows.nWndsCount; ++i)
			{
				const DH_WINDOW_COLLECTION& wndInfo = stuOutParam.stuWindows.stuWnds[i];
				m_wndScreen.AddWindow(RectUniversalize(wndInfo.stuRect), stuOutParam.stuWindows.stuWnds[i].nWindowID, GetSplitSourceCaption(&wndInfo));
			}
			m_wndScreen.Invalidate();
		}
	}
}

void CDemoMonitorWallDlg::OnSelchangeComboTVOut() 
{
	int nTVSel = m_cbTVOut.GetCurSel();
	if (nTVSel != CB_ERR)
	{
		// 获取当前TV的分割模式
		int nTVID = m_cbTVOut.GetItemData(nTVSel);
		DH_SPLIT_MODE_INFO stuInfo = { sizeof(DH_SPLIT_MODE_INFO) };
		if (!CLIENT_GetSplitMode(m_pDecoder->GetHandle(), nTVID, &stuInfo, WAIT_TIME))
		{
			MsgBoxErr("Get split mode error.");
			return;
		}
		if (stuInfo.emSplitMode == DH_SPLIT_FREE)
		{
			m_wndScreen.SetWindowControlType(CVideoScreen::SLIP_WINDOW);
		}
		else
		{
			m_wndScreen.SetWindowControlType(CVideoScreen::FIXED_WINDOW);
			m_wndScreen.SetSplitMode(stuInfo.emSplitMode);
		}
		EnableFreeStyleButton(stuInfo.emSplitMode == DH_SPLIT_FREE);
		
		// 获取当前分组中所有窗口的显示源
		UpdateSplitSource();

		// 分割按钮根据能力使能
		EnableSplitButton(-1, FALSE);
		const CDecChannel* pTVOut = m_pDecoder->GetDecodeChannel(nTVID);
		if (pTVOut)
		{
			const UINT* pnMode = pTVOut->m_nSplitModes;
			while (*pnMode)
			{
				EnableSplitButton(*pnMode, TRUE, SBF_BYVALUE);
				++pnMode;
			}
		}
	}
}

void CDemoMonitorWallDlg::EnableSplitButton( UINT nBtnIDorValue, BOOL bEnable /*= TRUE*/, UINT uFlag /*= SBF_BYID*/ )
{
	if (nBtnIDorValue == (UINT)-1)
	{
		for (int i = 0; i < _countof(g_ToolBarBtnSplit); ++i)
		{
			m_wndSplit.EnableButton(g_ToolBarBtnSplit[i].nID, bEnable);
		}
	}
	else
	{
		if (uFlag == SBF_BYID)
		{
			m_wndSplit.EnableButton(nBtnIDorValue, bEnable);
		}
		else
		{
			for (int i = 0; i < _countof(g_ToolBarBtnSplit); ++i)
			{
				if (g_ToolBarBtnSplit[i].nSplit == nBtnIDorValue)
				{
					m_wndSplit.EnableButton(g_ToolBarBtnSplit[i].nID, bEnable);
					return;
				}
			}
		}
	}
}

DH_RECT CDemoMonitorWallDlg::RectUniversalize( const CRect& rcWnd )
{
	const float nMaxCoordinate = MAX_RECT_VALUE;
	CRect rcWall;
	DH_RECT rect;
	m_wndScreen.GetClientRect(rcWall);
	rect.left = (long)rcWnd.left * (nMaxCoordinate / rcWall.Width());
	rect.top = (long)rcWnd.top * (nMaxCoordinate / rcWall.Height());
	rect.right = (long)rcWnd.right * (nMaxCoordinate / rcWall.Width());
	rect.bottom = (long)rcWnd.bottom * (nMaxCoordinate / rcWall.Height());
//	rect.right = rect.bottom = 4100;
	return rect;
}

CRect CDemoMonitorWallDlg::RectUniversalize( const DH_RECT& rcWnd )
{
	const float nMaxCoordinate = MAX_RECT_VALUE;
	CRect rcWall;
	CRect rect;
	m_wndScreen.GetClientRect(rcWall);
	rect.left = (long)rcWnd.left / nMaxCoordinate * rcWall.Width();
	rect.top = (long)rcWnd.top / nMaxCoordinate * rcWall.Height();
	rect.right = (long)rcWnd.right / nMaxCoordinate * rcWall.Width();
	rect.bottom = (long)rcWnd.bottom / nMaxCoordinate * rcWall.Height();
	return rect;
}

void CDemoMonitorWallDlg::AddDefaultEncoder()
{
	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, (char*)m_pDecoder->GetIP(), sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, (char*)m_pDecoder->GetPwd(), sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, (char*)m_pDecoder->GetUser(), sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = m_pDecoder->GetPort();
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lLoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if (lLoginID)
	{
		CEncoder* pDev = (CEncoder*)CDeviceFactory::Create(lLoginID, m_pDecoder->GetIP(), m_pDecoder->GetPort(), m_pDecoder->GetUser(), m_pDecoder->GetPwd(), m_pDecoder->GetDeviceClass(), CDeviceFactory::Encoder);
		if (pDev)
		{
			if (pDev->Update() && pDev->GetEncChnMap().size() > 0)
			{
				pDev->Online(TRUE);
				m_mapEncoder[lLoginID] = pDev;
				
				AddDevice2Tree(pDev);
				return;
			}
			delete pDev;
		}
		CLIENT_Logout(lLoginID);
	}	
}

void CDemoMonitorWallDlg::EnableFreeStyleButton( BOOL bEnable /*= TRUE*/ )
{
	m_wndSplit.EnableButton(ID_TOOLBAR_OPEN_WINDOW, bEnable);
	m_wndSplit.EnableButton(ID_TOOLBAR_CLOSE_WINDOW, bEnable);
	m_wndSplit.EnableButton(ID_TOOLBAR_SET_LEVEL, bEnable);
}

BOOL CDemoMonitorWallDlg::PreTranslateMessage(MSG* pMsg) 
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