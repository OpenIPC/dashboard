#if !defined(AFX_DLGCFGPSTNBREAKLINE_H__B74D2C3C_525F_467A_9F49_9D910CA54F79__INCLUDED_)
#define AFX_DLGCFGPSTNBREAKLINE_H__B74D2C3C_525F_467A_9F49_9D910CA54F79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgPSTNBreakLine.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNBreakLine dialog

class CDlgCfgPSTNBreakLine : public CDialog
{
// Construction
public:
	CDlgCfgPSTNBreakLine(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCfgPSTNBreakLine)
	enum { IDD = IDD_DLG_CFG_PSTNBREAKLINE };
	CButton	m_ckEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgPSTNBreakLine)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCfgPSTNBreakLine)
	virtual BOOL OnInitDialog();
	afx_msg void OnCfgPSTNBreakLineBtnHandler();
	afx_msg void OnCfgPSTNBreakLineBtnGet();
	afx_msg void OnCfgPSTNBreakLineBtnSet();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
        
private:
    BOOL getInfo();
    BOOL showInfo();
    BOOL SetConfigToDevice();
    BOOL GetConfigFromDevice();
    
private:
    LLONG               m_lLoginId;
	CFG_PSTN_BREAK_LINE_INFO m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGPSTNBREAKLINE_H__B74D2C3C_525F_467A_9F49_9D910CA54F79__INCLUDED_)
