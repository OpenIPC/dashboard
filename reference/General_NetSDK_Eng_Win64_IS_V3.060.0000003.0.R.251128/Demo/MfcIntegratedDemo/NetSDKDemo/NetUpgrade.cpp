// NetUpgrade.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "NetUpgrade.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_SETPROGRESS	(WM_USER+100)
/////////////////////////////////////////////////////////////////////////////
// CNetUpgrade dialog


CNetUpgrade::CNetUpgrade(CWnd* pParent /*=NULL*/)
	: CDialog(CNetUpgrade::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNetUpgrade)
	m_strFilePath = _T("");
	//}}AFX_DATA_INIT

	m_pDevice = NULL;
	m_lUpgradeID = 0;
	m_bStopped = 0;
}


void CNetUpgrade::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetUpgrade)
	DDX_Control(pDX, IDC_UPGRADEPROGRESS, m_upgradeProgress);
	DDX_Text(pDX, IDC_FILEPATH, m_strFilePath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNetUpgrade, CDialog)
	//{{AFX_MSG_MAP(CNetUpgrade)
	ON_BN_CLICKED(IDC_OPENFILE, OnOpenfile)
	ON_BN_CLICKED(IDC_SENDBIOS, OnSendbios)
	ON_BN_CLICKED(IDC_SENDWEB, OnSendweb)
	ON_BN_CLICKED(IDC_SENDBOOT, OnSendboot)
	ON_BN_CLICKED(IDC_BIOSANDBOOT, OnBiosandboot)
	ON_BN_CLICKED(IDC_SEND_PERIPHERAL, OnPeripheral)
	ON_BN_CLICKED(IDC_SENDGEOINFO, OnGeoInfo)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_UP_CANCEL, OnUpCancel)
	ON_MESSAGE(WM_SETPROGRESS, OnSetProgress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetUpgrade message handlers

void CNetUpgrade::OnOpenfile() 
{
	char buffer[32];
	CString str;
	memset(buffer,0,sizeof(buffer));
	
	//如果当前正在发送数据则不能打开文件。
	if(m_lUpgradeID) 
	{
		return;
	}
	
	CString strFilter = "Upgrade File(*.bin)|*.bin|All Files|*.*|";
	CFileDialog dlg(TRUE, NULL,NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
		strFilter, this);
	
	if(dlg.DoModal() == IDOK)
	{
		m_strFilePath = dlg.GetPathName();
		
		if(m_File.m_hFile != CFile::hFileNull) 
		{
			m_File.Close();
		}
		
		if(!m_File.Open(m_strFilePath, CFile::modeRead|CFile::typeBinary))
		{
			CString str;
			str.Format("%s %s.",ConvertString(MSG_UG_OPENFILE), m_strFilePath, MSG_UG_FAILED);
			MessageBox(str, ConvertString(MSG_UG_ERROR), MB_ICONERROR);
			return;
		}
		ULONGLONG nFileSize = m_File.GetLength();
		if (nFileSize < 2)
		{
			AfxMessageBox(ConvertString(MSG_FILE_MISSTYPE));
			m_File.Close();
			return;
		}
		
		m_File.Read(buffer,2); //取文件头两个字节
		str = buffer;
		if(str == "DH" || str == "PK") //Bios文件
		{
			GetDlgItem(IDC_SENDBIOS)->EnableWindow(TRUE);	
			GetDlgItem(IDC_SENDWEB)->EnableWindow(FALSE);
			GetDlgItem(IDC_SENDBOOT)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BIOSANDBOOT)->EnableWindow(FALSE);
		}
		else if(str == "U") //Web文件
		{
			GetDlgItem(IDC_SENDBIOS)->EnableWindow(FALSE);	
			GetDlgItem(IDC_SENDWEB)->EnableWindow(TRUE);
			GetDlgItem(IDC_SENDBOOT)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BIOSANDBOOT)->EnableWindow(FALSE);
		}
		else if(buffer[0] == (char)0xC0 && buffer[1]==(char)0x80) //boot文件
		{
			//boot文件
			if(nFileSize <= 1202180)
			{
				GetDlgItem(IDC_SENDBIOS)->EnableWindow(FALSE);	
				GetDlgItem(IDC_SENDWEB)->EnableWindow(FALSE);
				GetDlgItem(IDC_SENDBOOT)->EnableWindow(TRUE);	
				GetDlgItem(IDC_BIOSANDBOOT)->EnableWindow(FALSE);
			}
			//bios and boot文件
			else
			{
				GetDlgItem(IDC_SENDBIOS)->EnableWindow(FALSE);	
				GetDlgItem(IDC_SENDWEB)->EnableWindow(FALSE);
				GetDlgItem(IDC_BIOSANDBOOT)->EnableWindow(TRUE);	
				GetDlgItem(IDC_SENDBOOT)->EnableWindow(FALSE);
			}
		}
		else
		{
			//AfxMessageBox(ConvertString(MSG_FILE_MISSTYPE));
			if(m_File.m_hFile != CFile::hFileNull) 
			{
				m_File.Close();
			}
			GetDlgItem(IDC_SENDBIOS)->EnableWindow(FALSE);	
			GetDlgItem(IDC_SENDWEB)->EnableWindow(FALSE);
			GetDlgItem(IDC_SENDBOOT)->EnableWindow(FALSE);	
			GetDlgItem(IDC_BIOSANDBOOT)->EnableWindow(FALSE);
			return;
		}
		m_File.Close();
		
		UpdateData(FALSE);
	}	
}

void CALLBACK UpgradeCallBackFunc(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize, LDWORD dwUser)
{
	CNetUpgrade * pThis = (CNetUpgrade *)dwUser;
	if (-1 == nSendSize)
	{
		pThis->SetProgressPos(-1);
	}
	else if (-2 == nSendSize)
	{
		pThis->SetProgressPos(-2);
	}
	else if (-1 == nTotalSize)//升级进度回调
	{
		char szTemp[128] = {0};
		_snprintf(szTemp, sizeof(szTemp), "===nUpdate: %d\n", nSendSize);
		OutputDebugStringA(szTemp);
		if (nSendSize < 0 || nSendSize > 100)
		{
		}
		else
		{
			//pThis->SetProgressPos(nSendSize);
			pThis->PostMessage(WM_SETPROGRESS, 0, nSendSize);
		}
	}
	else
	{
		int nProgress = nSendSize/(nTotalSize/100);
		char szTemp[256] = {0};
		_snprintf(szTemp, sizeof(szTemp), "nProgress: %d, nSendSize: %d, nTotalSize: %d\n", nProgress, nSendSize, nTotalSize);
		OutputDebugStringA(szTemp);
		pThis->SetProgressPos(nProgress);
	}	
	return;
}

void CNetUpgrade::ProcessSend(unsigned int nUpgradeType)
{
	if (m_lUpgradeID)
	{
		return; //已经在升级了
	}
	LLONG lUpgradeID = 0;
    if (0 == nUpgradeType)
    {
        lUpgradeID = CLIENT_StartUpgrade(m_pDevice->LoginID, m_strFilePath.GetBuffer(0),
		UpgradeCallBackFunc, (LDWORD)this);
    }
	// 使用扩展接口，指定升级类型
	else
	{
		lUpgradeID = CLIENT_StartUpgradeEx(m_pDevice->LoginID, (EM_UPGRADE_TYPE)nUpgradeType, m_strFilePath.GetBuffer(0),
		UpgradeCallBackFunc, (LDWORD)this);
	}
	
	if (!lUpgradeID)
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		MessageBox(ConvertString(MSG_UG_STARTFAILED));
	}
	else
	{
		m_bStopped = FALSE;
		if ( !CLIENT_SendUpgrade(lUpgradeID))
		{
			m_bStopped = TRUE;
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
			MessageBox(ConvertString(MSG_UG_SENDFAILED));
			CLIENT_StopUpgrade(lUpgradeID);
		}
		else
		{
			m_lUpgradeID = lUpgradeID;
		}
	}
}

void CNetUpgrade::OnSendbios() 
{
	ProcessSend();
}

void CNetUpgrade::OnSendweb() 
{
	ProcessSend();
}

void CNetUpgrade::OnSendboot() 
{
	ProcessSend();
}

void CNetUpgrade::OnBiosandboot() 
{
	ProcessSend();
}

void CNetUpgrade::OnPeripheral() 
{
	ProcessSend(DH_UPGRADE_PERIPHERAL_TYPE);
}

void CNetUpgrade::OnGeoInfo() 
{
	ProcessSend(DH_UPGRADE_GEOINFO_TYPE);
}

void CNetUpgrade::SetProgressPos(int npos)
{
	if (m_bStopped)
	{
		return;
	}
	if (-1 == npos) 
	{
		if (m_lUpgradeID)
		{
			BOOL bret = CLIENT_StopUpgrade(m_lUpgradeID);
			if (!bret) 
			{
				int test = 0;
				MessageBox(ConvertString(MSG_UG_STOPFAILED));
			}
			else
			{
				MessageBox(ConvertString(MSG_UG_DONE));
			//	EndDialog(0);
			}
			m_lUpgradeID = 0;
			m_upgradeProgress.SetPos(0);
			GetDlgItem(IDC_UPPROGRESS)->SetWindowText(ConvertString(MSG_UP_PROGRESS));
		}
	}
	else if (-2 == npos)
	{
		MessageBox(ConvertString("Send upgrade data error!"));
		if (m_lUpgradeID)
		{
			BOOL bret = CLIENT_StopUpgrade(m_lUpgradeID);
			if (!bret) 
			{
				MessageBox(ConvertString(MSG_UG_STOPFAILED));
			}
			m_lUpgradeID = 0;
			m_upgradeProgress.SetPos(0);
			GetDlgItem(IDC_UPPROGRESS)->SetWindowText(ConvertString(MSG_UP_PROGRESS));
		}
	}
	else
	{
		m_upgradeProgress.SetPos(npos);
		if (npos >= 100)
		{
			GetDlgItem(IDC_UPPROGRESS)->SetWindowText(ConvertString(MSG_TRANSFER_DONE));
		}
	}
}

BOOL CNetUpgrade::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	m_lUpgradeID = 0;
	m_bStopped = TRUE;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNetUpgrade::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnClose();
}

void CNetUpgrade::OnUpCancel() 
{
	if (m_lUpgradeID)
	{
		m_bStopped = TRUE;
		BOOL bret = CLIENT_StopUpgrade(m_lUpgradeID);
		if (!bret)
		{
			MessageBox(ConvertString(MSG_UG_STOPFAILED));
		//	EndDialog(0);
		}
		else
		{
			m_upgradeProgress.SetPos(0);
			m_lUpgradeID = 0;
			GetDlgItem(IDC_UPPROGRESS)->SetWindowText(ConvertString(MSG_UP_PROGRESS));
		}
	}
	else
	{
	
	}
}

LRESULT CNetUpgrade::OnSetProgress( WPARAM wp, LPARAM lp )
{
	SetProgressPos((int)lp);
	return 0;
}
