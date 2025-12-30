// IntelligentDeviceDlg.h : header file
//

#pragma once

#include <list>
#include <string>
#include "afxwin.h"
#include "PictureCtrl.h"
#include "ShowPictureDlg.h"

#define EVENT_NUM 2

struct EventDataInfo
{

	std::string strGUID;
	std::string strDetail;
	std::string strTime;
	std::string strObjectType;
	int nChannel;
	int EventID;
	DWORD index;
	BYTE* Data;
	DWORD dataSize;

};

typedef std::list< EventDataInfo > EventList;
typedef std::list< EventDataInfo* > AllEventsInfo;
typedef std::list<LLONG> handle_list;


// CIntelligentDeviceDlg dialog
class CIntelligentDeviceDlg : public CDialog
{
// Construction
public:
	CIntelligentDeviceDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CIntelligentDeviceDlg();

// Dialog Data
	enum { IDD = IDD_INTELLIGENTDEVICE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

public:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


protected:
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();

	afx_msg void OnDestroy();
	afx_msg void OnBtnStartrealplay();
	afx_msg void OnBtnStoprealplay();
	afx_msg void OnBtnAttachevent();
	afx_msg void OnBtnDetachevent();
	afx_msg void OnSelchangeCbxChannel();
	afx_msg void OnCbnSelchangeCbxEvent();
	afx_msg void OnNMDblclkListEventinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnQueryDownload();
	BOOL		DestroyWindow() ;

	LRESULT OnAlarmCome(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeviceDisConnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeviceReconnect(WPARAM wParam, LPARAM lParam);

public:
	void	InitNetSDK();
	void	UnInitNetSDK();

	void	ShowLoginErrorReason(int nError);
	void	InitEventListCtrl();
	void	DealWithEvent(LLONG lAnalyzerHandle, DWORD dwAlarmType, char* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, int nSequence);

	//void	DeviceDisConnect(LLONG lLoginID, char *sDVRIP,LLONG nDVRPort);
	void	ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LLONG lParam);
	void	StopAttch();

private:
	LLONG m_loginID;
	LLONG m_lPlayID;
	LLONG m_lAttchID;

	//handle_list m_attachList;

	int m_nChannelNum;

	EventList m_eventList[EVENT_NUM];		//2 events
	AllEventsInfo m_allEvents;

	DWORD m_nFilterEventType;
	DWORD m_index;

	CShowPictureDlg m_dlgShowPic;


public:
	static void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	static void CALLBACK HaveReConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

	static int CALLBACK RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, 
		BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata);


private:
	static int GetAlarmInfoSize( DWORD AlarmType, DWORD *pAlarmSize);
	static int GetShowItemInfo(DWORD AlarmType,char* pAlarmInfo, std::string& strTime,int& channe, std::string& strDetail, std::string& strObject);



	bool StoreEventInfoToList(int channel, int EventID, BYTE* EventData,DWORD dataSize,
		std::string& strGUID,std::string& strTime, std::string& strDetail, std::string& strObject);

	void DisplayEventInfo(DWORD filterType, DWORD dwAlarmType, char* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize);

	void InitEventCombox();

	void DeleteAllItems();

	void DeleteAllStoreEventData();

	void EventType2Index(DWORD type, int* pIndex);

protected:
	CListCtrl	m_lcEventInfo;
	CComboBox	m_cbxChannel;
	CComboBox	m_cbxEvent;

	PictureCtrl m_EventPic;
};
