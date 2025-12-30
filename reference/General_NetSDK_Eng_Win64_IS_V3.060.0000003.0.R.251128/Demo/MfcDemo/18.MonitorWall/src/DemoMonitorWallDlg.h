// DemoMonitorWallDlg.h : header file
//

#if !defined(AFX_DEMOMONITORWALLDLG_H__60CAB033_5393_4279_ABAC_B7BC928646B7__INCLUDED_)
#define AFX_DEMOMONITORWALLDLG_H__60CAB033_5393_4279_ABAC_B7BC928646B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VideoScreen.h"
#include "Device.h"
#include "ToolBarEx.h"

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallDlg dialog

class CDemoMonitorWallDlg : public CDialog
{
// Construction
public:
	CDemoMonitorWallDlg(CWnd* pParent = NULL);	// standard constructor
	enum { SBF_BYID, SBF_BYVALUE };

// Dialog Data
	//{{AFX_DATA(CDemoMonitorWallDlg)
	enum { IDD = IDD_DEMOMONITORWALL_DIALOG };
	CTreeCtrl	m_treeDevice;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoMonitorWallDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	const CDecoder* GetDecoderInfo() const { return m_pDecoder; }

protected:

	void InitUI();

	void ClearDecoder();
	// 清除设备map
	void ClearEncoder();
	// 增加设备到树
	void AddDevice2Tree(CEncoder* pDev);
	// 在treeCtrl的hItem结点下查找满足条件(==data)的结点, 没有就返回NULL
	HTREEITEM FindTreeNode(const CTreeCtrl& treeCtrl, HTREEITEM hItem, const TreeItemData& data);
	// 清除设备树
	void ClearEncoderTree();
	// 清除treeCtrl中的某个结点及其子结点
	void ClearTreeNode(CTreeCtrl& treeCtrl, HTREEITEM hItem);
	// 添加默认编码设备
	void AddDefaultEncoder();

	// 刷新窗口的显示源信息
	void UpdateSplitSource();
	// 工具栏分割按钮使能, nBtnID为-1表示所有按钮
	void EnableSplitButton(UINT nBtnIDorValue, BOOL bEnable = TRUE, UINT uFlag = SBF_BYID);
	void EnableFreeStyleButton(BOOL bEnable = TRUE);

	// 得到显示源对应的显示信息
	CString GetSplitSourceCaption(const DH_SPLIT_SOURCE* pstuSrc);
	CString GetSplitSourceCaption(const DH_WINDOW_COLLECTION* pstuSrc);

	DH_RECT RectUniversalize(const CRect& rcWnd);
	CRect RectUniversalize(const DH_RECT& rcWnd);

	// 断线回调
	static void CALLBACK DisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);	
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	static void CALLBACK HaveReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);	

    BOOL ToolBarLogin(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarLogout(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarSplice(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarSplitN(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarOpenWindow(WPARAM wParam, LPARAM lParam); 
    BOOL ToolBarCloseWindow(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarSplitFree(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarSetLevel(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarPower(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarCollection(WPARAM wParam, LPARAM lParam);
    BOOL ToolBarSetPush(WPARAM wParam, LPARAM lParam);

protected:
	HICON		m_hIcon;
	CToolBarEx	m_wndToolBar;
	CToolBarEx	m_wndSplit;
	CVideoScreen m_wndScreen;
	CComboBox	m_cbTVOut;

	// 矩阵信息
	CDecoder*	m_pDecoder;
	// 设备信息
	typedef std::map<LLONG, CEncoder*> CEncoderMap;
	CEncoderMap	m_mapEncoder;

	// Generated message map functions
	//{{AFX_MSG(CDemoMonitorWallDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnAddDevice();
	afx_msg void OnBtnRemoveDevice();
	afx_msg void OnDblclkTreeDevice(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeComboTVOut();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOMONITORWALLDLG_H__60CAB033_5393_4279_ABAC_B7BC928646B7__INCLUDED_)
