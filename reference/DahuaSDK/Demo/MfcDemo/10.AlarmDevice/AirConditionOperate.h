#if !defined(AFX_AIRCONDITIONOPERATE_H__5914E7DD_7505_4CA7_B069_E8ADBF733BF3__INCLUDED_)
#define AFX_AIRCONDITIONOPERATE_H__5914E7DD_7505_4CA7_B069_E8ADBF733BF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AirConditionOperate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAirConditionOperate dialog

class CAirConditionOperate : public CDialog
{
// Construction
public:
	CAirConditionOperate(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAirConditionOperate)
	enum { IDD = IDD_DIALOG_AIRCONDITION_OPERATE };
    CComboBox	m_cmbDevID;
    CButton	    m_ctlON;
    CButton	    m_ctlONLine;
    UINT        m_nTempState;
    float	    m_fActualTemp;
    CComboBox	m_cmbModeState;
    CComboBox	m_cmbWindModeState;

    UINT        m_nTemp;
    int         m_nAdjustTemp;
    CComboBox	m_cmbMode;
    CComboBox	m_cmbWindMode;
	UINT		m_nWaitTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAirConditionOperate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAirConditionOperate)
	virtual BOOL OnInitDialog();
	afx_msg void OnAirconditionBtnOpen();
	afx_msg void OnAirconditionBtnClose();
	afx_msg void OnAirconditionBtnMode();
	afx_msg void OnAirconditionBtnTemp();
	afx_msg void OnAirconditionBtnWindmode();
	afx_msg void OnCfgAirconditionBtnGet();
	afx_msg void OnAirconditionBtnAdjust();
	afx_msg void OnAirconditionBtnGetState();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
        
private:
    void		InitDlg();
    BOOL		GetConfig();
    BOOL		GetState();
    void		StuToDlg();

private:
    LLONG						m_lLoginID;
	CFG_AIRCONDITION_INFO	    m_stuInfo;
    NET_GET_AIRCONDITION_STATE	m_stuState;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AIRCONDITIONOPERATE_H__5914E7DD_7505_4CA7_B069_E8ADBF733BF3__INCLUDED_)
