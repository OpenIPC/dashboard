// TimeSchedule.cpp
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "TimeSchedule.h"
#include "DlgSensorInfoDescription.h"

IMPLEMENT_DYNAMIC(TimeSchedule, CDialog)

TimeSchedule::TimeSchedule(CWnd* pParent /*=NULL*/)
	: CDialog(TimeSchedule::IDD, pParent)
{
	memset(&m_stuInfo, 0 , sizeof(m_stuInfo));
}

TimeSchedule::~TimeSchedule()
{
}

void TimeSchedule::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP1_START, m_dtp1Start);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP2_START, m_dtp2Start);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP3_START, m_dtp3Start);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP4_START, m_dtp4Start);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP1_END, m_dtp1End);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP2_END, m_dtp2End);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP3_END, m_dtp3End);
	DDX_Control(pDX, IDC_TIMESCHEDULE_DTP4_END, m_dtp4End);
	DDX_Control(pDX, IDC_TIMESCHEDULE_CHK_SEG1_ENABLE, m_chkSeg1);
	DDX_Control(pDX, IDC_TIMESCHEDULE_CHK_SEG2_ENABLE, m_chkSeg2);
	DDX_Control(pDX, IDC_TIMESCHEDULE_CHK_SEG3_ENABLE, m_chkSeg3);
	DDX_Control(pDX, IDC_TIMESCHEDULE_CHK_SEG4_ENABLE, m_chkSeg4);
	DDX_Control(pDX, IDC_TIMESCHEDULE_CMB_WEEKDAY, m_cmbWeekDay);
	DDX_Control(pDX, IDC_TIMESCHEDULE_CMB_INDEX, m_cmbIndex);

	DDX_Control(pDX, IDC_TIMESCHEDULE_CHK_ENABLE, m_timesenable);
}

BEGIN_MESSAGE_MAP(TimeSchedule, CDialog)

	ON_BN_CLICKED(IDC_TIMESCHEDULE_BTN_SET, &TimeSchedule::OnBnClickedTimescheduleBtnSet)
	ON_BN_CLICKED(IDC_TIMESCHEDULE_BTN_GET, &TimeSchedule::OnBnClickedTimescheduleBtnGet)
	ON_CBN_SELCHANGE(IDC_TIMESCHEDULE_CMB_INDEX, &TimeSchedule::OnCbnSelchangeTimescheduleCmbIndex)
	ON_CBN_SELCHANGE(IDC_TIMESCHEDULE_CMB_WEEKDAY, &TimeSchedule::OnCbnSelchangeTimescheduleCmbWeekday)
END_MESSAGE_MAP()


BOOL TimeSchedule::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	unsigned int i = 0;

	// Index
	m_cmbIndex.ResetContent();
	for (i = 0; i < 128; i++)
	{
		CString csInfo;
		csInfo.Format("%d", i + 1);
		m_cmbIndex.InsertString(-1, csInfo);
	}
	m_cmbIndex.SetCurSel(0);

	// WeekDay
	m_cmbWeekDay.ResetContent();
	for (i = 0; i < sizeof(stuDemoWeekDay) / sizeof(stuDemoWeekDay[0]); i++)
	{
		m_cmbWeekDay.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName));
	}
	m_cmbWeekDay.SetCurSel(0);

	// DateTimePickers...
	SYSTEMTIME stStart = {2000,1,1,1}, stEnd = {2000,1,1,1,23,59,59,0};
	m_dtp1Start.SetTime(&stStart);
	m_dtp1End.SetTime(&stEnd);
	m_dtp2Start.SetTime(&stStart);
	m_dtp2End.SetTime(&stEnd);
	m_dtp3Start.SetTime(&stStart);
	m_dtp3End.SetTime(&stEnd);
	m_dtp4Start.SetTime(&stStart);
	m_dtp4End.SetTime(&stEnd);
	m_chkSeg1.SetCheck(BST_UNCHECKED);
	m_chkSeg2.SetCheck(BST_UNCHECKED);
	m_chkSeg3.SetCheck(BST_UNCHECKED);
	m_chkSeg4.SetCheck(BST_UNCHECKED);
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_TIMESCHEDULE_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIMESCHEDULE_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIMECFG_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIMECFG_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->EnableWindow(FALSE);
	}
	else
	{
		OnBnClickedTimescheduleBtnGet();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void TimeSchedule::OnBnClickedTimescheduleBtnSet()
{
	GetTimeSectionInfoFromCtrl();
	BOOL bret = Device::GetInstance().SetTimeSchedule(m_cmbIndex.GetCurSel(), &m_stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set TimeSchedule failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void TimeSchedule::OnBnClickedTimescheduleBtnGet()
{
	BOOL bret = Device::GetInstance().GetTimeSchedule(m_cmbIndex.GetCurSel(), m_stuInfo);
	if (bret)
	{
		SetTimeSectionInfoToCtrl();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get TimeSchedule failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void TimeSchedule::GetTimeSectionInfoFromCtrl()
{
	m_stuInfo.bEnable = m_timesenable.GetCheck();
	GetDlgItemText(IDC_EDIT_TIMESCHEDULENAME, m_stuInfo.szName, sizeof(m_stuInfo.szName));
	int nSel = m_cmbWeekDay.GetCurSel();
	SYSTEMTIME stBegin = {0}, stEnd = {0};
	for (int i = 0; i < MAX_DOOR_TIME_SECTION; i++)
	{
		CFG_TIME_SECTION& stuTimeSection = m_stuInfo.stuTime[(em_WeekDay)nSel][i];
		switch (i)
		{
		case 0:
			stuTimeSection.dwRecordMask = m_chkSeg1.GetCheck();
			m_dtp1Start.GetTime(&stBegin);
			m_dtp1End.GetTime(&stEnd);
			break;
		case 1:
			stuTimeSection.dwRecordMask = m_chkSeg2.GetCheck();
			m_dtp2Start.GetTime(&stBegin);
			m_dtp2End.GetTime(&stEnd);
			break;
		case 2:
			stuTimeSection.dwRecordMask = m_chkSeg3.GetCheck();
			m_dtp3Start.GetTime(&stBegin);
			m_dtp3End.GetTime(&stEnd);
			break;
		case 3:
			stuTimeSection.dwRecordMask = m_chkSeg4.GetCheck();
			m_dtp4Start.GetTime(&stBegin);
			m_dtp4End.GetTime(&stEnd);
			break;
		}
		DTPToCfgTimeSection(stBegin, stEnd, stuTimeSection);
	}
}

void TimeSchedule::SetTimeSectionInfoToCtrl()
{
	if (m_stuInfo.bEnable)
	{
		m_timesenable.SetCheck(BST_CHECKED);
	}
	else
	{
		m_timesenable.SetCheck(BST_UNCHECKED);
	}
	SetDlgItemText(IDC_EDIT_TIMESCHEDULENAME, m_stuInfo.szName);
	int nSel = m_cmbWeekDay.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}

	for (int i = 0; i < MAX_DOOR_TIME_SECTION; i++)
	{
		ShowTimeSection(i, m_stuInfo.stuTime[nSel][i]);
	}
}

void TimeSchedule::ShowTimeSection(int nSeg, const CFG_TIME_SECTION& stuTimeSection)
{
	SYSTEMTIME stBegin = {2000, 1, 1, 1}, stEnd = {2000, 1, 1, 1};
	CfgTimeSectionToDTP(stuTimeSection, stBegin, stEnd);

	switch (nSeg)
	{
	case 0:
		m_dtp1Start.SetTime(&stBegin);
		m_dtp1End.SetTime(&stEnd);
		m_chkSeg1.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 1:
		m_dtp2Start.SetTime(&stBegin);
		m_dtp2End.SetTime(&stEnd);
		m_chkSeg2.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 2:
		m_dtp3Start.SetTime(&stBegin);
		m_dtp3End.SetTime(&stEnd);
		m_chkSeg3.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 3:
		m_dtp4Start.SetTime(&stBegin);
		m_dtp4End.SetTime(&stEnd);
		m_chkSeg4.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	}
}

void TimeSchedule::CfgTimeSectionToDTP(const CFG_TIME_SECTION& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd)
{
	stBegin.wHour = stuTimeSection.nBeginHour;
	stBegin.wMinute = stuTimeSection.nBeginMin;
	stBegin.wSecond = stuTimeSection.nBeginSec;
	stEnd.wHour = stuTimeSection.nEndHour;
	stEnd.wMinute = stuTimeSection.nEndMin;
	stEnd.wSecond = stuTimeSection.nEndSec;
}

void TimeSchedule::DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, CFG_TIME_SECTION& stuTimeSection)
{
	stuTimeSection.nBeginHour = stBegin.wHour;
	stuTimeSection.nBeginMin = stBegin.wMinute;
	stuTimeSection.nBeginSec = stBegin.wSecond;
	stuTimeSection.nEndHour = stEnd.wHour;
	stuTimeSection.nEndMin = stEnd.wMinute;
	stuTimeSection.nEndSec = stEnd.wSecond;
}

void TimeSchedule::OnCbnSelchangeTimescheduleCmbIndex()
{
	OnBnClickedTimescheduleBtnGet();
}

void TimeSchedule::OnCbnSelchangeTimescheduleCmbWeekday()
{
	int nSel = m_cmbWeekDay.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}

	for (int i = 0; i < MAX_DOOR_TIME_SECTION; i++)
	{
		ShowTimeSection(i, m_stuInfo.stuTime[nSel][i]);
	}
}

