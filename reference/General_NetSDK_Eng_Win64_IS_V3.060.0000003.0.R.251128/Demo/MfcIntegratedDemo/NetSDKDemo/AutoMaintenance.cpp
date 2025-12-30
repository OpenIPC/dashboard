// AutoMaintenance.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "AutoMaintenance.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoMaintenance dialog


CAutoMaintenance::CAutoMaintenance(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoMaintenance::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAutoMaintenance)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	memset(&m_atC, 0, sizeof(DHDEV_AUTOMT_CFG));
	m_dev = NULL;
}


void CAutoMaintenance::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoMaintenance)
	DDX_Control(pDX, IDC_COMBO_AUTOTIME, m_autotime);
	DDX_Control(pDX, IDC_COMBO_AUTODEL, m_autodel);
	DDX_Control(pDX, IDC_COMBO_AUTODAY, m_autoday);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoMaintenance, CDialog)
	//{{AFX_MSG_MAP(CAutoMaintenance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoMaintenance message handlers

BOOL CAutoMaintenance::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_dev != NULL)
	{
		DWORD ret = 0;
		BOOL b = CLIENT_GetDevConfig(m_dev->LoginID, DH_DEV_AUTOMTCFG, 0, &m_atC, sizeof(DHDEV_AUTOMT_CFG), &ret, CONFIG_WAITTIME);
		if (!b || ret != sizeof(DHDEV_AUTOMT_CFG))
		{
			MessageBox(ConvertString("get auto maintenance info failed!"));
		}
		else
		{
			m_autoday.SetCurSel(m_atC.byAutoRebootDay);
			m_autotime.SetCurSel(m_atC.byAutoRebootTime);
			m_autodel.SetCurSel(m_atC.byAutoDeleteFilesTime);
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAutoMaintenance::OnOK() 
{
	if (m_dev != NULL)
	{
		m_atC.dwSize = sizeof(m_atC);
		m_atC.byAutoRebootDay = m_autoday.GetCurSel();
		m_atC.byAutoRebootTime = m_autotime.GetCurSel();
		m_atC.byAutoDeleteFilesTime = m_autodel.GetCurSel();
		BOOL b = CLIENT_SetDevConfig(m_dev->LoginID, DH_DEV_AUTOMTCFG, 0, &m_atC, sizeof(m_atC), CONFIG_WAITTIME);
		if (!b)
		{
			MessageBox(ConvertString("set auto maintenance info failed!"));
		}
	}
	CDialog::OnOK();
}
