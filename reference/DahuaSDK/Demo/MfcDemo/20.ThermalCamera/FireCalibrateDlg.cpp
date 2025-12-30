// FireCalibrateDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "FireCalibrateDlg.h"
#include "ThermalCameraDlg.h"

// FireCalibrateDlg 对话框

IMPLEMENT_DYNAMIC(FireCalibrateDlg, CDialog)

FireCalibrateDlg::FireCalibrateDlg(CWnd* pParent /*=NULL*/, LLONG lLoginId/* = 0*/, int nChannelCount)
	: CDialog(FireCalibrateDlg::IDD, pParent)
	, m_name(_T(""))
// 	, m_dLongitude(0)
// 	, m_dLatitude(0)
// 	, m_dAltitude(0)
	, m_strx1(_T(""))
	, m_stry1(_T(""))
	, m_strx2(_T(""))
	, m_stry2(_T(""))
// 	, m_dGPSLongitude(0)
// 	, m_dGPSLatitude(0)
// 	, m_dGPSAltitude(0)
// 	, m_fGPSHeight(0)
, m_strGPSLongitude(_T(""))
, m_strGPSLatitude(_T(""))
, m_strGPSAltitude(_T(""))
, m_strGPSHeight(_T(""))
, m_strLongitude(_T(""))
, m_strLatitude(_T(""))
, m_strAltitude(_T(""))
{
	m_lLoginID = lLoginId;
	bFirstinit = false;

	memset(&stuLocationCalibrateInfo,0,sizeof(NET_LOCATION_CALIBRATE_INFO));
	memset(&stuLocationCalibrateInfoCFG,0,sizeof(CFG_LOCATION_CALIBRATE_INFO));
}

FireCalibrateDlg::~FireCalibrateDlg()
{
}

void FireCalibrateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PointID, m_combo_pointid);
	DDX_Text(pDX, IDC_EDIT_NAME, m_name);
	// 	DDX_Text(pDX, IDC_EDIT_PLongitude, m_dLongitude);
	// 	DDX_Text(pDX, IDC_EDIT_PLatitude, m_dLatitude);
	// 	DDX_Text(pDX, IDC_EDIT_PAltitude, m_dAltitude);
	DDX_Text(pDX, IDC_EDIT_X1, m_strx1);
	DDX_Text(pDX, IDC_EDIT_Y1, m_stry1);
	DDX_Text(pDX, IDC_EDIT_X2, m_strx2);
	DDX_Text(pDX, IDC_EDIT_Y2, m_stry2);
	DDX_Control(pDX, IDC_COMBO_CALID, m_combo_calid);
	// 	DDX_Text(pDX, IDC_EDIT_Longitude, m_dGPSLongitude);
	// 	DDX_Text(pDX, IDC_EDIT_Latitude, m_dGPSLatitude);
	// 	DDX_Text(pDX, IDC_EDIT_Altitude, m_dGPSAltitude);
	// 	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_fGPSHeight);
	DDX_Control(pDX, IDC_CHECK_Config, m_check_GPSconfig);
	// 	DDV_MinMaxDouble(pDX, m_dGPSLongitude, 0, 360);
	// 	DDV_MinMaxDouble(pDX, m_dGPSLatitude, 0, 180);
	DDX_Text(pDX, IDC_EDIT_Longitude, m_strGPSLongitude);
	DDX_Text(pDX, IDC_EDIT_Latitude, m_strGPSLatitude);
	DDX_Text(pDX, IDC_EDIT_Altitude, m_strGPSAltitude);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_strGPSHeight);
	DDX_Text(pDX, IDC_EDIT_PLongitude, m_strLongitude);
	DDX_Text(pDX, IDC_EDIT_PLatitude, m_strLatitude);
	DDX_Text(pDX, IDC_EDIT_PAltitude, m_strAltitude);
}


BEGIN_MESSAGE_MAP(FireCalibrateDlg, CDialog)

	ON_BN_CLICKED(IDC_BUTTON_GETDEVLOCATION, &FireCalibrateDlg::OnBnClickedButtonGetdevlocation)
	ON_BN_CLICKED(IDC_BUTTON_SETDEVLOCATION, &FireCalibrateDlg::OnBnClickedButtonSetdevlocation)
	ON_BN_CLICKED(IDC_BUTTON_GETCAL, &FireCalibrateDlg::OnBnClickedButtonGetcal)
	ON_BN_CLICKED(IDC_BUTTON_SETCAL, &FireCalibrateDlg::OnBnClickedButtonSetcal)
	ON_BN_CLICKED(IDC_BUTTON_DELCAL, &FireCalibrateDlg::OnBnClickedButtonDelcal)
	ON_BN_CLICKED(IDC_BUTTON_GETALLCAL, &FireCalibrateDlg::OnBnClickedButtonGetallcal)
	ON_CBN_SELCHANGE(IDC_COMBO_PointID, &FireCalibrateDlg::OnCbnSelchangeComboPointid)
	ON_CBN_SELCHANGE(IDC_COMBO_CALID, &FireCalibrateDlg::OnCbnSelchangeComboCalid)
END_MESSAGE_MAP()


// FireCalibrateDlg 消息处理程序

BOOL FireCalibrateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	CEdit* pEditname = (CEdit*)this->GetDlgItem(IDC_EDIT_NAME);
	pEditname->SetLimitText(64);

	CEdit* pEditPLongitude = (CEdit*)this->GetDlgItem(IDC_EDIT_PLongitude);
	pEditPLongitude->SetLimitText(64);
	CEdit* pEditPLatitude = (CEdit*)this->GetDlgItem(IDC_EDIT_PLatitude);
	pEditPLatitude->SetLimitText(64);
	CEdit* pEditPAltitude = (CEdit*)this->GetDlgItem(IDC_EDIT_PAltitude);
	pEditPAltitude->SetLimitText(64);

	CEdit* pEditGPSLongitude = (CEdit*)this->GetDlgItem(IDC_EDIT_Longitude);
	pEditGPSLongitude->SetLimitText(64);
	CEdit* pEditGPSLatitude = (CEdit*)this->GetDlgItem(IDC_EDIT_Latitude);
	pEditGPSLatitude->SetLimitText(64);
	CEdit* pEditGPSAltitude = (CEdit*)this->GetDlgItem(IDC_EDIT_Altitude);
	pEditGPSAltitude->SetLimitText(64);
	CEdit* pEditGPSHEIGHT = (CEdit*)this->GetDlgItem(IDC_EDIT_HEIGHT);
	pEditGPSHEIGHT->SetLimitText(64);

	// TODO:  在此添加额外的初始化
	if (m_lLoginID)
	{
		bFirstinit = true;
		OnBnClickedButtonGetallcal();
		OnBnClickedButtonGetdevlocation();
		//OnBnClickedButtonGetlocationcal();
		bFirstinit = false;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void FireCalibrateDlg::OnBnClickedButtonGetdevlocation()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETDEVLOCATION)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETDEVLOCATION)->EnableWindow(TRUE);
		return;
	}
	char buffData[100 * 1024] = {0};

	int n;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_DEVLOCATION, -1, buffData, 100 * 1024, &n, 5000);
	CFG_DEVLOCATION_INFO st = {0};
	if (bRet)
	{
		bRet = CLIENT_ParseData(CFG_CMD_DEVLOCATION, buffData, (char*)&st, sizeof(CFG_DEVLOCATION_INFO), &n);
		if (bRet)
		{
			double m_dGPSLongitude = UINTToLONLAT(st.unLongitude, 1);
			double m_dGPSLatitude = UINTToLONLAT(st.unLatitude, 2);
			double m_dGPSAltitude = st.dbAltitude;
			float m_fGPSHeight = st.fHeight;
			m_strGPSLongitude.Format("%.06f",m_dGPSLongitude);
			m_strGPSLatitude.Format("%.06f",m_dGPSLatitude);
			m_strGPSAltitude.Format("%.02f",m_dGPSAltitude);
			m_strGPSHeight.Format("%.02f",m_fGPSHeight);
			m_check_GPSconfig.SetCheck(st.bConfigEnable);
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
	GetDlgItem(IDC_BUTTON_GETDEVLOCATION)->EnableWindow(TRUE);
}

void FireCalibrateDlg::OnBnClickedButtonSetdevlocation()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETDEVLOCATION)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETDEVLOCATION)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETDEVLOCATION)->EnableWindow(TRUE);
		return;
	}
	char buffData[100 * 1024] = {0};
	CFG_DEVLOCATION_INFO st = {0};

	double m_dGPSLongitude = atof(m_strGPSLongitude);
	double m_dGPSLatitude = atof(m_strGPSLatitude);
	double m_dGPSAltitude = atof(m_strGPSAltitude);
	float m_fGPSHeight = atof(m_strGPSHeight);

	st.unLongitude = LONLATtoUINT(m_dGPSLongitude, 1);
	st.unLatitude = LONLATtoUINT(m_dGPSLatitude, 2);
	st.dbAltitude = m_dGPSAltitude;
	st.fHeight = m_fGPSHeight;
	st.bConfigEnable = m_check_GPSconfig.GetCheck();

	BOOL bRet = CLIENT_PacketData(CFG_CMD_DEVLOCATION, &st, sizeof(st), buffData, 100 * 1024);
	if (bRet)
	{
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_DEVLOCATION, -1, buffData, 100*1024, NULL, NULL, 5000);
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
	GetDlgItem(IDC_BUTTON_SETDEVLOCATION)->EnableWindow(TRUE);
}

void FireCalibrateDlg::OnBnClickedButtonGetcal()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETCAL)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETCAL)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_GETCAL)->EnableWindow(TRUE);
		return;
	}
	int ncombocount = m_combo_pointid.GetCount();
	if (ncombocount <= 0)
	{
		GetDlgItem(IDC_BUTTON_GETCAL)->EnableWindow(TRUE);
		return;
	}

	m_combo_calid.ResetContent();
	m_strx1=_T("");
	m_stry1=_T("");
	m_strx2=_T("");
	m_stry2=_T("");
	UpdateData(FALSE);

	NET_IN_GET_CALIBRATEINFO_INFO stuIn = {sizeof(stuIn)};
	NET_OUT_GET_CALIBRATEINFO_INFO stuOut = {sizeof(stuOut)};

	CString strID;
	m_combo_pointid.GetWindowText(strID);
	stuIn.nID = atoi(strID);
	BOOL bRet = CLIENT_OperateCalibrateInfo(m_lLoginID, EM_CALIBRATEINFO_OPERATE_GET, &stuIn, &stuOut, 3000);
	if (bRet)
	{
		m_name = stuOut.stuPointInfo.szName;
		double m_dLongitude = UINTToLONLAT(stuOut.stuPointInfo.nLongitude,1);//经度
		double m_dLatitude = UINTToLONLAT(stuOut.stuPointInfo.nLatitude,2);//纬度
		double m_dAltitude = stuOut.stuPointInfo.fAltitude;		//海拔
		m_strLongitude.Format("%.06f",m_dLongitude);
		m_strLatitude.Format("%.06f",m_dLatitude);
		m_strAltitude.Format("%.02f",m_dAltitude);

		//标定点
		int ncalibrateinfonum = stuOut.stuPointInfo.nPointNum;
		if (ncalibrateinfonum > 0)
		{
			for (int i=0;i<ncalibrateinfonum;i++)
			{
				CString str;
				str.Format("%d",i+1);
				m_combo_calid.AddString(str);
			}
			m_combo_calid.SetCurSel(0);

			NET_GET_CALIBRATEINFO_POINT_INFO stuPointInfo = stuOut.stuPointInfo;
			m_strx1.Format("%d", stuPointInfo.stuCalcInfo[0][0].nLocation[0]);
			m_stry1.Format("%d", stuPointInfo.stuCalcInfo[0][0].nLocation[1]);
			m_strx2.Format("%d", stuPointInfo.stuCalcInfo[0][1].nLocation[0]);
			m_stry2.Format("%d", stuPointInfo.stuCalcInfo[0][1].nLocation[1]);
		}

		UpdateData(FALSE);

		NET_IN_GETALL_CALIBRATEINFO_INFO stuIn = {sizeof(stuIn)};
		NET_OUT_GETALL_CALIBRATEINFO_INFO stuOut = {sizeof(stuOut)};
		BOOL bRetAll = CLIENT_OperateCalibrateInfo(m_lLoginID, EM_CALIBRATEINFO_OPERATE_GETALL, &stuIn, &stuOut, 3000);
		if (bRetAll)
		{
			stuLocationCalibrateInfo = stuOut.stuLocationCalibrateInfo;
		}
	}
	else
	{
		MessageBox(ConvertString("get info error"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_GETCAL)->EnableWindow(TRUE);

}

void FireCalibrateDlg::OnBnClickedButtonSetcal()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_SETCAL)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETCAL)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_SETCAL)->EnableWindow(TRUE);
		return;
	}
	int ncombocount = m_combo_pointid.GetCount();
	if (ncombocount <= 0)
	{
		GetDlgItem(IDC_BUTTON_SETCAL)->EnableWindow(TRUE);
		return;
	}

	NET_IN_SET_CALIBRATEINFO_INFO stuIn = {sizeof(stuIn)};
	NET_OUT_SET_CALIBRATEINFO_INFO stuOut = {sizeof(stuOut)};
	//先拷贝内存的东西
	for (int i=0;i<5;i++)
	{
		for (int j=0;j<2;j++)
		{
			stuIn.stuPointInfo.stuCalcInfo[i][j].fPosition[0] = stuLocationCalibrateInfo.stuPointInfo[m_combo_pointid.GetCurSel()].stuCalibrateInfo.stuCalibrateUnitInfo[i][j].nPosition[0];
			stuIn.stuPointInfo.stuCalcInfo[i][j].fPosition[1] = stuLocationCalibrateInfo.stuPointInfo[m_combo_pointid.GetCurSel()].stuCalibrateInfo.stuCalibrateUnitInfo[i][j].nPosition[1];
			stuIn.stuPointInfo.stuCalcInfo[i][j].fPosition[2] = stuLocationCalibrateInfo.stuPointInfo[m_combo_pointid.GetCurSel()].stuCalibrateInfo.stuCalibrateUnitInfo[i][j].nPosition[2];
			stuIn.stuPointInfo.stuCalcInfo[i][j].nLocation[0] = stuLocationCalibrateInfo.stuPointInfo[m_combo_pointid.GetCurSel()].stuCalibrateInfo.stuCalibrateUnitInfo[i][j].nLocation[0];
			stuIn.stuPointInfo.stuCalcInfo[i][j].nLocation[1] = stuLocationCalibrateInfo.stuPointInfo[m_combo_pointid.GetCurSel()].stuCalibrateInfo.stuCalibrateUnitInfo[i][j].nLocation[1];
		}
	}
// 	
// 	//再获取屏幕上修改的东西
	//stuIn.stuPointInfo.nID = m_combo_pointid.GetCurSel()+1;
	CString strID;
	m_combo_pointid.GetWindowText(strID);
	stuIn.stuPointInfo.nID = atoi(strID);
	strncpy(stuIn.stuPointInfo.szName, m_name, sizeof(stuIn.stuPointInfo.szName)-1);
	double m_dLongitude = atof(m_strLongitude);
	double m_dLatitude = atof(m_strLatitude);
	double m_dAltitude = atof(m_strAltitude);
	stuIn.stuPointInfo.nLongitude = LONLATtoUINT(m_dLongitude,1);
	stuIn.stuPointInfo.nLatitude = LONLATtoUINT(m_dLatitude,2);
	stuIn.stuPointInfo.fAltitude = m_dAltitude;

	stuIn.stuPointInfo.nPointNum = m_combo_calid.GetCount();
	stuIn.stuPointInfo.bEnable = TRUE;
	if (m_combo_calid.GetCount() > 0)
	{
		stuIn.stuPointInfo.stuCalcInfo[m_combo_calid.GetCurSel()][0].nLocation[0] = atoi(m_strx1) ;
		stuIn.stuPointInfo.stuCalcInfo[m_combo_calid.GetCurSel()][0].nLocation[1] = atoi(m_stry1) ;
		stuIn.stuPointInfo.stuCalcInfo[m_combo_calid.GetCurSel()][1].nLocation[0] = atoi(m_strx2) ;
		stuIn.stuPointInfo.stuCalcInfo[m_combo_calid.GetCurSel()][1].nLocation[1] = atoi(m_stry2) ;
	}

	BOOL bRet = CLIENT_OperateCalibrateInfo(m_lLoginID, EM_CALIBRATEINFO_OPERATE_SET, &stuIn, &stuOut, 3000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_SETCAL)->EnableWindow(TRUE);
		return;
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	bFirstinit = true;
	OnBnClickedButtonGetallcal();
	bFirstinit = false;
	GetDlgItem(IDC_BUTTON_SETCAL)->EnableWindow(TRUE);
}

void FireCalibrateDlg::OnBnClickedButtonDelcal()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_DELCAL)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_DELCAL)->EnableWindow(TRUE);
		return;
	}
	BOOL bret = UpdateData();
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_DELCAL)->EnableWindow(TRUE);
		return;
	}
	int ncombocount = m_combo_pointid.GetCount();
	if (ncombocount <= 0)
	{
		GetDlgItem(IDC_BUTTON_DELCAL)->EnableWindow(TRUE);
		return;
	}
	NET_IN_DELETE_CALIBRATEINFO_INFO stuIn = {sizeof(stuIn)};
	NET_OUT_DELETE_CALIBRATEINFO_INFO stuOut = {sizeof(stuOut)};

	//stuIn.nID = m_combo_pointid.GetCurSel()+1;
	CString strID;
	m_combo_pointid.GetWindowText(strID);
	stuIn.nID = atoi(strID);
	BOOL bRet = CLIENT_OperateCalibrateInfo(m_lLoginID, EM_CALIBRATEINFO_OPERATE_DELETE, &stuIn, &stuOut, 3000);
	if (!bRet)
	{
		MessageBox(ConvertString("setconfig error"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_DELCAL)->EnableWindow(TRUE);
		return;
	}
	else
	{
		//MessageBox(ConvertString("setconfig ok"), ConvertString("Prompt"));
	}
	bFirstinit = true;
	OnBnClickedButtonGetallcal();
	bFirstinit = false;
	GetDlgItem(IDC_BUTTON_DELCAL)->EnableWindow(TRUE);
}

void FireCalibrateDlg::OnBnClickedButtonGetallcal()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BUTTON_GETALLCAL)->EnableWindow(FALSE);
	if (!m_lLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETALLCAL)->EnableWindow(TRUE);
		return;
	}
	m_combo_pointid.ResetContent();
	m_combo_calid.ResetContent();
	m_name = "";
	m_strLongitude = "";
	m_strLatitude = "";
	m_strAltitude = "";
	m_strx1=_T("");
	m_stry1=_T("");
	m_strx2=_T("");
	m_stry2=_T("");
	GetDlgItem(IDC_EDIT_PLongitude)->SetWindowText("");
	UpdateData(FALSE);

	NET_IN_GETALL_CALIBRATEINFO_INFO stuIn = {sizeof(stuIn)};
	NET_OUT_GETALL_CALIBRATEINFO_INFO stuOut = {sizeof(stuOut)};
	BOOL bRet = CLIENT_OperateCalibrateInfo(m_lLoginID, EM_CALIBRATEINFO_OPERATE_GETALL, &stuIn, &stuOut, 3000);
	if (bRet)
	{
		stuLocationCalibrateInfo = stuOut.stuLocationCalibrateInfo;

		int npointnum = stuOut.stuLocationCalibrateInfo.nPointNum;
		if (npointnum > 0)
		{
			for (int i=0;i<npointnum;i++)
			{
				CString str;
				str.Format("%d",stuOut.stuLocationCalibrateInfo.stuPointInfo[i].nID);
				m_combo_pointid.AddString(str);
			}
			m_combo_pointid.SetCurSel(0);

			//显示第一个点的信息
			NET_LOCATION_CALIBRATE_POINT_INFO stuCalibratePoint = stuOut.stuLocationCalibrateInfo.stuPointInfo[0];
			m_name = stuCalibratePoint.szName;
			double m_dLongitude = UINTToLONLAT(stuCalibratePoint.nLongitude,1);//经度
			double m_dLatitude = UINTToLONLAT(stuCalibratePoint.nLatitude,2);//纬度
			double m_dAltitude = stuCalibratePoint.fAltitude;		//海拔
			m_strLongitude.Format("%.06f",m_dLongitude);
			m_strLatitude.Format("%.06f",m_dLatitude);
			m_strAltitude.Format("%.02f",m_dAltitude);
			int ncalibrateinfonum = stuCalibratePoint.stuCalibrateInfo.nInfoNum;
			if (ncalibrateinfonum > 0)
			{
				for (int i=0;i<ncalibrateinfonum;i++)
				{
					CString str;
					str.Format("%d",i+1);
					m_combo_calid.AddString(str);
				}
				m_combo_calid.SetCurSel(0);

				NET_CALIBRATE_INFO stuCalibrateInfo = stuCalibratePoint.stuCalibrateInfo;
				m_strx1.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][0].nLocation[0]);
				m_stry1.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][0].nLocation[1]);
				m_strx2.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][1].nLocation[0]);
				m_stry2.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][1].nLocation[1]);
			}

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
	GetDlgItem(IDC_BUTTON_GETALLCAL)->EnableWindow(TRUE);

}

double FireCalibrateDlg::UINTToLONLAT(UINT unParam, int nType)
{
	double dtemp = 0;
	if (nType == 1)//unint -> 经度
	{
		dtemp = ((unParam - 0) - 180.0 * 1000000.0)/1000000.0;
		//经度
// 		int nloninteger = unParam/1000000 - 180;//经纬度单位百万分之一
// 		double nlondecimal = (int)unParam%1000000;
// 		nlondecimal = nlondecimal/1000000.0;
// 		dtemp = nlondecimal + abs(nloninteger); 
	}
	else if(nType == 2)//unint -> 纬度
	{
		//纬度
		dtemp = ((unParam - 0) - 90.0 * 1000000.0)/1000000.0;
// 		int nlatinteger = unParam/1000000 - 90;
// 		double nlatdecimal = (int)unParam%1000000;
// 		nlatdecimal = nlatdecimal/1000000.0;
// 		dtemp = nlatdecimal + abs(nlatinteger);
	}
	return dtemp;
}

UINT FireCalibrateDlg::LONLATtoUINT(double dParam, int nType)
{
	UINT unTemp = 0;
	if (nType == 1)// 经度  -> unint
	{
		//经度
		unTemp = dParam * 1000000.0 + 180.0 * 1000000.0;

	}
	else if(nType == 2)// 纬度  -> unint
	{
		//纬度
		unTemp = dParam * 1000000.0 + 90.0 * 1000000.0;
	}
	return unTemp;
}


void FireCalibrateDlg::OnCbnSelchangeComboPointid()//参考点
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
	int npointid = m_combo_pointid.GetCurSel();
	NET_LOCATION_CALIBRATE_POINT_INFO stuCalibratePoint = stuLocationCalibrateInfo.stuPointInfo[npointid];
	m_name = stuCalibratePoint.szName;
	double m_dLongitude = UINTToLONLAT(stuCalibratePoint.nLongitude,1);//经度
	double m_dLatitude = UINTToLONLAT(stuCalibratePoint.nLatitude,2);//纬度
	double m_dAltitude = stuCalibratePoint.fAltitude;		//海拔
	m_strLongitude.Format("%.06f",m_dLongitude);
	m_strLatitude.Format("%.06f",m_dLatitude);
	m_strAltitude.Format("%.02f",m_dAltitude);

	m_combo_calid.ResetContent();
	m_strx1=_T("");
	m_stry1=_T("");
	m_strx2=_T("");
	m_stry2=_T("");
	UpdateData(FALSE);
	int ncalibrateinfonum = stuCalibratePoint.stuCalibrateInfo.nInfoNum;
	if (ncalibrateinfonum > 0)
	{
		
		for (int i=0;i<ncalibrateinfonum;i++)
		{
			CString str;
			str.Format("%d",i+1);
			m_combo_calid.AddString(str);
		}
		m_combo_calid.SetCurSel(0);

		NET_CALIBRATE_INFO stuCalibrateInfo = stuCalibratePoint.stuCalibrateInfo;
		m_strx1.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][0].nLocation[0]);
		m_stry1.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][0].nLocation[1]);
		m_strx2.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][1].nLocation[0]);
		m_stry2.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[0][1].nLocation[1]);
	}
	UpdateData(FALSE);
}

void FireCalibrateDlg::OnCbnSelchangeComboCalid()//标定点
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
	int npointid = m_combo_pointid.GetCurSel();
	int ncalid = m_combo_calid.GetCurSel();

	NET_LOCATION_CALIBRATE_POINT_INFO stuCalibratePoint = stuLocationCalibrateInfo.stuPointInfo[npointid];
	NET_CALIBRATE_INFO stuCalibrateInfo = stuCalibratePoint.stuCalibrateInfo;
	m_strx1.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[ncalid][0].nLocation[0]);
	m_stry1.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[ncalid][0].nLocation[1]);
	m_strx2.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[ncalid][1].nLocation[0]);
	m_stry2.Format("%d", stuCalibrateInfo.stuCalibrateUnitInfo[ncalid][1].nLocation[1]);
	UpdateData(FALSE);
	
}
