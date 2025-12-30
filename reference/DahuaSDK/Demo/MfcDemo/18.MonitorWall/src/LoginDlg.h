#if !defined(AFX_LOGINDLG_H__731F07D5_ABA6_441E_A6E7_35907E11E21C__INCLUDED_)
#define AFX_LOGINDLG_H__731F07D5_ABA6_441E_A6E7_35907E11E21C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoginDlg.h : header file
//
#define MAX_LOGIN_DEVICE	20

struct LoginDeviceInfo
{
	char		szAddress[64];
	int			nPort;
	char		szUser[64];
	char		szPassword[64];
	char		szDevClass[64];
	char		szDevType[64];
	char		reserved[700];
};

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog

class CLoginDlg : public CDialog
{
	// Construction
public:
	CLoginDlg(UINT nLoginType = LOGIN_DECODER, CWnd* pParent = NULL);   // standard constructor

	enum { LOGIN_DECODER, LOGIN_ENCODER };

	LPCTSTR DCID() const;
	
	// Dialog Data
	//{{AFX_DATA(CLoginDlg)
	enum { IDD = IDD_DIALOG_LOGIN };
	CComboBox	m_cbAddr;
	CString	m_strUser;
	CString	m_strPwd;
	//}}AFX_DATA

	LLONG			m_lLoginID;
	NET_DEVICEINFO_Ex	m_stuDevInfo;

	// ¿Ø¼þ±äÁ¿
	CString		m_strAddr;
	UINT		m_nPort;
	CString		m_strDevClass;
	CString		m_strDevType;
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoginDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	void LoadLoginDeviceHistory();
	void SaveLoginDeviceHistory();
	LPCTSTR LoginErrorString(int nError);

protected:
	UINT		m_nLoginType;

	LoginDeviceInfo m_aLoginDevs[MAX_LOGIN_DEVICE];
		
	// Generated message map functions
	//{{AFX_MSG(CLoginDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboAddress();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGINDLG_H__731F07D5_ABA6_441E_A6E7_35907E11E21C__INCLUDED_)
