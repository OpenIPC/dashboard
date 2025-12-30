#if !defined(AFX_SELECTPANNEL_H__5E459E8D_250A_4AD5_8BED_9AF2F851FD89__INCLUDED_)
#define AFX_SELECTPANNEL_H__5E459E8D_250A_4AD5_8BED_9AF2F851FD89__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectPannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectPannel dialog

class CSelectPannel : public CDialog
{
// Construction
public:
	CSelectPannel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelectPannel)
	enum { IDD = IDD_PANNEL_SELECT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectPannel)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectPannel)
	afx_msg void OnBtnSelDevlist();
	afx_msg void OnBtnSelPtz();
	afx_msg void OnBtnSelColor();
	afx_msg void OnBtnSelPlayctrl();
	afx_msg void OnBtnSelNormal();
	afx_msg void OnBtnSelAdvance();
	afx_msg void OnBtnSelData();
	virtual BOOL OnInitDialog();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPANNEL_H__5E459E8D_250A_4AD5_8BED_9AF2F851FD89__INCLUDED_)
