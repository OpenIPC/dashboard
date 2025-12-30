// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DemoMonitorWall.h"
#include "LoginDlg.h"
#include "Device.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	DEVICE_HISTORY	"devices.dat"
#define DEV_HIS_FILE	MODULE_DIR + DEVICE_HISTORY

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog


CLoginDlg::CLoginDlg(UINT nLoginType/* = LOGIN_DECODER*/, CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDlg::IDD, pParent)
	, m_strAddr(_T(""))
	, m_nPort(37777)
	, m_strUser(_T("admin"))
	, m_strPwd(_T("admin"))
	, m_lLoginID(0)
	, m_nLoginType(nLoginType)
{
	//{{AFX_DATA_INIT(CLoginDlg)
	m_strDevClass = _T("");
	m_strDevType = _T("");
    
    memset(&m_stuDevInfo, 0, sizeof(NET_DEVICEINFO_Ex));
    
	//}}AFX_DATA_INIT
	memset(&m_aLoginDevs, 0, sizeof(m_aLoginDevs));	
}


void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginDlg)
	DDX_Control(pDX, IDC_COMBO_ADDRESS, m_cbAddr);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUser);
	DDV_MaxChars(pDX, m_strUser, 64);
	DDX_Text(pDX, IDC_EDIT_DEV_CLASS, m_strDevClass);
	DDX_Text(pDX, IDC_EDIT_DEV_TYPE, m_strDevType);
	DDX_Text(pDX, IDC_EDIT_PWD, m_strPwd);
	DDV_MaxChars(pDX, m_strPwd, 64);
	//}}AFX_DATA_MAP
	//DDX_Text(pDX, IDC_EDIT_ADDRESS, m_strAddr);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialog)
	//{{AFX_MSG_MAP(CLoginDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_ADDRESS, OnSelchangeComboAddress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg message handlers

BOOL CLoginDlg::OnInitDialog()
{	
	UpdateData(FALSE);

	SetWindowText(m_nLoginType == LOGIN_DECODER ? "Login" : "Add device");
	GetDlgItem(IDC_EDIT_DEV_CLASS)->EnableWindow(m_nLoginType != LOGIN_DECODER);
	GetDlgItem(IDC_EDIT_DEV_TYPE)->EnableWindow(m_nLoginType != LOGIN_DECODER);

	LoadLoginDeviceHistory();
	m_cbAddr.ResetContent();
	for (int i = 0; i < MAX_LOGIN_DEVICE; ++i)
	{
		if (*m_aLoginDevs[i].szAddress)
		{
			m_cbAddr.AddString(m_aLoginDevs[i].szAddress);
		}
	}
	m_cbAddr.SetCurSel(0);
	OnSelchangeComboAddress();
	
	LMIU(this);

	return TRUE;
}

void CLoginDlg::OnOK() 
{
	UpdateData();
	
	m_cbAddr.GetWindowText(m_strAddr);

	if (m_strAddr.IsEmpty())
	{
		MsgBox(_T("Invalid Address."));
		return;
	}
	if (m_nPort <= 0)
	{
		MsgBox(_T("Invalid Port."));
		return;
	}
	if (m_strUser.IsEmpty())
	{
		MsgBox(_T("Invalid User Name."));
		return;
	}
	
	int nError = 0;
	NET_DEVICEINFO_Ex stNetDevInfo;
    memset(&stNetDevInfo, 0, sizeof(NET_DEVICEINFO_Ex));
	CString strTemp;
	
	LLONG lRet = CLIENT_LoginEx2((LPTSTR)(LPCTSTR)m_strAddr,  m_nPort, (LPTSTR)(LPCTSTR)m_strUser, 
                                  (LPTSTR)(LPCTSTR)m_strPwd, EM_LOGIN_SPEC_CAP_TCP, NULL, 
                                  &stNetDevInfo, &nError);
	
	if (lRet == 0)
	{
		MsgBox2(_T("Login Failed. Code: %d, %s."), nError, LMIC(LoginErrorString(nError)));
		return;
	}
	
	m_lLoginID = lRet;
	m_stuDevInfo = stNetDevInfo;

	// 
	LoginDeviceInfo stuLoginInfo = { 0 };
	strncpy(stuLoginInfo.szAddress, m_strAddr.GetBuffer(), sizeof(stuLoginInfo.szAddress) - 1);
	stuLoginInfo.nPort = m_nPort;
	strncpy(stuLoginInfo.szUser, m_strUser.GetBuffer(), sizeof(stuLoginInfo.szUser) - 1);
	strncpy(stuLoginInfo.szPassword, m_strPwd.GetBuffer(), sizeof(stuLoginInfo.szPassword) - 1);
	strncpy(stuLoginInfo.szDevClass, m_strDevClass.GetBuffer(), sizeof(stuLoginInfo.szDevClass) - 1);
	strncpy(stuLoginInfo.szDevType, m_strDevType.GetBuffer(), sizeof(stuLoginInfo.szDevType) - 1);

	m_strAddr.ReleaseBuffer();
	m_strUser.ReleaseBuffer();
	m_strPwd.ReleaseBuffer();
	m_strDevClass.ReleaseBuffer();
	m_strDevType.ReleaseBuffer();

	int nIndex = -1;
	for (int i = 0; i < MAX_LOGIN_DEVICE; ++i)
	{
		//if (memcmp(&stuLoginInfo, &m_aLoginDevs[i], sizeof(LoginDeviceInfo)) == 0)
		if (strcmp(stuLoginInfo.szAddress, m_aLoginDevs[i].szAddress) == 0)
		{
			nIndex = i;
			break;
		}
	}
	if (nIndex >= 0)
	{
		memmove(&m_aLoginDevs[1], &m_aLoginDevs[0], nIndex * sizeof(LoginDeviceInfo));
		memcpy(m_aLoginDevs, &stuLoginInfo, sizeof(LoginDeviceInfo));
	}
	else
	{
		memmove(&m_aLoginDevs[1], &m_aLoginDevs[0], sizeof(LoginDeviceInfo) * (MAX_LOGIN_DEVICE - 1));
		memcpy(m_aLoginDevs, &stuLoginInfo, sizeof(LoginDeviceInfo));
	}

	SaveLoginDeviceHistory();
	
	CDialog::OnOK();
}

void CLoginDlg::LoadLoginDeviceHistory()
{
	CFile file;
	if (file.Open(DEV_HIS_FILE, CFile::modeRead | CFile::typeBinary))
	{
		if (file.GetLength() == sizeof(m_aLoginDevs))
		{
			file.Read(&m_aLoginDevs, sizeof(m_aLoginDevs));
		}
	}
}

void CLoginDlg::SaveLoginDeviceHistory()
{	
	CFile file;
	if (file.Open(DEV_HIS_FILE, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
	{
		file.Write(&m_aLoginDevs, sizeof(m_aLoginDevs));
	}
}

void CLoginDlg::OnSelchangeComboAddress() 
{
	CString strAddr;
	int nIndex = m_cbAddr.GetCurSel();
	if (nIndex >= 0)
	{
		m_cbAddr.GetLBText(nIndex, strAddr);

		if (!strAddr.IsEmpty())
		{
			for (int i = 0; i < MAX_LOGIN_DEVICE; ++i)
			{
				if (strAddr == m_aLoginDevs[i].szAddress)
				{
					SetDlgItemInt(IDC_EDIT_PORT, m_aLoginDevs[i].nPort);
					SetDlgItemText(IDC_EDIT_USER, m_aLoginDevs[i].szUser);
					SetDlgItemText(IDC_EDIT_PWD, m_aLoginDevs[i].szPassword);
					SetDlgItemText(IDC_EDIT_DEV_CLASS, m_aLoginDevs[i].szDevClass);
					SetDlgItemText(IDC_EDIT_DEV_TYPE, m_aLoginDevs[i].szDevType);
					return;
				}
			}
		}		
	}	
}

LPCTSTR CLoginDlg::DCID() const
{
	static struct
	{
		BOOL	bDecoder;
		UINT	nType;
		LPCTSTR	lpszType;
	} types[] =
	{
		{ TRUE, NET_MATRIX_SERIAL, CDecoderMatrix::DCID() }, 
		{ FALSE, NET_MATRIX_SERIAL, CEncoderMatrix::DCID() }, 
		{ TRUE, NET_NVD_SERIAL, CDecoderNVD::DCID() }, 
		{ FALSE, NET_NVD_SERIAL, CEncoderNVD::DCID() }, 
		{ TRUE, NET_NVR_SERIAL, CDecoderNVR::DCID() }, 
		{ FALSE, NET_NVR_SERIAL, CEncoderNVR::DCID() }, 
		{ TRUE, NET_DSCON, CDecoderDSCON::DCID() }, 
		{ FALSE, NET_DSCON, CEncoderDSCON::DCID() }, 
	};

	for (int i = 0; i < _countof(types); ++i)
	{
		if (m_nLoginType == LOGIN_DECODER)
		{
			if (types[i].bDecoder && types[i].nType == m_stuDevInfo.nDVRType)
			{
				return types[i].lpszType;
			}
		}
		else
		{
			if (!types[i].bDecoder && types[i].nType == m_stuDevInfo.nDVRType)
			{
				return types[i].lpszType;
			}
		}
	}
	return NULL;
}

LPCTSTR CLoginDlg::LoginErrorString( int nError )
{
	static struct 
	{
		int			nCode;		// ´íÎóÂë
		const char* pszMsg;		// ´íÎóÐÅÏ¢
	} s_arErrMsg[] = 
	{
		{ 1, "error password" }, 
		{ 2, "error account" }, 
		{ 3, "wait response time out" }, 
		{ 4, "account is already login" }, 
		{ 5, "account is locked" }, 
		{ 6, "account is in block list" }, 
		{ 7, "system error" }, 
		{ 8, "subconnect error" }, 
		{ 9, "main connect error" }, 
		{ 10, "max connect count" }, 
		{ 11, "protocol v3 supported only" }, 
		{ 12, "ukey lost" }, 
		{ 13, "no authorized client ip" }, 
		{ 255, "copyright verify failed" },
	};
	const char* pszMsg = NULL;
	if (nError > 0)
	{
		for (int i = 0; i < _countof(s_arErrMsg); ++i)
		{
			if (s_arErrMsg[i].nCode == nError)
			{
				pszMsg = s_arErrMsg[i].pszMsg;
				break;
			}
		}
	}
	return pszMsg;
}
