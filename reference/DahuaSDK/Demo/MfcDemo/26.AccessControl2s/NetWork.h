#if !defined(_NETWORK_)
#define _NETWORK_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class NetWork : public CDialog
{
	DECLARE_DYNAMIC(NetWork)

public:
	NetWork(CWnd* pParent = NULL);
	virtual ~NetWork();
	enum { IDD = IDD_DIALOG_NETWORK };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedNetworkBtnSet();
	afx_msg void OnBnClickedNetworkBtnGet();
	afx_msg void OnBnClickedRegisterBtnSet();
	afx_msg void OnBnClickedRegisterBtnGet();
	DECLARE_MESSAGE_MAP()
private:
	CIPAddressCtrl m_ctlIp;
	CIPAddressCtrl m_ctlMask;
	CIPAddressCtrl m_ctlGateway;
	CIPAddressCtrl m_ctlAddress;
	CButton m_btnRegisterEnable;
	CFG_DVRIP_INFO m_stuInfo;
};
#endif