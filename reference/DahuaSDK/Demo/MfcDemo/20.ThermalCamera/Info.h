#if !defined(AFX_INFO_H__23B68368_FF5D_4DFC_901F_9F7C587F8344__INCLUDED_)
#define AFX_INFO_H__23B68368_FF5D_4DFC_901F_9F7C587F8344__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Info.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInfo dialog

class CInfo : public CDialog
{
// Construction
public:
	CInfo(CWnd* pParent = NULL, LLONG lLoginId = 0,int nChannel = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInfo)
	enum { IDD = IDD_DLG_INFO };
	CComboBox	m_ComboRegionNum;
	int		m_iBrightness;
	int		m_iSharpness;
	int		m_iEZoom;
	int		m_iThermographyGamma;
	CString	m_Colorization;
	int		m_iSmartOptimizer;
	int		m_iAgc;
	int		m_iAgcMaxGain;
	int		m_iAgcPlateau;
	CString	m_szSerialNum;
	CString	m_szSoftVersion;
	CString	m_szFirmVersion;
	CString	m_szLibraryVersion;
	CString	m_szOptRegion;
	CString	m_szRoiType;
	int		m_Amout;
	int		m_left;
	int		m_top;
	int		m_right;
	int		m_bottom;
	//}}AFX_DATA
    void OnGetPresetInfo();
    void OnGetROIInfo();
    void OnGetExternalSystemInfo();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    LLONG m_lLoginID;
    int m_nChannel;
	// Generated message map functions
	//{{AFX_MSG(CInfo)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboRegionnum();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFO_H__23B68368_FF5D_4DFC_901F_9F7C587F8344__INCLUDED_)
