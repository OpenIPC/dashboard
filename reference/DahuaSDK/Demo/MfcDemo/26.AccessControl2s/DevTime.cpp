// DevTime.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "DevTime.h"

IMPLEMENT_DYNAMIC(DevTime, CDialog)

DevTime::DevTime(CWnd* pParent /*=NULL*/)
	: CDialog(DevTime::IDD, pParent)
{
	memset(&m_stuNTPInfo,0,sizeof(m_stuNTPInfo));
	memset(&m_stuDSTInfo,0,sizeof(m_stuDSTInfo));
	m_stuDSTInfo.nStructSize = sizeof(m_stuDSTInfo);
	m_stuDSTInfo.stuDstStart.nStructSize = sizeof(m_stuDSTInfo.stuDstStart);
	m_stuDSTInfo.stuDstEnd.nStructSize = sizeof(m_stuDSTInfo.stuDstEnd);
}

DevTime::~DevTime()
{
}

void DevTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICETIME_DTP_DATE, m_dtpDate);
	DDX_Control(pDX, IDC_DEVICETIME_DTP_TIME, m_dtpTime);
	DDX_Control(pDX, IDC_CFG_NTP_CMB_TIMEZONE, m_cmbTimeZone);
	DDX_Control(pDX, IDC_CFG_NTP_CHK_ENABLE, m_chkEnable);
	DDX_Control(pDX, IDC_DST_CMB_START_YEAR, m_cmbDSTStartYear);
	DDX_Control(pDX, IDC_DST_CMB_START_MONTH, m_cmbDSTStartMonth);
	DDX_Control(pDX, IDC_DST_CMB_START_HOUR, m_cmbDSTStartHour);
	DDX_Control(pDX, IDC_DST_CMB_START_MINUTE, m_cmbDSTStartMinute);
	DDX_Control(pDX, IDC_DST_CMB_START_WEEKORDAY, m_cmbDSTStartWeekOrDay);
	DDX_Control(pDX, IDC_DST_CMB_START_WEEKDAY, m_cmbDSTStartWeekDay);
	DDX_Control(pDX, IDC_DST_CMB_START_DAY, m_cmbDSTStartDay);
	DDX_Control(pDX, IDC_DST_CMB_STOP_YEAR, m_cmbDSTStopYear);
	DDX_Control(pDX, IDC_DST_CMB_STOP_MONTH, m_cmbDSTStopMonth);
	DDX_Control(pDX, IDC_DST_CMB_STOP_HOUR, m_cmbDSTStopHour);
	DDX_Control(pDX, IDC_DST_CMB_STOP_MINUTE, m_cmbDSTStopMinute);
	DDX_Control(pDX, IDC_DST_CMB_STOP_WEEKORDAY, m_cmbDSTStopWeekOrDay);
	DDX_Control(pDX, IDC_DST_CMB_STOP_WEEKDAY, m_cmbDSTStopWeekDay);
	DDX_Control(pDX, IDC_DST_CMB_STOP_DAY, m_cmbDSTStopDay);
	DDX_Control(pDX, IDC_CFG_DST_CMB_TYPE, m_cmbDSTType);
	DDX_Control(pDX, IDC_CFG_DST_CHK_ENABLE, m_chkDSTEnable);
}


BEGIN_MESSAGE_MAP(DevTime, CDialog)
	ON_BN_CLICKED(IDC_DEVICETIME_BTN_SET, &DevTime::OnBnClickedDevicetimeBtnSet)
	ON_BN_CLICKED(IDC_DEVICETIME_BTN_GET, &DevTime::OnBnClickedDevicetimeBtnGet)
	ON_BN_CLICKED(IDC_CFG_NTP_BTN_SET, &DevTime::OnBnClickedCfgNtpBtnSet)
	ON_BN_CLICKED(IDC_CFG_NTP_BTN_GET, &DevTime::OnBnClickedCfgNtpBtnGet)
	ON_BN_CLICKED(IDC_CFG_DST_BTN_SET, &DevTime::OnBnClickedCfgDstBtnSet)
	ON_BN_CLICKED(IDC_CFG_DST_BTN_GET, &DevTime::OnBnClickedCfgDstBtnGet)
	ON_CBN_SELCHANGE(IDC_CFG_DST_CMB_TYPE, &DevTime::OnCbnSelchangeCfgDstCmbType)
END_MESSAGE_MAP()


BOOL DevTime::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedDevicetimeBtnGet();
		OnBnClickedCfgNtpBtnGet();
		OnBnClickedCfgDstBtnGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_DEVICETIME_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEVICETIME_BTN_GET)->EnableWindow(FALSE);

		GetDlgItem(IDC_CFG_NTP_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_CFG_NTP_BTN_GET)->EnableWindow(FALSE);

		GetDlgItem(IDC_CFG_DST_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_CFG_DST_BTN_GET)->EnableWindow(FALSE);
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void DevTime::OnBnClickedDevicetimeBtnSet()
{
	NET_TIME m_stuTime;
	SYSTEMTIME stDate = {0}, stTime = {0};
	m_dtpDate.GetTime(&stDate);
	m_dtpTime.GetTime(&stTime);
	m_stuTime.dwYear = stDate.wYear;
	m_stuTime.dwMonth = stDate.wMonth;
	m_stuTime.dwDay = stDate.wDay;
	m_stuTime.dwHour = stTime.wHour;
	m_stuTime.dwMinute = stTime.wMinute;
	m_stuTime.dwSecond = stTime.wSecond;
	BOOL bret = Device::GetInstance().SetDevTime(&m_stuTime);
	if (!bret)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Set device time failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void DevTime::OnBnClickedDevicetimeBtnGet()
{
	NET_TIME m_stuTime;
	memset(&m_stuTime, 0, sizeof(NET_TIME));
	BOOL bret = Device::GetInstance().GetDevTime(m_stuTime);
	if (bret)
	{
		m_dtpDate.SetTime(COleDateTime(m_stuTime.dwYear, m_stuTime.dwMonth, m_stuTime.dwDay, m_stuTime.dwHour, m_stuTime.dwMinute, m_stuTime.dwSecond));
		m_dtpTime.SetTime(COleDateTime(m_stuTime.dwYear, m_stuTime.dwMonth, m_stuTime.dwDay, m_stuTime.dwHour, m_stuTime.dwMinute, m_stuTime.dwSecond));
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Get device time failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void DevTime::OnBnClickedCfgNtpBtnSet()
{
	NTPDlgToStu();
	BOOL bret = Device::GetInstance().SetNTP(&m_stuNTPInfo);
	if (!bret)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Set NTP failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void DevTime::OnBnClickedCfgNtpBtnGet()
{
	CFG_NTP_INFO	stuNTPInfoTemp;
	memset(&stuNTPInfoTemp,0,sizeof(stuNTPInfoTemp));
	BOOL bret = Device::GetInstance().GetNTP(stuNTPInfoTemp);
	if (bret)
	{
		m_stuNTPInfo = stuNTPInfoTemp;
		NTPStuToDlg();
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Get NTP failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void DevTime::InitDlg()
{
	//NTP
	// enable
	m_chkEnable.SetCheck(BST_UNCHECKED);

	// name
	SetDlgItemText(IDC_CFG_NTP_EDT_NAME, "");

	// port
	SetDlgItemInt(IDC_CFG_NTP_EDT_PORT, 0, FALSE);

	// update period
	SetDlgItemInt(IDC_CFG_NTP_EDT_UPDATEPERIOD, 0, FALSE);

	// time zone
	m_cmbTimeZone.ResetContent();
	for (int i = 0; i < sizeof(DemoTimeZone) / sizeof(DemoTimeZone[0]); i++)
	{
		m_cmbTimeZone.InsertString(-1, DemoTimeZone[i].pszName);
	}
	m_cmbTimeZone.SetCurSel(-1);

	// description
	SetDlgItemText(IDC_CFG_NTP_EDT_DESCRIPTION, "");

	//DST
	m_chkDSTEnable.SetCheck(BST_UNCHECKED);

	m_cmbDSTType.ResetContent();
	m_cmbDSTType.InsertString(-1, ConvertString("By Date"));
	m_cmbDSTType.InsertString(-1, ConvertString("By Week"));
	m_cmbDSTType.SetCurSel(-1);

	m_cmbDSTStartYear.ResetContent();
	m_cmbDSTStopYear.ResetContent();
	for (int i = 0; i < 38; i++)
	{
		CString strtemp;
		strtemp.Format("%d",2000+i);
		m_cmbDSTStartYear.InsertString(-1, strtemp);
		m_cmbDSTStopYear.InsertString(-1, strtemp);
	}
	m_cmbDSTStartYear.SetCurSel(-1);
	m_cmbDSTStopYear.SetCurSel(-1);

	m_cmbDSTStartMonth.ResetContent();
	m_cmbDSTStopMonth.ResetContent();
	for (int i = 0; i < 12; i++)
	{
		CString strtemp;
		strtemp.Format("%d",i+1);
		m_cmbDSTStartMonth.InsertString(-1, strtemp);
		m_cmbDSTStopMonth.InsertString(-1, strtemp);
	}
	m_cmbDSTStartMonth.SetCurSel(-1);
	m_cmbDSTStopMonth.SetCurSel(-1);

	m_cmbDSTStartHour.ResetContent();
	m_cmbDSTStopHour.ResetContent();
	for (int i = 0; i < 24; i++)
	{
		CString strtemp;
		strtemp.Format("%d",i);
		m_cmbDSTStartHour.InsertString(-1, strtemp);
		m_cmbDSTStopHour.InsertString(-1, strtemp);
	}
	m_cmbDSTStartHour.SetCurSel(-1);
	m_cmbDSTStopHour.SetCurSel(-1);

	m_cmbDSTStartMinute.ResetContent();
	m_cmbDSTStopMinute.ResetContent();
	for (int i = 0; i < 60; i++)
	{
		CString strtemp;
		strtemp.Format("%d",i);
		m_cmbDSTStartMinute.InsertString(-1, strtemp);
		m_cmbDSTStopMinute.InsertString(-1, strtemp);
	}
	m_cmbDSTStartMinute.SetCurSel(-1);
	m_cmbDSTStopMinute.SetCurSel(-1);

	m_cmbDSTStartWeekOrDay.ResetContent();
	m_cmbDSTStartWeekOrDay.InsertString(-1, ConvertString("Last Week"));
	//m_cmbDSTStartWeekOrDay.InsertString(-1, ConvertString("By Date"));
	m_cmbDSTStartWeekOrDay.InsertString(-1, ConvertString("First Week"));
	m_cmbDSTStartWeekOrDay.InsertString(-1, ConvertString("Second Week"));
	m_cmbDSTStartWeekOrDay.InsertString(-1, ConvertString("Third Week"));
	m_cmbDSTStartWeekOrDay.InsertString(-1, ConvertString("Fourth Week"));
	m_cmbDSTStartWeekOrDay.SetCurSel(-1);

	m_cmbDSTStopWeekOrDay.ResetContent();
	m_cmbDSTStopWeekOrDay.InsertString(-1, ConvertString("Last Week"));
	//m_cmbDSTStopWeekOrDay.InsertString(-1, ConvertString("By Date"));
	m_cmbDSTStopWeekOrDay.InsertString(-1, ConvertString("First Week"));
	m_cmbDSTStopWeekOrDay.InsertString(-1, ConvertString("Second Week"));
	m_cmbDSTStopWeekOrDay.InsertString(-1, ConvertString("Third Week"));
	m_cmbDSTStopWeekOrDay.InsertString(-1, ConvertString("Fourth Week"));
	m_cmbDSTStopWeekOrDay.SetCurSel(-1);

	m_cmbDSTStartWeekDay.ResetContent();
	m_cmbDSTStopWeekDay.ResetContent();
	for (int i = 0; i < sizeof(stuDemoWeekDay) / sizeof(stuDemoWeekDay[0]); i++)
	{
		m_cmbDSTStartWeekDay.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName));
		m_cmbDSTStopWeekDay.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName));
	}
	m_cmbDSTStartWeekDay.SetCurSel(0);
	m_cmbDSTStopWeekDay.SetCurSel(0);

	m_cmbDSTStartDay.ResetContent();
	m_cmbDSTStopDay.ResetContent();
	for (int i = 0; i < 31; i++)
	{
		CString strtemp;
		strtemp.Format("%d",i+1);
		m_cmbDSTStartDay.InsertString(-1, strtemp);
		m_cmbDSTStopDay.InsertString(-1, strtemp);
	}
	m_cmbDSTStartDay.SetCurSel(-1);
	m_cmbDSTStopDay.SetCurSel(-1);
}

void DevTime::NTPStuToDlg()
{
	m_chkEnable.SetCheck(m_stuNTPInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemText(IDC_CFG_NTP_EDT_NAME, m_stuNTPInfo.szAddress);
	SetDlgItemInt(IDC_CFG_NTP_EDT_PORT, m_stuNTPInfo.nPort, FALSE);
	SetDlgItemInt(IDC_CFG_NTP_EDT_UPDATEPERIOD, m_stuNTPInfo.nUpdatePeriod, FALSE);
	m_cmbTimeZone.SetCurSel((int)m_stuNTPInfo.emTimeZoneType);
	SetDlgItemText(IDC_CFG_NTP_EDT_DESCRIPTION, m_stuNTPInfo.szTimeZoneDesc);
}

void DevTime::NTPDlgToStu()
{
	m_stuNTPInfo.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;
	GetDlgItemText(IDC_CFG_NTP_EDT_NAME, m_stuNTPInfo.szAddress, sizeof(m_stuNTPInfo.szAddress));
	m_stuNTPInfo.nPort = GetDlgItemInt(IDC_CFG_NTP_EDT_PORT, NULL, FALSE);
	m_stuNTPInfo.nUpdatePeriod = GetDlgItemInt(IDC_CFG_NTP_EDT_UPDATEPERIOD, NULL, FALSE);
	m_stuNTPInfo.emTimeZoneType = (EM_CFG_TIME_ZONE_TYPE)m_cmbTimeZone.GetCurSel();
	GetDlgItemText(IDC_CFG_NTP_EDT_DESCRIPTION, m_stuNTPInfo.szTimeZoneDesc, sizeof(m_stuNTPInfo.szTimeZoneDesc));
}

void DevTime::OnBnClickedCfgDstBtnSet()
{
	DSTDlgToStu();
	BOOL bret = Device::GetInstance().SetDST(&m_stuDSTInfo);
	if (!bret)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Set DST failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void DevTime::OnBnClickedCfgDstBtnGet()
{
	AV_CFG_Locales stuInfoTemp = {sizeof(stuInfoTemp)} ;
	BOOL bret = Device::GetInstance().GetDST(stuInfoTemp);
	if (bret)
	{
		m_stuDSTInfo = stuInfoTemp;
		DSTStuToDlg();
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Get DST failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void DevTime::DSTStuToDlg()
{
	m_chkDSTEnable.SetCheck(m_stuDSTInfo.bDSTEnable ? BST_CHECKED : BST_UNCHECKED);

	m_cmbDSTStartWeekOrDay.EnableWindow(TRUE);
	m_cmbDSTStopWeekOrDay.EnableWindow(TRUE);
	m_cmbDSTStartWeekDay.EnableWindow(TRUE);
	m_cmbDSTStartDay.EnableWindow(TRUE);
	m_cmbDSTStopWeekDay.EnableWindow(TRUE);
	m_cmbDSTStopDay.EnableWindow(TRUE);

	if(m_stuDSTInfo.stuDstStart.nWeek == 0)//日期
	{
		m_cmbDSTType.SetCurSel(0);
		m_cmbDSTStartWeekOrDay.SetCurSel(-1);
		m_cmbDSTStartWeekOrDay.EnableWindow(FALSE);
		m_cmbDSTStopWeekOrDay.SetCurSel(-1);
		m_cmbDSTStopWeekOrDay.EnableWindow(FALSE);
	}
	else
	{
		m_cmbDSTType.SetCurSel(1);
	}
	//开始时间
	m_cmbDSTStartYear.SetCurSel(m_stuDSTInfo.stuDstStart.nYear - 2000);
	m_cmbDSTStartMonth.SetCurSel(m_stuDSTInfo.stuDstStart.nMonth - 1);
	m_cmbDSTStartHour.SetCurSel(m_stuDSTInfo.stuDstStart.nHour);
	m_cmbDSTStartMinute.SetCurSel(m_stuDSTInfo.stuDstStart.nMinute);
	if (m_stuDSTInfo.stuDstStart.nWeek == 0)//日期
	{
		m_cmbDSTStartWeekDay.SetCurSel(-1);
		m_cmbDSTStartWeekDay.EnableWindow(FALSE);
		m_cmbDSTStartDay.SetCurSel(m_stuDSTInfo.stuDstStart.nDay-1);
	}
	else
	{
		if (m_stuDSTInfo.stuDstStart.nWeek == -1)
		{
			m_cmbDSTStartWeekOrDay.SetCurSel(0);
		}
		else
		{
			m_cmbDSTStartWeekOrDay.SetCurSel(m_stuDSTInfo.stuDstStart.nWeek);
		}
		m_cmbDSTStartWeekDay.SetCurSel(m_stuDSTInfo.stuDstStart.nDay);
		m_cmbDSTStartDay.SetCurSel(-1);
		m_cmbDSTStartDay.EnableWindow(FALSE);
	}
	//结束时间
	m_cmbDSTStopYear.SetCurSel(m_stuDSTInfo.stuDstEnd.nYear - 2000);
	m_cmbDSTStopMonth.SetCurSel(m_stuDSTInfo.stuDstEnd.nMonth - 1);
	m_cmbDSTStopHour.SetCurSel(m_stuDSTInfo.stuDstEnd.nHour);
	m_cmbDSTStopMinute.SetCurSel(m_stuDSTInfo.stuDstEnd.nMinute);
	if (m_stuDSTInfo.stuDstEnd.nWeek == 0)
	{
		m_cmbDSTStopWeekDay.SetCurSel(-1);
		m_cmbDSTStopWeekDay.EnableWindow(FALSE);
		m_cmbDSTStopDay.SetCurSel(m_stuDSTInfo.stuDstEnd.nDay-1);
	}
	else
	{
		if (m_stuDSTInfo.stuDstEnd.nWeek == -1)
		{
			m_cmbDSTStopWeekOrDay.SetCurSel(0);
		}
		else
		{
			m_cmbDSTStopWeekOrDay.SetCurSel(m_stuDSTInfo.stuDstEnd.nWeek);
		}
		m_cmbDSTStopWeekDay.SetCurSel(m_stuDSTInfo.stuDstEnd.nDay);
		m_cmbDSTStopDay.SetCurSel(-1);
		m_cmbDSTStopDay.EnableWindow(FALSE);
	}
}

void DevTime::DSTDlgToStu()
{
	m_stuDSTInfo.bDSTEnable = m_chkDSTEnable.GetCheck() ? TRUE : FALSE;

	int nDSTType = m_cmbDSTType.GetCurSel();
	if (nDSTType == 0)//日期
	{
		m_stuDSTInfo.stuDstStart.nWeek = 0;
		m_stuDSTInfo.stuDstStart.nDay = m_cmbDSTStartDay.GetCurSel() + 1;
		m_stuDSTInfo.stuDstEnd.nWeek = 0;
		m_stuDSTInfo.stuDstEnd.nDay = m_cmbDSTStopDay.GetCurSel() + 1;
	}
	else
	{
		if (m_cmbDSTStartWeekOrDay.GetCurSel() == 0)
		{
			m_stuDSTInfo.stuDstStart.nWeek = -1;
		}
		else
		{
			m_stuDSTInfo.stuDstStart.nWeek = m_cmbDSTStartWeekOrDay.GetCurSel();
		}
		m_stuDSTInfo.stuDstStart.nDay = m_cmbDSTStartWeekDay.GetCurSel();

		if (m_cmbDSTStopWeekOrDay.GetCurSel() == 0)
		{
			m_stuDSTInfo.stuDstEnd.nWeek = -1;
		}
		else
		{
			m_stuDSTInfo.stuDstEnd.nWeek = m_cmbDSTStopWeekOrDay.GetCurSel();
		}
		m_stuDSTInfo.stuDstEnd.nDay = m_cmbDSTStopWeekDay.GetCurSel();
	}
	m_stuDSTInfo.stuDstStart.nYear = m_cmbDSTStartYear.GetCurSel() + 2000;
	m_stuDSTInfo.stuDstStart.nMonth = m_cmbDSTStartMonth.GetCurSel() + 1;
	m_stuDSTInfo.stuDstStart.nHour = m_cmbDSTStartHour.GetCurSel();
	m_stuDSTInfo.stuDstStart.nMinute = m_cmbDSTStartMinute.GetCurSel();

	m_stuDSTInfo.stuDstEnd.nYear = m_cmbDSTStopYear.GetCurSel() + 2000;
	m_stuDSTInfo.stuDstEnd.nMonth = m_cmbDSTStopMonth.GetCurSel() + 1;
	m_stuDSTInfo.stuDstEnd.nHour = m_cmbDSTStopHour.GetCurSel();
	m_stuDSTInfo.stuDstEnd.nMinute = m_cmbDSTStopMinute.GetCurSel();

}

void DevTime::OnCbnSelchangeCfgDstCmbType()
{
	m_cmbDSTStartWeekOrDay.EnableWindow(TRUE);
	m_cmbDSTStopWeekOrDay.EnableWindow(TRUE);
	m_cmbDSTStartWeekDay.EnableWindow(TRUE);
	m_cmbDSTStartDay.EnableWindow(TRUE);
	m_cmbDSTStopWeekDay.EnableWindow(TRUE);
	m_cmbDSTStopDay.EnableWindow(TRUE);

	int nDSTType = m_cmbDSTType.GetCurSel();
	if (nDSTType == 0)
	{
		m_cmbDSTStartWeekOrDay.SetCurSel(-1);
		m_cmbDSTStartWeekOrDay.EnableWindow(FALSE);
		m_cmbDSTStopWeekOrDay.SetCurSel(-1);
		m_cmbDSTStopWeekOrDay.EnableWindow(FALSE);

		m_cmbDSTStartWeekDay.SetCurSel(-1);
		m_cmbDSTStartWeekDay.EnableWindow(FALSE);
		m_cmbDSTStopWeekDay.SetCurSel(-1);
		m_cmbDSTStopWeekDay.EnableWindow(FALSE);

		m_cmbDSTStartDay.SetCurSel(0);
		m_cmbDSTStopDay.SetCurSel(0);
	}
	else
	{
		m_cmbDSTStartDay.SetCurSel(-1);
		m_cmbDSTStartDay.EnableWindow(FALSE);
		m_cmbDSTStopDay.SetCurSel(-1);
		m_cmbDSTStopDay.EnableWindow(FALSE);

		m_cmbDSTStartWeekOrDay.SetCurSel(0);
		m_cmbDSTStartWeekDay.SetCurSel(0);
		m_cmbDSTStopWeekOrDay.SetCurSel(0);
		m_cmbDSTStopWeekDay.SetCurSel(0);
	}
}
