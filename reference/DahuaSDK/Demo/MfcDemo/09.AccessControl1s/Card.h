#include "afxwin.h"
#if !defined(_CARD_)
#define _CARD_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WM_ACCESS_FINGERPRINT (WM_USER + 300)

// card status
typedef struct tagDemoCardStatus 
{
	NET_ACCESSCTLCARD_STATE	emStatus;
	const char*				szName;
}DemoCardStatus;

const DemoCardStatus stuDemoCardStatus[] =
{
	{NET_ACCESSCTLCARD_STATE_UNKNOWN, "Unknown"},
	{NET_ACCESSCTLCARD_STATE_NORMAL, "Normal"},
	{NET_ACCESSCTLCARD_STATE_LOSE, "Lose"},
	{NET_ACCESSCTLCARD_STATE_LOGOFF, "LogOff"},
	{NET_ACCESSCTLCARD_STATE_FREEZE, "Freeze"},
};

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


class CCard : public CDialog
{
	DECLARE_DYNAMIC(CCard)

public:
	CCard(CWnd* pParent = NULL);
	virtual ~CCard();

	enum { IDD = IDD_DIALOG_CARD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButCardExecute();
	afx_msg void OnBnClickedRecsetCardBtnDoors();
	afx_msg void OnBnClickedRecsetCardBtnTm();
	afx_msg void OnCbnSelchangeComCardExecutetype();
	DECLARE_MESSAGE_MAP()
private:
	void	InitDlg();
	void	StuToDlg();
	void	DlgToStu();
	int		CardStatusToInt(NET_ACCESSCTLCARD_STATE emStatus);
	int		CardTypeToInt(NET_ACCESSCTLCARD_TYPE emType);
	NET_ACCESSCTLCARD_STATE IntToCardStatus(int n);
	NET_ACCESSCTLCARD_TYPE IntToCardType(int n);
	void	CardInsert(BOOL bEx);
	void	CardGet();
	void	CardUpdate(BOOL bEx);
	void	CardRemove();
	void	CardClear();
	
	CComboBox m_cmbCtlType;
	CDateTimeCtrl m_dtpCreateTimeDate;
	CDateTimeCtrl m_dtpCreateTimeTime;
	CDateTimeCtrl m_dtpVDStart;
	CDateTimeCtrl m_dtpVTStart;
	CDateTimeCtrl m_dtpVDEnd;
	CDateTimeCtrl m_dtpVTEnd;
	CButton m_chkFirstEnter;
	CComboBox m_cmbCardType;
	CComboBox m_cmbCardStatus;
	NET_RECORDSET_ACCESS_CTL_CARD	m_stuInfo;
	std::vector<int> vecDoor;
	std::vector<int> vecTimeSection;
	HWND			hParentWnd;
public:
	afx_msg void OnBnClickedRecsetFingerprintBtnGet();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	LRESULT OnFingerPrintMessage(WPARAM wParam, LPARAM lParam);

	bool	m_bCaptureSuc;
	bool	m_bStarted;
	bool	m_bFinished;
	char	m_byFingerprintData[2][6*1024];
	int		m_nFingerprintLen[2];
	CComboBox m_cmbFingerPrintNO;
	afx_msg void OnBnClickedButCardGet();

	void OnChangeUIState(int nState);
};
#endif