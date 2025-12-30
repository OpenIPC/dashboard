#if !defined(AFX_DLGDEVICETIME_H__9EBEF58D_C07F_4A71_8941_5C6508145631__INCLUDED_)
#define AFX_DLGDEVICETIME_H__9EBEF58D_C07F_4A71_8941_5C6508145631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDeviceTime.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgDeviceTime dialog

class CDlgDeviceTime : public CDialog
{
// Construction
public:
	CDlgDeviceTime(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgDeviceTime)
	enum { IDD = IDD_DLG_DEVICETIME };
	CComboBox	m_cmbSecond;
	CComboBox	m_cmbMinute;
	CComboBox	m_cmbHour;
	CDateTimeCtrl	m_dtpDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDeviceTime)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgDeviceTime)
	afx_msg void OnDevicetimeBtnGet();
	afx_msg void OnDevicetimeBtnSet();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void StuToDlg();
	void DlgToStu();

	LLONG		m_lLoginId;
	NET_TIME	m_stuTime;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDEVICETIME_H__9EBEF58D_C07F_4A71_8941_5C6508145631__INCLUDED_)
