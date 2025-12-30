// DeviceInfo.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveLoginDemo.h"
#include "DeviceInfo.h"
#include "PluginLogging.h"

#ifdef _DEBUG
#define NEW DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#else
#define NEW new(std::nothrow)
#endif



/////////////////////////////////////////////////////////////////////////////
// CDeviceInfo dialog


CDeviceInfo::CDeviceInfo(CWnd* pParent /*=NULL*/,LLONG lLoginID)
	: CDialog(CDeviceInfo::IDD, pParent)
	, m_bRadio(TRUE)
{
	//{{AFX_DATA_INIT(CDeviceInfo)
	m_strDeviceName = _T("");
	m_strID = _T("");
	m_strPsw = _T("");
	m_nPort = 0;
	m_strUserName = _T("");
	m_nServerPort = 0;
	m_strServerIP = _T("");
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginID;
    m_bStatus = FALSE;
    memset(&m_stuDvrIpInfo,0,sizeof(m_stuDvrIpInfo));
	m_strIP = _T("");
}


void CDeviceInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviceInfo)
	DDX_Text(pDX, IDC_EDIT_DEVNAME, m_strDeviceName);
	DDX_Text(pDX, IDC_EDIT_ID, m_strID);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPsw);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_EDIT_SERVERPORT, m_nServerPort);
	DDX_Text(pDX, IDC_EDIT_SERVERIP, m_strServerIP);
	DDX_Text(pDX, IDC_EDIT_IP, m_strIP);
	//}}AFX_DATA_MAP
	DDX_Radio(pDX, IDC_RADIO_IP, m_bRadio);
	DDV_MinMaxInt(pDX, m_nPort,0, 65535);
}


BEGIN_MESSAGE_MAP(CDeviceInfo, CDialog)
	//{{AFX_MSG_MAP(CDeviceInfo)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CDeviceInfo::OnBnClickedOk)
ON_BN_CLICKED(IDC_RADIO_AUTO, &CDeviceInfo::OnBnClickedRadio)
ON_BN_CLICKED(IDC_RADIO_IP, &CDeviceInfo::OnBnClickedRadio)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeviceInfo message handlers

BOOL CDeviceInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this,DLG_DEVICEINFO);

    if (m_bStatus)
    {
        GetDlgItem(IDC_EDIT_ID)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_PORT)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_IP)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_PASSWORD)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_DEVNAME)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_USERNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_AUTO)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_IP)->EnableWindow(FALSE);
    }
	else
	{
		if (m_bRadio)
		{
			GetDlgItem(IDC_EDIT_PORT)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_IP)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_EDIT_PORT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_IP)->EnableWindow(TRUE);
		}
		GetDlgItem(IDC_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SERVERIP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SERVERPORT)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeviceInfo::OnBtnGet() 
{
	char* szBuf = NEW char[40*1024];
    if (NULL == szBuf)
    {
        PRINT_ERROR("malloc szBuf failed!");
        return;
    }

    int nError = 0;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID,CFG_CMD_DVRIP,-1,szBuf,40*1024,&nError,5000);
    if (bRet)
    {
        bRet = CLIENT_ParseData(CFG_CMD_DVRIP,szBuf,&m_stuDvrIpInfo,sizeof(m_stuDvrIpInfo),NULL);
        if (bRet)
        {
            m_nServerPort = m_stuDvrIpInfo.stuRegisters[0].stuServers[0].nPort;
            m_strServerIP = m_stuDvrIpInfo.stuRegisters[0].stuServers[0].szAddress;
        }
    }
    else
    {
        DWORD h = CLIENT_GetLastError();
        CString str;
		str.Format(ConvertString("Get Config Fail!ErrorCode:%x",DLG_DEVICEINFO),h);
/*        str.Format("获取主动注册配置失败!错误码:%x",h);*/
        MessageBox(str);
    }
    UpdateData(FALSE);

    delete []szBuf;
    szBuf = NULL;
}

void CDeviceInfo::OnBtnSet() 
{
    UpdateData();

    char* szBuf = NEW char[40*1024];
    if (NULL == szBuf)
    {
        PRINT_ERROR("malloc szBuf failed!");
        return;
    }

    int nError = 0;
    int nReStart = 0;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID,CFG_CMD_DVRIP,-1,szBuf,40*1024,&nError,5000);
    if (bRet)
    {
        bRet = CLIENT_ParseData(CFG_CMD_DVRIP,szBuf,&m_stuDvrIpInfo,sizeof(m_stuDvrIpInfo),NULL);
        if (bRet)
        {
            m_stuDvrIpInfo.stuRegisters[0].stuServers[0].nPort = m_nServerPort;
            memcpy(&m_stuDvrIpInfo.stuRegisters[0].stuServers[0].szAddress,m_strServerIP,256);
            bRet = CLIENT_PacketData(CFG_CMD_DVRIP,(char*)&m_stuDvrIpInfo,sizeof(m_stuDvrIpInfo),szBuf,1024*40);
            if (bRet)
            {
                bRet = CLIENT_SetNewDevConfig(m_lLoginID,CFG_CMD_DVRIP, -1,szBuf,1024*40,&nError,&nReStart,5000);
				if (bRet)
				{
					CString str;
					str.Format(ConvertString("Set Config Success!",DLG_DEVICEINFO));
/*					str.Format("设置主动注册配置成功!");*/
					MessageBox(str);
				}
            }
        }
    }
    if (!bRet)
    {
        DWORD h = CLIENT_GetLastError();
        CString str;
		str.Format(ConvertString("Set Config Fail!ErrorCode:%x",DLG_DEVICEINFO),h);
/*        str.Format("设置主动注册配置失败!错误码:%x",h);*/
        MessageBox(str);
    }

    delete []szBuf;
    szBuf = NULL;
}

void CDeviceInfo::OnBnClickedOk()
{
	BOOL bValue = UpdateData();
	if(bValue)
	{
		m_strIP.Replace(_T(" "),_T(""));
		m_strID.Replace(_T(" "),_T(""));
		m_strDeviceName.Replace(_T(" "),_T(""));

		if (!m_bRadio)
		{
			if (m_strIP.IsEmpty() || NULL == m_nPort || m_strID.IsEmpty() || m_strDeviceName.IsEmpty() || m_strUserName.IsEmpty())
			{
				MessageBox(ConvertString("DeviceInfo is not right,Add Fail!",DLG_DEVICEINFO), ConvertString("Prompt"));
/*				MessageBox("设备信息不正确，添加失败！");*/
				return;
			}
		}
		else
		{
			if(m_strID.IsEmpty() || m_strDeviceName.IsEmpty() || m_strUserName.IsEmpty())
			{
				MessageBox(ConvertString("DeviceInfo is not right,Add Fail!",DLG_DEVICEINFO), ConvertString("Prompt"));
/*				MessageBox("设备信息不正确，添加失败！");*/
				return;
			}
		}
		UpdateData(FALSE);
		OnOK();
	}
}


void CDeviceInfo::OnBnClickedRadio()
{
	UpdateData(TRUE);
	if (m_bRadio)
	{
		GetDlgItem(IDC_EDIT_PORT)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_IP)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_PORT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_IP)->EnableWindow(TRUE);
	}
  }