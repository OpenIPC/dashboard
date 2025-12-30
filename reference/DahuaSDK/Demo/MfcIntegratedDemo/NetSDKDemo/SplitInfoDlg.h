#if !defined(AFX_SPLITINFODLG_H__77022332_8C79_4960_B67C_1EF0FEDD3BCC__INCLUDED_)
#define AFX_SPLITINFODLG_H__77022332_8C79_4960_B67C_1EF0FEDD3BCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SplitInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSplitInfoDlg dialog

class CSplitInfoDlg : public CDialog
{
// Construction
public:
	CSplitInfoDlg(CWnd* pParent = NULL);   // standard constructor

	void UpdateDeviceKbps();
	void OnRefreshInfo();
// Dialog Data
	//{{AFX_DATA(CSplitInfoDlg)
	enum { IDD = IDD_SPLIT_INFO };
	CComboBox	m_kbpssel;
	CComboBox	m_devicesel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:

	int UpdateDeviceKbpsCallback_Imp(const DeviceNode& node);
	int InitComboxCallback_Imp(const DeviceNode& node);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSplitInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSetDevicebps();
	afx_msg void OnSelchangeBpsDevsel();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLITINFODLG_H__77022332_8C79_4960_B67C_1EF0FEDD3BCC__INCLUDED_)
