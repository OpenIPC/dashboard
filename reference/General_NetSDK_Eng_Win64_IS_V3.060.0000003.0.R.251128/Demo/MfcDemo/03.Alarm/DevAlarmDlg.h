#pragma once
#include "afxcmn.h"
#include "AlarmInfo.h"


// CDevAlarmDlg dialog

class CDevAlarmDlg : public CDialog
{
	DECLARE_DYNAMIC(CDevAlarmDlg)

public:
	CDevAlarmDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDevAlarmDlg();
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL DestroyWindow();

	void UpdateAlarmListCtrl(const CAlarmInfoEx& alarmInfo);	


// Dialog Data
	enum { IDD = IDD_DIALOG_ALARM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
    // Initialize list control of alarm info
    void InitListCtrl();    
public:
    CListCtrl m_listCtrlAlarmInfo;

private:
	void ConvertAlarmType2CString(int emType, CString& strType);
public:
	void ClearAlarmInfo(void);

private:
	unsigned int m_nAlarmCount;
};