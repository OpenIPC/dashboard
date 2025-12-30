#if !defined(AFX_USERCONFIG_H__7801B766_4A12_4BDC_84F2_BDD4D66A9E4A__INCLUDED_)
#define AFX_USERCONFIG_H__7801B766_4A12_4BDC_84F2_BDD4D66A9E4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserConfig dialog

class CUserConfig : public CDialog
{
// Construction
public:
	CUserConfig(CWnd* pParent = NULL);   // standard constructor


	BOOL bSetUserParams;

	void SetDevice(DeviceNode *nDev);

	DWORD dwReturned;
// Dialog Data
	//{{AFX_DATA(CUserConfig)
	enum { IDD = IDD_CHILD_USERCFG };
	CComboBox	m_usersel;
	BOOL	m_bLright_log;
	BOOL	m_bLright_more;
	BOOL	m_bLright_playback;
	BOOL	m_bLright_ptz;
	BOOL	m_bLright_rec;
	CString	m_mac1;
	CString	m_mac2;
	CString	m_mac3;
	CString	m_mac4;
	CString	m_mac5;
	CString	m_mac6;
	BOOL	m_bRright_alarm;
	BOOL	m_bRright_cfg;
	BOOL	m_bRright_localop;
	BOOL	m_bRright_log;
	BOOL	m_bRright_monitor;
	BOOL	m_bRright_more;
	BOOL	m_bRright_playback;
	BOOL	m_bRright_ptz;
	BOOL	m_bRright_rec;
	BOOL	m_bRright_serial;
	BOOL	m_bRright_talk;
	CString	m_userip;
	CString	m_username;
	CString	m_userpsw;
	CString	m_userpsw2;
	BOOL	m_bLright_cfg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserConfig)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DeviceNode *m_Device;
	NET_DEV_USER m_user;
	BOOL m_bSuperUser;
	int m_usernum;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERCONFIG_H__7801B766_4A12_4BDC_84F2_BDD4D66A9E4A__INCLUDED_)
