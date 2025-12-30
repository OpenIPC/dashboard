// DevUpgrade.cpp
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "DevUpgrade.h"


void CALLBACK UpgradeCallBack(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize, LDWORD dwUser)
{
	DevUpgrade* pThis = (DevUpgrade*)dwUser;
	if (NULL == pThis)
	{
		return;
	}
	pThis->UpgradeState(lLoginID, lUpgradechannel, nTotalSize, nSendSize);
}

IMPLEMENT_DYNAMIC(DevUpgrade, CDialog)

DevUpgrade::DevUpgrade(CWnd* pParent /*=NULL*/)
	: CDialog(DevUpgrade::IDD, pParent)
{

}

DevUpgrade::~DevUpgrade()
{

}

void DevUpgrade::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_UPGRADE_LIST_INFO, m_lsInfo);
	DDX_Control(pDX, IDC_UPGRADE_PROGRESS, m_ctrlProgress);
}

BEGIN_MESSAGE_MAP(DevUpgrade, CDialog)
	ON_BN_CLICKED(IDC_DEVICEUPGRADE_BTN_START, &DevUpgrade::OnBnClickedDeviceupgradeBtnStart)
	ON_BN_CLICKED(IDC_DEVICEUPGRADE_BTN_STOP, &DevUpgrade::OnBnClickedDeviceupgradeBtnStop)
	ON_BN_CLICKED(IDC_DEVICEUPGRADE_BTN_OPENPATH, &DevUpgrade::OnBnClickedDeviceupgradeBtnOpenpath)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL DevUpgrade::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_DEVICEUPGRADE_BTN_OPENPATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEVICEUPGRADE_BTN_START)->EnableWindow(FALSE);
		GetDlgItem(IDC_DEVICEUPGRADE_BTN_STOP)->EnableWindow(FALSE);
	}
	m_ctrlProgress.SetRange(0, 100);
	m_ctrlProgress.SetPos(0);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void DevUpgrade::OnBnClickedDeviceupgradeBtnStart()
{
	OnBnClickedDeviceupgradeBtnStop();
	CString strFilePath;
	GetDlgItemText(IDC_UPGRADE_EDT_PATH, strFilePath);
	if (strFilePath.IsEmpty())
	{
		MessageBox(ConvertString("please choose a upgrade packet file."), ConvertString("Prompt"));
		return;
	}

	m_ctrlProgress.SetPos(0);

	BOOL bret = Device::GetInstance().StartUpgrade(strFilePath.GetBuffer(0), UpgradeCallBack, (LLONG)this);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Upgrade failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void DevUpgrade::OnBnClickedDeviceupgradeBtnStop()
{
	BOOL bret = Device::GetInstance().StopUpgrade();
}

void DevUpgrade::OnBnClickedDeviceupgradeBtnOpenpath()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_NOCHANGEDIR, _T("All Types (*.*)|*.*||"), this);

	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_UPGRADE_EDT_PATH, dlg.GetPathName());
	}	
}

void DevUpgrade::UpgradeState(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize)
{
	if (lLoginID && lUpgradechannel)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		int nCount = m_lsInfo.GetCount();
		CString csInfo;

		CString csProgress = ConvertString("upgrade progress");
		CString csSucceed = ConvertString("upgrade succeed");
		CString csFailed = ConvertString("upgrade failed");

		if (nTotalSize == 0)
		{
			if (nSendSize == -1)
			{
				csInfo.Format("%02d:%02d:%02d.%03d %s: %s", 
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, csProgress, csSucceed);
				m_lsInfo.InsertString(nCount, csInfo);
				m_lsInfo.SetCurSel(nCount);
			} 
			else if (nSendSize == -2)
			{
				csInfo.Format("%02d:%02d:%02d.%03d %s: %s", 
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, csProgress, csFailed);
				m_lsInfo.InsertString(nCount, csInfo);
				m_lsInfo.SetCurSel(nCount);
			}
			return;
		}
		else if (nTotalSize > 0)
		{
			int dProgress = (double)100.00*(double)nSendSize / (double)nTotalSize;
			if (dProgress <= 0)
			{
				dProgress = 0;
			}
			if (dProgress > 99)
			{
				dProgress = 100;
			}
			m_ctrlProgress.SetPos((int)dProgress);

			csInfo.Format("%02d:%02d:%02d.%03d %s: %d %d/%d", 
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, csProgress,
				dProgress, nSendSize, nTotalSize);
			m_lsInfo.InsertString(nCount, csInfo);
			m_lsInfo.SetCurSel(nCount);
		}
	}
}

void DevUpgrade::OnDestroy()
{
	CDialog::OnDestroy();
	BOOL bret = Device::GetInstance().StopUpgrade();
}
