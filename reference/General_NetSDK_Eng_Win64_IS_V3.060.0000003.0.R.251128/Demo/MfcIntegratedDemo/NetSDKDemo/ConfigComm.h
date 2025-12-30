#if !defined(AFX_CONFIGCOMM_H__E0D58DF0_BB02_4D9F_AF3C_F334C0A0F91F__INCLUDED_)
#define AFX_CONFIGCOMM_H__E0D58DF0_BB02_4D9F_AF3C_F334C0A0F91F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigComm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigComm dialog

class CConfigComm : public CDialog
{
// Construction
public:
	CConfigComm(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigComm)
	enum { IDD = IDD_CONFIG_COMM };
	CComboBox	m_stopbit485sel;
	CComboBox	m_stopbit232sel;
	CComboBox	m_protocol485sel;
	CComboBox	m_parity485sel;
	CComboBox	m_parity232sel;
	CComboBox	m_function232sel;
	CComboBox	m_485comsel;
	CComboBox	m_232comsel;
	CComboBox	m_databit485sel;
	CComboBox	m_databit232sel;
	CComboBox	m_baud485sel;
	CComboBox	m_baud232sel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigComm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigComm)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCombo232com();
	afx_msg void OnSelchangeCombo485com();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUndoAll();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetConfig();
	void SetDevice(DeviceNode *pDev);
	
private:
	DeviceNode *m_pDev;
	DHDEV_COMM_CFG m_comCFG;
	BOOL m_bInited;

	int m_232Index;
	int m_485Index;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGCOMM_H__E0D58DF0_BB02_4D9F_AF3C_F334C0A0F91F__INCLUDED_)
