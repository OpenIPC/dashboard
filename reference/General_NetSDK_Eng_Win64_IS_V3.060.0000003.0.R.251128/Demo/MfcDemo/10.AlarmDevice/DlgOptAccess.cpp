// DlgOptAccess.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgOptAccess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgOptAccess dialog


CDlgOptAccess::CDlgOptAccess(CWnd* pParent /* = NULL */, LLONG hLoginID /* = 0 */)
	: CDialog(CDlgOptAccess::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgOptAccess)
		// NOTE: the ClassWizard will add member initialization here
	m_hLoginID = hLoginID;
	//}}AFX_DATA_INIT
}


void CDlgOptAccess::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgOptAccess)
	DDX_Control(pDX, IDC_CMB_CHANNEL, m_cbChannelId);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgOptAccess, CDialog)
	//{{AFX_MSG_MAP(CDlgOptAccess)
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnCancel)
	ON_BN_CLICKED(IDC_BTN_OPEN, OnBtnOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOptAccess message handlers

void CDlgOptAccess::OnBtnCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CDlgOptAccess::OnBtnOpen() 
{
	// TODO: Add your control notification handler code here
	NET_CTRL_ACCESS_OPEN stuAccessPara = {sizeof(stuAccessPara)};
	stuAccessPara.nChannelID = m_cbChannelId.GetCurSel();

	BOOL bRet = CLIENT_ControlDevice(m_hLoginID, DH_CTRL_ACCESS_OPEN, &stuAccessPara, SDK_API_WAIT);
	if (!bRet)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Open access failed.", DLG_OPT_ACCESS), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return;
	}

	MessageBox(ConvertString("Open access successfully.", DLG_OPT_ACCESS), ConvertString("Prompt"));
}

BOOL CDlgOptAccess::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_OPT_ACCESS);
	
	if (!m_hLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_OPT_ACCESS), ConvertString("Prompt"));
		return TRUE;
	}

	// TODO: Add extra initialization here
	m_cbChannelId.InsertString(0, ConvertString("channel 0", DLG_OPT_ACCESS));
	m_cbChannelId.InsertString(1, ConvertString("channel 1", DLG_OPT_ACCESS));
	m_cbChannelId.InsertString(2, ConvertString("channel 2", DLG_OPT_ACCESS));
	m_cbChannelId.InsertString(3, ConvertString("channel 3", DLG_OPT_ACCESS));
	m_cbChannelId.InsertString(4, ConvertString("channel 4", DLG_OPT_ACCESS));

	m_cbChannelId.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
