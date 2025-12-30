#if !defined(AFX_PTZPANNEL_H__5E28A19B_FA0C_4337_ACC1_F81BF3EBC628__INCLUDED_)
#define AFX_PTZPANNEL_H__5E28A19B_FA0C_4337_ACC1_F81BF3EBC628__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PTZPannel.h : header file

#include "Exbutton.h"

/////////////////////////////////////////////////////////////////////////////
// CPTZPannel dialog

class CPTZPannel : public CDialog
{
// Construction
public:
	CPTZPannel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPTZPannel)
	enum { IDD = IDD_PANNEL_PTZ };
	CExButton	m_zoom_wide;
	CExButton	m_zoom_tele;
	CExButton	m_ptz_up;
	CExButton	m_ptz_rightup;
	CExButton	m_ptz_rightdown;
	CExButton	m_ptz_right;
	CExButton	m_ptz_leftup;
	CExButton	m_ptz_leftdown;
	CExButton	m_ptz_left;
	CExButton	m_ptz_down;
	CExButton	m_iris_open;
	CExButton	m_iris_close;
	CExButton	m_focus_near;
	CExButton	m_focus_far;
	UINT	m_ptz_data;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPTZPannel)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPTZPannel)
	virtual BOOL OnInitDialog();	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SendPtzControl(int type, BOOL stop);   //ÔÆÌ¨¿ØÖÆ²Ù×÷
private:
	BOOL m_bPTZCtrl;
	CBrush m_myBrush;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PTZPANNEL_H__5E28A19B_FA0C_4337_ACC1_F81BF3EBC628__INCLUDED_)
