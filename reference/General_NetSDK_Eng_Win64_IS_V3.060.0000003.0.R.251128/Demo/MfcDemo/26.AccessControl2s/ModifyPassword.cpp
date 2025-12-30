// ModifyPassword.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "ModifyPassword.h"

IMPLEMENT_DYNAMIC(ModifyPassword, CDialog)

ModifyPassword::ModifyPassword(CWnd* pParent /*=NULL*/)
	: CDialog(ModifyPassword::IDD, pParent)
{

}

ModifyPassword::~ModifyPassword()
{
}

void ModifyPassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ModifyPassword, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_MPASSWORD, &ModifyPassword::OnBnClickedButtonMpassword)
END_MESSAGE_MAP()


BOOL ModifyPassword::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUTTON_MPASSWORD)->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void ModifyPassword::OnBnClickedButtonMpassword()
{
	CString csName;
	GetDlgItemText(IDC_MODIFYPSW_EDT_NAME, csName);
	CString csOldPassword;
	GetDlgItemText(IDC_MODIFYPSW_EDT_OLD, csOldPassword);
	CString csNewPassword;
	GetDlgItemText(IDC_MODIFYPSW_EDT_NEW, csNewPassword);
	CString csCheckPassword;
	GetDlgItemText(IDC_MODIFYPSW_EDT_CHECK, csCheckPassword);

	if (!csName.GetLength())
	{
		MessageBox(ConvertString("Input Name."), ConvertString("Prompt"));
		return;
	}

	if (!csOldPassword.GetLength())
	{
		MessageBox(ConvertString("Input old password."), ConvertString("Prompt"));
		return;
	}

	if (!csNewPassword.GetLength())
	{
		MessageBox(ConvertString("Input new password."), ConvertString("Prompt"));
		return;
	}

	if (!csCheckPassword.GetLength())
	{
		MessageBox(ConvertString("Input check password."), ConvertString("Prompt"));
		return;
	}

	if (csCheckPassword != csNewPassword)
	{
		MessageBox(ConvertString("Two passwords are different, please check again."), ConvertString("Prompt"));
		return;
	}
	BOOL bret = Device::GetInstance().ModifyPassword(csName,csOldPassword,csNewPassword);
	if (bret)
	{
		MessageBox(ConvertString("Modify password successfully."), ConvertString("Prompt"));
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Modify password failed:"), Device::GetInstance().GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}
