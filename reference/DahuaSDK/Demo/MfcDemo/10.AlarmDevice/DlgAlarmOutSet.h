#if !defined(AFX_DLGALARMOUTSET_H__E56F1E2C_F69A_42A0_B593_69BD9F509A0C__INCLUDED_)
#define AFX_DLGALARMOUTSET_H__E56F1E2C_F69A_42A0_B593_69BD9F509A0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAlarmOutSet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmOutSet dialog

class CDlgAlarmOutSet : public CDialog
{
// Construction
public:
	CDlgAlarmOutSet(CWnd* pParent = NULL, LLONG lLoginID = 0, int nAlarmOut = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAlarmOutSet)
	enum { IDD = IDD_DLG_ALARMOUTSET };
	CButton	m_chkAction;
	CComboBox	m_cmbChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAlarmOutSet)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAlarmOutSet)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlarmoutsetBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void        InitDlg();

private:
    LLONG       m_lLoginID;
    int         m_nAlarmOut;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGALARMOUTSET_H__E56F1E2C_F69A_42A0_B593_69BD9F509A0C__INCLUDED_)
