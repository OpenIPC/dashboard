// InitDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "searchdevice.h"
#include "InitDeviceDlg.h"
#include <CTYPE.H>
#include <string>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////
// CInitDeviceDlg dialog


CInitDeviceDlg::CInitDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInitDeviceDlg::IDD, pParent)
    , m_strConfirmPwd(_T(""))
{
	//{{AFX_DATA_INIT(CInitDeviceDlg)
	m_strPwd = _T("");
	m_strRig = _T("");
	m_strUserName = _T("admin");
	m_strPwdRestWay = _T("");
    m_nIndex = 0;
    m_byPwdResetWay = 0;
	//}}AFX_DATA_INIT
}


void CInitDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CInitDeviceDlg)
    DDX_Text(pDX, IDC_EDIT_PWD, m_strPwd);
    DDX_Text(pDX, IDC_EDIT_RIG, m_strRig);
    DDX_Text(pDX, IDC_EDIT_UNAME, m_strUserName);
    DDX_Text(pDX, IDC_EDIT_PwdRestWsy, m_strPwdRestWay);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_EDIT_CNFM_MPWD, m_strConfirmPwd);
}


BEGIN_MESSAGE_MAP(CInitDeviceDlg, CDialog)
	//{{AFX_MSG_MAP(CInitDeviceDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CInitDeviceDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInitDeviceDlg message handlers

BOOL CInitDeviceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    g_SetWndStaticText(this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInitDeviceDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE); 
    
    CDialog::OnOK();
}

void CInitDeviceDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	CDialog::OnCancel();
}

BOOL CInitDeviceDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	return CDialog::DestroyWindow();
}



void CInitDeviceDlg::OnBnClickedOk()
{
	// TODO: Add your specialized code here and/or call the base class
    UpdateData(TRUE);

    if (m_strPwd.IsEmpty())
    {
        MessageBox(ConvertString("Please input password"), ConvertString("Prompt"));
        return;
    }

    if (m_strConfirmPwd.IsEmpty())
    {
        MessageBox(ConvertString("Please confirm password"), ConvertString("Prompt"));
        return;
    }

    if (m_strConfirmPwd != m_strPwd)
    {
        MessageBox(ConvertString("Confirm password is invalid,please input again"), ConvertString("Prompt"));
        return;
    }

	OnOK();
}


BOOL CInitDeviceDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP)
	{
		if (VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam)
		{
			return TRUE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}