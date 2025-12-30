#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CTrafficFlowDlg Dialog

class CTrafficFlowDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrafficFlowDlg)

public:
	CTrafficFlowDlg(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor
	virtual ~CTrafficFlowDlg();

	// Dialog Data
	enum { IDD = IDD_TRAFIC_FLOW_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnSelchangeCmbLane();
	afx_msg void OnBnClickedBtnQuery();
	afx_msg void OnBnClickedBtnPreviousPage();
	afx_msg void OnBnClickedBtnNextPage();
	afx_msg void OnBnClickedBtnSunscirbeAndUnsubscribe();
	afx_msg void OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	void CleanUp();

private:
	// Self-define function WM_TRAFFIC_FLOW
	LRESULT OnTrafficFlowDate(WPARAM wParam, LPARAM lParam);

	void CleanUpResultOfLastTimeQuery();
	LLONG StartFindTrafficFlow();
	int FindTotalCountOfTrafficFlow();
	bool DoFindTrafficFolw( int nQueryCount );
	void ShowTrafficFlow(NET_RECORD_TRAFFIC_FLOW_STATE* pRecordList, int nCurrnetIndex);
	void GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime);
	bool IsTimeCorrent();

private:
	CButton     m_btnSubscribe;
	CButton     m_btnQuery;
	CButton     m_btnPreviousPage;
	CButton     m_btnNextPage;
	CListCtrl   m_ctrTrafficFlow;
	CListCtrl   m_ctrQueryTrafficFlow;
	CComboBox   m_cmbChannel;
	CComboBox   m_cmbLane;

	LLONG		m_lLoginHandle;
	int			m_nChannel;
	LLONG		m_lSubscribeTrafficFlowHandle;
	LLONG		m_lQueryTrafficFlowHandle;   

	int m_nTotalPage;
	int m_nCurPage; 
	int m_nIndexOfSubscribeTrafficFlow;
	int m_nTotalCountOfTrafficFlow;
	int m_nIndexOfQueryTrafficFlow;
	std::vector<NET_RECORD_TRAFFIC_FLOW_STATE*> m_vecTrafficFlow;  
};
