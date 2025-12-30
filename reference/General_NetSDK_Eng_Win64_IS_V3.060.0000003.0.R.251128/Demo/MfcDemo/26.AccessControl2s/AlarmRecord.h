#if !defined(_ALARMRECORD_)
#define _ALARMRECORD_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AlarmRecord : public CDialog
{
	DECLARE_DYNAMIC(AlarmRecord)

public:
	AlarmRecord(CWnd* pParent = NULL);
	virtual ~AlarmRecord();
	enum { IDD = IDD_DIALOG_ALARMRECORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonGetreccount();
	afx_msg void OnBnClickedButtonQueryrec();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonRequery();
	DECLARE_MESSAGE_MAP()
private:
	void UpdateRecordListCtrlInfo( NET_RECORD_ACCESS_ALARMRECORD_INFO* pstuCardRec);
	void ClearListCtrlInfo();
	void InitDlg();
	CListCtrl m_listAlarmRec;
};
#endif