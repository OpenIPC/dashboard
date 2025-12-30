// ThermalSetDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "ThermalSetDlg.h"
#include "ThermalCameraDlg.h"

// ThermalSetDlg 对话框

IMPLEMENT_DYNAMIC(ThermalSetDlg, CDialog)

ThermalSetDlg::ThermalSetDlg(CWnd* pParent /*=NULL*/, LLONG lLoginId, int nChannelCount)
	: CDialog(ThermalSetDlg::IDD, pParent)
	, m_strPeriod(_T("0"))
	, m_strfusionrate(_T("0"))
	, m_strLCEValue(_T("0"))
	, m_strHistGramValue(_T("0"))
	, m_strnoisevalue(_T("0"))
	//, m_speed(0)
	, m_strasschn1(_T("0"))
	, m_strasschn2(_T("1"))
	, m_strspeed(_T("0"))
{
	m_lLoginID = lLoginId;
	m_nChannelCount = nChannelCount;
	bFirstinit = false;

	memset(&stuThermDemoise,0,sizeof(CFG_THERM_DENOISE));
}

ThermalSetDlg::~ThermalSetDlg()
{
}

void ThermalSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_FFCMODE, m_Combo_FFCMode);

	DDX_Text(pDX, IDC_EDIT_FFCPERIOD, m_strPeriod);
	DDX_Control(pDX, IDC_COMBO_FUSIONMODE, m_Combo_fusionmode);
	DDX_Text(pDX, IDC_EDIT_FUSIONRATE, m_strfusionrate);
	DDX_Text(pDX, IDC_EDIT_LCEVALUE, m_strLCEValue);
	//DDV_MinMaxUInt(pDX, m_unLCEValue, 0, 128);
	DDX_Text(pDX, IDC_EDIT_HISTGRAMVALUE, m_strHistGramValue);
	//DDV_MinMaxUInt(pDX, m_unHistGramValue, 0, 32);
	DDX_Control(pDX, IDC_COMBO_FFCCHN, m_combo_ffcchn);
	DDX_Control(pDX, IDC_COMBO_LCECHN, m_combo_lcechn);
	DDX_Control(pDX, IDC_COMBO_NOISECHN, m_combo_noisechn);
	DDX_Control(pDX, IDC_COMBO_NOISESCENE, m_combo_noisescene);
	DDX_Text(pDX, IDC_EDIT_noisevalue, m_strnoisevalue);
	DDX_Control(pDX, IDC_CHECK_noise, m_check_noise);
	//DDX_Text(pDX, IDC_EDIT_speed, m_speed);
	// 	DDV_MinMaxInt(pDX, m_nPeriod, 0, 1200);
	// 	DDV_MinMaxInt(pDX, m_unfusionrate, 0, 100);
	// 	DDV_MinMaxInt(pDX, m_unLCEValue, 0, 128);
	// 	DDV_MinMaxInt(pDX, m_unHistGramValue, 0, 32);
	// 	DDV_MinMaxInt(pDX, m_noisevalue, 0, 100);

	DDX_Text(pDX, IDC_EDIT_Asschn1, m_strasschn1);
	DDX_Text(pDX, IDC_EDIT_Asschn2, m_strasschn2);
	DDX_Text(pDX, IDC_EDIT_speed, m_strspeed);
}


BEGIN_MESSAGE_MAP(ThermalSetDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GETFFC, &ThermalSetDlg::OnBnClickedButtonGetffc)
	ON_BN_CLICKED(IDC_BUTTON_SETFFC, &ThermalSetDlg::OnBnClickedButtonSetffc)
	ON_BN_CLICKED(IDC_BUTTON_GETFUSION, &ThermalSetDlg::OnBnClickedButtonGetfusion)
	ON_BN_CLICKED(IDC_BUTTON_SETFUSION, &ThermalSetDlg::OnBnClickedButtonSetfusion)
	ON_BN_CLICKED(IDC_BUTTON_GETLCESTATE, &ThermalSetDlg::OnBnClickedButtonGetlcestate)
	ON_BN_CLICKED(IDC_BUTTON_SETLCESTATE, &ThermalSetDlg::OnBnClickedButtonSetlcestate)

	ON_BN_CLICKED(IDC_BUTTON_GETNOISE, &ThermalSetDlg::OnBnClickedButtonGetnoise)
	ON_BN_CLICKED(IDC_BUTTON_SETNOISE, &ThermalSetDlg::OnBnClickedButtonSetnoise)
	ON_CBN_SELCHANGE(IDC_COMBO_NOISESCENE, &ThermalSetDlg::OnCbnSelchangeComboNoisescene)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS, &ThermalSetDlg::OnBnClickedButtonFocus)
	ON_BN_CLICKED(IDC_BUTTON_DOFFC, &ThermalSetDlg::OnBnClickedButtonDoffc)
END_MESSAGE_MAP()


// ThermalSetDlg 消息处理程序

BOOL ThermalSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	//m_speed = 0;

	CEdit* pEdit = (CEdit*)this->GetDlgItem(IDC_EDIT_FFCPERIOD);

	CEdit* pEditspeed = (CEdit*)this->GetDlgItem(IDC_EDIT_speed);
	pEditspeed->SetLimitText(32);

	// TODO:  在此添加额外的初始化
	for (int i = 0 ;i<m_nChannelCount;i++)
	{
		CString str;
		str.Format("%d",i);
		m_combo_ffcchn.AddString(str);
		m_combo_lcechn.AddString(str);
		m_combo_noisechn.AddString(str);
	}
	if (m_nChannelCount>0)
	{
		m_combo_ffcchn.SetCurSel(m_nChannelCount-1);
		m_combo_lcechn.SetCurSel(m_nChannelCount-1);
		m_combo_noisechn.SetCurSel(m_nChannelCount-1);
	}

	m_Combo_FFCMode.AddString(ConvertString("UnKnown"));
	m_Combo_FFCMode.AddString(ConvertString("Auto"));
	m_Combo_FFCMode.AddString(ConvertString("Manual"));
	m_Combo_FFCMode.SetCurSel(0);

	m_Combo_fusionmode.AddString(ConvertString("Original"));
	m_Combo_fusionmode.AddString(ConvertString("Fusion"));
	m_Combo_fusionmode.SetCurSel(0);

	m_combo_noisescene.AddString(ConvertString("day"));
	m_combo_noisescene.AddString(ConvertString("night"));
	m_combo_noisescene.AddString(ConvertString("ordinary"));
	m_combo_noisescene.SetCurSel(0);

	if (m_lLoginID)
	{
		bFirstinit = true;
		OnBnClickedButtonGetffc();
		OnBnClickedButtonGetfusion();
		OnBnClickedButtonGetlcestate();
		OnBnClickedButtonGetnoise();
		bFirstinit = false;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void ThermalSetDlg::OnBnClickedButtonGetffc()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETFFC)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETFFC)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GETFFC)->EnableWindow(TRUE);
		return;
	}
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nChannel = m_combo_ffcchn.GetCurSel();
	int index = 0;
	BOOL ret = CLIENT_GetNewDevConfig(m_lLoginID,CFG_CMD_FLAT_FIELD_CORRECTION,nChannel,szJsonBuf,1024*40,&nerror,3000);
	if (0 != ret)
	{
		CFG_FLAT_FIELD_CORRECTION_INFO stuInfo;
		memset(&stuInfo,0, sizeof(CFG_FLAT_FIELD_CORRECTION_INFO));
		DWORD dwRetLen = 0;
		ret = CLIENT_ParseData(CFG_CMD_FLAT_FIELD_CORRECTION,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
		if (!ret)
		{
			if (!bFirstinit)
			{
				MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
			}
			GetDlgItem(IDC_BUTTON_GETFFC)->EnableWindow(TRUE);
			return;
		}
		else
		{
			EM_FLAT_FIELD_CORRECTION_MODE   emMode = stuInfo.emMode;
			m_Combo_FFCMode.SetCurSel(emMode);
			//m_strPeriod = stuInfo.nPeriod;
			m_strPeriod.Format("%d",stuInfo.nPeriod);
			UpdateData(FALSE);
			if (!bFirstinit)
			{
				//MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
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
	UpdateData(FALSE);
	GetDlgItem(IDC_BUTTON_GETFFC)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonSetffc()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETFFC)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETFFC)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETFFC)->EnableWindow(TRUE);
		return;
	}
	CFG_FLAT_FIELD_CORRECTION_INFO stuInfo;
	memset(&stuInfo,0, sizeof(CFG_FLAT_FIELD_CORRECTION_INFO));
	
	stuInfo.emMode = (EM_FLAT_FIELD_CORRECTION_MODE)m_Combo_FFCMode.GetCurSel();
	stuInfo.nPeriod = atoi(m_strPeriod);

	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_FLAT_FIELD_CORRECTION, &stuInfo, sizeof(stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		int nChannelID = m_combo_ffcchn.GetCurSel();
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_FLAT_FIELD_CORRECTION, nChannelID, szJsonBuf, strlen(szJsonBuf), &nerror, &nrestart, 3000);
		if (!bRet)
		{
			MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		}
		else
		{
			//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_SETFFC)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonGetfusion()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETFUSION)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETFUSION)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GETFUSION)->EnableWindow(TRUE);
		return;
	}
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nChannel = 1;
	int index = 0;
	BOOL ret = CLIENT_GetNewDevConfig(m_lLoginID,CFG_CMD_THERMO_FUSION,nChannel,szJsonBuf,1024*40,&nerror,3000);
	if (0 != ret)
	{
		CFG_THERMO_FUSION_INFO stuInfo;
		memset(&stuInfo,0, sizeof(CFG_THERMO_FUSION_INFO));
		DWORD dwRetLen = 0;
		ret = CLIENT_ParseData(CFG_CMD_THERMO_FUSION,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
		if (!ret)
		{
			if (!bFirstinit)
			{
				MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
			}
			GetDlgItem(IDC_BUTTON_GETFUSION)->EnableWindow(TRUE);
			return;
		}
		else
		{
			m_Combo_fusionmode.SetCurSel(stuInfo.unMode);
			m_strfusionrate.Format("%d",stuInfo.unRate);
			UpdateData(FALSE);
			if (!bFirstinit)
			{
				//MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
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
	UpdateData(FALSE);
	GetDlgItem(IDC_BUTTON_GETFUSION)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonSetfusion()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETFUSION)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETFUSION)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETFUSION)->EnableWindow(TRUE);
		return;
	}

	CFG_THERMO_FUSION_INFO stuInfo;
	memset(&stuInfo,0, sizeof(CFG_THERMO_FUSION_INFO));

	stuInfo.unMode = m_Combo_fusionmode.GetCurSel();
	stuInfo.unRate = atoi(m_strfusionrate);

	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_THERMO_FUSION, &stuInfo, sizeof(stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		int nChannelID = 1;
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_THERMO_FUSION, nChannelID, szJsonBuf, strlen(szJsonBuf), &nerror, &nrestart, 3000);
		if (!bRet)
		{
			MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		}
		else
		{
			//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_SETFUSION)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonGetlcestate()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETLCESTATE)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETLCESTATE)->EnableWindow(TRUE);
		return;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GETLCESTATE)->EnableWindow(TRUE);
		return;
	}
	int nerror = 0;
	int nChannel = m_combo_lcechn.GetCurSel();
	int index = 0;
	BOOL ret = CLIENT_GetNewDevConfig(m_lLoginID,CFG_CMD_LCE_STATE,nChannel,szJsonBuf,1024*40,&nerror,3000);
	if (0 != ret)
	{
		CFG_LCE_STATE_INFO stuInfo;
		memset(&stuInfo,0, sizeof(CFG_LCE_STATE_INFO));
		DWORD dwRetLen = 0;
		ret = CLIENT_ParseData(CFG_CMD_LCE_STATE,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
		if (!ret)
		{
			if (!bFirstinit)
			{
				MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
			}
			GetDlgItem(IDC_BUTTON_GETLCESTATE)->EnableWindow(TRUE);
			return;
		}
		else
		{
			//m_strLCEValue = stuInfo.unLCEValue;
			m_strLCEValue.Format("%d",stuInfo.unLCEValue);
			m_strHistGramValue.Format("%d",stuInfo.unHistGramValue);
			UpdateData(FALSE);
			if (!bFirstinit)
			{
				//MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
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
	UpdateData(FALSE);
	GetDlgItem(IDC_BUTTON_GETLCESTATE)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonSetlcestate()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETLCESTATE)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETLCESTATE)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETLCESTATE)->EnableWindow(TRUE);
		return;
	}
	CFG_LCE_STATE_INFO stuInfo;
	memset(&stuInfo,0, sizeof(CFG_LCE_STATE_INFO));

	stuInfo.unLCEValue = atoi(m_strLCEValue);
	stuInfo.unHistGramValue = atoi(m_strHistGramValue);

	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_LCE_STATE, &stuInfo, sizeof(stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		int nChannelID = m_combo_lcechn.GetCurSel();
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_LCE_STATE, nChannelID, szJsonBuf, strlen(szJsonBuf), &nerror, &nrestart, 3000);
		if (!bRet)
		{
			MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		}
		else
		{
			//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_SETLCESTATE)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonGetnoise()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETNOISE)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETNOISE)->EnableWindow(TRUE);
		return;
	}
	char szJsonBuf[1024 * 4] = {0};
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GETNOISE)->EnableWindow(TRUE);
		return;
	}
	int nerror = 0;
	int nChannel = m_combo_noisechn.GetCurSel();
	int index = 0;
	BOOL ret = CLIENT_GetNewDevConfig(m_lLoginID,CFG_CMD_THERM_DENOISE,nChannel,szJsonBuf,1024*4,&nerror,3000);
	if (0 != ret)
	{
		CFG_THERM_DENOISE stuInfo;
		memset(&stuInfo,0, sizeof(CFG_THERM_DENOISE));
		DWORD dwRetLen = 0;
		int dd = sizeof(stuInfo);
		ret = CLIENT_ParseData(CFG_CMD_THERM_DENOISE,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
		if (!ret)
		{
			if (!bFirstinit)
			{
				MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
			}
			GetDlgItem(IDC_BUTTON_GETNOISE)->EnableWindow(TRUE);
			return;
		}
		else
		{
			stuThermDemoise = stuInfo;
			m_combo_noisescene.SetCurSel(0);
			m_check_noise.SetCheck(stuInfo.stuThermDenoiseInfo[0].bEnable);
			m_strnoisevalue.Format("%d",stuInfo.stuThermDenoiseInfo[0].nDenoiseValue);
			UpdateData(FALSE);
			if (!bFirstinit)
			{
				//MessageBox(ConvertString("getconfig ok"), ConvertString("Prompt"));
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
	GetDlgItem(IDC_BUTTON_GETNOISE)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonSetnoise()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETNOISE)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETNOISE)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETNOISE)->EnableWindow(TRUE);
		return;
	}

	CFG_THERM_DENOISE stuInfo;
	memset(&stuInfo,0, sizeof(CFG_THERM_DENOISE));
	stuInfo = stuThermDemoise;

	int nnoisescene = m_combo_noisescene.GetCurSel();
	stuInfo.stuThermDenoiseInfo[nnoisescene].bEnable = m_check_noise.GetCheck();
	stuInfo.stuThermDenoiseInfo[nnoisescene].nDenoiseValue = atoi(m_strnoisevalue);

	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_THERM_DENOISE, &stuInfo, sizeof(stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		int nChannelID = m_combo_lcechn.GetCurSel();
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_THERM_DENOISE, nChannelID, szJsonBuf, strlen(szJsonBuf), &nerror, &nrestart, 3000);
		if (!bRet)
		{
			MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		}
		else
		{
			//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
		}
	}
	GetDlgItem(IDC_BUTTON_SETNOISE)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnCbnSelchangeComboNoisescene()
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
	int nnoisescene = m_combo_noisescene.GetCurSel();

 	m_check_noise.SetCheck(stuThermDemoise.stuThermDenoiseInfo[nnoisescene].bEnable);
	m_strnoisevalue.Format("%d",stuThermDemoise.stuThermDenoiseInfo[nnoisescene].nDenoiseValue);
	UpdateData(FALSE);
}

void ThermalSetDlg::OnBnClickedButtonFocus()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_FOCUS)->EnableWindow(FALSE);
	BOOL bret = UpdateData();
	if (!bret)
	{
		return;
	}
	NET_IN_THERMO_FIX_FOCUS stuInThermoFixFocus = {sizeof(stuInThermoFixFocus)};
	tagNET_OUT_THERMO_FIX_FOCUS stuOutThermoFixFocus = {sizeof(stuOutThermoFixFocus)};
	stuInThermoFixFocus.anLinkVideoChannel[0] = atoi(m_strasschn1);
	stuInThermoFixFocus.anLinkVideoChannel[1] = atoi(m_strasschn2);
 	stuInThermoFixFocus.dSpeed = atof(m_strspeed);
	BOOL bRet = CLIENT_ControlDeviceEx(m_lLoginID, DH_CTRL_THERMO_FIX_FOCUS, &stuInThermoFixFocus, &stuOutThermoFixFocus, 3000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_FOCUS)->EnableWindow(TRUE);
}

void ThermalSetDlg::OnBnClickedButtonDoffc()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_DOFFC)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_DOFFC)->EnableWindow(TRUE);
		return;
	}
	NET_IN_THERMO_DO_FFC stuInThermoDoFFC = {sizeof(stuInThermoDoFFC)};
	NET_OUT_THERMO_DO_FFC stuOutThermoDoFFC  = {sizeof(stuOutThermoDoFFC)};
	stuInThermoDoFFC.nChannel = m_combo_ffcchn.GetCurSel();
	BOOL bRet = CLIENT_ControlDeviceEx(m_lLoginID, DH_CTRL_THERMO_DO_FFC, &stuInThermoDoFFC, &stuOutThermoDoFFC, 5000);
	if (!bRet)
	{
		MessageBox(ConvertString("Doffc error"), ConvertString("Prompt"));
	}
	else
	{
		//MessageBox(ConvertString("Doffc ok"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_DOFFC)->EnableWindow(TRUE);
}

