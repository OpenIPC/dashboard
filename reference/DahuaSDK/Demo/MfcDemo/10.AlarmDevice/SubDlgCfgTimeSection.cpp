// SubDlgCfgTimeSection.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "SubDlgCfgTimeSection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgTimeSection dialog


CSubDlgCfgTimeSection::CSubDlgCfgTimeSection(CWnd* pParent /*=NULL*/)
	: CDialog(CSubDlgCfgTimeSection::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSubDlgCfgTimeSection)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	memset(&m_stuTimeSection, 0, sizeof(m_stuTimeSection));
}


void CSubDlgCfgTimeSection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSubDlgCfgTimeSection)
	DDX_Control(pDX, IDC_TIMESECTION_CMB_WEEKDAY, m_cmbWeekDay);
	DDX_Control(pDX, IDC_TIMESECTION_CHK_SEG1_ENABLE, m_chkSeg1);
	DDX_Control(pDX, IDC_TIMESECTION_CHK_SEG2_ENABLE, m_chkSeg2);
	DDX_Control(pDX, IDC_TIMESECTION_CHK_SEG3_ENABLE, m_chkSeg3);
	DDX_Control(pDX, IDC_TIMESECTION_CHK_SEG4_ENABLE, m_chkSeg4);
	DDX_Control(pDX, IDC_TIMESECTION_CHK_SEG5_ENABLE, m_chkSeg5);
	DDX_Control(pDX, IDC_TIMESECTION_CHK_SEG6_ENABLE, m_chkSeg6);
	DDX_Control(pDX, IDC_TIMESECTION_DTP1_END, m_dtp1End);
	DDX_Control(pDX, IDC_TIMESECTION_DTP1_START, m_dtp1Start);
	DDX_Control(pDX, IDC_TIMESECTION_DTP2_END, m_dtp2End);
	DDX_Control(pDX, IDC_TIMESECTION_DTP2_START, m_dtp2Start);
	DDX_Control(pDX, IDC_TIMESECTION_DTP3_END, m_dtp3End);
	DDX_Control(pDX, IDC_TIMESECTION_DTP3_START, m_dtp3Start);
	DDX_Control(pDX, IDC_TIMESECTION_DTP4_END, m_dtp4End);
	DDX_Control(pDX, IDC_TIMESECTION_DTP4_START, m_dtp4Start);
	DDX_Control(pDX, IDC_TIMESECTION_DTP5_END, m_dtp5End);
	DDX_Control(pDX, IDC_TIMESECTION_DTP5_START, m_dtp5Start);
	DDX_Control(pDX, IDC_TIMESECTION_DTP6_END, m_dtp6End);
	DDX_Control(pDX, IDC_TIMESECTION_DTP6_START, m_dtp6Start);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSubDlgCfgTimeSection, CDialog)
	//{{AFX_MSG_MAP(CSubDlgCfgTimeSection)
	ON_CBN_SELCHANGE(IDC_TIMESECTION_CMB_WEEKDAY, OnSelchangeTimesectionCmbWeekday)
	ON_BN_CLICKED(IDC_TIMESECTION_CHK_SEG1_ENABLE, OnEventhandlerChkSeg1Enable)
	ON_BN_CLICKED(IDC_TIMESECTION_CHK_SEG2_ENABLE, OnEventhandlerChkSeg2Enable)
	ON_BN_CLICKED(IDC_TIMESECTION_CHK_SEG3_ENABLE, OnEventhandlerChkSeg3Enable)
	ON_BN_CLICKED(IDC_TIMESECTION_CHK_SEG4_ENABLE, OnEventhandlerChkSeg4Enable)
	ON_BN_CLICKED(IDC_TIMESECTION_CHK_SEG5_ENABLE, OnEventhandlerChkSeg5Enable)
	ON_BN_CLICKED(IDC_TIMESECTION_CHK_SEG6_ENABLE, OnEventhandlerChkSeg6Enable)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP1_START, OnDatetimechangeEventhandlerDtp1Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP1_END, OnDatetimechangeEventhandlerDtp1End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP2_START, OnDatetimechangeEventhandlerDtp2Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP2_END, OnDatetimechangeEventhandlerDtp2End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP3_START, OnDatetimechangeEventhandlerDtp3Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP3_END, OnDatetimechangeEventhandlerDtp3End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP4_START, OnDatetimechangeEventhandlerDtp4Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP4_END, OnDatetimechangeEventhandlerDtp4End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP5_START, OnDatetimechangeEventhandlerDtp5Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP5_END, OnDatetimechangeEventhandlerDtp5End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP6_START, OnDatetimechangeEventhandlerDtp6Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TIMESECTION_DTP6_END, OnDatetimechangeEventhandlerDtp6End)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgTimeSection private method

void CSubDlgCfgTimeSection::InitDlg()
{
	unsigned int i = 0;
	
	// WeekDay
	m_cmbWeekDay.ResetContent();
	for (i = 0; i < sizeof(stuDemoWeekDay) / sizeof(stuDemoWeekDay[0]); i++)
	{
		m_cmbWeekDay.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName, DLG_EVENTHANDLER));
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
	m_dtp5Start.SetTime(&stStart);
	m_dtp5End.SetTime(&stEnd);
	m_dtp6Start.SetTime(&stStart);
	m_dtp6End.SetTime(&stEnd);
	m_chkSeg1.SetCheck(BST_UNCHECKED);
	m_chkSeg2.SetCheck(BST_UNCHECKED);
	m_chkSeg3.SetCheck(BST_UNCHECKED);
	m_chkSeg4.SetCheck(BST_UNCHECKED);
	m_chkSeg5.SetCheck(BST_UNCHECKED);
	m_chkSeg6.SetCheck(BST_UNCHECKED);
}

void CSubDlgCfgTimeSection::StuToDlg()
{
	OnSelchangeTimesectionCmbWeekday();
}

void CSubDlgCfgTimeSection::DlgToStu()
{
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::ShowTimeSection(int nSeg, const CFG_TIME_SECTION& stuTimeSection)
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
	case 4:
		m_dtp5Start.SetTime(&stBegin);
		m_dtp5End.SetTime(&stEnd);
		m_chkSeg5.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 5:
		m_dtp6Start.SetTime(&stBegin);
		m_dtp6End.SetTime(&stEnd);
		m_chkSeg6.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	}
}

void CSubDlgCfgTimeSection::GetTimeSectionFromDlg()
{
	int nSel = m_cmbWeekDay.GetCurSel();
	SYSTEMTIME stBegin = {0}, stEnd = {0};
	for (int i = 0; i < MAX_REC_TSECT; i++)
	{
		CFG_TIME_SECTION& stuTimeSection = m_stuTimeSection[(em_WeekDay)nSel][i];
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
		case 4:
			stuTimeSection.dwRecordMask = m_chkSeg5.GetCheck();
			m_dtp5Start.GetTime(&stBegin);
			m_dtp5End.GetTime(&stEnd);
			break;
		case 5:
			stuTimeSection.dwRecordMask = m_chkSeg6.GetCheck();
			m_dtp6Start.GetTime(&stBegin);
			m_dtp6End.GetTime(&stEnd);
			break;
		}
		DTPToCfgTimeSection(stBegin, stEnd, stuTimeSection);
	}
}

void CSubDlgCfgTimeSection::CfgTimeSectionToDTP(const CFG_TIME_SECTION& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd)
{
	stBegin.wHour = stuTimeSection.nBeginHour;
	stBegin.wMinute = stuTimeSection.nBeginMin;
	stBegin.wSecond = stuTimeSection.nBeginSec;
	stEnd.wHour = stuTimeSection.nEndHour;
	stEnd.wMinute = stuTimeSection.nEndMin;
	stEnd.wSecond = stuTimeSection.nEndSec;
}

void CSubDlgCfgTimeSection::DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, CFG_TIME_SECTION& stuTimeSection)
{
	stuTimeSection.nBeginHour = stBegin.wHour;
	stuTimeSection.nBeginMin = stBegin.wMinute;
	stuTimeSection.nBeginSec = stBegin.wSecond;
	stuTimeSection.nEndHour = stEnd.wHour;
	stuTimeSection.nEndMin = stEnd.wMinute;
	stuTimeSection.nEndSec = stEnd.wSecond;
}

/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgTimeSection message handlers

BOOL CSubDlgCfgTimeSection::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	InitDlg();
	StuToDlg();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSubDlgCfgTimeSection::OnSelchangeTimesectionCmbWeekday() 
{
	// TODO: Add your control notification handler code here
	int nSel = m_cmbWeekDay.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}
	
	for (int i = 0; i < MAX_REC_TSECT; i++)
	{
		ShowTimeSection(i, m_stuTimeSection[nSel][i]);
	}
}

void CSubDlgCfgTimeSection::OnOK() 
{
	// TODO: Add extra validation here
	StuToDlg();

	CDialog::OnOK();
}

void CSubDlgCfgTimeSection::OnEventhandlerChkSeg1Enable() 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::OnEventhandlerChkSeg2Enable() 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::OnEventhandlerChkSeg3Enable() 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::OnEventhandlerChkSeg4Enable() 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::OnEventhandlerChkSeg5Enable() 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::OnEventhandlerChkSeg6Enable() 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp1Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp1End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp2Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp2End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp3Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp3End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp4Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp4End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp5Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp5End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp6Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}

void CSubDlgCfgTimeSection::OnDatetimechangeEventhandlerDtp6End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	GetTimeSectionFromDlg();
	*pResult = 0;
}
