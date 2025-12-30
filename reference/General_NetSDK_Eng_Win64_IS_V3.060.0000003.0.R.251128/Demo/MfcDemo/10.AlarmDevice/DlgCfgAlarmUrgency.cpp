// DlgCfgAlarmUrgency.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmUrgency.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmUrgency dialog


CDlgCfgAlarmUrgency::CDlgCfgAlarmUrgency(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgCfgAlarmUrgency::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmUrgency)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuUrgency, 0, sizeof(CFG_URGENCY_INFO));
}


void CDlgCfgAlarmUrgency::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmUrgency)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_URGENCY_CHECK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmUrgency, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmUrgency)
	ON_BN_CLICKED(IDC_URGENCY_BUTTON_GET, OnUrgencyButtonGet)
	ON_BN_CLICKED(IDC_URGENCY_BUTTON_SET, OnUrgencyButtonSet)
	ON_BN_CLICKED(IDC_URGENCY_BUTTON_EVENTHANDLER, OnUrgencyButtonEventhandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmUrgency message handlers

BOOL CDlgCfgAlarmUrgency::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this, DLG_CFG_URGENCY);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_CFG_URGENCY), ConvertString("Prompt"));
	}
	
	if (GetConfigFromDevice())
	{
		showInfo();
		return TRUE;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgCfgAlarmUrgency::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = -1;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_URGENCY, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
	
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_URGENCY, szJsonBuf, (void*)&m_stuUrgency, sizeof(m_stuUrgency), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse Urgency error..."), DLG_CFG_URGENCY), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig Urgency error:", DLG_CFG_URGENCY), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgAlarmUrgency::SetConfigToDevice()
{
	char szJsonBuf[1024 * 40] = {0};	
	BOOL bRet = CLIENT_PacketData(CFG_CMD_URGENCY, &m_stuUrgency, sizeof(m_stuUrgency), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet Urgency error..."), DLG_CFG_URGENCY), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_URGENCY, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig Urgency failed:", DLG_CFG_URGENCY), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig Urgency ok!"), DLG_CFG_URGENCY), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgAlarmUrgency::showInfo()
{
	m_ckEnable.SetCheck(m_stuUrgency.bEnable ? BST_CHECKED : BST_UNCHECKED);
	return TRUE;
}

BOOL CDlgCfgAlarmUrgency::getInfo()
{
	m_stuUrgency.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
	return TRUE;
}

void CDlgCfgAlarmUrgency::OnUrgencyButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		showInfo();
	}	
}

void CDlgCfgAlarmUrgency::OnUrgencyButtonSet() 
{
	// TODO: Add your control notification handler code here
	getInfo();
	SetConfigToDevice();	
}

void CDlgCfgAlarmUrgency::OnUrgencyButtonEventhandler() 
{
	// TODO: Add your control notification handler code here
	CSubDlgCfgEventHandler* pdlgEventHandler = new CSubDlgCfgEventHandler;
	if (pdlgEventHandler != NULL)
	{
		pdlgEventHandler->SetEventHandler(m_stuUrgency.stuEventHandler);
		pdlgEventHandler->SetConfigType(emConfigType_AlarmChassisIntrusion);
		
		if (IDOK == pdlgEventHandler->DoModal())
		{
			const CFG_ALARM_MSG_HANDLE& stuEventHandler = pdlgEventHandler->GetEventHandler();
			memcpy(&m_stuUrgency.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
		}
		
		delete pdlgEventHandler;
		pdlgEventHandler = NULL;
	}
	else
	{
		MessageBox(ConvertString("new handler dialog error!", DLG_CFG_URGENCY), ConvertString("Prompt"));
	}
}
