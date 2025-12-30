// DoorRecord.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "DoorRecord.h"

IMPLEMENT_DYNAMIC(DoorRecord, CDialog)

DoorRecord::DoorRecord(CWnd* pParent /*=NULL*/)
	: CDialog(DoorRecord::IDD, pParent)
{

}

DoorRecord::~DoorRecord()
{
}

void DoorRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DOORREC, m_listDoorRec);
}


BEGIN_MESSAGE_MAP(DoorRecord, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GETRECCOUNT, &DoorRecord::OnBnClickedButtonGetreccount)
	ON_BN_CLICKED(IDC_BUTTON_QUERYREC, &DoorRecord::OnBnClickedButtonQueryrec)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_REQUERY, &DoorRecord::OnBnClickedButtonRequery)
END_MESSAGE_MAP()


BOOL DoorRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		Device::GetInstance().CardRecStartFind();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_GETRECCOUNT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_QUERYREC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_REQUERY)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void DoorRecord::InitDlg()
{
	m_listDoorRec.SetExtendedStyle(m_listDoorRec.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listDoorRec.SetExtendedStyle(m_listDoorRec.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("RecNo", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("CardNo", 80));
	vecTitles.push_back(ColDesObj("Status", 100));
	vecTitles.push_back(ColDesObj("Door", 60));
	vecTitles.push_back(ColDesObj("Method", 260));
	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_listDoorRec.InsertColumn(j, &lvc);
	}
}

void DoorRecord::OnBnClickedButtonGetreccount()
{
	int ncount = 0;
	BOOL bret = Device::GetInstance().CardRecQueryCount(ncount);
	if (bret)
	{
		SetDlgItemInt(IDC_EDIT_DOORRECCOUNT, ncount);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void DoorRecord::OnBnClickedButtonQueryrec()
{
	NET_RECORDSET_ACCESS_CTL_CARDREC* pstuCardRec = new NET_RECORDSET_ACCESS_CTL_CARDREC[10];
	if (NULL == pstuCardRec)
	{
		return;
	}
	memset(pstuCardRec, 0, sizeof(NET_RECORDSET_ACCESS_CTL_CARDREC) * 10);

	for (int i = 0; i < 10; i++)
	{
		pstuCardRec[i].dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARDREC);
	}
	int nRecordNum = 0;
	BOOL bret = Device::GetInstance().CardRecFindNext(10, pstuCardRec, nRecordNum);
	if (bret)
	{
		ClearListCtrlInfo();
// 		for (int i=0;i<nRecordNum;i++)
// 		{
// 			UpdateRecordListCtrlInfo(&pstuCardRec[i]);
// 		}
		for (int i=nRecordNum;i>0;i--)
		{
			UpdateRecordListCtrlInfo(&pstuCardRec[i-1]);
		}
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	delete []pstuCardRec;
	pstuCardRec = NULL;
}

void DoorRecord::UpdateRecordListCtrlInfo( NET_RECORDSET_ACCESS_CTL_CARDREC* pstuCardRec)
{
	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = "";
	lvi.iImage = 0;
	lvi.iItem = 0;
	m_listDoorRec.InsertItem(&lvi);	
	m_listDoorRec.SetItemText(0, 0, (Int2Str(pstuCardRec->nRecNo)).c_str());
	m_listDoorRec.SetItemText(0, 1, (NetTime2Str(pstuCardRec->stuTime)).c_str());
	m_listDoorRec.SetItemText(0, 2, pstuCardRec->szCardNo);
	m_listDoorRec.SetItemText(0, 3, pstuCardRec->bStatus ? "True":"False");
	m_listDoorRec.SetItemText(0, 4, (Int2Str(pstuCardRec->nDoor)).c_str());
	m_listDoorRec.SetItemText(0, 5, (AccessDoorOpenMethod2Str(pstuCardRec->emMethod)).c_str());
}

void DoorRecord::ClearListCtrlInfo()
{
	m_listDoorRec.DeleteAllItems();
}

void DoorRecord::OnDestroy()
{
	CDialog::OnDestroy();
	Device::GetInstance().CardRecStopFind();
}

void DoorRecord::OnBnClickedButtonRequery()
{
	BOOL bret = Device::GetInstance().CardRecStopFind();
	if (bret)
	{
		bret = Device::GetInstance().CardRecStartFind();
		if (bret)
		{
			OnBnClickedButtonQueryrec();
		}
		else
		{
			CString csInfo;
			csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}
