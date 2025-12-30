// UserInfoGetCard.cpp : 实现文件
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "UserInfoGetCard.h"


// UserInfoGetCard 对话框

IMPLEMENT_DYNAMIC(UserInfoGetCard, CDialog)

UserInfoGetCard::UserInfoGetCard(CWnd* pParent /*=NULL*/)
	: CDialog(UserInfoGetCard::IDD, pParent)
{
	memset(&m_stuCardInfo,0,sizeof(m_stuCardInfo));

	m_nDlgMode = 0;
}

UserInfoGetCard::~UserInfoGetCard()
{
}

void UserInfoGetCard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CARD_CMB_CARDTYPE, m_comCardType);
}


BEGIN_MESSAGE_MAP(UserInfoGetCard, CDialog)
	ON_BN_CLICKED(IDOK, &UserInfoGetCard::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL UserInfoGetCard::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	m_comCardType.ResetContent();
	for (int i = 0; i < sizeof(stuDemoCardType)/sizeof(stuDemoCardType[0]); i++)
	{
		m_comCardType.InsertString(-1, ConvertString(stuDemoCardType[i].szName));
	}
	m_comCardType.SetCurSel(0);
	
	StuToDlg();
	if (m_nDlgMode == 2)//modify
	{
		GetDlgItem(IDC_CARD_EDT_CARDNO)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}
// UserInfoGetCard 消息处理程序

void UserInfoGetCard::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	DlgToStu();
	OnOK();
}

void UserInfoGetCard::StuToDlg()
{
	SetDlgItemText(IDC_CARD_EDT_CARDNO, m_stuCardInfo.szCardNo);
	m_comCardType.SetCurSel(CardTypeToInt(m_stuCardInfo.emType));
}
void UserInfoGetCard::DlgToStu()
{
	GetDlgItemText(IDC_CARD_EDT_CARDNO, m_stuCardInfo.szCardNo, sizeof(m_stuCardInfo.szCardNo));
	m_stuCardInfo.emType = IntToCardType(m_comCardType.GetCurSel());
}

int UserInfoGetCard::CardTypeToInt(NET_ACCESSCTLCARD_TYPE emType)
{
	int n = -1;
	if (NET_ACCESSCTLCARD_TYPE_UNKNOWN == emType)
	{
		n = 0;
	}
	else if (NET_ACCESSCTLCARD_TYPE_GENERAL == emType)
	{
		n = 1;
	}
	else if (NET_ACCESSCTLCARD_TYPE_VIP == emType)
	{
		n = 2;
	}
	else if (NET_ACCESSCTLCARD_TYPE_GUEST == emType)
	{
		n = 3;
	}
	else if (NET_ACCESSCTLCARD_TYPE_PATROL == emType)
	{
		n = 4;
	}
	else if (NET_ACCESSCTLCARD_TYPE_BLACKLIST == emType)
	{
		n = 5;
	}
	else if (NET_ACCESSCTLCARD_TYPE_CORCE == emType)
	{
		n = 6;
	}
	else if (NET_ACCESSCTLCARD_TYPE_MOTHERCARD == emType)
	{
		n = 7;
	}
	return n;
}

NET_ACCESSCTLCARD_TYPE UserInfoGetCard::IntToCardType(int n)
{
	NET_ACCESSCTLCARD_TYPE emType = NET_ACCESSCTLCARD_TYPE_UNKNOWN;
	if (0 == n)
	{
	}
	else if (1 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_GENERAL;
	}
	else if (2 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_VIP;
	}
	else if (3 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_GUEST;
	}
	else if (4 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_PATROL;
	}
	else if (5 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_BLACKLIST;
	}
	else if (6 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_CORCE;
	}
	else if (7 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_MOTHERCARD;
	}
	return emType;
}