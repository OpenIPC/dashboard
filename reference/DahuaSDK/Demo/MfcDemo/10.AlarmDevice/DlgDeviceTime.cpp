// DlgDeviceTime.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgDeviceTime.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDeviceTime dialog


CDlgDeviceTime::CDlgDeviceTime(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgDeviceTime::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgDeviceTime)
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuTime, 0, sizeof(NET_TIME));
}


void CDlgDeviceTime::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDeviceTime)
	DDX_Control(pDX, IDC_DEVICETIME_CMB_SECOND, m_cmbSecond);
	DDX_Control(pDX, IDC_DEVICETIME_CMB_MINUTE, m_cmbMinute);
	DDX_Control(pDX, IDC_DEVICETIME_CMB_HOUR, m_cmbHour);
	DDX_Control(pDX, IDC_DEVICETIME_DTP_DATE, m_dtpDate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDeviceTime, CDialog)
	//{{AFX_MSG_MAP(CDlgDeviceTime)
	ON_BN_CLICKED(IDC_DEVICETIME_BTN_GET, OnDevicetimeBtnGet)
	ON_BN_CLICKED(IDC_DEVICETIME_BTN_SET, OnDevicetimeBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgDeviceTime message handlers
BOOL CDlgDeviceTime::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_DEVICETIME);
	// TODO: Add extra initialization here
	int i = 0;
	// hour
	m_cmbHour.ResetContent();
	for (i = 0; i < 24; i++)
	{
		CString csTemp;
		csTemp.Format("%d", i);
		m_cmbHour.InsertString(-1, csTemp);
		
	}
	m_cmbHour.SetCurSel(0);
	
	// min
	m_cmbMinute.ResetContent();
	for (i = 0; i < 60; i++)
	{
		CString csTemp;
		csTemp.Format("%d", i);
		m_cmbMinute.InsertString(-1, csTemp);
	}
	m_cmbMinute.SetCurSel(0);
	
	// second
	m_cmbSecond.ResetContent();
	for (i = 0; i < 60; i++)
	{
		CString csTemp;
		csTemp.Format("%d", i);
		m_cmbSecond.InsertString(-1, csTemp);
	}
	m_cmbSecond.SetCurSel(0);

	//获取系统当前时间
	SYSTEMTIME st;
	GetLocalTime(&st);
	m_cmbHour.SetCurSel(st.wHour);
	m_cmbMinute.SetCurSel(st.wMinute);
	m_cmbSecond.SetCurSel(st.wSecond);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgDeviceTime::OnDevicetimeBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_DEVICETIME), ConvertString("Prompt"));
		return;
	}

	BOOL bRet = CLIENT_QueryDeviceTime(m_lLoginId, &m_stuTime, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Get Device Time failed:", DLG_DEVICETIME), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
	else
	{
		StuToDlg();
		//MessageBox("Get Device Time OK!", ConvertString("Prompt"));
	}
}

void CDlgDeviceTime::OnDevicetimeBtnSet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_DEVICETIME), ConvertString("Prompt"));
		return;
	}

	DlgToStu();
	BOOL bRet = CLIENT_SetupDeviceTime(m_lLoginId, &m_stuTime);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Set Device Time failed:", DLG_DEVICETIME), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
	else
	{
		StuToDlg();
		MessageBox(ConvertString("Set Device Time OK!", DLG_DEVICETIME), ConvertString("Prompt"));
	}
	
}

void CDlgDeviceTime::StuToDlg()
{
	m_dtpDate.SetTime(COleDateTime(m_stuTime.dwYear, m_stuTime.dwMonth, m_stuTime.dwDay, 0, 0, 0));
	// hour, min, second
	m_cmbHour.SetCurSel(m_stuTime.dwHour);
	m_cmbMinute.SetCurSel(m_stuTime.dwMinute);
	m_cmbSecond.SetCurSel(m_stuTime.dwSecond);
}

void CDlgDeviceTime::DlgToStu()
{
	SYSTEMTIME stBegin = {0}, stEnd = {0};
	m_dtpDate.GetTime(&stBegin);
	m_stuTime.dwYear = stBegin.wYear;
	m_stuTime.dwMonth = stBegin.wMonth;
	m_stuTime.dwDay = stBegin.wDay;

	// hour, min, second
	m_stuTime.dwHour = m_cmbHour.GetCurSel();
	m_stuTime.dwMinute = m_cmbMinute.GetCurSel();
	m_stuTime.dwSecond = m_cmbSecond.GetCurSel();
}
