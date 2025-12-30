#include "afxwin.h"
#if !defined(_TIMESCHEDULE_)
#define _TIMESCHEDULE_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TimeSchedule : public CDialog
{
	DECLARE_DYNAMIC(TimeSchedule)

public:
	TimeSchedule(CWnd* pParent = NULL);
	virtual ~TimeSchedule();
	virtual BOOL OnInitDialog();
	enum { IDD = IDD_DIALOG_TIMESCHEDULE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnBnClickedTimescheduleBtnSet();
	afx_msg void OnBnClickedTimescheduleBtnGet();
	afx_msg void OnBnClickedTimecfgBtnSet();
	afx_msg void OnBnClickedTimecfgBtnGet();
	afx_msg void OnBnClickedButHolidayExecute();
	afx_msg void OnBnClickedInfoHolidayButtonDoors();
	afx_msg void OnCbnSelchangeComHolidayExecutetype();
	afx_msg void OnCbnSelchangeTimescheduleCmbIndex();
	afx_msg void OnCbnSelchangeTimescheduleCmbWeekday();
	DECLARE_MESSAGE_MAP()
private:
	void CfgTimeSectionToDTP(const CFG_TIME_SECTION& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd);
	void DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, CFG_TIME_SECTION& stuTimeSection);
	void GetTimeSectionInfoFromCtrl();
	void SetTimeSectionInfoToCtrl();
	void ShowTimeSection(int nSeg, const CFG_TIME_SECTION& stuTimeSection);
	void HolidayInsert();
	void HolidayGet();
	void HolidayUpdate();
	void HolidayRemove();
	void HolidayClear();
	void GetHolidayInfoFromCtrl();
	void SetHolidayInfoToCtrl();
	
	CDateTimeCtrl	m_dtp1Start;
	CDateTimeCtrl	m_dtp2Start;
	CDateTimeCtrl	m_dtp3Start;
	CDateTimeCtrl	m_dtp4Start;
	CDateTimeCtrl	m_dtp1End;
	CDateTimeCtrl	m_dtp2End;
	CDateTimeCtrl	m_dtp3End;
	CDateTimeCtrl	m_dtp4End;
	CButton			m_chkSeg1;
	CButton			m_chkSeg2;
	CButton			m_chkSeg3;
	CButton			m_chkSeg4;
	CComboBox		m_cmbWeekDay;
	CComboBox		m_cmbIndex;
	CComboBox		m_cmbChannel;
	CButton			m_chkAutoRemote;
	CComboBox		m_cmbCtlType;
	CDateTimeCtrl	m_HolidayStartDate;
	CDateTimeCtrl	m_HolidayEndDate;
	CButton			m_chcHolidayEnable;

	CFG_ACCESS_TIMESCHEDULE_INFO	m_stuInfo;
	CFG_ACCESS_EVENT_INFO			m_stuAccessEvent;
	NET_RECORDSET_HOLIDAY	        m_stuRecordHoliday;
	std::vector<int>				vecDoor;
public:
	CButton m_chkOpenAlways;
	CButton m_chkCloseAlways;
	afx_msg void OnBnClickedCheckOpenalways();
	afx_msg void OnBnClickedCheckClosealways();
	afx_msg void OnBnClickedButHolidayGet();
	void OnChangeUIState(int nState);
};
#endif