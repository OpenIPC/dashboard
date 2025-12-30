#if !defined(AFX_DISKCONTROL_H__05C35818_7A50_48E3_871F_98C8BBBD21DB__INCLUDED_)
#define AFX_DISKCONTROL_H__05C35818_7A50_48E3_871F_98C8BBBD21DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiskControl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDiskControl dialog

class CDiskControl : public CDialog
{
// Construction
public:
	CDiskControl(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDiskControl)
	enum { IDD = IDD_DISK_CONTROL };
	CComboBox	m_disksel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiskControl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDiskControl)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeDiskselCombo();
	/*
	afx_msg void OnClearData();
	afx_msg void OnSetReadWrite();
	afx_msg void OnSetReadOnly();
	afx_msg void OnSetRedundant();
	afx_msg void OnErrorRecovery();
	*/
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetDeviceId(DeviceNode *dev);

private:
	NET_DEV_WORKSTATE m_devWorkState;
	DeviceNode *m_dev;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISKCONTROL_H__05C35818_7A50_48E3_871F_98C8BBBD21DB__INCLUDED_)
