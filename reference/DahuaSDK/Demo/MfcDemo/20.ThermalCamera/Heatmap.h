#include "afxwin.h"
#if !defined(AFX_HEATMAP_H__F6DEBA5F_CAF7_4883_8104_C418B5C88460__INCLUDED_)
#define AFX_HEATMAP_H__F6DEBA5F_CAF7_4883_8104_C418B5C88460__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Heatmap.h : header file
//
#define WM_HEATMAPINFO              (WM_USER+100)
/////////////////////////////////////////////////////////////////////////////
// CHeatmap dialog

class CHeatmap : public CDialog
{
// Construction
public:
	CHeatmap(CWnd* pParent = NULL,LLONG lLoginId = 0,int nChannel = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHeatmap)
	enum { IDD = IDD_DLG_HEATMAP };
	CButton	m_BtnStop;
	CButton	m_BtnStart;
	CButton	m_BtnAttach;
	CString	m_strStatus;
	int		m_nChannel;
	int		m_nHeight;
	int		m_nLength;
	CString	m_StrSensorType;
	CString	m_StrTime;
	int		m_nWidth;
	NET_RADIOMETRY_DATA *m_pBuf;
	//}}AFX_DATA
    void ShowDlg(NET_RADIOMETRY_DATA* pBuf);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHeatmap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
    LLONG m_lLoginID;
    LLONG m_lAttachhandle;
    int m_nHeatChannel;
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHeatmap)
	virtual BOOL OnInitDialog();
	afx_msg void OnStartfetch();
	afx_msg void OnAttach();
	afx_msg void OnStop();
    afx_msg LRESULT OnShowInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnimgandtemsave();
	CButton m_BtnImgAndTemSave;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HEATMAP_H__F6DEBA5F_CAF7_4883_8104_C418B5C88460__INCLUDED_)
