#if !defined(AFX_PTZCONTROLDLG_H__281C08D5_2B1C_4042_88C3_E8270812E61D__INCLUDED_)
#define AFX_PTZCONTROLDLG_H__281C08D5_2B1C_4042_88C3_E8270812E61D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PTZControlDlg.h : header file
//
#include "ExButton.h"
/////////////////////////////////////////////////////////////////////////////
// CPTZControlDlg dialog

class CPTZControlDlg : public CDialog
{
// Construction
public:
	CPTZControlDlg(CWnd* pParent = NULL);   // standard constructor
    LLONG   m_lLoginID;
    int     m_nChannel;
    void PtzControl(int type, BOOL stop);
    void PtzExtControl(DWORD dwCommand, DWORD dwParam = 0);
// Dialog Data
	//{{AFX_DATA(CPTZControlDlg)
	enum { IDD = IDD_DIALOG_PTZCONTROL };
	CComboBox	m_comboPTZData;
	int		m_presetData;
    CExButton	m_iris_open;
    CExButton	m_iris_close;
    CExButton	m_focus_far;
    CExButton	m_focus_near;
    CExButton	m_zoom_tele;
    CExButton	m_zoom_wide;
    CExButton	m_ptz_rightdown;
    CExButton	m_ptz_rightup;
    CExButton	m_ptz_leftdown;
    CExButton	m_ptz_leftup;
    CExButton	m_ptz_right;
    CExButton	m_ptz_left;
    CExButton	m_ptz_down;
	CExButton	m_ptz_up;
	int		m_posX;
	int		m_posY;
	int		m_posZoom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPTZControlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:
    //PTZ extensive control function 
	// Generated message map functions
	//{{AFX_MSG(CPTZControlDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPresetSet();
	afx_msg void OnPresetAdd();
	afx_msg void OnPresetDele();
	afx_msg void OnFastGo();
	afx_msg void OnExactGo();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PTZCONTROLDLG_H__281C08D5_2B1C_4042_88C3_E8270812E61D__INCLUDED_)
