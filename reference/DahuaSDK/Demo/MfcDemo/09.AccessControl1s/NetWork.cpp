// NetWork.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "NetWork.h"

IMPLEMENT_DYNAMIC(NetWork, CDialog)

NetWork::NetWork(CWnd* pParent /*=NULL*/)
	: CDialog(NetWork::IDD, pParent)
{
	memset(&m_stuInfo,0,sizeof(m_stuInfo));
}

NetWork::~NetWork()
{
}

void NetWork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NETWORK_IP, m_ctlIp);
	DDX_Control(pDX, IDC_NETWORK_MASK, m_ctlMask);
	DDX_Control(pDX, IDC_NETWORK_GATEWAY, m_ctlGateway);
	DDX_Control(pDX, IDC_REGISTER_ADDRESS, m_ctlAddress);
	DDX_Control(pDX, IDC_REGISTER_ENABLE, m_btnRegisterEnable);
}


BEGIN_MESSAGE_MAP(NetWork, CDialog)
	ON_BN_CLICKED(IDC_NETWORK_BTN_SET, &NetWork::OnBnClickedNetworkBtnSet)
	ON_BN_CLICKED(IDC_NETWORK_BTN_GET, &NetWork::OnBnClickedNetworkBtnGet)
	ON_BN_CLICKED(IDC_REGISTER_BTN_SET, &NetWork::OnBnClickedRegisterBtnSet)
	ON_BN_CLICKED(IDC_REGISTER_BTN_GET, &NetWork::OnBnClickedRegisterBtnGet)
END_MESSAGE_MAP()


BOOL NetWork::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	m_ctlIp.SetWindowText("0,0,0,0");
	m_ctlMask.SetWindowText("0,0,0,0");
	m_ctlGateway.SetWindowText("0,0,0,0");

	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedNetworkBtnGet();
		OnBnClickedRegisterBtnGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_NETWORK_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_NETWORK_BTN_GET)->EnableWindow(FALSE);
		
		GetDlgItem(IDC_REGISTER_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_REGISTER_BTN_GET)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void NetWork::OnBnClickedNetworkBtnSet()
{
	CString csIp, csMask, csGateway;
	m_ctlIp.GetWindowText(csIp);
	m_ctlMask.GetWindowText(csMask);
	m_ctlGateway.GetWindowText(csGateway);
	BOOL bret = Device::GetInstance().SetNetWork(csIp, csMask, csGateway);
	if (!bret)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Set network failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void NetWork::OnBnClickedNetworkBtnGet()
{
	CString csIp = "";
	CString csMask = "";
	CString csGateway = "";
	BOOL bret = Device::GetInstance().GetNetWork(csIp, csMask, csGateway);
	if (bret)
	{
		m_ctlIp.SetWindowText(csIp);
		m_ctlMask.SetWindowText(csMask);
		m_ctlGateway.SetWindowText(csGateway);
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Get network failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void NetWork::OnBnClickedRegisterBtnSet()
{
	m_stuInfo.stuRegisters[0].bEnable = m_btnRegisterEnable.GetCheck();
	GetDlgItemText(IDC_REGISTER_DEVICEID,m_stuInfo.stuRegisters[0].szDeviceID, sizeof(m_stuInfo.stuRegisters[0].szDeviceID));
	m_stuInfo.stuRegisters[0].stuServers[0].nPort = GetDlgItemInt(IDC_REGISTER_PORT,NULL, FALSE);
	CString strTemp;
	m_ctlAddress.GetWindowText(strTemp);
	strncpy(m_stuInfo.stuRegisters[0].stuServers[0].szAddress, strTemp.GetBuffer(0), sizeof(m_stuInfo.stuRegisters[0].stuServers[0].szAddress) - 1);
	BOOL bret = Device::GetInstance().SetRegisterCFG(&m_stuInfo);
	if (!bret)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Set Register failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void NetWork::OnBnClickedRegisterBtnGet()
{
	CFG_DVRIP_INFO stuInfoTemp;
	memset(&stuInfoTemp, 0, sizeof(stuInfoTemp));

	BOOL bret = Device::GetInstance().GetRegisterCFG(stuInfoTemp);
	if (bret)
	{
		m_stuInfo = stuInfoTemp;
		m_btnRegisterEnable.SetCheck(stuInfoTemp.stuRegisters[0].bEnable);
		SetDlgItemText(IDC_REGISTER_DEVICEID,stuInfoTemp.stuRegisters[0].szDeviceID);
		SetDlgItemInt(IDC_REGISTER_PORT,stuInfoTemp.stuRegisters[0].stuServers[0].nPort);
		m_ctlAddress.SetWindowText(stuInfoTemp.stuRegisters[0].stuServers[0].szAddress);
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Get Register failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}
