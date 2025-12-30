// ConfigRecord.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigRecord.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigRecord dialog


CConfigRecord::CConfigRecord(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigRecord::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigRecord)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pDev = 0;;
	memset(m_recCFG, 0, 16*sizeof(DHDEV_RECORD_CFG));
	m_bInited = FALSE;
	m_chnIdx = -1;
	m_weekIdx = -1;
}


void CConfigRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigRecord)
	DDX_Control(pDX, IDC_CHECK_REDUNDANCY, m_redundancychk);
	DDX_Control(pDX, IDC_COMBO_RECORDWEEK, m_weeksel);
	DDX_Control(pDX, IDC_COMBO_CHANNO, m_channelsel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigRecord, CDialog)
	//{{AFX_MSG_MAP(CConfigRecord)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_UNDO_ALL, OnUndoAll)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_CBN_SELCHANGE(IDC_COMBO_RECORDWEEK, OnSelchangeComboRecordweek)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNO, OnSelchangeComboChanno)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigRecord message handlers

void CConfigRecord::GetConfig()
{
	if (!m_pDev || m_bInited)
	{
		return;
	}
	
	BOOL bRet = FALSE;
	DWORD retlen = 0;
	bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_RECORDCFG, -1/*all channel*/,
		&m_recCFG, 16*sizeof(DHDEV_RECORD_CFG), &retlen, CONFIG_WAITTIME);
	if (!bRet || retlen != m_pDev->Info.nChanNum*sizeof(DHDEV_RECORD_CFG))
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		m_bInited = TRUE;
	}
	
	//show config information
	if (m_channelsel.GetCount() > 0)
	{
		m_channelsel.SetCurSel(0);
		OnSelchangeComboChanno();
	}
}

void CConfigRecord::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

BOOL CConfigRecord::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	if (!m_pDev)
	{
		return TRUE;
	}
	
	CString strChn;
	for (int i = 0; i < m_pDev->Info.nChanNum; i++)
	{
		strChn.Format(ConvertString("Channel_%d"), i+1);
		m_channelsel.InsertString(i, strChn);
	}
	
	m_weeksel.InsertString(0, ConvertString("Sunday"));
	m_weeksel.InsertString(1, ConvertString("Monday"));
	m_weeksel.InsertString(2, ConvertString("Tuesday"));
	m_weeksel.InsertString(3, ConvertString("Wedensday"));
	m_weeksel.InsertString(4, ConvertString("Thursday"));
	m_weeksel.InsertString(5, ConvertString("Friday"));
	m_weeksel.InsertString(6, ConvertString("Saturday"));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigRecord::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow)
	{
		GetConfig();
	}
}


void CConfigRecord::OnSelchangeComboChanno() 
{
	if (!m_bInited)
	{
		return;
	}
	
	int weekIdx;
	int secIdx = 0;
	int i;
	//store current config
	if (m_chnIdx < 0)
	{
		goto skipsave;
	}
	m_recCFG[m_chnIdx].byPreRecordLen = GetDlgItemInt(IDC_EDIT_PRERECLEN);
	m_recCFG[m_chnIdx].byRedundancyEn = m_redundancychk.GetCheck();
	
	weekIdx = m_weeksel.GetCurSel();
	if (weekIdx < 0)
	{
		goto skipsave;
	}
	
	for (i = 0; i < 6; i++)
	{
		m_recCFG[m_chnIdx].stSect[weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
		secIdx++;
		m_recCFG[m_chnIdx].stSect[weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
		secIdx++;
		m_recCFG[m_chnIdx].stSect[weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
		secIdx++;
		
		m_recCFG[m_chnIdx].stSect[weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
		secIdx++;
		m_recCFG[m_chnIdx].stSect[weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
		secIdx++;
		m_recCFG[m_chnIdx].stSect[weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
		secIdx++;
	}
	
	//show new config
skipsave:
	int chnIdx = m_channelsel.GetCurSel();
	if (chnIdx < 0)
	{
		return;
	}
	m_chnIdx = chnIdx;
	
	SetDlgItemInt(IDC_EDIT_PRERECLEN, m_recCFG[chnIdx].byPreRecordLen);
	m_redundancychk.SetCheck((m_recCFG[chnIdx].byRedundancyEn)?1:0);
	if (m_weeksel.GetCount())
	{
		m_weeksel.SetCurSel(0);
	}
	m_weekIdx = -1; //prevent from unwanted saving
	OnSelchangeComboRecordweek();
}

void CConfigRecord::OnSelchangeComboRecordweek() 
{
	if (!m_bInited || m_chnIdx < 0)
	{
		return;
	}
	
	//store current time sectors
	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}
	
	//show new time sectors
	int weekIdx = m_weeksel.GetCurSel();
	if (weekIdx < 0)
	{
		return;
	}
	m_weekIdx = weekIdx;
	
	int timeCtrlIdx = 0;
	for (int i = 0; i < 6; i++)
	{
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_recCFG[m_chnIdx].stSect[weekIdx][i].iBeginHour);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_recCFG[m_chnIdx].stSect[weekIdx][i].iBeginMin);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_recCFG[m_chnIdx].stSect[weekIdx][i].iBeginSec);
		timeCtrlIdx++;

		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_recCFG[m_chnIdx].stSect[weekIdx][i].iEndHour);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_recCFG[m_chnIdx].stSect[weekIdx][i].iEndMin);
		timeCtrlIdx++;
		SetDlgItemInt(IDC_TIME_18+timeCtrlIdx, m_recCFG[m_chnIdx].stSect[weekIdx][i].iEndSec);
		timeCtrlIdx++;
	}
}

void CConfigRecord::OnUndoAll() 
{
	m_bInited = FALSE;
	m_chnIdx = -1;
	m_weekIdx = -1;

	GetConfig();
}

void CConfigRecord::OnApply() 
{
	if (!m_bInited || m_chnIdx < 0)
	{
		return;
	}
	
	int weekIdx = m_weeksel.GetCurSel();
	if (weekIdx < 0)
	{
		return;
	}
	
	m_recCFG[m_chnIdx].byPreRecordLen = GetDlgItemInt(IDC_EDIT_PRERECLEN);
	m_recCFG[m_chnIdx].byRedundancyEn = m_redundancychk.GetCheck();

	if (m_weekIdx >= 0)
	{
		int secIdx = 0;
		for (int i = 0; i < 6; i++)
		{
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iBeginHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iBeginMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iBeginSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iEndHour = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iEndMin = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
			m_recCFG[m_chnIdx].stSect[m_weekIdx][i].iEndSec = GetDlgItemInt(IDC_TIME_18+secIdx);
			secIdx++;
		}
	}
	
	BOOL bRet = CLIENT_SetDevConfig(m_pDev->LoginID, DH_DEV_RECORDCFG, -1/*all channel*/,
								&m_recCFG, 16*sizeof(DHDEV_RECORD_CFG), CONFIG_WAITTIME);
	if (!bRet)
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		MessageBox(ConvertString(MSG_CONFIG_SUCCESS), "OK");
	}
}

