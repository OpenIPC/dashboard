// DevAlarmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Alarm.h"
#include "DevAlarmDlg.h"
#include "../../../Include/Common/dhnetsdk.h"


static const int nMaxAlarmInfoInListCtrl = 100;

// CDevAlarmDlg Dialog

IMPLEMENT_DYNAMIC(CDevAlarmDlg, CDialog)

CDevAlarmDlg::CDevAlarmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDevAlarmDlg::IDD, pParent)
{
	m_nAlarmCount = 0;
}

CDevAlarmDlg::~CDevAlarmDlg()
{
}

void CDevAlarmDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_ALARM, m_listCtrlAlarmInfo);
}


BEGIN_MESSAGE_MAP(CDevAlarmDlg, CDialog)
    ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL CDevAlarmDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    InitListCtrl();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDevAlarmDlg::DestroyWindow()
{
    ClearAlarmInfo();
    return CDialog::DestroyWindow();
}


void CDevAlarmDlg::InitListCtrl()
{
    m_listCtrlAlarmInfo.SetExtendedStyle(m_listCtrlAlarmInfo.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listCtrlAlarmInfo.InsertColumn(0, ConvertString("No."), LVCFMT_LEFT, 80);
	 m_listCtrlAlarmInfo.InsertColumn(1, ConvertString("Time"), LVCFMT_LEFT, 150);
    m_listCtrlAlarmInfo.InsertColumn(2, ConvertString("Channel"), LVCFMT_LEFT, 100);
    m_listCtrlAlarmInfo.InsertColumn(3, ConvertString("Alarm Type"), LVCFMT_LEFT, 150);   
    m_listCtrlAlarmInfo.InsertColumn(4, ConvertString("Status"), LVCFMT_LEFT, 100);	  
}

void CDevAlarmDlg::UpdateAlarmListCtrl(const CAlarmInfoEx& alarmInfo)
{
	m_nAlarmCount ++;

	int nCount = m_listCtrlAlarmInfo.GetItemCount();
	if (nCount >= nMaxAlarmInfoInListCtrl)
	{
		m_listCtrlAlarmInfo.DeleteItem(nCount - 1);	
	}	

	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = _T("");
	lvi.iImage = 0;
	lvi.iItem = 0;

	m_listCtrlAlarmInfo.InsertItem(&lvi);	

	// Index	
	char szIndex[10] = {0};
	_itoa_s(m_nAlarmCount, szIndex, 10);		
	m_listCtrlAlarmInfo.SetItemText(0, 0, szIndex);

	// Time
	m_listCtrlAlarmInfo.SetItemText(0, 1, ConvertString(alarmInfo.m_strTime));

	// Channel
	CString strChannelID;
	strChannelID.Format(ConvertString("Channel %d"), alarmInfo.m_nChannel);
	m_listCtrlAlarmInfo.SetItemText(0, 2, strChannelID);

	// alarm type
	CString strType;
	ConvertAlarmType2CString(alarmInfo.m_nAlarmType, strType);
	m_listCtrlAlarmInfo.SetItemText(0, 3, ConvertString(strType));

	// State
	CString strState = (ALARM_START == alarmInfo.m_emAlarmStatus) ? "Start" : "Stop" ;
	m_listCtrlAlarmInfo.SetItemText(0, 4, ConvertString(strState));
}


void CDevAlarmDlg::ConvertAlarmType2CString(int emType, CString& strType)
{
	switch (emType)
	{
	case DH_ALARM_ALARM_EX:
		strType = "External alarm";
		break;
	case DH_MOTION_ALARM_EX:
		strType = "Motion detection alarm";
		break;
	case DH_VIDEOLOST_ALARM_EX:
		strType = "Video loss alarm";
		break;
	case DH_SHELTER_ALARM_EX:
		strType = "Camera masking alarm";
		break;
	case DH_DISKFULL_ALARM_EX:
		strType = "HDD full alarm";
		break;
	case DH_DISKERROR_ALARM_EX:
		strType = "HDD error alarm";
		break;
	default:
		break;
	}
}

BOOL CDevAlarmDlg::PreTranslateMessage(MSG* pMsg) 
{
    if(pMsg->message == WM_KEYDOWN)
    {
        //Enter
        if(pMsg->wParam == VK_RETURN) return TRUE;
        //ESC
        if(pMsg->wParam == VK_ESCAPE) return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CDevAlarmDlg::ClearAlarmInfo(void)
{
	m_listCtrlAlarmInfo.DeleteAllItems();
	m_nAlarmCount = 0;
}