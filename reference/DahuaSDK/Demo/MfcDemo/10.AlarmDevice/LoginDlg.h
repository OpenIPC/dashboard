#if !defined(AFX_LOGINDLG_H__FF5BBC84_CFD4_4A88_A99B_E42D2DADB105__INCLUDED_)
#define AFX_LOGINDLG_H__FF5BBC84_CFD4_4A88_A99B_E42D2DADB105__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoginDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog

class CLoginDlg : public CDialog
{
// Construction
public:
	CLoginDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoginDlg)
	enum { IDD = IDD_DIALOG_LOGIN };
	CComboBox	m_cbAddr;
	//}}AFX_DATA

	LLONG	GetLoginHandle() const { return m_lLoginID; }
	LPCTSTR GetDeviceAddr() const { return m_strAddr; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoginDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	LLONG			m_lLoginID;
	CString			m_strAddr;
	NET_DEVICEINFO_Ex	m_stuDevInfo;

	// Generated message map functions
	//{{AFX_MSG(CLoginDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGINDLG_H__FF5BBC84_CFD4_4A88_A99B_E42D2DADB105__INCLUDED_)
