#pragma once

//Use Channel as Object to control
#include "afxwin.h"

typedef struct tagNET_DISPATGROUP_INFO
{
	char		szGroupID[DH_COMMON_STRING_64];		
	int			nSimilarity;						
	BOOL		bDispatch;							
	BOOL		bAddToList;		
	BOOL		bNeedRevoke;
}NET_DISPATGROUP_INFO;

class CDispatchGroupChannelDlg : public CDialog
{
	DECLARE_DYNAMIC(CDispatchGroupChannelDlg)

public:
	CDispatchGroupChannelDlg(LLONG lLoginId, int nChannel,CWnd* pParent =NULL);   
	virtual ~CDispatchGroupChannelDlg();
	afx_msg void OnBnClickedDispatchForchannelRefresh();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	enum { IDD = IDD_DIALOG_DISPATCH_CHANNEL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);  
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickeddispatchdispatch();
	afx_msg void OnBnClickedDispatchForchannelDelete();
	afx_msg void OnBnClickeddispatchadd();

	DECLARE_MESSAGE_MAP()

private:
	void		InitComboxGroupID();
	void		ClearDispatchGroupInfoList();
	void		RefreshDispatchInfoListBox();
	void		DeleteDispatchInfoByGroupID(char *pGroupID);
	void		RevokeDispatchGroupID(char *pGroupID);
	void		DispatchListGroupID();
	BOOL		IsGroupIDDispatch(char *pGroupID);
	
private:
	LLONG		m_lLoginID;
	int			m_nChannel;
	BOOL		m_bNeedClearItem;
	int			m_nStateDlg;	//0:dispatch info dlg  1: add info dlg
	std::list<NET_DISPATGROUP_INFO*>	m_lsDispatchGroupInfo;
public:
	
	CListBox m_DispatchInfoList;
};
