// TimeSchedule.cpp
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "TimeSchedule.h"
#include "DlgSensorInfoDescription.h"

IMPLEMENT_DYNAMIC(TimeSchedule, CDialog)

TimeSchedule::TimeSchedule(CWnd* pParent /*=NULL*/)
	: CDialog(TimeSchedule::IDD, pParent)
{
	memset(&m_stuInfo, 0 , sizeof(m_stuInfo));
	vecDoor.clear();

	memset(&m_stuAccessEvent, 0, sizeof(m_stuAccessEvent));
	memset(&m_stuRecordHoliday, 0, sizeof(m_stuRecordHoliday));
	m_stuRecordHoliday.dwSize = sizeof(m_stuRecordHoliday);
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
	DDX_Control(pDX, IDC_ACCESSCONTROL_CMB_CHANNEL, m_cmbChannel);
	DDX_Control(pDX, IDC_CHECK_AUTOREMOTE, m_chkAutoRemote);
	DDX_Control(pDX, IDC_COM_HOLIDAY_EXECUTETYPE, m_cmbCtlType);
	DDX_Control(pDX, IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE, m_HolidayStartDate);
	DDX_Control(pDX, IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE, m_HolidayEndDate);
	DDX_Control(pDX, IDC_INFO_HOLIDAY_CHECK_ENABLE, m_chcHolidayEnable);
	DDX_Control(pDX, IDC_CHECK_OPENALWAYS, m_chkOpenAlways);
	DDX_Control(pDX, IDC_CHECK_CLOSEALWAYS, m_chkCloseAlways);
}


BEGIN_MESSAGE_MAP(TimeSchedule, CDialog)

	ON_BN_CLICKED(IDC_TIMESCHEDULE_BTN_SET, &TimeSchedule::OnBnClickedTimescheduleBtnSet)
	ON_BN_CLICKED(IDC_TIMESCHEDULE_BTN_GET, &TimeSchedule::OnBnClickedTimescheduleBtnGet)
	ON_BN_CLICKED(IDC_TIMECFG_BTN_SET, &TimeSchedule::OnBnClickedTimecfgBtnSet)
	ON_BN_CLICKED(IDC_TIMECFG_BTN_GET, &TimeSchedule::OnBnClickedTimecfgBtnGet)
	ON_BN_CLICKED(IDC_BUT_HOLIDAY_EXECUTE, &TimeSchedule::OnBnClickedButHolidayExecute)
	ON_BN_CLICKED(IDC_INFO_HOLIDAY_BUTTON_DOORS, &TimeSchedule::OnBnClickedInfoHolidayButtonDoors)
	ON_CBN_SELCHANGE(IDC_COM_HOLIDAY_EXECUTETYPE, &TimeSchedule::OnCbnSelchangeComHolidayExecutetype)
	ON_CBN_SELCHANGE(IDC_TIMESCHEDULE_CMB_INDEX, &TimeSchedule::OnCbnSelchangeTimescheduleCmbIndex)
	ON_CBN_SELCHANGE(IDC_TIMESCHEDULE_CMB_WEEKDAY, &TimeSchedule::OnCbnSelchangeTimescheduleCmbWeekday)
	ON_BN_CLICKED(IDC_CHECK_OPENALWAYS, &TimeSchedule::OnBnClickedCheckOpenalways)
	ON_BN_CLICKED(IDC_CHECK_CLOSEALWAYS, &TimeSchedule::OnBnClickedCheckClosealways)
	ON_BN_CLICKED(IDC_BUT_HOLIDAY_GET, &TimeSchedule::OnBnClickedButHolidayGet)
END_MESSAGE_MAP()


BOOL TimeSchedule::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	unsigned int i = 0;

	m_cmbCtlType.ResetContent();
	for (int n = 0; n < sizeof(stuDemoRecordSetCtlType)/sizeof(stuDemoRecordSetCtlType[0]); n++)
	{
		m_cmbCtlType.InsertString(-1, ConvertString(stuDemoRecordSetCtlType[n].szName));
	}
	m_cmbCtlType.SetCurSel(0);

	m_cmbChannel.ResetContent();
	int nChn = 4;
	int nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(nAccessGroup);
	if (bret && nAccessGroup > 0)
	{
		nChn = nAccessGroup;
	}
	for (i = 0; i < nChn; i++)
	{
		char szContent[8] = {0};
		_snprintf(szContent, sizeof(szContent), "%d", i + 1);
		m_cmbChannel.AddString(szContent);
		m_cmbChannel.SetItemData(i, (DWORD)i);
	}
	m_cmbChannel.SetCurSel(0);	

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
		OnBnClickedTimecfgBtnGet();
	}
	OnCbnSelchangeComHolidayExecutetype();
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

/////////////
void TimeSchedule::OnBnClickedTimecfgBtnSet()
{
	CFG_ACCESS_EVENT_INFO stuAccessEvent;
	memset(&stuAccessEvent, 0, sizeof(stuAccessEvent));
	stuAccessEvent = m_stuAccessEvent;
	CString strTemp;
	GetDlgItem(IDC_EDIT_OPENALWAYS)->GetWindowText(strTemp);
	stuAccessEvent.nOpenAlwaysTimeIndex = atoi(strTemp);
	stuAccessEvent.abOpenAlwaysTimeIndex = true/*m_chkOpenAlways.GetCheck()*/;
	
	GetDlgItem(IDC_EDIT_CLOSEALWAYS)->GetWindowText(strTemp);
	stuAccessEvent.nCloseAlwaysTimeIndex = atoi(strTemp);
	stuAccessEvent.abCloseAlwaysTimeIndex = true/*m_chkCloseAlways.GetCheck()*/;

	GetDlgItem(IDC_EDIT_AUTOREMOTE)->GetWindowText(strTemp);
	stuAccessEvent.stuAutoRemoteCheck.nTimeSechdule = atoi(strTemp);
	stuAccessEvent.stuAutoRemoteCheck.bEnable = m_chkAutoRemote.GetCheck();
	BOOL bret = Device::GetInstance().SetAccessEventCfg(m_cmbChannel.GetCurSel(), &stuAccessEvent);
	if (bret)
	{
		m_stuAccessEvent = stuAccessEvent;
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set DevTimeSchedule failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void TimeSchedule::OnBnClickedTimecfgBtnGet()
{
	BOOL bret = Device::GetInstance().GetAccessEventCfg(m_cmbChannel.GetCurSel(), m_stuAccessEvent);
	if (bret)
	{
		CString strTemp;
		strTemp.Format("%d",m_stuAccessEvent.nOpenAlwaysTimeIndex);
		GetDlgItem(IDC_EDIT_OPENALWAYS)->SetWindowText(strTemp);
		//m_chkOpenAlways.SetCheck(m_stuAccessEvent.abOpenAlwaysTimeIndex);
		//GetDlgItem(IDC_EDIT_OPENALWAYS)->EnableWindow(m_stuAccessEvent.abOpenAlwaysTimeIndex);

		strTemp.Format("%d",m_stuAccessEvent.nCloseAlwaysTimeIndex);
		GetDlgItem(IDC_EDIT_CLOSEALWAYS)->SetWindowText(strTemp);
		//m_chkCloseAlways.SetCheck(m_stuAccessEvent.abCloseAlwaysTimeIndex);
		//GetDlgItem(IDC_EDIT_CLOSEALWAYS)->EnableWindow(m_stuAccessEvent.abCloseAlwaysTimeIndex);

		strTemp.Format("%d",m_stuAccessEvent.stuAutoRemoteCheck.nTimeSechdule);
		GetDlgItem(IDC_EDIT_AUTOREMOTE)->SetWindowText(strTemp);
		m_chkAutoRemote.SetCheck(m_stuAccessEvent.stuAutoRemoteCheck.bEnable);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get DevTimeSchedule failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void TimeSchedule::OnBnClickedInfoHolidayButtonDoors()
{
	std::vector<int> vecChn;
	int i = 0;
	for (; i < __min(m_stuRecordHoliday.nDoorNum, DH_MAX_DOOR_NUM); i++)
	{
		vecChn.push_back(m_stuRecordHoliday.sznDoors[i]);
	}
	int m_emOperateType = m_cmbCtlType.GetCurSel() + 1;
	int m_nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(m_nAccessGroup);
	if (bret && m_nAccessGroup > 0)
	{
		DlgSensorInfoDescription dlg(this, m_nAccessGroup);
		dlg.SetID(vecChn);
		if (IDOK == dlg.DoModal())
		{
			if (Em_Operate_Type_Insert == m_emOperateType
				|| Em_Operate_Type_Update == m_emOperateType)
			{
				vecChn.clear();
				vecChn = dlg.GetID();
				vecDoor.clear();
				vecDoor = dlg.GetID();
			}
		}
	}
}

void TimeSchedule::OnBnClickedButHolidayExecute()
{
	int nCtlType = m_cmbCtlType.GetCurSel();
	if (-1 == nCtlType)
	{
		return;
	}
	memset(&m_stuRecordHoliday, 0, sizeof(m_stuRecordHoliday));
	m_stuRecordHoliday.dwSize = sizeof(m_stuRecordHoliday);
	if (Em_Operate_Type_Insert == nCtlType + 1)
	{
		HolidayInsert();
	}
	else if (Em_Operate_Type_Get == nCtlType + 1)
	{
		HolidayGet();
	}
	else if (Em_Operate_Type_Update == nCtlType + 1)
	{
		HolidayUpdate();
		OnChangeUIState(2);
	}
	else if (Em_Operate_Type_Remove == nCtlType + 1)
	{
		HolidayRemove();
	}
	else if (Em_Operate_Type_Clear == nCtlType + 1)
	{
		HolidayClear();
	}
}

void TimeSchedule::HolidayInsert()
{
	GetHolidayInfoFromCtrl();
	int nRecNo;
	BOOL bret = Device::GetInstance().InsertHolidayRecorde(&m_stuRecordHoliday,nRecNo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s:%d", ConvertString("Insert holiday ok with RecNo"), nRecNo);
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void TimeSchedule::HolidayGet()
{
	GetHolidayInfoFromCtrl();
	BOOL bret = Device::GetInstance().GetHolidayRecorde(m_stuRecordHoliday);
	if (bret)
	{
		SetHolidayInfoToCtrl();
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void TimeSchedule::HolidayUpdate()
{
	GetHolidayInfoFromCtrl();
	BOOL bret = Device::GetInstance().UpdateHolidayRecorde(&m_stuRecordHoliday);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void TimeSchedule::HolidayRemove()
{
	GetHolidayInfoFromCtrl();
	BOOL bret = Device::GetInstance().RemoveHolidayRecorde(&m_stuRecordHoliday);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void TimeSchedule::HolidayClear()
{
	BOOL bret = Device::GetInstance().ClearHolidayRecorde();
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void TimeSchedule::GetHolidayInfoFromCtrl()
{
	m_stuRecordHoliday.nRecNo = GetDlgItemInt(IDC_INFO_HOLIDAY_EDIT_RECNO);

	// enable
	if (m_chcHolidayEnable.GetCheck())
	{
		m_stuRecordHoliday.bEnable = TRUE;
	}
	else 
	{
		m_stuRecordHoliday.bEnable = FALSE;
	}

	// start time
	SYSTEMTIME st = {0};
	m_HolidayStartDate.GetTime(&st);
	m_stuRecordHoliday.stuStartTime.dwYear = st.wYear;
	m_stuRecordHoliday.stuStartTime.dwMonth = st.wMonth;
	m_stuRecordHoliday.stuStartTime.dwDay = st.wDay;

	// end time
	m_HolidayEndDate.GetTime(&st);
	m_stuRecordHoliday.stuEndTime.dwYear = st.wYear;
	m_stuRecordHoliday.stuEndTime.dwMonth = st.wMonth;
	m_stuRecordHoliday.stuEndTime.dwDay = st.wDay;

	// door
	std::vector<int>::iterator it = vecDoor.begin();
	for (int i = 0; i < __min(vecDoor.size(), DH_MAX_DOOR_NUM) && it != vecDoor.end(); i++, it++)
	{
		m_stuRecordHoliday.sznDoors[i] = *it;
	}
	m_stuRecordHoliday.nDoorNum = __min(vecDoor.size(), DH_MAX_DOOR_NUM);

	// holiday no
	GetDlgItemText(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO, m_stuRecordHoliday.szHolidayNo, sizeof(m_stuRecordHoliday.szHolidayNo));
}

void TimeSchedule::SetHolidayInfoToCtrl()
{
	// RecNo
	SetDlgItemInt(IDC_INFO_HOLIDAY_EDIT_RECNO, m_stuRecordHoliday.nRecNo);

	// enable
	m_chcHolidayEnable.SetCheck(m_stuRecordHoliday.bEnable ? BST_CHECKED : BST_UNCHECKED);

	// start time
	SYSTEMTIME st = {0};
	st.wYear	= (WORD)m_stuRecordHoliday.stuStartTime.dwYear;
	st.wMonth	= (WORD)m_stuRecordHoliday.stuStartTime.dwMonth;
	st.wDay		= (WORD)m_stuRecordHoliday.stuStartTime.dwDay;
	m_HolidayStartDate.SetTime(&st);

	// end time
	st.wYear	= (WORD)m_stuRecordHoliday.stuEndTime.dwYear;
	st.wMonth	= (WORD)m_stuRecordHoliday.stuEndTime.dwMonth;
	st.wDay		= (WORD)m_stuRecordHoliday.stuEndTime.dwDay;
	m_HolidayEndDate.SetTime(&st);

	// door

	// holiday no
	SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO, m_stuRecordHoliday.szHolidayNo);
}

void TimeSchedule::OnCbnSelchangeComHolidayExecutetype()
{
	int nCtlType = m_cmbCtlType.GetCurSel();
	OnChangeUIState(nCtlType);
}

void TimeSchedule::OnChangeUIState(int nState)
{
	GetDlgItem(IDC_INFO_HOLIDAY_EDIT_RECNO)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_HOLIDAY_BUTTON_DOORS)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO)->EnableWindow(TRUE);

	GetDlgItem(IDC_BUT_HOLIDAY_GET)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUT_HOLIDAY_GET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->EnableWindow(TRUE);
	//int nCtlType = m_cmbCtlType.GetCurSel();
	switch(nState)
	{
	case 0://Insert
		{
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_RECNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(TRUE);
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_RECNO, "");
			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->SetWindowText(ConvertString("Insert"));
		}
		break;
	case 1://Get
		{
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO, "");
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_RECNO, "");
			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->SetWindowText(ConvertString("Get"));
		}
		break;
	case 2://Update step1
		{
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO, "");
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_RECNO, "");

			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->SetWindowText(ConvertString("Update"));
			GetDlgItem(IDC_BUT_HOLIDAY_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_HOLIDAY_GET)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->EnableWindow(FALSE);
		}
		break;
	case 3://Remove
		{
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_RECNO, "");
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO, "");
			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->SetWindowText(ConvertString("Remove"));
		}
		break;
	case 4://Clear
		{
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_RECNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_RECNO, "");
			SetDlgItemText(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO, "");
			m_chcHolidayEnable.SetCheck(BST_UNCHECKED);

			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->SetWindowText(ConvertString("Clear"));
		}
		break;
	case 5://Update step2
		{
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_RECNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_STARTDATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_INFO_HOLIDAY_DATETIMEPICKER_ENDDATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_INFO_HOLIDAY_CHECK_ENABLE)->EnableWindow(TRUE);
			GetDlgItem(IDC_INFO_HOLIDAY_BUTTON_DOORS)->EnableWindow(TRUE);
			GetDlgItem(IDC_INFO_HOLIDAY_EDIT_HOLIDAYNO)->EnableWindow(TRUE);

			GetDlgItem(IDC_BUT_HOLIDAY_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_HOLIDAY_GET)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUT_HOLIDAY_EXECUTE)->EnableWindow(TRUE);
		}
		break;
	default:
		break;
	}
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

void TimeSchedule::OnBnClickedCheckOpenalways()
{
	GetDlgItem(IDC_EDIT_OPENALWAYS)->EnableWindow(m_chkOpenAlways.GetCheck());
}

void TimeSchedule::OnBnClickedCheckClosealways()
{
	GetDlgItem(IDC_EDIT_CLOSEALWAYS)->EnableWindow(m_chkCloseAlways.GetCheck());
}

void TimeSchedule::OnBnClickedButHolidayGet()
{
	GetHolidayInfoFromCtrl();
	BOOL bret = Device::GetInstance().GetHolidayRecorde(m_stuRecordHoliday);
	if (bret)
	{
		SetHolidayInfoToCtrl();
		//
		OnChangeUIState(5);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}
