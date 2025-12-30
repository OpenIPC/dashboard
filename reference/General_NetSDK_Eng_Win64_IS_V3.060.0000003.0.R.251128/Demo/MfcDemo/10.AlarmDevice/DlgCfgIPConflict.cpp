// DlgCfgIPConflict.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgIPConflict.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgIPConflict dialog


CDlgCfgIPConflict::CDlgCfgIPConflict(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgIPConflict::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgIPConflict)
		// NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(CFG_IPCONFLICT_INFO));
}


void CDlgCfgIPConflict::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgIPConflict)
	DDX_Control(pDX, IDC_CFGIPCONFLICT_CHK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgIPConflict, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgIPConflict)
	ON_BN_CLICKED(IDC_CFGIPCONFLICT_BTN_EVENTHANDLER, OnCfgipconflictBtnEventhandler)
	ON_BN_CLICKED(IDC_CFGIPCONFLICT_BTN_GET, OnCfgipconflictBtnGet)
	ON_BN_CLICKED(IDC_CFGIPCONFLICT_BTN_SET, OnCfgipconflictBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgIPConflict private method

BOOL CDlgCfgIPConflict::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = -1;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_IPCONFLICT, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_IPCONFLICT, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse IPConflict error..."), DLG_CFG_IPCONFLICT), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig IPConflict error:", DLG_CFG_IPCONFLICT), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgIPConflict::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};   
    BOOL bRet = CLIENT_PacketData(CFG_CMD_IPCONFLICT, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet IPConflict error..."), DLG_CFG_IPCONFLICT), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_IPCONFLICT, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig IPConflict failed:", DLG_CFG_IPCONFLICT), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig IPConflict ok!"), DLG_CFG_IPCONFLICT), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgIPConflict::showInfo()
{
    m_ckEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
    return TRUE;
}

BOOL CDlgCfgIPConflict::getInfo()
{
    m_stuInfo.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgIPConflict message handlers

BOOL CDlgCfgIPConflict::OnInitDialog() 
{
	CDialog::OnInitDialog();
    
	g_SetWndStaticText(this, DLG_CFG_IPCONFLICT);
	// TODO: Add extra initialization here
    
    if (GetConfigFromDevice())
    {
        showInfo();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgIPConflict::OnCfgipconflictBtnEventhandler() 
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

void CDlgCfgIPConflict::OnCfgipconflictBtnGet() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        showInfo();
	}
}

void CDlgCfgIPConflict::OnCfgipconflictBtnSet() 
{
	// TODO: Add your control notification handler code here
    getInfo();
    SetConfigToDevice();
}
