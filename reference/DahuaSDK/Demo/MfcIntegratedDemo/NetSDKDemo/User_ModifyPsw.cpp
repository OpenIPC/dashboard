// User_ModifyPsw.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "User_ModifyPsw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyPsw dialog


CUser_ModifyPsw::CUser_ModifyPsw(CWnd* pParent /*=NULL*/)
	: CDialog(CUser_ModifyPsw::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUser_ModifyPsw)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_user_info = NULL;
	m_dev = NULL;
	m_nUserIndex = 0;
}


void CUser_ModifyPsw::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUser_ModifyPsw)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUser_ModifyPsw, CDialog)
	//{{AFX_MSG_MAP(CUser_ModifyPsw)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyPsw message handlers

BOOL CUser_ModifyPsw::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	if (!m_dev || !m_user_info)
	{
		return TRUE;
	}

	GetDlgItem(IDC_NAME_EDIT)->SetWindowText(m_user_info->userList[m_nUserIndex].name);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUser_ModifyPsw::SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD userIdx, DeviceNode *dev)
{
	m_dev = dev;
	m_user_info = info;
	m_nUserIndex = userIdx;
}

void CUser_ModifyPsw::OnBtnOk() 
{
	USER_INFO_NEW newInfo = { sizeof(USER_INFO_NEW) };
	USER_INFO_NEW oldInfo = { sizeof(USER_INFO_NEW) };
	char tmp1[DH_NEW_USER_PSW_LENGTH] = {0};
	char tmp2[DH_NEW_USER_PSW_LENGTH] = {0};
	GetDlgItem(IDC_NEW_PSW_EDIT1)->GetWindowText(tmp1, DH_NEW_USER_PSW_LENGTH);
	GetDlgItem(IDC_NEW_PSW_EDIT2)->GetWindowText(tmp2, DH_NEW_USER_PSW_LENGTH);
	if (strcmp(tmp1, tmp2) != 0)
	{
		MessageBox(ConvertString("The password is not match, please input again"),  ConvertString("Prompt"));
		return;
	}

	strncpy(newInfo.passWord, tmp1, DH_NEW_USER_PSW_LENGTH-1);
	strncpy(oldInfo.name, m_user_info->userList[m_nUserIndex].name, DH_USER_NAME_LENGTH_EX-1);
	GetDlgItem(IDC_OLD_PSW_EDIT)->GetWindowText(tmp1, DH_NEW_USER_PSW_LENGTH);
	strncpy(oldInfo.passWord, tmp1, DH_NEW_USER_PSW_LENGTH-1);

	BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 6, (void *)&newInfo, &oldInfo, NULL, MAX_TIMEOUT);
	if (!bRet)
    {
        int nError = CLIENT_GetLastError();
        if(nError == NET_RETURN_DATA_ERROR)	MessageBox(ConvertString("checkout return data error!"), ConvertString("Prompt"));
        else if(nError == NET_ILLEGAL_PARAM) MessageBox(ConvertString("Input error!"), ConvertString("Prompt"));
        else if(nError == NET_USER_PWD) MessageBox(ConvertString("Reserved account is not able to modify password!"), ConvertString("Prompt"));
        else if(nError == NET_USER_FLASEPWD) MessageBox(ConvertString("Invalid password!"), ConvertString("Prompt"));
        else if(nError == NET_USER_NOMATCHING) MessageBox(ConvertString("The password is not matching!"), ConvertString("Prompt"));
        else MessageBox(ConvertString("Modify user password failed!"), ConvertString("Prompt"));
	}
	else
	{
		EndDialog(0);
	}
}
