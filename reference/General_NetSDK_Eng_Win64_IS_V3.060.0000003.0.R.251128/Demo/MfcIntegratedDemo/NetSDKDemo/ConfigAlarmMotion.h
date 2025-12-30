#if !defined(AFX_CONFIGALARMMOTION_H__7D11DE0F_78F0_4DD5_B4C8_A0887D29E482__INCLUDED_)
#define AFX_CONFIGALARMMOTION_H__7D11DE0F_78F0_4DD5_B4C8_A0887D29E482__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarmMotion.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotion dialog

class CConfigAlarmMotion : public CDialog
{
// Construction
public:
	CConfigAlarmMotion(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarmMotion)
	enum { IDD = IDD_CONFIG_ALARM_MOTION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarmMotion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarmMotion)
	afx_msg void OnSelchangeComboVideochl();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeComboAlarmweek();
	afx_msg void OnSelchangeComboAlarmptzchl();
	afx_msg void OnBtnArea();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetDevice(DeviceNode *pDev);
	void SetMotionCFG(DH_MOTION_DETECT_CFG *cfg);
	void GetMotionCFG(DH_MOTION_DETECT_CFG *cfg);
	void ShowConfig();
	void UndoAll();

private:
	DeviceNode *m_pDev;
	DH_MOTION_DETECT_CFG m_motionCFG[DH_MAX_VIDEO_IN_NUM];
	BOOL m_bReady;
	int m_chlIdx;
	int m_weekIdx;
	int m_ptzchlIdx;
	int m_dontsave;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARMMOTION_H__7D11DE0F_78F0_4DD5_B4C8_A0887D29E482__INCLUDED_)
