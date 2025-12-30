// DlgCfgLocales.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgLocales.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgLocales dialog


CDlgCfgLocales::CDlgCfgLocales(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgCfgLocales::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgLocales)
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.nStructSize = sizeof(m_stuInfo);
	m_stuInfo.stuDstStart.nStructSize = sizeof(AV_CFG_DSTTime);
	m_stuInfo.stuDstEnd.nStructSize = sizeof(AV_CFG_DSTTime);
	m_stuInfo.stuDstStart.nYear = m_stuInfo.stuDstEnd.nYear = 2000;
	m_stuInfo.stuDstStart.nMonth = m_stuInfo.stuDstEnd.nMonth = 1;
	m_stuInfo.stuDstStart.nDay = m_stuInfo.stuDstEnd.nDay = 1;
}


void CDlgCfgLocales::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgLocales)
	DDX_Control(pDX, IDC_LOCALES_CHK_DST, m_chkDST);
	DDX_Control(pDX, IDC_LOCALES_CMB_ENDWEEK, m_cbWeekEnd);
	DDX_Control(pDX, IDC_LOCALES_CMB_BEGINWEEK, m_cbWeekBegin);
	DDX_Control(pDX, IDC_LOCALES_CMB_ENDMONTH, m_cbMonthEnd);
	DDX_Control(pDX, IDC_LOCALES_CMB_BEGINMONTH, m_cbMonthBegin);
	DDX_Control(pDX, IDC_LOCALES_CMB_ENDMIN, m_cbMinEnd);
	DDX_Control(pDX, IDC_LOCALES_CMB_BEGINMIN, m_cbMinBegin);
	DDX_Control(pDX, IDC_LOCALES_CMB_ENDHOUR, m_cbHourEnd);
	DDX_Control(pDX, IDC_LOCALES_CMB_BEGINHOUR, m_cbHourBegin);
	DDX_Control(pDX, IDC_LOCALES_DTP_END, m_dtpDateEnd);
	DDX_Control(pDX, IDC_LOCALES_DTP_BEGIN, m_dtpDateBegin);
	DDX_Control(pDX, IDC_LOCALES_CMB_BEGINWEEKDAY, m_cbWeekdayBegin);
	DDX_Control(pDX, IDC_LOCALES_CMB_ENDWEEKDAY, m_cbWeekdayEnd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgLocales, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgLocales)
	ON_BN_CLICKED(IDC_LOCALES_RADIO_BYDAY, OnLocalesRadioByday)
	ON_BN_CLICKED(IDC_LOCALES_RADIO_BYWEEKDAY, OnLocalesRadioByweekday)
	ON_BN_CLICKED(IDC_LOCALES_BTN_GET, OnLocalesBtnGet)
	ON_BN_CLICKED(IDC_LOCALES_BTN_SET, OnLocalesBtnSet)
	ON_BN_CLICKED(IDC_LOCALES_CHK_DST, OnLocalesChkDst)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgLocales private method

void CDlgCfgLocales::InitDlg(BOOL bRecheck /* = TRUE */)
{
	int i = 0;
	CButton* pBtnByDay = (CButton*)GetDlgItem(IDC_LOCALES_RADIO_BYDAY);
	CButton* pBtnByWeekday = (CButton*)GetDlgItem(IDC_LOCALES_RADIO_BYWEEKDAY);
	if (bRecheck)
	{
		if (0 == m_stuInfo.stuDstStart.nWeek && 0 == m_stuInfo.stuDstEnd.nWeek)
		{
			pBtnByDay->SetCheck(BST_CHECKED);
			pBtnByWeekday->SetCheck(BST_UNCHECKED);
		}
		else
		{
			pBtnByDay->SetCheck(BST_UNCHECKED);
			pBtnByWeekday->SetCheck(BST_CHECKED);
		}
	}

	// date
	m_dtpDateBegin.SetTime(COleDateTime(2000,1,1,0,0,0));
	m_dtpDateEnd.SetTime(COleDateTime(2000,1,1,0,0,0));

	// month
	m_cbMonthBegin.ResetContent();
	m_cbMonthEnd.ResetContent();
	for (i = 0; i < 12; i++)
	{
		CString csTmp;
		csTmp.Format("%d", i + 1);
		m_cbMonthBegin.InsertString(-1, csTmp);
		m_cbMonthEnd.InsertString(-1, csTmp);
	}
	m_cbMonthBegin.SetCurSel(0);
	m_cbMonthEnd.SetCurSel(0);

	// week
	m_cbWeekBegin.ResetContent();
	m_cbWeekEnd.ResetContent();
	m_cbWeekBegin.InsertString(-1, ConvertString("First", DLG_CFG_LOCALES));
	m_cbWeekBegin.InsertString(-1, ConvertString("Second", DLG_CFG_LOCALES));
	m_cbWeekBegin.InsertString(-1, ConvertString("Third", DLG_CFG_LOCALES));
	m_cbWeekBegin.InsertString(-1, ConvertString("Fourth", DLG_CFG_LOCALES));
	m_cbWeekBegin.InsertString(-1, ConvertString("Last", DLG_CFG_LOCALES));
	m_cbWeekEnd.InsertString(-1, ConvertString("First", DLG_CFG_LOCALES));
	m_cbWeekEnd.InsertString(-1, ConvertString("Second", DLG_CFG_LOCALES));
	m_cbWeekEnd.InsertString(-1, ConvertString("Third", DLG_CFG_LOCALES));
	m_cbWeekEnd.InsertString(-1, ConvertString("Fourth", DLG_CFG_LOCALES));
	m_cbWeekEnd.InsertString(-1, ConvertString("Last", DLG_CFG_LOCALES));
	m_cbWeekBegin.SetCurSel(0);
	m_cbWeekEnd.SetCurSel(0);

	if (pBtnByDay && pBtnByDay->GetCheck())
	{
		// by day
		GetDlgItem(IDC_STATIC_MONTH)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_WEEK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_WEEKDAY)->ShowWindow(SW_HIDE);

		m_cbMonthBegin.ShowWindow(SW_HIDE);
		m_cbMonthEnd.ShowWindow(SW_HIDE);
		m_cbWeekBegin.ShowWindow(SW_HIDE);
		m_cbWeekEnd.ShowWindow(SW_HIDE);
		m_cbWeekdayBegin.ShowWindow(SW_HIDE);
		m_cbWeekdayEnd.ShowWindow(SW_HIDE);	
		
		m_dtpDateBegin.ShowWindow(SW_SHOW);
		m_dtpDateBegin.MoveWindow(23, 73, 150, 22);
		m_dtpDateEnd.ShowWindow(SW_SHOW);
		m_dtpDateEnd.MoveWindow(23, 100, 150, 22);
	}
	if (pBtnByWeekday && pBtnByWeekday->GetCheck())
	{
		// by weekday
		GetDlgItem(IDC_STATIC_MONTH)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_WEEK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC_WEEKDAY)->ShowWindow(SW_SHOW);
		
		m_cbMonthBegin.ShowWindow(SW_SHOW);
		m_cbMonthEnd.ShowWindow(SW_SHOW);
		m_cbWeekBegin.ShowWindow(SW_SHOW);
		m_cbWeekEnd.ShowWindow(SW_SHOW);
		m_cbWeekdayBegin.ShowWindow(SW_SHOW);
		m_cbWeekdayEnd.ShowWindow(SW_SHOW);	
		
		m_dtpDateBegin.ShowWindow(SW_HIDE);
		m_dtpDateBegin.MoveWindow(23, 73, 150, 22);
		m_dtpDateEnd.ShowWindow(SW_HIDE);
		m_dtpDateEnd.MoveWindow(23, 100, 150, 22);
	}

	// weekday
	m_cbWeekdayBegin.ResetContent();
	m_cbWeekdayEnd.ResetContent();
	for (i = 0; i < sizeof(stuDemoWeekDay) / sizeof(stuDemoWeekDay[i]); i++)
	{
		m_cbWeekdayBegin.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName, DLG_EVENTHANDLER));
		m_cbWeekdayEnd.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName, DLG_EVENTHANDLER));
	}
	m_cbWeekdayBegin.SetCurSel(0);
	m_cbWeekdayEnd.SetCurSel(0);

	// hour
	m_cbHourBegin.ResetContent();
	m_cbHourEnd.ResetContent();
	for (i = 0; i < 24; i++)
	{
		CString csTemp;
		csTemp.Format("%d", i);
		m_cbHourBegin.InsertString(-1, csTemp);
		m_cbHourEnd.InsertString(-1, csTemp);
	}
	m_cbHourBegin.SetCurSel(0);
	m_cbHourEnd.SetCurSel(0);

	// min
	m_cbMinBegin.ResetContent();
	m_cbMinEnd.ResetContent();
	for (i = 0; i < 60; i++)
	{
		CString csTemp;
		csTemp.Format("%d", i);
		m_cbMinBegin.InsertString(-1, csTemp);
		m_cbMinEnd.InsertString(-1, csTemp);
	}
	m_cbMinBegin.SetCurSel(0);
	m_cbMinEnd.SetCurSel(0);
}

BOOL CDlgCfgLocales::SetConfig()
{
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_LOCALS, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet Locals error..."), DLG_CFG_LOCALES), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_LOCALS,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		// 有些设备没有回复操作结果，故无法直接判断配置是否成功
		// 通过再次查询来确认修改是否成功
		/*
		if (!bRetSetupCfg)
		{
			CString csErr;
			csErr.Format("SetupConfig=>%s failed: %08x", CFG_CMD_LOCALS, CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(CString("SetConfig Locales ok!"), ConvertString("Prompt"));
		}*/
	}
	return TRUE;
}

BOOL CDlgCfgLocales::GetConfig()
{
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_LOCALS,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		AV_CFG_Locales stuInfo = {sizeof(AV_CFG_Locales)};
		stuInfo.stuDstStart.nStructSize = sizeof(AV_CFG_DSTTime);
		stuInfo.stuDstEnd.nStructSize = sizeof(AV_CFG_DSTTime);

		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_LOCALS, szJsonBuf, (void*)&stuInfo, sizeof(stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse Locales error..."), DLG_CFG_LOCALES), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(AV_CFG_Locales));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Get Config Locales error:", DLG_CFG_LOCALES), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgLocales::StuToDlg()
{
	// enable
	m_chkDST.SetCheck(m_stuInfo.bDSTEnable ? BST_CHECKED : BST_UNCHECKED);

	// byDay or byWeekday
	{
		CButton* pBtnByDay = (CButton*)GetDlgItem(IDC_LOCALES_RADIO_BYDAY);
		CButton* pBtnByWeekday = (CButton*)GetDlgItem(IDC_LOCALES_RADIO_BYWEEKDAY);
		if (!pBtnByDay || !pBtnByWeekday)
		{
			return;
		}

		if (0 == m_stuInfo.stuDstStart.nWeek && 0 == m_stuInfo.stuDstEnd.nWeek)
		{
			// byDay
			//pBtnByDay->SetCheck(BST_CHECKED);
			//pBtnByWeekday->SetCheck(BST_UNCHECKED);

			m_dtpDateBegin.SetTime(COleDateTime(m_stuInfo.stuDstStart.nYear, m_stuInfo.stuDstStart.nMonth, m_stuInfo.stuDstStart.nDay, 0, 0, 0));
			m_dtpDateEnd.SetTime(COleDateTime(m_stuInfo.stuDstEnd.nYear, m_stuInfo.stuDstEnd.nMonth, m_stuInfo.stuDstEnd.nDay, 0, 0, 0));
		} 
		else
		{
			// byWeekday
			//pBtnByDay->SetCheck(BST_UNCHECKED);
			//pBtnByWeekday->SetCheck(BST_CHECKED);

			m_cbMonthBegin.SetCurSel(m_stuInfo.stuDstStart.nMonth - 1);
			m_cbMonthEnd.SetCurSel(m_stuInfo.stuDstEnd.nMonth - 1);

			if (-1 == m_stuInfo.stuDstStart.nWeek)
			{
				m_cbWeekBegin.SetCurSel(4);
			}
			else
			{
				m_cbWeekBegin.SetCurSel(m_stuInfo.stuDstStart.nWeek - 1);
			}
			if (-1 == m_stuInfo.stuDstEnd.nWeek)
			{
				m_cbWeekEnd.SetCurSel(4);
			}
			else
			{
				m_cbWeekEnd.SetCurSel(m_stuInfo.stuDstEnd.nWeek - 1);
			}

			m_cbWeekdayBegin.SetCurSel((int)m_stuInfo.stuDstStart.nDay);
			m_cbWeekdayEnd.SetCurSel((int)m_stuInfo.stuDstEnd.nDay);
		}
	}

	// hour, min
	m_cbHourBegin.SetCurSel(m_stuInfo.stuDstStart.nHour);
	m_cbHourEnd.SetCurSel(m_stuInfo.stuDstEnd.nHour);
	m_cbMinBegin.SetCurSel(m_stuInfo.stuDstStart.nMinute);
	m_cbMinEnd.SetCurSel(m_stuInfo.stuDstEnd.nMinute);
}

void CDlgCfgLocales::DlgToStu()
{
	CButton* pBtnByDay = (CButton*)GetDlgItem(IDC_LOCALES_RADIO_BYDAY);
	CButton* pBtnByWeekday = (CButton*)GetDlgItem(IDC_LOCALES_RADIO_BYWEEKDAY);

	// dst
	m_stuInfo.bDSTEnable = m_chkDST.GetCheck() ? TRUE : FALSE;

	if (pBtnByDay && pBtnByDay->GetCheck())
	{
		// byDay
		SYSTEMTIME stBegin = {0}, stEnd = {0};
		m_dtpDateBegin.GetTime(&stBegin);
		m_dtpDateEnd.GetTime(&stEnd);

		m_stuInfo.stuDstStart.nYear = stBegin.wYear;
		m_stuInfo.stuDstStart.nMonth = stBegin.wMonth;
		m_stuInfo.stuDstStart.nDay = stBegin.wDay;
		m_stuInfo.stuDstStart.nWeek = 0;

		m_stuInfo.stuDstEnd.nYear = stEnd.wYear;
		m_stuInfo.stuDstEnd.nMonth = stEnd.wMonth;
		m_stuInfo.stuDstEnd.nDay = stEnd.wDay;
		m_stuInfo.stuDstEnd.nWeek = 0;
	}
	if (pBtnByWeekday && pBtnByWeekday->GetCheck())
	{
		// byWeekday
		m_stuInfo.stuDstStart.nMonth = m_cbMonthBegin.GetCurSel() + 1;
		if (4 == m_cbWeekBegin.GetCurSel())
		{
			m_stuInfo.stuDstStart.nWeek = -1;
		} 
		else
		{
			m_stuInfo.stuDstStart.nWeek = m_cbWeekBegin.GetCurSel() + 1;
		}
		m_stuInfo.stuDstStart.nDay = m_cbWeekdayBegin.GetCurSel();

		m_stuInfo.stuDstEnd.nMonth = m_cbMonthEnd.GetCurSel() + 1;
		if (4 == m_cbWeekEnd.GetCurSel())
		{
			m_stuInfo.stuDstEnd.nWeek = -1;
		}
		else
		{
			m_stuInfo.stuDstEnd.nWeek = m_cbWeekEnd.GetCurSel() + 1;
		}
		m_stuInfo.stuDstEnd.nDay = m_cbWeekdayEnd.GetCurSel();
	}

	// hour, min
	m_stuInfo.stuDstStart.nHour = m_cbHourBegin.GetCurSel();
	m_stuInfo.stuDstEnd.nHour = m_cbHourEnd.GetCurSel();
	m_stuInfo.stuDstStart.nMinute = m_cbMinBegin.GetCurSel();
	m_stuInfo.stuDstEnd.nMinute = m_cbMinEnd.GetCurSel();
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgLocales message handlers

BOOL CDlgCfgLocales::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_LOCALES);

	// TODO: Add extra initialization here	
	InitDlg();
	OnLocalesBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgLocales::OnLocalesRadioByday() 
{
	// TODO: Add your control notification handler code here
	InitDlg(FALSE);
	StuToDlg();
}

void CDlgCfgLocales::OnLocalesRadioByweekday() 
{
	// TODO: Add your control notification handler code here
	InitDlg(FALSE);
	StuToDlg();
}

void CDlgCfgLocales::OnLocalesBtnGet() 
{
	// TODO: Add your control notification handler code here	
	if (GetConfig())
	{
		InitDlg();
		StuToDlg();
	}
}

void CDlgCfgLocales::OnLocalesBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfig();
}

void CDlgCfgLocales::OnLocalesChkDst() 
{
	// TODO: Add your control notification handler code here
	if (m_chkDST.GetCheck())
	{
		m_stuInfo.bDSTEnable = TRUE;
	} 
	else
	{
		m_stuInfo.bDSTEnable = FALSE;
	}
}
