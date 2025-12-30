#if !defined(_STROBESETDLG_)
#define _STROBESETDLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxwin.h"

class StrobeSetDlg : public CDialog
{
	DECLARE_DYNAMIC(StrobeSetDlg)

public:
	StrobeSetDlg(CWnd* pParent = NULL);
	virtual ~StrobeSetDlg();

	enum { IDD = IDD_DIALOG_STROBESET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
private:
	afx_msg void OnCbnSelchangeComboWeek();
	afx_msg void OnCbnSelchangeComboTimeslot();
	afx_msg void OnBnClickedButtonSet();
	afx_msg void OnBnClickedButtonGet();
	afx_msg void OnEnChangeEdithours();
	afx_msg void OnEnChangeEditminutes();
	afx_msg void OnEnChangeEditseconds();
	afx_msg void OnEnChangeEdithoure();
	afx_msg void OnEnChangeEditminutee();
	afx_msg void OnEnChangeEditseconde();
	void GetTimefromStu();
	CString		m_strhours;
	CString		m_strminutes;
	CString		m_strseconds;
	CString		m_strhoure;
	CString		m_strminutee;
	CString		m_strseconde;
	NET_CFG_TRAFFICSTROBE_INFO m_stuTrafficStrobeInfo_Get;
	void MakeTimeNormal(BOOL bStartEnd);
public:
	afx_msg void OnEnKillfocusEdithours();
	afx_msg void OnEnKillfocusEditminutes();
	afx_msg void OnEnKillfocusEditseconds();
	afx_msg void OnEnKillfocusEdithoure();
	afx_msg void OnEnKillfocusEditminutee();
	afx_msg void OnEnKillfocusEditseconde();
};
#endif