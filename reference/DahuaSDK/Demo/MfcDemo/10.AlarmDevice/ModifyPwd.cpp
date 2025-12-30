// ModifyPwd.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "ModifyPwd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyPwd dialog


CModifyPwd::CModifyPwd(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyPwd::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyPwd)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_usersInfo = 0;
	m_hLoginID = 0;
	m_dwUserID = 0;
}


void CModifyPwd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyPwd)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyPwd, CDialog)
	//{{AFX_MSG_MAP(CModifyPwd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyPwd message handlers

void CModifyPwd::OnOK() 
{
	// TODO: Add extra validation here
	USER_INFO_NEW newInfo = {sizeof(newInfo)};
	USER_INFO_NEW oldInfo = {sizeof(oldInfo)};
	char strPwd1[DH_NEW_USER_PSW_LENGTH + 1] = {0};
	char strPwd2[DH_NEW_USER_PSW_LENGTH + 1] = {0};
	GetDlgItem(IDC_MODIFY_PWD_EDIT_NEWPWD1)->GetWindowText(strPwd1, DH_NEW_USER_PSW_LENGTH+1);
	GetDlgItem(IDC_MODIFY_PWD_EDIT_NEWPWD2)->GetWindowText(strPwd2, DH_NEW_USER_PSW_LENGTH+1);
	if (strcmp(strPwd1, strPwd2) == 0)
	{
		memcpy(newInfo.passWord, strPwd1, DH_NEW_USER_PSW_LENGTH);
	}
	else
	{
		MessageBox(ConvertString("Please check the new words you have set, they are not matching!", DLG_USERMANAGER), ConvertString("Prompt"));
		return;
	}
	
	memcpy(oldInfo.name, m_usersInfo->userList[m_dwUserID].name, DH_NEW_USER_NAME_LENGTH);
	GetDlgItem(IDC_MODIFY_PWD_EDIT_OLDPWD)->GetWindowText(strPwd1, DH_NEW_USER_PSW_LENGTH+1);
	memcpy(oldInfo.passWord, strPwd1, DH_NEW_USER_PSW_LENGTH);

	BOOL bRet = CLIENT_OperateUserInfoNew(m_hLoginID, 6, (void *)&newInfo, &oldInfo, 0, 1000 * 10);
	if (!bRet)
	{
		int nError = CLIENT_GetLastError();
		if(nError == NET_RETURN_DATA_ERROR)	MessageBox(ConvertString("checkout return data error!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_ILLEGAL_PARAM) MessageBox(ConvertString("Input error!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_PWD) MessageBox(ConvertString("Reserved account is not able to modify password!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_FLASEPWD) MessageBox(ConvertString("Invalid password!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_NOMATCHING) MessageBox(ConvertString("The password is not matching!", DLG_USERMANAGER), ConvertString("Prompt"));
		else MessageBox(ConvertString("Modify user password failed!", DLG_USERMANAGER), ConvertString("Prompt"));
	}
	else
	{
		CDialog::OnOK();
	}
}
void CModifyPwd::InitModifyPwdInfoDlg(USER_MANAGE_INFO_NEW *info, DWORD dwID, LLONG hLoginID)
{
	m_usersInfo = info;
	m_dwUserID = dwID;
	m_hLoginID = hLoginID;
}

BOOL CModifyPwd::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_USERMANAGER);
	// TODO: Add extra initialization here
	if (!m_usersInfo || !m_hLoginID)
	{
		return TRUE;
	}

	char szUserName[DH_NEW_USER_NAME_LENGTH + 1] = {0};
	memcpy(szUserName, m_usersInfo->userList[m_dwUserID].name, DH_NEW_USER_NAME_LENGTH);
	GetDlgItem(IDC_MODIFY_PWD_EDIT_USERNAME)->SetWindowText(szUserName);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CModifyPwd::IsValid()
{
	BOOL bValid = TRUE;
	CString str1,str2,str3;
	GetDlgItemText(IDC_MODIFY_PWD_EDIT_OLDPWD,str1);
	GetDlgItemText(IDC_MODIFY_PWD_EDIT_NEWPWD1,str2);
	GetDlgItemText(IDC_MODIFY_PWD_EDIT_NEWPWD2,str3);
	if(str1.IsEmpty() || str2.IsEmpty() || str3.IsEmpty())
	{
		MessageBox(ConvertString("Please Input!", DLG_USERMANAGER), ConvertString("Prompt"));
		bValid = FALSE;
	}
	else if(str2 != str3)
	{
		MessageBox(ConvertString("Input  password error, please input again.", DLG_USERMANAGER), ConvertString("Prompt"));
		bValid = FALSE;
	}
	return bValid;
}

