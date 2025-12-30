// SubDlgCfgEventHandler.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "SubDlgCfgEventHandler.h"
#include "DlgExAlarmHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgEventHandler dialog

CSubDlgCfgEventHandler::CSubDlgCfgEventHandler(CWnd* pParent /* = NULL */)
	: CDialog(CSubDlgCfgEventHandler::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSubDlgCfgEventHandler)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	memset(&m_stuEventHandler, 0, sizeof(m_stuEventHandler));	
	m_bInited = FALSE;
	memset(&m_stuTimeSection, 0, sizeof(m_stuTimeSection));
	m_emConfigType = emConfigType_Unknown;
	m_bTimeChange = FALSE;
}


void CSubDlgCfgEventHandler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSubDlgCfgEventHandler)
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_MMS_ENABLE, m_chkMMS);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_TIP_ENABLE, m_chkTipEnable);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHECK_PSTN_NEEDREPORT, m_chkNeedReport);
	DDX_Control(pDX, IDC_EVENTHANDLER_CMB_CALLPROTOCOL, m_cmbCallProtocol);
	DDX_Control(pDX, IDC_EVENTHANDLER_CMB_CALLER, m_cmbCaller);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_CALLENABLE, m_chkCallEnable);
	DDX_Control(pDX, IDC_EVENTHANDLER_CMB_AC_NUM, m_cmbAccessControlNum);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_SEG1_ENABLE, m_chkSeg1);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_SEG2_ENABLE, m_chkSeg2);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_SEG3_ENABLE, m_chkSeg3);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_SEG4_ENABLE, m_chkSeg4);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_SEG5_ENABLE, m_chkSeg5);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_SEG6_ENABLE, m_chkSeg6);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP1_END, m_dtp1End);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP1_START, m_dtp1Start);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP2_END, m_dtp2End);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP2_START, m_dtp2Start);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP3_END, m_dtp3End);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP3_START, m_dtp3Start);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP4_END, m_dtp4End);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP4_START, m_dtp4Start);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP5_END, m_dtp5End);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP5_START, m_dtp5Start);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP6_END, m_dtp6End);
	DDX_Control(pDX, IDC_EVENTHANDLER_DTP6_START, m_dtp6Start);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_BEEP_ENABLE, m_chkBeep);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_MSG_ENABLE, m_chkMsg);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_VOICE_ENABLE, m_chkVoice);
	DDX_Control(pDX, IDC_EVENTHANDLER_CMB_ACO, m_cmbAccessControlOperation);
	DDX_Control(pDX, IDC_EVENTHANDLER_CMB_WEEKDAY, m_cmbWeekDay);
	DDX_Control(pDX, IDC_EVENTHANDLER_EDT_AOL, m_edtAlarmOutLatch);
	DDX_Control(pDX, IDC_EVENTHANDLER_EDT_FILEPATH, m_edtFilePath);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_ALARMOUT_ENABLE, m_chkAlarmOut);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_ALARMBELL_ENABLE, m_chkAlarmBell);
	DDX_Control(pDX, IDC_EVENTHANDLER_CHK_AC_ENABLE, m_chkAccessControl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSubDlgCfgEventHandler, CDialog)
	//{{AFX_MSG_MAP(CSubDlgCfgEventHandler)
	ON_CBN_SELCHANGE(IDC_EVENTHANDLER_CMB_WEEKDAY, OnSelchangeEventhandlerCmbWeekday)
	ON_BN_CLICKED(IDC_EVENTHANDLER_BTN_FILEPATH, OnEventhandlerBtnFilepath)
	ON_CBN_SELCHANGE(IDC_EVENTHANDLER_CMB_ACO, OnSelchangeEventhandlerCmbAco)
	ON_BN_CLICKED(IDC_EVENTHANDLER_CHK_SEG1_ENABLE, OnEventhandlerChkSeg1Enable)
	ON_BN_CLICKED(IDC_EVENTHANDLER_CHK_SEG2_ENABLE, OnEventhandlerChkSeg2Enable)
	ON_BN_CLICKED(IDC_EVENTHANDLER_CHK_SEG3_ENABLE, OnEventhandlerChkSeg3Enable)
	ON_BN_CLICKED(IDC_EVENTHANDLER_CHK_SEG4_ENABLE, OnEventhandlerChkSeg4Enable)
	ON_BN_CLICKED(IDC_EVENTHANDLER_CHK_SEG5_ENABLE, OnEventhandlerChkSeg5Enable)
	ON_BN_CLICKED(IDC_EVENTHANDLER_CHK_SEG6_ENABLE, OnEventhandlerChkSeg6Enable)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP1_START, OnDatetimechangeEventhandlerDtp1Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP1_END, OnDatetimechangeEventhandlerDtp1End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP2_START, OnDatetimechangeEventhandlerDtp2Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP2_END, OnDatetimechangeEventhandlerDtp2End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP3_START, OnDatetimechangeEventhandlerDtp3Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP3_END, OnDatetimechangeEventhandlerDtp3End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP4_START, OnDatetimechangeEventhandlerDtp4Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP4_END, OnDatetimechangeEventhandlerDtp4End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP5_START, OnDatetimechangeEventhandlerDtp5Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP5_END, OnDatetimechangeEventhandlerDtp5End)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP6_START, OnDatetimechangeEventhandlerDtp6Start)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EVENTHANDLER_DTP6_END, OnDatetimechangeEventhandlerDtp6End)
	ON_CBN_SELCHANGE(IDC_EVENTHANDLER_CMB_AC_NUM, OnSelchangeEventhandlerCmbAcNum)
	ON_BN_CLICKED(IDC_EVENTHANDLER_BTN_EXALARMOUT, OnEventhandlerBtnExalarmout)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// Private method

void CSubDlgCfgEventHandler::InitDlg()
{
	unsigned int i = 0;

	// WeekDay
	m_cmbWeekDay.ResetContent();
	for (i = 0; i < sizeof(stuDemoWeekDay) / sizeof(stuDemoWeekDay[0]); i++)
	{
		m_cmbWeekDay.InsertString(-1, ConvertString(stuDemoWeekDay[i].pszName, DLG_EVENTHANDLER));
	}
	m_cmbWeekDay.SetCurSel(0);

	// DateTimePickers...
	SYSTEMTIME stStart = {2000,1,1,1}, stEnd = {2000,1,1,1,23,59,59,0};
	m_dtp1Start.SetTime(&stStart);
	m_dtp1End.SetTime(&stEnd);
	m_dtp2Start.SetTime(&stStart);
	m_dtp2End.SetTime(&stEnd);
	m_dtp3Start.SetTime(&stStart);
	m_dtp3End.SetTime(&stEnd);
	m_dtp4Start.SetTime(&stStart);
	m_dtp4End.SetTime(&stEnd);
	m_dtp5Start.SetTime(&stStart);
	m_dtp5End.SetTime(&stEnd);
	m_dtp6Start.SetTime(&stStart);
	m_dtp6End.SetTime(&stEnd);
	m_chkSeg1.SetCheck(BST_UNCHECKED);
	m_chkSeg2.SetCheck(BST_UNCHECKED);
	m_chkSeg3.SetCheck(BST_UNCHECKED);
	m_chkSeg4.SetCheck(BST_UNCHECKED);
	m_chkSeg5.SetCheck(BST_UNCHECKED);
	m_chkSeg6.SetCheck(BST_UNCHECKED);

	// AlarmOut
	m_chkAlarmOut.SetCheck(BST_UNCHECKED);
	
	// AlarmOutLatch
	SetDlgItemInt(IDC_EVENTHANDLER_EDT_AOL, 0);
	
	// Message
	m_chkMsg.SetCheck(BST_UNCHECKED);

	// MMS
	m_chkMMS.SetCheck(BST_UNCHECKED);
	
	// Beep
	m_chkBeep.SetCheck(BST_UNCHECKED);
	
	// Voice
	m_chkVoice.SetCheck(BST_UNCHECKED);
	
	// AudioFilePath
	SetDlgItemText(IDC_EVENTHANDLER_EDT_FILEPATH, NULL);
	
	// AlarmBell
	m_chkAlarmBell.SetCheck(BST_UNCHECKED);
	
	// AccessControl
	m_chkAccessControl.SetCheck(BST_UNCHECKED);

	// AccessControlNum
	m_cmbAccessControlNum.ResetContent();
	for (i = 0; i < m_stuEventHandler.dwAccessControl; i++)
	{
		CString csTemp;
		csTemp.Format("%d", i + 1);
		m_cmbAccessControlNum.InsertString(-1, csTemp);
	}
	m_cmbAccessControlNum.SetCurSel(-1);
//	m_cmbAccessControlNum.EnableWindow(FALSE);
	
	// AccessControlOperation
	m_cmbAccessControlOperation.ResetContent();
	for (i = 0; i < sizeof(stuAccessControlType) / sizeof(stuAccessControlType[0]); i++)
	{
		m_cmbAccessControlOperation.InsertString(-1, ConvertString(stuAccessControlType[i].pszName, DLG_EVENTHANDLER));
	}
	m_cmbAccessControlOperation.SetCurSel(-1);

	// Call enable
	m_chkCallEnable.SetCheck(BST_UNCHECKED);

	// Caller
	m_cmbCaller.ResetContent();
	m_cmbCaller.InsertString(-1, ConvertString("Device", DLG_EVENTHANDLER));

	// Call protocol
	m_cmbCallProtocol.ResetContent();
	m_cmbCallProtocol.InsertString(-1, ConvertString("Cellular", DLG_EVENTHANDLER));
	ShowWindowByType();
}


void CSubDlgCfgEventHandler::StuToDlg()
{
	// AlarmOut
	if (m_stuEventHandler.abAlarmOutEn)
	{
		m_chkAlarmOut.SetCheck(m_stuEventHandler.bAlarmOutEn ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_chkAlarmOut.SetCheck(BST_UNCHECKED);
	}

	// AlarmOutChannels
	if (m_stuEventHandler.abAlarmOutMask)
	{
		for (int i = 0; i < 24; i++)
        {
			int nRow = i / (sizeof(DWORD)*8);
			int nCol = i % (sizeof(DWORD)*8);
			if (i<16)
			{
				CButton* pBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_AOC1_ENABLE + i);
				if (NULL == pBtn)
				{
					return;
				}
				if (m_stuEventHandler.dwAlarmOutMask[nRow] & (0x1 << nCol))
				{
					pBtn->SetCheck(BST_CHECKED);
				}
				else
				{
					pBtn->SetCheck(BST_UNCHECKED);
				}
			}
			else
			{
				CButton* pBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_AOC17_ENABLE + i -16);
				if (NULL == pBtn)
				{
					return;
				}
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
		for (int i = 0; i < 24; ++i)
		{
			if (i<16)
			{
				CButton* pBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_AOC1_ENABLE + i);
				if (NULL == pBtn)
				{
					return;
				}
				pBtn->SetCheck(BST_UNCHECKED);
			}
			else
			{
				CButton* pBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_AOC17_ENABLE + i -16);
				if (NULL == pBtn)
				{
					return;
				}
				pBtn->SetCheck(BST_UNCHECKED);
			}
		}
	}	

	// AlarmOutLatch
	if (m_stuEventHandler.abAlarmOutLatch)
	{
		SetDlgItemInt(IDC_EVENTHANDLER_EDT_AOL, m_stuEventHandler.nAlarmOutLatch);
	}
	else
	{
		SetDlgItemInt(IDC_EVENTHANDLER_EDT_AOL, 0);
	}

	// Message
	if (m_stuEventHandler.abMessageEnable)
	{
		m_chkMsg.SetCheck(m_stuEventHandler.bMessageEnable ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_chkMsg.SetCheck(BST_UNCHECKED);
	}

	// MMS
	if (m_stuEventHandler.abMMSEnable)
	{
		m_chkMMS.SetCheck(m_stuEventHandler.bMMSEnable ? BST_CHECKED : BST_UNCHECKED);
	} 
	else
	{
		m_chkMMS.SetCheck(BST_UNCHECKED);
	}

	// Beep
	if (m_stuEventHandler.abBeepEnable)
	{
		m_chkBeep.SetCheck(m_stuEventHandler.bBeepEnable ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_chkBeep.SetCheck(BST_UNCHECKED);
	}

	// Voice
	if (m_stuEventHandler.abVoiceEnable)
	{
		m_chkVoice.SetCheck(m_stuEventHandler.bVoiceEnable ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_chkVoice.SetCheck(BST_UNCHECKED);
	}

	// AudioFilePath
	if (m_stuEventHandler.abAudioFileName)
	{
		SetDlgItemText(IDC_EVENTHANDLER_EDT_FILEPATH, m_stuEventHandler.szAudioFileName);
	}
	else
	{
		SetDlgItemText(IDC_EVENTHANDLER_EDT_FILEPATH, NULL);
	}

	// AlarmBell
	if (m_stuEventHandler.abAlarmBellEn)
	{
		m_chkAlarmBell.SetCheck(m_stuEventHandler.bAlarmBellEn ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_chkAlarmBell.SetCheck(BST_UNCHECKED);
	}

	// AlarmBellLatch
	if (m_stuEventHandler.abAlarmBellLatch)
	{
		SetDlgItemInt(IDC_EVENTHANDLER_EDT_ABL, m_stuEventHandler.nAlarmBellLatch);
	}
	else
	{
		SetDlgItemInt(IDC_EVENTHANDLER_EDT_ABL, 0);
	}

	// AccessControl
	if (m_stuEventHandler.abAccessControlEn)
	{
		m_chkAccessControl.SetCheck(m_stuEventHandler.bAccessControlEn ? BST_CHECKED : BST_UNCHECKED);
	}
	else
	{
		m_chkAccessControl.SetCheck(BST_UNCHECKED);
	}

	// AccessControlNum
	m_cmbAccessControlNum.SetCurSel(0);
 	OnSelchangeEventhandlerCmbAcNum();

	// AccessControlOperation

	// TimeSection
	OnSelchangeEventhandlerCmbWeekday();

	// Talk back
	if (m_stuEventHandler.abTalkBack)
	{
		m_chkCallEnable.SetCheck(m_stuEventHandler.stuTalkback.bCallEnable ? BST_CHECKED : BST_UNCHECKED);
		m_cmbCaller.SetCurSel((int)m_stuEventHandler.stuTalkback.emCallerType);
		m_cmbCallProtocol.SetCurSel((int)m_stuEventHandler.stuTalkback.emCallerProtocol);
	}

	// PSTNAlarmServer
	if (m_stuEventHandler.abPSTNAlarmServer)
	{
		if (m_stuEventHandler.stuPSTNAlarmServer.bNeedReport)
		{	
			m_chkNeedReport.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkNeedReport.SetCheck(BST_UNCHECKED);
		}

		for (int i = 0; i < MAX_PSTN_SERVER_NUM; ++i)
		{
			CButton* pPSTNBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_PSTN_ENABLE_1 + i);
			if (NULL == pPSTNBtn)
			{
				return;
			}
			for(int j = 0; j < m_stuEventHandler.stuPSTNAlarmServer.nServerCount; ++j)
			{
				if (i == m_stuEventHandler.stuPSTNAlarmServer.byDestination[j])
				{
					pPSTNBtn->SetCheck(BST_CHECKED);
					break;
				}
				else
				{
					pPSTNBtn->SetCheck(BST_UNCHECKED);
				}
			}
		}
	} 
	else
	{
		m_chkNeedReport.SetCheck(BST_UNCHECKED);
		for (int i = 0; i < MAX_PSTN_SERVER_NUM; ++i)
		{
			CButton* pPSTNBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_PSTN_ENABLE_1 + i);
			if (NULL == pPSTNBtn)
			{
				return;
			}
			pPSTNBtn->SetCheck(BST_UNCHECKED);
		}
	}

	// TipEnable
	if (m_stuEventHandler.abTipEnable)
	{
		BOOL bCheck = m_stuEventHandler.bTipEnable;
		m_chkTipEnable.SetCheck(bCheck);
	}
}

void CSubDlgCfgEventHandler::DlgToStu()
{
	// AlarmOut
	m_stuEventHandler.bAlarmOutEn = m_chkAlarmOut.GetCheck();
	m_stuEventHandler.abAlarmOutEn = true;

	// AlarmOutChannels
	{
		m_stuEventHandler.abAlarmOutMask = true;
		ZeroMemory(&m_stuEventHandler.dwAlarmOutMask, sizeof(m_stuEventHandler.dwAlarmOutMask));
		for (int i = 0; i < 24/*sizeof(DWORD) * 8*/; i++)
		{
			int nRow = i / (sizeof(DWORD)*8);
			int nCol = i % (sizeof(DWORD)*8);
            if (i<16)
            {
                CButton* pBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_AOC1_ENABLE + i);
				if (NULL == pBtn)
				{
					return;
				}				
				if (BST_CHECKED == pBtn->GetCheck())
				{
					m_stuEventHandler.dwAlarmOutMask[nRow] |= 0x1 << nCol;
				}
            }
            else
            {
                CButton* pBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_AOC17_ENABLE + i - 16);
				if (NULL == pBtn)
				{
					return;
				}				
				if (BST_CHECKED == pBtn->GetCheck())
				{
					m_stuEventHandler.dwAlarmOutMask[nRow] |= 0x1 << nCol;
				}
            }
		}
	}

	// AlarmOutLatch
	m_stuEventHandler.nAlarmOutLatch = GetDlgItemInt(IDC_EVENTHANDLER_EDT_AOL);
	if (m_stuEventHandler.nAlarmOutLatch)
	{
		m_stuEventHandler.abAlarmOutLatch = true;
	}
	
	// Message
	m_stuEventHandler.bMessageEnable = m_chkMsg.GetCheck();
	m_stuEventHandler.abMessageEnable = true;

	// MMS
	m_stuEventHandler.bMMSEnable = m_chkMMS.GetCheck();
	m_stuEventHandler.abMMSEnable = true;
	
	// Beep
	m_stuEventHandler.bBeepEnable = m_chkBeep.GetCheck();
	m_stuEventHandler.abBeepEnable = true;
	
	// Voice
	m_stuEventHandler.bVoiceEnable = m_chkVoice.GetCheck();
	m_stuEventHandler.abVoiceEnable = true;
	
	// AudioFilePath
	GetDlgItemText(IDC_EVENTHANDLER_EDT_FILEPATH, m_stuEventHandler.szAudioFileName, sizeof(m_stuEventHandler.szAudioFileName));
	if (strlen(m_stuEventHandler.szAudioFileName))
	{
		m_stuEventHandler.abAudioFileName = true;
	}

	// AlarmBell
	m_stuEventHandler.bAlarmBellEn = m_chkAlarmBell.GetCheck();
	m_stuEventHandler.abAlarmBellEn = true;

	// AlarmBellLatch
	m_stuEventHandler.nAlarmBellLatch = GetDlgItemInt(IDC_EVENTHANDLER_EDT_ABL, NULL);
	m_stuEventHandler.abAlarmBellLatch = true;
	
	// AccessControl
	m_stuEventHandler.bAccessControlEn = m_chkAccessControl.GetCheck();
	m_stuEventHandler.abAccessControlEn = true;
	
	// AccessControlOperation

	// TimeSection
	GetTimeSectionFromDlg();

	// Talk back
	m_stuEventHandler.abTalkBack = true;
	m_stuEventHandler.stuTalkback.bCallEnable = m_chkCallEnable.GetCheck() ? TRUE : FALSE;
	m_stuEventHandler.stuTalkback.emCallerType = m_cmbCaller.GetCurSel() >= 0 ? ((EM_CALLER_TYPE)m_cmbCaller.GetCurSel()) : EM_CALLER_DEVICE;
	m_stuEventHandler.stuTalkback.emCallerProtocol = m_cmbCallProtocol.GetCurSel() >= 0 ? ((EM_CALLER_PROTOCOL_TYPE)m_cmbCallProtocol.GetCurSel()) : EM_CALLER_PROTOCOL_CELLULAR;

	// PSTNAlarmServer
	{
		m_stuEventHandler.abPSTNAlarmServer = true;
		m_stuEventHandler.stuPSTNAlarmServer.bNeedReport = m_chkNeedReport.GetCheck() ? TRUE : FALSE;
		int nServerCount = 0;
		for (int i = 0; i < MAX_PSTN_SERVER_NUM; ++i)
		{
			CButton* pPSTNBtn = (CButton*)GetDlgItem(IDC_EVENTHANDLER_CHK_PSTN_ENABLE_1 + i);
			if (NULL == pPSTNBtn)
			{
				return;
			}
			if (BST_CHECKED == pPSTNBtn->GetCheck())
			{
				// 以0为开始
				m_stuEventHandler.stuPSTNAlarmServer.byDestination[nServerCount++] = (unsigned char)i;
			}
		}
		m_stuEventHandler.stuPSTNAlarmServer.nServerCount = nServerCount;
	}

	// TipEnable
	m_stuEventHandler.abTipEnable = true;
	m_stuEventHandler.bTipEnable = m_chkTipEnable.GetCheck() ? TRUE : FALSE;
}

void CSubDlgCfgEventHandler::ShowTimeSection(int nSeg, const CFG_TIME_SECTION& stuTimeSection)
{
	SYSTEMTIME stBegin = {2000, 1, 1, 1}, stEnd = {2000, 1, 1, 1};
	CfgTimeSectionToDTP(stuTimeSection, stBegin, stEnd);

	switch (nSeg)
	{
	case 0:
		m_dtp1Start.SetTime(&stBegin);
		m_dtp1End.SetTime(&stEnd);
		m_chkSeg1.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 1:
		m_dtp2Start.SetTime(&stBegin);
		m_dtp2End.SetTime(&stEnd);
		m_chkSeg2.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 2:
		m_dtp3Start.SetTime(&stBegin);
		m_dtp3End.SetTime(&stEnd);
		m_chkSeg3.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 3:
		m_dtp4Start.SetTime(&stBegin);
		m_dtp4End.SetTime(&stEnd);
		m_chkSeg4.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 4:
		m_dtp5Start.SetTime(&stBegin);
		m_dtp5End.SetTime(&stEnd);
		m_chkSeg5.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	case 5:
		m_dtp6Start.SetTime(&stBegin);
		m_dtp6End.SetTime(&stEnd);
		m_chkSeg6.SetCheck(stuTimeSection.dwRecordMask ? BST_CHECKED : BST_UNCHECKED);
		break;
	}
}

void CSubDlgCfgEventHandler::GetTimeSectionFromDlg()
{
	int nSel = m_cmbWeekDay.GetCurSel();
	SYSTEMTIME stBegin = {0}, stEnd = {0};
	for (int i = 0; i < MAX_REC_TSECT; i++)
	{
		CFG_TIME_SECTION& stuTimeSection = m_stuTimeSection[(em_WeekDay)nSel][i];
		switch (i)
		{
		case 0:
			stuTimeSection.dwRecordMask = m_chkSeg1.GetCheck();
			m_dtp1Start.GetTime(&stBegin);
			m_dtp1End.GetTime(&stEnd);
			break;
		case 1:
			stuTimeSection.dwRecordMask = m_chkSeg2.GetCheck();
			m_dtp2Start.GetTime(&stBegin);
			m_dtp2End.GetTime(&stEnd);
			break;
		case 2:
			stuTimeSection.dwRecordMask = m_chkSeg3.GetCheck();
			m_dtp3Start.GetTime(&stBegin);
			m_dtp3End.GetTime(&stEnd);
			break;
		case 3:
			stuTimeSection.dwRecordMask = m_chkSeg4.GetCheck();
			m_dtp4Start.GetTime(&stBegin);
			m_dtp4End.GetTime(&stEnd);
			break;
		case 4:
			stuTimeSection.dwRecordMask = m_chkSeg5.GetCheck();
			m_dtp5Start.GetTime(&stBegin);
			m_dtp5End.GetTime(&stEnd);
			break;
		case 5:
			stuTimeSection.dwRecordMask = m_chkSeg6.GetCheck();
			m_dtp6Start.GetTime(&stBegin);
			m_dtp6End.GetTime(&stEnd);
			break;
		}
		DTPToCfgTimeSection(stBegin, stEnd, stuTimeSection);
	}
}

void CSubDlgCfgEventHandler::CfgTimeSectionToDTP(const CFG_TIME_SECTION& stuTimeSection, SYSTEMTIME& stBegin, SYSTEMTIME& stEnd)
{
	stBegin.wHour = stuTimeSection.nBeginHour;
	stBegin.wMinute = stuTimeSection.nBeginMin;
	stBegin.wSecond = stuTimeSection.nBeginSec;
	stEnd.wHour = stuTimeSection.nEndHour;
	stEnd.wMinute = stuTimeSection.nEndMin;
	stEnd.wSecond = stuTimeSection.nEndSec;
}

void CSubDlgCfgEventHandler::DTPToCfgTimeSection(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, CFG_TIME_SECTION& stuTimeSection)
{
	if (!m_bTimeChange)
	{
		TimeChange(stBegin, stEnd, stuTimeSection);
	}

	stuTimeSection.nBeginHour = stBegin.wHour;
	stuTimeSection.nBeginMin = stBegin.wMinute;
	stuTimeSection.nBeginSec = stBegin.wSecond;
	stuTimeSection.nEndHour = stEnd.wHour;
	stuTimeSection.nEndMin = stEnd.wMinute;
	stuTimeSection.nEndSec = stEnd.wSecond;
}

void CSubDlgCfgEventHandler::TimeChange(const SYSTEMTIME& stBegin, const SYSTEMTIME& stEnd, const CFG_TIME_SECTION& stuTimeSection)
{
	// 当时间有修改时需要增大超时时间，是由于设备更新时间会很慢，大概需要几十秒
	if (stuTimeSection.nBeginHour != stBegin.wHour || stuTimeSection.nBeginMin != stBegin.wMinute
		|| stuTimeSection.nBeginSec != stBegin.wSecond || stuTimeSection.nEndHour != stEnd.wHour
		|| stuTimeSection.nEndMin != stEnd.wMinute || stuTimeSection.nEndSec != stEnd.wSecond)
	{
		m_bTimeChange = TRUE;
	}
}
/////////////////////////////////////////////////////////////////////////////
// CSubDlgCfgEventHandler message handlers

BOOL CSubDlgCfgEventHandler::OnInitDialog() 
{
	CDialog::OnInitDialog();
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


void CSubDlgCfgEventHandler::OnSelchangeEventhandlerCmbWeekday() 
{
	// TODO: Add your control notification handler code here
	int nSel = m_cmbWeekDay.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}

	for (int i = 0; i < MAX_REC_TSECT; i++)
	{
		ShowTimeSection(i, m_stuTimeSection[nSel][i]);
	}
}

void CSubDlgCfgEventHandler::OnEventhandlerBtnFilepath() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, 
					"*.*", 
					"", 
					OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
					"All File(*.*)|*.*||",
					this);
	if(dlg.DoModal() == IDOK)
	{
		CString csPath = dlg.GetFileName();
		SetDlgItemText(IDC_EVENTHANDLER_EDT_FILEPATH, csPath);
	}
}

void CSubDlgCfgEventHandler::OnOK() 
{
	// TODO: Add extra validation here
	DlgToStu();

	CDialog::OnOK();
}

void CSubDlgCfgEventHandler::OnSelchangeEventhandlerCmbAcNum() 
{
	// TODO: Add your control notification handler code here
	int nSel = m_cmbAccessControlNum.GetCurSel();
	if (-1 == nSel 
		|| (nSel + 1) > m_stuEventHandler.dwAccessControl 
		|| (nSel + 1) > MAX_ACCESSCONTROL_NUM)
	{
		return;
	}

	m_cmbAccessControlOperation.SetCurSel((int)m_stuEventHandler.emAccessControlType[nSel]);
}

void CSubDlgCfgEventHandler::OnSelchangeEventhandlerCmbAco() 
{
	// TODO: Add your control notification handler code here
	int nNum = m_cmbAccessControlNum.GetCurSel();
	if (-1 == nNum || (nNum + 1) > m_stuEventHandler.dwAccessControl)
	{
		return;
	}

	int nSel = m_cmbAccessControlOperation.GetCurSel();
	if (nSel >= 0)
	{
		m_stuEventHandler.emAccessControlType[nNum] = (EM_CFG_ACCESSCONTROLTYPE)nSel;
	}
}

void CSubDlgCfgEventHandler::OnEventhandlerChkSeg1Enable() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CSubDlgCfgEventHandler::OnEventhandlerChkSeg2Enable() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CSubDlgCfgEventHandler::OnEventhandlerChkSeg3Enable() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CSubDlgCfgEventHandler::OnEventhandlerChkSeg4Enable() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CSubDlgCfgEventHandler::OnEventhandlerChkSeg5Enable() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CSubDlgCfgEventHandler::OnEventhandlerChkSeg6Enable() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp1Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp1End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp2Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp2End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp3Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp3End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp4Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp4End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp5Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp5End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp6Start(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::OnDatetimechangeEventhandlerDtp6End(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	*pResult = 0;
}

void CSubDlgCfgEventHandler::ShowWindowByType()
{
	switch (m_emConfigType)
	{
	case emConfigType_Alarm:
		{
			ShowAlarmInfo();
		}
		break;
	case emConfigType_AlarmChassisIntrusion:
		{
			ShowChassisIntrusionInfo();
		}
		break;
// 	case emConfigType_AlarmPowerFault:
// 		{
// 			ShowPowerHandlerInfo();
// 		}
// 		break;
	case emConfigType_AlarmExAlarmInput:
		{
			ShowExAlarmInputInfo();
		}
		break;
	default:
		break;
	}
}

void CSubDlgCfgEventHandler::ShowPowerHandlerInfo()
{
	m_cmbCallProtocol.EnableWindow(FALSE);
	m_cmbCaller.EnableWindow(FALSE);
	m_chkCallEnable.EnableWindow(FALSE);
	m_cmbAccessControlNum.EnableWindow(FALSE);
	m_chkSeg1.EnableWindow(FALSE);
	m_chkSeg2.EnableWindow(FALSE);
	m_chkSeg3.EnableWindow(FALSE);
	m_chkSeg4.EnableWindow(FALSE);
	m_chkSeg5.EnableWindow(FALSE);
	m_chkSeg6.EnableWindow(FALSE);
	m_dtp1End.EnableWindow(FALSE);
	m_dtp1Start.EnableWindow(FALSE);
	m_dtp2End.EnableWindow(FALSE);
	m_dtp2Start.EnableWindow(FALSE);
	m_dtp3End.EnableWindow(FALSE);
	m_dtp3Start.EnableWindow(FALSE);
	m_dtp4End.EnableWindow(FALSE);
	m_dtp4Start.EnableWindow(FALSE);
	m_dtp5End.EnableWindow(FALSE);
	m_dtp5Start.EnableWindow(FALSE);
	m_dtp6End.EnableWindow(FALSE);
	m_dtp6Start.EnableWindow(FALSE);
	m_chkMsg.EnableWindow(FALSE);
	m_cmbAccessControlOperation.EnableWindow(FALSE);
	m_cmbWeekDay.EnableWindow(FALSE);
	m_edtAlarmOutLatch.EnableWindow(FALSE);
	m_chkAccessControl.EnableWindow(FALSE);
	m_chkMMS.EnableWindow(FALSE);
	m_edtFilePath.EnableWindow(FALSE);
	m_chkVoice.EnableWindow(FALSE);
	
	int i = 0;
	
	for (i = 0; i < 6; ++i)
	{
		GetDlgItem(IDC_EVENTHANDLER_CHK_ACO1_ENABLE + i)->EnableWindow(FALSE);
	}
	
	GetDlgItem(IDC_EVENTHANDLER_BTN_EXALARMOUT)->EnableWindow(FALSE);
}

void CSubDlgCfgEventHandler::ShowChassisIntrusionInfo()
{
	// TODO 暂时同电源联动项要求一致，先复用
	ShowPowerHandlerInfo();
	m_chkVoice.EnableWindow(TRUE);
	m_chkMsg.EnableWindow(TRUE);
	m_edtFilePath.EnableWindow(TRUE);
}

void CSubDlgCfgEventHandler::ShowExAlarmInputInfo()
{
	// TODO 暂时同电源联动项要求一致，先复用
	ShowPowerHandlerInfo();
	GetDlgItem(IDC_EVENTHANDLER_BTN_EXALARMOUT)->EnableWindow(TRUE);
	m_chkTipEnable.EnableWindow(FALSE);

}

void CSubDlgCfgEventHandler::ShowAlarmInfo()
{
	int i = 0;
	
	for (i = 0; i < 6; ++i)
	{
		GetDlgItem(IDC_EVENTHANDLER_CHK_ACO1_ENABLE + i)->EnableWindow(FALSE);
	}
	m_chkAccessControl.EnableWindow(FALSE);
	m_cmbAccessControlNum.EnableWindow(FALSE);
	m_cmbAccessControlOperation.EnableWindow(FALSE);
}
void CSubDlgCfgEventHandler::OnEventhandlerBtnExalarmout() 
{
	// TODO: Add your control notification handler code here
	CDlgExAlarmHandler CDlg(this);
	m_stuEventHandler.abExAlarmOutMask = true;
	
	CDlg.SetAlarmOutMask(m_stuEventHandler.dwExAlarmOutMask);
	
	if (IDOK == CDlg.DoModal())
	{
		DWORD dwExAlarmOutMask[MAX_CHANNEL_COUNT] = {0};
		CDlg.GetAlarmOutMask(dwExAlarmOutMask);
		
		for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			m_stuEventHandler.dwExAlarmOutMask[i] = dwExAlarmOutMask[i];
		}
	}
}
