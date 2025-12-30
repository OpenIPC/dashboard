// DlgUserManage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgUserManage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUserManage dialog


CDlgUserManage::CDlgUserManage(CWnd* pParent /* = NULL */, LLONG hLoginID /* = 0 */, NET_DEVICE_TYPE emDevType /* = NET_PRODUCT_NONE */)
	: CDialog(CDlgUserManage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgUserManage)
	m_csNewPasswd = _T("");
	m_csCheckPasswd = _T("");
	m_csOldPasswd = _T("");
	m_hLoginID = hLoginID;
	m_emDevType = emDevType;
	//}}AFX_DATA_INIT
}


void CDlgUserManage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUserManage)
	DDX_Text(pDX, IDC_MODIFYPSW_EDT_NEW, m_csNewPasswd);
	DDX_Text(pDX, IDC_MODIFYPSW_EDT_CHECK, m_csCheckPasswd);
	DDX_Text(pDX, IDC_MODIFYPSW_EDT_OLD, m_csOldPasswd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUserManage, CDialog)
	//{{AFX_MSG_MAP(CDlgUserManage)
	ON_BN_CLICKED(IDC_MODIFYPSW_BTN_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_MODIFYPSW_BTN_SET, OnButtonSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgUserManage message handlers

void CDlgUserManage::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CDlgUserManage::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (!m_hLoginID)
	{
		MessageBox(ConvertString("Please login first.", DLG_USER), ConvertString("Prompt"));
		return;
	}

// 	if (!m_csOldPasswd.GetLength())
// 	{
// 		MessageBox(ConvertString("Input old password.", DLG_USER), ConvertString("Prompt"));
// 		return;
// 	}

	if (!m_csNewPasswd.GetLength())
	{
		MessageBox(ConvertString("Input new password.", DLG_USER), ConvertString("Prompt"));
		return;
	}

	if (!m_csCheckPasswd.GetLength())
	{
		MessageBox(ConvertString("Input check password.", DLG_USER), ConvertString("Prompt"));
		return;
	}

	if (m_csCheckPasswd != m_csNewPasswd)
	{
		MessageBox(ConvertString("Two passwords are different, please check again.", DLG_USER), ConvertString("Prompt"));
		return;
	}

	USER_INFO_NEW stuOldInfo = {sizeof(stuOldInfo)};
	GetDlgItemText(IDC_MODIFYPSW_EDT_NAME, stuOldInfo.name, sizeof(stuOldInfo.name) - 1);	
	strncpy(stuOldInfo.passWord, m_csOldPasswd.GetBuffer(0), sizeof(stuOldInfo.passWord) - 1);

	USER_INFO_NEW stuModifiedInfo = {sizeof(stuModifiedInfo)};
	strncpy(stuModifiedInfo.passWord, m_csNewPasswd.GetBuffer(0), sizeof(stuModifiedInfo.passWord)-1);

	BOOL bRet = CLIENT_OperateUserInfoNew(m_hLoginID, 6, &stuModifiedInfo, &stuOldInfo, NULL, SDK_API_WAIT);
	if (bRet)
	{
		MessageBox(ConvertString("Modify password successfully.", DLG_USER), ConvertString("Prompt"));
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Modify password failed:", DLG_USER), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

BOOL CDlgUserManage::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_USER);

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
