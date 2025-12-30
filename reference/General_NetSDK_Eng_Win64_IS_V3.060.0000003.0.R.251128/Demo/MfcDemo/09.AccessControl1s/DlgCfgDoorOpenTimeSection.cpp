// DlgCfgDoorOpenTimeSection.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "DlgCfgDoorOpenTimeSection.h"

IMPLEMENT_DYNAMIC(DlgCfgDoorOpenTimeSection, CDialog)

DlgCfgDoorOpenTimeSection::DlgCfgDoorOpenTimeSection(CWnd* pParent /*=NULL*/)
	: CDialog(DlgCfgDoorOpenTimeSection::IDD, pParent)
{
	memset(&m_stuDoorTimeSection, 0, sizeof(m_stuDoorTimeSection));
}

DlgCfgDoorOpenTimeSection::~DlgCfgDoorOpenTimeSection()
{
}

void DlgCfgDoorOpenTimeSection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_CMB_WEEKDAY, m_cmbWeekDay);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ST1, m_dtp1Start);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ST2, m_dtp2Start);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ST3, m_dtp3Start);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ST4, m_dtp4Start);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ET1, m_dtp1End);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ET2, m_dtp2End);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ET3, m_dtp3End);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_DTP_ET4, m_dtp4End);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD1, m_cmbOpenMethod1);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD2, m_cmbOpenMethod2);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD3, m_cmbOpenMethod3);
	DDX_Control(pDX, IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD4, m_cmbOpenMethod4);
}


BEGIN_MESSAGE_MAP(DlgCfgDoorOpenTimeSection, CDialog)

	ON_CBN_SELCHANGE(IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD1, &DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod1)
	ON_CBN_SELCHANGE(IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD2, &DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod2)
	ON_CBN_SELCHANGE(IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD3, &DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod3)
	ON_CBN_SELCHANGE(IDC_DOOROPENTIMESECTION_CMB_OPENMETHOD4, &DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod4)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ST1, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt1)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ST2, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt2)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ST3, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt3)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ST4, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt4)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ET1, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt1)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ET2, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt2)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ET3, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt3)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DOOROPENTIMESECTION_DTP_ET4, &DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt4)
	ON_CBN_SELCHANGE(IDC_DOOROPENTIMESECTION_CMB_WEEKDAY, &DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbWeekday)
END_MESSAGE_MAP()


BOOL DlgCfgDoorOpenTimeSection::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	StuToDlg();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbWeekday()
{
	int nSel = m_cmbWeekDay.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}

	for (int i = 0; i < MAX_DOOR_TIME_SECTION; i++)
	{
		ShowTimeSection(i, m_stuDoorTimeSection[nSel][i]);
	}
}

void DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod1()
{
	DlgToStu();
}

void DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod2()
{
	DlgToStu();
}

void DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod3()
{
	DlgToStu();
}

void DlgCfgDoorOpenTimeSection::OnCbnSelchangeDooropentimesectionCmbOpenmethod4()
{
	DlgToStu();
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpSt4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::OnDtnDatetimechangeDooropentimesectionDtpEt4(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	DlgToStu();
	*pResult = 0;
}

void DlgCfgDoorOpenTimeSection::InitDlg()
{
	unsigned int i = 0;

	// week day
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

	// door open method
	m_cmbOpenMethod1.ResetContent();
	m_cmbOpenMethod2.ResetContent();
	m_cmbOpenMethod3.ResetContent();
	m_cmbOpenMethod4.ResetContent();
	for (i = 0; i < sizeof(stuDemoOpenMethod)/sizeof(stuDemoOpenMethod[0]); i++)
	{
		m_cmbOpenMethod1.InsertString(-1, ConvertString(stuDemoOpenMethod[i].szName));
		m_cmbOpenMethod2.InsertString(-1, ConvertString(stuDemoOpenMethod[i].szName));
		m_cmbOpenMethod3.InsertString(-1, ConvertString(stuDemoOpenMethod[i].szName));
		m_cmbOpenMethod4.InsertString(-1, ConvertString(stuDemoOpenMethod[i].szName));
	}
	m_cmbOpenMethod1.SetDroppedWidth(160);
	m_cmbOpenMethod2.SetDroppedWidth(160);
	m_cmbOpenMethod3.SetDroppedWidth(160);
	m_cmbOpenMethod4.SetDroppedWidth(160);
	m_cmbOpenMethod1.SetCurSel(-1);
	m_cmbOpenMethod2.SetCurSel(-1);
	m_cmbOpenMethod3.SetCurSel(-1);
	m_cmbOpenMethod4.SetCurSel(-1);	
}

void DlgCfgDoorOpenTimeSection::StuToDlg()
{
	OnCbnSelchangeDooropentimesectionCmbWeekday();
}

void DlgCfgDoorOpenTimeSection::DlgToStu()
{
	GetTimeSectionFromDlg();
}

void DlgCfgDoorOpenTimeSection::GetTimeSectionFromDlg()
{
	int nSel = m_cmbWeekDay.GetCurSel();
	SYSTEMTIME stBegin = {0}, stEnd = {0};
	for (int i = 0; i < MAX_DOOR_TIME_SECTION; i++)
	{
		CFG_DOOROPEN_TIMESECTION_INFO& stuTimeSection = m_stuDoorTimeSection[(em_WeekDay)nSel][i];
		switch (i)
		{
		case 0:
			stuTimeSection.emDoorOpenMethod = (CFG_DOOR_OPEN_METHOD)m_cmbOpenMethod1.GetCurSel();
			m_dtp1Start.GetTime(&stBegin);
			m_dtp1End.GetTime(&stEnd);
			break;
		case 1:
			stuTimeSection.emDoorOpenMethod = (CFG_DOOR_OPEN_METHOD)m_cmbOpenMethod2.GetCurSel();
			m_dtp2Start.GetTime(&stBegin);
			m_dtp2End.GetTime(&stEnd);
			break;
		case 2:
			stuTimeSection.emDoorOpenMethod = (CFG_DOOR_OPEN_METHOD)m_cmbOpenMethod3.GetCurSel();
			m_dtp3Start.GetTime(&stBegin);
			m_dtp3End.GetTime(&stEnd);
			break;
		case 3:
			stuTimeSection.emDoorOpenMethod = (CFG_DOOR_OPEN_METHOD)m_cmbOpenMethod4.GetCurSel();
			m_dtp4Start.GetTime(&stBegin);
			m_dtp4End.GetTime(&stEnd);
			break;
		}
		DTPToCfgTimePeriod(stBegin, stEnd, stuTimeSection.stuTime);
	}
}

void DlgCfgDoorOpenTimeSection::CfgTimePeriodToDTP(const CFG_TIME_PERIOD& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd)
{
	stBegin.wHour	= (WORD)stuTimeSection.stuStartTime.dwHour;
	stBegin.wMinute = (WORD)stuTimeSection.stuStartTime.dwMinute;
	stBegin.wSecond = (WORD)stuTimeSection.stuStartTime.dwSecond;
	stEnd.wHour		= (WORD)stuTimeSection.stuEndTime.dwHour;
	stEnd.wMinute	= (WORD)stuTimeSection.stuEndTime.dwMinute;
	stEnd.wSecond	= (WORD)stuTimeSection.stuEndTime.dwSecond;
}

void DlgCfgDoorOpenTimeSection::DTPToCfgTimePeriod(const SYSTEMTIME& stuStart, const SYSTEMTIME& stuEnd, CFG_TIME_PERIOD& stuPeriod)
{
	stuPeriod.stuStartTime.dwHour = stuStart.wHour;
	stuPeriod.stuStartTime.dwMinute = stuStart.wMinute;
	stuPeriod.stuStartTime.dwSecond = stuStart.wSecond;
	stuPeriod.stuEndTime.dwHour = stuEnd.wHour;
	stuPeriod.stuEndTime.dwMinute = stuEnd.wMinute;
	stuPeriod.stuEndTime.dwSecond = stuEnd.wSecond;
}

void DlgCfgDoorOpenTimeSection::ShowTimeSection(int nSeg, const CFG_DOOROPEN_TIMESECTION_INFO& stuTimeSection)
{
	SYSTEMTIME stBegin = {2000, 1, 1, 1}, stEnd = {2000, 1, 1, 1};
	CfgTimePeriodToDTP(stuTimeSection.stuTime, stBegin, stEnd);

	switch (nSeg)
	{
	case 0:
		m_dtp1Start.SetTime(&stBegin);
		m_dtp1End.SetTime(&stEnd);
		m_cmbOpenMethod1.SetCurSel((int)stuTimeSection.emDoorOpenMethod);
		break;
	case 1:
		m_dtp2Start.SetTime(&stBegin);
		m_dtp2End.SetTime(&stEnd);
		m_cmbOpenMethod2.SetCurSel((int)stuTimeSection.emDoorOpenMethod);
		break;
	case 2:
		m_dtp3Start.SetTime(&stBegin);
		m_dtp3End.SetTime(&stEnd);
		m_cmbOpenMethod3.SetCurSel((int)stuTimeSection.emDoorOpenMethod);
		break;
	case 3:
		m_dtp4Start.SetTime(&stBegin);
		m_dtp4End.SetTime(&stEnd);
		m_cmbOpenMethod4.SetCurSel((int)stuTimeSection.emDoorOpenMethod);
		break;
	}
}

