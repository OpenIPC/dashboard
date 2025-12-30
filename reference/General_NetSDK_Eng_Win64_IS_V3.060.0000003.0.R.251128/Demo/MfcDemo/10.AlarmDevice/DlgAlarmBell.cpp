// DlgAlarmBell.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAlarmBell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmBell dialog


CDlgAlarmBell::CDlgAlarmBell(CWnd* pParent /*=NULL*/, LLONG hLoginID /*=NULL*/)
	: CDialog(CDlgAlarmBell::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAlarmBell)
	m_hLogin = hLoginID;
	//}}AFX_DATA_INIT
}


void CDlgAlarmBell::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarmBell)
	DDX_Control(pDX, IDC_DLG_ALARMBELL_LIST_ERR, m_lsErrorInfo);
	DDX_Control(pDX, IDC_DLG_ALARMBELL_CMB_CHANNEL, m_cmbChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAlarmBell, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarmBell)
	ON_BN_CLICKED(IDC_DLG_ALARMBELL_BTN_START, OnDlgAlarmbellBtnStart)
	ON_BN_CLICKED(IDC_DLG_ALARMBELL_BTN_STOP, OnDlgAlarmbellBtnStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmBell message handlers

BOOL CDlgAlarmBell::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARMBELL);
	// TODO: Add extra initialization here
	if (!m_hLogin)
	{
		OutputInfo(ConvertString(CString("we haven't login a device yet!"), DLG_ALARMBELL));
		GetDlgItem(IDC_DLG_ALARMBELL_BTN_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_DLG_ALARMBELL_BTN_STOP)->EnableWindow(FALSE);
		return TRUE;
	}

	for (int i = 0; i < 1; i++)
	{
		CString csNum;
		csNum.Format("%s %d", ConvertString("Channel", DLG_ALARMBELL), i + 1);
		m_cmbChannel.InsertString(-1, csNum);
	}
	m_cmbChannel.SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAlarmBell::OutputInfo(const CString& csInfo)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	CString csOut;
	csOut.Format("%04d-%02d-%02d %02d:%02d:%02d: %s",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
		csInfo);
	
	m_lsErrorInfo.InsertString(0, csOut);
}

void CDlgAlarmBell::OnDlgAlarmbellBtnStart() 
{
	// TODO: Add your control notification handler code here
	NET_CTRL_ALARMBELL stuParam = {sizeof(NET_CTRL_ALARMBELL)};
	stuParam.nChannelID = m_cmbChannel.GetCurSel();
	BOOL bRet = CLIENT_ControlDevice((LLONG)m_hLogin, DH_CTRL_START_ALARMBELL, &stuParam, SDK_API_WAIT);
	CString csOut;
	if (bRet)
	{
		csOut.Format("%s %d %s", ConvertString("channel", DLG_ALARMBELL), m_cmbChannel.GetCurSel() + 1, ConvertString("alarm_bell start...", DLG_ALARMBELL));
	} 
	else
	{
		csOut.Format("%s %d %s %08x", ConvertString("channel", DLG_ALARMBELL),
			m_cmbChannel.GetCurSel() + 1, ConvertString("alarm_bell start err:", DLG_ALARMBELL), CLIENT_GetLastError());
	}
	OutputInfo(csOut);
}

void CDlgAlarmBell::OnDlgAlarmbellBtnStop() 
{
	NET_CTRL_ALARMBELL stuParam = {sizeof(NET_CTRL_ALARMBELL)};
	stuParam.nChannelID = m_cmbChannel.GetCurSel();
	BOOL bRet = CLIENT_ControlDevice((LLONG)m_hLogin, DH_CTRL_STOP_ALARMBELL, &stuParam, SDK_API_WAIT);
	CString csOut;
	if (bRet)
	{
		csOut.Format("%s %d %s", ConvertString("channel", DLG_ALARMBELL), m_cmbChannel.GetCurSel() + 1, ConvertString("alarm_bell stop...", DLG_ALARMBELL));
	} 
	else
	{
		csOut.Format("%s %d %s %08x", ConvertString("channel", DLG_ALARMBELL), m_cmbChannel.GetCurSel() + 1, ConvertString("alarm_bell stop err:", DLG_ALARMBELL), CLIENT_GetLastError());
	}
	OutputInfo(csOut);	
}
