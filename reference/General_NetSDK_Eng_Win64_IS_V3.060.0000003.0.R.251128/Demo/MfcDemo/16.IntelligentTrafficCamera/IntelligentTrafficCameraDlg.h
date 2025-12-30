// IntelligentTrafficCameraDlg.h : header file
//

#if !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
#define AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PictureWnd.h"
#include "afxwin.h"
#include "PictureCtrl.h"
#include "Lock.h"

#define WM_USER_EVENT_COME (WM_USER + 1)

// data stored in ListCtrl Item
struct StuListItemData
{
	char*	szGUID;		// GUID
	BYTE*	pBuffer;	// License Plate image buffer
	DWORD	dwBufSize;	// License Plate image size

	StuListItemData() : szGUID(NULL), pBuffer(NULL), dwBufSize(0)
	{

	}
};

struct StuEventItem
{
	StuListItemData listItemData;  // data stored in ListCtrl Item	
	DWORD			dwEventType;
	int				nLen;
	void*			pEventInfo;
	int				nCheckFlag;
	std::string		strDeviceAddress;
	
	StuEventItem(): dwEventType(0), nLen(0), pEventInfo(NULL), nCheckFlag(0), strDeviceAddress("")
	{

	}	
};

typedef  std::map<std::string, StuEventItem> EventMap;

struct EventParam;

/////////////////////////////////////////////////////////////////////////////
// CRealLoadPictureDlg dialog

class CRealLoadPictureDlg : public CDialog
{
	typedef enum UI_STATE
	{
		UI_STATE_SUBSCRIBE_NOW,
		UI_STATE_SUBSCRIBE_STOP
	};
// Construction
public:
	CRealLoadPictureDlg(CWnd* pParent = NULL);	// standard constructor
	~CRealLoadPictureDlg();	

// Dialog Data
	//{{AFX_DATA(CRealLoadPictureDlg)
	enum { IDD = IDD_REALLOADPICTURE_DIALOG };
	CIPAddressCtrl	m_ctrlIP;
	CString			m_strUserName;
	int				m_nPort;
	CString			m_strPasswd;	
	CComboBox		m_cbDisplayFilter;
	CListCtrl		m_lcRealEvent;	
	CComboBox		m_cbChannel;
	CPictureCtrl	m_ctrlPlateImage;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRealLoadPictureDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CRealLoadPictureDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLogin();
	afx_msg void OnButtonLogout();
	afx_msg void OnButtonStartPreview();
	afx_msg void OnButtonStopPreview();
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	afx_msg void OnSelchangeComboDisplayFilter();
	afx_msg void OnNMClickListctrlRealEvent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();		
	afx_msg void OnButtonClear();
	afx_msg void OnKeydownListctrlRealEvent(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonManualSnap();
	afx_msg void OnClose();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEventCome(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	static int  __stdcall RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata);
	
private:	
	static void __stdcall DevDisConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);	
	static void __stdcall DevReConnect(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);

	void UpdateChannelCtrl(BOOL bLogin);
	void OnReceiveEvent(EventParam* pEventParam);
	StuEventItem* GetEventItemByGUID(const char* szGUID);

	void AddEventInfo(StuEventItem& item);
	void ClearRecordData();
	void InitRealEventCtrl();
	void ClearInfoInEventCtrl();
	void SetPictureStorePath();	
	BOOL UpdateLicensePlateNumber(const DWORD dwEventType, const void* pEventInfo);
	// Another way to display license plate image. If DisplayLicensePlateImage fail, try this method to display license plate image
	BOOL DisplayEventImageAndLicensePlateImage(const char* szJpgPath, const StuEventItem* item);
	void FillCWndWithDefaultColor(CWnd* cwnd);

	BOOL StoreEvent(const char* szGUID, const char* szCfgFilePath, const char* szJpgFilePath, EventParam* pEventParam);
	BOOL DisplayImage(const char* szGUID, const char* szJpgFilePath, StuEventItem* pEventItem);	
	BOOL UpdateListCtrlInfo(const char* szGUID, StuEventItem* pEventItem, bool bFilter = false, int nIndex = 0);

	BOOL WhetherToDisplayEventInCtrlList(DWORD dwType) 
	{		
		DWORD dwFilterEventType = GetFilterEventType();	

		if ((dwType != dwFilterEventType) && (dwFilterEventType != EVENT_IVS_ALL))
		{
			return FALSE;
		}
		return TRUE;
	}

	DWORD GetFilterEventType() 
	{
		int nCurSel = m_cbDisplayFilter.GetCurSel();
		if (nCurSel < 0)
		{
			return -1;
		}

		return m_cbDisplayFilter.GetItemData(nCurSel);		
	}

	BOOL DisplayLicensePlateImage(BYTE* pBuffer, size_t nSize);

	BOOL DisplayEventImage(const char* szJPGPath);

	void ChangeDisplayTitle();
	void ChangeUI(int nState);	
	void DisplaySelectedPicInfo(int nItemIndex, CListCtrl* listCtrl);	

	size_t CalculateEventNum(DWORD dwType);

private:
	LLONG					m_lLoginID;
	NET_DEVICEINFO_Ex		m_netDevInfo;
	LLONG					m_lPlayID;
	int						m_nChannel;
	LLONG					m_lRealLoadPicHandle;
	CPictureWnd				m_cPictureWnd;
	CFont*					m_Font;
	unsigned int			m_nEventCount;

	EventMap				m_mapAllEventItem;

	CRITICAL_SECTION		m_csMap; // for m_mapAllAlarmItem

	static const int MAX_EVENT_TO_DISPLAY = 100 ;
	static const int MAX_EVENT_STORE_IN_MAP = 200;
	static const int HIMETRIC_INCH = 2540;
public:
	afx_msg void OnBnClickedButtonBwList();
	afx_msg void OnBnClickedButtonTrafficFlow();
	afx_msg void OnBnClickedButtonTrafficQuery();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
