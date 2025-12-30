#pragma once
#include "afxwin.h"


// CAddAndModifyBWDlg Dialog 

enum EM_TRAFFIC_LIST_OPERATOR_TYPE{EM_ADD_BACKLIST, EM_MODIFY_BACKLIST, EM_ADD_ALLOWLIST, EM_MODIFY_ALLOWLIST};
class CAddAndModifyBWDlg : public CDialog
{
	DECLARE_DYNAMIC(CAddAndModifyBWDlg)

public:
	CAddAndModifyBWDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddAndModifyBWDlg();

	// Dialog Data
	enum { IDD = IDD_ADD_AND_MODIFY_BW_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancle();
	afx_msg void OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	void SetLoginHandle(const LLONG lLoginHandle); 
	void SetTrafficListInfo(const NET_TRAFFIC_LIST_RECORD* pTrafficListInfo);
	void SetTrafficOperatorType(EM_TRAFFIC_LIST_OPERATOR_TYPE emTrafficOperatorType);

private:
	LLONG m_lLoginHandle;
	NET_TRAFFIC_LIST_RECORD m_stTrafficListInfo;
	EM_TRAFFIC_LIST_OPERATOR_TYPE m_emTrafficOperatorType;
	void ShowTime();
	void GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime);
	bool IsTimeCorrent();  
public:
	CComboBox m_cbPlateType;
	CComboBox m_cbPlateColor;
	CComboBox m_cbVehicleType;
	CComboBox m_cbVehicleColor;
	CComboBox m_cbControlType;
};
