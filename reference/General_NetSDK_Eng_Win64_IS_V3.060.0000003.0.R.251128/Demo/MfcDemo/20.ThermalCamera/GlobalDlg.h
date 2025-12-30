#if !defined(AFX_GLOBALDLG_H__1D7AA55D_4053_4318_B4D9_EC50635DF133__INCLUDED_)
#define AFX_GLOBALDLG_H__1D7AA55D_4053_4318_B4D9_EC50635DF133__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GlobalDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGlobalDlg dialog

class CGlobalDlg : public CDialog
{
// Construction
public:
	CGlobalDlg(CWnd* pParent = NULL,LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGlobalDlg)
	enum { IDD = IDD_DLG_GLOBAL };
	CComboBox	m_Combo_Unit;
	CComboBox	m_Combo_Switch;
	CComboBox	m_Combo_HotSpotFollow;
	CComboBox	m_Combo_ColorBar;
	CComboBox	m_Combo_ColorBatch;
	int		m_nHumidity;
	float	m_fAtmosphericTemp;
	float	m_fCoefficient;
	int		m_nDistance;
	float	m_fReflectedTemp;
	int		m_nSatTemp;
	int		m_nMaxTemp;
	int		m_nMidTemp;
	int		m_nMinTemp;
	int		m_nBottom;
	int		m_nLeft;
	int		m_nRight;
	int		m_nTop;
	int		m_nHighAlpha;
	int		m_nHighBlue;
	int		m_nHighGreen;
	int		m_nHighRed;
	int		m_nLowAlpha;
	int		m_nLowBlue;
	int		m_nLowGreen;
	int		m_nLowRed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGlobalDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_iLoginID; 
	// Generated message map functions
	//{{AFX_MSG(CGlobalDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnSet();
	afx_msg void OnBtnGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GLOBALDLG_H__1D7AA55D_4053_4318_B4D9_EC50635DF133__INCLUDED_)
