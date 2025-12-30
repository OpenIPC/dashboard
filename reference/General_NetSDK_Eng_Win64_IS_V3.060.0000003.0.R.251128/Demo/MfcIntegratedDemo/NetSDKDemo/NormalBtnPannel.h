#if !defined(AFX_NORMALBTNPANNEL_H__381161BA_6396_4E81_A3C1_58489E1E6B7B__INCLUDED_)
#define AFX_NORMALBTNPANNEL_H__381161BA_6396_4E81_A3C1_58489E1E6B7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NormalBtnPannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNormalBtnPannel dialog

class CNormalBtnPannel : public CDialog
{
// Construction
public:
	CNormalBtnPannel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNormalBtnPannel)
	enum { IDD = IDD_PANNEL_NORMAL_BTN };
	CComboBox	m_subtypesel;
	CComboBox	m_splitsel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNormalBtnPannel)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNormalBtnPannel)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSplittesel();
	afx_msg void OnFullscreen();
	afx_msg void OnAddDevice();
	afx_msg void OnDeleteDevice();
	afx_msg void OnOpenChannel();
	afx_msg void OnOpenMultiplay();
	afx_msg void OnCloseScreen();
	afx_msg void OnCloseAllchannel();
	afx_msg void OnSearchRecord();
	afx_msg void OnCycleMonitor();
	afx_msg void OnPlaybackbytime();
	afx_msg void OnDownloadbytime();
	afx_msg void OnSysconfig();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int GetSplit();
	void SetSplit(int nSplit);
	void InitSplit(int curSplit);
	void EnableMultiplay(BOOL bEnable);
	int GetSubType();

private:
	CBrush m_myBrush;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NORMALBTNPANNEL_H__381161BA_6396_4E81_A3C1_58489E1E6B7B__INCLUDED_)
