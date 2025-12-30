#if !defined(AFX_DLGCFGSENSORINFOOFVW_H__C0D27281_818B_42A9_AF67_3A9D12DDBAFE__INCLUDED_)
#define AFX_DLGCFGSENSORINFOOFVW_H__C0D27281_818B_42A9_AF67_3A9D12DDBAFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgSensorInfoOfVW.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorInfoOfVW dialog

class CDlgCfgSensorInfoOfVW : public CDialog
{
// Construction
public:
	CDlgCfgSensorInfoOfVW(CWnd* pParent = NULL, LLONG lLoginID = 0, int nChannel = 4);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgSensorInfoOfVW)
	enum { IDD = IDD_DLG_CFG_VIDEOWIDGET_SENSOINFO };
	CButton	m_chkPreviewBlend;
	CButton	m_chkEncodeBlend;
	CComboBox	m_cmbChn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgSensorInfoOfVW)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgSensorInfoOfVW)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSensorinfoCmbChn();
	afx_msg void OnSensorinfoBtnDescription();
	afx_msg void OnSensorinfoBtnGet();
	afx_msg void OnSensorinfoBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void		InitDlg();
	void		DlgToStu();
	void		StuToDlg();
	BOOL		GetConfig();
	BOOL		SetConfig();

private:
	LLONG				m_lLoginID;
	int					m_nChannel;
	AV_CFG_VideoWidget	m_stuVW;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGSENSORINFOOFVW_H__C0D27281_818B_42A9_AF67_3A9D12DDBAFE__INCLUDED_)
