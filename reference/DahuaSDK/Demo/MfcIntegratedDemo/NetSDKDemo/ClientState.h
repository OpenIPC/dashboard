#if !defined(AFX_LIENTSTATE_H__B0FCA87F_78FD_4AD5_B164_0D819AA0AED1__INCLUDED_)
#define AFX_LIENTSTATE_H__B0FCA87F_78FD_4AD5_B164_0D819AA0AED1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// lientState.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CClientState dialog

class CClientState : public CDialog
{
// Construction
public:
	CClientState(CWnd* pParent = NULL);   // standard constructor
	virtual ~CClientState();

// Dialog Data
	//{{AFX_DATA(CClientState)
	enum { IDD = IDD_CLIENT_STATE };
	BOOL	m_isNoMoreShow ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientState)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CClientState)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SetDevice(DeviceNode *dev);
	DeviceNode *GetDevice();

	void UpdateState();   //¸üÐÂÏÔÊ¾×´Ì¬
	void StartRefresh();
	void StopRefresh();
private:
	DEV_STATE	m_myState;
	DeviceNode			*m_dev;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LIENTSTATE_H__B0FCA87F_78FD_4AD5_B164_0D819AA0AED1__INCLUDED_)
