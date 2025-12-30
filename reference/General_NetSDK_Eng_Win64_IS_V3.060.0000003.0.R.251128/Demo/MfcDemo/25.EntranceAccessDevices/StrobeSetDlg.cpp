// StrobeSetDlg.cpp :
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "StrobeSetDlg.h"
IMPLEMENT_DYNAMIC(StrobeSetDlg, CDialog)

StrobeSetDlg::StrobeSetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(StrobeSetDlg::IDD, pParent)
	, m_strhours(_T("0"))
	, m_strminutes(_T("0"))
	, m_strseconds(_T("0"))
	, m_strhoure(_T("23"))
	, m_strminutee(_T("59"))
	, m_strseconde(_T("59"))
{
	memset(&m_stuTrafficStrobeInfo_Get,0,sizeof(m_stuTrafficStrobeInfo_Get));
}

StrobeSetDlg::~StrobeSetDlg()
{

}

void StrobeSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(StrobeSetDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_Week, &StrobeSetDlg::OnCbnSelchangeComboWeek)
	ON_CBN_SELCHANGE(IDC_COMBO_TimeSlot, &StrobeSetDlg::OnCbnSelchangeComboTimeslot)
	ON_BN_CLICKED(IDC_BUTTON_SET, &StrobeSetDlg::OnBnClickedButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_GET, &StrobeSetDlg::OnBnClickedButtonGet)
	ON_EN_CHANGE(IDC_EDIT_hours, &StrobeSetDlg::OnEnChangeEdithours)
	ON_EN_CHANGE(IDC_EDIT_minutes, &StrobeSetDlg::OnEnChangeEditminutes)
	ON_EN_CHANGE(IDC_EDIT_seconds, &StrobeSetDlg::OnEnChangeEditseconds)
	ON_EN_CHANGE(IDC_EDIT_houre, &StrobeSetDlg::OnEnChangeEdithoure)
	ON_EN_CHANGE(IDC_EDIT_minutee, &StrobeSetDlg::OnEnChangeEditminutee)
	ON_EN_CHANGE(IDC_EDIT_seconde, &StrobeSetDlg::OnEnChangeEditseconde)
	ON_EN_KILLFOCUS(IDC_EDIT_hours, &StrobeSetDlg::OnEnKillfocusEdithours)
	ON_EN_KILLFOCUS(IDC_EDIT_minutes, &StrobeSetDlg::OnEnKillfocusEditminutes)
	ON_EN_KILLFOCUS(IDC_EDIT_seconds, &StrobeSetDlg::OnEnKillfocusEditseconds)
	ON_EN_KILLFOCUS(IDC_EDIT_houre, &StrobeSetDlg::OnEnKillfocusEdithoure)
	ON_EN_KILLFOCUS(IDC_EDIT_minutee, &StrobeSetDlg::OnEnKillfocusEditminutee)
	ON_EN_KILLFOCUS(IDC_EDIT_seconde, &StrobeSetDlg::OnEnKillfocusEditseconde)
END_MESSAGE_MAP()

BOOL StrobeSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Sun"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Mon"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Tue"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Wed"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Thu"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Fri"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->AddString(ConvertString("Sat"));
	((CComboBox*)GetDlgItem(IDC_COMBO_Week))->SetCurSel(0);

	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->AddString(ConvertString("period1"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->AddString(ConvertString("period2"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->AddString(ConvertString("period3"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->AddString(ConvertString("period4"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->AddString(ConvertString("period5"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->AddString(ConvertString("period6"));
	((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->SetCurSel(0);

	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedButtonGet();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void StrobeSetDlg::OnCbnSelchangeComboWeek()
{
	GetTimefromStu();
}

void StrobeSetDlg::OnCbnSelchangeComboTimeslot()
{
	GetTimefromStu();
}

void StrobeSetDlg::GetTimefromStu()
{
	NET_CFG_TRAFFICSTROBE_INFO *stuTrafficStrobeInfo = NEW NET_CFG_TRAFFICSTROBE_INFO;
	if (NULL == stuTrafficStrobeInfo)
	{
		return;
	}
	memset(stuTrafficStrobeInfo,0,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
	stuTrafficStrobeInfo->dwSize = sizeof(NET_CFG_TRAFFICSTROBE_INFO);

	memcpy(stuTrafficStrobeInfo,&m_stuTrafficStrobeInfo_Get,sizeof(NET_CFG_TRAFFICSTROBE_INFO));

	int nweek = ((CComboBox*)GetDlgItem(IDC_COMBO_Week))->GetCurSel();
	int ntimslot = ((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->GetCurSel();

	((CButton*)GetDlgItem(IDC_CHECK_OPEN))->SetCheck(stuTrafficStrobeInfo->stuStationaryOpen.bEnable);

	m_strhours.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iBeginHour);
	SetDlgItemText(IDC_EDIT_hours,m_strhours);
	m_strminutes.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iBeginMin);
	SetDlgItemText(IDC_EDIT_minutes,m_strminutes);
	m_strseconds.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iBeginSec);
	SetDlgItemText(IDC_EDIT_seconds,m_strseconds);
	m_strhoure.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iEndHour);
	SetDlgItemText(IDC_EDIT_houre,m_strhoure);
	m_strminutee.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iEndMin);
	SetDlgItemText(IDC_EDIT_minutee,m_strminutee);
	m_strseconde.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iEndSec);
	SetDlgItemText(IDC_EDIT_seconde,m_strseconde);

	((CButton*)GetDlgItem(IDC_CHECK_TIME))->SetCheck(stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].bEnable);
	if (NULL != stuTrafficStrobeInfo)
	{
		delete stuTrafficStrobeInfo;
		stuTrafficStrobeInfo = NULL;
	}
}

void StrobeSetDlg::OnBnClickedButtonSet()
{
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_GET)->EnableWindow(FALSE);
	int nweek = ((CComboBox*)GetDlgItem(IDC_COMBO_Week))->GetCurSel();
	int ntimslot = ((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->GetCurSel();

	NET_CFG_TRAFFICSTROBE_INFO *stuTrafficStrobeInfo = NEW NET_CFG_TRAFFICSTROBE_INFO;
	if (NULL == stuTrafficStrobeInfo)
	{
		return;
	}
	memset(stuTrafficStrobeInfo,0,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
	stuTrafficStrobeInfo->dwSize = sizeof(NET_CFG_TRAFFICSTROBE_INFO);

	memcpy(stuTrafficStrobeInfo,&m_stuTrafficStrobeInfo_Get,sizeof(NET_CFG_TRAFFICSTROBE_INFO));

	stuTrafficStrobeInfo->stuStationaryOpen.bEnable = ((CButton*)GetDlgItem(IDC_CHECK_OPEN))->GetCheck();
	GetDlgItemText(IDC_EDIT_hours,m_strhours);
	stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iBeginHour = atoi(m_strhours);
	GetDlgItemText(IDC_EDIT_minutes,m_strminutes);
	stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iBeginMin = atoi(m_strminutes);
	GetDlgItemText(IDC_EDIT_seconds,m_strseconds);
	stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iBeginSec = atoi(m_strseconds);
	GetDlgItemText(IDC_EDIT_houre,m_strhoure);
	stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iEndHour = atoi(m_strhoure);
	GetDlgItemText(IDC_EDIT_minutee,m_strminutee);
	stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iEndMin = atoi(m_strminutee);
	GetDlgItemText(IDC_EDIT_seconde,m_strseconde);
	stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].iEndSec = atoi(m_strseconde);

	if (((CButton*)GetDlgItem(IDC_CHECK_TIME))->GetCheck() == 1)
	{
		stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].bEnable = 1;
	} 
	else
	{
		stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[nweek][ntimslot].bEnable = 0;
	}
	BOOL bret = Device::GetInstance().SetStrobe(NET_EM_CFG_TRAFFICSTROBE,0,stuTrafficStrobeInfo,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
	else
	{
		memcpy(&m_stuTrafficStrobeInfo_Get,stuTrafficStrobeInfo,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
	}
	if (NULL != stuTrafficStrobeInfo)
	{
		delete stuTrafficStrobeInfo;
		stuTrafficStrobeInfo = NULL;
	}
	
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_GET)->EnableWindow(TRUE);
}

void StrobeSetDlg::OnBnClickedButtonGet()
{
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_GET)->EnableWindow(FALSE);
	NET_CFG_TRAFFICSTROBE_INFO *stuTrafficStrobeInfo = NEW NET_CFG_TRAFFICSTROBE_INFO;
	if (NULL == stuTrafficStrobeInfo)
	{
		return;
	}
	memset(stuTrafficStrobeInfo,0,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
	stuTrafficStrobeInfo->dwSize = sizeof(NET_CFG_TRAFFICSTROBE_INFO);
	BOOL bret = Device::GetInstance().GetStrobe(NET_EM_CFG_TRAFFICSTROBE,0,stuTrafficStrobeInfo,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
	if (bret)
	{
		memcpy(&m_stuTrafficStrobeInfo_Get,stuTrafficStrobeInfo,sizeof(NET_CFG_TRAFFICSTROBE_INFO));
		((CComboBox*)GetDlgItem(IDC_COMBO_Week))->SetCurSel(0);
		((CComboBox*)GetDlgItem(IDC_COMBO_TimeSlot))->SetCurSel(0);

		((CButton*)GetDlgItem(IDC_CHECK_OPEN))->SetCheck(stuTrafficStrobeInfo->stuStationaryOpen.bEnable);

		m_strhours.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].iBeginHour);
		SetDlgItemText(IDC_EDIT_hours,m_strhours);
		m_strminutes.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].iBeginMin);
		SetDlgItemText(IDC_EDIT_minutes,m_strminutes);
		m_strseconds.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].iBeginSec);
		SetDlgItemText(IDC_EDIT_seconds,m_strseconds);
		m_strhoure.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].iEndHour);
		SetDlgItemText(IDC_EDIT_houre,m_strhoure);
		m_strminutee.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].iEndMin);
		SetDlgItemText(IDC_EDIT_minutee,m_strminutee);
		m_strseconde.Format("%d",stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].iEndSec);
		SetDlgItemText(IDC_EDIT_seconde,m_strseconde);

		((CButton*)GetDlgItem(IDC_CHECK_TIME))->SetCheck(stuTrafficStrobeInfo->stuStationaryOpen.stTimeShecule.stuTimeSection[0][0].bEnable);
	}
	else
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
	if (NULL != stuTrafficStrobeInfo)
	{
		delete stuTrafficStrobeInfo;
		stuTrafficStrobeInfo = NULL;
	}
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_GET)->EnableWindow(TRUE);
}

void StrobeSetDlg::OnEnChangeEdithours()
{
	GetDlgItemText(IDC_EDIT_hours,m_strhours);
	if (atoi(m_strhours) > 23)
	{
		m_strhours.Format("%d", 23);
		SetDlgItemText(IDC_EDIT_hours,m_strhours);
	}
	if (atoi(m_strhours) < 0)
	{
		m_strhours.Format("%d", 0);
		SetDlgItemText(IDC_EDIT_hours,m_strhours);
	}
}

void StrobeSetDlg::OnEnChangeEditminutes()
{
	GetDlgItemText(IDC_EDIT_minutes,m_strminutes);
	if (atoi(m_strminutes) > 59)
	{
		m_strminutes.Format("%d", 59);
		SetDlgItemText(IDC_EDIT_minutes,m_strminutes);
	}
	if (atoi(m_strminutes) < 0)
	{
		m_strminutes.Format("%d", 0);
		SetDlgItemText(IDC_EDIT_minutes,m_strminutes);
	}
}

void StrobeSetDlg::OnEnChangeEditseconds()
{
	GetDlgItemText(IDC_EDIT_seconds,m_strseconds);
	if (atoi(m_strseconds) > 59)
	{
		m_strseconds.Format("%d", 59);
		SetDlgItemText(IDC_EDIT_seconds,m_strseconds);
	}
	if (atoi(m_strseconds) < 0)
	{
		m_strseconds.Format("%d", 0);
		SetDlgItemText(IDC_EDIT_seconds,m_strseconds);
	}
}

void StrobeSetDlg::OnEnChangeEdithoure()
{
	GetDlgItemText(IDC_EDIT_houre,m_strhoure);
	if (atoi(m_strhoure) > 23)
	{
		m_strhoure.Format("%d", 23);
		SetDlgItemText(IDC_EDIT_houre,m_strhoure);
	}
	if (atoi(m_strhoure) < 0)
	{
		m_strhoure.Format("%d", 0);
		SetDlgItemText(IDC_EDIT_houre,m_strhoure);
	}
}

void StrobeSetDlg::OnEnChangeEditminutee()
{
	GetDlgItemText(IDC_EDIT_minutee,m_strminutee);
	if (atoi(m_strminutee) > 59)
	{
		m_strminutee.Format("%d", 59);
		SetDlgItemText(IDC_EDIT_minutee,m_strminutee);
	}
	if (atoi(m_strminutee) < 0)
	{
		m_strminutee.Format("%d", 0);
		SetDlgItemText(IDC_EDIT_minutee,m_strminutee);
	}
}

void StrobeSetDlg::OnEnChangeEditseconde()
{
	GetDlgItemText(IDC_EDIT_seconde,m_strseconde);
	if (atoi(m_strseconde) > 59)
	{
		m_strseconde.Format("%d", 59);
		SetDlgItemText(IDC_EDIT_seconde,m_strseconde);
	}
	if (atoi(m_strseconde) < 0)
	{
		m_strseconde.Format("%d", 0);
		SetDlgItemText(IDC_EDIT_seconde,m_strseconde);
	}
}

void StrobeSetDlg::OnEnKillfocusEdithours()
{
	MakeTimeNormal(TRUE);
}

void StrobeSetDlg::OnEnKillfocusEditminutes()
{
	MakeTimeNormal(TRUE);
}

void StrobeSetDlg::OnEnKillfocusEditseconds()
{
	MakeTimeNormal(TRUE);
}

void StrobeSetDlg::OnEnKillfocusEdithoure()
{
	MakeTimeNormal(FALSE);
}

void StrobeSetDlg::OnEnKillfocusEditminutee()
{
	MakeTimeNormal(FALSE);
}

void StrobeSetDlg::OnEnKillfocusEditseconde()
{
	MakeTimeNormal(FALSE);
}

void StrobeSetDlg::MakeTimeNormal(BOOL bStartEnd)
{
	GetDlgItemText(IDC_EDIT_hours,m_strhours);
	GetDlgItemText(IDC_EDIT_minutes,m_strminutes);
	GetDlgItemText(IDC_EDIT_seconds,m_strseconds);
	GetDlgItemText(IDC_EDIT_houre,m_strhoure);
	GetDlgItemText(IDC_EDIT_minutee,m_strminutee);
	GetDlgItemText(IDC_EDIT_seconde,m_strseconde);
	DWORD timeS = 0;
	timeS += 3600 * atoi(m_strhours);
	timeS += 60 * atoi(m_strminutes);
	timeS += atoi(m_strseconds);

	DWORD timeE = 0;
	timeE += 3600 * atoi(m_strhoure);
	timeE += 60 * atoi(m_strminutee);
	timeE += atoi(m_strseconde);
	
	if (timeS >= timeE)
	{
		if (bStartEnd)
		{
			timeE = timeS + 1;
		} 
		else
		{
			timeS = timeE - 1;
		}

		if (timeE % (24*3600) == 0)
		{
			timeS = 0;
			timeE = 1;
		}
		m_strhours.Format("%d", timeS/3600);
		m_strminutes.Format("%d", (timeS%3600)/60);
		m_strseconds.Format("%d", timeS%60);
		m_strhoure.Format("%d", timeE/3600);
		m_strminutee.Format("%d", (timeE%3600)/60);
		m_strseconde.Format("%d", timeE%60);
		SetDlgItemText(IDC_EDIT_hours,m_strhours);
		SetDlgItemText(IDC_EDIT_minutes,m_strminutes);
		SetDlgItemText(IDC_EDIT_seconds,m_strseconds);
		SetDlgItemText(IDC_EDIT_houre,m_strhoure);
		SetDlgItemText(IDC_EDIT_minutee,m_strminutee);
		SetDlgItemText(IDC_EDIT_seconde,m_strseconde);
	}
}