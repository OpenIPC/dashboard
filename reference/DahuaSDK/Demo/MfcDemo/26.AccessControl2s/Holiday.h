#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxdtctl.h"


// Holiday 对话框

class Holiday : public CDialog
{
	DECLARE_DYNAMIC(Holiday)

public:
	Holiday(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~Holiday();

// 对话框数据
	enum { IDD = IDD_DIALOG_HOLIDAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void	GroupStuToDlg();
	void	GroupDlgToStu();
	void	SetListCtl();

	void	ScheduleStuToDlg();
	void	ScheduleDlgToStu();

public:
	CListCtrl m_listSpecialdayInfo;
	CComboBox m_cmbCtlID;
	CButton m_chkGroupEnable;
	afx_msg void OnBnClickedSpecialdayInfoBtnAdd();
	afx_msg void OnBnClickedSpecialdayInfoBtnModify();
	afx_msg void OnBnClickedSpecialdayInfoBtnDelete();
	afx_msg void OnBnClickedSpecialdaygroupBtnGet();
	afx_msg void OnBnClickedSpecialdaygroupBtnSet();
	afx_msg void OnNMClickListctrlSpecialdayInfo(NMHDR *pNMHDR, LRESULT *pResult);
	NET_CFG_ACCESSCTL_SPECIALDAY_GROUP_INFO stuSpecialdayGroup;
	int m_nSpeciadayIndex;
	

	CComboBox m_cmbCtlScheduleID;
	CButton m_chkSpeciadayEnable;
	CDateTimeCtrl m_dtp1Start;
	CDateTimeCtrl m_dtp2Start;
	CDateTimeCtrl m_dtp3Start;
	CDateTimeCtrl m_dtp4Start;
	CDateTimeCtrl m_dtp1End;
	CDateTimeCtrl m_dtp2End;
	CDateTimeCtrl m_dtp3End;
	CDateTimeCtrl m_dtp4End;
	afx_msg void OnBnClickedSpecialdayscheduleBtnGet();
	afx_msg void OnBnClickedSpecialdayscheduleBtnSet();
	afx_msg void OnBnClickedSpecialdayscheduleBtnDoors();
	NET_CFG_ACCESSCTL_SPECIALDAYS_SCHEDULE_INFO stuSpecialdaySchedule;
	void GetTimeSection();
	void SetTimeSection();
	void CfgTimeSectionToDTP(const DH_TSECT& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd);
	void DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, DH_TSECT& stuTimeSection);

	afx_msg void OnLvnItemchangedListctrlSpecialdayInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeSpecialdaygroupCmbNo();
	afx_msg void OnCbnSelchangeSpecialdayscheduleCmbNo();
};
