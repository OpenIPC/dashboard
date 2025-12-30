// FireWarnSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "FireWarnSetDlg.h"
#include "ThermalCameraDlg.h"

// FireWarnSetDlg 对话框

IMPLEMENT_DYNAMIC(FireWarnSetDlg, CDialog)

FireWarnSetDlg::FireWarnSetDlg(CWnd* pParent /*=NULL*/, LLONG lLoginId/* = 0*/, int nChannelCount)
	: CDialog(FireWarnSetDlg::IDD, pParent)
	, m_strThermoSnapTimes(_T("0"))
	, m_strFireDuration(_T("0"))
	, m_strSensitivity(_T("0"))
	, m_strRecordLatch(_T("0"))
	, m_stralarmoutlatch(_T("0"))
	, m_strhours(_T("0"))
	, m_strminutes(_T("0"))
	, m_strseconds(_T("0"))
	, m_strhoure(_T("0"))
	, m_strminutee(_T("0"))
	, m_strseconde(_T("0"))
{
	m_lLoginID = lLoginId;
	m_nChannelCount = nChannelCount;

	m_row = 0; 
	m_column = 0;
	bFirstinit = false;
	memset(&m_stuFirewarningInfo_Get,0,sizeof(NET_FIRE_WARNING_INFO));
	m_stuFirewarningInfo_Get.dwSize = sizeof(NET_FIRE_WARNING_INFO);
	m_FirewarinigModeType = NET_EM_FIREWARNING_TYPE_PTZPRESET;
}

FireWarnSetDlg::~FireWarnSetDlg()
{
}

void FireWarnSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_RCH1, m_check_rch1);
	DDX_Control(pDX, IDC_CHECK_RCH2, m_check_rch2);
	DDX_Control(pDX, IDC_CHECK_SCH1, m_check_sch1);
	DDX_Control(pDX, IDC_CHECK_SCH2, m_check_sch2);
	DDX_Control(pDX, IDC_COMBO_Week, m_combo_week);
	DDX_Control(pDX, IDC_COMBO_TimeSlot, m_combo_timslot);
	DDX_Control(pDX, IDC_CHECK_VisualOverview, m_check_visualoverview);
	DDX_Control(pDX, IDC_CHECK_VisualDetail, m_check_visualdetail);
	DDX_Text(pDX, IDC_EDIT_ThermoSnapTimes, m_strThermoSnapTimes);
	DDX_Control(pDX, IDC_COMBO_PTZDATA, m_combo_ptzdata);
	DDX_Control(pDX, IDC_CHECK_Enable, m_check_enable);
	DDX_Text(pDX, IDC_EDIT_FireDuration, m_strFireDuration);
	DDX_Control(pDX, IDC_COMBO_Region, m_combo_region);
	DDX_Text(pDX, IDC_EDIT_Sensitivity, m_strSensitivity);
	DDX_Control(pDX, IDC_DATETIMEPICKER_S, m_datatimestart);
	DDX_Control(pDX, IDC_DATETIMEPICKER_E, m_datatimeend);
	DDX_Control(pDX, IDC_CHECK_RECORD, m_check_record);
	DDX_Text(pDX, IDC_EDIT_RECORDLATCH, m_strRecordLatch);
	DDX_Control(pDX, IDC_CHECK_ALARMOUT, m_check_alarmout);
	DDX_Control(pDX, IDC_CHECK_MAIL, m_check_mail);
	DDX_Control(pDX, IDC_CHECK_PTZ, m_check_ptz);
	DDX_Control(pDX, IDC_CHECK_SNAP, m_check_snap);

	DDX_Text(pDX, IDC_EDIT_ALARMOUTLATCH, m_stralarmoutlatch);
	DDX_Control(pDX, IDC_LIST_Regions, m_list_regions);
	DDX_Control(pDX, IDC_EDIT_1, m_edit);
	DDX_Control(pDX, IDC_CHECK_Time, m_check_time);
	// 	DDV_MinMaxInt(pDX, m_nRecordLatch, 10, 300);
	// 	DDV_MinMaxInt(pDX, m_alarmoutlatch, 10, 300);
	// 	DDV_MinMaxInt(pDX, m_nSensitivity, 1, 100);
	// 	DDV_MinMaxInt(pDX, m_FireDuration, 0, 32767);
	DDX_Text(pDX, IDC_EDIT_hours, m_strhours);
	DDX_Text(pDX, IDC_EDIT_minutes, m_strminutes);
	DDX_Text(pDX, IDC_EDIT_seconds, m_strseconds);
	DDX_Text(pDX, IDC_EDIT_houre, m_strhoure);
	DDX_Text(pDX, IDC_EDIT_minutee, m_strminutee);
	DDX_Text(pDX, IDC_EDIT_seconde, m_strseconde);
}


BEGIN_MESSAGE_MAP(FireWarnSetDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GETFIRESET, &FireWarnSetDlg::OnBnClickedButtonGetfireset)
	ON_BN_CLICKED(IDC_BUTTON_SETFIRESET, &FireWarnSetDlg::OnBnClickedButtonSetfireset)
	ON_BN_CLICKED(IDC_BUTTON_GETFIREEXT, &FireWarnSetDlg::OnBnClickedButtonGetfireext)
	ON_BN_CLICKED(IDC_BUTTON_SETFIREEXT, &FireWarnSetDlg::OnBnClickedButtonSetfireext)
	ON_BN_CLICKED(IDC_BUTTON_GETFIREMODESET, &FireWarnSetDlg::OnBnClickedButtonGetfiremodeset)
	ON_BN_CLICKED(IDC_BUTTON_SETFIREMODESET, &FireWarnSetDlg::OnBnClickedButtonSetfiremodeset)
	ON_BN_CLICKED(IDC_RADIO_Auto, &FireWarnSetDlg::OnBnClickedRadioAuto)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_Regions, &FireWarnSetDlg::OnNMDblclkListRegions)
	ON_EN_KILLFOCUS(IDC_EDIT_1, &FireWarnSetDlg::OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_RADIO_Manual, &FireWarnSetDlg::OnBnClickedRadioManual)
	ON_CBN_SELCHANGE(IDC_COMBO_PTZDATA, &FireWarnSetDlg::OnCbnSelchangeComboPtzdata)
	ON_CBN_SELCHANGE(IDC_COMBO_Region, &FireWarnSetDlg::OnCbnSelchangeComboRegion)
	ON_CBN_SELCHANGE(IDC_COMBO_Week, &FireWarnSetDlg::OnCbnSelchangeComboWeek)
	ON_CBN_SELCHANGE(IDC_COMBO_TimeSlot, &FireWarnSetDlg::OnCbnSelchangeComboTimeslot)
	ON_EN_CHANGE(IDC_EDIT_hours, &FireWarnSetDlg::OnEnChangeEdithours)
	ON_EN_CHANGE(IDC_EDIT_minutes, &FireWarnSetDlg::OnEnChangeEditminutes)
	ON_EN_CHANGE(IDC_EDIT_seconds, &FireWarnSetDlg::OnEnChangeEditseconds)
	ON_EN_CHANGE(IDC_EDIT_houre, &FireWarnSetDlg::OnEnChangeEdithoure)
	ON_EN_CHANGE(IDC_EDIT_minutee, &FireWarnSetDlg::OnEnChangeEditminutee)
	ON_EN_CHANGE(IDC_EDIT_seconde, &FireWarnSetDlg::OnEnChangeEditseconde)
END_MESSAGE_MAP()


// FireWarnSetDlg 消息处理程序

BOOL FireWarnSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	CEdit* pEdit = (CEdit*)this->GetDlgItem(IDC_EDIT_1);
	pEdit->SetLimitText(18);

	CEdit* pEdithours = (CEdit*)this->GetDlgItem(IDC_EDIT_hours);
	pEdithours->SetLimitText(2);
	CEdit* pEditminutes = (CEdit*)this->GetDlgItem(IDC_EDIT_minutes);
	pEditminutes->SetLimitText(2);
	CEdit* pEditseconds = (CEdit*)this->GetDlgItem(IDC_EDIT_seconds);
	pEditseconds->SetLimitText(2);

	CEdit* pEdithoure = (CEdit*)this->GetDlgItem(IDC_EDIT_houre);
	pEdithoure->SetLimitText(2);
	CEdit* pEditminutee = (CEdit*)this->GetDlgItem(IDC_EDIT_minutee);
	pEditminutee->SetLimitText(2);
	CEdit* pEditseconde = (CEdit*)this->GetDlgItem(IDC_EDIT_seconde);
	pEditseconde->SetLimitText(2);

	GetDlgItem(IDC_EDIT_preset)->EnableWindow(FALSE);
	// TODO:  在此添加额外的初始化
	if (m_nChannelCount > 1)
	{
		m_check_rch1.ShowWindow(SW_SHOW);
		m_check_rch2.ShowWindow(SW_SHOW);
		m_check_sch1.ShowWindow(SW_SHOW);
		m_check_sch2.ShowWindow(SW_SHOW);
	}
	else
	{
		m_check_rch1.ShowWindow(SW_SHOW);
		m_check_rch2.ShowWindow(SW_HIDE);
		m_check_sch1.ShowWindow(SW_SHOW);
		m_check_sch2.ShowWindow(SW_HIDE);
	}

	m_combo_week.AddString(ConvertString("Sun"));
	m_combo_week.AddString(ConvertString("Mon"));
	m_combo_week.AddString(ConvertString("Tue"));
	m_combo_week.AddString(ConvertString("Wed"));
	m_combo_week.AddString(ConvertString("Thu"));
	m_combo_week.AddString(ConvertString("Fri"));
	m_combo_week.AddString(ConvertString("Sat"));
	m_combo_week.SetCurSel(0);

	m_combo_timslot.AddString(ConvertString("period1"));
	m_combo_timslot.AddString(ConvertString("period2"));
	m_combo_timslot.AddString(ConvertString("period3"));
	m_combo_timslot.AddString(ConvertString("period4"));
	m_combo_timslot.AddString(ConvertString("period5"));
	m_combo_timslot.AddString(ConvertString("period6"));
	m_combo_timslot.SetCurSel(0);

	if (m_nChannelCount < 2)
	{
		m_check_rch2.ShowWindow(SW_HIDE);
		m_check_sch2.ShowWindow(SW_HIDE);
	}
	else
	{
		m_check_rch2.ShowWindow(SW_SHOW);
		m_check_sch2.ShowWindow(SW_SHOW);
	}
	

	m_list_regions.SetExtendedStyle
		(m_list_regions.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);	

	m_list_regions.InsertColumn(0,ConvertString("NO"), LVCFMT_LEFT, 50, 0);
	m_list_regions.InsertColumn(1,ConvertString("Detection area mask"), LVCFMT_LEFT, 160, 1);

	m_edit.ShowWindow(SW_HIDE);

	if (m_lLoginID)
	{
		bFirstinit = true;
		OnBnClickedButtonGetfireext();
		OnBnClickedButtonGetfiremodeset();
		bFirstinit = false;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void FireWarnSetDlg::OnBnClickedButtonGetfireset()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETFIRESET)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETFIRESET)->EnableWindow(TRUE);
		return;
	}
	int nChannelID = 1;
	if (m_nChannelCount > 1)
	{
		nChannelID = 1;
	} 
	else
	{
		nChannelID = 0;
	}
	NET_FIRE_WARNING_INFO m_stuFirewarningInfo = {sizeof(m_stuFirewarningInfo)};
	BOOL bRet = CLIENT_GetConfig(m_lLoginID, NET_EM_CFG_FIRE_WARNING, nChannelID, &m_stuFirewarningInfo,sizeof(m_stuFirewarningInfo), 5000);
	if (bRet)
	{
		m_stuFirewarningInfo_Get = m_stuFirewarningInfo;
		//预置点
		if (m_stuFirewarningInfo.nFireWarnRuleNum > 0)
		{
			m_combo_ptzdata.ResetContent();
			if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
			{
				for (int i = 1;i<m_stuFirewarningInfo.nFireWarnRuleNum;i++)
				{
					CString str;
					str.Format("%d",i);
					m_combo_ptzdata.AddString(str);
				}
			}
			else
			{
				for (int i = 0;i<m_stuFirewarningInfo.nFireWarnRuleNum;i++)
				{
					CString str;
					str.Format("%d",i+1);
					m_combo_ptzdata.AddString(str);
				}
			}
			
			m_combo_ptzdata.SetCurSel(0);

			NET_FIREWARN_RULE_INFO stuFirewarnrule;
			if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
			{
				stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[1];
			}
			else
			{
				stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[0];
			}

			//预置点编号
			CString strPresetId;
			strPresetId.Format("%d",stuFirewarnrule.nPresetId);
			GetDlgItem(IDC_EDIT_preset)->SetWindowText(strPresetId);

			//启用
			m_check_enable.SetCheck(stuFirewarnrule.bEnable);

			//模式 抗干扰  高响应
			if (stuFirewarnrule.emFireWarningDetectMode == NET_EM_FIREWARNING_DETECTMODE_TYPE_AUTO)
			{
				((CButton*)GetDlgItem(IDC_RADIO_antijamming))->SetCheck(TRUE);
				((CButton*)GetDlgItem(IDC_RADIO_highresponse))->SetCheck(FALSE);
			} 
			else
			{
				((CButton*)GetDlgItem(IDC_RADIO_antijamming))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_highresponse))->SetCheck(TRUE);
			}

			//火点等待  自动 手动
			if (stuFirewarnrule.bTimeDurationEnable)
			{
				((CButton*)GetDlgItem(IDC_RADIO_Auto))->SetCheck(TRUE);
				((CButton*)GetDlgItem(IDC_RADIO_Manual))->SetCheck(FALSE);
				GetDlgItem(IDC_EDIT_FireDuration)->EnableWindow(TRUE);
			} 
			else
			{
				((CButton*)GetDlgItem(IDC_RADIO_Auto))->SetCheck(FALSE);
				((CButton*)GetDlgItem(IDC_RADIO_Manual))->SetCheck(TRUE);
				GetDlgItem(IDC_EDIT_FireDuration)->EnableWindow(FALSE);
			}

			//火情持续时间
			m_strFireDuration.Format("%d",stuFirewarnrule.nFireDuration);
			// ** 区域
			if (stuFirewarnrule.nDetectWindowNum > 0)
			{
				m_combo_region.ResetContent();
				for (int i = 0;i<stuFirewarnrule.nDetectWindowNum;i++)
				{
					CString str;
					str.Format("%d",i+1);
					m_combo_region.AddString(str);
				}
				m_combo_region.SetCurSel(0);

				//m_strname = "1";
				m_strSensitivity.Format("%d", stuFirewarnrule.stuDetectWnd[0].nSensitivity);

				m_list_regions.DeleteAllItems();
				for (int i=0;i<MAX_FIREWARNING_DETECTRGN_NUM;i++)
				{
					CString str;
					str.Format("%d",i);
					char buff[65];
					_i64toa(stuFirewarnrule.stuDetectWnd[0].nRegions[i],buff,10);
					CString strnRegions = buff;
					m_list_regions.InsertItem(i, str);
					m_list_regions.SetItemText(i, 1, strnRegions);
				}
				//m_nRgnNum = stuFirewarnrule.stuDetectWnd[0].nRgnNum;
				//m_nTargetSize = stuFirewarnrule.stuDetectWnd[0].nTargetSize;
			}

			NET_FIREWARN_EVENTHANDLE_INFO stuEventhandle = stuFirewarnrule.stuEventHandler;

			//报警设置
			m_combo_week.SetCurSel(0);
			m_combo_timslot.SetCurSel(0);
// 			SYSTEMTIME pTimeBegin = {0};
// 			GetLocalTime(&pTimeBegin);
// 			pTimeBegin.wHour = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginHour;
// 			pTimeBegin.wMinute = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginMin;
// 			pTimeBegin.wSecond = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginSec;
// 			SYSTEMTIME pTimeEnd = {0};
// 			GetLocalTime(&pTimeEnd);
// 			pTimeEnd.wHour = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndHour;
// 			pTimeEnd.wMinute = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndMin;
// 			pTimeEnd.wSecond = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndSec;
// 			m_datatimestart.SetTime(&pTimeBegin);
// 			m_datatimeend.SetTime(&pTimeEnd);
			m_strhours.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginHour);
			m_strminutes.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginMin);
			m_strseconds.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginSec);
			m_strhoure.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndHour);
			m_strminutee.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndMin);
			m_strseconde.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndSec);

			m_check_time.SetCheck(stuEventhandle.stuTimeSection.stuTimeSection[0][0].bEnable);

			m_check_record.SetCheck(stuEventhandle.bRecordEnable);
			m_strRecordLatch.Format("%d", stuEventhandle.nRecordLatch);
			m_check_rch1.SetCheck(FALSE);
			m_check_rch2.SetCheck(FALSE);
			for (int i=0;i<stuEventhandle.nRecordChannelNum;i++)
			{
				if (stuEventhandle.nRecordChannels[i] == 0)
				{
					m_check_rch1.SetCheck(TRUE);
				}
				else if (stuEventhandle.nRecordChannels[i] == 1)
				{
					m_check_rch2.SetCheck(TRUE);
				}
			}

			m_check_alarmout.SetCheck(stuEventhandle.bAlarmOutEnable);
			m_stralarmoutlatch.Format("%d",stuEventhandle.nAlarmOutLatch);
			m_check_mail.SetCheck(stuEventhandle.bMailEnable);
			m_check_ptz.SetCheck(stuEventhandle.bPtzLinkEnable);
			m_check_snap.SetCheck(stuEventhandle.bSnapshotEnable);

			m_check_sch1.SetCheck(FALSE);
			m_check_sch2.SetCheck(FALSE);
			for (int i=0;i<stuEventhandle.nSnapshotChannelNum;i++)
			{
				if (stuEventhandle.nSnapshotChannels[i] == 0)
				{
					m_check_sch1.SetCheck(TRUE);
				}
				else if (stuEventhandle.nSnapshotChannels[i] == 1)
				{
					m_check_sch2.SetCheck(TRUE);
				}
			}
		}
		UpdateData(FALSE);
		if (!bFirstinit)
		{
			//MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
		}
	}
	else
	{
		if (!bFirstinit)
		{
			MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_GETFIRESET)->EnableWindow(TRUE);
}

void FireWarnSetDlg::OnBnClickedButtonSetfireset()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETFIRESET)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETFIRESET)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETFIRESET)->EnableWindow(TRUE);
		return;
	}

	int nChannelID = 1;
	if (m_nChannelCount > 1)
	{
		nChannelID = 1;
	} 
	else
	{
		nChannelID = 0;
	}

	BOOL bMode = ((CButton*)GetDlgItem(IDC_RADIO_PtzPreset))->GetCheck();
	if (m_stuFirewarningInfo_Get.nFireWarnRuleNum <= 1 && bMode)
	{
		MessageBox(ConvertString("The device configuration is lost. Please configure it on the web first.Or assemble configuration according to preset point."), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETFIRESET)->EnableWindow(TRUE);
		return;
	}

	NET_FIRE_WARNING_INFO m_stuFirewarningInfo = {sizeof(m_stuFirewarningInfo)};
	m_stuFirewarningInfo = m_stuFirewarningInfo_Get;

	NET_FIREWARN_RULE_INFO stuFirewarnrule = {0};
	if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
	{
		int nptzdata = m_combo_ptzdata.GetCurSel();
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nptzdata+1];
	} 
	else
	{
		int nptzdata = m_combo_ptzdata.GetCurSel();
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nptzdata];
	}
	
	stuFirewarnrule.bEnable = m_check_enable.GetCheck();//使能

	if (((CButton*)GetDlgItem(IDC_RADIO_antijamming))->GetCheck())//模式 抗干扰  高响应
	{
		stuFirewarnrule.emFireWarningDetectMode = NET_EM_FIREWARNING_DETECTMODE_TYPE_AUTO;
	}
	else
	{
		stuFirewarnrule.emFireWarningDetectMode = NET_EM_FIREWARNING_DETECTMODE_TYPE_NORMAL;
	}

	if (((CButton*)GetDlgItem(IDC_RADIO_Auto))->GetCheck())//火点等待  自动 手动
	{
		stuFirewarnrule.bTimeDurationEnable = TRUE;
	}
	else
	{
		stuFirewarnrule.bTimeDurationEnable = FALSE;
	}

	//火情持续时间
	stuFirewarnrule.nFireDuration = atoi(m_strFireDuration);

	//获取当前窗口G
	int nregion = m_combo_region.GetCurSel();
	NET_FIREWARN_DETECTWND_INFO	studetectwnd = stuFirewarnrule.stuDetectWnd[nregion];
	studetectwnd.nSensitivity = atoi(m_strSensitivity);
	for (int i=0;i<MAX_FIREWARNING_DETECTRGN_NUM;i++)
	{
		CString str = m_list_regions.GetItemText(i,1);
		studetectwnd.nRegions[i] = _atoi64(str);
	}
	stuFirewarnrule.stuDetectWnd[nregion] = studetectwnd;

	NET_FIREWARN_EVENTHANDLE_INFO	stueventhandler = stuFirewarnrule.stuEventHandler;	
	//联动
	int nweek = m_combo_week.GetCurSel();
	int ntimslot = m_combo_timslot.GetCurSel();
// 	SYSTEMTIME pTimeBegin = {0};
// 	GetLocalTime(&pTimeBegin);
// 	m_datatimestart.GetTime(&pTimeBegin);
// 	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginHour = pTimeBegin.wHour;
// 	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginMin = pTimeBegin.wMinute;
// 	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginSec = pTimeBegin.wSecond;
// 	SYSTEMTIME pTimeEnd = {0};
// 	GetLocalTime(&pTimeEnd);
// 	m_datatimeend.GetTime(&pTimeEnd);
// 	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndHour = pTimeEnd.wHour;
// 	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndMin = pTimeEnd.wMinute;
// 	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndSec = pTimeEnd.wSecond;

	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginHour = atoi(m_strhours);
	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginMin = atoi(m_strminutes);
	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginSec = atoi(m_strseconds);
	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndHour = atoi(m_strhoure);
	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndMin = atoi(m_strminutee);
	stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndSec = atoi(m_strseconde);

	if (m_check_time.GetCheck() == 1)
	{
		stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].bEnable = 1;
	} 
	else
	{
		stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].bEnable = 0;
	}
	//stueventhandler.stuTimeSection.stuTimeSection[nweek][ntimslot].bEnable = m_check_time.GetCheck();

	//录像
	stueventhandler.bRecordEnable = m_check_record.GetCheck();
	stueventhandler.nRecordLatch =atoi(m_strRecordLatch);
	////
	stueventhandler.nRecordChannelNum = 0;
	if (m_check_rch1.GetCheck())
	{
		stueventhandler.nRecordChannels[stueventhandler.nRecordChannelNum] = 0;
		stueventhandler.nRecordChannelNum++;
	}
	if (m_check_rch2.GetCheck())
	{
		stueventhandler.nRecordChannels[stueventhandler.nRecordChannelNum] = 1;
		stueventhandler.nRecordChannelNum++;
	}

	//报警输出
	stueventhandler.bAlarmOutEnable = m_check_alarmout.GetCheck();
	stueventhandler.nAlarmOutLatch = atoi(m_stralarmoutlatch);
	//发送邮件
	stueventhandler.bMailEnable = m_check_mail.GetCheck();
	//云台
	stueventhandler.bPtzLinkEnable = m_check_ptz.GetCheck();
	//抓图
	stueventhandler.bSnapshotEnable = m_check_snap.GetCheck();

	stueventhandler.nSnapshotChannelNum = 0;
	if (m_check_sch1.GetCheck())
	{
		stueventhandler.nSnapshotChannels[stueventhandler.nSnapshotChannelNum] = 0;
		stueventhandler.nSnapshotChannelNum++;
	}
	if (m_check_sch2.GetCheck())
	{
		stueventhandler.nSnapshotChannels[stueventhandler.nSnapshotChannelNum] = 1;
		stueventhandler.nSnapshotChannelNum++;
	}

	stuFirewarnrule.stuEventHandler = stueventhandler;

	if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
	{
		int nptzdata = m_combo_ptzdata.GetCurSel();
		m_stuFirewarningInfo.stuFireWarnRule[nptzdata+1] = stuFirewarnrule;
	} 
	else
	{
		int nptzdata = m_combo_ptzdata.GetCurSel();
		m_stuFirewarningInfo.stuFireWarnRule[nptzdata] = stuFirewarnrule;
	}

	BOOL bRet = CLIENT_SetConfig(m_lLoginID, NET_EM_CFG_FIRE_WARNING, nChannelID, &m_stuFirewarningInfo,sizeof(m_stuFirewarningInfo), 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		m_stuFirewarningInfo_Get = m_stuFirewarningInfo;
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_SETFIRESET)->EnableWindow(TRUE);
}

void FireWarnSetDlg::OnBnClickedButtonGetfiremodeset()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETFIREMODESET)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETFIREMODESET)->EnableWindow(TRUE);
		return;
	}
	NET_FIREWARNING_MODE_INFO m_stuFirewarningmodeInfo = {sizeof(m_stuFirewarningmodeInfo)};
	int nChannelID = 1;
	if (m_nChannelCount > 1)
	{
		nChannelID = 1;
	} 
	else
	{
		nChannelID = 0;
	}
	BOOL bRet = CLIENT_GetConfig(m_lLoginID, NET_EM_CFG_FIRE_WARNINGMODE, nChannelID, &m_stuFirewarningmodeInfo,sizeof(m_stuFirewarningmodeInfo), 5000);
	if (bRet)
	{
		m_FirewarinigModeType = m_stuFirewarningmodeInfo.emFireWarningMode;
		if (m_stuFirewarningmodeInfo.emFireWarningMode == NET_EM_FIREWARNING_TYPE_PTZPRESET)
		{
			((CButton*)GetDlgItem(IDC_RADIO_PtzPreset))->SetCheck(TRUE);
			((CButton*)GetDlgItem(IDC_RADIO_SpaceExClude))->SetCheck(FALSE);
		} 
		else
		{
			((CButton*)GetDlgItem(IDC_RADIO_PtzPreset))->SetCheck(FALSE);
			((CButton*)GetDlgItem(IDC_RADIO_SpaceExClude))->SetCheck(TRUE);
		}
		UpdateData(FALSE);
		OnBnClickedButtonGetfireset();
// 		if (!bFirstinit)
// 		{
// 			MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
// 		}
	}
	else
	{
		if (!bFirstinit)
		{
			MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_GETFIREMODESET)->EnableWindow(TRUE);
}

void FireWarnSetDlg::OnBnClickedButtonSetfiremodeset()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETFIREMODESET)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETFIREMODESET)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETFIREMODESET)->EnableWindow(TRUE);
		return;
	}

	int nChannelID = 1;
	if (m_nChannelCount > 1)
	{
		nChannelID = 1;
	} 
	else
	{
		nChannelID = 0;
	}

	NET_FIREWARNING_MODE_INFO m_stuFirewarningmodeInfo = {sizeof(m_stuFirewarningmodeInfo)};

	BOOL bMode = ((CButton*)GetDlgItem(IDC_RADIO_PtzPreset))->GetCheck();
	if (bMode)
	{
		m_stuFirewarningmodeInfo.emFireWarningMode = NET_EM_FIREWARNING_TYPE_PTZPRESET;
	} 
	else
	{
		m_stuFirewarningmodeInfo.emFireWarningMode = NET_EM_FIREWARNING_TYPE_SPACEEXCLUDE;
	}

	BOOL bRet = CLIENT_SetConfig(m_lLoginID, NET_EM_CFG_FIRE_WARNINGMODE,nChannelID,&m_stuFirewarningmodeInfo,sizeof(m_stuFirewarningmodeInfo), 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		m_FirewarinigModeType = m_stuFirewarningmodeInfo.emFireWarningMode;
		OnBnClickedButtonGetfireset();
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_SETFIREMODESET)->EnableWindow(TRUE);
}


void FireWarnSetDlg::OnBnClickedButtonGetfireext()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETFIREEXT)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETFIREEXT)->EnableWindow(TRUE);
		return;
	}
	char buffData[100 * 1024] = {0};
	int n;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_FIRE_WARNING_EXT, -1, buffData, 100 * 1024, &n, 5000);
	if (bRet)
	{
		CFG_FIREWARNING_EXT_INFO st = {0};
		bRet = CLIENT_ParseData(CFG_CMD_FIRE_WARNING_EXT, buffData, (char*)&st, sizeof(CFG_FIREWARNING_EXT_INFO), &n);
		if (bRet)
		{
			m_check_visualoverview.SetCheck(st.bVisualOverviewEnable);
			m_check_visualdetail.SetCheck(st.bVisualDetailEnable);
			m_strThermoSnapTimes.Format("%d", st.nThermoSnapTimes);
			UpdateData(FALSE);
			if (!bFirstinit)
			{
				//MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
			}
		}
		else
		{
			if (!bFirstinit)
			{
				MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
			}
		}
	}
	else
	{
		if (!bFirstinit)
		{
			MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_GETFIREEXT)->EnableWindow(TRUE);
}

void FireWarnSetDlg::OnBnClickedButtonSetfireext()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETFIREEXT)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETFIREEXT)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETFIREEXT)->EnableWindow(TRUE);
		return;
	}

	char buffData[100 * 1024] = {0};

	int n;
 	CFG_FIREWARNING_EXT_INFO st = {0};
	st.bVisualOverviewEnable = m_check_visualoverview.GetCheck();
	st.bVisualDetailEnable = m_check_visualdetail.GetCheck();
	st.nThermoSnapTimes = atoi(m_strThermoSnapTimes);

	BOOL bRet = CLIENT_PacketData(CFG_CMD_FIRE_WARNING_EXT, &st, sizeof(st), buffData, 100 * 1024);
	if (bRet)
	{
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_FIRE_WARNING_EXT, -1, buffData, 100*1024, NULL, NULL, 5000);
		if (!bRet)
		{
			MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		}
		else
		{
			//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
		}
	}
	else
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_SETFIREEXT)->EnableWindow(TRUE);
}

void FireWarnSetDlg::OnNMDblclkListRegions(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	NM_LISTVIEW * pNMListView = (NM_LISTVIEW*)pNMHDR;
	CRect rc;
	 m_row = pNMListView->iItem;
	 m_column = pNMListView->iSubItem;
	 if (m_row < 0)
	 {
		 return;
	 }
	if (pNMListView->iSubItem != 0)
	{
		m_list_regions.GetSubItemRect(m_row,m_column,LVIR_LABEL,rc);
		m_edit.SetParent(&m_list_regions);
		m_edit.MoveWindow(rc);
		m_edit.SetWindowText(m_list_regions.GetItemText(m_row,m_column));
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetFocus();
		m_edit.ShowCaret();
		m_edit.SetSel(-1);
	}
}

void FireWarnSetDlg::OnEnKillfocusEdit1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	m_edit.GetWindowText(str);
	m_list_regions.SetItemText(m_row,m_column,str);
	m_edit.ShowWindow(SW_HIDE);
}

void FireWarnSetDlg::OnBnClickedRadioAuto()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_EDIT_FireDuration)->EnableWindow(TRUE);
}

void FireWarnSetDlg::OnBnClickedRadioManual()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_EDIT_FireDuration)->EnableWindow(FALSE);
}

void FireWarnSetDlg::OnCbnSelchangeComboPtzdata()
{
	// TODO: 在此添加控件通知处理程序代码
	//预置点
	NET_FIRE_WARNING_INFO m_stuFirewarningInfo = m_stuFirewarningInfo_Get;
	int nPtzdata = m_combo_ptzdata.GetCurSel();
	//预置点
	NET_FIREWARN_RULE_INFO stuFirewarnrule;

	if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
	{
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nPtzdata+1];
	} 
	else
	{
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nPtzdata];
	}

	//预置点编号
	CString strPresetId;
	strPresetId.Format("%d",stuFirewarnrule.nPresetId);
	GetDlgItem(IDC_EDIT_preset)->SetWindowText(strPresetId);

	//启用
	m_check_enable.SetCheck(stuFirewarnrule.bEnable);

	//模式 抗干扰  高响应
	if (stuFirewarnrule.emFireWarningDetectMode == NET_EM_FIREWARNING_DETECTMODE_TYPE_AUTO)
	{
		((CButton*)GetDlgItem(IDC_RADIO_antijamming))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_RADIO_highresponse))->SetCheck(FALSE);
	} 
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_antijamming))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_highresponse))->SetCheck(TRUE);
	}

	//火点等待  自动 手动
	if (stuFirewarnrule.bTimeDurationEnable)
	{
		((CButton*)GetDlgItem(IDC_RADIO_Auto))->SetCheck(TRUE);
		((CButton*)GetDlgItem(IDC_RADIO_Manual))->SetCheck(FALSE);
		GetDlgItem(IDC_EDIT_FireDuration)->EnableWindow(TRUE);
	} 
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_Auto))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_RADIO_Manual))->SetCheck(TRUE);
		GetDlgItem(IDC_EDIT_FireDuration)->EnableWindow(FALSE);
	}

	//火情持续时间
	m_strFireDuration.Format("%d",stuFirewarnrule.nFireDuration);
	// ** 区域
	if (stuFirewarnrule.nDetectWindowNum > 0)
	{
		m_combo_region.ResetContent();
		for (int i = 0;i<stuFirewarnrule.nDetectWindowNum;i++)
		{
			CString str;
			str.Format("%d",i+1);
			m_combo_region.AddString(str);
		}
		m_combo_region.SetCurSel(0);

		//m_strname = "1";
		m_strSensitivity.Format("%d", stuFirewarnrule.stuDetectWnd[0].nSensitivity);

		m_list_regions.DeleteAllItems();
		for (int i=0;i<MAX_FIREWARNING_DETECTRGN_NUM;i++)
		{
			CString str;
			str.Format("%d",i);
			char buff[65];
			_i64toa(stuFirewarnrule.stuDetectWnd[0].nRegions[i],buff,10);
			CString strnRegions = buff;
			m_list_regions.InsertItem(i, str);
			m_list_regions.SetItemText(i, 1, strnRegions);
		}
		//m_nRgnNum = stuFirewarnrule.stuDetectWnd[0].nRgnNum;
		//m_nTargetSize = stuFirewarnrule.stuDetectWnd[0].nTargetSize;
	}

	NET_FIREWARN_EVENTHANDLE_INFO stuEventhandle = stuFirewarnrule.stuEventHandler;

	//报警设置
	m_combo_week.SetCurSel(0);
	m_combo_timslot.SetCurSel(0);
// 	SYSTEMTIME pTimeBegin = {0};
// 	GetLocalTime(&pTimeBegin);
// 	pTimeBegin.wHour = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginHour;
// 	pTimeBegin.wMinute = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginMin;
// 	pTimeBegin.wSecond = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginSec;
// 	SYSTEMTIME pTimeEnd = {0};
// 	GetLocalTime(&pTimeEnd);
// 	pTimeEnd.wHour = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndHour;
// 	pTimeEnd.wMinute = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndMin;
// 	pTimeEnd.wSecond = stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndSec;
// 	m_datatimestart.SetTime(&pTimeBegin);
// 	m_datatimeend.SetTime(&pTimeEnd);
	m_strhours.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginHour);
	m_strminutes.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginMin);
	m_strseconds.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iBeginSec);
	m_strhoure.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndHour);
	m_strminutee.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndMin);
	m_strseconde.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[0][0].iEndSec);

	m_check_time.SetCheck(stuEventhandle.stuTimeSection.stuTimeSection[0][0].bEnable);

	m_check_record.SetCheck(stuEventhandle.bRecordEnable);
	m_strRecordLatch.Format("%d",stuEventhandle.nRecordLatch);
	m_check_rch1.SetCheck(FALSE);
	m_check_rch2.SetCheck(FALSE);
	for (int i=0;i<stuEventhandle.nRecordChannelNum;i++)
	{
		if (stuEventhandle.nRecordChannels[i] == 0)
		{
			m_check_rch1.SetCheck(TRUE);
		}
		else if (stuEventhandle.nRecordChannels[i] == 1)
		{
			m_check_rch2.SetCheck(TRUE);
		}
	}
// 	if (stuEventhandle.nRecordChannels[0] == 1)
// 	{
// 		m_check_rch1.SetCheck(TRUE);
// 	}
// 	else
// 	{
// 		m_check_rch1.SetCheck(FALSE);
// 	}
// 	if (stuEventhandle.nRecordChannels[1] == 1)
// 	{
// 		m_check_rch2.SetCheck(TRUE);
// 	}
// 	else
// 	{
// 		m_check_rch2.SetCheck(FALSE);
// 	}
	m_check_alarmout.SetCheck(stuEventhandle.bAlarmOutEnable);
	m_stralarmoutlatch.Format("%d",stuEventhandle.nAlarmOutLatch);
	m_check_mail.SetCheck(stuEventhandle.bMailEnable);
	m_check_ptz.SetCheck(stuEventhandle.bPtzLinkEnable);
	m_check_snap.SetCheck(stuEventhandle.bSnapshotEnable);
	m_check_sch1.SetCheck(FALSE);
	m_check_sch2.SetCheck(FALSE);
	for (int i=0;i<stuEventhandle.nSnapshotChannelNum;i++)
	{
		if (stuEventhandle.nSnapshotChannels[i] == 0)
		{
			m_check_sch1.SetCheck(TRUE);
		}
		else if (stuEventhandle.nSnapshotChannels[i] == 1)
		{
			m_check_sch2.SetCheck(TRUE);
		}
	}
// 	if (stuEventhandle.nSnapshotChannels[0] == 1)
// 	{
// 		m_check_sch1.SetCheck(TRUE);
// 	}
// 	else
// 	{
// 		m_check_sch1.SetCheck(FALSE);
// 	}
// 	if (stuEventhandle.nSnapshotChannels[1] == 1)
// 	{
// 		m_check_sch2.SetCheck(TRUE);
// 	}
// 	else
// 	{
// 		m_check_sch2.SetCheck(FALSE);
// 	}

	UpdateData(FALSE);
}

void FireWarnSetDlg::OnCbnSelchangeComboRegion()
{
	// TODO: 在此添加控件通知处理程序代码
	//预置点
	NET_FIRE_WARNING_INFO m_stuFirewarningInfo = m_stuFirewarningInfo_Get;
	int nPtzdata = m_combo_ptzdata.GetCurSel();
	//预置点
	NET_FIREWARN_RULE_INFO stuFirewarnrule;

	if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
	{
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nPtzdata+1];
	} 
	else
	{
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nPtzdata];
	}

	// ** 区域

	m_strSensitivity.Format("%d",stuFirewarnrule.stuDetectWnd[m_combo_region.GetCurSel()].nSensitivity);

	m_list_regions.DeleteAllItems();
	for (int i=0;i<MAX_FIREWARNING_DETECTRGN_NUM;i++)
	{
		CString str;
		str.Format("%d",i);
		char buff[65];
		_i64toa(stuFirewarnrule.stuDetectWnd[m_combo_region.GetCurSel()].nRegions[i],buff,10);
		CString strnRegions = buff;
		m_list_regions.InsertItem(i, str);
		m_list_regions.SetItemText(i, 1, strnRegions);
	}
	UpdateData(FALSE);
}

void FireWarnSetDlg::OnCbnSelchangeComboWeek()
{
	// TODO: 在此添加控件通知处理程序代码
	GetTimefromStu();
}

void FireWarnSetDlg::OnCbnSelchangeComboTimeslot()
{
	// TODO: 在此添加控件通知处理程序代码
	GetTimefromStu();
}

void FireWarnSetDlg::GetTimefromStu()
{
	NET_FIRE_WARNING_INFO m_stuFirewarningInfo = m_stuFirewarningInfo_Get;
	int nPtzdata = m_combo_ptzdata.GetCurSel();
	//预置点
	NET_FIREWARN_RULE_INFO stuFirewarnrule;

	if (m_stuFirewarningInfo.nFireWarnRuleNum > 1 && m_FirewarinigModeType == NET_EM_FIREWARNING_TYPE_PTZPRESET)
	{
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nPtzdata+1];
	} 
	else
	{
		stuFirewarnrule = m_stuFirewarningInfo.stuFireWarnRule[nPtzdata];
	}

	NET_FIREWARN_EVENTHANDLE_INFO stuEventhandle = stuFirewarnrule.stuEventHandler;

	//报警设置
	//联动
	int nweek = m_combo_week.GetCurSel();
	int ntimslot = m_combo_timslot.GetCurSel();
// 	SYSTEMTIME pTimeBegin = {0};
// 	GetLocalTime(&pTimeBegin);
// 	pTimeBegin.wHour = stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginHour;
// 	pTimeBegin.wMinute = stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginMin;
// 	pTimeBegin.wSecond = stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginSec;
// 	SYSTEMTIME pTimeEnd = {0};
// 	GetLocalTime(&pTimeEnd);
// 	pTimeEnd.wHour = stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndHour;
// 	pTimeEnd.wMinute = stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndMin;
// 	pTimeEnd.wSecond = stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndSec;
// 	m_datatimestart.SetTime(&pTimeBegin);
// 	m_datatimeend.SetTime(&pTimeEnd);

	m_strhours.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginHour);
	m_strminutes.Format("%d",stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginMin);
	m_strseconds.Format("%d", stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iBeginSec);
	m_strhoure.Format("%d", stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndHour);
	m_strminutee.Format("%d", stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndMin);
	m_strseconde.Format("%d", stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].iEndSec);

	m_check_time.SetCheck(stuEventhandle.stuTimeSection.stuTimeSection[nweek][ntimslot].bEnable);
	UpdateData(FALSE);
}

void FireWarnSetDlg::OnEnChangeEdithours()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	if (atoi(m_strhours) > 23)
	{
		m_strhours.Format("%d", 23);
	}
	if (atoi(m_strhours) < 0)
	{
		m_strhours.Format("%d", 0);
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void FireWarnSetDlg::OnEnChangeEditminutes()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	if (atoi(m_strminutes) > 59)
	{
		m_strminutes.Format("%d", 59);
	}
	if (atoi(m_strminutes) < 0)
	{
		m_strminutes.Format("%d", 0);
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void FireWarnSetDlg::OnEnChangeEditseconds()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	if (atoi(m_strseconds) > 59)
	{
		m_strminutes.Format("%d", 59);
	}
	if (atoi(m_strseconds) < 0)
	{
		m_strseconds.Format("%d", 0);
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void FireWarnSetDlg::OnEnChangeEdithoure()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	if (atoi(m_strhoure) > 23)
	{
		m_strhoure.Format("%d", 23);
	}
	if (atoi(m_strhoure) < 0)
	{
		m_strhoure.Format("%d", 0);
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void FireWarnSetDlg::OnEnChangeEditminutee()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	if (atoi(m_strminutee) > 59)
	{
		m_strminutee.Format("%d", 59);
	}
	if (atoi(m_strminutee) < 0)
	{
		m_strminutee.Format("%d", 0);
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}

void FireWarnSetDlg::OnEnChangeEditseconde()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	if (atoi(m_strseconde) > 59)
	{
		m_strseconde.Format("%d", 59);
	}
	if (atoi(m_strseconde) < 0)
	{
		m_strseconde.Format("%d", 0);
	}
	UpdateData(FALSE);
	// TODO:  在此添加控件通知处理程序代码
}
