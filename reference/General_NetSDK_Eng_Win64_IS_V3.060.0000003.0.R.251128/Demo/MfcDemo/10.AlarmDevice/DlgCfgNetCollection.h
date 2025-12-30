#if !defined(AFX_DLGCFGNETCOLLECTION_H__47A4D0B3_2802_4184_ABF5_C7C5641EEE49__INCLUDED_)
#define AFX_DLGCFGNETCOLLECTION_H__47A4D0B3_2802_4184_ABF5_C7C5641EEE49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgNetCollection.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetCollection dialog

class CDlgCfgNetCollection : public CDialog
{
// Construction
public:
	CDlgCfgNetCollection(CWnd* pParent = NULL, LLONG lLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgNetCollection)
	enum { IDD = IDD_DLG_CFG_NETCOLLECTION };
	CIPAddressCtrl	m_ctrlIP;
	CComboBox	m_cmbChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgNetCollection)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgNetCollection)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeNetcollectionCmbChn();
	afx_msg void OnNetcollectionBtnGet();
	afx_msg void OnNetcollectionBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void					InitDlg();
    BOOL					getInfo();
    BOOL					showInfo();
    BOOL					SetConfigToDevice();
    BOOL					GetConfigFromDevice();

private:
	LLONG					m_lLoginID;
	CFG_NET_COLLECTION_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGNETCOLLECTION_H__47A4D0B3_2802_4184_ABF5_C7C5641EEE49__INCLUDED_)
