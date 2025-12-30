// ActiveLoginDemoDlg.h : header file
//

#if !defined(AFX_ACTIVELOGINDEMODLG_H__1DC33715_C83D_4B0B_97D9_D4FC6971F5CE__INCLUDED_)
#define AFX_ACTIVELOGINDEMODLG_H__1DC33715_C83D_4B0B_97D9_D4FC6971F5CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PtzScreen.h"
#include "dhmutex.h"
#include "ReadWriteExcel.h"
#include "PluginLogging.h"
#include "TreeCtrlEx.h"
#include "PTZControlDlg.h"
#include <list>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "afxcmn.h"

using namespace std;
/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoDlg dialog
#define NEW new(std::nothrow)
#define RGB_OnLine  RGB(55,217,104)
#define RGB_OFFLine RGB(0,0,0)
#define RGB_CHECKED RGB(255,0,0)

#define WM_DEVICE_DISCONNECTED		(WM_USER + 200)
#define WM_LOGINSUCCESS				(WM_USER + 201)
#define WM_LOGIN_DISCONNECTED		(WM_USER + 202)

#define MAX_ROW    1001
#define SET_NULL   ""	

#define	HAND	0
#define AUTO	1
typedef struct tagDH_DEVICE_INFO 
{
    CString             szDevSerial;   
    CString             szDevName;
    CString             szIP;
    int                 nPort;
    CString             szUseName;
    CString             szPassWord;
    int                 nLoginType;				 // 0：IP登录1：主动注册登录
    BOOL                nLoginStatus;            // 设备状态，0:离线 1:排队登录 2:在线
    int                 nLoginTimes;             // 尝试登录次数
    LLONG               lLoginID;				 // 登录句柄
    int                 nDeviceRow;              // 设备所在Excel行号
    CString             strSecDirName;           // 二级目录名称
    int                 nChannelNum;             // 设备所包含的通道数
    HTREEITEM           srcRoot;                 // 设备所在目录的句柄
    HTREEITEM           DstRoot;                 // 设备自身节点的句柄
    BOOL                bSort;                   // 是否已排过序
	BOOL				bOnTree;				 // 设备是否在树上
}DH_DEVICE_INFO;

typedef struct tagDIR_NAME_NUMBER
{
    CString             strSecDirName;             // 二级目录名称
    int                 nSecDirNumber;             // 二级目录所在Excel行号
    HTREEITEM           root;                      // 节点句柄
}DIR_NAME_NUMBER;

// 为每个播放的窗口保存设备句柄和通道号，用于云台控制
typedef struct tagVideoInfo						 
{
    LLONG       lLoginID;
    int         nChannel;
}VideoInfo;

// 为每个播放的窗口保存信息，用于设备树重构后对应窗口和设备节点
typedef struct tagPlayHandle					  
{
    int                 nChannel;                  // 正在播放的通道号
    HTREEITEM           hParent;                   // 父节点句柄
    HTREEITEM           root;                      // 节点句柄
    BOOL                bPlay;                     // 节点是否已播放
	int					bFrameType;				   // 播放的码流类型
}PlayHandle;

// 设备掉线信息
typedef struct tagDisConnectionInfo
{
	CString				strIp;
	int					nPort;
	LLONG				lLoginID;
}DisConnectionInfo;

class CActiveLoginDemoDlg : public CDialog
{
// Construction
public:
	CActiveLoginDemoDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CActiveLoginDemoDlg();
public:
    friend void CALLBACK  MessageProcFunc(int nWndID, UINT message, DWORD dwUser);
    friend BOOL CALLBACK  GetParamsFunc(int nWndID, int type, DWORD dwUser);
    friend void CALLBACK SetParamsFunc(int nWndID, int type, DWORD dwUser);
	friend void CALLBACK RectEventFunc(RECT WinRect,CPoint &pointStart,CPoint &pointEnd,DWORD dwUser);
// Dialog Data
	//{{AFX_DATA(CActiveLoginDemoDlg)
	enum { IDD = IDD_ACTIVELOGINDEMO_DIALOG };
	CStatic	m_StaticInfo;
	CComboBox	m_Cmb_Split;
	CComboBox	m_Cmb_ShowType;
	CTabCtrl	m_CtlTab;
	CIPAddressCtrl	m_LocalAddr;
	CTreeCtrlEx 	m_DevTree;
	int		m_LocalPort;
	CString	m_strSearch;
	BOOL m_bInitExcel;
	BOOL m_bSorted;
	int m_nShowType;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActiveLoginDemoDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	//}}AFX_VIRTUAL
    afx_msg void OnTreeMenu(UINT nID);
    public:
    void    ReName(HTREEITEM root, CString strName);
// Implementation
public:
    CString m_strPath;                                  // Excel文件路径
	CString m_strPDFPath;								// PDF帮助文档路径
    CString m_strRootName;                              // 根目录名称
    CPTZControlDlg m_PtzControlDlg;
    HANDLE                      m_hEvent;
	unsigned int dwThreadID;
    BOOL    m_bExit;
    BOOL    m_bSort;
	BOOL	m_bDirSort;
	BOOL	m_bSearch;
	BOOL	m_bDeleteDev;
    LLONG   m_lListenHdl;
    LLONG   m_lLoginID;
    int     m_nChannel;
    LLONG   m_DispHanle[9];
    PlayHandle  m_stuPlayInfo[9];
    VideoInfo   m_stuVideoInfo[9];
	void LoginThread();
	void login(DH_DEVICE_INFO* pInfo);
    void InitExcel();
    void Sort();
	void DirSort();
	void DeleteDev();
    HWND GetDispHandle();
    void DealListenCB(LLONG lHandle, char *pIp, WORD wPort, LONG lCommand, void *pParam, DWORD dwParamLen);
	void DoEvent();
	void ClearRecourse();
	void Search();												// 搜索设备
	void DeleteEmptyDir(HTREEITEM root);						// 删除空的二级目录
	void DeleteDeviceDir(HTREEITEM root);						// 删除设备目录
protected:
	HICON m_hIcon;
private:
    void UpdataScreenPos(void);
    void MessageProc(int nWndID, UINT message);
    BOOL GetParams(int nWndID, int type);
    void SetParams(int nWndID, int type);
    BOOL GetExitDecode(int nCurWndID);
    BOOL GetExitCycle(int nCurWndID);
    void SetExitDecode(int nCurWndID);
	void SetExitCycle(int nCurWndID);
    void SetFastGOTO(int nposX, int nposY, int nZoom);
    void InitTabCtrl();
	void FindItem(HTREEITEM hRoot,int ShowType);              // 查找节点
    int				m_CurScreen;
    CPtzScreen		m_ptzScreen;
    CRect			m_screenRect;
	CRect			m_clientRect;
	BOOL			m_bWndExitCycle[16];
private:
    DHMutex                     m_cs;
    HANDLE	            m_hThread;				
    std::list<DH_DEVICE_INFO*>	m_lstDeviceInfos;	/* 设备信息列表 */
    DHMutex						m_csDeviceInfos;

    std::list<DIR_NAME_NUMBER*> m_lstSecDirInfos;   // 目录信息
    DHMutex                     m_csSecDirInfos; 

    void GetLocalIPAndPort(char *pcIPAddr, int *pnPort);
    void InitDeviceInfo(int iCol,DH_DEVICE_INFO* stuDevice_Info, CString str);
    HTREEITEM Initdirectory(CString strSecDir, HTREEITEM root);           // nDirLevel:目录等级,nSecDir:目录序号,strSecDir:目录名称,root:目录标示
private:
    DH_DEVICE_INFO* Find(HTREEITEM hRoot);          // 按节点查找设备信息
    std::vector<std::string> m_vecIDInfo;           // ID信息
    std::vector<std::string> m_vecIPInfo;           // IP信息
    std::vector<std::string> m_vecNameInfo;         // 设备名称信息

    std::map<CString,char> m_mapDirName;
    std::map<CString,char> m_mapDeviceID;
    void DeleteVector(std::vector<std::string>& vec,char* string);
	// Generated message map functions
	//{{AFX_MSG(CActiveLoginDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnStartserver();
	afx_msg void OnStopserver();
    afx_msg LRESULT OnDeviceDisconnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoginSuccess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnLoginDisconnected(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClickDevtree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkDevtree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickDevtree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboShowtype();
	afx_msg void OnBtnSearch();
	afx_msg void OnBtnStopall();
	afx_msg void OnSelchangeCmbSplit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedHelp();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTIVELOGINDEMODLG_H__1DC33715_C83D_4B0B_97D9_D4FC6971F5CE__INCLUDED_)
