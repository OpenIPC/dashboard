// QueryAlarmState.cpp : implementation file
//

#include "stdafx.h"
#include "Alarm.h"
#include "QueryAlarmState.h"
#include "AlarmDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryAlarmState dialog


CQueryAlarmState::CQueryAlarmState(CWnd* pParent /*=NULL*/)
	: CDialog(CQueryAlarmState::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryAlarmState)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	memset(&m_stNetAlarmState, 0, sizeof(NET_CLIENT_ALARM_STATE));
	m_stNetAlarmState.dwSize = sizeof(NET_CLIENT_ALARM_STATE);

	memset(&m_stNetVideoLostState, 0, sizeof(NET_CLIENT_VIDEOLOST_STATE));
	m_stNetVideoLostState.dwSize = sizeof(NET_CLIENT_VIDEOLOST_STATE);

	memset(&m_stNetMotionDetectState, 0, sizeof(NET_CLIENT_MOTIONDETECT_STATE));
	m_stNetMotionDetectState.dwSize = sizeof(NET_CLIENT_MOTIONDETECT_STATE);

	memset(m_gShelterAlarm, 0, sizeof(m_gShelterAlarm));
	memset(m_gStaticAlarm, 0, sizeof(m_gStaticAlarm));

	memset(&m_stuAlarmArmDisarmState, 0, sizeof(ALARM_ARM_DISARM_STATE_INFO));	 
}


void CQueryAlarmState::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryAlarmState)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryAlarmState, CDialog)
	//{{AFX_MSG_MAP(CQueryAlarmState)
	ON_BN_CLICKED(IDC_BUTTON_QUERY, OnDoubleclickedButtonQuery)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryAlarmState message handlers

void CQueryAlarmState::OnDoubleclickedButtonQuery() 
{
	// TODO: Add your control notification handler code here
	int nRet =0 ;
	int nRetLen =0;
	LLONG lLogin =((CAlarmDlg *)(GetParent()->GetParent()))->m_LoginID;
	
	memset(&m_stNetAlarmState,0,sizeof(m_stNetAlarmState));
	m_stNetAlarmState.dwSize = sizeof(NET_CLIENT_ALARM_STATE);
	nRet = CLIENT_QueryDevState(lLogin,DH_DEVSTATE_ALARM,(char*)&m_stNetAlarmState,sizeof(m_stNetAlarmState),&nRetLen);
	if(nRet <= 0 && nRetLen == 0)
	{
		DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
		MessageBox(ConvertString("Get AlarmState failed!"), ConvertString("Prompt"));
	}
	memset(&m_stNetVideoLostState,0,sizeof(m_stNetVideoLostState));
	m_stNetVideoLostState.dwSize = sizeof(NET_CLIENT_VIDEOLOST_STATE);
	nRet = CLIENT_QueryDevState(lLogin,DH_DEVSTATE_VIDEOLOST,(char*)&m_stNetVideoLostState,sizeof(m_stNetVideoLostState),&nRetLen);
	if(nRet <= 0 && nRetLen == 0)
	{
		DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
		MessageBox(ConvertString("Get VideoLostState failed!"), ConvertString("Prompt"));
	}
	memset(&m_stNetMotionDetectState,0,sizeof(m_stNetMotionDetectState));
	m_stNetMotionDetectState.dwSize = sizeof(NET_CLIENT_MOTIONDETECT_STATE);
	nRet = CLIENT_QueryDevState(lLogin,DH_DEVSTATE_MOTIONDETECT,(char*)&m_stNetMotionDetectState,sizeof(m_stNetMotionDetectState),&nRetLen);
	if(nRet <= 0 && nRetLen == 0)
	{
		DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
		MessageBox(ConvertString("Get MotionDetectState failed!"), ConvertString("Prompt"));
	}

	memset(m_gShelterAlarm, 0, sizeof(char) * 16);
	nRet = CLIENT_QueryDevState(lLogin, DH_DEVSTATE_SHELTER_ALARM, (char*)m_gShelterAlarm, sizeof(char) * 16, &nRetLen);
	if (nRet <= 0 && nRetLen == 0)
	{
		DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
		MessageBox(ConvertString("Get shelter alarm failed!"), ConvertString("Prompt"));
	}

	//Display alarm 
	SetAlarmInfo();
}

void CQueryAlarmState::SetAlarmInfo()
{
	int ID1 = IDC_CHECK1;
	int ID2 = IDC_CHECK17;
	int ID3 = IDC_CHECK33;
	int ID4 = IDC_CHECK49;
	int ID5 = IDC_CHECK65;
	for(int i=0;i<16;i++)
	{
		if(i<m_stNetAlarmState.alarminputcount)
		{
			((CButton*)GetDlgItem(ID1 + i))->SetCheck((m_stNetAlarmState.dwAlarmState[0] >> i) & 0x01);
			((CButton*)GetDlgItem(ID1 + i))->EnableWindow(1);
		}
		else
		{
			((CButton*)GetDlgItem(ID1 + i))->EnableWindow(0);
		}
		if(i < m_stNetVideoLostState.channelcount)
		{
			((CButton*)GetDlgItem(ID2 + i))->SetCheck((m_stNetVideoLostState.dwAlarmState[0] >> i) & 0x01);
			((CButton*)GetDlgItem(ID3 + i))->SetCheck((m_stNetMotionDetectState.dwAlarmState[0] >> i) & 0x01);
			((CButton*)GetDlgItem(ID4 + i))->SetCheck(m_gShelterAlarm[i]);
			((CButton*)GetDlgItem(ID5 + i))->SetCheck(m_gStaticAlarm[i]);
			
			((CButton*)GetDlgItem(ID2 + i))->EnableWindow(1);
			((CButton*)GetDlgItem(ID3 + i))->EnableWindow(1);
			((CButton*)GetDlgItem(ID4 + i))->EnableWindow(1);
			((CButton*)GetDlgItem(ID5 + i))->EnableWindow(1);
			
		}
		else
		{
			((CButton*)GetDlgItem(ID2 + i))->EnableWindow(0);
			((CButton*)GetDlgItem(ID3 + i))->EnableWindow(0);
			((CButton*)GetDlgItem(ID4 + i))->EnableWindow(0);
			((CButton*)GetDlgItem(ID5 + i))->EnableWindow(0);
			
		}
	}

	((CButton*)GetDlgItem(IDC_CHECK_ARM_DISARM))->SetCheck(m_stuAlarmArmDisarmState.bState);
}


BOOL CQueryAlarmState::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQueryAlarmState::CleanAllInfo() 
{
	CleanColum(IDC_CHECK1, 16);
	CleanColum(IDC_CHECK17, 16);
	CleanColum(IDC_CHECK33, 16);
	CleanColum(IDC_CHECK49, 16);
	CleanColum(IDC_CHECK65, 16);
}

void CQueryAlarmState::CleanColum(UINT uID, int nLen) 
{
	for(int i = 0; i < nLen; i++)
	{
		((CButton*)GetDlgItem(uID + i))->SetCheck(FALSE);
		((CButton*)GetDlgItem(uID + i))->EnableWindow(TRUE);
	}
}