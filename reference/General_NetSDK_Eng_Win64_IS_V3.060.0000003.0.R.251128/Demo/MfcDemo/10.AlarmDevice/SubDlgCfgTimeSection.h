#if !defined(AFX_SUBDLGCFGTIMESECTION_H__A169B031_BA78_4F2B_948F_E93ADF265B94__INCLUDED_)
#define AFX_SUBDLGCFGTIMESECTION_H__A169B031_BA78_4F2B_948F_E93ADF265B94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SubDlgCfgTimeSection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgTimeSection dialog

class CSubDlgCfgTimeSection : public CDialog
{
// Construction
public:
	CSubDlgCfgTimeSection(CWnd* pParent = NULL);   // standard constructor
	
	void SetTimeSection(const CFG_TIME_SECTION* pstuTimeSection)
	{
		memcpy(&m_stuTimeSection[0][0], pstuTimeSection, sizeof(m_stuTimeSection));
	}
	
	const CFG_TIME_SECTION* GetTimeSection(int i, int j)
	{
		if ((i >=0 && i < WEEK_DAY_NUM) && (j >= 0 && j < MAX_REC_TSECT))
		{
			return &m_stuTimeSection[i][j];
		} 
		else
		{
			return NULL;
		}
	}

// Dialog Data
	//{{AFX_DATA(CSubDlgCfgTimeSection)
	enum { IDD = IDD_SUBDLG_TIMESECTIOIN };
	CComboBox	m_cmbWeekDay;
	CButton	m_chkSeg1;
	CButton	m_chkSeg2;
	CButton	m_chkSeg3;
	CButton	m_chkSeg4;
	CButton	m_chkSeg5;
	CButton	m_chkSeg6;
	CDateTimeCtrl	m_dtp1End;
	CDateTimeCtrl	m_dtp1Start;
	CDateTimeCtrl	m_dtp2End;
	CDateTimeCtrl	m_dtp2Start;
	CDateTimeCtrl	m_dtp3End;
	CDateTimeCtrl	m_dtp3Start;
	CDateTimeCtrl	m_dtp4End;
	CDateTimeCtrl	m_dtp4Start;
	CDateTimeCtrl	m_dtp5End;
	CDateTimeCtrl	m_dtp5Start;
	CDateTimeCtrl	m_dtp6End;
	CDateTimeCtrl	m_dtp6Start;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSubDlgCfgTimeSection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSubDlgCfgTimeSection)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTimesectionCmbWeekday();
	virtual void OnOK();
	afx_msg void OnEventhandlerChkSeg1Enable();
	afx_msg void OnEventhandlerChkSeg2Enable();
	afx_msg void OnEventhandlerChkSeg3Enable();
	afx_msg void OnEventhandlerChkSeg4Enable();
	afx_msg void OnEventhandlerChkSeg5Enable();
	afx_msg void OnEventhandlerChkSeg6Enable();
	afx_msg void OnDatetimechangeEventhandlerDtp1Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp1End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp2Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp2End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp3Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp3End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp4Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp4End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp5Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp5End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp6Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp6End(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	void ShowTimeSection(int nSeg, const CFG_TIME_SECTION& stuTimeSection);
	void GetTimeSectionFromDlg();
	void CfgTimeSectionToDTP(const CFG_TIME_SECTION& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd);
	void DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, CFG_TIME_SECTION& stuTimeSection);
	
private:
	CFG_TIME_SECTION		m_stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBDLGCFGTIMESECTION_H__A169B031_BA78_4F2B_948F_E93ADF265B94__INCLUDED_)
