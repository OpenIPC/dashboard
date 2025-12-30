// SerialConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "SerialConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSerialConfig dialog


CSerialConfig::CSerialConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CSerialConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSerialConfig)
	m_bDialBack = FALSE;
	m_bEncrypt = FALSE;
	m_IPmask = _T("");
	m_localIP = _T("");
	m_phoneNo = _T("");
	m_remoteIP = _T("");
	m_username = _T("");
	m_userpsw = _T("");
	m_verifypsw = _T("");
	m_decodeAddress = 0;
	//}}AFX_DATA_INIT
}


void CSerialConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSerialConfig)
	DDX_Control(pDX, IDC_BOUD485SEL, m_baud485sel);
	DDX_Control(pDX, IDC_BOUD232SEL, m_baud232sel);
	DDX_Control(pDX, IDC_STOPBIT485SEL, m_stopbit485sel);
	DDX_Control(pDX, IDC_STOPBIT232SEL, m_stopbit232sel);
	DDX_Control(pDX, IDC_PPPMODESEL, m_PPPmodesel);
	DDX_Control(pDX, IDC_OPERATEMODESEL, m_operatemodesel);
	DDX_Control(pDX, IDC_FLOW485SEL, m_flow485sel);
	DDX_Control(pDX, IDC_FLOW232SEL, m_flow232sel);
	DDX_Control(pDX, IDC_DECODETYPESEL, m_decodetypesel);
	DDX_Control(pDX, IDC_DBMODESEL, m_dbmodesel);
	DDX_Control(pDX, IDC_DATABIT485SEL, m_databit485sel);
	DDX_Control(pDX, IDC_DATABIT232SEL, m_databit232sel);
	DDX_Control(pDX, IDC_COPYCHLSEL, m_copychlsel);
	DDX_Control(pDX, IDC_CHLNOSEL, m_chlnosel);
	DDX_Control(pDX, IDC_CHECKBIT485SEL, m_checkbit485sel);
	DDX_Control(pDX, IDC_CHECKBIT232SEL, m_checkbit232sel);
	DDX_Check(pDX, IDC_DIALBACK_CHECK, m_bDialBack);
	DDX_Check(pDX, IDC_ENCRYPT_CHECK, m_bEncrypt);
	DDX_Text(pDX, IDC_IPMASK, m_IPmask);
	DDX_Text(pDX, IDC_LOCALIP, m_localIP);
	DDX_Text(pDX, IDC_PHONENO, m_phoneNo);
	DDX_Text(pDX, IDC_REMOTEIP, m_remoteIP);
	DDX_Text(pDX, IDC_USERNAME, m_username);
	DDX_Text(pDX, IDC_USERPSW, m_userpsw);
	DDX_Text(pDX, IDC_VERIFYPSW, m_verifypsw);
	DDX_Text(pDX, IDC_DECODEIP, m_decodeAddress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSerialConfig, CDialog)
	//{{AFX_MSG_MAP(CSerialConfig)
	ON_BN_CLICKED(IDC_APPLY232, OnApply232)
	ON_CBN_SELCHANGE(IDC_OPERATEMODESEL, OnSelchangeOperatemodesel)
	ON_BN_CLICKED(IDC_APPLY485, OnApply485)
	ON_CBN_SELCHANGE(IDC_CHLNOSEL, OnSelchangeChlnosel)
	ON_BN_CLICKED(IDC_RESTORE232, OnRestore232)
	ON_BN_CLICKED(IDC_RESTORE485, OnRestore485)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSerialConfig message handlers

void CSerialConfig::SetDevice (DeviceNode *nDev)
{
	m_Device = nDev;
}

int GetBaudRateSel(int boudrate)
{
	int ret = 0;
	switch(boudrate) 
	{
	case 300:
		ret = 0;
		break;
	case 600:
		ret = 1;
		break;
	case 1200:
		ret = 2;
		break;
	case 2400:
		ret = 3;
		break;
	case 4800:
		ret = 4;
		break;
	case 9600:
		ret = 5;
		break;
	case 19200:
		ret = 6;
		break;
	case 38400:
		ret = 7;
		break;
	case 57600:
		ret = 8;
		break;
	case 76800:
		ret = 9;
		break;
	case 115200:
		ret = 10;
		break;
	default:
		break;
	}
	return ret;
}

int GetBaudRate(int boudratesel)
{
	int ret = 0;
	switch(boudratesel) 
	{
	case 0:
		ret = 300;
		break;
	case 1:
		ret = 600;
		break;
	case 2:
		ret = 1200;
		break;
	case 3:
		ret = 2400;
		break;
	case 4:
		ret = 4800;
		break;
	case 5:
		ret = 9600;
		break;
	case 6:
		ret = 19200;
		break;
	case 7:
		ret = 38400;
		break;
	case 8:
		ret = 57600;
		break;
	case 9:
		ret = 76800;
		break;
	case 10:
		ret = 115200;
		break;
	default:
		break;
	}
	return ret;
}

int GetDatabitSel(int databit)
{
	int ret = 0;
	switch(databit) 
	{
	case 5:
		ret = 0;
		break;
	case 6:
		ret = 1;
		break;
	case 7:
		ret = 2;
		break;
	case 8:
		ret = 3;
		break;
	default:
		break;
	}
	return ret;
}

int GetDatabit(int databitsel)
{
	int ret = 0;
	switch(databitsel) 
	{
	case 0:
		ret = 5;
		break;
	case 1:
		ret = 6;
		break;
	case 2:
		ret = 7;
		break;
	case 3:
		ret = 8;
		break;
	default:
		break;
	}
	return ret;
}

BOOL CSerialConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);

	//初始化界面
	m_checkbit232sel.InsertString(0, ConvertString(MSG_SERIALCFG_NONE));
	m_checkbit232sel.InsertString(1, ConvertString(MSG_SERIALCFG_ODD));
	m_checkbit232sel.InsertString(2, ConvertString(MSG_SERIALCFG_EVEN));
	
	m_flow232sel.InsertString(0, MSG_SERIALCFG_NOCTRL);
	m_flow232sel.InsertString(1, MSG_SERIALCFG_SCTRL);
	m_flow232sel.InsertString(2, MSG_SERIALCFG_HCTRL);

	m_operatemodesel.InsertString(0, MSG_SERIALCFG_PPPOE);
	m_operatemodesel.InsertString(1, MSG_SERIALCFG_CONSOLE);
	m_operatemodesel.InsertString(2, MSG_SERIALCFG_TRANSPARENT);

	m_PPPmodesel.InsertString(0, MSG_SERIALCFG_ACTIVE);	
	m_PPPmodesel.InsertString(1, MSG_SERIALCFG_PASSIVE);
	
	m_dbmodesel.InsertString(0, MSG_SERIALCFG_SPECIFIED);
	m_dbmodesel.InsertString(1, MSG_SERIALCFG_DEFAULT);

	m_checkbit485sel.InsertString(0, MSG_SERIALCFG_NONE);
	m_checkbit485sel.InsertString(1, MSG_SERIALCFG_ODD);
	m_checkbit485sel.InsertString(2, MSG_SERIALCFG_EVEN);

	m_flow485sel.InsertString(0, MSG_SERIALCFG_NOCTRL);
	m_flow485sel.InsertString(1, MSG_SERIALCFG_SCTRL);
	m_flow485sel.InsertString(2, MSG_SERIALCFG_HCTRL);


	CString sTemp;
	char cTemp[100];
	m_nChanSel = 0;
	m_nCopyChanNum = -1;
	m_copychlsel.ResetContent();
	m_chlnosel.ResetContent();	
	sTemp.Format(ConvertString(_T(NAME_SERIALCFG_ALLCHANNEL)));
	m_copychlsel.AddString(sTemp);
	
	bSetSerial232 = FALSE;
	bGet232Serial = FALSE;
	
	for(int i=0; i < m_Device->Info.byChanNum; i++)
	{
		bGet485Serial[i] = FALSE;
		bSetSerial485[i] = FALSE;
		sTemp.Format("%d", (i+1));
		sTemp = ConvertString(NAME_SERIALCFG_CHANNEL) + sTemp;
		m_chlnosel.AddString(sTemp);
		m_copychlsel.AddString(sTemp);
		m_nChanNum = i;
		//if(m_bConnectServer)
		//{
			if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_DECODERCFG, m_nChanNum,&decodercfg[i], sizeof(NET_DEV_DECODERCFG), &dwReturned, CONFIG_WAITTIME))
			{		
			//	sTemp.Format("获取通道%d 485串口参数失败!: DEV_GET_DECODERCFG = %d \n", m_nChanNum, NET_DVR_GetLastError());
				AfxMessageBox(ConvertString("485 error"));	
			}
			else
			{
				bGet485Serial[i] = TRUE;
			}
	//	}	
	}
	if(bGet485Serial[m_nChanSel])
	{
		m_nChanNum = m_nChanSel;
		m_baud485sel.SetCurSel(GetBaudRateSel(decodercfg[m_nChanSel].dwBaudRate));
		m_databit485sel.SetCurSel(GetDatabitSel(decodercfg[m_nChanSel].byDataBit));
		m_stopbit485sel.SetCurSel(decodercfg[m_nChanSel].byStopBit);
		m_checkbit485sel.SetCurSel(decodercfg[m_nChanSel].byParity);
		m_flow485sel.SetCurSel(decodercfg[m_nChanSel].byFlowcontrol);
		m_decodetypesel.SetCurSel(decodercfg[m_nChanSel].wDecoderType);
		m_decodeAddress = (BYTE)decodercfg[m_nChanSel].wDecoderAddress;
		GetDlgItem(IDC_APPLY232)->EnableWindow(TRUE);
		GetDlgItem(IDC_COPYOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_APPLY485)->EnableWindow(TRUE);	
	}	
//	if(m_bConnectServer)
//	{
		if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_RS232CFG, 0, &rs232cfg, sizeof(NET_DEV_RS232CFG), &dwReturned, CONFIG_WAITTIME))
		{
			//sTemp.Format("获取232串口参数失败!: NET_DVR_GET_RS232CFG = %d \n", NET_DVR_GetLastError());
			AfxMessageBox(ConvertString("232error"));
		}
		else
		{
			m_baud232sel.SetCurSel(GetBaudRateSel(rs232cfg.dwBaudRate));
			m_databit232sel.SetCurSel(GetDatabitSel(rs232cfg.byDataBit));
			m_stopbit232sel.SetCurSel(rs232cfg.byStopBit);
			m_checkbit232sel.SetCurSel(rs232cfg.byParity);
			m_flow232sel.SetCurSel(rs232cfg.byFlowcontrol);
			m_operatemodesel.SetCurSel(rs232cfg.dwWorkMode);
			if(rs232cfg.dwWorkMode == 0)
			{
				m_PPPmodesel.SetCurSel(rs232cfg.struPPPConfig.byPPPMode);
				m_dbmodesel.SetCurSel(rs232cfg.struPPPConfig.byRedialMode);
				m_bDialBack = rs232cfg.struPPPConfig.byRedial;
				m_bEncrypt = rs232cfg.struPPPConfig.byDataEncrypt;
				ZeroMemory(cTemp, 100);
				memcpy(cTemp, rs232cfg.struPPPConfig.sUsername, NAME_LEN);
				m_username.Format("%s", cTemp);
				ZeroMemory(cTemp, 100);
				memcpy(cTemp, rs232cfg.struPPPConfig.sPassword, PASSWD_LEN);
				m_userpsw.Format("%s", cTemp);
				ZeroMemory(cTemp, 100);
				memcpy(cTemp, rs232cfg.struPPPConfig.sPassword, PASSWD_LEN);
				m_verifypsw.Format("%s", cTemp);

				m_remoteIP = rs232cfg.struPPPConfig.sRemoteIP;
				m_localIP = rs232cfg.struPPPConfig.sLocalIP;
				m_IPmask = rs232cfg.struPPPConfig.sLocalIPMask;
				
				ZeroMemory(cTemp, 100);
				memcpy(cTemp, rs232cfg.struPPPConfig.sTelephoneNumber, PHONENUMBER_LEN);
				m_phoneNo.Format("%s", cTemp);
			}
			else if(rs232cfg.dwWorkMode == 1)
			{
				SwitchMode(FALSE);
			}
			else if(rs232cfg.dwWorkMode == 2)
			{
				SwitchMode(FALSE);
			}
			bGet232Serial = TRUE;
			GetDlgItem(IDC_APPLY232)->EnableWindow(TRUE);
		}
//	}
		
	m_chlnosel.SetCurSel(m_nChanSel);
	m_copychlsel.SetCurSel(m_nChanSel);
//	pSerialCFG = this;
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSerialConfig::OnApply232() 
{
	if (!UpdateData(TRUE))
	{
		return;
	}
	
	if(bGet232Serial)
	{		
		rs232cfg.dwSize = sizeof(NET_DEV_RS232CFG);
		rs232cfg.dwBaudRate = GetBaudRate(m_baud232sel.GetCurSel());
		rs232cfg.byDataBit = GetDatabit(m_databit232sel.GetCurSel());
		rs232cfg.byStopBit = m_stopbit232sel.GetCurSel();
		rs232cfg.byParity = m_checkbit232sel.GetCurSel();
		rs232cfg.byFlowcontrol = m_flow232sel.GetCurSel();
		rs232cfg.dwWorkMode = m_operatemodesel.GetCurSel();

		if(0 == m_operatemodesel.GetCurSel())
		{
			if(m_verifypsw != m_userpsw)
			{
				AfxMessageBox(MSG_SERIALCFG_PSWCHECKERROR);
				return ;
			}
			rs232cfg.struPPPConfig.byPPPMode = m_PPPmodesel.GetCurSel();
			rs232cfg.struPPPConfig.byRedialMode = m_dbmodesel.GetCurSel();
			rs232cfg.struPPPConfig.byRedial = (BYTE)m_bDialBack;
			rs232cfg.struPPPConfig.byDataEncrypt = (BYTE)m_bEncrypt;
			memcpy(rs232cfg.struPPPConfig.sUsername,m_username,NAME_LEN);
			memcpy(rs232cfg.struPPPConfig.sPassword, m_userpsw, PASSWD_LEN);
			memcpy(rs232cfg.struPPPConfig.sRemoteIP, m_remoteIP, 16);
			memcpy(rs232cfg.struPPPConfig.sLocalIP, m_localIP, 16);
			memcpy(rs232cfg.struPPPConfig.sLocalIPMask, m_IPmask, 16);
			memcpy(rs232cfg.struPPPConfig.sTelephoneNumber, m_phoneNo, PHONENUMBER_LEN);
		}
	}
//	bSetSerial232 = TRUE;
	if(!CLIENT_SetDevConfig(m_Device->LoginID, DEV_SET_RS232CFG, 0, &rs232cfg, sizeof(NET_DEV_RS232CFG), CONFIG_WAITTIME))
	{
		AfxMessageBox(ConvertString("rs232 config save failed!"));	
		return;
	}
	
}

void CSerialConfig::OnSelchangeOperatemodesel() 
{
	char cTemp[100];
	if (!UpdateData(TRUE))
	{
		return;
	}
	int OperateSel = m_operatemodesel.GetCurSel();
	if(OperateSel == 0)
	{
		SwitchMode(TRUE);
		m_PPPmodesel.SetCurSel(rs232cfg.struPPPConfig.byPPPMode);
		m_dbmodesel.SetCurSel(rs232cfg.struPPPConfig.byRedialMode);
		m_bDialBack = rs232cfg.struPPPConfig.byRedial;
		m_bEncrypt = rs232cfg.struPPPConfig.byDataEncrypt;
		ZeroMemory(cTemp, 100);
		memcpy(cTemp, rs232cfg.struPPPConfig.sUsername, NAME_LEN);
		m_username.Format("%s", cTemp);
		ZeroMemory(cTemp, 100);
		memcpy(cTemp, rs232cfg.struPPPConfig.sPassword, PASSWD_LEN);
		m_userpsw.Format("%s", cTemp);
		ZeroMemory(cTemp, 100);
		memcpy(cTemp, rs232cfg.struPPPConfig.sPassword, PASSWD_LEN);
		m_verifypsw.Format("%s", cTemp);
		m_remoteIP = rs232cfg.struPPPConfig.sRemoteIP;
		m_localIP = rs232cfg.struPPPConfig.sLocalIP;
		m_IPmask = rs232cfg.struPPPConfig.sLocalIPMask;
		ZeroMemory(cTemp, 100);
		memcpy(cTemp, rs232cfg.struPPPConfig.sTelephoneNumber, PHONENUMBER_LEN);
		m_phoneNo.Format("%s", cTemp);
	}
	else if(OperateSel == 1)
	{
		SwitchMode(FALSE);
	}
	else if(OperateSel == 2)
	{
		SwitchMode(FALSE);
	}
}

void CSerialConfig::SwitchMode(BOOL bEnable)
{
	GetDlgItem(IDC_PPPMODESEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_DBMODESEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_USERNAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_USERPSW)->EnableWindow(bEnable);
	GetDlgItem(IDC_VERIFYPSW)->EnableWindow(bEnable);
	GetDlgItem(IDC_REMOTEIP)->EnableWindow(bEnable);
	GetDlgItem(IDC_LOCALIP)->EnableWindow(bEnable);
	GetDlgItem(IDC_IPMASK)->EnableWindow(bEnable);
	GetDlgItem(IDC_PHONENO)->EnableWindow(bEnable);
	GetDlgItem(IDC_ENCRYPT_CHECK)->EnableWindow(bEnable);
	GetDlgItem(IDC_DIALBACK_CHECK)->EnableWindow(bEnable);
}

void CSerialConfig::OnApply485() 
{
	if (!UpdateData(TRUE))
	{
		return;
	}
	m_nChanSel = m_chlnosel.GetCurSel();
	if(bGet485Serial[m_nChanSel])
	{
		decodercfg[m_nChanSel].dwSize = sizeof(NET_DEV_DECODERCFG);
		decodercfg[m_nChanSel].dwBaudRate = GetBaudRate(m_baud485sel.GetCurSel());
		decodercfg[m_nChanSel].byDataBit = GetDatabit(m_databit485sel.GetCurSel());
		decodercfg[m_nChanSel].byStopBit = m_stopbit485sel.GetCurSel();
		decodercfg[m_nChanSel].byParity = m_checkbit485sel.GetCurSel();
		decodercfg[m_nChanSel].byFlowcontrol = m_flow485sel.GetCurSel();
		decodercfg[m_nChanSel].wDecoderType = m_decodetypesel.GetCurSel();
		decodercfg[m_nChanSel].wDecoderAddress = m_decodeAddress;
		bSetSerial485[m_nChanSel] = TRUE;
	}
	for(int i=0; i < int(m_Device->Info.byChanNum); i++)
	{
		if (bSetSerial485[m_nChanSel])
		{
			int m_nChanNum = i;
			if(!CLIENT_SetDevConfig(m_Device->LoginID, DEV_SET_DECODERCFG, m_nChanNum,&decodercfg[i], sizeof(NET_DEV_DECODERCFG), CONFIG_WAITTIME))
			{		
				AfxMessageBox(ConvertString("rs485 config save failed!"));		
				return;
			}
		}	
	}
}

void CSerialConfig::OnSelchangeChlnosel() 
{
	m_nChanSel = m_chlnosel.GetCurSel();
	m_nChanNum = m_nChanSel;
	m_baud485sel.SetCurSel(decodercfg[m_nChanSel].dwBaudRate);
	m_databit485sel.SetCurSel(decodercfg[m_nChanSel].byDataBit);
	m_stopbit485sel.SetCurSel(decodercfg[m_nChanSel].byStopBit);
	m_checkbit485sel.SetCurSel(decodercfg[m_nChanSel].byParity);
	m_flow485sel.SetCurSel(decodercfg[m_nChanSel].byFlowcontrol);
	m_decodetypesel.SetCurSel(decodercfg[m_nChanSel].wDecoderType);
	m_decodeAddress = (BYTE)decodercfg[m_nChanSel].wDecoderAddress;


	UpdateData(FALSE);
}

void CSerialConfig::OnRestore232() 
{
CString sTemp;
	char cTemp[100];
	bGet232Serial = FALSE;

	if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_RS232CFG, 0, &rs232cfg, sizeof(NET_DEV_RS232CFG), &dwReturned, CONFIG_WAITTIME))
	{
		AfxMessageBox(ConvertString("232error"));
	}
	else
	{
		m_baud232sel.SetCurSel(rs232cfg.dwBaudRate);
		m_databit232sel.SetCurSel(rs232cfg.byDataBit);
		m_stopbit232sel.SetCurSel(rs232cfg.byStopBit);
		m_checkbit232sel.SetCurSel(rs232cfg.byParity);
		m_flow232sel.SetCurSel(rs232cfg.byFlowcontrol);
		m_operatemodesel.SetCurSel(rs232cfg.dwWorkMode);
		if(rs232cfg.dwWorkMode == 0)
		{
			m_PPPmodesel.SetCurSel(rs232cfg.struPPPConfig.byPPPMode);
			m_dbmodesel.SetCurSel(rs232cfg.struPPPConfig.byRedialMode);
			m_bDialBack = rs232cfg.struPPPConfig.byRedial;
			m_bEncrypt = rs232cfg.struPPPConfig.byDataEncrypt;
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, rs232cfg.struPPPConfig.sUsername, NAME_LEN);
			m_username.Format("%s", cTemp);
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, rs232cfg.struPPPConfig.sPassword, PASSWD_LEN);
			m_userpsw.Format("%s", cTemp);
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, rs232cfg.struPPPConfig.sPassword, PASSWD_LEN);
			m_verifypsw.Format("%s", cTemp);

			m_remoteIP = rs232cfg.struPPPConfig.sRemoteIP;
			m_localIP = rs232cfg.struPPPConfig.sLocalIP;
			m_IPmask = rs232cfg.struPPPConfig.sLocalIPMask;
			
			ZeroMemory(cTemp, 100);
			memcpy(cTemp, rs232cfg.struPPPConfig.sTelephoneNumber, PHONENUMBER_LEN);
			m_phoneNo.Format("%s", cTemp);
		}
		else if(rs232cfg.dwWorkMode == 1)
		{
			SwitchMode(FALSE);
		}
		else if(rs232cfg.dwWorkMode == 2)
		{
			SwitchMode(FALSE);
		}
		bGet232Serial = TRUE;
		GetDlgItem(IDC_APPLY232)->EnableWindow(TRUE);
	}
	UpdateData(FALSE);
}

void CSerialConfig::OnRestore485() 
{
	CString sTemp;
//	char cTemp[100];
	m_nChanSel = 0;
	m_nCopyChanNum = -1;
	m_copychlsel.ResetContent();
	m_chlnosel.ResetContent();	
	sTemp.Format(_T(ConvertString(NAME_SERIALCFG_ALLCHANNEL)));
	m_copychlsel.AddString(sTemp);
	
	for(int i=0; i < m_Device->Info.byChanNum; i++)
	{
		bGet485Serial[i] = FALSE;
		bSetSerial485[i] = FALSE;
		sTemp.Format("%d", (i+1));
		sTemp = ConvertString(NAME_SERIALCFG_CHANNEL) + sTemp;
		m_chlnosel.AddString(sTemp);
		m_copychlsel.AddString(sTemp);
		m_nChanNum = i;
	
		if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_DECODERCFG, m_nChanNum,&decodercfg[i], sizeof(NET_DEV_DECODERCFG), &dwReturned, CONFIG_WAITTIME))
		{		
			AfxMessageBox(ConvertString("485 error"));	
		}
		else
		{
			bGet485Serial[i] = TRUE;
		}

	}
	if(bGet485Serial[m_nChanSel])
	{
		m_nChanNum = m_nChanSel;
		m_baud485sel.SetCurSel(decodercfg[m_nChanSel].dwBaudRate);
		m_databit485sel.SetCurSel(decodercfg[m_nChanSel].byDataBit);
		m_stopbit485sel.SetCurSel(decodercfg[m_nChanSel].byStopBit);
		m_checkbit485sel.SetCurSel(decodercfg[m_nChanSel].byParity);
		m_flow485sel.SetCurSel(decodercfg[m_nChanSel].byFlowcontrol);
		m_decodetypesel.SetCurSel(decodercfg[m_nChanSel].wDecoderType);
		m_decodeAddress = (BYTE)decodercfg[m_nChanSel].wDecoderAddress;
		GetDlgItem(IDC_APPLY232)->EnableWindow(TRUE);
		GetDlgItem(IDC_COPYOK)->EnableWindow(TRUE);
		GetDlgItem(IDC_APPLY485)->EnableWindow(TRUE);	
	}	
	
	m_chlnosel.SetCurSel(m_nChanSel);
	m_copychlsel.SetCurSel(m_nChanSel);
	UpdateData(FALSE);
}
