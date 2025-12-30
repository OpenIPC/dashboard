#if !defined(AFX_EXTPTZCTRL_H__12D9C4FD_3019_49AA_8567_5444C8AE6CF0__INCLUDED_)
#define AFX_EXTPTZCTRL_H__12D9C4FD_3019_49AA_8567_5444C8AE6CF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExtPtzCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExtPtzCtrl dialog

class CExtPtzCtrl : public CDialog
{
	LLONG m_DeviceID ;
	DWORD m_Channel ;
// Construction
public:
	CExtPtzCtrl(CWnd* pParent = NULL);   // standard constructor
	void SetExtPtzParam(LLONG iHandle, int iChannel);
	void PtzExtControl(DWORD dwCommand, DWORD dwParam = 0);
	void PtzGetState();

// Dialog Data
	//{{AFX_DATA(CExtPtzCtrl)
	enum { IDD = IDD_EXT_PTZCTRL };
	CComboBox	m_auxNosel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExtPtzCtrl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExtPtzCtrl)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePtzTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnPresetAdd();
	afx_msg void OnPresetDele();
	afx_msg void OnPresetSet();
	afx_msg void OnCruiseAddPoint();
	afx_msg void OnCruiseDelPoint();
	afx_msg void OnCruiseDelGroup();
	afx_msg void OnStartCruise();
	afx_msg void OnStopCruise();
	afx_msg void OnLampActivate();
	afx_msg void OnLampDeactivate();
	afx_msg void OnRotateStart();
	afx_msg void OnRotateStop();
	afx_msg void OnLineSetLeft();
	afx_msg void OnLineSetRight();
	afx_msg void OnLineStart();
	afx_msg void OnLineStop();
	afx_msg void OnModeSetBegin();
	afx_msg void OnModeSetDelete();
	afx_msg void OnModeSetEnd();
	afx_msg void OnModeStart();
	afx_msg void OnModeStop();
	afx_msg void OnQueryAlarm();
	afx_msg void OnFastGo();
	afx_msg void OnAuxOpen();
	afx_msg void OnAuxClose();
	afx_msg void OnLightOpen();
	afx_msg void OnLightClose();
	afx_msg void OnTest();
	afx_msg void OnTest2();
	afx_msg void OnGetPos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	UINT m_presetPoint;
	UINT m_cruiseGroup;
	UINT m_modeNo;

	UINT m_pos_x;
	UINT m_pos_y;
	UINT m_pos_zoom;

	UINT m_auxNo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXTPTZCTRL_H__12D9C4FD_3019_49AA_8567_5444C8AE6CF0__INCLUDED_)
