// HolidaySpecialdayInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "HolidaySpecialdayInfo.h"


// HolidaySpecialdayInfo 对话框

IMPLEMENT_DYNAMIC(HolidaySpecialdayInfo, CDialog)

HolidaySpecialdayInfo::HolidaySpecialdayInfo(CWnd* pParent /*=NULL*/)
	: CDialog(HolidaySpecialdayInfo::IDD, pParent)
{
	memset(&m_stuSpecialDayInfo,0,sizeof(m_stuSpecialDayInfo));
}

HolidaySpecialdayInfo::~HolidaySpecialdayInfo()
{
}

void HolidaySpecialdayInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SPECIALDAY_START_DATE, m_dtpVDStart);
	DDX_Control(pDX, IDC_SPECIALDAY_START_TIME, m_dtpVTStart);
	DDX_Control(pDX, IDC_SPECIALDAY_END_DATE, m_dtpVDEnd);
	DDX_Control(pDX, IDC_SPECIALDAY_END_TIME, m_dtpVTEnd);
}


BEGIN_MESSAGE_MAP(HolidaySpecialdayInfo, CDialog)
	ON_BN_CLICKED(IDOK, &HolidaySpecialdayInfo::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL HolidaySpecialdayInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	StuToDlg();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void HolidaySpecialdayInfo::InitDlg()
{

}
void HolidaySpecialdayInfo::StuToDlg()
{
	//szDayName
	SetDlgItemText(IDC_SPECIALDAY_EDIT_NAME, m_stuSpecialDayInfo.szDayName);
	// valid time start
	SYSTEMTIME st = {0};
	st.wYear	= (WORD)m_stuSpecialDayInfo.stuStartTime.dwYear;
	st.wMonth	= (WORD)m_stuSpecialDayInfo.stuStartTime.dwMonth;
	st.wDay		= (WORD)m_stuSpecialDayInfo.stuStartTime.dwDay;
	m_dtpVDStart.SetTime(&st);
	st.wHour    = (WORD)m_stuSpecialDayInfo.stuStartTime.dwHour;
	st.wMinute  = (WORD)m_stuSpecialDayInfo.stuStartTime.dwMinute;
	st.wSecond  = (WORD)m_stuSpecialDayInfo.stuStartTime.dwSecond;
	m_dtpVTStart.SetTime(&st);

	// valid time end
	st.wYear	= (WORD)m_stuSpecialDayInfo.stuEndTime.dwYear;
	st.wMonth	= (WORD)m_stuSpecialDayInfo.stuEndTime.dwMonth;
	st.wDay		= (WORD)m_stuSpecialDayInfo.stuEndTime.dwDay;
	m_dtpVDEnd.SetTime(&st);
	st.wHour    = (WORD)m_stuSpecialDayInfo.stuEndTime.dwHour;
	st.wMinute  = (WORD)m_stuSpecialDayInfo.stuEndTime.dwMinute;
	st.wSecond  = (WORD)m_stuSpecialDayInfo.stuEndTime.dwSecond;
	m_dtpVTEnd.SetTime(&st);
}
void HolidaySpecialdayInfo::DlgToStu()
{
	//szDayName
	GetDlgItemText(IDC_SPECIALDAY_EDIT_NAME, m_stuSpecialDayInfo.szDayName, sizeof(m_stuSpecialDayInfo.szDayName));
	// valid time start
	SYSTEMTIME st = {0};
	m_dtpVDStart.GetTime(&st);
	m_stuSpecialDayInfo.stuStartTime.dwYear = st.wYear;
	m_stuSpecialDayInfo.stuStartTime.dwMonth = st.wMonth;
	m_stuSpecialDayInfo.stuStartTime.dwDay = st.wDay;
	m_dtpVTStart.GetTime(&st);
	m_stuSpecialDayInfo.stuStartTime.dwHour = st.wHour;
	m_stuSpecialDayInfo.stuStartTime.dwMinute = st.wMinute;
	m_stuSpecialDayInfo.stuStartTime.dwSecond = st.wSecond;

	// valid time end
	m_dtpVDEnd.GetTime(&st);
	m_stuSpecialDayInfo.stuEndTime.dwYear = st.wYear;
	m_stuSpecialDayInfo.stuEndTime.dwMonth = st.wMonth;
	m_stuSpecialDayInfo.stuEndTime.dwDay = st.wDay;
	m_dtpVTEnd.GetTime(&st);
	m_stuSpecialDayInfo.stuEndTime.dwHour = st.wHour;
	m_stuSpecialDayInfo.stuEndTime.dwMinute = st.wMinute;
	m_stuSpecialDayInfo.stuEndTime.dwSecond = st.wSecond;

}
void HolidaySpecialdayInfo::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}


void HolidaySpecialdayInfo::OnOK() 
{
	// TODO: Add extra validation here
	DlgToStu();
	CDialog::OnOK();
}