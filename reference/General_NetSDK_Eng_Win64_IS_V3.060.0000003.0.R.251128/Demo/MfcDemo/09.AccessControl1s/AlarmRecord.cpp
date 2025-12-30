// AlarmRecord.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "AlarmRecord.h"

IMPLEMENT_DYNAMIC(AlarmRecord, CDialog)

AlarmRecord::AlarmRecord(CWnd* pParent /*=NULL*/)
	: CDialog(AlarmRecord::IDD, pParent)
{

}

AlarmRecord::~AlarmRecord()
{
}

void AlarmRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ALARMREC, m_listAlarmRec);
}


BEGIN_MESSAGE_MAP(AlarmRecord, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GETRECCOUNT, &AlarmRecord::OnBnClickedButtonGetreccount)
	ON_BN_CLICKED(IDC_BUTTON_QUERYREC, &AlarmRecord::OnBnClickedButtonQueryrec)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_REQUERY, &AlarmRecord::OnBnClickedButtonRequery)
END_MESSAGE_MAP()


BOOL AlarmRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		Device::GetInstance().AlarmStartFind();
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


void AlarmRecord::UpdateRecordListCtrlInfo( NET_RECORD_ACCESS_ALARMRECORD_INFO* pstuAlarm)
{
	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = "";
	lvi.iImage = 0;
	lvi.iItem = 0;
	m_listAlarmRec.InsertItem(&lvi);	
	m_listAlarmRec.SetItemText(0, 0, (Int2Str(pstuAlarm->nRecNo)).c_str());
	m_listAlarmRec.SetItemText(0, 1, (NetTime2Str(pstuAlarm->stuTime)).c_str());
	m_listAlarmRec.SetItemText(0, 2, pstuAlarm->szUserID);
	m_listAlarmRec.SetItemText(0, 3, (AccessAlarmType2Str(pstuAlarm->emAlarmType)).c_str());
	m_listAlarmRec.SetItemText(0, 4, (Int2Str(pstuAlarm->nDevAddress)).c_str());
	m_listAlarmRec.SetItemText(0, 5, (Int2Str(pstuAlarm->nChannel)).c_str());
	
}
void AlarmRecord::ClearListCtrlInfo()
{
	m_listAlarmRec.DeleteAllItems();
}
void AlarmRecord::InitDlg()
{
	m_listAlarmRec.SetExtendedStyle(m_listAlarmRec.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listAlarmRec.SetExtendedStyle(m_listAlarmRec.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("RecNo", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("UserID", 80));
	vecTitles.push_back(ColDesObj("AlarmType", 100));
	vecTitles.push_back(ColDesObj("DevAddress", 60));
	vecTitles.push_back(ColDesObj("Channel", 260));
	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_listAlarmRec.InsertColumn(j, &lvc);
	}
}
void AlarmRecord::OnBnClickedButtonGetreccount()
{
	int ncount = 0;
	BOOL bret = Device::GetInstance().AlarmQueryCount(ncount);
	if (bret)
	{
		SetDlgItemInt(IDC_EDIT_ALARMRECCOUNT, ncount);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void AlarmRecord::OnBnClickedButtonQueryrec()
{
	NET_RECORD_ACCESS_ALARMRECORD_INFO* pstuAlarm = new NET_RECORD_ACCESS_ALARMRECORD_INFO[10];
	if (NULL == pstuAlarm)
	{
		return;
	}
	memset(pstuAlarm, 0, sizeof(NET_RECORD_ACCESS_ALARMRECORD_INFO) * 10);

	for (int i = 0; i < 10; i++)
	{
		pstuAlarm[i].dwSize = sizeof(NET_RECORD_ACCESS_ALARMRECORD_INFO);
	}
	int nRecordNum = 0;
	BOOL bret = Device::GetInstance().AlarmFindNext(10, pstuAlarm, nRecordNum);
	if (bret)
	{
		ClearListCtrlInfo();
		for (int i=0;i<nRecordNum;i++)
		{
			UpdateRecordListCtrlInfo(&pstuAlarm[i]);
		}
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	delete []pstuAlarm;
	pstuAlarm = NULL;
}

void AlarmRecord::OnDestroy()
{
	CDialog::OnDestroy();
	Device::GetInstance().AlarmStopFind();
}

void AlarmRecord::OnBnClickedButtonRequery()
{
	BOOL bret = Device::GetInstance().AlarmStopFind();

	bret = Device::GetInstance().AlarmStartFind();
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
