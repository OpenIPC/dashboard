#pragma once
#include "afxwin.h"


// card type
typedef struct tagDemoCardType 
{
	NET_ACCESSCTLCARD_TYPE	emType;
	const char*				szName;
}DemoCardType;

const DemoCardType stuDemoCardType[] = 
{
	{NET_ACCESSCTLCARD_TYPE_UNKNOWN, "Unknown"},
	{NET_ACCESSCTLCARD_TYPE_GENERAL, "General"},
	{NET_ACCESSCTLCARD_TYPE_VIP, "VIP"},
	{NET_ACCESSCTLCARD_TYPE_GUEST, "Guest"},
	{NET_ACCESSCTLCARD_TYPE_PATROL, "Patrol"},
	{NET_ACCESSCTLCARD_TYPE_BLACKLIST, "Blocklist"},
	{NET_ACCESSCTLCARD_TYPE_CORCE, "Duress"},
	{NET_ACCESSCTLCARD_TYPE_MOTHERCARD, "MotherCard"},
};

class UserInfoGetCard : public CDialog
{
	DECLARE_DYNAMIC(UserInfoGetCard)

public:
	UserInfoGetCard(CWnd* pParent = NULL);
	virtual ~UserInfoGetCard();

	void SetDlgMode(int n)
	{
		m_nDlgMode = n;
	}

	void SetCardInfo(const NET_ACCESS_CARD_INFO* pstuCardInfo)
	{
		memcpy(&m_stuCardInfo, pstuCardInfo, sizeof(NET_ACCESS_CARD_INFO));
	}

	const NET_ACCESS_CARD_INFO* GetCardInfo()
	{
		return &m_stuCardInfo;
	}

	enum { IDD = IDD_DIALOG_USERINFO_GETCARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()


	NET_ACCESS_CARD_INFO m_stuCardInfo;
public:
	void	StuToDlg();
	void	DlgToStu();
	CComboBox m_comCardType;
	afx_msg void OnBnClickedOk();

	int CardTypeToInt(NET_ACCESSCTLCARD_TYPE emType);
	NET_ACCESSCTLCARD_TYPE IntToCardType(int n);

	int m_nDlgMode; // 1-ADD  2-Modify
};
