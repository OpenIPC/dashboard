#if !defined(AFX_QUERYITEMINFODLG_H__247C3978_53A6_4665_9917_DD325F47EE23__INCLUDED_)
#define AFX_QUERYITEMINFODLG_H__247C3978_53A6_4665_9917_DD325F47EE23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueryItemInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQueryItemInfoDlg dialog

class CQueryItemInfoDlg : public CDialog
{
// Construction
    
public:
	CQueryItemInfoDlg(CWnd* pParent = NULL,LLONG iLoginId = 0,int nChannel = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQueryItemInfoDlg)
	enum { IDD = IDD_DLG_ITEMINFO };
	CComboBox	m_ComboMeterType;
	int		m_RuleID;
	int		m_PresetID;
	CString	m_Measuretype;
	float	m_MaxTemp;
	CString	m_Unit;
	float	m_AveTemp;
	float	m_MidTemp;
	float	m_MinTemp;
	float	m_StdTemp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueryItemInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_iLoginID;
    int m_nChannel;
	// Generated message map functions
	//{{AFX_MSG(CQueryItemInfoDlg)
	afx_msg void OnQueryiteminfo();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUERYITEMINFODLG_H__247C3978_53A6_4665_9917_DD325F47EE23__INCLUDED_)
