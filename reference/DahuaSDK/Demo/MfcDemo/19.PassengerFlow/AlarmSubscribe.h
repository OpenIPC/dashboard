#pragma once
#include "afxcmn.h"
#include "ShowPictureDlg.h"
#include "PictureCtrl.h"

struct EventPicInfo
{
	BYTE		*pPicBuf;
	DWORD		dwBufSize;
	std::string strGuid;
};

// CAlarmSubscribe 对话框

class CAlarmSubscribe : public CDialog
{
	DECLARE_DYNAMIC(CAlarmSubscribe)

public:
	CAlarmSubscribe(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAlarmSubscribe();

// 对话框数据
	enum { IDD = IDD_ALARM_ATTCH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	void	InitALarmDlg(int nChannel, LLONG mLoginID);
	void	SetChannelNum(int nChannel);
	void	ExitDlg();
	afx_msg LRESULT DoDeviceAttach(WPARAM wParam, LPARAM lParam);

private:
	void	InitAlarmInfoLct();
	void	DoAttchData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize);
	void	DisplayEventInfo(int nChannel, CString strType, CString strRuleName, CString strAlarmInfo, CString strTime, std::string& strGuid);
	void	StoreEventPicInfo(BYTE *pBuffer, DWORD dwBufSize, std::string& strGuid);

	void	DeleteAllItemInfo();
	void	DeleteAllStoreEventData();
	
private:
	int		m_nChannel;
	LLONG	m_lLoginId;
	LLONG	m_lRealPicHandle;
	DWORD	m_index;

	std::list <EventPicInfo*> m_EventPicList;

	CShowPictureDlg m_dlgShowPic;

public:
	afx_msg void OnBnClickedButtonAlarmattach();
	CListCtrl m_lcAlarmInfo;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAlarmdetach();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLvnItemchangedListRealAttchinfo(NMHDR *pNMHDR, LRESULT *pResult);
};
