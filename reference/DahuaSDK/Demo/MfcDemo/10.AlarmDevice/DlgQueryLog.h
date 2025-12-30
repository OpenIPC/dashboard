#if !defined(AFX_DLGQUERYLOG_H__66A018A1_78F5_4E34_A3E9_DE4FD441562A__INCLUDED_)
#define AFX_DLGQUERYLOG_H__66A018A1_78F5_4E34_A3E9_DE4FD441562A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgQueryLog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgQueryLog dialog

class CDlgQueryLog : public CDialog
{
// Construction
public:
	CDlgQueryLog(CWnd* pParent = NULL, LLONG lLoginId = 0, NET_DEVICE_TYPE emDevType = NET_PRODUCT_NONE, int nMaxPageSize = 10);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgQueryLog)
	enum { IDD = IDD_DLG_QUERY_LOG };
	CListCtrl	m_logList;
	CComboBox	m_cmbQueryType;
	int		m_nPageLines; // [1, 1024]
	CTime	m_timeEnd;
	CTime	m_timeBegin;
	COleDateTime	m_dataEnd;
	COleDateTime	m_dataBegin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgQueryLog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgQueryLog)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnQuery();
	afx_msg void OnBtnPrePage();
	afx_msg void OnBtnNextPage();
	afx_msg void OnBtnClrLog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void			InitDlg();
	int				QueryLog(int nStartNo, int nPageSize);
	void			InsertListViewItem(int nStartNo, const CString& strLogType, const CString& strLogTime, const CString& strLogContext);

private:
	LLONG			m_lLoginId;
	int				m_nStartNo;
	int				m_nMaxPageSize;
	NET_DEVICE_TYPE	m_emDevType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGQUERYLOG_H__66A018A1_78F5_4E34_A3E9_DE4FD441562A__INCLUDED_)
