#if !defined(AFX_DLGCAPABILITY_H__732FEF0C_8170_4BA1_8F6F_E65F65DAB3FB__INCLUDED_)
#define AFX_DLGCAPABILITY_H__732FEF0C_8170_4BA1_8F6F_E65F65DAB3FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCapability.h : header file
//

const AV_int32 g_nNetInterfaceNum = 16;

/////////////////////////////////////////////////////////////////////////////
// CDlgCapability dialog

class CDlgCapability : public CDialog
{
// Construction
public:
	CDlgCapability(CWnd* pParent = NULL, LLONG hLoginID = NULL, AV_uint32 uiAlarmIn = 0, AV_uint32 uiAlarmOut = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCapability)
	enum { IDD = IDD_DLG_CAPABILITY };
	CListBox	m_lsErrorInfo;
	CEdit	m_edtShow;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCapability)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCapability)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void	GetCapability();
	void	OutputInfo(const CString& csInfo);

	BOOL	GetAlarmKeyboardOnlineCount(CString& csCap);
	BOOL	GetFileManagerCap(CString& csCap);
	BOOL	GetLogServiceCap(CString& csCap);
	BOOL	GetAlarmInCap(CString& csCap);
	BOOL	GetEventHandlerCap(CString& csCap);
	BOOL	GetExAlarmBoxCap(CString& csCap);
	BOOL	GetExAlarmInCap(CString& csCap);
	BOOL	GetAccessControlCap(CString& csCap);
	BOOL	GetAnalogAlarmCap(CString& csCap);
    BOOL    GetLowRateWPanCap(CString& csCap);

private:
	LLONG		m_hLogin;
	UINT32		m_uiAlarmIn;
	UINT32		m_uiAlarmOut;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCAPABILITY_H__732FEF0C_8170_4BA1_8F6F_E65F65DAB3FB__INCLUDED_)
