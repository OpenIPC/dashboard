#if !defined(AFX_CONFIGRECORD_H__B8EDAFAA_8D04_4BD8_9C1D_6B69810410D3__INCLUDED_)
#define AFX_CONFIGRECORD_H__B8EDAFAA_8D04_4BD8_9C1D_6B69810410D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigRecord.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigRecord dialog

class CConfigRecord : public CDialog
{
// Construction
public:
	CConfigRecord(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigRecord)
	enum { IDD = IDD_CONFIG_RECORD };
	CButton	m_redundancychk;
	CComboBox	m_weeksel;
	CComboBox	m_channelsel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigRecord)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigRecord)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUndoAll();
	afx_msg void OnApply();
	afx_msg void OnSelchangeComboRecordweek();
	afx_msg void OnSelchangeComboChanno();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
public:
	void GetConfig();
	void SetDevice(DeviceNode *pDev);
	
private:
	DeviceNode *m_pDev;
	DHDEV_RECORD_CFG m_recCFG[16];
	BOOL m_bInited;
	int m_chnIdx;
	int m_weekIdx;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGRECORD_H__B8EDAFAA_8D04_4BD8_9C1D_6B69810410D3__INCLUDED_)
