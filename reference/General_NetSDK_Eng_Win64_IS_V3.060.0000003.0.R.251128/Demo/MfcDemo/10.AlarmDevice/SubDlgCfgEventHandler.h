#if !defined(AFX_SUBDLGCFGEVENTHANDLER_H__6149AE03_A17B_40A7_8DA4_CAA9FE191524__INCLUDED_)
#define AFX_SUBDLGCFGEVENTHANDLER_H__6149AE03_A17B_40A7_8DA4_CAA9FE191524__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SubDlgCfgEventHandler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgEventHandler dialog

class CSubDlgCfgEventHandler : public CDialog
{
// Construction
public:
	CSubDlgCfgEventHandler(CWnd* pParent = NULL);   // standard constructor
	
	// 在DoModal()之前调用，可以告知窗口初始化时去显示信息
	void SetEventHandler(const CFG_ALARM_MSG_HANDLE& EventHandler)
	{
		m_bInited = TRUE;
		memcpy(&m_stuEventHandler, &EventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
	}

	const CFG_ALARM_MSG_HANDLE& GetEventHandler()
	{
		return m_stuEventHandler;
	}

	void SetTimeSection(const CFG_TIME_SECTION* pstuTimeSection)
	{
		memcpy(&m_stuTimeSection[0][0], pstuTimeSection, sizeof(m_stuTimeSection));
	}

	const CFG_TIME_SECTION* GetTimeSection(int i, int j)
	{
		if ((i >=0 && i < WEEK_DAY_NUM) && (j >= 0 && j < MAX_REC_TSECT))
		{
			return &m_stuTimeSection[i][j];
		} 
		else
		{
			return NULL;
		}
	}

	void SetConfigType(tagEMConfigType emType)
	{
		m_emConfigType = emType;
	}

	BOOL GetTiemChangeFlag() {return m_bTimeChange;}

// Dialog Data
	//{{AFX_DATA(CSubDlgCfgEventHandler)
	enum { IDD = IDD_SUBDLG_EVENTHANDLER };
	CButton	m_chkMMS;
	CButton	m_chkTipEnable;
	CButton	m_chkNeedReport;
	CComboBox	m_cmbCallProtocol;
	CComboBox	m_cmbCaller;
	CButton	m_chkCallEnable;
	CComboBox	m_cmbAccessControlNum;
	CButton	m_chkSeg1;
	CButton	m_chkSeg2;
	CButton	m_chkSeg3;
	CButton	m_chkSeg4;
	CButton	m_chkSeg5;
	CButton	m_chkSeg6;
	CDateTimeCtrl	m_dtp1End;
	CDateTimeCtrl	m_dtp1Start;
	CDateTimeCtrl	m_dtp2End;
	CDateTimeCtrl	m_dtp2Start;
	CDateTimeCtrl	m_dtp3End;
	CDateTimeCtrl	m_dtp3Start;
	CDateTimeCtrl	m_dtp4End;
	CDateTimeCtrl	m_dtp4Start;
	CDateTimeCtrl	m_dtp5End;
	CDateTimeCtrl	m_dtp5Start;
	CDateTimeCtrl	m_dtp6End;
	CDateTimeCtrl	m_dtp6Start;
	CButton	m_chkBeep;
	CButton	m_chkMsg;
	CButton	m_chkVoice;
	CComboBox	m_cmbAccessControlOperation;
	CComboBox	m_cmbWeekDay;
	CEdit	m_edtAlarmOutLatch;
	CEdit	m_edtFilePath;
	CButton	m_chkAlarmOut;
	CButton	m_chkAlarmBell;
	CButton	m_chkAccessControl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSubDlgCfgEventHandler)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSubDlgCfgEventHandler)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeEventhandlerCmbWeekday();
	afx_msg void OnEventhandlerBtnFilepath();
	virtual void OnOK();
	afx_msg void OnSelchangeEventhandlerCmbAco();
	afx_msg void OnEventhandlerChkSeg1Enable();
	afx_msg void OnEventhandlerChkSeg2Enable();
	afx_msg void OnEventhandlerChkSeg3Enable();
	afx_msg void OnEventhandlerChkSeg4Enable();
	afx_msg void OnEventhandlerChkSeg5Enable();
	afx_msg void OnEventhandlerChkSeg6Enable();
	afx_msg void OnDatetimechangeEventhandlerDtp1Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp1End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp2Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp2End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp3Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp3End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp4Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp4End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp5Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp5End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp6Start(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeEventhandlerDtp6End(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeEventhandlerCmbAcNum();
	afx_msg void OnEventhandlerBtnExalarmout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	void ShowTimeSection(int nSeg, const CFG_TIME_SECTION& stuTimeSection);
	void GetTimeSectionFromDlg();
	void CfgTimeSectionToDTP(const CFG_TIME_SECTION& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd);
	void DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, CFG_TIME_SECTION& stuTimeSection);

	void GetAccessControlFromDlg();
	void ShowWindowByType();
	void ShowPowerHandlerInfo();
	void ShowChassisIntrusionInfo();
	void ShowExAlarmInputInfo();
	void ShowAlarmInfo();
	void TimeChange(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, const CFG_TIME_SECTION& stuTimeSection);

private:
	CFG_ALARM_MSG_HANDLE	m_stuEventHandler;
	CFG_TIME_SECTION		m_stuTimeSection[WEEK_DAY_NUM][MAX_REC_TSECT];
	BOOL					m_bInited;
	tagEMConfigType			m_emConfigType;
	BOOL					m_bTimeChange;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBDLGCFGEVENTHANDLER_H__6149AE03_A17B_40A7_8DA4_CAA9FE191524__INCLUDED_)
