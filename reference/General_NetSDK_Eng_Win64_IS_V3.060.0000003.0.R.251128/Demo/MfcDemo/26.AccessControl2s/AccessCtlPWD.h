#if !defined(_ACCESSCTLPWD_)
#define _ACCESSCTLPWD_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AccessCtlPWD : public CDialog
{
	DECLARE_DYNAMIC(AccessCtlPWD)

public:
	AccessCtlPWD(CWnd* pParent = NULL);
	virtual ~AccessCtlPWD();
	enum { IDD = IDD_DIALOG_ACCESSCTLPWD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButAccesspwdExecute();
	afx_msg void OnBnClickedInfoPasswordBtnDoors();
	afx_msg void OnCbnSelchangeComAccesspwdExecutetype();
	DECLARE_MESSAGE_MAP()
private:
	void		AccessPWDInsert();
	void		AccessPWDGet();
	void		AccessPWDUpdate();
	void		AccessPWDRemove();
	void		AccessPWDClear();
	void		GetAccessPWDInfoFromCtrl();
	void		SetAccessPWDInfoToCtrl();
	CComboBox						m_cmbCtlType;
	NET_RECORDSET_ACCESS_CTL_PWD	m_stuRecordAccessPWD;
	std::vector<int>				vecDoor;
public:
	afx_msg void OnBnClickedButAccesspwdGet();
	void OnChangeUIState(int nState);
};
#endif