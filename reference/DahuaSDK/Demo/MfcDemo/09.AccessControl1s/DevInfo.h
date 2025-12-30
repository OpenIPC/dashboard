#if !defined(_DEVINFO_)
#define _DEVINFO_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DevInfo : public CDialog
{
	DECLARE_DYNAMIC(DevInfo)

public:
	DevInfo(CWnd* pParent = NULL);
	virtual ~DevInfo();
	enum { IDD = IDD_DIALOG_DEVINFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDeviceinfoBtnGet();
	DECLARE_MESSAGE_MAP()
};
#endif