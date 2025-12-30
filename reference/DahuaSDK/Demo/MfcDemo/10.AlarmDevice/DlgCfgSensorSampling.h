#if !defined(AFX_DLGCFGSENSORSAMPLING_H__2D88F54D_4967_4788_A314_9E27566A73CD__INCLUDED_)
#define AFX_DLGCFGSENSORSAMPLING_H__2D88F54D_4967_4788_A314_9E27566A73CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgSensorSampling.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorSampling dialog

class CDlgCfgSensorSampling : public CDialog
{
// Construction
public:
	CDlgCfgSensorSampling(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgSensorSampling)
	enum { IDD = IDD_DLG_CFG_SENSORSAMPLING };
	int		m_edtDetectionPeriod;
	int		m_edtStorageitem;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgSensorSampling)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgSensorSampling)
	virtual BOOL OnInitDialog();
	afx_msg void OnSensorsamplingBtnGet();
	afx_msg void OnSensorsamplingBtnSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void DlgToStu();
	void StuToDlg();
	BOOL getCfgFromDevice();
	BOOL setCfgToDevice(); 
	
	LLONG					m_lLoginId;
	CFG_SENSORSAMPLING_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGSENSORSAMPLING_H__2D88F54D_4967_4788_A314_9E27566A73CD__INCLUDED_)
