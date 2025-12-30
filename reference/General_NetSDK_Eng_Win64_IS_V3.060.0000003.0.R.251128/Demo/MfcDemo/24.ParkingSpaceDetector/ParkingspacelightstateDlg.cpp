// ParkingspacelightstateDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ParkingSpaceDetector.h"
#include "ParkingspacelightstateDlg.h"
#include "Controller.h"


// ParkingspacelightstateDlg 对话框
CString g_LightColor[] = {"Red", "Yellow", "Blue", "Green", "Purple", "White", "Pink", "Close"};

IMPLEMENT_DYNAMIC(ParkingspacelightstateDlg, CDialog)

ParkingspacelightstateDlg::ParkingspacelightstateDlg(CController* pCtl, CWnd* pParent /*=NULL*/)
	: CDialog(ParkingspacelightstateDlg::IDD, pParent)
	,m_pCtl(pCtl)
{
	memset(&m_stuParkLightState, 0, sizeof(m_stuParkLightState));
	m_stuParkLightState.dwSize = sizeof(m_stuParkLightState);
}

ParkingspacelightstateDlg::~ParkingspacelightstateDlg()
{
}

void ParkingspacelightstateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_SPACEFREE, m_combo_spacefree);
	DDX_Control(pDX, IDC_COMBO_SPACEFULL, m_combo_spacefull);
	DDX_Control(pDX, IDC_COMBO_NETPORT1ABORT, m_combo_netport1abort);
	DDX_Control(pDX, IDC_COMBO_NETPORT2ABORT, m_combo_netport2abort);
}


BEGIN_MESSAGE_MAP(ParkingspacelightstateDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SETSTATE, &ParkingspacelightstateDlg::OnBnClickedButtonSetstate)
	ON_BN_CLICKED(IDC_BUTTON_GETSTATE, &ParkingspacelightstateDlg::OnBnClickedButtonGetstate)
END_MESSAGE_MAP()


// ParkingspacelightstateDlg 消息处理程序
BOOL ParkingspacelightstateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	getConfig();
	InitControl();
	showDataToControl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void ParkingspacelightstateDlg::InitControl()
{
	for (int i = 0; i < sizeof(g_LightColor) / sizeof(g_LightColor[0]); i++)
	{
		m_combo_spacefree.AddString(ConvertString(g_LightColor[i]));
		m_combo_spacefull.AddString(ConvertString(g_LightColor[i]));
		m_combo_netport1abort.AddString(ConvertString(g_LightColor[i]));
		m_combo_netport2abort.AddString(ConvertString(g_LightColor[i]));
	}
}

void ParkingspacelightstateDlg::OnBnClickedButtonSetstate()
{
	getDataFromControl();
	setConfig();
}

void ParkingspacelightstateDlg::OnBnClickedButtonGetstate()
{
	getConfig();
	showDataToControl();

}

void ParkingspacelightstateDlg::showDataToControl()
{
	setComboBoxInfoFromLightInfo(m_stuParkLightState.stuSpaceFreeInfo, &m_combo_spacefree, IDC_CHECK_FREE);
	setComboBoxInfoFromLightInfo(m_stuParkLightState.stuSpaceFullInfo, &m_combo_spacefull, IDC_CHECK_FULL);
	setComboBoxInfoFromLightInfo(m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[0], &m_combo_netport1abort, IDC_CHECK_SIGNAL);
	setComboBoxInfoFromLightInfo(m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[1], &m_combo_netport2abort, IDC_CHECK_DOUBLE);
}

void ParkingspacelightstateDlg::setComboBoxInfoFromLightInfo(NET_PARKINGSPACELIGHT_INFO lightInfo, CComboBox *comBox, int nCheckID)
{	
	if (lightInfo.nRed >= 1)
	{
		int nCheck = lightInfo.nRed - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);
		
		comBox->SetCurSel(0);
		return ;
	}
	if (lightInfo.nYellow >= 1)
	{
		int nCheck = lightInfo.nYellow - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);
		
		comBox->SetCurSel(1);
		return ;
	}
	if (lightInfo.nBlue >= 1)
	{
		int nCheck = lightInfo.nBlue - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);

		comBox->SetCurSel(2);
		return ;
	}
	if (lightInfo.nGreen >= 1)
	{
		int nCheck = lightInfo.nGreen - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);

		comBox->SetCurSel(3);
		return ;
	}
	if (lightInfo.nPurple >= 1)
	{
		int nCheck = lightInfo.nPurple - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);

		comBox->SetCurSel(4);
		return ;
	}
	if (lightInfo.nWhite >= 1)
	{
		int nCheck = lightInfo.nWhite - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);

		comBox->SetCurSel(5);
		return ;
	}
	if (lightInfo.nPink >= 1)
	{
		int nCheck = lightInfo.nPink - 1;	
		((CButton*)GetDlgItem(nCheckID))->SetCheck(nCheck);

		comBox->SetCurSel(6);
		return ;
	}
	comBox->SetCurSel(7);
	((CButton*)GetDlgItem(nCheckID))->SetCheck(0);
}

void ParkingspacelightstateDlg::getDataFromControl()
{	
	m_stuParkLightState.dwSize = sizeof(m_stuParkLightState);

	m_stuParkLightState.stuNetWorkExceptionInfo.nRetNetPortAbortNum = 2;	//At present, the device only supports dual network ports

	memset(&m_stuParkLightState.stuSpaceFreeInfo, 0, sizeof(m_stuParkLightState.stuSpaceFreeInfo));
	memset(&m_stuParkLightState.stuSpaceFullInfo, 0, sizeof(m_stuParkLightState.stuSpaceFullInfo));
	memset(&m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[0], 0, sizeof(m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[0]));
	memset(&m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[1], 0, sizeof(m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[1]));

	getLightInfoFromComBox(m_stuParkLightState.stuSpaceFreeInfo, &m_combo_spacefree, IDC_CHECK_FREE);
	getLightInfoFromComBox(m_stuParkLightState.stuSpaceFullInfo, &m_combo_spacefull, IDC_CHECK_FULL);
	getLightInfoFromComBox(m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[0], &m_combo_netport1abort, IDC_CHECK_SIGNAL);
	getLightInfoFromComBox(m_stuParkLightState.stuNetWorkExceptionInfo.stNetPortAbortInfo[1], &m_combo_netport2abort, IDC_CHECK_DOUBLE);
}

void ParkingspacelightstateDlg::getLightInfoFromComBox(NET_PARKINGSPACELIGHT_INFO &lightInfo, CComboBox *comBox, int nCheckID)
{
	int nSel = comBox->GetCurSel();

	int nCheck = ((CButton*)GetDlgItem(nCheckID))->GetCheck();
	switch(nSel)
	{
	case 0:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nRed = 2;
			}
			else
			{
				lightInfo.nRed = 1;
			}
			break;
		}
	case 1:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nYellow = 2;
			}
			else
			{
				lightInfo.nYellow = 1;
			}
			break;
		}
	case 2:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nBlue = 2;
			}
			else
			{
				lightInfo.nBlue = 1;
			}
			break;
		}
	case 3:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nGreen = 2;
			}
			else
			{
				lightInfo.nGreen = 1;
			}
			break;
		}
	case 4:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nPurple = 2;
			}
			else
			{
				lightInfo.nPurple = 1;
			}
			break;
		}
	case 5:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nWhite = 2;
			}
			else
			{
				lightInfo.nWhite = 1;
			}
			break;
		}
	case 6:
		{
			if (BST_CHECKED == nCheck)
			{
				lightInfo.nPink = 2;
			}
			else
			{
				lightInfo.nPink = 1;
			}
			break;
		}
	default:
		break;
	}
}

void ParkingspacelightstateDlg::setConfig()
{
	if (m_pCtl->SetParkingSpaceLightState(&m_stuParkLightState) == false)
	{
		MessageBox(ConvertString("Set config failed"),ConvertString("Prompt"));
	}
}

void ParkingspacelightstateDlg::getConfig()
{
	memset(&m_stuParkLightState, 0, sizeof(m_stuParkLightState));
	m_stuParkLightState.dwSize = sizeof(m_stuParkLightState);
	if (m_pCtl->GetParkingSpaceLightState(&m_stuParkLightState) == false)
	{
		MessageBox(ConvertString("Get config failed"),ConvertString("Prompt"));
	}
}