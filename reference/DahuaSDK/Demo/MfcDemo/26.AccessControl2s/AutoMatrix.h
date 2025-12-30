#if !defined(_AUTOMATRIX_)
#define _AUTOMATRIX_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AutoMatrix : public CDialog
{
	DECLARE_DYNAMIC(AutoMatrix)

public:
	AutoMatrix(CWnd* pParent = NULL);
	virtual ~AutoMatrix();

	enum { IDD = IDD_DIALOG_AUTOMATRIX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAutomatrixBtnGet();
	afx_msg void OnBnClickedAutomatrixBtnSet();
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	CComboBox m_comAutoRebootDay;
	CComboBox m_comAutoRebootTime;
	CComboBox m_comAutoDeleteFileTime;
};
#endif