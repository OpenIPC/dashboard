#include "afxwin.h"
#include "afxcmn.h"
#if !defined(AFX_DLGCFGPSTNPERSONSEVER_H__347D68CE_E649_497F_BB6E_D82FFCA286E8__INCLUDED_)
#define AFX_DLGCFGPSTNPERSONSEVER_H__347D68CE_E649_497F_BB6E_D82FFCA286E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// CDlgCfgPSTNPersonServer 对话框

class CDlgCfgPSTNPersonServer : public CDialog
{
	DECLARE_DYNAMIC(CDlgCfgPSTNPersonServer)

public:
	CDlgCfgPSTNPersonServer(CWnd* pParent = NULL , LLONG lLoginId = 0 );   // 标准构造函数
	virtual ~CDlgCfgPSTNPersonServer();

// 对话框数据
	enum { IDD = IDD_DLG_CFG_PSTNPERSONSERVER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CButton m_chkEnable;
	CListCtrl m_lsPersonServerList;

private:
	void InitDlg(BOOL bShow = TRUE);
	BOOL SetConfigToDevice();
	BOOL GetConfigFromDevice();
	void DlgToStu();
	void StuToDlg();

private:
	LLONG								m_lLoginId;
	CFG_PSTN_PERSON_SERVER_INFO_ALL		m_stuInfo;
public:
	afx_msg void OnBnClickedBtnPstnpersonserverGet();
	afx_msg void OnBnClickedBtnPstnpersonserverSet();
	afx_msg void OnBnClickedBtnPstnpersonserverAddnumber();
	afx_msg void OnBnClickedBtnPstnpersonserverDelnumber();
};

#endif
