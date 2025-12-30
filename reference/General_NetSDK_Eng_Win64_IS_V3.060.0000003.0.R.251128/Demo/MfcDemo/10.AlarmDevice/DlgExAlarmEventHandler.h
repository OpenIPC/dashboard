#if !defined(AFX_DLGEXALARMEVENTHANDLER_H__AB73C2C0_A2EE_48FB_B4BE_8264365B92C2__INCLUDED_)
#define AFX_DLGEXALARMEVENTHANDLER_H__AB73C2C0_A2EE_48FB_B4BE_8264365B92C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgExAlarmEventHandler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmEventHandler dialog

class CDlgExAlarmEventHandler : public CDialog
{
// Construction
public:
	CDlgExAlarmEventHandler(CWnd* pParent = NULL);   // standard constructor

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
// Dialog Data
	//{{AFX_DATA(CDlgExAlarmEventHandler)
	enum { IDD = IDD_DIG_EXALARM_EVENTHANDLER };
	CButton	m_ckExAlarmNeedReport;
	CButton	m_ckExAlarmBeepEnable;
	CButton	m_ckExAlarmBellEnable;
	CButton	m_ckExAlarmOutEnable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgExAlarmEventHandler)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgExAlarmEventHandler)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();

	BOOL					m_bInited;
	CFG_ALARM_MSG_HANDLE	m_stuEventHandler;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEXALARMEVENTHANDLER_H__AB73C2C0_A2EE_48FB_B4BE_8264365B92C2__INCLUDED_)
