// DoorRecord.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
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
	DDX_Control(pDX, IDC_RECORD_DOOR_DTP_VDSTART, m_dtpVDStart);
	DDX_Control(pDX, IDC_RECORD_DOOR_DTP_VTSTART, m_dtpVTStart);
	DDX_Control(pDX, IDC_RECORD_DOOR_DTP_VDEND, m_dtpVDEnd);
	DDX_Control(pDX, IDC_RECORD_DOOR_DTP_VTEND, m_dtpVTEnd);
}


BEGIN_MESSAGE_MAP(DoorRecord, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GETRECCOUNT, &DoorRecord::OnBnClickedButtonGetreccount)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_STARTQUERY, &DoorRecord::OnBnClickedButtonStartquery)
	ON_BN_CLICKED(IDC_BUTTON_NEXTPAGE, &DoorRecord::OnBnClickedButtonNextpage)
	ON_BN_CLICKED(IDC_BUTTON_STOPQUERY, &DoorRecord::OnBnClickedButtonStopquery)
END_MESSAGE_MAP()


BOOL DoorRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	UIState(FALSE);
	if (!Device::GetInstance().GetLoginState())
	{
		GetDlgItem(IDC_BUTTON_STARTQUERY)->EnableWindow(FALSE);
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

	SYSTEMTIME time;
	GetSystemTime(&time);
	m_dtpVDStart.SetTime(&time);
	time.wHour    = (WORD)0;
	time.wMinute  = (WORD)0;
	time.wSecond  = (WORD)0;
	m_dtpVTStart.SetTime(&time);
	m_dtpVDEnd.SetTime(&time);
	time.wHour    = (WORD)23;
	time.wMinute  = (WORD)59;
	time.wSecond  = (WORD)59;
	m_dtpVTEnd.SetTime(&time);
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

void DoorRecord::OnBnClickedButtonStartquery()
{
	SYSTEMTIME stStart = {0};
	SYSTEMTIME stEnd = {0};
	SYSTEMTIME stTemp = {0};
	
	m_dtpVDStart.GetTime(&stTemp);
	stStart.wYear = stTemp.wYear;
	stStart.wMonth = stTemp.wMonth;
	stStart.wDay = stTemp.wDay;
	m_dtpVTStart.GetTime(&stTemp);
	stStart.wHour = stTemp.wHour;
	stStart.wMinute = stTemp.wMinute;
	stStart.wSecond = stTemp.wSecond;

	m_dtpVDEnd.GetTime(&stTemp);
	stEnd.wYear = stTemp.wYear;
	stEnd.wMonth = stTemp.wMonth;
	stEnd.wDay = stTemp.wDay;
	m_dtpVTEnd.GetTime(&stTemp);
	stEnd.wHour = stTemp.wHour;
	stEnd.wMinute = stTemp.wMinute;
	stEnd.wSecond = stTemp.wSecond;

	BOOL bret = Device::GetInstance().CardRecStartFind(stStart, stEnd);
	if (bret)
	{
		OnBnClickedButtonNextpage();
		UIState(TRUE);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Start Find failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void DoorRecord::OnBnClickedButtonNextpage()
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

void DoorRecord::OnBnClickedButtonStopquery()
{
	Device::GetInstance().CardRecStopFind();
	UIState(FALSE);
}

void DoorRecord::UIState(BOOL b)
{
	GetDlgItem(IDC_RECORD_DOOR_DTP_VDSTART)->EnableWindow(!b);
	GetDlgItem(IDC_RECORD_DOOR_DTP_VTSTART)->EnableWindow(!b);
	GetDlgItem(IDC_RECORD_DOOR_DTP_VDEND)->EnableWindow(!b);
	GetDlgItem(IDC_RECORD_DOOR_DTP_VTEND)->EnableWindow(!b);

	GetDlgItem(IDC_BUTTON_STARTQUERY)->EnableWindow(!b);
	GetDlgItem(IDC_BUTTON_NEXTPAGE)->EnableWindow(b);
	GetDlgItem(IDC_BUTTON_STOPQUERY)->EnableWindow(b);

	GetDlgItem(IDC_BUTTON_GETRECCOUNT)->EnableWindow(b);
	GetDlgItem(IDC_EDIT_DOORRECCOUNT)->EnableWindow(FALSE);
}