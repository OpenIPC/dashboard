// DlgAlarmState.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAlarmState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmState dialog


CDlgAlarmState::CDlgAlarmState(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgAlarmState::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAlarmState)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuAlarmChannelState, 0, sizeof(NET_CLIENT_ALARM_CHANNELS_STATE));
	m_stuAlarmChannelState.dwSize = sizeof(NET_CLIENT_ALARM_CHANNELS_STATE);
}

CDlgAlarmState::~CDlgAlarmState()
{
}

void CDlgAlarmState::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarmState)
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_EXALARMOUT_RETCOUNT, m_edExAlarmOutRetCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT, m_edExAlarmOutCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_EXALARMIN_RETCOUNT, m_edExAlarmInRetCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT, m_edExAlarmInCount);
	DDX_Control(pDX, IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL, m_cmbExAlarmOutChannel);
	DDX_Control(pDX, IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL, m_cmbExAlarmInChannel);
	DDX_Control(pDX, IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE, m_ckExAlarmOutState);
	DDX_Control(pDX, IDC_ALARMSTATE_CHECK_EXALARMIN_STATE, m_ckExAlarmInState);
	DDX_Control(pDX, IDC_ALARMSTATE_COMBO_ALARM_TYPE, m_cmbAlarmType);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_ALARMOUT_RETCOUNT, m_edAlarmOutRetCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT, m_edAlarmOutCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_ALARMIN_RETCOUNT, m_edAlarmInRetCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_ALARMIN_COUNT, m_edAlarmInCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_ALARMBELL_RETCOUNT, m_edAlarmBellRetCount);
	DDX_Control(pDX, IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT, m_edAlarmBellCount);
	DDX_Control(pDX, IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL, m_cmbAlarmOutChannel);
	DDX_Control(pDX, IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL, m_cmbAlarmInChannel);
	DDX_Control(pDX, IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL, m_cmbAlarmBellChannel);
	DDX_Control(pDX, IDC_ALARMSTATE_CHECK_ALARMOUT_STATE, m_ckAlarmOutState);
	DDX_Control(pDX, IDC_ALARMSTATE_CHECK_ALARMIN_STATE, m_ckAlarmInState);
	DDX_Control(pDX, IDC_ALARMSTATE_CHECK_ALARMBELL_STATE, m_ckAlarmBellState);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAlarmState, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarmState)
	ON_BN_CLICKED(IDC_ALARMSTATE_BUTTON, OnAlarmstateButton)
	ON_CBN_SELCHANGE(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL, OnSelchangeAlarmstateComboAlarmbellChannel)
	ON_CBN_SELCHANGE(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL, OnSelchangeAlarmstateComboAlarminChannel)
	ON_CBN_SELCHANGE(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL, OnSelchangeAlarmstateComboAlarmoutChannel)
	ON_CBN_SELCHANGE(IDC_ALARMSTATE_COMBO_ALARM_TYPE, OnSelchangeAlarmstateComboAlarmType)
	ON_CBN_SELCHANGE(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL, OnSelchangeAlarmstateComboExalarmoutChannel)
	ON_CBN_SELCHANGE(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL, OnSelchangeAlarmstateComboExalarminChannel)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmState message handlers

BOOL CDlgAlarmState::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARM_CHANNEL_STATE);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_ALARM_CHANNEL_STATE), ConvertString("Prompt"));
		GetDlgItem(IDC_ALARMSTATE_COMBO_ALARM_TYPE)->EnableWindow(FALSE);
	}
	
	for (unsigned int i = 0; i < sizeof(DemoAlarmType) / sizeof(DemoAlarmType[0]); ++i)
	{
		m_cmbAlarmType.InsertString(-1, ConvertString(DemoAlarmType[i].pszType, DLG_ALARM_CHANNEL_STATE));
	}

	GetDlgItem(IDC_ALARMSTATE_BUTTON)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_RETCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_RETCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_RETCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_RETCOUNT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_RETCOUNT)->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CDlgAlarmState::getCfgFromDevice()
{
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_ALL_ALARM_CHANNELS_STATE, 
		(char*)&m_stuAlarmChannelState, sizeof(NET_CLIENT_ALARM_CHANNELS_STATE), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryAlarmState failed:", DLG_ALARM_CHANNEL_STATE), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		return TRUE;
	}
}

void CDlgAlarmState::showInfo(int nCurSel)
{
	m_cmbAlarmInChannel.ResetContent();
	m_cmbAlarmOutChannel.ResetContent();
	m_cmbAlarmBellChannel.ResetContent();
	char szAllarmBellRetCount[10] = {0}, szAllarmInRetCount[10] = {0}, szAllarmOutRetCount[10] ={0};
	char szExAllarmInRetCount[10] = {0}, szExAllarmOutRetCount[10] = {0};

	int i = 0;
	int nAlarmCount = 0, nExAlarmCount = 0;
	if (0 < m_stuAlarmChannelState.nAlarmInRetCount)
	{
		nAlarmCount = __min(m_stuAlarmChannelState.nAlarmInRetCount, m_stuAlarmChannelState.nAlarmInCount);
		m_cmbAlarmInChannel.ResetContent();
		for (i = 0; i < nAlarmCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %04d", ConvertString("AlarmInChannel", DLG_ALARM_CHANNEL_STATE), i + 1);
			m_cmbAlarmInChannel.InsertString(-1, csTemp);	
		}
		_itoa(m_stuAlarmChannelState.nAlarmInRetCount, szAllarmInRetCount, 10);
		m_edAlarmInRetCount.SetWindowText(szAllarmInRetCount);
		m_cmbAlarmInChannel.SetCurSel(nCurSel);
		m_ckAlarmInState.SetCheck(*(m_stuAlarmChannelState.pbAlarmInState + nCurSel) ? BST_CHECKED : BST_UNCHECKED);
	}

	if (0 < m_stuAlarmChannelState.nAlarmOutRetCount)
	{
		nAlarmCount = __min(m_stuAlarmChannelState.nAlarmOutCount, m_stuAlarmChannelState.nAlarmOutRetCount);
		m_cmbAlarmOutChannel.ResetContent();
		for (i = 0; i < nAlarmCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %04d", ConvertString("AlarmOutChannel", DLG_ALARM_CHANNEL_STATE), i + 1);
			m_cmbAlarmOutChannel.InsertString(-1, csTemp);	
		}
		_itoa(m_stuAlarmChannelState.nAlarmOutRetCount, szAllarmOutRetCount, 10);
		m_edAlarmOutRetCount.SetWindowText(szAllarmOutRetCount);
		m_cmbAlarmOutChannel.SetCurSel(nCurSel);
		m_ckAlarmOutState.SetCheck(*(m_stuAlarmChannelState.pbAlarmOutState + nCurSel) ? BST_CHECKED : BST_UNCHECKED);
	}


	if (0 < m_stuAlarmChannelState.nAlarmBellRetCount)
	{
		nAlarmCount = __min(m_stuAlarmChannelState.nAlarmBellCount, m_stuAlarmChannelState.nAlarmBellRetCount);
		m_cmbAlarmBellChannel.ResetContent();
		for (i = 0; i < nAlarmCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %04d", ConvertString("AlarmBellChannel", DLG_ALARM_CHANNEL_STATE), i + 1);
			m_cmbAlarmBellChannel.InsertString(-1, csTemp);	
		}
		_itoa(m_stuAlarmChannelState.nAlarmBellRetCount, szAllarmBellRetCount, 10);
		m_edAlarmBellRetCount.SetWindowText(szAllarmBellRetCount);
		m_cmbAlarmBellChannel.SetCurSel(nCurSel);
		m_ckAlarmBellState.SetCheck(*(m_stuAlarmChannelState.pbAlarmBellState + nCurSel) ? BST_CHECKED : BST_UNCHECKED);
	}

	if (0 < m_stuAlarmChannelState.nExAlarmInRetCount)
	{
		nExAlarmCount = __min(m_stuAlarmChannelState.nExAlarmInCount, m_stuAlarmChannelState.nExAlarmInRetCount);
		m_cmbExAlarmInChannel.ResetContent();
		for (i = 0; i < nExAlarmCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %04d", ConvertString("exAlarmInChannel", DLG_ALARM_CHANNEL_STATE), 
				*(m_stuAlarmChannelState.pnExAlarmInDestionation + i) + 1);
			m_cmbExAlarmInChannel.InsertString(-1, csTemp);
		}
		_itoa(m_stuAlarmChannelState.nExAlarmInRetCount, szExAllarmInRetCount, 10);
		m_edExAlarmInRetCount.SetWindowText(szExAllarmInRetCount);
		m_cmbExAlarmInChannel.SetCurSel(nCurSel);
		m_ckExAlarmInState.SetCheck(*(m_stuAlarmChannelState.pbExAlarmInState + nCurSel) ? BST_CHECKED : BST_UNCHECKED);
	}

	if (0 < m_stuAlarmChannelState.nExAlarmOutRetCount)
	{
		nExAlarmCount = __min(m_stuAlarmChannelState.nExAlarmOutCount, m_stuAlarmChannelState.nExAlarmOutRetCount);
		m_cmbExAlarmOutChannel.ResetContent();
		for (i = 0; i < nExAlarmCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %04d", ConvertString("exAlarmOutChannel", DLG_ALARM_CHANNEL_STATE), 
				*(m_stuAlarmChannelState.pnExAlarmOutDestionation + i) + 1);
			m_cmbExAlarmOutChannel.InsertString(-1, csTemp);
		}
		_itoa(m_stuAlarmChannelState.nExAlarmOutRetCount, szExAllarmOutRetCount, 10);
		m_edExAlarmOutRetCount.SetWindowText(szExAllarmOutRetCount);
		m_cmbExAlarmOutChannel.SetCurSel(nCurSel);
		m_ckExAlarmOutState.SetCheck(*(m_stuAlarmChannelState.pbExAlarmOutState + nCurSel) ? BST_CHECKED : BST_UNCHECKED);
	}
}

bool CDlgAlarmState::getInfo()
{
	m_stuAlarmChannelState.emType = (NET_ALARM_CHANNEL_TYPE)m_cmbAlarmType.GetCurSel();
	char szAllarmBellCount[10] = {0}, szAllarmInCount[10] = {0}, szAllarmOutCount[10] = {0};
	char szExAllarmInCount[10] = {0}, szExAllarmOutCount[10] = {0};
	m_edAlarmBellCount.GetWindowText(szAllarmBellCount, 10);
	m_stuAlarmChannelState.nAlarmBellCount = atoi(szAllarmBellCount);
	if (m_stuAlarmChannelState.nAlarmBellCount > 0)
	{
		if (m_stuAlarmChannelState.pbAlarmBellState != NULL)
		{
			delete []m_stuAlarmChannelState.pbAlarmBellState;
			m_stuAlarmChannelState.pbAlarmBellState = NULL;
		}
		m_stuAlarmChannelState.pbAlarmBellState = new BOOL[m_stuAlarmChannelState.nAlarmBellCount];
		if (m_stuAlarmChannelState.pbAlarmBellState == NULL)
		{
			deleteMemory();
			return false;
		}
		memset(m_stuAlarmChannelState.pbAlarmBellState, 0, m_stuAlarmChannelState.nAlarmBellCount * sizeof(BOOL));
	}

	m_edAlarmInCount.GetWindowText(szAllarmInCount, 10);
	m_stuAlarmChannelState.nAlarmInCount = atoi(szAllarmInCount);
	if (m_stuAlarmChannelState.nAlarmInCount > 0)
	{
		if (m_stuAlarmChannelState.pbAlarmInState != NULL)
		{
			delete []m_stuAlarmChannelState.pbAlarmInState;
			m_stuAlarmChannelState.pbAlarmInState = NULL;
		}
		m_stuAlarmChannelState.pbAlarmInState = new BOOL[m_stuAlarmChannelState.nAlarmInCount];
		if (m_stuAlarmChannelState.pbAlarmInState == NULL)
		{
			deleteMemory();
			return false;
		}
		memset(m_stuAlarmChannelState.pbAlarmInState, 0, m_stuAlarmChannelState.nAlarmInCount * sizeof(BOOL));
	}

	m_edAlarmOutCount.GetWindowText(szAllarmOutCount, 10);
	m_stuAlarmChannelState.nAlarmOutCount = atoi(szAllarmOutCount);	
	if (m_stuAlarmChannelState.nAlarmOutCount > 0)
	{
		if (m_stuAlarmChannelState.pbAlarmOutState != NULL)
		{
			delete []m_stuAlarmChannelState.pbAlarmOutState;
			m_stuAlarmChannelState.pbAlarmOutState = NULL;
		}
		m_stuAlarmChannelState.pbAlarmOutState = new BOOL[m_stuAlarmChannelState.nAlarmOutCount];
		if (m_stuAlarmChannelState.pbAlarmOutState == NULL)
		{
			deleteMemory();
			return false;
		}
		memset(m_stuAlarmChannelState.pbAlarmOutState, 0, m_stuAlarmChannelState.nAlarmOutCount * sizeof(BOOL));
	}

	m_edExAlarmInCount.GetWindowText(szExAllarmInCount, 10);
	m_stuAlarmChannelState.nExAlarmInCount = atoi(szExAllarmInCount);	
	if (m_stuAlarmChannelState.nExAlarmInCount > 0)
	{
		if (m_stuAlarmChannelState.pbExAlarmInState != NULL)
		{
			delete []m_stuAlarmChannelState.pbExAlarmInState;
			m_stuAlarmChannelState.pbExAlarmInState = NULL;
		}
		if (m_stuAlarmChannelState.pnExAlarmInDestionation != NULL)
		{
			delete []m_stuAlarmChannelState.pnExAlarmInDestionation;
			m_stuAlarmChannelState.pnExAlarmInDestionation = NULL;
		}
		m_stuAlarmChannelState.pbExAlarmInState = new BOOL[m_stuAlarmChannelState.nExAlarmInCount];
		m_stuAlarmChannelState.pnExAlarmInDestionation = new int[m_stuAlarmChannelState.nExAlarmInCount];
		if (m_stuAlarmChannelState.pbExAlarmInState == NULL || m_stuAlarmChannelState.pnExAlarmInDestionation == NULL)
		{
			deleteMemory();
			return false;
		}
		memset(m_stuAlarmChannelState.pbExAlarmInState, 0, m_stuAlarmChannelState.nExAlarmInCount * sizeof(BOOL));
		memset(m_stuAlarmChannelState.pnExAlarmInDestionation, 0, m_stuAlarmChannelState.nExAlarmInCount * sizeof(int));
	}
	
	m_edExAlarmOutCount.GetWindowText(szExAllarmOutCount, 10);
	m_stuAlarmChannelState.nExAlarmOutCount = atoi(szExAllarmOutCount);	
	if (m_stuAlarmChannelState.nExAlarmOutCount > 0)
	{
		if (m_stuAlarmChannelState.pbExAlarmOutState != NULL)
		{
			delete []m_stuAlarmChannelState.pbExAlarmOutState;
			m_stuAlarmChannelState.pbExAlarmOutState = NULL;
		}
		if (m_stuAlarmChannelState.pnExAlarmOutDestionation != NULL)
		{
			delete []m_stuAlarmChannelState.pnExAlarmOutDestionation;
			m_stuAlarmChannelState.pnExAlarmOutDestionation = NULL;
		}
		m_stuAlarmChannelState.pbExAlarmOutState = new BOOL[m_stuAlarmChannelState.nExAlarmOutCount];
		m_stuAlarmChannelState.pnExAlarmOutDestionation = new int[m_stuAlarmChannelState.nExAlarmOutCount];
		if (m_stuAlarmChannelState.pbExAlarmOutState == NULL || m_stuAlarmChannelState.pnExAlarmOutDestionation == NULL)
		{
			deleteMemory();
			return false;
		}
		memset(m_stuAlarmChannelState.pbExAlarmOutState, 0, m_stuAlarmChannelState.nExAlarmOutCount * sizeof(BOOL));
		memset(m_stuAlarmChannelState.pnExAlarmOutDestionation, 0, m_stuAlarmChannelState.nExAlarmOutCount * sizeof(int));
	}

	return true;
}

void CDlgAlarmState::OnSelchangeAlarmstateComboAlarmbellChannel() 
{
	// TODO: Add your control notification handler code here
	int nAlarmBellNum = m_cmbAlarmBellChannel.GetCurSel();
	m_ckAlarmBellState.SetCheck(*(m_stuAlarmChannelState.pbAlarmBellState + nAlarmBellNum) ? BST_CHECKED : BST_UNCHECKED);
}

void CDlgAlarmState::OnSelchangeAlarmstateComboAlarminChannel() 
{
	// TODO: Add your control notification handler code here
	int nAlarmInNum = m_cmbAlarmInChannel.GetCurSel();
	m_ckAlarmInState.SetCheck(*(m_stuAlarmChannelState.pbAlarmInState + nAlarmInNum) ? BST_CHECKED : BST_UNCHECKED);
}

void CDlgAlarmState::OnSelchangeAlarmstateComboAlarmoutChannel() 
{
	// TODO: Add your control notification handler code here
	int nAlarmOutNum = m_cmbAlarmOutChannel.GetCurSel();
	m_ckAlarmOutState.SetCheck(*(m_stuAlarmChannelState.pbAlarmOutState + nAlarmOutNum) ? BST_CHECKED : BST_UNCHECKED);
}

void CDlgAlarmState::OnSelchangeAlarmstateComboExalarmoutChannel() 
{
	// TODO: Add your control notification handler code here
	int nExAlarmOutNum = m_cmbExAlarmOutChannel.GetCurSel();
	m_ckExAlarmOutState.SetCheck(*(m_stuAlarmChannelState.pbExAlarmOutState + nExAlarmOutNum) ? BST_CHECKED : BST_UNCHECKED);
}

void CDlgAlarmState::OnSelchangeAlarmstateComboExalarminChannel() 
{
	// TODO: Add your control notification handler code here
	int nExAlarmInNum = m_cmbExAlarmInChannel.GetCurSel();
	m_ckExAlarmInState.SetCheck(*(m_stuAlarmChannelState.pbExAlarmInState + nExAlarmInNum) ? BST_CHECKED : BST_UNCHECKED);
}
void CDlgAlarmState::OnAlarmstateButton() 
{
	// TODO: Add your control notification handler code here
	if (getInfo())
	{
		if (getCfgFromDevice())
		{
			showInfo(0);
		}	
	}
	else
	{
		DestroyWindow();
		MessageBox(ConvertString("new error!!!", DLG_ALARM_CHANNEL_STATE), ConvertString("Prompt"));
	}

}

void CDlgAlarmState::OnSelchangeAlarmstateComboAlarmType() 
{
	// TODO: Add your control notification handler code here

	NET_ALARM_CHANNEL_TYPE emType = (NET_ALARM_CHANNEL_TYPE)m_cmbAlarmType.GetCurSel();
	switch(emType)
	{
	case NET_ALARM_CHANNEL_TYPE_ALL:				// 所有通道
		{
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(TRUE);
		}
		break;
	case NET_ALARM_CHANNEL_TYPE_ALARMIN:			// 报警输入通道
		{
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(FALSE);
		}
		break;
	case NET_ALARM_CHANNEL_TYPE_ALARMOUT:			// 报警输出通道
		{			
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(FALSE);
		}
		break;
	case NET_ALARM_CHANNEL_TYPE_ALARMBELL:			// 警号通道
		{
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(FALSE);
		}
		break;
	case NET_ALARM_CHANNEL_TYPE_EXALARMIN:			// 扩展模块报警输入通道
		{
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(FALSE);
		}
		break;
	case NET_ALARM_CHANNEL_TYPE_EXALARMOUT:			// 扩展模块报警输出通道
		{
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMOUT_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_ALARMBELL_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMIN_CHANNEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_COMBO_EXALARMOUT_CHANNEL)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMOUT_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_ALARMBELL_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMIN_COUNT)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_EDIT_EXALARMOUT_COUNT)->EnableWindow(TRUE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMOUT_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_ALARMBELL_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMIN_STATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_ALARMSTATE_CHECK_EXALARMOUT_STATE)->EnableWindow(TRUE);
		}
		break;
	default:
		break;
	}
	GetDlgItem(IDC_ALARMSTATE_BUTTON)->EnableWindow(TRUE);
}

BOOL CDlgAlarmState::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	deleteMemory();
	return CDialog::DestroyWindow();
}

void CDlgAlarmState::deleteMemory()
{
	if (m_stuAlarmChannelState.pbAlarmBellState != NULL)
	{		
		delete []m_stuAlarmChannelState.pbAlarmBellState;
		m_stuAlarmChannelState.pbAlarmBellState = NULL;
	}
	if (m_stuAlarmChannelState.pbAlarmInState != NULL)
	{
		delete []m_stuAlarmChannelState.pbAlarmInState;
		m_stuAlarmChannelState.pbAlarmInState = NULL;
	}
	if (m_stuAlarmChannelState.pbAlarmOutState != NULL)
	{
		delete []m_stuAlarmChannelState.pbAlarmOutState;
		m_stuAlarmChannelState.pbAlarmOutState = NULL;
	}
	if (m_stuAlarmChannelState.pbExAlarmInState != NULL)
	{
		delete []m_stuAlarmChannelState.pbExAlarmInState;
		m_stuAlarmChannelState.pbExAlarmInState = NULL;
	}
	if (m_stuAlarmChannelState.pbExAlarmOutState != NULL)
	{
		delete []m_stuAlarmChannelState.pbExAlarmOutState;
		m_stuAlarmChannelState.pbExAlarmOutState = NULL;
	}
	if (m_stuAlarmChannelState.pnExAlarmInDestionation != NULL)
	{
		delete []m_stuAlarmChannelState.pnExAlarmInDestionation;
		m_stuAlarmChannelState.pnExAlarmInDestionation = NULL;
	}
	if (m_stuAlarmChannelState.pnExAlarmOutDestionation != NULL)
	{
		delete []m_stuAlarmChannelState.pnExAlarmOutDestionation;
		m_stuAlarmChannelState.pnExAlarmOutDestionation = NULL;
	}
}

void CDlgAlarmState::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
