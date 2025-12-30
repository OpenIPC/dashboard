// DlgBypassCtl.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgBypassCtl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBypassCtl dialog


CDlgBypassCtl::CDlgBypassCtl(CWnd* pParent /* = NULL */, LLONG lLoginId /* = NULL */, unsigned int uiChannelNum /* = 0 */)
	: CDialog(CDlgBypassCtl::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgBypassCtl)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginId = lLoginId;
	m_uiChannelNum = uiChannelNum;
	m_nExtChannelNum = 256;
	//}}AFX_DATA_INIT
}


void CDlgBypassCtl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgBypassCtl)
	DDX_Control(pDX, IDC_BYPASSCTL_CMB_EXTCHANNEL, m_cmbExtChannel);
	DDX_Control(pDX, IDC_BYPASSCTL_CMB_MODE, m_cmbBypassMode);
	DDX_Control(pDX, IDC_BYPASSCTL_CMB_CHANNEL, m_cmbChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgBypassCtl, CDialog)
	//{{AFX_MSG_MAP(CDlgBypassCtl)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BYPASSCTL_BTN_GET, OnBypassctlBtnGet)
	ON_BN_CLICKED(IDC_BYPASSCTL_BTN_SET, OnBypassctlBtnSet)
	ON_BN_CLICKED(IDC_BYPASSCTL_RADIO_CHANNEL, OnBypassctlRadioChannel)
	ON_BN_CLICKED(IDC_BYPASSCTL_RADIO_EXTCHANNEL, OnBypassctlRadioExtchannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBypassCtl private methods

void CDlgBypassCtl::InitDlg()
{
	if (m_uiChannelNum <= 0)
	{
		return;
	}

	m_cmbChannel.ResetContent();
	for (unsigned int i = 0; i < m_uiChannelNum; i++)
	{
		CString csName;
		csName.Format("%s %d", ConvertString("Channel", DLG_BYPASSCTL), i+1);
		m_cmbChannel.InsertString(-1, csName);
	}
	m_cmbChannel.SetCurSel(-1);
	m_cmbChannel.SetDroppedWidth(100);

	m_cmbExtChannel.ResetContent();
	for (int k = 0; k < m_nExtChannelNum; k++)
	{
		CString csName;
		csName.Format("%s %d", ConvertString("ExtChannel", DLG_BYPASSCTL), k+1);
		m_cmbExtChannel.InsertString(-1, csName);
	}
	m_cmbExtChannel.SetCurSel(-1);
	m_cmbExtChannel.SetDroppedWidth(120);

	CButton* pBtn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_CHANNEL);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_EXTCHANNEL);
	pBtn->SetCheck(BST_UNCHECKED);
	GetDlgItem(IDC_BYPASSCTL_CMB_CHANNEL)->EnableWindow();
	GetDlgItem(IDC_BYPASSCTL_CMB_EXTCHANNEL)->EnableWindow(FALSE);

	m_cmbBypassMode.ResetContent();
	for (int j = 0; j < sizeof(stuBypassModeInfo) / sizeof(stuBypassModeInfo[0]); j++)
	{
		m_cmbBypassMode.InsertString(-1, ConvertString(stuBypassModeInfo[j].szMode, DLG_BYPASSCTL));
	}
	m_cmbBypassMode.SetCurSel(-1);
}


/////////////////////////////////////////////////////////////////////////////
// CDlgBypassCtl message handlers

BOOL CDlgBypassCtl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_BYPASSCTL);

	// TODO: Add extra initialization here
	InitDlg();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgBypassCtl::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CDlgBypassCtl::OnBypassctlBtnGet() 
{
	// TODO: Add your control notification handler code here
	CButton* pBtnChn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_CHANNEL);
	CButton* pBtnExtChn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_EXTCHANNEL);


	NET_BYPASS_MODE emBypassMode = NET_BYPASS_MODE_UNKNOW;
	int nLocal[] = {m_cmbChannel.GetCurSel()};
	int nExtLocal[] = {m_cmbExtChannel.GetCurSel()};

	NET_DEVSTATE_GET_BYPASS stuGetBypass = {sizeof(stuGetBypass)};

	if (pBtnChn->GetCheck())
	{
		stuGetBypass.nLocalCount = sizeof(nLocal)/sizeof(nLocal[0]);
		stuGetBypass.pemLocal = &emBypassMode;
		stuGetBypass.pnLocal = nLocal;
	}
	else if (pBtnExtChn->GetCheck())
	{
		stuGetBypass.nExtendedCount = sizeof(nExtLocal)/sizeof(nExtLocal[0]);
		stuGetBypass.pemExtended = &emBypassMode;
		stuGetBypass.pnExtended = nExtLocal;
	}


	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_GET_BYPASS, 
		(char*)&stuGetBypass, sizeof(NET_DEVSTATE_GET_BYPASS), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryChannelMode failed:", DLG_BYPASSCTL), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	} 
	else
	{
		m_cmbBypassMode.SetCurSel((int)emBypassMode);
	}
}

void CDlgBypassCtl::OnBypassctlBtnSet() 
{
	// TODO: Add your control notification handler code here
	CButton* pBtnChn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_CHANNEL);
	CButton* pBtnExtChn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_EXTCHANNEL);

	CString csPsw;
	GetDlgItemText(IDC_BYPASSCTL_EDT_PSW, csPsw);

	NET_CTRL_SET_BYPASS stuSetBypass = {sizeof(stuSetBypass)};
	stuSetBypass.szDevPwd = csPsw.GetBuffer(0);
	stuSetBypass.emMode = (NET_BYPASS_MODE)m_cmbBypassMode.GetCurSel();

	int nLocal[] = {m_cmbChannel.GetCurSel()};
	int nExtLocal[] = {m_cmbExtChannel.GetCurSel()};

	if (pBtnChn->GetCheck())
	{
		stuSetBypass.nLocalCount = sizeof(nLocal)/sizeof(nLocal[0]);
		stuSetBypass.pnLocal = nLocal;
	}
	else if (pBtnExtChn->GetCheck())
	{
		stuSetBypass.nExtendedCount = sizeof(nExtLocal)/sizeof(nExtLocal[0]);
		stuSetBypass.pnExtended = nExtLocal;
	}

	BOOL bRet = CLIENT_ControlDevice(m_lLoginId, DH_CTRL_SET_BYPASS, &stuSetBypass, SDK_API_WAIT);
	if (bRet)
	{
		MessageBox(ConvertString("Set bypass ok!", DLG_BYPASSCTL), ConvertString("Prompt"));
	} 
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("set bypass failed:", DLG_BYPASSCTL), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
}

void CDlgBypassCtl::OnBypassctlRadioChannel() 
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_CHANNEL);
	pBtn->SetCheck(BST_CHECKED);
	pBtn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_EXTCHANNEL);
	pBtn->SetCheck(BST_UNCHECKED);
	GetDlgItem(IDC_BYPASSCTL_CMB_CHANNEL)->EnableWindow();
	GetDlgItem(IDC_BYPASSCTL_CMB_EXTCHANNEL)->EnableWindow(FALSE);
}

void CDlgBypassCtl::OnBypassctlRadioExtchannel() 
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_CHANNEL);
	pBtn->SetCheck(BST_UNCHECKED);
	pBtn = (CButton*)GetDlgItem(IDC_BYPASSCTL_RADIO_EXTCHANNEL);
	pBtn->SetCheck(BST_CHECKED);
	GetDlgItem(IDC_BYPASSCTL_CMB_CHANNEL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BYPASSCTL_CMB_EXTCHANNEL)->EnableWindow();
}
