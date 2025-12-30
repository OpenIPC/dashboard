#if !defined(AFX_CONFIGALARMBLIND_H__80A4C22A_2B31_49B8_B26A_C249E80AEEA0__INCLUDED_)
#define AFX_CONFIGALARMBLIND_H__80A4C22A_2B31_49B8_B26A_C249E80AEEA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarmBlind.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmBlind dialog

class CConfigAlarmBlind : public CDialog
{
// Construction
public:
	CConfigAlarmBlind(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarmBlind)
	enum { IDD = IDD_CONFIG_ALARM_BLIND };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarmBlind)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarmBlind)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeComboVideochl();
	afx_msg void OnSelchangeComboAlarmweek();
	afx_msg void OnSelchangeComboAlarmptzchl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetDevice(DeviceNode *pDev);
	void SetBlindCFG(DH_BLIND_CFG *cfg);
	void GetBlindCFG(DH_BLIND_CFG *cfg);
	void ShowConfig();
	void UndoAll();

private:
	DeviceNode *m_pDev;
	DH_BLIND_CFG m_blindCFG[DH_MAX_VIDEO_IN_NUM];
	BOOL m_bReady;
	int m_chlIdx;
	int m_weekIdx;
	int m_ptzchlIdx;
	int m_dontsave;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARMBLIND_H__80A4C22A_2B31_49B8_B26A_C249E80AEEA0__INCLUDED_)
