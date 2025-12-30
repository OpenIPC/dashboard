#if !defined(AFX_AUTOMAINTENANCE_H__D38B0103_DAA8_43B0_BEC9_0543AB744013__INCLUDED_)
#define AFX_AUTOMAINTENANCE_H__D38B0103_DAA8_43B0_BEC9_0543AB744013__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoMaintenance.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAutoMaintenance dialog

class CAutoMaintenance : public CDialog
{
// Construction
public:
	CAutoMaintenance(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAutoMaintenance)
	enum { IDD = IDD_DIALOG2 };
	CComboBox	m_autotime;
	CComboBox	m_autodel;
	CComboBox	m_autoday;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoMaintenance)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAutoMaintenance)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetDeviceNode(DeviceNode *dev) {m_dev = dev;}

private:
	DeviceNode *m_dev;
	DHDEV_AUTOMT_CFG m_atC;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOMAINTENANCE_H__D38B0103_DAA8_43B0_BEC9_0543AB744013__INCLUDED_)
