#if !defined(AFX_CONFIGNETWORK_H__41575BD7_FEA9_4AFF_BF93_11D5BB97F176__INCLUDED_)
#define AFX_CONFIGNETWORK_H__41575BD7_FEA9_4AFF_BF93_11D5BB97F176__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigNetwork.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigNetwork dialog

class CConfigNetwork : public CDialog
{
// Construction
public:
	CConfigNetwork(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigNetwork)
	enum { IDD = IDD_CONFIG_NETWORK };
	CComboBox	m_ctlDDNSIDList;
	CButton	m_ddnsEnable;
	CButton	m_hostEnchk;
	CComboBox	m_interfacesel;
	CComboBox	m_remotehostsel;
	CComboBox	m_ethernetsel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigNetwork)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigNetwork)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnApply();
	afx_msg void OnUndoAll();
	afx_msg void OnSelchangeComboRemotehost();
	afx_msg void OnSelchangeComboEthernet();
	afx_msg void OnSelchangeComboDdnsid();
	afx_msg void OnSelendokComboDdnsid();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetConfig();
	void SetDevice(DeviceNode *pDev);
	DH_REMOTE_HOST *GetRemoteHostType(int hostIdx, BOOL bChg = FALSE);
	
private:
	DeviceNode *m_pDev;
	DHDEV_NET_CFG m_netCFG;
	DHDEV_SYSTEM_ATTR_CFG m_sysCFG;
	DHDEV_MULTI_DDNS_CFG m_ddnsCFG;
	BOOL m_bInited;
	//Begin: Add by li_deming(11517) 2008-1-30
	int selIndexOld;
	//End:li_deming(11517)
	int m_etherIdx;
	int m_hostIdx;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGNETWORK_H__41575BD7_FEA9_4AFF_BF93_11D5BB97F176__INCLUDED_)
