// Log.cpp
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "Log.h"

IMPLEMENT_DYNAMIC(Log, CDialog)

Log::Log(CWnd* pParent /*=NULL*/)
	: CDialog(Log::IDD, pParent)
{
	m_LogNo = 0;
}

Log::~Log()
{
}

void Log::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOGREC, m_listLogRec);
}


BEGIN_MESSAGE_MAP(Log, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_GETRECCOUNT, &Log::OnBnClickedButtonGetreccount)
	ON_BN_CLICKED(IDC_BUTTON_QUERYREC, &Log::OnBnClickedButtonQueryrec)
	ON_BN_CLICKED(IDC_BUTTON_REQUERY, &Log::OnBnClickedButtonRequery)
END_MESSAGE_MAP()

BOOL Log::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		Device::GetInstance().LogStartFind();
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

void Log::OnDestroy()
{
	CDialog::OnDestroy();
	Device::GetInstance().LogStopFind();
}

void Log::UpdateRecordListCtrlInfo( NET_LOG_INFO* pstuLog)
{
	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = "";
	lvi.iImage = 0;
	lvi.iItem = 0;
	m_listLogRec.InsertItem(&lvi);	
	m_listLogRec.SetItemText(0, 0, (Int2Str(m_LogNo++)).c_str());
	m_listLogRec.SetItemText(0, 1, (NetTime2Str(pstuLog->stuTime)).c_str());
	m_listLogRec.SetItemText(0, 2, pstuLog->szUserName);
	m_listLogRec.SetItemText(0, 3, pstuLog->szLogType);
	m_listLogRec.SetItemText(0, 4, pstuLog->stuLogMsg.szLogMessage);
}
void Log::ClearListCtrlInfo()
{
	m_listLogRec.DeleteAllItems();
}
void Log::InitDlg()
{
	m_listLogRec.SetExtendedStyle(m_listLogRec.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listLogRec.SetExtendedStyle(m_listLogRec.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("RecNo", 60));
	vecTitles.push_back(ColDesObj("Time", 140));
	vecTitles.push_back(ColDesObj("UserName", 80));
	vecTitles.push_back(ColDesObj("LogType", 100));
	vecTitles.push_back(ColDesObj("LogMsg", 260));

	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_listLogRec.InsertColumn(j, &lvc);
	}
}
void Log::OnBnClickedButtonGetreccount()
{
	int ncount = 0;
	BOOL bret = Device::GetInstance().LogQueryCount(ncount);
	if (bret)
	{
		SetDlgItemInt(IDC_EDIT_LOGRECCOUNT, ncount);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void Log::OnBnClickedButtonQueryrec()
{
	NET_LOG_INFO* pstuLog = new NET_LOG_INFO[10];
	if (NULL == pstuLog)
	{
		return;
	}
	memset(pstuLog, 0, sizeof(NET_LOG_INFO) * 10);

	for (int i = 0; i < 10; i++)
	{
		pstuLog[i].dwSize = sizeof(NET_LOG_INFO);
		pstuLog[i].stuLogMsg.dwSize = sizeof(NET_LOG_MESSAGE);
	}
	int nRecordNum = 0;
	BOOL bret = Device::GetInstance().LogFindNext(10, pstuLog, nRecordNum);
	if (bret)
	{
		ClearListCtrlInfo();
		for (int i=0;i<nRecordNum;i++)
		{
			UpdateRecordListCtrlInfo(&pstuLog[i]);
		}
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	delete []pstuLog;
	pstuLog = NULL;
}

void Log::OnBnClickedButtonRequery()
{
	m_LogNo = 0;
	BOOL bret = Device::GetInstance().LogStopFind();

	bret = Device::GetInstance().LogStartFind();
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
