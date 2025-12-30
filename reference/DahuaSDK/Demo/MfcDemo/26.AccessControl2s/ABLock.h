#include "afxwin.h"
#if !defined(_ABLOCK_)
#define _ABLOCK_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ABLock : public CDialog
{
	DECLARE_DYNAMIC(ABLock)

public:
	ABLock(CWnd* pParent = NULL);
	virtual ~ABLock();
	enum { IDD = IDD_DIALOG_ABLOCK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnBnClickedAblockButGet();
	afx_msg void OnBnClickedAblockButSet();
	afx_msg void OnCbnSelchangeAblockComGroupnum();
	afx_msg void OnCbnSetfocusAblockComGroupnum();
	DECLARE_MESSAGE_MAP()	
private:
	virtual BOOL OnInitDialog();
	void SetABLockInfoToCtrl();
	void GetABLockInfoFromCtrl();
	CString Get_ABLOCK_EDIT_DOOR_text(int nid);
	void Set_ABLOCK_EDIT_DOOR_text(int nid, CString str);
	int Get_ABLOCK_EDIT_DOOR_int(int nid);
	void Set_ABLOCK_EDIT_DOOR_int(int nid, int nvalue);
	void InitDlg();
	CFG_ACCESS_GENERAL_INFO m_stuInfo;
	CButton					m_cbeLockEnable;
	CComboBox m_comABLockGroupNO;
	
};
#endif