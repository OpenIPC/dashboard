#if !defined(_FIRSTENTER_)
#define _FIRSTENTER_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FirstEnter : public CDialog
{
	DECLARE_DYNAMIC(FirstEnter)

public:
	FirstEnter(CWnd* pParent = NULL);
	virtual ~FirstEnter();
	enum { IDD = IDD_DIALOG_FIRSTENTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedFirstenterBtnSet();
	afx_msg void OnBnClickedFirstenterBtnGet();
	DECLARE_MESSAGE_MAP()
private:
	void SetFirstEnterInfoToCtrl();
	void GetFirstEnterInfoFromCtrl();
	void InitDlg();
	CComboBox				m_cbFirstEnterStatus;
	CButton					m_chkFirstEnterEnable;
	CComboBox				m_cmbChannel;
	CFG_ACCESS_EVENT_INFO	m_stuInfo;
};
#endif