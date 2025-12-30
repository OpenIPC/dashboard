#pragma once

//Use Face Library as Object to control
#include "StdAfx.h"

typedef struct tagNET_DISPATCH_INFO
{
	int			nChannel;			
	int			nSimilarity;	
	BOOL		bDispatch;		
}NET_DISPATCH_INFO;

// CDispatchGroupDlg dialog

class CDispatchGroupDlg : public CDialog
{
	DECLARE_DYNAMIC(CDispatchGroupDlg)

public:
	CDispatchGroupDlg(LLONG lLoginId, int nChnCount, NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDispatchGroupDlg();

	enum { IDD = IDD_DISPATCH_DIALOG };

protected:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);    

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickeddispatchadd();
	afx_msg void OnBnClickeddispatchdelete();
	afx_msg void OnBnClickeddispatchdispatch();
	afx_msg void OnBnClickeddispatchrefresh();

	NET_FACERECONGNITION_GROUP_INFO *GetDispatchGroupInfo() {return &m_stuDispatchGroupInfo;}

private:
	void CleanDispatchList();
	NET_DISPATCH_INFO* SearchDispatchInfoByChannel(const int nChannel);
	void RefreshDispatchInfo(const char *szGroupId, BOOL bRefreshShow = FALSE);
	void DeleteDispatchInfoByChannel(const int nChannel);

	

	CComboBox	m_comboDispatchChannel;
	CListBox	m_DispatchInfoList;

	LLONG		m_lLoginID;
	int			m_nChnCount;
	NET_FACERECONGNITION_GROUP_INFO m_stuDispatchGroupInfo;
	std::list<NET_DISPATCH_INFO*>	 m_lstDispatchChannelInfo;		
};
