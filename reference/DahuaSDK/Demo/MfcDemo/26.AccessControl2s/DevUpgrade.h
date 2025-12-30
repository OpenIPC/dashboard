#if !defined(_DEVUPGRADE_)
#define _DEVUPGRADE_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DevUpgrade : public CDialog
{
	DECLARE_DYNAMIC(DevUpgrade)

public:
	DevUpgrade(CWnd* pParent = NULL);
	virtual ~DevUpgrade();
	void UpgradeState(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize);
	enum { IDD = IDD_DIALOG_DEVUPGRADE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDeviceupgradeBtnStart();
	afx_msg void OnBnClickedDeviceupgradeBtnStop();
	afx_msg void OnBnClickedDeviceupgradeBtnOpenpath();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	CListBox m_lsInfo;
	CProgressCtrl m_ctrlProgress;
};
#endif