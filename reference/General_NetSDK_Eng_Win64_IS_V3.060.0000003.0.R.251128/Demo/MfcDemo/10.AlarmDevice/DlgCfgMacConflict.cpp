// DlgCfgMacConflict.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgMacConflict.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMacConflict dialog


CDlgCfgMacConflict::CDlgCfgMacConflict(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgMacConflict::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgMacConflict)
		// NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(CFG_MACCONFLICT_INFO));
}


void CDlgCfgMacConflict::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgMacConflict)
	DDX_Control(pDX, IDC_CFGMACCONFLICT_CHK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgMacConflict, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgMacConflict)
	ON_BN_CLICKED(IDC_CFGMACCONFLICT_BTN_EVENTHANDLER, OnCfgmacconflictBtnEventhandler)
	ON_BN_CLICKED(IDC_CFGMACCONFLICT_BTN_GET, OnCfgmacconflictBtnGet)
	ON_BN_CLICKED(IDC_CFGMACCONFLICT_BTN_SET, OnCfgmacconflictBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMacConflict private method

BOOL CDlgCfgMacConflict::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = -1;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_MACCONFLICT, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_MACCONFLICT, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse MacConflict error..."), DLG_CFG_MACONFLICT), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig MacConflict error:", DLG_CFG_MACONFLICT), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgMacConflict::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};   
    BOOL bRet = CLIENT_PacketData(CFG_CMD_MACCONFLICT, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet MacConflict error..."), DLG_CFG_MACONFLICT), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_MACCONFLICT, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig MacConflict failed:", DLG_CFG_MACONFLICT), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig MacConflict ok!"), DLG_CFG_MACONFLICT), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgMacConflict::showInfo()
{
    m_ckEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
    return TRUE;
}

BOOL CDlgCfgMacConflict::getInfo()
{
    m_stuInfo.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMacConflict message handlers

BOOL CDlgCfgMacConflict::OnInitDialog() 
{
	CDialog::OnInitDialog();
    
	g_SetWndStaticText(this, DLG_CFG_MACONFLICT);
	// TODO: Add extra initialization here
    
    if (GetConfigFromDevice())
    {
        showInfo();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgMacConflict::OnCfgmacconflictBtnEventhandler() 
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

void CDlgCfgMacConflict::OnCfgmacconflictBtnGet() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgMacConflict::OnCfgmacconflictBtnSet() 
{
	// TODO: Add your control notification handler code here
    getInfo();
    SetConfigToDevice();
}
