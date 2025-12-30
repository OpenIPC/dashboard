#if !defined(AFX_DLGCFGPHONENOTIFY_H__2540F74B_96EA_48D8_BBE2_8740DCA739E8__INCLUDED_)
#define AFX_DLGCFGPHONENOTIFY_H__2540F74B_96EA_48D8_BBE2_8740DCA739E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgPhoneNotify.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPhoneNotify dialog

class CDlgCfgPhoneNotify : public CDialog
{
// Construction
public:
	CDlgCfgPhoneNotify(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgPhoneNotify)
	enum { IDD = IDD_DLG_CFG_PHONENOTIFY };
	CListCtrl	m_lsNumberList;
	CButton	m_chkEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgPhoneNotify)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgPhoneNotify)
	virtual BOOL OnInitDialog();
	afx_msg void OnPhonenotifyBtnAddnumber();
	afx_msg void OnPhonenotifyBtnDelnumber();
	afx_msg void OnPhonenotifyBtnGet();
	afx_msg void OnPhonenotifyBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg(BOOL bShow = TRUE);
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	void DlgToStu();
	void StuToDlg();
	
private:
	LLONG			m_lLoginId;
	CFG_PHONEEVENTNOTIFY_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGPHONENOTIFY_H__2540F74B_96EA_48D8_BBE2_8740DCA739E8__INCLUDED_)
