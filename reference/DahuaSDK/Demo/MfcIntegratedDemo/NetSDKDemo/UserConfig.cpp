// UserConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "UserConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserConfig dialog

CUserConfig::CUserConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CUserConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserConfig)
	m_bLright_log = FALSE;
	m_bLright_more = FALSE;
	m_bLright_playback = FALSE;
	m_bLright_ptz = FALSE;
	m_bLright_rec = FALSE;
	m_mac1 = _T("");
	m_mac2 = _T("");
	m_mac3 = _T("");
	m_mac4 = _T("");
	m_mac5 = _T("");
	m_mac6 = _T("");
	m_bRright_alarm = FALSE;
	m_bRright_cfg = FALSE;
	m_bRright_localop = FALSE;
	m_bRright_log = FALSE;
	m_bRright_monitor = FALSE;
	m_bRright_more = FALSE;
	m_bRright_playback = FALSE;
	m_bRright_ptz = FALSE;
	m_bRright_rec = FALSE;
	m_bRright_serial = FALSE;
	m_bRright_talk = FALSE;
	m_userip = _T("");
	m_username = _T("");
	m_userpsw = _T("");
	m_userpsw2 = _T("");
	m_bLright_cfg = FALSE;
	//}}AFX_DATA_INIT
}


void CUserConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserConfig)
	DDX_Control(pDX, IDC_USERSEL, m_usersel);
	DDX_Check(pDX, IDC_LRIGHT_LOG, m_bLright_log);
	DDX_Check(pDX, IDC_LRIGHT_MORE, m_bLright_more);
	DDX_Check(pDX, IDC_LRIGHT_PLAYBACK, m_bLright_playback);
	DDX_Check(pDX, IDC_LRIGHT_PTZ, m_bLright_ptz);
	DDX_Check(pDX, IDC_LRIGHT_REC, m_bLright_rec);
	DDX_Text(pDX, IDC_MAC1, m_mac1);
	DDX_Text(pDX, IDC_MAC2, m_mac2);
	DDX_Text(pDX, IDC_MAC3, m_mac3);
	DDX_Text(pDX, IDC_MAC4, m_mac4);
	DDX_Text(pDX, IDC_MAC5, m_mac5);
	DDX_Text(pDX, IDC_MAC6, m_mac6);
	DDX_Check(pDX, IDC_RRIGHT_ALARM, m_bRright_alarm);
	DDX_Check(pDX, IDC_RRIGHT_CFG, m_bRright_cfg);
	DDX_Check(pDX, IDC_RRIGHT_LOCALOP, m_bRright_localop);
	DDX_Check(pDX, IDC_RRIGHT_LOG, m_bRright_log);
	DDX_Check(pDX, IDC_RRIGHT_MONITOR, m_bRright_monitor);
	DDX_Check(pDX, IDC_RRIGHT_MORE, m_bRright_more);
	DDX_Check(pDX, IDC_RRIGHT_PLAYBACK, m_bRright_playback);
	DDX_Check(pDX, IDC_RRIGHT_PTZ, m_bRright_ptz);
	DDX_Check(pDX, IDC_RRIGHT_REC, m_bRright_rec);
	DDX_Check(pDX, IDC_RRIGHT_SERIAL, m_bRright_serial);
	DDX_Check(pDX, IDC_RRIGHT_TALK, m_bRright_talk);
	DDX_Text(pDX, IDC_USERIP, m_userip);
	DDX_Text(pDX, IDC_USERNAME, m_username);
	DDX_Text(pDX, IDC_USERPSW, m_userpsw);
	DDX_Text(pDX, IDC_USERPSW2, m_userpsw2);
	DDX_Check(pDX, IDC_LRIGHT_CFG, m_bLright_cfg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserConfig, CDialog)
	//{{AFX_MSG_MAP(CUserConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserConfig message handlers

void CUserConfig::SetDevice(DeviceNode *nDev)
{
	m_Device = nDev;
}

BOOL CUserConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);
	
	CString csUserName;
	DWORD dwReturned;
	bSetUserParams = FALSE;
	char cTemp[100];

//	if(m_bConnectServer)
//	{
		if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_USERCFG , 0,&m_user, sizeof(NET_DEV_USER), &dwReturned, CONFIG_WAITTIME))
		{
		//	CString sTemp;
		//	sTemp.Format("ERROR: NET_DVR_GET_USERCFG = %d \n", NET_DVR_GetLastError());
			AfxMessageBox(ConvertString("usercfg error"));	
		}
		else
		{
			if(m_bSuperUser)
			{
				m_usernum = 0;
				m_usersel.SetCurSel(m_usernum);
			}
			else
			{
				for(int i=0; i<MAX_USERNUM; i++)
				{
					ZeroMemory(cTemp, 100);
					memcpy(cTemp, m_user.struUser[i].sUserName, MAX_NAMELEN);
					csUserName.Format("%s", cTemp);
					if(strcmp(csUserName, m_Device->UserNanme) == 0)
					{
						break;
					}
				}
				m_usernum = i;
				m_usersel.SetCurSel(m_usernum);
			}
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, m_user.struUser[m_usernum].sUserName, MAX_NAMELEN);
			m_username.Format("%s", cTemp);
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, m_user.struUser[m_usernum].sPassword, PASSWD_LEN);
			m_userpsw.Format("%s", cTemp);
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, m_user.struUser[m_usernum].sPassword, PASSWD_LEN);
			m_userpsw2.Format("%s", cTemp);
			m_userip.Format("%s",m_user.struUser[m_usernum].sUserIP);
			m_mac1.Format("%02x",m_user.struUser[m_usernum].byMACAddr[0]);
			m_mac2.Format("%02x",m_user.struUser[m_usernum].byMACAddr[1]);
			m_mac3.Format("%02x",m_user.struUser[m_usernum].byMACAddr[2]);
			m_mac4.Format("%02x",m_user.struUser[m_usernum].byMACAddr[3]);
			m_mac5.Format("%02x",m_user.struUser[m_usernum].byMACAddr[4]);
			m_mac6.Format("%02x",m_user.struUser[m_usernum].byMACAddr[5]);		
			m_bLright_ptz = m_user.struUser[m_usernum].dwLocalRight[0];
			m_bLright_rec = m_user.struUser[m_usernum].dwLocalRight[1];
			m_bLright_playback = m_user.struUser[m_usernum].dwLocalRight[2];
			m_bLright_cfg = m_user.struUser[m_usernum].dwLocalRight[3];
			m_bLright_log = m_user.struUser[m_usernum].dwLocalRight[4];
			m_bLright_more = m_user.struUser[m_usernum].dwLocalRight[5];
			m_bRright_ptz = m_user.struUser[m_usernum].dwRemoteRight[0];
			m_bRright_rec = m_user.struUser[m_usernum].dwRemoteRight[1];
			m_bRright_playback = m_user.struUser[m_usernum].dwRemoteRight[2];
			m_bRright_cfg = m_user.struUser[m_usernum].dwRemoteRight[3];
			m_bRright_log = m_user.struUser[m_usernum].dwRemoteRight[4];
			m_bRright_more = m_user.struUser[m_usernum].dwRemoteRight[5];
			m_bRright_talk = m_user.struUser[m_usernum].dwRemoteRight[6];
			m_bRright_monitor = m_user.struUser[m_usernum].dwRemoteRight[7];
			m_bRright_alarm = m_user.struUser[m_usernum].dwRemoteRight[8];
			m_bRright_localop = m_user.struUser[m_usernum].dwRemoteRight[9];
			m_bRright_serial = m_user.struUser[m_usernum].dwRemoteRight[10];
			GetDlgItem(IDC_USERSEL)->EnableWindow(FALSE);
			GetDlgItem(IDC_USERIP)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAC1)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAC2)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAC3)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAC4)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAC5)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAC6)->EnableWindow(FALSE);
			GetDlgItem(IDC_USERNAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_USERPSW)->EnableWindow(FALSE);
			GetDlgItem(IDC_USERPSW2)->EnableWindow(FALSE);			
			GetDlgItem(IDC_LRIGHT_PTZ)->EnableWindow(FALSE);
			GetDlgItem(IDC_LRIGHT_REC)->EnableWindow(FALSE);	
			GetDlgItem(IDC_LRIGHT_PLAYBACK)->EnableWindow(FALSE);
			GetDlgItem(IDC_LRIGHT_CFG)->EnableWindow(FALSE);	
			GetDlgItem(IDC_LRIGHT_LOG)->EnableWindow(FALSE);
			GetDlgItem(IDC_LRIGHT_MORE)->EnableWindow(FALSE);	
			GetDlgItem(IDC_RRIGHT_PTZ)->EnableWindow(FALSE);	
			GetDlgItem(IDC_RRIGHT_REC)->EnableWindow(FALSE);
			GetDlgItem(IDC_RRIGHT_PLAYBACK)->EnableWindow(FALSE);	
			GetDlgItem(IDC_RRIGHT_CFG)->EnableWindow(FALSE);
			GetDlgItem(IDC_RRIGHT_LOG)->EnableWindow(FALSE);
			GetDlgItem(IDC_RRIGHT_MORE)->EnableWindow(FALSE);	
			GetDlgItem(IDC_RRIGHT_TALK)->EnableWindow(FALSE);
			GetDlgItem(IDC_RRIGHT_MONITOR)->EnableWindow(FALSE);	
			GetDlgItem(IDC_RRIGHT_ALARM)->EnableWindow(FALSE);
			GetDlgItem(IDC_RRIGHT_LOCALOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_RRIGHT_SERIAL)->EnableWindow(FALSE);
			
			if(m_bSuperUser)
			{
				GetDlgItem(IDC_USERSEL)->EnableWindow(FALSE);
				GetDlgItem(IDC_USERIP)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAC1)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAC2)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAC3)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAC4)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAC5)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAC6)->EnableWindow(FALSE);
				GetDlgItem(IDC_USERNAME)->EnableWindow(FALSE);
				GetDlgItem(IDC_USERPSW)->EnableWindow(FALSE);
				GetDlgItem(IDC_USERPSW2)->EnableWindow(FALSE);	
			}		
			GetDlgItem(IDC_USERCFG_APPLY)->EnableWindow(TRUE);
		}
//	}
	
	GetDlgItem(IDC_USERNAME)->EnableWindow(FALSE);
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
