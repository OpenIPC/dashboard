#if !defined(AFX_DLGSETNETWORKPARAM_H__34936527_C541_4294_89D0_1E2176E21B16__INCLUDED_)
#define AFX_DLGSETNETWORKPARAM_H__34936527_C541_4294_89D0_1E2176E21B16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSetNetWorkParam.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgSetNetWorkParam dialog

class CDlgSetNetWorkParam : public CDialog
{
// Construction
public:
	CDlgSetNetWorkParam(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgSetNetWorkParam)
	enum { IDD = IDD_DLG_SETNETWORKPARAM };
	CComboBox	m_cmbNetType;
	int		m_edConnectBufSize;
	int		m_edConnectTime;
	int		m_edConnectTryNum;
	int		m_edGetDevInfoTime;
	int		m_edPicBufSize;
	int		m_edPlaybackBufSize;
	int		m_edSearchRecordTime;
	int		m_edSubConnectSpaceTime;
	int		m_edSubDisconnectTime;
	int		m_edWaittime;
	int		m_edGetConnInfoTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgSetNetWorkParam)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgSetNetWorkParam)
	afx_msg void OnSetnetworkparamBtnSet();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void DlgToStu();
	void Init();
private:
	LLONG		m_lLoginId;
	NET_PARAM	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSETNETWORKPARAM_H__34936527_C541_4294_89D0_1E2176E21B16__INCLUDED_)
