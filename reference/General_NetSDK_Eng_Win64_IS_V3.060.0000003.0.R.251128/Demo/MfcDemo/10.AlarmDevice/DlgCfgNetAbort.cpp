// DlgCfgNetAbort.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgNetAbort.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetAbort dialog


CDlgCfgNetAbort::CDlgCfgNetAbort(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgNetAbort::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgNetAbort)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(CFG_NETABORT_INFO));
}


void CDlgCfgNetAbort::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgNetAbort)
	DDX_Control(pDX, IDC_CFGNETABORT_CHK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgNetAbort, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgNetAbort)
	ON_BN_CLICKED(IDC_CFGNETABORT_BTN_EVENTHANDLER, OnCfgnetabortBtnEventhandler)
	ON_BN_CLICKED(IDC_CFGNETABORT_BTN_GET, OnCfgnetabortBtnGet)
	ON_BN_CLICKED(IDC_CFGNETABORT_BTN_SET, OnCfgnetabortBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetAbort private method

BOOL CDlgCfgNetAbort::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = -1;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_NETABORT, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_NETABORT, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse NetAbort error..."), DLG_CFG_NETABORT), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig NetAbort error:", DLG_CFG_NETABORT), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgNetAbort::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};    
    BOOL bRet = CLIENT_PacketData(CFG_CMD_NETABORT, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet NetAbort error..."), DLG_CFG_NETABORT), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_NETABORT, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig NetAbort failed:", DLG_CFG_NETABORT), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig NetAbort ok!"), DLG_CFG_NETABORT), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgNetAbort::showInfo()
{
    m_ckEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
    return TRUE;
}

BOOL CDlgCfgNetAbort::getInfo()
{
    m_stuInfo.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CDlgCfgNetAbort message handlers

BOOL CDlgCfgNetAbort::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_NETABORT);
	// TODO: Add extra initialization here
    
    if (GetConfigFromDevice())
    {
        showInfo();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgNetAbort::OnCfgnetabortBtnEventhandler() 
{
	// TODO: Add your control notification handler code here
    CSubDlgCfgEventHandler* pdlgEventHandler = new CSubDlgCfgEventHandler;
    if (pdlgEventHandler != NULL)
    {
        pdlgEventHandler->SetEventHandler(m_stuInfo.stuEventHandler);
        
        if (IDOK == pdlgEventHandler->DoModal())
        {
            const CFG_ALARM_MSG_HANDLE& stuEventHandler = pdlgEventHandler->GetEventHandler();
            memcpy(&m_stuInfo.stuEventHandler, &stuEventHandler, sizeof(CFG_ALARM_MSG_HANDLE));
        }
        
        delete pdlgEventHandler;
        pdlgEventHandler = NULL;
    }
    else
    {
        MessageBox(ConvertString("new handler dialog error!", DLG_CFG_NETABORT), ConvertString("Prompt"));
	}
}

void CDlgCfgNetAbort::OnCfgnetabortBtnGet() 
{
	// TODO: Add your control notification handler code here    
    if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgNetAbort::OnCfgnetabortBtnSet() 
{
	// TODO: Add your control notification handler code here
    getInfo();
    SetConfigToDevice();
}
