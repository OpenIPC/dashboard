#include "afxdtctl.h"
#if !defined(_DOORRECORD_)
#define _DOORRECORD_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DoorRecord : public CDialog
{
	DECLARE_DYNAMIC(DoorRecord)

public:
	DoorRecord(CWnd* pParent = NULL);
	virtual ~DoorRecord();
	enum { IDD = IDD_DIALOG_DOORRECORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonGetreccount();
	afx_msg void OnBnClickedButtonStartquery();
	afx_msg void OnBnClickedButtonNextpage();
	afx_msg void OnBnClickedButtonStopquery();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
private:
	void UpdateRecordListCtrlInfo( NET_RECORDSET_ACCESS_CTL_CARDREC* pstuCardRec);
	void ClearListCtrlInfo();
	void InitDlg();
	CListCtrl m_listDoorRec;
public:
	afx_msg void OnDtnDatetimechangeRecsetCardDtpVdend(NMHDR *pNMHDR, LRESULT *pResult);
	CDateTimeCtrl m_dtpVDStart;
	CDateTimeCtrl m_dtpVTStart;
	CDateTimeCtrl m_dtpVDEnd;
	CDateTimeCtrl m_dtpVTEnd;
	
	void UIState(BOOL b);
};
#endif