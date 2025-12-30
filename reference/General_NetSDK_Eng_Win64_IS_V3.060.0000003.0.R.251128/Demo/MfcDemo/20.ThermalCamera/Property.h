#if !defined(AFX_PROPERTY_H__FAD78CEF_F673_4911_8FB4_2C34D9F535F3__INCLUDED_)
#define AFX_PROPERTY_H__FAD78CEF_F673_4911_8FB4_2C34D9F535F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Property.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProperty dialog

class CProperty : public CDialog
{
// Construction
public:
	CProperty(CWnd* pParent = NULL,LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProperty)
	enum { IDD = IDD_DLG_PROPERTY };
	CComboBox	m_Combo_GainMode;
	CComboBox	m_Combo_Num;
	CComboBox	m_Combo_RoiType;
	CComboBox	m_Combo_OptRegion;
	CComboBox	m_Combo_Pseudocolor;
	int		m_nZoom;
	int		m_nGamma;
	int		m_nOptIndicator;
	int		m_nAmount;
	int		m_nAgc;
	int		m_nAgcMaxGain;
	int		m_nAgcPlateau;
	int		m_nHighValue;
	int		m_nLHRoi;
	int		m_nHLRoi;
	int		m_nLowValue;
	int		m_nBottom;
	int		m_nLeft;
	int		m_nRight;
	int		m_nTop;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProperty)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    LLONG  m_iLoginID;
	// Generated message map functions
	//{{AFX_MSG(CProperty)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnGet();
	afx_msg void OnSelchangeComboNum();
	afx_msg void OnBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTY_H__FAD78CEF_F673_4911_8FB4_2C34D9F535F3__INCLUDED_)
