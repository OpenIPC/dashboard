#if !defined(_LATTICESCREENDLG_)
#define _LATTICESCREENDLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxwin.h"

class LatticeScreenDlg : public CDialog
{
	DECLARE_DYNAMIC(LatticeScreenDlg)

public:
	LatticeScreenDlg(CWnd* pParent = NULL);
	virtual ~LatticeScreenDlg();

	enum { IDD = IDD_DIALOG_LATTICESCREEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	afx_msg void OnBnClickedButtonSet();
	afx_msg void OnCbnSelchangeComboStatus();
	CComboBox		m_combo_usertype;
	CComboBox		m_combo_status;
};
#endif