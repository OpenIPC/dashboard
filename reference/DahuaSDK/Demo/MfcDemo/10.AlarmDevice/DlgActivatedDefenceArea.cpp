// DlgActivatedDefenceArea.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgActivatedDefenceArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_TIME_LEN	128

/////////////////////////////////////////////////////////////////////////////
// CDlgActivatedDefenceArea dialog


CDlgActivatedDefenceArea::CDlgActivatedDefenceArea(CWnd* pParent /*=NULL*/, LLONG lloginId)
	: CDialog(CDlgActivatedDefenceArea::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgActivatedDefenceArea)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lloginId;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);

}


void CDlgActivatedDefenceArea::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgActivatedDefenceArea)
	DDX_Control(pDX, IDC_ACTIVATEDDEFENCEAREA_LIST_EXALARMINREPORT, m_strListExAlarmIn);
	DDX_Control(pDX, IDC_ACTIVATEDDEFENCEAREA_LIST_ALARMINREPORT, m_strListAlarmIn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgActivatedDefenceArea, CDialog)
	//{{AFX_MSG_MAP(CDlgActivatedDefenceArea)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ACTIVATEDDEFENCEAREA_BTN_GET, OnActivateddefenceareaBtnGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgActivatedDefenceArea message handlers

BOOL CDlgActivatedDefenceArea::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ACTIVATEDDEFENCE);
	// TODO: Add extra initialization here
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_ACTIVATEDDEFENCE), ConvertString("Prompt"));
		//EndDialog(0);
	}
	
	InitDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgActivatedDefenceArea::OnActivateddefenceareaBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("please login first!", DLG_ACTIVATEDDEFENCE), ConvertString("Prompt"));
		return;
	}
	if (!getInfo())
	{
		MessageBox(ConvertString("new error!!!", DLG_ACTIVATEDDEFENCE), ConvertString("Prompt"));
		return;
	}
	if (getInfoFromDevice())
	{
		showInfo();
	}
}

void CDlgActivatedDefenceArea::showInfo()
{
	SetDlgItemInt(IDC_ACTIVATEDDEFENCEAREA_EDIT_ALARMINRETCOUNT, m_stuInfo.nRetAlarmInCount, FALSE);
	SetDlgItemInt(IDC_ACTIVATEDDEFENCEAREA_EDIT_EXALARMINRETCOUNT, m_stuInfo.nRetExAlarmInCount, FALSE);
	int i = 0, nIndex = 0;
	int nMinCount = __min(m_stuInfo.nAlarmInCount, m_stuInfo.nRetAlarmInCount);
	m_strListAlarmIn.DeleteAllItems();
	m_strListExAlarmIn.DeleteAllItems();
	for (i = 0; i < nMinCount; ++i)
	{
		char szIndex[10] = {0};
		char szChannel[10] = {0};
		char szTime[MAX_TIME_LEN] = {0};
		_itoa(i + 1, szIndex, 10);
		_itoa(m_stuInfo.pstuAlarmInDefenceAreaInfo[i].nChannel + 1, szChannel, 10);
		GetTimeStringByStruct(m_stuInfo.pstuAlarmInDefenceAreaInfo[i].stuActivationTime, szTime);
		m_strListAlarmIn.InsertItem(nIndex, NULL);
		m_strListAlarmIn.SetItemText(nIndex, 0, szIndex);
		m_strListAlarmIn.SetItemText(nIndex, 1, szChannel);
		m_strListAlarmIn.SetItemText(nIndex, 2, szTime);
		nIndex++;
	}

	nIndex = 0;
	nMinCount = __min(m_stuInfo.nExAlarmInCount, m_stuInfo.nRetExAlarmInCount);
	for (i = 0; i < nMinCount; ++i)
	{
		char szIndex[10] = {0};
		char szChannel[10] = {0};
		char szTime[MAX_TIME_LEN] = {0};
		_itoa(i + 1, szIndex, 10);
		_itoa(m_stuInfo.pstuExAlarmInDefenceAreaInfo[i].nChannel + 1, szChannel, 10);
		GetTimeStringByStruct(m_stuInfo.pstuExAlarmInDefenceAreaInfo[i].stuActivationTime, szTime);
		m_strListExAlarmIn.InsertItem(nIndex, NULL);
		m_strListExAlarmIn.SetItemText(nIndex, 0, szIndex);
		m_strListExAlarmIn.SetItemText(nIndex, 1, szChannel);
		m_strListExAlarmIn.SetItemText(nIndex, 2, szTime);
		nIndex++;
	}
}

BOOL CDlgActivatedDefenceArea::getInfo()
{
	m_stuInfo.nAlarmInCount = GetDlgItemInt(IDC_ACTIVATEDDEFENCEAREA_EDIT_ALARMINCOUNT);
	m_stuInfo.nExAlarmInCount = GetDlgItemInt(IDC_ACTIVATEDDEFENCEAREA_EDIT_EXALARMINCOUNT);
	if (m_stuInfo.nAlarmInCount > 0)
	{
		if (m_stuInfo.pstuAlarmInDefenceAreaInfo != NULL)
		{
			delete []m_stuInfo.pstuAlarmInDefenceAreaInfo;
			m_stuInfo.pstuAlarmInDefenceAreaInfo = NULL;
		}
		m_stuInfo.pstuAlarmInDefenceAreaInfo = new NET_ACTIVATEDDEFENCEAREA_INFO[m_stuInfo.nAlarmInCount];
		if (m_stuInfo.pstuAlarmInDefenceAreaInfo == NULL)
		{
			return FALSE;
		}
		memset(m_stuInfo.pstuAlarmInDefenceAreaInfo, 0, sizeof(NET_ACTIVATEDDEFENCEAREA_INFO) * m_stuInfo.nAlarmInCount);
	}

	if (m_stuInfo.nExAlarmInCount > 0)
	{
		if (m_stuInfo.pstuExAlarmInDefenceAreaInfo != NULL)
		{
			delete []m_stuInfo.pstuExAlarmInDefenceAreaInfo;
			m_stuInfo.pstuExAlarmInDefenceAreaInfo = NULL;
		}
		m_stuInfo.pstuExAlarmInDefenceAreaInfo = new NET_ACTIVATEDDEFENCEAREA_INFO[m_stuInfo.nExAlarmInCount];
		if (m_stuInfo.pstuExAlarmInDefenceAreaInfo == NULL)
		{
			if (m_stuInfo.pstuAlarmInDefenceAreaInfo != NULL)
			{
				delete []m_stuInfo.pstuAlarmInDefenceAreaInfo;
				m_stuInfo.pstuAlarmInDefenceAreaInfo = NULL;
			}
			return FALSE;
		}
		memset(m_stuInfo.pstuExAlarmInDefenceAreaInfo, 0, sizeof(NET_ACTIVATEDDEFENCEAREA_INFO) * m_stuInfo.nExAlarmInCount);
	}
	return TRUE;
}


BOOL CDlgActivatedDefenceArea::getInfoFromDevice()
{
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_ACTIVATEDDEFENCEAREA, 
		(char*)&m_stuInfo, sizeof(NET_ACTIVATEDDEFENCEAREA), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x",ConvertString("Query state: GetActivatedDefence failed:", DLG_ACTIVATEDDEFENCE), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		return TRUE;
	}
}

void CDlgActivatedDefenceArea::InitDlg()
{
	GetDlgItem(IDC_ACTIVATEDDEFENCEAREA_EDIT_ALARMINRETCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ACTIVATEDDEFENCEAREA_EDIT_EXALARMINRETCOUNT)->EnableWindow(FALSE);
	// 设置默认查询个数
	SetDlgItemInt(IDC_ACTIVATEDDEFENCEAREA_EDIT_ALARMINCOUNT, 16, FALSE);
	SetDlgItemInt(IDC_ACTIVATEDDEFENCEAREA_EDIT_EXALARMINCOUNT, 64, FALSE);

	m_strListAlarmIn.SetExtendedStyle(m_strListAlarmIn.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_strListAlarmIn.InsertColumn(0, ConvertString("Index", DLG_ACTIVATEDDEFENCE), LVCFMT_LEFT, 60, -1);
	m_strListAlarmIn.InsertColumn(1, ConvertString("Channel", DLG_ACTIVATEDDEFENCE), LVCFMT_LEFT, 90, -1);
	m_strListAlarmIn.InsertColumn(2, ConvertString("ActivationTime", DLG_ACTIVATEDDEFENCE), LVCFMT_LEFT, 150, -1);

	m_strListExAlarmIn.SetExtendedStyle(m_strListExAlarmIn.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
	m_strListExAlarmIn.InsertColumn(0, ConvertString("Index", DLG_ACTIVATEDDEFENCE), LVCFMT_LEFT, 60, -1);
	m_strListExAlarmIn.InsertColumn(1, ConvertString("Channel", DLG_ACTIVATEDDEFENCE), LVCFMT_LEFT, 90, -1);
	m_strListExAlarmIn.InsertColumn(2, ConvertString("ActivationTime", DLG_ACTIVATEDDEFENCE), LVCFMT_LEFT, 150, -1);
}

void CDlgActivatedDefenceArea::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_stuInfo.pstuAlarmInDefenceAreaInfo != NULL)
	{
		delete []m_stuInfo.pstuAlarmInDefenceAreaInfo;
		m_stuInfo.pstuAlarmInDefenceAreaInfo = NULL;
	}
	
	if (m_stuInfo.pstuExAlarmInDefenceAreaInfo != NULL)
	{
		delete []m_stuInfo.pstuExAlarmInDefenceAreaInfo;
		m_stuInfo.pstuExAlarmInDefenceAreaInfo = NULL;
	}
}

void CDlgActivatedDefenceArea::GetTimeStringByStruct(NET_TIME stuTime, char *pTime)
{
	_snprintf(pTime, MAX_TIME_LEN, "%d-%d-%d %d:%d:%d", stuTime.dwYear, stuTime.dwMonth, stuTime.dwDay,
		stuTime.dwHour, stuTime.dwMinute, stuTime.dwSecond);
}