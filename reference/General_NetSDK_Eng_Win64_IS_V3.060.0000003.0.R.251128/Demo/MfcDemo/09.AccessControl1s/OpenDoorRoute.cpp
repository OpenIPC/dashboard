// OpenDoorRoute.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "OpenDoorRoute.h"

IMPLEMENT_DYNAMIC(OpenDoorRoute, CDialog)

OpenDoorRoute::OpenDoorRoute(CWnd* pParent /*=NULL*/)
	: CDialog(OpenDoorRoute::IDD, pParent)
{
	memset(&stuInfo,0,sizeof(stuInfo));
}

OpenDoorRoute::~OpenDoorRoute()
{
}

void OpenDoorRoute::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(OpenDoorRoute, CDialog)
	ON_BN_CLICKED(IDC_OPENDOORROUTE_BUT_SET, &OpenDoorRoute::OnBnClickedOpendoorrouteButSet)
	ON_BN_CLICKED(IDC_OPENDOORROUTE_BUT_GET, &OpenDoorRoute::OnBnClickedOpendoorrouteButGet)
END_MESSAGE_MAP()


BOOL OpenDoorRoute::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedOpendoorrouteButGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_OPENDOORROUTE_BUT_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPENDOORROUTE_BUT_GET)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void OpenDoorRoute::InitDlg()
{

}

void OpenDoorRoute::SetOpenDoorRouteInfoToCtrl()
{
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_RESETTIME, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID0, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID1, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID2, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID3, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID4, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID5, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID6, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID7, "");

	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_RESETTIME, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID0, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID1, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID2, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID3, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID4, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID5, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID6, "");
	SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID7, "");

	if (stuInfo.nDoorList >= 1)
	{
		SetDlgItemInt(IDC_OPENDOORROUTE_EDIT_0_RESETTIME, stuInfo.stuDoorList[0].nResetTime, FALSE);
		if (stuInfo.stuDoorList[0].nDoors >= 1)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID0, stuInfo.stuDoorList[0].stuDoors[0].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 2)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID1, stuInfo.stuDoorList[0].stuDoors[1].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 3)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID2, stuInfo.stuDoorList[0].stuDoors[2].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 4)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID3, stuInfo.stuDoorList[0].stuDoors[3].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 5)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID4, stuInfo.stuDoorList[0].stuDoors[4].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 6)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID5, stuInfo.stuDoorList[0].stuDoors[5].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 7)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID6, stuInfo.stuDoorList[0].stuDoors[6].szReaderID);
		}
		if (stuInfo.stuDoorList[0].nDoors >= 8)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID7, stuInfo.stuDoorList[0].stuDoors[7].szReaderID);
		}
	}
	if (stuInfo.nDoorList >= 2)
	{
		SetDlgItemInt(IDC_OPENDOORROUTE_EDIT_1_RESETTIME, stuInfo.stuDoorList[1].nResetTime, FALSE);
		if (stuInfo.stuDoorList[1].nDoors >= 1)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID0, stuInfo.stuDoorList[1].stuDoors[0].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 2)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID1, stuInfo.stuDoorList[1].stuDoors[1].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 3)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID2, stuInfo.stuDoorList[1].stuDoors[2].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 4)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID3, stuInfo.stuDoorList[1].stuDoors[3].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 5)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID4, stuInfo.stuDoorList[1].stuDoors[4].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 6)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID5, stuInfo.stuDoorList[1].stuDoors[5].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 7)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID6, stuInfo.stuDoorList[1].stuDoors[6].szReaderID);
		}
		if (stuInfo.stuDoorList[1].nDoors >= 8)
		{
			SetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID7, stuInfo.stuDoorList[1].stuDoors[7].szReaderID);
		}
	}
}

void OpenDoorRoute::GetOpenDoorRouteInfoFromCtrl()
{
	CString strT1;
	GetDlgItem(IDC_OPENDOORROUTE_EDIT_0_RESETTIME)->GetWindowText(strT1);
	if (!strT1.IsEmpty())
	{
		stuInfo.nDoorList = 1;
		stuInfo.stuDoorList[0].nResetTime = GetDlgItemInt(IDC_OPENDOORROUTE_EDIT_0_RESETTIME, NULL, 1);

		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID0,stuInfo.stuDoorList[0].stuDoors[0].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID1,stuInfo.stuDoorList[0].stuDoors[1].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID2,stuInfo.stuDoorList[0].stuDoors[2].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID3,stuInfo.stuDoorList[0].stuDoors[3].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID4,stuInfo.stuDoorList[0].stuDoors[4].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID5,stuInfo.stuDoorList[0].stuDoors[5].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID6,stuInfo.stuDoorList[0].stuDoors[6].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_0_READERID7,stuInfo.stuDoorList[0].stuDoors[7].szReaderID,MAX_READER_ID_LEN);
		stuInfo.stuDoorList[0].nDoors = 8;
		for (int i=0;i<8;i++)
		{
			CString strTemp = stuInfo.stuDoorList[0].stuDoors[i].szReaderID;
			if (strTemp.IsEmpty())
			{
				stuInfo.stuDoorList[0].nDoors = i;
				break;
			}
		}
	}
	CString strT2;
	GetDlgItem(IDC_OPENDOORROUTE_EDIT_1_RESETTIME)->GetWindowText(strT2);
	if (!strT2.IsEmpty())
	{
		stuInfo.nDoorList = 2;
		stuInfo.stuDoorList[1].nResetTime = GetDlgItemInt(IDC_OPENDOORROUTE_EDIT_1_RESETTIME, NULL, 1);

		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID0,stuInfo.stuDoorList[1].stuDoors[0].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID1,stuInfo.stuDoorList[1].stuDoors[1].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID2,stuInfo.stuDoorList[1].stuDoors[2].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID3,stuInfo.stuDoorList[1].stuDoors[3].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID4,stuInfo.stuDoorList[1].stuDoors[4].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID5,stuInfo.stuDoorList[1].stuDoors[5].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID6,stuInfo.stuDoorList[1].stuDoors[6].szReaderID,MAX_READER_ID_LEN);
		GetDlgItemText(IDC_OPENDOORROUTE_EDIT_1_READERID7,stuInfo.stuDoorList[1].stuDoors[7].szReaderID,MAX_READER_ID_LEN);
		stuInfo.stuDoorList[1].nDoors = 8;
		for (int i=0;i<8;i++)
		{
			CString strTemp = stuInfo.stuDoorList[1].stuDoors[i].szReaderID;
			if (strTemp.IsEmpty())
			{
				stuInfo.stuDoorList[1].nDoors = i;
				break;
			}
		}
	}
}

void OpenDoorRoute::OnBnClickedOpendoorrouteButSet()
{
	GetOpenDoorRouteInfoFromCtrl();
	BOOL bret = Device::GetInstance().SetOpenDoorRouteInfo(&stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void OpenDoorRoute::OnBnClickedOpendoorrouteButGet()
{
	BOOL bret = Device::GetInstance().GetOpenDoorRouteInfo(stuInfo);
	if (bret)
	{
		SetOpenDoorRouteInfoToCtrl();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}
