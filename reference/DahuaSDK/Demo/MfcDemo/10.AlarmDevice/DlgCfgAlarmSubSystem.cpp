// DlgCfgAlarmSubSystem.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgAlarmSubSystem.h"
#include "SubDlgCfgTimeSection.h"
#include "SubDlgSensorInfoDescription.h"
#include <VECTOR>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSubSystem dialog


CDlgCfgAlarmSubSystem::CDlgCfgAlarmSubSystem(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgAlarmSubSystem::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgAlarmSubSystem)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginID = lLoginID;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgAlarmSubSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgAlarmSubSystem)
	DDX_Control(pDX, IDC_ALARMSUBSYSTEMCFG_CHK_ISPUBLIC, m_chkIsPublic);
	DDX_Control(pDX, IDC_ALARMSUBSYSTEMCFG_CMB_SEQ, m_cmbSeq);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgAlarmSubSystem, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgAlarmSubSystem)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEMCFG_BTN_GET, OnAlarmsubsystemBtnGet)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEMCFG_BTN_SET, OnAlarmsubsystemBtnSet)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEMCFG_BTN_ZONE, OnAlarmsubsystemBtnZone)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEMCFG_BTN_EXZONE, OnAlarmsubsystemBtnExZone)
	ON_BN_CLICKED(IDC_ALARMSUBSYSTEMCFG_BTN_PUBSUBSYSTEM, OnAlarmsubsystemBtnPubsubsystem)
	ON_CBN_SELCHANGE(IDC_ALARMSUBSYSTEMCFG_CMB_SEQ, OnSelchangeAlarmsubsystemCmbSeq)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSubSystem private method

void CDlgCfgAlarmSubSystem::InitDlg()
{
	// seq
	m_cmbSeq.ResetContent();
	const int nSeq = 8;
	for (int i = 0; i < nSeq; i++)
	{
		CString csSeq;
		csSeq.Format("%d", i + 1);
		m_cmbSeq.InsertString(-1, csSeq);
	}
	m_cmbSeq.SetCurSel(0);

	// name
	SetDlgItemText(IDC_ALARMSUBSYSTEMCFG_EDT_NAME, NULL);

	// zone

	// ex-zone

	// time section

	// disable delay
	SetDlgItemInt(IDC_ALARMSUBSYSTEMCFG_EDT_DISDELAY, 0);

	// enable delay
	SetDlgItemInt(IDC_ALARMSUBSYSTEMCFG_EDT_ENDELAY, 0);

	// is public
	m_chkIsPublic.SetCheck(BST_UNCHECKED);

	// public alarm subsystem
}

void CDlgCfgAlarmSubSystem::StuToDlg()
{
	// name
	SetDlgItemText(IDC_ALARMSUBSYSTEMCFG_EDT_NAME, m_stuInfo.szName);
	
	// zone
	
	// ex-zone
	
	// time section
	
	// disable delay
	SetDlgItemInt(IDC_ALARMSUBSYSTEMCFG_EDT_DISDELAY, m_stuInfo.nDisableDelay);
	
	// enable delay
	SetDlgItemInt(IDC_ALARMSUBSYSTEMCFG_EDT_ENDELAY, m_stuInfo.nEnableDelay);
	
	// is public
	if (m_stuInfo.bIsPublic)
	{
		m_chkIsPublic.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkIsPublic.SetCheck(BST_UNCHECKED);
	}

	// public alarm subsystem
}

void CDlgCfgAlarmSubSystem::DlgToStu()
{
	// name
	GetDlgItemText(IDC_ALARMSUBSYSTEMCFG_EDT_NAME, m_stuInfo.szName, sizeof(m_stuInfo.szName) - 1);
	
	// zone
	
	// ex-zone
	
	// time section
	
	// disable delay
	m_stuInfo.nDisableDelay = GetDlgItemInt(IDC_ALARMSUBSYSTEMCFG_EDT_DISDELAY, NULL, TRUE);
	
	// enable delay
	m_stuInfo.nEnableDelay = GetDlgItemInt(IDC_ALARMSUBSYSTEMCFG_EDT_ENDELAY, NULL, TRUE);
	
	// is public
	m_stuInfo.bIsPublic = m_chkIsPublic.GetCheck() ? TRUE : FALSE;
	
	// public alarm subsystem
}

BOOL CDlgCfgAlarmSubSystem::GetConfig()
{
	int nChn = m_cmbSeq.GetCurSel();
	if (-1 == nChn)
	{
		return FALSE;
	}
	char szBuf[1024*10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_ALARM_SUBSYSTEM, nChn, szBuf, sizeof(szBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		CString csInfo;
		csInfo.Format("%s:%08x", ConvertString("Get config of AlarmSubSystem error", DLG_CFG_ALARM_SUBSYSTEM), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}
	
	int nRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_ALARM_SUBSYSTEM, szBuf, &m_stuInfo, sizeof(m_stuInfo), &nRetLen);
	if (!bRet || nRetLen != sizeof(m_stuInfo))
	{
		MessageBox(ConvertString("Parse config of AlarmSubSystem error", DLG_CFG_ALARM_SUBSYSTEM), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgAlarmSubSystem::SetConfig()
{
	int nChn = m_cmbSeq.GetCurSel();
	if (-1 == nChn)
	{
		return FALSE;
	}
	char szBuf[1024*10] = {0};
	int nErr = 0;
	int nRestart = 0;
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ALARM_SUBSYSTEM, &m_stuInfo, sizeof(m_stuInfo), szBuf, sizeof(szBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet config of AlarmSubSystem error", DLG_CFG_ALARM_SUBSYSTEM), ConvertString("Prompt"));
		return FALSE;
	}
	bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_ALARM_SUBSYSTEM, nChn, szBuf, strlen(szBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!bRet)
	{
		CString csInfo;
		csInfo.Format("%s:%08x", ConvertString("Set config of AlarmSubSystem error", DLG_CFG_ALARM_SUBSYSTEM), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}
	else
	{
		MessageBox(ConvertString("Set config of AlarmSubSystem ok", DLG_CFG_ALARM_SUBSYSTEM), ConvertString("Prompt"));
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgAlarmSubSystem message handlers

BOOL CDlgCfgAlarmSubSystem::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ALARM_SUBSYSTEM);
	// TODO: Add extra initialization here
	InitDlg();
	if (GetConfig())
	{
		StuToDlg();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgAlarmSubSystem::OnAlarmsubsystemBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfig())
	{
		StuToDlg();
	}	
}

void CDlgCfgAlarmSubSystem::OnAlarmsubsystemBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfig();
}

void CDlgCfgAlarmSubSystem::OnAlarmsubsystemBtnZone() 
{
	// TODO: Add your control notification handler code here
	std::vector<int> vecChn;
	int i = 0;
	for (; i < m_stuInfo.nZoneNum; i++)
	{
		vecChn.push_back(m_stuInfo.anZone[i]);
	}
	
	CSubDlgSensorInfoDescription dlg(this, 256);
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		vecChn.clear();
		vecChn = dlg.GetID();
		std::vector<int>::iterator it = vecChn.begin();
		for (i = 0; i < __min(vecChn.size(), DH_MAX_ZONE_NUM) && it != vecChn.end(); i++, it++)
		{
			m_stuInfo.anZone[i] = *it;
		}
		m_stuInfo.nZoneNum = __min(vecChn.size(), DH_MAX_ZONE_NUM);
	}	
}

void CDlgCfgAlarmSubSystem::OnAlarmsubsystemBtnExZone() 
{
	// TODO: Add your control notification handler code here
	std::vector<int> vecChn;
	int i = 0;
	for (; i < m_stuInfo.nExZoneNum; i++)
	{
		vecChn.push_back(m_stuInfo.anExZone[i]);
	}
	
	CSubDlgSensorInfoDescription dlg(this, 32);
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		vecChn.clear();
		vecChn = dlg.GetID();
		std::vector<int>::iterator it = vecChn.begin();
		for (i = 0; i < __min(vecChn.size(), DH_MAX_ZONE_NUM) && it != vecChn.end(); i++, it++)
		{
			m_stuInfo.anExZone[i] = *it;
		}
		m_stuInfo.nExZoneNum = __min(vecChn.size(), DH_MAX_ZONE_NUM);
	}		
}

void CDlgCfgAlarmSubSystem::OnAlarmsubsystemBtnPubsubsystem() 
{
	// TODO: Add your control notification handler code here
	std::vector<int> vecChn;
	int i = 0;
	for (; i < m_stuInfo.nPublicSubSystem; i++)
	{
		vecChn.push_back(m_stuInfo.anPublicSubSystem[i]);
	}
	
	CSubDlgSensorInfoDescription dlg(this, DH_MAX_PUBLIC_SUBSYSTEM_NUM);
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		vecChn.clear();
		vecChn = dlg.GetID();
		std::vector<int>::iterator it = vecChn.begin();
		for (i = 0; i < __min(vecChn.size(), DH_MAX_PUBLIC_SUBSYSTEM_NUM) && it != vecChn.end(); i++, it++)
		{
			m_stuInfo.anPublicSubSystem[i] = *it;
		}
		m_stuInfo.nPublicSubSystem = __min(vecChn.size(), DH_MAX_PUBLIC_SUBSYSTEM_NUM);
	}			
}

void CDlgCfgAlarmSubSystem::OnSelchangeAlarmsubsystemCmbSeq() 
{
	// TODO: Add your control notification handler code here
	int nSel = m_cmbSeq.GetCurSel();
	if (-1 == nSel)
	{
		return;
	}
	if (GetConfig())
	{
		StuToDlg();
	}
}
