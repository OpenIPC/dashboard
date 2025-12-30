// UserDetailDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Attendance.h"
#include "UserDetailDlg.h"
#include "AttendanceManager.h"
#include "Util.h"

// CUserDetailDlg 对话框

IMPLEMENT_DYNAMIC(CUserDetailDlg, CDialog)

CUserDetailDlg::CUserDetailDlg(CAttendanceManager *pManager, CString strUserID,
							   CString strUserName, CString strCardNo, 
							   EM_USER_OPERATOR_TYPE type, CWnd* pParent)
	: CDialog(CUserDetailDlg::IDD, pParent)
	, m_pManager(pManager)
	, m_type(type)
	, m_strUserDetail_userid(strUserID)
	, m_struserdetail_username(strUserName)
	, m_struserdetail_cardno(strCardNo)
{
	
}

CUserDetailDlg::~CUserDetailDlg()
{
}

void CUserDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_USERDETAIL_USERID, m_strUserDetail_userid);
	DDX_Text(pDX, IDC_EDIT_USERDETAIL_USERNAME, m_struserdetail_username);
	DDX_Text(pDX, IDC_EDIT_USERDETAIL_CARDNO, m_struserdetail_cardno);
}


BEGIN_MESSAGE_MAP(CUserDetailDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_USERDETAIL, &CUserDetailDlg::OnBnClickedButtonUserdetail)
END_MESSAGE_MAP()


void CUserDetailDlg::OnBnClickedButtonUserdetail()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	char *pcUserId = (LPSTR)(LPCSTR)m_strUserDetail_userid;
	char *pcUserName = (LPSTR)(LPCSTR)m_struserdetail_username;
	char *pcCardNo = (LPSTR)(LPCSTR)m_struserdetail_cardno;

	if (strlen(pcUserId) == 0)
	{
		MessageBox(ConvertString("Please Input UserID"), ConvertString("Prompt"));
		return ;
	}
	if (EM_USER_OPERATOR_TYPE_ADD == m_type)
	{
		NET_IN_ATTENDANCE_ADDUSER stuInAddUser = {sizeof(stuInAddUser)};
		stuInAddUser.pbyPhotoData = NULL;
		strncpy(stuInAddUser.stuUserInfo.szUserID, pcUserId, sizeof(stuInAddUser.stuUserInfo.szUserID)-1);
		strncpy(stuInAddUser.stuUserInfo.szUserName, pcUserName, sizeof(stuInAddUser.stuUserInfo.szUserName)-1);
		strncpy(stuInAddUser.stuUserInfo.szCardNo, pcCardNo, sizeof(stuInAddUser.stuUserInfo.szCardNo)-1);
		stuInAddUser.stuUserInfo.emAuthority = NET_ATTENDANCE_AUTHORITY_CUSTOMER;
		bool bRet = m_pManager->AddAttendanceUser(&stuInAddUser);
		if (bRet)
		{
			MessageBox(ConvertString("Add User success"), ConvertString("Prompt"));
		}
		else
		{
			MessageBox(ConvertString("Add User failed"), ConvertString("Prompt"));	
		}
	}
	else if (EM_USER_OPERATOR_TYPE_MODIFY == m_type)
	{
		NET_IN_ATTENDANCE_ModifyUSER stuInModifyUser = { sizeof(stuInModifyUser) };
		strncpy(stuInModifyUser.stuUserInfo.szUserID, pcUserId, sizeof(stuInModifyUser.stuUserInfo.szUserID)-1);
		strncpy(stuInModifyUser.stuUserInfo.szUserName, pcUserName, sizeof(stuInModifyUser.stuUserInfo.szUserName)-1);
		strncpy(stuInModifyUser.stuUserInfo.szCardNo, pcCardNo, sizeof(stuInModifyUser.stuUserInfo.szCardNo)-1);
		stuInModifyUser.stuUserInfo.emAuthority = NET_ATTENDANCE_AUTHORITY_CUSTOMER;
		bool bRet = m_pManager->ModifyAttendanceUser(&stuInModifyUser);
		if (bRet)
		{
			MessageBox(ConvertString("Modify User success"), ConvertString("Prompt"));
		}
		else
		{
			MessageBox(ConvertString("Modify User failed"), ConvertString("Prompt"));	
		}
	}
}

BOOL CUserDetailDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO:  在此添加额外的初始化

	if (EM_USER_OPERATOR_TYPE_ADD == m_type)
	{
		SetWindowText(ConvertString("Add User"));
	}
	else if (EM_USER_OPERATOR_TYPE_MODIFY == m_type)
	{
		SetWindowText(ConvertString("Modify User"));
		GetDlgItem(IDC_EDIT_USERDETAIL_USERID)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CUserDetailDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_KEYDOWN == pMsg->message)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			return true;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
