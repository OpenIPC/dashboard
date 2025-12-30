// DDNS_QueryIP.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "DDNS_QueryIP.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDDNS_QueryIP dialog


CDDNS_QueryIP::CDDNS_QueryIP(CWnd* pParent /*=NULL*/)
	: CDialog(CDDNS_QueryIP::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDDNS_QueryIP)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDDNS_QueryIP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDDNS_QueryIP)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDDNS_QueryIP, CDialog)
	//{{AFX_MSG_MAP(CDDNS_QueryIP)
	ON_BN_CLICKED(IDC_GO, OnGo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDDNS_QueryIP message handlers

void CDDNS_QueryIP::OnGo() 
{
	char sOutput[32];
	ZeroMemory(sOutput, 32); //ÓÃÓÚÊä³öIP

	CString strIP;
	CString strName;
	
	GetDlgItem(IDC_EDIT_IP)->GetWindowText(strIP);
	int nPort = GetDlgItemInt(IDC_EDIT_PORT);
	GetDlgItem(IDC_EDIT_DEVNAME)->GetWindowText(strName);
	
	BOOL nRet = CLIENT_GetDVRIPByResolveSvr(strIP.GetBuffer(0), nPort, (BYTE *)strName.GetBuffer(0), strName.GetLength(), 0, 0, (char *)sOutput);
	if (!nRet)
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();
		GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(ConvertString("failed"));
	}
	else
	{
		GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(sOutput);
	}
}

BOOL CDDNS_QueryIP::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	SetDlgItemText(IDC_EDIT_IP, "10.5.2.149");
	SetDlgItemInt(IDC_EDIT_PORT, 7070);
	SetDlgItemText(IDC_EDIT_DEVNAME, "111");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
