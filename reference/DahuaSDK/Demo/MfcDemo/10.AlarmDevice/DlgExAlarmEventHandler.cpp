// DlgExAlarmEventHandler.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgExAlarmEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmEventHandler dialog


CDlgExAlarmEventHandler::CDlgExAlarmEventHandler(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgExAlarmEventHandler::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgExAlarmEventHandler)
	//}}AFX_DATA_INIT
	m_bInited = FALSE;
	memset(&m_stuEventHandler, 0, sizeof(m_stuEventHandler));
}


void CDlgExAlarmEventHandler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgExAlarmEventHandler)
	DDX_Control(pDX, IDC_EXALARM_EVENTHANDLER_CHK_PSTN_NEEDREPORT, m_ckExAlarmNeedReport);
	DDX_Control(pDX, IDC_EXALARM_EVENTHANDLER_CHK_BEEP_ENABLE, m_ckExAlarmBeepEnable);
	DDX_Control(pDX, IDC_EXALARM_EVENTHANDLER_CHK_ALARMBELL_ENABLE, m_ckExAlarmBellEnable);
	DDX_Control(pDX, IDC_EXALARM_EVENTHANDLER_CHK_ALARMOUT_ENABLE, m_ckExAlarmOutEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgExAlarmEventHandler, CDialog)
	//{{AFX_MSG_MAP(CDlgExAlarmEventHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmEventHandler message handlers

BOOL CDlgExAlarmEventHandler::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	g_SetWndStaticText(this, DLG_EVENTHANDLER);
	
	// TODO: Add extra initialization here
	InitDlg();
	if (m_bInited)
	{
		StuToDlg();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExAlarmEventHandler::InitDlg()
{
	m_ckExAlarmBeepEnable.SetCheck(BST_UNCHECKED);
	m_ckExAlarmBellEnable.SetCheck(BST_UNCHECKED);
	m_ckExAlarmNeedReport.SetCheck(BST_UNCHECKED);
	m_ckExAlarmOutEnable.SetCheck(BST_UNCHECKED);
}

void CDlgExAlarmEventHandler::StuToDlg()
{
	// AlarmOut
	if (m_stuEventHandler.abAlarmOutEn)
	{
		m_ckExAlarmOutEnable.SetCheck(m_stuEventHandler.bAlarmOutEn ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_ckExAlarmOutEnable.SetCheck(BST_UNCHECKED);
	}
	
	// AlarmOutChannels
	if (m_stuEventHandler.abAlarmOutMask)
	{
		for (int i = 0; i < 16/*sizeof(DWORD) * 8*/; i++)
		{
			int nRow = i / (sizeof(DWORD)*8);
			int nCol = i % (sizeof(DWORD)*8);
			CButton* pBtn = (CButton*)GetDlgItem(IDC_EXALARM_EVENTHANDLER_CHK_AOC1_ENABLE + i);
			if (pBtn)
			{
				if (m_stuEventHandler.dwAlarmOutMask[nRow] & (0x1 << nCol))
				{
					pBtn->SetCheck(BST_CHECKED);
				}
				else
				{
					pBtn->SetCheck(BST_UNCHECKED);
				}
			}
		}
	} 
	else
	{
		for (int i = 0; i < 16/*sizeof(DWORD) * 8*/; i++)
		{
			CButton* pBtn = (CButton*)GetDlgItem(IDC_EXALARM_EVENTHANDLER_CHK_AOC1_ENABLE + i);
			if (pBtn)
			{
				pBtn->SetCheck(BST_UNCHECKED);
			}
		}
	}

	// Beep
	if (m_stuEventHandler.abBeepEnable)
	{
		m_ckExAlarmBeepEnable.SetCheck(m_stuEventHandler.bBeepEnable ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_ckExAlarmBeepEnable.SetCheck(BST_UNCHECKED);
	}

	// AlarmBell
	if (m_stuEventHandler.abAlarmBellEn)
	{
		m_ckExAlarmBellEnable.SetCheck(m_stuEventHandler.bAlarmBellEn ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_ckExAlarmBellEnable.SetCheck(BST_UNCHECKED);
	}

	// PSTNAlarmServer
	if (m_stuEventHandler.abPSTNAlarmServer && m_stuEventHandler.stuPSTNAlarmServer.bNeedReport)
	{
		m_ckExAlarmNeedReport.SetCheck(BST_CHECKED);
		for (int i = 0; i < MAX_PSTN_SERVER_NUM; ++i)
		{
			CButton* pPSTNBtn = (CButton*)GetDlgItem(IDC_EXALARM_EVENTHANDLER_CHK_PSTN_ENABLE_1 + i);
			if (pPSTNBtn)
			{
				for(int j = 0; j < m_stuEventHandler.stuPSTNAlarmServer.nServerCount; ++j)
				{
					if (i == m_stuEventHandler.stuPSTNAlarmServer.byDestination[j])
					{
						pPSTNBtn->SetCheck(BST_CHECKED);
					}
					else
					{
						pPSTNBtn->SetCheck(BST_UNCHECKED);
					}
				}
				
			}
		}
	} 
	else
	{
		m_ckExAlarmNeedReport.SetCheck(BST_UNCHECKED);
		for (int i = 0; i < MAX_PSTN_SERVER_NUM; ++i)
		{
			CButton* pPSTNBtn = (CButton*)GetDlgItem(IDC_EXALARM_EVENTHANDLER_CHK_PSTN_ENABLE_1 + i);
			if (pPSTNBtn)
			{
				pPSTNBtn->SetCheck(BST_UNCHECKED);
			}
		}
	}
}

void CDlgExAlarmEventHandler::DlgToStu()
{
	// 能力由设备来决定

	// AlarmOut
	m_stuEventHandler.bAlarmOutEn = m_ckExAlarmOutEnable.GetCheck();
	//m_stuEventHandler.abAlarmOutEn = true;
	
	// AlarmOutChannels
	{
		m_stuEventHandler.abAlarmOutMask = true;
		ZeroMemory(&m_stuEventHandler.dwAlarmOutMask, sizeof(m_stuEventHandler.dwAlarmOutMask));
		for (int i = 0; i < 16/*sizeof(DWORD) * 8*/; i++)
		{
			int nRow = i / (sizeof(DWORD)*8);
			int nCol = i % (sizeof(DWORD)*8);
			CButton* pBtn = (CButton*)GetDlgItem(IDC_EXALARM_EVENTHANDLER_CHK_AOC1_ENABLE + i);
			if (pBtn)
			{
				int nChk = pBtn->GetCheck();
				if (BST_CHECKED == nChk)
				{
					m_stuEventHandler.dwAlarmOutMask[nRow] |= 0x1 << nCol;
				}
			}
		}
	}

	// Beep
	m_stuEventHandler.bBeepEnable = m_ckExAlarmBeepEnable.GetCheck();
//	m_stuEventHandler.abBeepEnable = true;

	// AlarmBell
	m_stuEventHandler.bAlarmBellEn = m_ckExAlarmBellEnable.GetCheck();
//	m_stuEventHandler.abAlarmBellEn = true;

	// PSTNAlarmServer
	{
	//	m_stuEventHandler.abPSTNAlarmServer = true;
		m_stuEventHandler.stuPSTNAlarmServer.bNeedReport = m_ckExAlarmNeedReport.GetCheck() ? TRUE : FALSE;
		int nServerCount = 0;
		for (int i = 0; i < MAX_PSTN_SERVER_NUM; ++i)
		{
			CButton* pPSTNBtn = (CButton*)GetDlgItem(IDC_EXALARM_EVENTHANDLER_CHK_PSTN_ENABLE_1 + i);
			if (pPSTNBtn)
			{
				if (BST_CHECKED == pPSTNBtn->GetCheck())
				{
					// 以0为开始
					m_stuEventHandler.stuPSTNAlarmServer.byDestination[nServerCount++] = (unsigned char)i;
				}
			}
		}
		m_stuEventHandler.stuPSTNAlarmServer.nServerCount = nServerCount;
	}

}

void CDlgExAlarmEventHandler::OnOK() 
{
	// TODO: Add extra validation here
	DlgToStu();

	CDialog::OnOK();
}
