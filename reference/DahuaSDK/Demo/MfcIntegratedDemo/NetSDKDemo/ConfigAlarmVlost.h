#if !defined(AFX_CONFIGALARMVLOST_H__D323A0C6_9F01_4D44_992C_A59818F54823__INCLUDED_)
#define AFX_CONFIGALARMVLOST_H__D323A0C6_9F01_4D44_992C_A59818F54823__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarmVlost.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmVlost dialog

class CConfigAlarmVlost : public CDialog
{
// Construction
public:
	CConfigAlarmVlost(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarmVlost)
	enum { IDD = IDD_CONFIG_ALARM_VLOST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarmVlost)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarmVlost)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboVideochl();
	afx_msg void OnSelchangeComboAlarmweek();
	afx_msg void OnSelchangeComboAlarmptzchl();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetDevice(DeviceNode *pDev);
	void SetVlostCFG(DH_VIDEO_LOST_CFG *cfg);
	void GetVlostCFG(DH_VIDEO_LOST_CFG *cfg);
	void ShowConfig();
	void UndoAll();

private:
	DeviceNode *m_pDev;
	DH_VIDEO_LOST_CFG m_vlostCFG[DH_MAX_VIDEO_IN_NUM];
	BOOL m_bReady;
	int m_chlIdx;
	int m_weekIdx;
	int m_ptzchlIdx;
	int m_dontsave;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARMVLOST_H__D323A0C6_9F01_4D44_992C_A59818F54823__INCLUDED_)
