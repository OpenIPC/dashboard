#if !defined(AFX_FOCUSVALUEDLG_H__EA5E467D_28F1_4B05_B0FE_9821881CDF1E__INCLUDED_)
#define AFX_FOCUSVALUEDLG_H__EA5E467D_28F1_4B05_B0FE_9821881CDF1E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FocusValueDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFocusValueDlg dialog

class CFocusValueDlg : public CDialog
{
// Construction
public:
	CFocusValueDlg(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFocusValueDlg)
	enum { IDD = IDD_FOCUSVALUE_DIALOG };
	CComboBox	m_Cmb_FocusMode;
	CComboBox	m_Cmb_Type;
	CComboBox	m_Cmb_Sensitivity;
	CComboBox	m_Cmb_LimitMode;
	CComboBox	m_Cmb_IrCorrection;
	BOOL	m_bAutoFocus;
	int		m_nLimit;
	//}}AFX_DATA
    NET_VIDEOIN_FOCUSVALUE_INFO  m_stuFocusValue;
    NET_VIDEOIN_FOCUSMODE_INFO  m_stuFocusMode;
    int m_nChannel;
    LLONG m_LoginID;
    BOOL GetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType,NET_EM_CONFIG_TYPE EmConfigType = NET_EM_CONFIG_DAYTIME);
    BOOL SetVideoIn(NET_EM_CFG_OPERATE_TYPE emOperateType,NET_EM_CONFIG_TYPE EmConfigType = NET_EM_CONFIG_DAYTIME);
    void StuToDlg(NET_EM_CFG_OPERATE_TYPE emOperateType);
    void DlgToStu(NET_EM_CFG_OPERATE_TYPE emOperateType);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFocusValueDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFocusValueDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnButton1();
	afx_msg void OnSet();
	afx_msg void OnSelchangeCmbType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOCUSVALUEDLG_H__EA5E467D_28F1_4B05_B0FE_9821881CDF1E__INCLUDED_)
