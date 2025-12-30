#if !defined(AFX_DLGSTATEBATTERY_H__6B8E951C_308C_44D0_981D_9D6547DD4AB7__INCLUDED_)
#define AFX_DLGSTATEBATTERY_H__6B8E951C_308C_44D0_981D_9D6547DD4AB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgStateBattery.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgStateBattery dialog

class CDlgStateBattery : public CDialog
{
// Construction
public:
	CDlgStateBattery(CWnd* pParent = NULL, LLONG hLoginID = NULL);   // standard constructor
	BOOL GetPowerState();

	void InitPower(INT nNum, DH_POWER_STATUS *pstuInpara);
	void InitBattery(INT nNum, DH_POWER_STATUS *pstuInpara);


// Dialog Data
	//{{AFX_DATA(CDlgStateBattery)
	enum { IDD = IDD_DLG_STATE_BATTERY };
	CButton	m_chkIsCharging;
	CProgressCtrl	m_prgPercent;
	CComboBox	m_cbPowerIndex;
	CComboBox	m_cbBatteryIndex;
	CComboBox	m_cbState;
	CString	m_csPercent;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgStateBattery)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgStateBattery)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnGet();
	afx_msg void OnSelchangeCmbBatteryIndex();
	afx_msg void OnSelchangeCmbPowerIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	LLONG	m_hLoginID;
	BOOL	m_bPowerState[16];			//电源状态,开/关
	BOOL	m_bBatteryState[16];		//电池状态,充电/未充电
	INT		m_nBatteryPercent[16];		//电池电量百分比
	INT		m_nPowerNum;
	INT		m_nBatteryNum;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSTATEBATTERY_H__6B8E951C_308C_44D0_981D_9D6547DD4AB7__INCLUDED_)
