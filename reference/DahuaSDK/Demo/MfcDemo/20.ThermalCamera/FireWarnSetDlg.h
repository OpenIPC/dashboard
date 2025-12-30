#pragma once
#include "afxwin.h"
#include "afxdtctl.h"
#include "afxcmn.h"


// FireWarnSetDlg 对话框

class FireWarnSetDlg : public CDialog
{
	DECLARE_DYNAMIC(FireWarnSetDlg)

public:
	FireWarnSetDlg(CWnd* pParent = NULL, LLONG lLoginId = 0, int nChannelCount = 0);   // 标准构造函数
	virtual ~FireWarnSetDlg();

// 对话框数据
	enum { IDD = IDD_DIG_FIREWARNSET };

protected:
	LLONG m_lLoginID;
	int m_nChannelCount;
	bool bFirstinit;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonGetfireset();
	afx_msg void OnBnClickedButtonSetfireset();
	afx_msg void OnBnClickedButtonGetfireext();
	afx_msg void OnBnClickedButtonSetfireext();
	afx_msg void OnBnClickedButtonGetfiremodeset();
	afx_msg void OnBnClickedButtonSetfiremodeset();
	afx_msg void OnBnClickedRadioAuto();
	afx_msg void OnNMDblclkListRegions(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedRadioManual();
	afx_msg void OnCbnSelchangeComboPtzdata();
	afx_msg void OnCbnSelchangeComboRegion();
	afx_msg void OnCbnSelchangeComboWeek();
	afx_msg void OnCbnSelchangeComboTimeslot();

	CButton m_check_rch1;
	CButton m_check_rch2;
	CButton m_check_sch1;
	CButton m_check_sch2;
	CComboBox m_combo_week;
	CComboBox m_combo_timslot;
	CButton m_check_visualoverview;
	CButton m_check_visualdetail;
	CString m_strThermoSnapTimes;
	
	// 预置点
	CComboBox m_combo_ptzdata;
	// 启用
	CButton m_check_enable;
	CString m_strFireDuration;
	CComboBox m_combo_region;

	CString m_strSensitivity;

	CDateTimeCtrl m_datatimestart;
	CDateTimeCtrl m_datatimeend;
	CButton m_check_record;
	CString m_strRecordLatch;
	CButton m_check_alarmout;
	CButton m_check_mail;
	CButton m_check_ptz;
	CButton m_check_snap;
	CString m_stralarmoutlatch;
	CListCtrl m_list_regions;
	CEdit m_edit;
	
	int m_row;
	int m_column;
	
	NET_FIRE_WARNING_INFO m_stuFirewarningInfo_Get;
	NET_EM_FIREWARNING_MODE_TYPE m_FirewarinigModeType;
	CButton m_check_time;
	
	void GetTimefromStu();

	CString m_strhours;
	CString m_strminutes;
	CString m_strseconds;
	CString m_strhoure;
	CString m_strminutee;
	CString m_strseconde;
	afx_msg void OnEnChangeEdithours();
	afx_msg void OnEnChangeEditminutes();
	afx_msg void OnEnChangeEditseconds();
	afx_msg void OnEnChangeEdithoure();
	afx_msg void OnEnChangeEditminutee();
	afx_msg void OnEnChangeEditseconde();
};
