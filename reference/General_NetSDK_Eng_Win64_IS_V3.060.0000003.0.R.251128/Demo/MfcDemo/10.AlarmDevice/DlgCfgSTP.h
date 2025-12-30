#if !defined(AFX_DLGCFGSTP_H__BC20714E_386E_4E12_8847_9E8BE7D56B65__INCLUDED_)
#define AFX_DLGCFGSTP_H__BC20714E_386E_4E12_8847_9E8BE7D56B65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgSTP.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSTP dialog

class CDlgCfgSTP : public CDialog
{
// Construction
public:
	CDlgCfgSTP(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgSTP)
	enum { IDD = IDD_DLG_CFG_STP };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgSTP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgSTP)
	virtual BOOL OnInitDialog();
	afx_msg void OnStpButtonGet();
	afx_msg void OnStpButtonSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void DlgToStu();
	void StuToDlg();
	BOOL getCfgFromDevice();
	BOOL setCfgToDevice(); 
	
	LLONG					m_lLoginId;
	CFG_STP_INFO			m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGSTP_H__BC20714E_386E_4E12_8847_9E8BE7D56B65__INCLUDED_)
