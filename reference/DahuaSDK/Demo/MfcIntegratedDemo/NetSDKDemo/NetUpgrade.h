#if !defined(AFX_NETUPGRADE_H__A7CA5934_7BAD_46A2_8F4C_18EEAC6513B4__INCLUDED_)
#define AFX_NETUPGRADE_H__A7CA5934_7BAD_46A2_8F4C_18EEAC6513B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NetUpgrade.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNetUpgrade dialog

class CNetUpgrade : public CDialog
{
// Construction
public:
	CNetUpgrade(CWnd* pParent = NULL);   // standard constructor

	void SetProgress();
	void SetDevice(DeviceNode *pDev)  {m_pDevice = pDev;}
	void SetProgressPos(int npos);
	void ProcessSend(unsigned int nUpgradeType = 0);
// Dialog Data
	//{{AFX_DATA(CNetUpgrade)
	enum { IDD = IDD_NETUPGRADE };
	CProgressCtrl	m_upgradeProgress;
	CString	m_strFilePath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetUpgrade)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNetUpgrade)
	afx_msg void OnOpenfile();
	afx_msg void OnSendbios();
	afx_msg void OnSendweb();
	afx_msg void OnSendboot();
	afx_msg void OnBiosandboot();
    afx_msg void OnPeripheral();
	afx_msg void OnGeoInfo();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnUpCancel();
	//}}AFX_MSG
	afx_msg LRESULT OnSetProgress(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()
private:
	CFile m_File;
	DeviceNode *m_pDevice;
	LLONG m_lUpgradeID;
	BOOL m_bStopped;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETUPGRADE_H__A7CA5934_7BAD_46A2_8F4C_18EEAC6513B4__INCLUDED_)
