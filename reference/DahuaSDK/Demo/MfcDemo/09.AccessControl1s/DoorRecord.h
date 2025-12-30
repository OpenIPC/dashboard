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
	afx_msg void OnBnClickedButtonQueryrec();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonRequery();
	DECLARE_MESSAGE_MAP()
private:
	void UpdateRecordListCtrlInfo( NET_RECORDSET_ACCESS_CTL_CARDREC* pstuCardRec);
	void ClearListCtrlInfo();
	void InitDlg();
	CListCtrl m_listDoorRec;
};
#endif