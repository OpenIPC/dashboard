#if !defined(AFX_DEVICEWORKSTATE_H__7818B4F5_1A0E_4E5B_B37F_B9702C5A8FB7__INCLUDED_)
#define AFX_DEVICEWORKSTATE_H__7818B4F5_1A0E_4E5B_B37F_B9702C5A8FB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeviceWorkState.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeviceWorkState dialog

class CDeviceWorkState : public CDialog
{
// Construction
public:
	CDeviceWorkState(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeviceWorkState)
	enum { IDD = IDD_DEVICE_WORKSTATE };
	CComboBox	m_disksel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceWorkState)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeviceWorkState)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeDiskselCombo();
	afx_msg void OnSelchangeChanselCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SetDevice(DeviceNode *dev);

private:
	DeviceNode *m_dev;
//	NET_DEV_WORKSTATE m_workState;
	NET_CLIENT_STATE m_almState;
	char m_shltAlarm[16];
	char m_recording[16];
	DH_HARDDISK_STATE m_diskState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICEWORKSTATE_H__7818B4F5_1A0E_4E5B_B37F_B9702C5A8FB7__INCLUDED_)
