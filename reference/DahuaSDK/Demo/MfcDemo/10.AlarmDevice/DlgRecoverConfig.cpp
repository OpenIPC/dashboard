// DlgRecoverConfig.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgRecoverConfig.h"
#include "DlgRestoreCfgNew.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgRecoverConfig dialog


CDlgRecoverConfig::CDlgRecoverConfig(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */, NET_DEVICE_TYPE emType /* = NET_PRODUCT_NONE */)
	: CDialog(CDlgRecoverConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgRecoverConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	m_emType = emType;
}


void CDlgRecoverConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRecoverConfig)
	DDX_Control(pDX, IDC_RECOVERCONFIG_CMB_REVERTTYPE, m_ctlReverType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgRecoverConfig, CDialog)
	//{{AFX_MSG_MAP(CDlgRecoverConfig)
	ON_BN_CLICKED(IDC_RECOVERCONFIG_BTN_RESET, OnRecoverConfigBtnReset)
	ON_BN_CLICKED(IDC_RECOVERCONFIG_BTN_RESETEX, OnRecoverConfigBtnResetEx)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRecoverConfig message handlers

void CDlgRecoverConfig::OnRecoverConfigBtnReset() 
{
	// TODO: Add your control notification handler code here
    int nIndex = m_ctlReverType.GetCurSel();
    DWORD dwFlag = m_ctlReverType.GetItemData(nIndex);
    BOOL bRet = CLIENT_ControlDevice(m_lLoginId, DH_CTRL_RESTOREDEFAULT, &dwFlag, SDK_API_WAIT);
    if (bRet)
    {
        MessageBox(ConvertString("Set restore ok!", DLG_RECOVERCONFIG), ConvertString("Prompt"));
    }
    else
    {
        CString csInfo;
        csInfo.Format("%s:0x%08x", ConvertString("Set restore failed", DLG_RECOVERCONFIG), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}

BOOL CDlgRecoverConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this, DLG_RECOVERCONFIG);
	
	m_ctlReverType.ResetContent();
	{
		m_ctlReverType.ResetContent();
		int nIndex = m_ctlReverType.AddString(ConvertString("COM Setting", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_COMM);
		nIndex = m_ctlReverType.AddString(ConvertString("Network Setting", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_NETWORK);
		nIndex = m_ctlReverType.AddString(ConvertString("Alarm Setting", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_ALARM);
		nIndex = m_ctlReverType.AddString(ConvertString("PTZ Control", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_PTZ);
		nIndex = m_ctlReverType.AddString(ConvertString("Ouput Mode", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_OUTPUTMODE);
		nIndex = m_ctlReverType.AddString(ConvertString("Restore all config", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_ALL);
		nIndex = m_ctlReverType.AddString(ConvertString("COM Setting|Alarm Setting", DLG_RECOVERCONFIG));
		m_ctlReverType.SetItemData(nIndex,DH_RESTORE_COMM | DH_RESTORE_ALARM);
		nIndex = m_ctlReverType.AddString(ConvertString("PTZ Control|Channel Name", DLG_RECOVERCONFIG));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgRecoverConfig::OnRecoverConfigBtnResetEx() 
{
	// TODO: Add your control notification handler code here
	CDlgRestoreCfgNew dlg(this, m_lLoginId);
    dlg.DoModal();
}
