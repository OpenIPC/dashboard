// DlgCfgPSTNBreakLine.cpp : implementation file
//

#include "stdafx.h"
#include "alarmdevice.h"
#include "DlgCfgPSTNBreakLine.h"
#include "SubDlgCfgEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNBreakLine dialog


CDlgCfgPSTNBreakLine::CDlgCfgPSTNBreakLine(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgPSTNBreakLine::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgPSTNBreakLine)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_lLoginId = lLoginID;
	memset(&m_stuInfo, 0, sizeof(CFG_PSTN_BREAK_LINE_INFO));
}


void CDlgCfgPSTNBreakLine::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgPSTNBreakLine)
	DDX_Control(pDX, IDC_CFGPSTNBREAKLINE_CHK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgPSTNBreakLine, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgPSTNBreakLine)
	ON_BN_CLICKED(IDC_CFGPSTNBREAKLINE_BTN_HANDLER, OnCfgPSTNBreakLineBtnHandler)
	ON_BN_CLICKED(IDC_CFGPSTNBREAKLINE_BTN_GET, OnCfgPSTNBreakLineBtnGet)
	ON_BN_CLICKED(IDC_CFGPSTNBREAKLINE_BTN_SET, OnCfgPSTNBreakLineBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNBreakLine private method

BOOL CDlgCfgPSTNBreakLine::GetConfigFromDevice()
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nSelChn = -1;
    BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_PSTN_BREAK_LINE, nSelChn, szJsonBuf, sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
    
    if (bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_PSTN_BREAK_LINE, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
        if (!bRet)
        {
            MessageBox(ConvertString(CString("parse PSTNBreakLine error..."), DLG_CFG_PSTNBREAKLINE), ConvertString("Prompt"));
            return FALSE;
        }
    }
    else
    {			
        CString csErr;
        csErr.Format("%s 0x%08x",ConvertString("QueryConfig PSTNBreakLine error:", DLG_CFG_PSTNBREAKLINE), CLIENT_GetLastError());
        MessageBox(csErr, ConvertString("Prompt"));
        return FALSE;
    }
    return TRUE;
}

BOOL CDlgCfgPSTNBreakLine::SetConfigToDevice()
{
    char szJsonBuf[1024 * 40] = {0};    
    BOOL bRet = CLIENT_PacketData(CFG_CMD_PSTN_BREAK_LINE, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        MessageBox(ConvertString(CString("packet PSTNBreakLine error..."), DLG_CFG_PSTNBREAKLINE), ConvertString("Prompt"));
        return FALSE;
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_PSTN_BREAK_LINE, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
        if (!bRet)
        {
            CString csErr;
            csErr.Format("%s 0x%08x", ConvertString("SetupConfig PSTNBreakLine failed:", DLG_CFG_PSTNBREAKLINE), CLIENT_GetLastError());
            MessageBox(csErr, ConvertString("Prompt"));
            return FALSE;
        }
        else
        {
            MessageBox(ConvertString(CString("SetConfig PSTNBreakLine ok!"), DLG_CFG_PSTNBREAKLINE), ConvertString("Prompt"));
        }
    }
    return TRUE;
}

BOOL CDlgCfgPSTNBreakLine::showInfo()
{
    m_ckEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
    return TRUE;
}

BOOL CDlgCfgPSTNBreakLine::getInfo()
{
    m_stuInfo.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNBreakLine message handlers

BOOL CDlgCfgPSTNBreakLine::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_PSTNBREAKLINE);
    // TODO: Add extra initialization here
    
    if (GetConfigFromDevice())
    {
        showInfo();
	}	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgPSTNBreakLine::OnCfgPSTNBreakLineBtnHandler() 
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

void CDlgCfgPSTNBreakLine::OnCfgPSTNBreakLineBtnGet() 
{
	// TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        showInfo();
    }
}

void CDlgCfgPSTNBreakLine::OnCfgPSTNBreakLineBtnSet() 
{
	// TODO: Add your control notification handler code here
    getInfo();
    SetConfigToDevice();
}
