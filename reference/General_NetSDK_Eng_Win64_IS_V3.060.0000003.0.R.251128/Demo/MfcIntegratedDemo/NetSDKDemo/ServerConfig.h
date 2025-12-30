#if !defined(AFX_SERVERCONFIG_H__D7819E2B_8392_4CE1_B0C6_73ECB533432B__INCLUDED_)
#define AFX_SERVERCONFIG_H__D7819E2B_8392_4CE1_B0C6_73ECB533432B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServerConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CServerConfig dialog

class CServerConfig : public CDialog
{
// Construction
public:
	CServerConfig(CWnd* pParent = NULL);   // standard constructor

	void SetLoginID(LONG lLoginID);

	
// Dialog Data
	//{{AFX_DATA(CServerConfig)
	enum { IDD = IDD_CHILD_SERVERCFG };
	CComboBox	m_serverTypesel;
	CComboBox	m_PPPOEsel;
	CComboBox	m_cycleRecordsel;
	CComboBox	m_interfacesel;
	CString	m_dvrName;
	UINT	m_alarmInNum;
	UINT	m_alarmOutNum;
	CString	m_DNSIP;
	CString	m_DSPVersion;
	CString	m_dvrIP;
	CString	m_dvrGatewayIP;
	CString	m_hardwareVersion;
	CString	m_MACAddr;
	CString	m_manageHostIP;
	CString	m_multicastIP;
	CString	m_NASDir;
	CString	m_NASIP;
	CString	m_PPPOEIP;
	CString	m_PPPOEUserName;
	CString	m_serialNo;
	CString	m_softwareVersion;
	UINT	m_manageHostPort;
	UINT	m_httpPort;
	UINT	m_dvrPort;
	CString	m_dvrIPMask;
	UINT	m_YKQID;
	CString	m_PPPOEPsw;
	DWORD	m_panelVersion;
	UINT	m_channelNum;
	BYTE	m_harddiskNum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CServerConfig)
	afx_msg void OnApply();
	afx_msg void OnRestore();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	NET_DEV_DEVICECFG m_deviceCFG;
	NET_DEV_NETCFG m_netCFG;
	LONG m_lLoginID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERCONFIG_H__D7819E2B_8392_4CE1_B0C6_73ECB533432B__INCLUDED_)
