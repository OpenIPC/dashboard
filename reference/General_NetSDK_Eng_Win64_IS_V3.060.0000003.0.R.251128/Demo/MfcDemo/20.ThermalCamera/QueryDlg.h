#if !defined(AFX_QUERYDLG_H__59F7BDC1_5CEC_40BC_A3E4_25ABD2C028C6__INCLUDED_)
#define AFX_QUERYDLG_H__59F7BDC1_5CEC_40BC_A3E4_25ABD2C028C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// QueryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg dialog

class CQueryDlg : public CDialog
{
// Construction
public:
	CQueryDlg(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQueryDlg)
	enum { IDD = IDD_DLG_QUERY };
	CComboBox	m_Combo_PointNum;
	CComboBox	m_Combo_Querytype;
	CComboBox	m_Combo_ChannelNum;
	COleDateTime	m_StartDay;
	CTime	m_StartTime;
	COleDateTime	m_EndDay;
	CTime	m_EndTime;
	int		m_nBeginNum;
	int		m_nReallyNum;
	int		m_nCount;
	CString	m_strTime;
	int		m_nPresentID;
	int		m_nRuleID;
	CString	m_strName;
	int		m_nChannel;
	short		m_nX;
	short		m_nY;
	CString	m_strMeasuretype;
	CString	m_strUnit;
	float	m_fAveTemp;
	float	m_fMaxTemp;
	float	m_fMidTemp;
	float	m_fMinTemp;
	float	m_fStdTemp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQueryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    NET_OUT_RADIOMETRY_DOFIND stOutDo;
    LLONG m_lLoginID;
    LONG m_nFinderHanle; // int
	// Generated message map functions
	//{{AFX_MSG(CQueryDlg)
	afx_msg void OnBtnQuery();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_QUERYDLG_H__59F7BDC1_5CEC_40BC_A3E4_25ABD2C028C6__INCLUDED_)
