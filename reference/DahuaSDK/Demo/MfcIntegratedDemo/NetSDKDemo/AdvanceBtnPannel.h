#include "afxwin.h"
#if !defined(AFX_ADVANCEBTNPANNEL_H__3631B30D_C8B3_4B56_BE83_7ACC611040F5__INCLUDED_)
#define AFX_ADVANCEBTNPANNEL_H__3631B30D_C8B3_4B56_BE83_7ACC611040F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvanceBtnPannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAdvanceBtnPannel dialog

class CAdvanceBtnPannel : public CDialog
{
// Construction
public:
	CAdvanceBtnPannel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAdvanceBtnPannel)
	enum { IDD = IDD_PANNEL_ADVANCE_BTN };
	CButton	m_isTalkOpen;
	CButton	m_isSoundOpen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvanceBtnPannel)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAdvanceBtnPannel)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenSound();
	afx_msg void OnOpenTalk();
	afx_msg void OnUpdateDevice();
	afx_msg void OnRecordstate();
	afx_msg void OnRebootDevice();
	afx_msg void OnShutdownDevice();
	afx_msg void OnAlarmIoControl();
	afx_msg void OnDeviceState();
	afx_msg void OnCaptureScreen();
	afx_msg void OnSetIFrame();
	afx_msg void OnShowFlux();
	afx_msg void OnTranscom();
	afx_msg void OnDdnsQueryIp();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDeviceWorkstate();
	afx_msg void OnDeviceControldisk();
	afx_msg void OnDeviceUserinfo();
	afx_msg void OnTest();
	afx_msg void OnAudioBroadcast();
	afx_msg void OnCycleMonitor();
	afx_msg void OnBnClickedSaveRealdata();
	afx_msg void OnBnClickedSavecbdataraw();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void EnableTalk(BOOL bEnable);
	void SetCheckSound(int nCheck);
	void SetCheckTalk(int nCheck);
	BOOL IsTalkOpen(void);		
	
private:
	CBrush m_myBrush;
public:	
	CButton m_IsSaveRealData;
	CButton m_IsSaveDataRaw;	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCEBTNPANNEL_H__3631B30D_C8B3_4B56_BE83_7ACC611040F5__INCLUDED_)
