#pragma once
#include "afxdtctl.h"

class HolidaySpecialdayInfo : public CDialog
{
	DECLARE_DYNAMIC(HolidaySpecialdayInfo)

public:
	HolidaySpecialdayInfo(CWnd* pParent = NULL);
	virtual ~HolidaySpecialdayInfo();

	void SetSpecialDayInfo(const NET_ACCESSCTL_SPECIALDAY_INFO* pstuSpecialDayInfo)
	{
		memcpy(&m_stuSpecialDayInfo, pstuSpecialDayInfo, sizeof(NET_ACCESSCTL_SPECIALDAY_INFO));
	}

	const NET_ACCESSCTL_SPECIALDAY_INFO* GetSpecialDayInfo()
	{
		return &m_stuSpecialDayInfo;
	}

	enum { IDD = IDD_DIALOG_HOLIDAY_SPECIALDAY_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()

private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	NET_ACCESSCTL_SPECIALDAY_INFO m_stuSpecialDayInfo;
public:
	afx_msg void OnBnClickedOk();
	CDateTimeCtrl m_dtpVDStart;
	CDateTimeCtrl m_dtpVTStart;
	CDateTimeCtrl m_dtpVDEnd;
	CDateTimeCtrl m_dtpVTEnd;
};
