// DlgCfgSTP.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgSTP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSTP dialog


CDlgCfgSTP::CDlgCfgSTP(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgCfgSTP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgSTP)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgSTP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgSTP)
	DDX_Control(pDX, IDC_STP_CHECK_ENABLE, m_ckEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgSTP, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgSTP)
	ON_BN_CLICKED(IDC_STP_BUTTON_GET, OnStpButtonGet)
	ON_BN_CLICKED(IDC_STP_BUTTON_SET, OnStpButtonSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSTP message handlers

BOOL CDlgCfgSTP::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_STP);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_CFG_STP), ConvertString("Prompt"));
		return FALSE;
	}

	if (getCfgFromDevice())
	{
		StuToDlg();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgSTP::OnStpButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (getCfgFromDevice())
	{
		StuToDlg();
	}
}

void CDlgCfgSTP::OnStpButtonSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	setCfgToDevice();
}

void CDlgCfgSTP::DlgToStu()
{
	m_stuInfo.bEnable = m_ckEnable.GetCheck() ? TRUE : FALSE;
}

void CDlgCfgSTP::StuToDlg()
{
	m_ckEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
}

BOOL CDlgCfgSTP::getCfgFromDevice()
{
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_CFG_STP), ConvertString("Prompt"));
		return FALSE;
	}
	char szOutBuffer[1024] = {0};
	int nerror = 0;

	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_STP, -1, szOutBuffer, 1024, &nerror, SDK_API_WAIT);
	if (bRet)
	{
		DWORD dwRetLen = 0;		
		bRet = CLIENT_ParseData(CFG_CMD_STP, szOutBuffer, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse STP error..."), DLG_CFG_STP), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("get Config STP error:", DLG_CFG_STP), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgSTP::setCfgToDevice()
{
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_CFG_STP), ConvertString("Prompt"));
		return FALSE;
	}

	char szJsonBuf[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_STP, (void*)&m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet STP error..."), DLG_CFG_STP), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_STP, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig STP failed:", DLG_CFG_STP), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
	}
	return TRUE;
}
