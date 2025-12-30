#if !defined(AFX_DLGBYPASSCTL_H__D1BEFC92_C015_4E6F_9D4B_974E690CE786__INCLUDED_)
#define AFX_DLGBYPASSCTL_H__D1BEFC92_C015_4E6F_9D4B_974E690CE786__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgBypassCtl.h : header file
//

typedef enum __BypassModeType
{
	BypassMode_Unknown = 0,
	BypassMode_Bypass,
	BypassMode_Normal,
	BypassMode_Isolated,
};

typedef struct tagBypassMode 
{
	__BypassModeType	emMode;
	const char*			szMode;
}BYPASS_MODE_INFO;

const BYPASS_MODE_INFO stuBypassModeInfo[] = 
{
	{BypassMode_Unknown, "Unknown"},
	{BypassMode_Bypass, "Bypass"},
	{BypassMode_Normal, "Normal"},
	{BypassMode_Isolated, "Isolated"},
};


/////////////////////////////////////////////////////////////////////////////
// CDlgBypassCtl dialog

class CDlgBypassCtl : public CDialog
{
// Construction
public:
	CDlgBypassCtl(CWnd* pParent = NULL, LLONG lLoginId = NULL, unsigned int uiChannelNum = 0);   // standard constructor


public:
	void	InitDlg();

// Dialog Data
	//{{AFX_DATA(CDlgBypassCtl)
	enum { IDD = IDD_DLG_BYPASSCTL };
	CComboBox	m_cmbExtChannel;
	CComboBox	m_cmbBypassMode;
	CComboBox	m_cmbChannel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgBypassCtl)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgBypassCtl)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBypassctlBtnGet();
	afx_msg void OnBypassctlBtnSet();
	afx_msg void OnBypassctlRadioChannel();
	afx_msg void OnBypassctlRadioExtchannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG			m_lLoginId;
	unsigned int	m_uiChannelNum;
	int				m_nExtChannelNum;

private:
	enum emComBoxID
	{
		emComBoxID_Base = 0xff,
	};

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGBYPASSCTL_H__D1BEFC92_C015_4E6F_9D4B_974E690CE786__INCLUDED_)
