#if !defined(AFX_CONTROL_H__9C30E7C7_5599_4DB1_9483_C26360C6E63A__INCLUDED_)
#define AFX_CONTROL_H__9C30E7C7_5599_4DB1_9483_C26360C6E63A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Control.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CControl dialog

class CControl : public CDialog
{
// Construction
public:
	CControl(CWnd* pParent = NULL,LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CControl)
	enum { IDD = IDD_DLG_CONTROL };
	CComboBox	m_ComboMeterType;
	int		m_Channel;
	int		m_PresetID;
	CString	m_PresetName;
	int		m_RuleID;
	//}}AFX_DATA
    void InitDlg();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CControl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_lLoginId;
	// Generated message map functions
	//{{AFX_MSG(CControl)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONTROL_H__9C30E7C7_5599_4DB1_9483_C26360C6E63A__INCLUDED_)
