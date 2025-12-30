#if !defined(_LOG_)
#define _LOG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Log : public CDialog
{
	DECLARE_DYNAMIC(Log)

public:
	Log(CWnd* pParent = NULL);
	virtual ~Log();

	enum { IDD = IDD_DIALOG_LOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonGetreccount();
	afx_msg void OnBnClickedButtonQueryrec();
	afx_msg void OnBnClickedButtonRequery();
	DECLARE_MESSAGE_MAP()
private:
	void UpdateRecordListCtrlInfo( NET_LOG_INFO* pstuLog);
	void ClearListCtrlInfo();
	void InitDlg();
	CListCtrl	m_listLogRec;
	int			m_LogNo;
};
#endif