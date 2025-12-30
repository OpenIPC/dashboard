#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CHistoryQuery 对话框

//Exact to the hour
struct StTime
{
	int nYear;
	int nMonth;
	int nDay;
	int nHour;
};

class CHistoryQuery : public CDialog
{
	DECLARE_DYNAMIC(CHistoryQuery)

public:
	CHistoryQuery(LLONG lLogin, int nChannel, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHistoryQuery();

// 对话框数据
	enum { IDD = IDD_HISTORY_QUERY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	void	InitControl();
	BOOL	StartFindNumberState(int nChannel, int nType);
	int		DoFindNumberStarteEx(int nBegin, int nSearchCount);
	void	StopFindNumberState();
	void	GetTimeFromUI(int nType);

	void	UpdateLcQueryInfo(DH_NUMBERSTAT *pstuNumberStat, int nCount);

private:
	int			m_nChannel;
	LLONG		m_lLoginHandle;
	LLONG		m_lQueryHandle;
	StTime		m_BeginTime;
	StTime		m_EndTime;
	int			m_nPage;				//页数
	int			m_nRuleType;			//0:人数统计  1:区域内人数统计
	int			m_QueryCount;
	int			m_nStayTime;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonQuery();
	afx_msg void OnCbnSelchangeComboQueryType();
	afx_msg void OnCbnSelchangeComboRuleType();
	afx_msg void OnBnClickedButtonNextpage();
	afx_msg void OnBnClickedButtonPrepage();

private:
	CComboBox m_cbxType;
	CComboBox m_cbxQueryChannel;
	CComboBox m_cbxRuleType;
	CListCtrl m_lcQueryInfo;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
