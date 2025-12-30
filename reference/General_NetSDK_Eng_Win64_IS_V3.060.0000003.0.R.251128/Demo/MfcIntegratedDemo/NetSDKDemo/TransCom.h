#if !defined(AFX_TRANSCOM_H__697041C9_4904_48D9_8E53_DA82202C3BE5__INCLUDED_)
#define AFX_TRANSCOM_H__697041C9_4904_48D9_8E53_DA82202C3BE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransCom.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTransCom dialog

class CTransCom : public CDialog
{
	LLONG m_devHandle;
	BOOL m_isComOpen;
	LLONG m_comHandle;

	CDC *m_revDC;
// Construction
public:
	CTransCom(CWnd* pParent = NULL);   // standard constructor
	void UpdataOpenCloseState();
	void SetDeviceId(LLONG nDeviceId);

	void ComRecieveData(LLONG lLoginID, LLONG lTransComChannel,char *pBuffer, DWORD dwBufSize);

// Dialog Data
	//{{AFX_DATA(CTransCom)
	enum { IDD = IDD_TRANSCOM };
	CComboBox	m_comport;
	CComboBox	m_stopbit;
	CComboBox	m_comsel;
	CComboBox	m_parity;
	CComboBox	m_databit;
	CComboBox	m_baudrate;
	CString	m_recievestring;
	CString	m_sendstring;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransCom)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTransCom)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenclose();
	afx_msg void OnComsend();
	afx_msg void OnDeleterevice();
	afx_msg void OnDeletesend();
	afx_msg void OnClose();
	afx_msg void OnCheckComport();
	afx_msg void OnQueryComState();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_count;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSCOM_H__697041C9_4904_48D9_8E53_DA82202C3BE5__INCLUDED_)
