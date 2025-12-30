// DevInfo.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "DevInfo.h"

IMPLEMENT_DYNAMIC(DevInfo, CDialog)

DevInfo::DevInfo(CWnd* pParent /*=NULL*/)
	: CDialog(DevInfo::IDD, pParent)
{

}

DevInfo::~DevInfo()
{
}

void DevInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(DevInfo, CDialog)
	ON_BN_CLICKED(IDC_DEVICEINFO_BTN_GET, &DevInfo::OnBnClickedDeviceinfoBtnGet)
END_MESSAGE_MAP()


BOOL DevInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_DEVICEINFO_BTN_GET)->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void DevInfo::OnBnClickedDeviceinfoBtnGet()
{
	CString m_strCap = "";
	BOOL bret = Device::GetInstance().GetCapability(m_strCap);
	if (bret)
	{
		GetDlgItem(IDC_DLG_CAP_EDT_SHOW)->SetWindowText(m_strCap);
	}
	CString m_strVer = "";
	bret = Device::GetInstance().GetVersion(m_strVer);
	CString m_strMAC = "";
	bret = Device::GetInstance().GetMAC(m_strMAC);
	CString m_strMCUVer = "";
	bret = Device::GetInstance().GetMCUVer(m_strMCUVer);
	CString m_strV = m_strVer + m_strMAC/*+m_strMCUVer*/;
	GetDlgItem(IDC_VERSION_STATIC_INFO)->SetWindowText(m_strV);
}
