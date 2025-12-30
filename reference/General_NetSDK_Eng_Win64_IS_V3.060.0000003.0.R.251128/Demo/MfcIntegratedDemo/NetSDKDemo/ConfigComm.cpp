// ConfigComm.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigComm.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigComm dialog


CConfigComm::CConfigComm(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigComm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigComm)
	//}}AFX_DATA_INIT
	m_pDev = 0;
	memset(&m_comCFG, 0, sizeof(DHDEV_COMM_CFG));
	m_bInited = FALSE;

	m_232Index = -1;
	m_485Index = -1;
}
	

void CConfigComm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigComm)
	DDX_Control(pDX, IDC_COMBO_STOPBIT485, m_stopbit485sel);
	DDX_Control(pDX, IDC_COMBO_STOPBIT232, m_stopbit232sel);
	DDX_Control(pDX, IDC_COMBO_PROTOCOL485, m_protocol485sel);
	DDX_Control(pDX, IDC_COMBO_PARITY485, m_parity485sel);
	DDX_Control(pDX, IDC_COMBO_PARITY232, m_parity232sel);
	DDX_Control(pDX, IDC_COMBO_FUNCTION232, m_function232sel);
	DDX_Control(pDX, IDC_COMBO_485COM, m_485comsel);
	DDX_Control(pDX, IDC_COMBO_232COM, m_232comsel);
	DDX_Control(pDX, IDC_COMBO_DATABIT485, m_databit485sel);
	DDX_Control(pDX, IDC_COMBO_DATABIT232, m_databit232sel);
	DDX_Control(pDX, IDC_COMBO_BAUD485, m_baud485sel);
	DDX_Control(pDX, IDC_COMBO_BAUD232, m_baud232sel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigComm, CDialog)
	//{{AFX_MSG_MAP(CConfigComm)
	ON_CBN_SELCHANGE(IDC_COMBO_232COM, OnSelchangeCombo232com)
	ON_CBN_SELCHANGE(IDC_COMBO_485COM, OnSelchangeCombo485com)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_UNDO_ALL, OnUndoAll)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CConfigComm::GetConfig()
{
	if (!m_pDev || m_bInited)
	{
		return;
	}

	while(m_function232sel.GetCount() > 0)
	{
		m_function232sel.DeleteString(0);
	}
	while(m_protocol485sel.GetCount() > 0)
	{
		m_protocol485sel.DeleteString(0);
	}

	DWORD retlen = 0;
	BOOL bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_COMMCFG, 0,
								&m_comCFG, sizeof(DHDEV_COMM_CFG), &retlen, CONFIG_WAITTIME);
	int aaa = sizeof(DHDEV_COMM_CFG);
	if (!bRet || retlen != sizeof(DHDEV_COMM_CFG))
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		m_bInited = TRUE;
	}
	
	//show config information
	for (int i = 0; i < m_comCFG.dw232FuncNameNum; i++)
	{
		m_function232sel.InsertString(i, (char *)m_comCFG.s232FuncName[i]);
	}
	for (int j = 0; j < m_comCFG.dwDecProListNum; j++)
	{
		m_protocol485sel.InsertString(j, (char *)m_comCFG.DecProName[j]);
	}
	if (m_232comsel.GetCount() > 0)
	{
		m_232comsel.SetCurSel(0);
		OnSelchangeCombo232com();
	}
	if (m_485comsel.GetCount() > 0)
	{
		m_485comsel.SetCurSel(0);
		OnSelchangeCombo485com();
	}
}
void CConfigComm::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

BOOL CConfigComm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	
	if (!m_pDev)
	{
		return TRUE;
	}
	
	DWORD retlen = 0;
	DHDEV_SYSTEM_ATTR_CFG sysCFG = {0};
	BOOL bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_DEVICECFG, 0,
								&sysCFG, sizeof(DHDEV_SYSTEM_ATTR_CFG), &retlen, CONFIG_WAITTIME);
	if (!bRet || retlen != sizeof(DHDEV_SYSTEM_ATTR_CFG))
	{
		return TRUE;
	}

	CString strCom;
	for (int i = 0; i < sysCFG.byComIONum; i++)
	{
		strCom.Format(ConvertString("COM %d"), i+1);
		m_232comsel.InsertString(i, strCom);
	}
	CString strDecoder;
	for (int j = 0; j < m_pDev->Info.nChanNum; j++)
	{
		strDecoder.Format(ConvertString("Decoder %d"), j+1);
		m_485comsel.InsertString(j, strDecoder);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigComm::OnSelchangeCombo232com() 
{
	if (!m_bInited)
	{
		return;
	}
	//store current selection
	if (m_232Index >= 0)
	{
		m_comCFG.st232[m_232Index].byFunction = m_function232sel.GetCurSel();
		m_comCFG.st232[m_232Index].struComm.byDataBit = m_databit232sel.GetCurSel();
		m_comCFG.st232[m_232Index].struComm.byStopBit = m_stopbit232sel.GetCurSel();
		m_comCFG.st232[m_232Index].struComm.byBaudRate = m_baud232sel.GetCurSel();
		m_comCFG.st232[m_232Index].struComm.byParity = m_parity232sel.GetCurSel();
	}

	//show new ones
	int Idx232 = m_232comsel.GetCurSel();
	if (Idx232 < 0 || Idx232 >= DH_MAX_232_NUM) 
	{
		return;
	}
	
	m_function232sel.SetCurSel(m_comCFG.st232[Idx232].byFunction);
	m_databit232sel.SetCurSel(m_comCFG.st232[Idx232].struComm.byDataBit);
	m_stopbit232sel.SetCurSel(m_comCFG.st232[Idx232].struComm.byStopBit);
	m_baud232sel.SetCurSel(m_comCFG.st232[Idx232].struComm.byBaudRate);
	m_parity232sel.SetCurSel(m_comCFG.st232[Idx232].struComm.byParity);

	m_232Index = Idx232;
}

void CConfigComm::OnSelchangeCombo485com() 
{
	if (!m_bInited)
	{
		return;
	}
	if (m_485Index >= 0)
	{
		m_comCFG.stDecoder[m_485Index].wProtocol = m_protocol485sel.GetCurSel();
		m_comCFG.stDecoder[m_485Index].struComm.byDataBit = m_databit485sel.GetCurSel();
		m_comCFG.stDecoder[m_485Index].struComm.byStopBit = m_stopbit485sel.GetCurSel();
		m_comCFG.stDecoder[m_485Index].struComm.byBaudRate = m_baud485sel.GetCurSel();
		m_comCFG.stDecoder[m_485Index].struComm.byParity = m_parity485sel.GetCurSel();
		m_comCFG.stDecoder[m_485Index].wDecoderAddress = GetDlgItemInt(IDC_EDIT_ADDR485);
	}

	int Idx485 = m_485comsel.GetCurSel();
	if (Idx485 < 0 || Idx485 >= DH_MAX_DECODER_NUM) 
	{
		return;
	}
	
	m_protocol485sel.SetCurSel(m_comCFG.stDecoder[Idx485].wProtocol);
	m_databit485sel.SetCurSel(m_comCFG.stDecoder[Idx485].struComm.byDataBit);
	m_stopbit485sel.SetCurSel(m_comCFG.stDecoder[Idx485].struComm.byStopBit);
	m_baud485sel.SetCurSel(m_comCFG.stDecoder[Idx485].struComm.byBaudRate);
	m_parity485sel.SetCurSel(m_comCFG.stDecoder[Idx485].struComm.byParity);
	SetDlgItemInt(IDC_EDIT_ADDR485, m_comCFG.stDecoder[Idx485].wDecoderAddress);

	m_485Index = Idx485;
}

void CConfigComm::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	if (bShow)
	{
		GetConfig();
	}
}

void CConfigComm::OnUndoAll() 
{
	m_bInited = FALSE;
	m_232Index = -1;
	m_485Index = -1;
	GetConfig();
}

void CConfigComm::OnApply() 
{
	if (!m_bInited)
	{
		return;
	}

	int Idx232 = m_232comsel.GetCurSel();
	if (Idx232 < 0)
	{
		return;
	}
	m_comCFG.st232[Idx232].byFunction = m_function232sel.GetCurSel();
	m_comCFG.st232[Idx232].struComm.byDataBit = m_databit232sel.GetCurSel();
	m_comCFG.st232[Idx232].struComm.byStopBit = m_stopbit232sel.GetCurSel();
	m_comCFG.st232[Idx232].struComm.byBaudRate = m_baud232sel.GetCurSel();
	m_comCFG.st232[Idx232].struComm.byParity = m_parity232sel.GetCurSel();

	int Idx485 = m_485comsel.GetCurSel();
	if (Idx485 < 0)
	{
		return;
	}
	m_comCFG.stDecoder[Idx485].wProtocol = m_protocol485sel.GetCurSel();
	m_comCFG.stDecoder[Idx485].struComm.byDataBit = m_databit485sel.GetCurSel();
	m_comCFG.stDecoder[Idx485].struComm.byStopBit = m_stopbit485sel.GetCurSel();
	m_comCFG.stDecoder[Idx485].struComm.byBaudRate = m_baud485sel.GetCurSel();
	m_comCFG.stDecoder[Idx485].struComm.byParity = m_parity485sel.GetCurSel();
	m_comCFG.stDecoder[Idx485].wDecoderAddress = GetDlgItemInt(IDC_EDIT_ADDR485);

	BOOL bRet = CLIENT_SetDevConfig(m_pDev->LoginID, DH_DEV_COMMCFG, 0,
								&m_comCFG, 16*sizeof(DHDEV_COMM_CFG), CONFIG_WAITTIME);
	if (!bRet)
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		MessageBox(ConvertString(MSG_CONFIG_SUCCESS), "OK");
	}
}
