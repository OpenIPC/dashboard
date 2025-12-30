#include "afxwin.h"
#if !defined(_DEVTIME_)
#define _DEVTIME_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct
{
	EM_CFG_TIME_ZONE_TYPE	emTimeZone;
	char*					pszName;
}Demo_TimeZone;

const Demo_TimeZone DemoTimeZone[] = 
{
	{EM_CFG_TIME_ZONE_0, "GMT+00:00"},
	{EM_CFG_TIME_ZONE_1, "GMT+01:00"},
	{EM_CFG_TIME_ZONE_2, "GMT+02:00"},
	{EM_CFG_TIME_ZONE_3, "GMT+03:00"},
	{EM_CFG_TIME_ZONE_4, "GMT+03:30"},
	{EM_CFG_TIME_ZONE_5, "GMT+04:00"},
	{EM_CFG_TIME_ZONE_6, "GMT+04:30"},
	{EM_CFG_TIME_ZONE_7, "GMT+05:00"},
	{EM_CFG_TIME_ZONE_8, "GMT+05:30"},
	{EM_CFG_TIME_ZONE_9, "GMT+05:45"},
	{EM_CFG_TIME_ZONE_10, "GMT+06:00"},
	{EM_CFG_TIME_ZONE_11, "GMT+06:30"},
	{EM_CFG_TIME_ZONE_12, "GMT+07:00"},
	{EM_CFG_TIME_ZONE_13, "GMT+08:00"},
	{EM_CFG_TIME_ZONE_14, "GMT+09:00"},
	{EM_CFG_TIME_ZONE_15, "GMT+09:30"},
	{EM_CFG_TIME_ZONE_16, "GMT+10:00"},
	{EM_CFG_TIME_ZONE_17, "GMT+11:00"},
	{EM_CFG_TIME_ZONE_18, "GMT+12:00"},
	{EM_CFG_TIME_ZONE_19, "GMT+13:00"},
	{EM_CFG_TIME_ZONE_20, "GMT-01:00"},
	{EM_CFG_TIME_ZONE_21, "GMT-02:00"},
	{EM_CFG_TIME_ZONE_22, "GMT-03:00"},
	{EM_CFG_TIME_ZONE_23, "GMT-03:30"},
	{EM_CFG_TIME_ZONE_24, "GMT-04:00"},
	{EM_CFG_TIME_ZONE_25, "GMT-05:00"},
	{EM_CFG_TIME_ZONE_26, "GMT-06:00"},
	{EM_CFG_TIME_ZONE_27, "GMT-07:00"},
	{EM_CFG_TIME_ZONE_28, "GMT-08:00"},
	{EM_CFG_TIME_ZONE_29, "GMT-09:00"},
	{EM_CFG_TIME_ZONE_30, "GMT-10:00"},
	{EM_CFG_TIME_ZONE_31, "GMT-11:00"},
	{EM_CFG_TIME_ZONE_32, "GMT-12:00"},
	{EM_CFG_TIME_ZONE_33, "GMT-04:30"},
	{EM_CFG_TIME_ZONE_34, "GMT+10:30"},
	{EM_CFG_TIME_ZONE_35, "GMT+14:00"},
	{EM_CFG_TIME_ZONE_36, "GMT-09:30"},
	{EM_CFG_TIME_ZONE_37, "GMT+08:30"},
	{EM_CFG_TIME_ZONE_38, "GMT+08:45"},
	{EM_CFG_TIME_ZONE_39, "GMT+12:45"},
};

class DevTime : public CDialog
{
	DECLARE_DYNAMIC(DevTime)

public:
	DevTime(CWnd* pParent = NULL);
	virtual ~DevTime();
	enum { IDD = IDD_DIALOG_DEVTIME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDevicetimeBtnSet();
	afx_msg void OnBnClickedDevicetimeBtnGet();
	afx_msg void OnBnClickedCfgNtpBtnSet();
	afx_msg void OnBnClickedCfgNtpBtnGet();
	afx_msg void OnBnClickedCfgDstBtnSet();
	afx_msg void OnBnClickedCfgDstBtnGet();
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void NTPStuToDlg();
	void NTPDlgToStu();
	void DSTStuToDlg();
	void DSTDlgToStu();
	CComboBox m_cmbDSTStartYear;
	CComboBox m_cmbDSTStartMonth;
	CComboBox m_cmbDSTStartHour;
	CComboBox m_cmbDSTStartMinute;
	CComboBox m_cmbDSTStartWeekOrDay;
	CComboBox m_cmbDSTStartWeekDay;
	CComboBox m_cmbDSTStartDay;
	CComboBox m_cmbDSTStopYear;
	CComboBox m_cmbDSTStopMonth;
	CComboBox m_cmbDSTStopHour;
	CComboBox m_cmbDSTStopMinute;
	CComboBox m_cmbDSTStopWeekOrDay;
	CComboBox m_cmbDSTStopWeekDay;
	CComboBox m_cmbDSTStopDay;
	CComboBox m_cmbDSTType;
	CDateTimeCtrl	m_dtpDate;
	CDateTimeCtrl	m_dtpTime;
	CComboBox		m_cmbTimeZone;
	CButton			m_chkEnable;
	CFG_NTP_INFO	m_stuNTPInfo;
	//DHDEV_DST_CFG	m_stuDSTInfo;
	AV_CFG_Locales	m_stuDSTInfo;
	CButton			m_chkDSTEnable;
public:
	afx_msg void OnCbnSelchangeCfgDstCmbType();
	
};
#endif
/*
typedef struct  
{
	DWORD               dwSize;
	int                 nDSTType;                   // 夏令时定位方式 0:按日期定位方式, 1:按周定位方式
	DH_DST_POINT        stDSTStart;                 // 开始夏令时
	DH_DST_POINT        stDSTEnd;                   // 结束夏令时
	DWORD               dwReserved[16];             // 保留    
}DHDEV_DST_CFG;

struct AV_CFG_Locales 
{
	AV_int32			nStructSize;
	char				szTimeFormat[AV_CFG_Time_Format_Len];	// 时间格式
	AV_BOOL				bDSTEnable;						// 夏令时时能
	AV_CFG_DSTTime		stuDstStart;					// 夏令时起始时间
	AV_CFG_DSTTime		stuDstEnd;						// 夏令时结束时间
	AV_BOOL             bWeekEnable;                    // 星期是否使能
	unsigned char       ucWorkDay;                      // 按位表示一周的工作日掩码，bit0表示周日，bit1表示周一
};*/