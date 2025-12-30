#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxdtctl.h"
#include "PictureCtrl.h"
#include "UserInfoGetFingerPrint.h"
// Authority
typedef struct tagDemoAuthority 
{
	NET_ATTENDANCE_AUTHORITY	emAuthority;
	const char*					szName;
}DemoAuthority;

const DemoAuthority stuDemoAuthority[] =
{
	{NET_ATTENDANCE_AUTHORITY_UNKNOWN, "Unknown"},
	{NET_ATTENDANCE_AUTHORITY_CUSTOMER, "Customer"},
	{NET_ATTENDANCE_AUTHORITY_ADMINISTRATORS, "Administrators"},
};

// USER_TYPE
typedef struct tagDemoUserType 
{
	NET_ENUM_USER_TYPE	emUserType;
	const char*			szName;
}DemoUserType ;

const DemoUserType  stuDemoUserType [] =
{
	{NET_ENUM_USER_TYPE_UNKNOWN, "Unknown"},
	{NET_ENUM_USER_TYPE_NORMAL, "Normal"},
	{NET_ENUM_USER_TYPE_BLACKLIST, "Blocklist"},
	{NET_ENUM_USER_TYPE_GUEST, "Guest"},
	{NET_ENUM_USER_TYPE_PATROL, "Patrol"},
	{NET_ENUM_USER_TYPE_VIP, "VIP"},
	{NET_ENUM_USER_TYPE_HANDICAP, "Handicap"},
	{NET_ENUM_USER_TYPE_CUSTOM1, "CUSTOM1"},
	{NET_ENUM_USER_TYPE_CUSTOM2, "CUSTOM2"},
};

// UserInfo 对话框

class UserInfo : public CDialog
{
	DECLARE_DYNAMIC(UserInfo)

public:
	UserInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~UserInfo();

	void SetDlgMode(int n)
	{
		m_nDlgMode = n;
	}
	void SetDlgHWND(HWND  hParentWndtemp)
	{
		hParentWnd = hParentWndtemp;
	}
	
	void SetUserInfo(const NET_ACCESS_USER_INFO* pstuUserInfo)
	{
		memcpy(&m_stuUserInfo, pstuUserInfo, sizeof(NET_ACCESS_USER_INFO));
	}

	const NET_ACCESS_USER_INFO* GetUserInfo()
	{
		return &m_stuUserInfo;
	}

// 对话框数据
	enum { IDD = IDD_DIALOG_USERINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButFaceOpenpic();
	afx_msg void OnBnClickedButFaceGet();
	afx_msg void OnBnClickedButFaceSet();
	afx_msg void OnBnClickedButFaceModify();
	afx_msg void OnBnClickedButFaceDelete();
	afx_msg void OnBnClickedButFingerprintGet();
	afx_msg void OnBnClickedButFingerprintAdd();
	afx_msg void OnBnClickedButFingerprintModify();
	afx_msg void OnBnClickedButFingerprintDelete();
	afx_msg void OnNMClickListctrlFingerprint(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButCardGet();
	afx_msg void OnBnClickedButCardAdd();
	afx_msg void OnBnClickedButCardModify();
	afx_msg void OnBnClickedButCardDelete();
	afx_msg void OnNMClickListctrlCard(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	int AuthorityToInt(NET_ATTENDANCE_AUTHORITY emStatus);
	NET_ATTENDANCE_AUTHORITY IntToAuthority(int n);
	int UserTypeToInt(NET_ENUM_USER_TYPE emStatus);
	NET_ENUM_USER_TYPE IntToUserType(int n);
	void FaceUIState(BOOL b);
	void FingerPrintUIState(BOOL b);
	void CardListReload();
	void CardUIState(BOOL b);

	HWND			hParentWnd;
	int				m_nDlgMode;
	CListCtrl		m_listFingerPrint;
	CListCtrl		m_listCard;
	CComboBox		m_cmbAuthority;
	CComboBox		m_cmbUserType;
	CButton			m_chkFirstEnter;
	CDateTimeCtrl	m_dtpVDStart;
	CDateTimeCtrl	m_dtpVTStart;
	CDateTimeCtrl	m_dtpVDEnd;
	CDateTimeCtrl	m_dtpVTEnd;

	NET_ACCESS_USER_INFO m_stuUserInfo;
	std::vector<int>	 vecDoor;
	PictureCtrl		m_modifyPic;
	char			m_szFilePath[512];
	char			m_byFingerprintData[6*1024];
	int				m_nFingerprintLen;
	int				m_nFingerprintIndex;

	NET_OUT_ACCESS_FINGERPRINT_SERVICE_GET	m_stuFingerprint;
	NET_ACCESS_CARD_INFO					m_pCardInfo[5];
	int										m_nCardNum;
	HANDLE									m_ThreadHandle;
	BOOL									m_bIsDoFindNextCard;
	DWORD									m_dwThreadID;
	std::vector<NET_ACCESS_CARD_INFO>		m_CardInfoVector;
	int										m_nCardIndex;
public:
	void DoFindNextCardRecord();
	UserInfoGetFingerPrint*		pUserInfoGetFingerPrintDlg;
	afx_msg void OnBnClickedInfoUserinfoBtnDoors();
	LRESULT UserListLoad(WPARAM wParam, LPARAM lParam);
	LRESULT UserCtrlEnable(WPARAM wParam, LPARAM lParam);
	
};
