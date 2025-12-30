#if !defined(AFX_RUNTIMEMSGPANNEL_H__D90B6E33_421E_4587_840F_206BD502A046__INCLUDED_)
#define AFX_RUNTIMEMSGPANNEL_H__D90B6E33_421E_4587_840F_206BD502A046__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RuntimeMsgPannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRuntimeMsgPannel dialog

class CRuntimeMsgPannel : public CDialog
{
// Construction
public:
	CRuntimeMsgPannel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRuntimeMsgPannel)
	enum { IDD = IDD_PANNEL_RUNTIME_MSG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRuntimeMsgPannel)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRuntimeMsgPannel)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void ShowLastError();
	void EnableShowFlux(BOOL bShow);
private:
	int m_nCurScreen;
	LARGE_INTEGER		m_sysFreq;
	LARGE_INTEGER		m_sysCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RUNTIMEMSGPANNEL_H__D90B6E33_421E_4587_840F_206BD502A046__INCLUDED_)
