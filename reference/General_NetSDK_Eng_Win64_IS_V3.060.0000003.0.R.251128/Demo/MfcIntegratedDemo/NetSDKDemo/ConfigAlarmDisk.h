#if !defined(AFX_CONFIGALARMDISK_H__49DD85E5_416E_41CA_9F18_325D7CAB3CC2__INCLUDED_)
#define AFX_CONFIGALARMDISK_H__49DD85E5_416E_41CA_9F18_325D7CAB3CC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarmDisk.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmDisk dialog

class CConfigAlarmDisk : public CDialog
{
// Construction
public:
	CConfigAlarmDisk(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarmDisk)
	enum { IDD = IDD_CONFIG_ALARM_DISK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarmDisk)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarmDisk)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARMDISK_H__49DD85E5_416E_41CA_9F18_325D7CAB3CC2__INCLUDED_)
