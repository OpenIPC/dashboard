// DlgCfgAlarmChassisIntrusion.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmChassisIntrusion.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmChassisIntrusion dialog


CDlgCfgAlarmChassisIntrusion::CDlgCfgAlarmChassisIntrusion(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgCfgAlarmChassisIntrusion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmChassisIntrusion)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuChassisIntrusion, 0, sizeof(CFG_CHASSISINTRUSION_INFO));
}


void CDlgCfgAlarmChassisIntrusion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmChassisIntrusion)
	DDX_Control(pDX, IDC_CHASSISINTRUSION_CHECK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmChassisIntrusion, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmChassisIntrusion)
	ON_BN_CLICKED(IDC_CHASSISINTRUSION_BUTTON_GET, OnChassisintrusionButtonGet)
	ON_BN_CLICKED(IDC_CHASSISINTRUSION_BUTTON_SET, OnChassisintrusionButtonSet)
	ON_BN_CLICKED(IDC_CHASSISINTRUSION_BUTTON_EVENTHANDLER, OnChassisintrusionButtonEventhandler)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmChassisIntrusion message handlers

BOOL CDlgCfgAlarmChassisIntrusion::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this, DLG_ALARM_CHASSISINTRUSION);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_ALARM_CHASSISINTRUSION), ConvertString("Prompt"));
	}
	
 	if (GetConfigFromDevice())
	{
		showInfo();
		return TRUE;
	}
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgCfgAlarmChassisIntrusion::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = -1;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_CHASSISINTRUSION, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);

	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_CHASSISINTRUSION, szJsonBuf, (void*)&m_stuChassisIntrusion, sizeof(m_stuChassisIntrusion), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse ChassisIntrusion error..."), DLG_ALARM_CHASSISINTRUSION), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x",ConvertString("QueryConfig ChassisIntrusion error:", DLG_ALARM_CHASSISINTRUSION), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgAlarmChassisIntrusion::SetConfigToDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_CHASSISINTRUSION, &m_stuChassisIntrusion, sizeof(m_stuChassisIntrusion), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet ChassisIntrusion error..."), DLG_ALARM_CHASSISINTRUSION), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_CHASSISINTRUSION, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig ChassisIntrusion failed:", DLG_ALARM_CHASSISINTRUSION), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig ChassisIntrusion ok!"), DLG_ALARM_CHASSISINTRUSION), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgAlarmChassisIntrusion::showInfo()
{
	m_ckEnable.SetCheck(m_stuChassisIntrusion.bEnable ? BST_CHECKED : BST_UNCHECKED);
	return TRUE;
}

BOOL CDlgCfgAlarmChassisIntrusion::getInfo()
{
	m_stuChassisIntrusion.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
	return TRUE;
}

void CDlgCfgAlarmChassisIntrusion::OnChassisintrusionButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		showInfo();
	}
}

void CDlgCfgAlarmChassisIntrusion::OnChassisintrusionButtonSet() 
{
	// TODO: Add your control notification handler code here
	getInfo();
	SetConfigToDevice();
}

void CDlgCfgAlarmChassisIntrusion::OnChassisintrusionButtonEventhandler() 
{
	// TODO: Add your control notification handler code here
	// 先设置，后获取
	CSubDlgCfgEventHandler* pdlgEventHandler = new CSubDlgCfgEventHandler;
	if (pdlgEventHandler != NULL)
	{
		pdlgEventHandler->SetEventHandler(m_stuChassisIntrusion.stuEventHandler);
		pdlgEventHandler->SetConfigType(emConfigType_AlarmChassisIntrusion);
		
		if (IDOK == pdlgEventHandler->DoModal())
		{
			const CFG_ALARM_MSG_HANDLE& stuEventHandler = pdlgEventHandler->GetEventHandler();
			memcpy(&m_stuChassisIntrusion.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
		}

		delete pdlgEventHandler;
		pdlgEventHandler = NULL;
	}
	else
	{
		MessageBox(ConvertString("new handler dialog error!", DLG_ALARM_CHASSISINTRUSION), ConvertString("Prompt"));
	}

}

void CDlgCfgAlarmChassisIntrusion::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
