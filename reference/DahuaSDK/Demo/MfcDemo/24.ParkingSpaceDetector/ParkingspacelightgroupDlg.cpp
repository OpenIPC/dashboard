// ParkingspacelightgroupDlg.cpp : 实现文件
//

#include "StdAfx.h"
#include "ParkingSpaceDetector.h"
#include "ParkingspacelightgroupDlg.h"
#include "Controller.h"


// ParkingspacelightgroupDlg 对话框

IMPLEMENT_DYNAMIC(ParkingspacelightgroupDlg, CDialog)

ParkingspacelightgroupDlg::ParkingspacelightgroupDlg(CController* pCtl, CWnd* pParent /*=NULL*/)
	: CDialog(ParkingspacelightgroupDlg::IDD, pParent),
	m_pCtl(pCtl),
	m_nCfgNum(0),
	m_nLanesNum(0),
	m_row(0),
	m_column(0)
{
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}

ParkingspacelightgroupDlg::~ParkingspacelightgroupDlg()
{
}

void ParkingspacelightgroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LIGHTGROUP_LIST, m_listLightGroup);
	DDX_Control(pDX, IDC_EDIT2, m_edit);
}


BEGIN_MESSAGE_MAP(ParkingspacelightgroupDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GET, &ParkingspacelightgroupDlg::OnBnClickedButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, &ParkingspacelightgroupDlg::OnBnClickedButtonSet)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_LIGHTGROUP_LIST, &ParkingspacelightgroupDlg::OnNMDblclkListLightgroupList)
	ON_EN_KILLFOCUS(IDC_EDIT2, &ParkingspacelightgroupDlg::OnEnKillfocusEdit2)
END_MESSAGE_MAP()


// ParkingspacelightgroupDlg 消息处理程序
BOOL ParkingspacelightgroupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	getConfig();
	InitControl();
	showDataToControl();
	return TRUE;  // return TRUE unless you set the focus to a control
}

void ParkingspacelightgroupDlg::InitControl()
{
	m_listLightGroup.SetExtendedStyle
		(m_listLightGroup.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);	

	m_listLightGroup.InsertColumn(0,ConvertString("Light Type"), LVCFMT_LEFT, 80, 0);

	CString strPrkingLot;
	m_nCfgNum = m_stuInfo.nCfgNum;
	m_nLanesNum = m_stuInfo.stuLightGroupInfo[0].nLanesNum;
	for (int i = 0; i < m_stuInfo.stuLightGroupInfo[0].nLanesNum; i++)
	{
		strPrkingLot.Format("%s %d", ConvertString("Parking Lot"), i);
		m_listLightGroup.InsertColumn(i+1, strPrkingLot, LVCFMT_LEFT, 80, 1);
	}

	m_edit.ShowWindow(SW_HIDE);
}

void ParkingspacelightgroupDlg::OnBnClickedButtonGet()
{
	getConfig();
	showDataToControl();
}

void ParkingspacelightgroupDlg::OnBnClickedButtonSet()
{
	getDataFromControl();
	setConfig();
	showDataToControl();
}

void ParkingspacelightgroupDlg::getConfig()
{
	int nError = 0;
	char szJsonBuf[1024 * 50] = {0};
	int nChannelID = -1;

	if (m_pCtl->GetParkingSpaceLightGroupInfo(&m_stuInfo) == false)
	{
		MessageBox(ConvertString("Get config failed"),ConvertString("Prompt"));
	}
}

void ParkingspacelightgroupDlg::setConfig()
{
	if (m_pCtl->SetParkingSpaceLightGroupInfo(&m_stuInfo) == false)
	{
		MessageBox(ConvertString("Set config failed"),ConvertString("Prompt"));
		return ;
	}
}

void ParkingspacelightgroupDlg::getDataFromControl()
{
	int nCfgNum = min(MAX_LIGHT_GROUP_INFO_NUM, m_nCfgNum);
	for (int i = 0; i < nCfgNum; i++)
	{
		m_stuInfo.nCfgNum = nCfgNum;
		m_stuInfo.stuLightGroupInfo[i].bEnable = TRUE;
		m_stuInfo.stuLightGroupInfo[i].nLanesNum = m_nLanesNum;

		for (int n = 0; n < m_nLanesNum; n++)
		{
			CString str = m_listLightGroup.GetItemText(i, n+1);

			if (str.Compare("0") != 0 && str.Compare("1") != 0 )
			{
				MessageBox(ConvertString("Input error"),ConvertString("Prompt"));
				return;
			}
			int nState = atoi(str);
			m_stuInfo.stuLightGroupInfo[i].emLaneStatus[n] = nState == 0?EM_CFG_LANE_STATUS_UNSUPERVISE:EM_CFG_LANE_STATUS_SUPERVISE;
		}
	}
}

void ParkingspacelightgroupDlg::showDataToControl()
{
	m_listLightGroup.DeleteAllItems();
	if (m_stuInfo.nCfgNum <= 0)
	{
		return;
	}

	for (int i=0;i<m_stuInfo.nCfgNum;i++)
	{
		if (i == 0)
		{
			m_listLightGroup.InsertItem(0, ConvertString("Build-in lamp"));
		}
		else
		{
			CString str;
			str.Format("%s %d",ConvertString("External lamp"), i);

			m_listLightGroup.InsertItem(i, str);
		}

		if (m_stuInfo.stuLightGroupInfo[i].bEnable)
		{
			for (int n = 0; n < m_stuInfo.stuLightGroupInfo[i].nLanesNum; n++)
			{
				if (m_stuInfo.stuLightGroupInfo[i].emLaneStatus[n] == EM_CFG_LANE_STATUS_SUPERVISE)
				{
					m_listLightGroup.SetItemText(i, n+1, "1");
				}
				else
				{
					m_listLightGroup.SetItemText(i, n+1, "0");
				}
			}
		}
	}
}

void ParkingspacelightgroupDlg::OnNMDblclkListLightgroupList(NMHDR *pNMHDR, LRESULT *pResult)
{
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
		m_listLightGroup.GetSubItemRect(m_row,m_column,LVIR_LABEL,rc);
		m_edit.SetParent(&m_listLightGroup);
		m_edit.MoveWindow(rc);
		m_edit.SetWindowText(m_listLightGroup.GetItemText(m_row,m_column));
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetFocus();
		m_edit.ShowCaret();
		m_edit.SetSel(-1);
	}
}

void ParkingspacelightgroupDlg::OnEnKillfocusEdit2()
{
	CString str;
	m_edit.GetWindowText(str);
	m_listLightGroup.SetItemText(m_row,m_column,str);
	m_edit.ShowWindow(SW_HIDE);
}
