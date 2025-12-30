// PTZDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "PTZDlg.h"
#include "ThermalCameraDlg.h"

// PTZDlg 对话框

IMPLEMENT_DYNAMIC(PTZDlg, CDialog)

PTZDlg::PTZDlg(CWnd* pParent /*=NULL*/, LLONG lLoginId/* = 0*/, int nChannelCount)
	: CDialog(PTZDlg::IDD, pParent)
	, m_group_name(_T(""))
	, m_strgroup_speed(_T("0"))
	, m_strgroup_interval(_T("0"))
{
	 m_lLoginID = lLoginId;
	 m_nChannelCount = nChannelCount;
	 bFirstinit = false;

	 memset(&stuOutGetPanGroupParam,0,sizeof(NET_OUT_GET_PAN_GROUP_PARAM));
	 stuOutGetPanGroupParam.dwSize = sizeof(NET_OUT_GET_PAN_GROUP_PARAM);
}

PTZDlg::~PTZDlg()
{
}

void PTZDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CH1, m_combo_ch1);
	DDX_Control(pDX, IDC_COMBO_CH2, m_combo_ch2);
	DDX_Control(pDX, IDC_COMBO_CH3, m_combo_ch3);
	DDX_Control(pDX, IDC_COMBO_LimitMode, m_combo_limitmode);
	DDX_Control(pDX, IDC_COMBO_SetLimitMode, m_combo_setlimitmode);
	DDX_Control(pDX, IDC_CHECK_LimitEnable, m_check_limit);
	DDX_Control(pDX, IDC_COMBO_Group, m_combo_group);
	DDX_Text(pDX, IDC_EDIT_PanGroupName, m_group_name);
	DDX_Text(pDX, IDC_EDIT_PanGroupSpeed, m_strgroup_speed);
	DDX_Control(pDX, IDC_CHECK_PanGroupEnable, m_check_groupenable);
	DDX_Text(pDX, IDC_EDIT_PanGroupInterval, m_strgroup_interval);
	DDX_Control(pDX, IDC_COMBO_Index, m_combo_index);
	DDX_Control(pDX, IDC_EDIT_PanGroupName, m_ctrlname);
	DDX_Control(pDX, IDC_COMBO_Index2, m_combo_index2);
	DDX_Control(pDX, IDC_COMBO_CH4, m_combo_ch4);
	//DDV_MinMaxInt(pDX, m_group_speed, 0, 7);
}

BEGIN_MESSAGE_MAP(PTZDlg, CDialog)

	ON_BN_CLICKED(IDC_BUTTON_SetPanGroupScan, &PTZDlg::OnBnClickedButtonSetpangroupscan)
	ON_BN_CLICKED(IDC_BUTTON_GetPanGroupScan, &PTZDlg::OnBnClickedButtonGetpangroupscan)
	ON_BN_CLICKED(IDC_BUTTON_SetPanGroup, &PTZDlg::OnBnClickedButtonSetpangroup)
	ON_BN_CLICKED(IDC_BUTTON_GetPanGroup, &PTZDlg::OnBnClickedButtonGetpangroup)
	ON_BN_CLICKED(IDC_BUTTON_gotoPanPos, &PTZDlg::OnBnClickedButtongotopanpos)
	ON_BN_CLICKED(IDC_BUTTON_SetPanGroupLimit, &PTZDlg::OnBnClickedButtonSetpangrouplimit)
	ON_BN_CLICKED(IDC_BUTTON_pause, &PTZDlg::OnBnClickedButtonpause)
	ON_BN_CLICKED(IDC_BUTTON_resume, &PTZDlg::OnBnClickedButtonresume)
	ON_CBN_SELCHANGE(IDC_COMBO_Group, &PTZDlg::OnCbnSelchangeComboGroup)

END_MESSAGE_MAP()


// PTZDlg 消息处理程序

BOOL PTZDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	CEdit* pEdit = (CEdit*)this->GetDlgItem(IDC_EDIT_PanGroupName);
	pEdit->SetLimitText(32);
	
	// TODO:  在此添加额外的初始化
	for (int i = 0 ;i<m_nChannelCount;i++)
	{
		CString str;
		str.Format("%d",i);
		m_combo_ch1.AddString(str);
 		m_combo_ch2.AddString(str);
 		m_combo_ch3.AddString(str);
		m_combo_ch4.AddString(str);
	}
	if (m_nChannelCount>0)
	{
		m_combo_ch1.SetCurSel(0);
		m_combo_ch2.SetCurSel(m_nChannelCount-1);
		m_combo_ch3.SetCurSel(0);
		m_combo_ch4.SetCurSel(0);
	}


	m_combo_limitmode.AddString(ConvertString("Enable Left and Right Limit"));
	m_combo_limitmode.AddString(ConvertString("Enable Area Scan"));
	m_combo_limitmode.SetCurSel(0);

	m_combo_setlimitmode.AddString(ConvertString("Left"));
	m_combo_setlimitmode.AddString(ConvertString("Right"));
	m_combo_setlimitmode.AddString(ConvertString("Up"));
	m_combo_setlimitmode.AddString(ConvertString("Down"));
	m_combo_setlimitmode.SetCurSel(0);

	m_combo_index.AddString("1");
	m_combo_index.AddString("2");
	m_combo_index.AddString("3");
	m_combo_index.AddString("4");
	m_combo_index.AddString("5");
	m_combo_index.AddString("6");
	m_combo_index.AddString("7");
	m_combo_index.AddString("8");
	m_combo_index.SetCurSel(0);

	m_combo_index2.AddString("1");
	m_combo_index2.AddString("2");
	m_combo_index2.AddString("3");
	m_combo_index2.AddString("4");
	m_combo_index2.AddString("5");
	m_combo_index2.AddString("6");
	m_combo_index2.AddString("7");
	m_combo_index2.AddString("8");
	m_combo_index2.SetCurSel(0);
	
	if (m_lLoginID)
	{
		bFirstinit = true;
		OnBnClickedButtonGetpangroupscan();
		OnBnClickedButtonGetpangroup();
		bFirstinit = false;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



void PTZDlg::OnBnClickedButtonSetpangroupscan()//水平旋转组配置
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SetPanGroupScan)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SetPanGroupScan)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SetPanGroupScan)->EnableWindow(TRUE);
		return;
	}

	int nChannelID = m_combo_ch1.GetCurSel();

	NET_CFG_HORIZONTAL_ROTATION_GROUP_SCAN_INFO m_stuPanGroupScan = {sizeof(m_stuPanGroupScan)};
	m_stuPanGroupScan.stuHorizontalRotationInfo.bLimitEnable = m_check_limit.GetCheck();
	m_stuPanGroupScan.stuHorizontalRotationInfo.nLimitMode = m_combo_limitmode.GetCurSel();
	BOOL bRet = CLIENT_SetConfig(m_lLoginID, NET_EM_CFG_PTZ_HORIZONTAL_ROTATION_GROUP_SCAN, nChannelID, &m_stuPanGroupScan,sizeof(m_stuPanGroupScan), 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_SetPanGroupScan)->EnableWindow(TRUE);
}

void PTZDlg::OnBnClickedButtonGetpangroupscan()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GetPanGroupScan)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GetPanGroupScan)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GetPanGroupScan)->EnableWindow(TRUE);
		return;
	}
	int nChannelID = m_combo_ch1.GetCurSel();

	NET_CFG_HORIZONTAL_ROTATION_GROUP_SCAN_INFO m_stuPanGroupScan = {sizeof(m_stuPanGroupScan)};
	
	BOOL bRet = CLIENT_GetConfig(m_lLoginID, NET_EM_CFG_PTZ_HORIZONTAL_ROTATION_GROUP_SCAN, nChannelID, &m_stuPanGroupScan,sizeof(m_stuPanGroupScan), 5000);
	if (bRet)
	{
		m_check_limit.SetCheck(m_stuPanGroupScan.stuHorizontalRotationInfo.bLimitEnable);
		m_combo_limitmode.SetCurSel(m_stuPanGroupScan.stuHorizontalRotationInfo.nLimitMode);
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
	GetDlgItem(IDC_BUTTON_GetPanGroupScan)->EnableWindow(TRUE);
}

void PTZDlg::OnBnClickedButtonSetpangroup()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SetPanGroup)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SetPanGroup)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SetPanGroup)->EnableWindow(TRUE);
		return;
	}
	int nChannelID = m_combo_ch2.GetCurSel();
	int ngroup = m_combo_group.GetCurSel();

	NET_IN_SET_PAN_GROUP_PARAM stuInSetPanGroup = {sizeof(stuInSetPanGroup)};
	NET_OUT_SET_PAN_GROUP_PARAM stuOutSetPanGroup = {sizeof(stuOutSetPanGroup)};
	stuInSetPanGroup.nChannel = nChannelID;
	stuInSetPanGroup.nIndex = ngroup + 1;
	m_ctrlname.GetWindowText(stuInSetPanGroup.stuPanGroupInfo.szName, sizeof(stuInSetPanGroup.stuPanGroupInfo.szName));
	stuInSetPanGroup.stuPanGroupInfo.nSpeed = atoi(m_strgroup_speed);
	stuInSetPanGroup.stuPanGroupInfo.bEnable = m_check_groupenable.GetCheck();
	stuInSetPanGroup.stuPanGroupInfo.nInterval = atoi(m_strgroup_interval);

	BOOL bRet = CLIENT_PTZSetPanGroup(m_lLoginID, &stuInSetPanGroup, &stuOutSetPanGroup, 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_SetPanGroup)->EnableWindow(TRUE);

}

void PTZDlg::OnBnClickedButtonGetpangroup()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GetPanGroup)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GetPanGroup)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GetPanGroup)->EnableWindow(TRUE);
		return;
	}
	int nChannelID = m_combo_ch2.GetCurSel();

	NET_IN_GET_PAN_GROUP_PARAM stuInGetPanGroup = {sizeof(stuInGetPanGroup)};
	NET_OUT_GET_PAN_GROUP_PARAM stuOutGetPanGroup = {sizeof(stuOutGetPanGroup)};
	stuInGetPanGroup.nChannel = nChannelID;

	BOOL bRet = CLIENT_PTZGetPanGroup(m_lLoginID, &stuInGetPanGroup, &stuOutGetPanGroup, 5000);
	if (bRet)
	{
		stuOutGetPanGroupParam = stuOutGetPanGroup;

		if (stuOutGetPanGroup.nRetNum > 0)
		{
			m_combo_group.ResetContent();
			for (int i = 0; i < stuOutGetPanGroup.nRetNum; i++)
			{
				CString str;
				str.Format("%d",i+1);
				m_combo_group.AddString(str);
			}
			m_combo_group.SetCurSel(0);

			m_group_name = stuOutGetPanGroup.stuPanGroupInfo[0].szName;
			m_strgroup_speed.Format("%d",stuOutGetPanGroup.stuPanGroupInfo[0].nSpeed);
			m_check_groupenable.SetCheck(stuOutGetPanGroup.stuPanGroupInfo[0].bEnable);
			m_strgroup_interval.Format("%d",stuOutGetPanGroup.stuPanGroupInfo[0].nInterval);
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
	GetDlgItem(IDC_BUTTON_GetPanGroup)->EnableWindow(TRUE);
}

void PTZDlg::OnBnClickedButtongotopanpos()//PtzFunc.gotoPanPosition
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_gotoPanPos)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_gotoPanPos)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_gotoPanPos)->EnableWindow(TRUE);
		return;
	}
	NET_IN_GOTO_PAN_POSITION stuInGotoPanPosition = {sizeof(stuInGotoPanPosition)};
	NET_OUT_GOTO_PAN_POSITION stuOunGotoPanPosition = {sizeof(stuOunGotoPanPosition)};
	stuInGotoPanPosition.nChannel = m_combo_ch4.GetCurSel();
	stuInGotoPanPosition.nIndex = m_combo_index2.GetCurSel() + 1;
	BOOL bRet =CLIENT_PTZGotoPanPosition(m_lLoginID, &stuInGotoPanPosition, &stuOunGotoPanPosition, 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_gotoPanPos)->EnableWindow(TRUE);
}

void PTZDlg::OnBnClickedButtonSetpangrouplimit()//PtzFunc.setPanGroupLimit 
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SetPanGroupLimit)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SetPanGroupLimit)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SetPanGroupLimit)->EnableWindow(TRUE);
		return;
	}
	NET_IN_PAN_GROUP_LIMIT_INFO stuInPanGroupLimit = {sizeof(stuInPanGroupLimit)};
	NET_OUT_PAN_GROUP_LIMIT_INFO stuOutPanGroupLimit = {sizeof(stuOutPanGroupLimit)};
	stuInPanGroupLimit.nChannelID = m_combo_ch3.GetCurSel();
	stuInPanGroupLimit.nIndex = m_combo_index.GetCurSel() + 1;
	stuInPanGroupLimit.emPanGroupLimitMode = (NET_EM_PAN_GROUP_LIMIT_MODE)(m_combo_setlimitmode.GetCurSel() + 1);
	BOOL bRet = CLIENT_PTZSetPanGroupLimit(m_lLoginID, &stuInPanGroupLimit, &stuOutPanGroupLimit, 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_SetPanGroupLimit)->EnableWindow(TRUE);
}

void PTZDlg::OnBnClickedButtonpause()//PtzFunc.pausePtzAction
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_pause)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_pause)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_pause)->EnableWindow(TRUE);
		return;
	}
	NET_IN_PAUSE_PTZ_ACTION_INFO stuInPausePtzAction = {sizeof(stuInPausePtzAction)};
	NET_OUT_PAUSE_PTZ_ACTION_INFO stuOutPausePtzAction = {sizeof(stuOutPausePtzAction)};
	stuInPausePtzAction.nChannelID = m_combo_ch4.GetCurSel();
	BOOL bRet = CLIENT_PausePtzAction(m_lLoginID, &stuInPausePtzAction, &stuOutPausePtzAction, 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_pause)->EnableWindow(TRUE);
}

void PTZDlg::OnBnClickedButtonresume()//PtzFunc.resumeLastTask
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_resume)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_resume)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_resume)->EnableWindow(TRUE);
		return;
	}
	NET_IN_RESUME_PTZ_LASTTASK_INFO stuInResumePtzAction = {sizeof(stuInResumePtzAction)};
	NET_OUT_RESUME_PTZ_LASTTASK_INFO stuOutResumePtzAction = {sizeof(stuOutResumePtzAction)};
	stuInResumePtzAction.nChannelID = m_combo_ch4.GetCurSel();
	BOOL bRet = CLIENT_ResumePtzLastTask(m_lLoginID, &stuInResumePtzAction, &stuOutResumePtzAction, 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_resume)->EnableWindow(TRUE);
}

void PTZDlg::OnCbnSelchangeComboGroup()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	int ngroup = m_combo_group.GetCurSel();
	
	m_group_name = stuOutGetPanGroupParam.stuPanGroupInfo[ngroup].szName;
	m_strgroup_speed.Format("%d",stuOutGetPanGroupParam.stuPanGroupInfo[ngroup].nSpeed);
	m_check_groupenable.SetCheck(stuOutGetPanGroupParam.stuPanGroupInfo[ngroup].bEnable);
	m_strgroup_interval.Format("%d",stuOutGetPanGroupParam.stuPanGroupInfo[ngroup].nInterval);
	UpdateData(FALSE);
}

