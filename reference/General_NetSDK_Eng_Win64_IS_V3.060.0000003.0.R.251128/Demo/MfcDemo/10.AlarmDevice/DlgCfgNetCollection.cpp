// DlgCfgNetCollection.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgNetCollection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetCollection dialog


CDlgCfgNetCollection::CDlgCfgNetCollection(CWnd* pParent /* = NULL */, LLONG lLoginID /* = NULL */)
	: CDialog(CDlgCfgNetCollection::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgNetCollection)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginID = lLoginID;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgNetCollection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgNetCollection)
	DDX_Control(pDX, IDC_NETCOLLECTION_IPADDR, m_ctrlIP);
	DDX_Control(pDX, IDC_NETCOLLECTION_CMB_CHN, m_cmbChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgNetCollection, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgNetCollection)
	ON_CBN_SELCHANGE(IDC_NETCOLLECTION_CMB_CHN, OnSelchangeNetcollectionCmbChn)
	ON_BN_CLICKED(IDC_NETCOLLECTION_BTN_GET, OnNetcollectionBtnGet)
	ON_BN_CLICKED(IDC_NETCOLLECTION_BTN_SET, OnNetcollectionBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetCollection private method

void CDlgCfgNetCollection::InitDlg()
{
	m_cmbChannel.ResetContent();
	for (int i = 0; i < 256; i++)
	{
		CString csItem;
		csItem.Format("%s %03d", ConvertString("Channel", DLG_CFG_ALARM), i + 1);
		m_cmbChannel.InsertString(-1, csItem);
	}
	m_cmbChannel.SetCurSel(0);
}

BOOL CDlgCfgNetCollection::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = m_cmbChannel.GetCurSel() == -1 ? 0 : m_cmbChannel.GetCurSel();
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_NET_COLLECTION, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_NET_COLLECTION, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse NetCollection error..."), DLG_CFG_NETCOLLECTION), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig NetCollection error:", DLG_CFG_NETCOLLECTION), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgNetCollection::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};    
    BOOL bRet = CLIENT_PacketData(CFG_CMD_NET_COLLECTION, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet NetCollection error..."), DLG_CFG_NETCOLLECTION), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
		int nSelChn = m_cmbChannel.GetCurSel() == -1 ? 0 : m_cmbChannel.GetCurSel();
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_NET_COLLECTION, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig NetCollection failed:", DLG_CFG_NETCOLLECTION), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig NetCollection ok!"), DLG_CFG_NETCOLLECTION), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgNetCollection::showInfo()
{
	// ip
	m_ctrlIP.SetWindowText(m_stuInfo.szIp);

	// port
	SetDlgItemInt(IDC_NETCOLLECTION_EDT_PORT, m_stuInfo.nPort, TRUE);

    return TRUE;
}

BOOL CDlgCfgNetCollection::getInfo()
{
	// ip
	m_ctrlIP.GetWindowText(m_stuInfo.szIp, sizeof(m_stuInfo.szIp) - 1);

	// port
	m_stuInfo.nPort = GetDlgItemInt(IDC_NETCOLLECTION_EDT_PORT);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetCollection message handlers

BOOL CDlgCfgNetCollection::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_NETCOLLECTION);
	// TODO: Add extra initialization here
	InitDlg();
	if (GetConfigFromDevice())
	{
		showInfo();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgNetCollection::OnSelchangeNetcollectionCmbChn() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgNetCollection::OnNetcollectionBtnGet() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgNetCollection::OnNetcollectionBtnSet() 
{
	// TODO: Add your control notification handler code here
	getInfo();
    SetConfigToDevice();
}
