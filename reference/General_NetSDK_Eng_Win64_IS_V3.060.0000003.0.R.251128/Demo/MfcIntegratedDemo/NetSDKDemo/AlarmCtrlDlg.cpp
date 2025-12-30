// AlarmCtrlDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "AlarmCtrlDlg.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlarmCtrlDlg dialog


CAlarmCtrlDlg::CAlarmCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAlarmCtrlDlg::IDD, pParent)
{
	m_DeviceId = 0;
	m_inNum = 0;
	m_outNum = 0;
	//{{AFX_DATA_INIT(CAlarmCtrlDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	memset(m_inState,0,sizeof(m_inState));
	memset(m_outState,0, sizeof(m_outState));
	memset(m_triggerIn,0,sizeof(m_triggerIn));
	for (int i = 0; i < MAX_IO_NUM; i++)
	{
		m_triggerIn[i].dwSize = sizeof(m_triggerIn[i]);
	}
}


void CAlarmCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlarmCtrlDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlarmCtrlDlg, CDialog)
	//{{AFX_MSG_MAP(CAlarmCtrlDlg)
	ON_BN_CLICKED(IDC_SAVE_IO, OnSaveIo)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_TRIGGER, OnTrigger)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlarmCtrlDlg message handlers

void CAlarmCtrlDlg::SetDeviceId(LLONG nDeviceId)
{
	m_DeviceId = nDeviceId;	
}


void CAlarmCtrlDlg::OnSaveIo() 
{
	int i;
	BOOL nRet = FALSE;
	
	if(m_DeviceId)
	{
		for(i = 0; i < m_inNum; i++)
		{
			if(((CButton *)GetDlgItem(IDC_CTRLIN1 + i))->GetCheck())
			{
				m_inState[i].state = 1;
			}
			else
			{
				m_inState[i].state = 0;			
			}
		}
		if(m_inNum)
		{
			nRet = CLIENT_IOControl(m_DeviceId, DH_ALARMINPUT, m_inState, sizeof(ALARM_CONTROL) *m_inNum );
			if (!nRet)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();
			}
		}
		if(!nRet)
		{
			MessageBox(ConvertString(MSG_ERROR_IOCTRL_IN));
			return;
		}
		for(int i = 0; i < m_outNum; i++)
		{
			if(((CButton *)GetDlgItem(IDC_CTRLOUT1 + i))->GetCheck())
			{
				m_outState[i].state  = 1;
			}
			else
			{
				m_outState[i].state = 0;			
			}
		}
		if(m_outNum)
		{
			nRet = CLIENT_IOControl(m_DeviceId, DH_ALARMOUTPUT, m_outState, sizeof(ALARM_CONTROL) *m_outNum );
			if (!nRet)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();
			}
		}
		if(!nRet)
		{
			MessageBox(ConvertString(MSG_ERROR_IOCTRL_OUT));
			return;
		}
	}
	CDialog::OnOK();
	// TODO: Add your control notification handler code here
	
}

void CAlarmCtrlDlg::OnCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

BOOL CAlarmCtrlDlg::OnInitDialog() 
{
	g_SetWndStaticText(this);
	
	BOOL nRet;
	int i, j;
	WORD nState;

	CDialog::OnInitDialog();

	if(m_DeviceId)
	{
		nRet = CLIENT_QueryIOControlState(m_DeviceId,DH_ALARMINPUT,NULL,0,&m_inNum);
		if(nRet)
		{
			if(m_inNum > MAX_IO_NUM)
			{
				MessageBox(ConvertString(MSG_ERROR_ALARMCHL_OVERMAX));
				goto e_close;
			}
			nRet = CLIENT_QueryIOControlState(m_DeviceId,DH_ALARMINPUT,m_inState,sizeof(ALARM_CONTROL)*m_inNum,&m_inNum);
			if (!nRet)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();
			}
		}
		if(!nRet || !m_inNum)
		{
			MessageBox(ConvertString(MSG_ERROR_ALARM_STATUS));
			goto e_close;
		}
		if(m_inNum > MAX_IO_NUM)
		{
			m_inNum = MAX_IO_NUM;
		}
		for(i = 0; i < m_inNum; i++)
		{
			nState = m_inState[i].state;
			if(nState)
			{
				((CButton *)GetDlgItem(IDC_CTRLIN1 + i))->SetCheck(1);
			}
			else
			{
				((CButton *)GetDlgItem(IDC_CTRLIN1 + i))->SetCheck(0);

			}				
		}
		for(j = m_inNum; j < MAX_IO_NUM; j++)
		{
			((CButton *)GetDlgItem(IDC_CTRLIN1 + j))->EnableWindow(false);
			((CButton *)GetDlgItem(IDC_CTRLIN17 + j))->EnableWindow(false);
		}
		//输出
		nRet = CLIENT_QueryIOControlState(m_DeviceId,DH_ALARMOUTPUT,NULL,0,&m_outNum);
		if(nRet)
		{
			if(m_outNum > MAX_IO_NUM)
			{
				MessageBox(ConvertString(MSG_ERROR_ALARMCHL_OVERMAX));
				goto e_close;
			}
			nRet = CLIENT_QueryIOControlState(m_DeviceId,DH_ALARMOUTPUT,m_outState,sizeof(ALARM_CONTROL)*m_outNum,&m_outNum);
			if (!nRet)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();
			}
		}
		if(!nRet || !m_outNum)
		{
			MessageBox(ConvertString(MSG_ERROR_ALARM_STATUS));
			goto e_close;
		}

		if(m_outNum > MAX_IO_NUM)
		{
			m_outNum = MAX_IO_NUM;
		}
		for(i = 0; i < m_outNum; i++)
		{
			nState = m_outState[i].state;
			if(nState)
			{
				((CButton *)GetDlgItem(IDC_CTRLOUT1 + i))->SetCheck(1);
			}
			else
			{
				((CButton *)GetDlgItem(IDC_CTRLOUT1 + i))->SetCheck(0);

			}				
		}
		for(j = m_outNum; j < MAX_IO_NUM; j++)
		{
			((CButton *)GetDlgItem(IDC_CTRLOUT1 + j))->EnableWindow(false);
		}

	}	

	memset(m_triggerIn, 0, sizeof(ALARMCTRL_PARAM)*MAX_IO_NUM);
	
	return TRUE; 

e_close:
	
	((CButton *)GetDlgItem(IDC_SAVE_IO))->EnableWindow(FALSE);
	return TRUE;       
}


void CAlarmCtrlDlg::OnTrigger() 
{
	//解发网络报警
	for(int i = 0; i < m_inNum; i++)
	{
		m_triggerIn[i].dwSize = sizeof(ALARMCTRL_PARAM);
		m_triggerIn[i].nAlarmNo = i;
		if(((CButton *)GetDlgItem(IDC_CTRLIN17 + i))->GetCheck())
		{
			m_triggerIn[i].nAction = 1;
		}
		else
		{
			m_triggerIn[i].nAction = 0;			
		}
		
		BOOL b = CLIENT_ControlDevice(m_DeviceId, DH_TRIGGER_ALARM_IN, &m_triggerIn[i], 3000);
		if (!b)
		{
			MessageBox(ConvertString("trigger failed!"));
			((CNetSDKDemoDlg *)GetParent())->LastError();
			break;
		}
	}
}