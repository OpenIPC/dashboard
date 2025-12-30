#if !defined(_BLACKWHILTLISTDLG_)
#define _BLACKWHILTLISTDLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxwin.h"
#include "afxcmn.h"

#define WM_TRANSMIT_EVENT (WM_USER + 200)
#define WM_STOP_EXPORT (WM_USER + 201)
#define WM_STOP_IMPORT (WM_USER + 202)

enum EM_TRAFFIC_LIST_OPERATOR_TYPE
{
	EM_UNKNOWN,
	EM_IMPORT_LIST_START, 
	EM_EXPORT_LIST_START
};

class BlackWhiltListDlg : public CDialog
{
	DECLARE_DYNAMIC(BlackWhiltListDlg)

public:
	BlackWhiltListDlg(CWnd* pParent = NULL);
	virtual ~BlackWhiltListDlg();

	enum { IDD = IDD_DIALOG_BLACKWHILTLIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	
private:
	afx_msg void OnDestroy();
	void ChangeUI(EM_TRAFFIC_LIST_OPERATOR_TYPE emchangeType);
	void StopLoadList();
	void StopSendList();
	afx_msg void OnBnClickedButtonImport();
	afx_msg void OnBnClickedButtonExport();
	afx_msg LRESULT OnTransmitEvent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopExport(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopImport(WPARAM wParam, LPARAM lParam);
	//LLONG					m_nTransmitHandle;
	EM_TRAFFIC_LIST_OPERATOR_TYPE m_emchangeType;
	CProgressCtrl			m_progress;
	CComboBox				m_combo_list;
};
#endif