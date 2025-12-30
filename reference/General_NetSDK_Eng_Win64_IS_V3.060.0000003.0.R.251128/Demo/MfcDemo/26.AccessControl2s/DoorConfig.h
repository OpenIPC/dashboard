#include "afxwin.h"
#if !defined(_DOORCONFIG_)
#define _DOORCONFIG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DoorConfig : public CDialog
{
	DECLARE_DYNAMIC(DoorConfig)

public:
	DoorConfig(CWnd* pParent = NULL);
	virtual ~DoorConfig();
	enum { IDD = IDD_DIALOG_DOORCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDoorcfgBtnGet();
	afx_msg void OnBnClickedDoorcfgBtnSet();
	afx_msg void OnBnClickedDoorcfgBtnTimesection();
	DECLARE_MESSAGE_MAP()
private:
	void SetDoorCfgInfoToCtrl();
	void GetDoorCfgInfoFromCtrl();
	void InitDlg();
	CButton		m_chkRemoteCheck;
	CButton		m_chkFirstEnterEnable;
	CComboBox	m_cbFirstEnterStatus;
	CComboBox	m_cmbChannel;
	CComboBox	m_cmbOpenTimeIndex;
	CComboBox	m_cmbOpenMethod;
	CButton		m_chkSensor;
	CButton		m_chkRepeatEnterAlarm;
	CButton		m_chkDuressAlarm;
	CButton		m_chkDoorNotCloseAlarm;
	CButton		m_chkBreakAlarm;
	CFG_ACCESS_EVENT_INFO	m_stuInfo;
public:
	CComboBox m_cmbAccessState;
	afx_msg void OnBnClickedButTimescheduleconfig();
	afx_msg void OnBnClickedButHolidaymanage();
	CButton m_chkAutoRemoteTimeEnable;
	CButton m_chkMaliciousAlarm;
	CButton m_chkCustomPassword;
	afx_msg void OnCbnSelchangeDoorcfgCmbChannel();
};
#endif