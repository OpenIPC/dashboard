// Holiday.cpp : 实现文件
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "Holiday.h"
#include "HolidaySpecialdayInfo.h"
#include "DlgSensorInfoDescription.h"
// Holiday 对话框

IMPLEMENT_DYNAMIC(Holiday, CDialog)

Holiday::Holiday(CWnd* pParent /*=NULL*/)
	: CDialog(Holiday::IDD, pParent)
{
	memset(&stuSpecialdayGroup,0,sizeof(stuSpecialdayGroup));
	stuSpecialdayGroup.dwSize = sizeof(stuSpecialdayGroup);
	memset(&stuSpecialdaySchedule,0,sizeof(stuSpecialdaySchedule));
	stuSpecialdaySchedule.dwSize = sizeof(stuSpecialdaySchedule);

	m_nSpeciadayIndex = -1;
}

Holiday::~Holiday()
{
}

void Holiday::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL_SPECIALDAY_INFO, m_listSpecialdayInfo);
	DDX_Control(pDX, IDC_SPECIALDAYGROUP_CMB_NO, m_cmbCtlID);
	DDX_Control(pDX, IDC_SPECIALDAYGROUP_CHK_ENABLE, m_chkGroupEnable);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_CMB_NO, m_cmbCtlScheduleID);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_CHK_ENABLE, m_chkSpeciadayEnable);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP1_START, m_dtp1Start);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP2_START, m_dtp2Start);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP3_START, m_dtp3Start);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP4_START, m_dtp4Start);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP1_END, m_dtp1End);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP2_END, m_dtp2End);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP3_END, m_dtp3End);
	DDX_Control(pDX, IDC_SPECIALDAYSCHEDULE_DTP4_END, m_dtp4End);
}


BEGIN_MESSAGE_MAP(Holiday, CDialog)
	ON_BN_CLICKED(IDC_SPECIALDAY_INFO_BTN_ADD, &Holiday::OnBnClickedSpecialdayInfoBtnAdd)
	ON_BN_CLICKED(IDC_SPECIALDAY_INFO_BTN_MODIFY, &Holiday::OnBnClickedSpecialdayInfoBtnModify)
	ON_BN_CLICKED(IDC_SPECIALDAY_INFO_BTN_DELETE, &Holiday::OnBnClickedSpecialdayInfoBtnDelete)
	ON_BN_CLICKED(IDC_SPECIALDAYGROUP_BTN_GET, &Holiday::OnBnClickedSpecialdaygroupBtnGet)
	ON_BN_CLICKED(IDC_SPECIALDAYGROUP_BTN_SET, &Holiday::OnBnClickedSpecialdaygroupBtnSet)
	ON_BN_CLICKED(IDC_SPECIALDAYSCHEDULE_BTN_GET, &Holiday::OnBnClickedSpecialdayscheduleBtnGet)
	ON_BN_CLICKED(IDC_SPECIALDAYSCHEDULE_BTN_SET, &Holiday::OnBnClickedSpecialdayscheduleBtnSet)

	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_SPECIALDAY_INFO, &Holiday::OnNMClickListctrlSpecialdayInfo)
//	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LISTCTRL_SPECIALDAY_INFO, &Holiday::OnLvnItemActivateListctrlSpecialdayInfo)
ON_BN_CLICKED(IDC_SPECIALDAYSCHEDULE_BTN_DOORS, &Holiday::OnBnClickedSpecialdayscheduleBtnDoors)
ON_CBN_SELCHANGE(IDC_SPECIALDAYGROUP_CMB_NO, &Holiday::OnCbnSelchangeSpecialdaygroupCmbNo)
ON_CBN_SELCHANGE(IDC_SPECIALDAYSCHEDULE_CMB_NO, &Holiday::OnCbnSelchangeSpecialdayscheduleCmbNo)
END_MESSAGE_MAP()


BOOL Holiday::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedSpecialdaygroupBtnGet();
		OnBnClickedSpecialdayscheduleBtnGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_SPECIALDAYGROUP_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIALDAYGROUP_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIALDAY_INFO_BTN_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIALDAY_INFO_BTN_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIALDAY_INFO_BTN_DELETE)->EnableWindow(FALSE);

		GetDlgItem(IDC_SPECIALDAYSCHEDULE_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPECIALDAYSCHEDULE_BTN_SET)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Holiday::InitDlg()
{
	// m_cmbCtlID
	m_cmbCtlID.ResetContent();
	for (int i = 0; i < 128; i++)
	{
		CString csInfo;
		csInfo.Format("%d", i);
		m_cmbCtlID.InsertString(-1, csInfo);
	}
	m_cmbCtlID.SetCurSel(0);

	//m_listSpecialdayInfo
	m_listSpecialdayInfo.SetExtendedStyle(m_listSpecialdayInfo.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listSpecialdayInfo.SetExtendedStyle(m_listSpecialdayInfo.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("Index", 40));
	vecTitles.push_back(ColDesObj("CustomName", 80));
	vecTitles.push_back(ColDesObj("StartTime", 120));
	vecTitles.push_back(ColDesObj("EndTime", 120));

	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_listSpecialdayInfo.InsertColumn(j, &lvc);
	}


	// m_cmbCtlScheduleID
	m_cmbCtlScheduleID.ResetContent();
	for (int i = 0; i < 128; i++)
	{
		CString csInfo;
		csInfo.Format("%d", i);
		m_cmbCtlScheduleID.InsertString(-1, csInfo);
	}
	m_cmbCtlScheduleID.SetCurSel(0);
}

void Holiday::GroupStuToDlg()
{
	// bGroupEnable
	if (stuSpecialdayGroup.bGroupEnable)
	{
		m_chkGroupEnable.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkGroupEnable.SetCheck(BST_UNCHECKED);
	}

	// szGroupName
	SetDlgItemText(IDC_SPECIALDAYGROUP_EDT_NAME, stuSpecialdayGroup.szGroupName);

	//stuSpeciaday
	SetListCtl();
}

void Holiday::GroupDlgToStu()
{
	// bGroupEnable
	stuSpecialdayGroup.bGroupEnable = m_chkGroupEnable.GetCheck() ? TRUE : FALSE;

	// szGroupName
	GetDlgItemText(IDC_SPECIALDAYGROUP_EDT_NAME, stuSpecialdayGroup.szGroupName, sizeof(stuSpecialdayGroup.szGroupName));
}

void Holiday::SetListCtl()
{
	m_listSpecialdayInfo.DeleteAllItems();
	m_listSpecialdayInfo.SetRedraw(FALSE);
	for (int i = 0; i < stuSpecialdayGroup.nSpeciaday; i++)
	{
		CString strIndex;
		strIndex.Format("%d",i + 1);
		LV_ITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.pszText = _T("");
		lvi.iImage = 0;
		lvi.iItem = i;
		m_listSpecialdayInfo.InsertItem(&lvi);
		m_listSpecialdayInfo.SetItemText(i, 0, strIndex);
		CString str;
		str.Format("%s",stuSpecialdayGroup.stuSpeciaday[i].szDayName);
		m_listSpecialdayInfo.SetItemText(i, 1, str);
		NET_TIME &StartTime = stuSpecialdayGroup.stuSpeciaday[i].stuStartTime;
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",StartTime.dwYear, StartTime.dwMonth, StartTime.dwDay, StartTime.dwHour, StartTime.dwMinute, StartTime.dwSecond);
		m_listSpecialdayInfo.SetItemText(i, 2, str);
		NET_TIME &EndTime = stuSpecialdayGroup.stuSpeciaday[i].stuEndTime;
		str.Format("%04d-%02d-%02d %02d:%02d:%02d",EndTime.dwYear, EndTime.dwMonth, EndTime.dwDay, EndTime.dwHour, EndTime.dwMinute, EndTime.dwSecond);
		m_listSpecialdayInfo.SetItemText(i, 3, str);
	}
	m_listSpecialdayInfo.SetRedraw(TRUE);
}


void Holiday::OnBnClickedSpecialdayInfoBtnAdd()
{
	if (stuSpecialdayGroup.nSpeciaday >= 16)
	{
		//已经满了
		MessageBox(ConvertString("Is full"), ConvertString("Prompt"));
		return;
	}
	HolidaySpecialdayInfo dlg(this);
	if (IDOK == dlg.DoModal())
	{
		const NET_ACCESSCTL_SPECIALDAY_INFO* pstuSpecialDayInfo = dlg.GetSpecialDayInfo();
		if (pstuSpecialDayInfo != NULL)
		{
			memcpy(&stuSpecialdayGroup.stuSpeciaday[stuSpecialdayGroup.nSpeciaday], pstuSpecialDayInfo, sizeof(NET_ACCESSCTL_SPECIALDAY_INFO));
		}
		stuSpecialdayGroup.nSpeciaday = stuSpecialdayGroup.nSpeciaday +1;
	}
	//GroupStuToDlg();
	//stuSpeciaday
	SetListCtl();
}

void Holiday::OnBnClickedSpecialdayInfoBtnModify()
{
	if (m_nSpeciadayIndex == -1)
	{
		return;
	}
	HolidaySpecialdayInfo dlg(this);
	dlg.SetSpecialDayInfo(&stuSpecialdayGroup.stuSpeciaday[m_nSpeciadayIndex]);
	if (IDOK == dlg.DoModal())
	{
		const NET_ACCESSCTL_SPECIALDAY_INFO* pstuSpecialDayInfo = dlg.GetSpecialDayInfo();
		if (pstuSpecialDayInfo != NULL)
		{
			memcpy(&stuSpecialdayGroup.stuSpeciaday[m_nSpeciadayIndex], pstuSpecialDayInfo, sizeof(NET_ACCESSCTL_SPECIALDAY_INFO));
		}
	}
	//GroupStuToDlg();
	//stuSpeciaday
	SetListCtl();
}

void Holiday::OnBnClickedSpecialdayInfoBtnDelete()
{
	if (m_nSpeciadayIndex == -1)
	{
		return;
	}
	for (int i=m_nSpeciadayIndex;i<(stuSpecialdayGroup.nSpeciaday -1);i++)
	{
		memcpy(&stuSpecialdayGroup.stuSpeciaday[i], &stuSpecialdayGroup.stuSpeciaday[i+1], sizeof(NET_ACCESSCTL_SPECIALDAY_INFO));
	}
	stuSpecialdayGroup.nSpeciaday = stuSpecialdayGroup.nSpeciaday -1;
	if (stuSpecialdayGroup.nSpeciaday < 0)
	{
		stuSpecialdayGroup.nSpeciaday = 0;
	}
	//GroupStuToDlg();
	//stuSpeciaday
	SetListCtl();
}

void Holiday::OnBnClickedSpecialdaygroupBtnGet()
{
	int nID = m_cmbCtlID.GetCurSel();
	BOOL bret = Device::GetInstance().GetSpecialDayGroup(nID, stuSpecialdayGroup);
	if (bret)
	{
		GroupStuToDlg();
	}
	else
	{
		MessageBox(ConvertString("Failed to get holiday group"), ConvertString("Prompt"));
	}
}

void Holiday::OnBnClickedSpecialdaygroupBtnSet()
{
	int nID = m_cmbCtlID.GetCurSel();
	GroupDlgToStu();
	BOOL bret = Device::GetInstance().SetSpecialDayGroup(nID, &stuSpecialdayGroup);
	if (!bret)
	{
		MessageBox(ConvertString("Failed to set holiday group"), ConvertString("Prompt"));
	}
}

void Holiday::OnNMClickListctrlSpecialdayInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pNMHDR)
	{
		HD_NOTIFY *phdn = (HD_NOTIFY*)pNMHDR;
		m_nSpeciadayIndex = phdn->iItem;
	}
	*pResult = 0;
}


void Holiday::ScheduleStuToDlg()
{
	//szSchduleName
	SetDlgItemText(IDC_SPECIALDAYSCHEDULE_EDT_NAME, stuSpecialdaySchedule.szSchduleName);
	// bGroupEnable
	if (stuSpecialdaySchedule.bSchdule)
	{
		m_chkSpeciadayEnable.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkSpeciadayEnable.SetCheck(BST_UNCHECKED);
	}
	//nGroupNo
	SetDlgItemInt(IDC_SPECIALDAYSCHEDULE_EDT_GROUPNO, stuSpecialdaySchedule.nGroupNo);
	//TimeSection
	SetTimeSection();
	//door

}

void Holiday::ScheduleDlgToStu()
{
	//szSchduleName
	GetDlgItemText(IDC_SPECIALDAYSCHEDULE_EDT_NAME, stuSpecialdaySchedule.szSchduleName, sizeof(stuSpecialdaySchedule.szSchduleName));
	//bGroupEnable
	stuSpecialdaySchedule.bSchdule = m_chkSpeciadayEnable.GetCheck() ? TRUE : FALSE;
	//nGroupNo
	stuSpecialdaySchedule.nGroupNo = GetDlgItemInt(IDC_SPECIALDAYSCHEDULE_EDT_GROUPNO, NULL, TRUE);
	//TimeSection
	GetTimeSection();
	//door

}

void Holiday::OnBnClickedSpecialdayscheduleBtnGet()
{
	int nID = m_cmbCtlScheduleID.GetCurSel();
	BOOL bret = Device::GetInstance().GetSpecialDaysSchedule(nID, stuSpecialdaySchedule);
	if (bret)
	{
		ScheduleStuToDlg();
	}
	else
	{
		MessageBox(ConvertString("Failed to get holiday plan"), ConvertString("Prompt"));
	}
}

void Holiday::OnBnClickedSpecialdayscheduleBtnSet()
{
	int nID = m_cmbCtlScheduleID.GetCurSel();
	ScheduleDlgToStu();
	BOOL bret = Device::GetInstance().SetSpecialDaysSchedule(nID, &stuSpecialdaySchedule);
	if (!bret)
	{
		MessageBox(ConvertString("Failed to set holiday plan"), ConvertString("Prompt"));
	}
}

void Holiday::OnBnClickedSpecialdayscheduleBtnDoors()
{
	std::vector<int> vecChn;
	int i = 0;
	for (; i < stuSpecialdaySchedule.nDoorNum; i++)
	{
		vecChn.push_back(stuSpecialdaySchedule.nDoors[i]);
	}

// 	int m_nAccessGroup = 0;
// 	BOOL bret = Device::GetInstance().GetAccessCount(m_nAccessGroup);
	DlgSensorInfoDescription dlg(this, 64/*m_nAccessGroup*/);
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		vecChn.clear();
		vecChn = dlg.GetID();

		std::vector<int>::iterator itDoor = vecChn.begin();
		for (int i = 0; i < __min(vecChn.size(), 64) && itDoor != vecChn.end(); i++, itDoor++)
		{
			stuSpecialdaySchedule.nDoors[i] = *itDoor;
		}
		stuSpecialdaySchedule.nDoorNum = __min(vecChn.size(), 64);
	}
}


void Holiday::GetTimeSection()
{
	for (int i = 0; i < 4; i++)
	{
		SYSTEMTIME stBegin = {0}, stEnd = {0};
		DH_TSECT& stuTimeSection = stuSpecialdaySchedule.stuTimeSection[i];
		switch (i)
		{
		case 0:
			m_dtp1Start.GetTime(&stBegin);
			m_dtp1End.GetTime(&stEnd);
			break;
		case 1:
			m_dtp2Start.GetTime(&stBegin);
			m_dtp2End.GetTime(&stEnd);
			break;
		case 2:
			m_dtp3Start.GetTime(&stBegin);
			m_dtp3End.GetTime(&stEnd);
			break;
		case 3:
			m_dtp4Start.GetTime(&stBegin);
			m_dtp4End.GetTime(&stEnd);
			break;
		}
		DTPToCfgTimeSection(stBegin, stEnd, stuTimeSection);
	}
}

void Holiday::SetTimeSection()
{
	for (int i =0; i< 4;i++)
	{
		SYSTEMTIME stBegin = {2000, 1, 1, 1}, stEnd = {2000, 1, 1, 1};
		CfgTimeSectionToDTP(stuSpecialdaySchedule.stuTimeSection[i], stBegin, stEnd);

		switch (i)
		{
		case 0:
			m_dtp1Start.SetTime(&stBegin);
			m_dtp1End.SetTime(&stEnd);
			break;
		case 1:
			m_dtp2Start.SetTime(&stBegin);
			m_dtp2End.SetTime(&stEnd);
			break;
		case 2:
			m_dtp3Start.SetTime(&stBegin);
			m_dtp3End.SetTime(&stEnd);
			break;
		case 3:
			m_dtp4Start.SetTime(&stBegin);
			m_dtp4End.SetTime(&stEnd);
			break;
		}
	}
}

void Holiday::CfgTimeSectionToDTP(const DH_TSECT& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd)
{
	stBegin.wHour = stuTimeSection.iBeginHour;
	stBegin.wMinute = stuTimeSection.iBeginMin;
	stBegin.wSecond = stuTimeSection.iBeginSec;
	stEnd.wHour = stuTimeSection.iEndHour;
	stEnd.wMinute = stuTimeSection.iEndMin;
	stEnd.wSecond = stuTimeSection.iEndSec;
}

void Holiday::DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, DH_TSECT& stuTimeSection)
{
	stuTimeSection.iBeginHour = stBegin.wHour;
	stuTimeSection.iBeginMin = stBegin.wMinute;
	stuTimeSection.iBeginSec = stBegin.wSecond;
	stuTimeSection.iEndHour = stEnd.wHour;
	stuTimeSection.iEndMin = stEnd.wMinute;
	stuTimeSection.iEndSec = stEnd.wSecond;
}

void Holiday::OnCbnSelchangeSpecialdaygroupCmbNo()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedSpecialdaygroupBtnGet();
}

void Holiday::OnCbnSelchangeSpecialdayscheduleCmbNo()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedSpecialdayscheduleBtnGet();
}
