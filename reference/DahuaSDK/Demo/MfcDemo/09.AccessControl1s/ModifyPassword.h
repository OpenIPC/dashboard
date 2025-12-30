#if !defined(_MODIFYPASSWORD_)
#define _MODIFYPASSWORD_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ModifyPassword : public CDialog
{
	DECLARE_DYNAMIC(ModifyPassword)

public:
	ModifyPassword(CWnd* pParent = NULL); 
	virtual ~ModifyPassword();

	enum { IDD = IDD_DIALOG_MODIFYPASSWORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonMpassword();
	DECLARE_MESSAGE_MAP()
};
#endif