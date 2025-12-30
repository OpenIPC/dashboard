#if !defined(AFX_DEFOG_H__1B32F85F_B50C_468E_B3B4_269F2FEE4255__INCLUDED_)
#define AFX_DEFOG_H__1B32F85F_B50C_468E_B3B4_269F2FEE4255__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Defog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDefog dialog

class CDefog : public CDialog
{
// Construction
public:
	CDefog(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDefog)
	enum { IDD = IDD_DEFOG_DIALOG };
	CComboBox	m_Cmb_Type;
	CComboBox	m_Cmb_IntensityMode;
	CComboBox	m_Cmb_DefogMode;
	BOOL	m_bDefog;
	int		m_nIntensity;
	int		m_nLightLevel;
	//}}AFX_DATA

    int m_nChannel;
    LLONG m_LoginID;
    NET_VIDEOIN_DEFOG_INFO m_stuDefog;
    BOOL GetVideoIn(NET_EM_CONFIG_TYPE emConfigType = NET_EM_CONFIG_DAYTIME);
    BOOL SetVideoIn(NET_EM_CONFIG_TYPE emConfigType = NET_EM_CONFIG_DAYTIME);
    void StuToDlg();
    void DlgToStu();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDefog)
	afx_msg void OnGet();
	afx_msg void OnSet();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCmbType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFOG_H__1B32F85F_B50C_468E_B3B4_269F2FEE4255__INCLUDED_)
