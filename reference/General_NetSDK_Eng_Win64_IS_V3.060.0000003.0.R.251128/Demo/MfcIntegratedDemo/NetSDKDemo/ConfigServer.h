#if !defined(AFX_CONFIGSERVER_H__BACEC8D5_EFAA_4DBE_9F2B_C9F4C42016FC__INCLUDED_)
#define AFX_CONFIGSERVER_H__BACEC8D5_EFAA_4DBE_9F2B_C9F4C42016FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigServer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigServer dialog

class CConfigServer : public CDialog
{
// Construction
public:
	CConfigServer(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigServer)
	enum { IDD = IDD_CONFIG_SERVER };
	CComboBox	m_datesprsel;
	CComboBox	m_timefmtsel;
	CComboBox	m_datefmtsel;
	CComboBox	m_videofmtsel;
	CComboBox	m_overwritesel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigServer)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUndoAll();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetConfig();
	void SetDevice(DeviceNode *pDev);

private:
	DeviceNode *m_pDev;
	DHDEV_SYSTEM_ATTR_CFG m_sysCFG;
	BOOL m_bInited;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGSERVER_H__BACEC8D5_EFAA_4DBE_9F2B_C9F4C42016FC__INCLUDED_)
