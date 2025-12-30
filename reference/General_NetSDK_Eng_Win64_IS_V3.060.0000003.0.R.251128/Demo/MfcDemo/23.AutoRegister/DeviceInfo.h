#include "afxcmn.h"
#if !defined(AFX_DEVICEINFO_H__9DD1F76B_2144_49D2_AB06_1B0EED755867__INCLUDED_)
#define AFX_DEVICEINFO_H__9DD1F76B_2144_49D2_AB06_1B0EED755867__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DeviceInfo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeviceInfo dialog

class CDeviceInfo : public CDialog
{
// Construction
public:
	CDeviceInfo(CWnd* pParent = NULL,LLONG lLoginID = 0);   // standard constructor
    LLONG m_lLoginID;
// Dialog Data
	//{{AFX_DATA(CDeviceInfo)
	enum { IDD = IDD_DIALOG_DEVICEINFO };
	CString	m_strDeviceName;
	CString	m_strID;
	CString	m_strPsw;
	int		m_nPort;
	CString	m_strUserName;
	int		m_nServerPort;
	CString	m_strServerIP;
	//}}AFX_DATA
    CFG_DVRIP_INFO m_stuDvrIpInfo;
    BOOL m_bStatus;
	CString	m_strIP;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeviceInfo)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnGet();
	afx_msg void OnBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadio();
	BOOL m_bRadio;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICEINFO_H__9DD1F76B_2144_49D2_AB06_1B0EED755867__INCLUDED_)
