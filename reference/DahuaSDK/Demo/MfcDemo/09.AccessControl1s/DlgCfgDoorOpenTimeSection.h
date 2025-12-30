#if !defined(_DLGCFGDOOROPENTIMESECTION_)
#define _DLGCFGDOOROPENTIMESECTION_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DlgCfgDoorOpenTimeSection : public CDialog
{
	DECLARE_DYNAMIC(DlgCfgDoorOpenTimeSection)

public:
	DlgCfgDoorOpenTimeSection(CWnd* pParent = NULL);
	virtual ~DlgCfgDoorOpenTimeSection();

	void SetTimeSection(const CFG_DOOROPEN_TIMESECTION_INFO* pstuTimeSection)
	{
		memcpy(&m_stuDoorTimeSection[0][0], pstuTimeSection, sizeof(m_stuDoorTimeSection));
	}

	const CFG_DOOROPEN_TIMESECTION_INFO* GetTimeSection(int i, int j)
	{
		if ((i >=0 && i < WEEK_DAY_NUM) && (j >= 0 && j < MAX_DOOR_TIME_SECTION))
		{
			return &m_stuDoorTimeSection[i][j];
		} 
		else
		{
			return NULL;
		}
	}
	enum { IDD = IDD_DOOR_OPEN_TIMESECTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeDooropentimesectionCmbWeekday();
	afx_msg void OnCbnSelchangeDooropentimesectionCmbOpenmethod1();
	afx_msg void OnCbnSelchangeDooropentimesectionCmbOpenmethod2();
	afx_msg void OnCbnSelchangeDooropentimesectionCmbOpenmethod3();
	afx_msg void OnCbnSelchangeDooropentimesectionCmbOpenmethod4();
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpSt1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpSt2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpSt3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpSt4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpEt1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpEt2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpEt3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDooropentimesectionDtpEt4(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	void ShowTimeSection(int nSeg, const CFG_DOOROPEN_TIMESECTION_INFO& stuTimeSection);
	void GetTimeSectionFromDlg();
	void CfgTimePeriodToDTP(const CFG_TIME_PERIOD& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd);
	void DTPToCfgTimePeriod(const SYSTEMTIME& stuStart, const SYSTEMTIME& stuEnd, CFG_TIME_PERIOD& stuPeriod);
	CComboBox m_cmbWeekDay;
	CDateTimeCtrl m_dtp1Start;
	CDateTimeCtrl m_dtp2Start;
	CDateTimeCtrl m_dtp3Start;
	CDateTimeCtrl m_dtp4Start;
	CDateTimeCtrl m_dtp1End;
	CDateTimeCtrl m_dtp2End;
	CDateTimeCtrl m_dtp3End;
	CDateTimeCtrl m_dtp4End;
	CComboBox m_cmbOpenMethod1;
	CComboBox m_cmbOpenMethod2;
	CComboBox m_cmbOpenMethod3;
	CComboBox m_cmbOpenMethod4;
	CFG_DOOROPEN_TIMESECTION_INFO	m_stuDoorTimeSection[WEEK_DAY_NUM][MAX_DOOR_TIME_SECTION];
};
#endif