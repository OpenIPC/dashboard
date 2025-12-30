#if !defined(AFX_DLGCFGMOBILE_H__347D68CE_E649_497F_BB6E_D82FFCA286E8__INCLUDED_)
#define AFX_DLGCFGMOBILE_H__347D68CE_E649_497F_BB6E_D82FFCA286E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgMobile.h : header file
//

typedef struct  
{
	EM_MSG_TYPE	emType;
	char*		pszName;
}Demo_MsgType;

const Demo_MsgType stuDemoMsgType[] = 
{
	{EM_MSG_UNKNOWN, "Unknown"},
	{EM_MSG_SMS, "SMS"},
	{EM_MSG_MMS, "MMS"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMobile dialog

class CDlgCfgMobile : public CDialog
{
// Construction
public:
	CDlgCfgMobile(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgMobile)
	enum { IDD = IDD_DLG_CFG_MOBILE };
	CButton	m_chkEnable;
	CListCtrl	m_lsNumberList;
	CComboBox	m_cmbType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgMobile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
	protected:
		
		// Generated message map functions
		//{{AFX_MSG(CDlgCfgMobile)
		virtual BOOL OnInitDialog();
		afx_msg void OnMobileBtnAddnumber();
		afx_msg void OnMobileBtnDelnumber();
		afx_msg void OnMobileBtnGet();
		afx_msg void OnMobileBtnSet();
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
		CFG_MOBILE_INFO	m_stuInfo;
		
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGMOBILE_H__347D68CE_E649_497F_BB6E_D82FFCA286E8__INCLUDED_)
