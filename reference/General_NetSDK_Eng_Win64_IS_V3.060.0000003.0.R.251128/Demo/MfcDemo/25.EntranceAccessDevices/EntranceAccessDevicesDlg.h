// IntelligentTrafficDlg.h : header file
//

#if !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
#define AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "TalkDlg.h"
#include "PictureWnd.h"
#include "afxwin.h"
#include "PictureCtrl.h"
#include "Lock.h"
#include "afxcmn.h"
#include "AlarmFactory.h"
#define WM_PICTUREALARM_COME (WM_USER + 1)
#define WM_EVENTALARM_POSITION_INFO (WM_USER + 5)
#define WM_LISTENINVITE (WM_USER + 6)
#define MAX_MSG_SHOW	(100)

/////////////////////////////////////////////////////////////////////////////
// CEntranceAccessDevicesDlg dialog
class CEntranceAccessDevicesDlg : public CDialog
{
// Construction
public:
	CEntranceAccessDevicesDlg(CWnd* pParent = NULL);	// standard constructor
	~CEntranceAccessDevicesDlg();	

// Dialog Data
	//{{AFX_DATA(CEntranceAccessDevicesDlg)
	enum { IDD = IDD_ENTRANCEACCESSDEVICES_DIALOG };
	CIPAddressCtrl	m_ctrlIP;
	CString			m_strUserName;
	int				m_nPort;
	CString			m_strPasswd;	
	CListCtrl		m_listPictureAlarm;	
	CListCtrl		m_listEventAlarm;
	CComboBox		m_cbChannel;
	CPictureCtrl	m_ctrlPlateImage;
	CMenu			m_Menu;
	CButton			m_check_open_type1;
	CButton			m_check_open_type2;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEntranceAccessDevicesDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	//{{AFX_MSG(CEntranceAccessDevicesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnNMClickListctrlRealEvent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();		
	afx_msg void OnKeydownListctrlRealEvent(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	afx_msg LRESULT OnPictureAlarmCome(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEventAlarmInfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT ListenInvite(WPARAM wParam, LPARAM lParam);

	afx_msg void OnButtonLogin();
	afx_msg void OnButtonLogout();
	afx_msg void OnButtonStartPreview();
	afx_msg void OnButtonStopPreview();
	afx_msg void OnButtonStartSubscribePicture();
	afx_msg void OnButtonStopSubscribePicture();
	afx_msg void OnBnClickedButtonManualSnap();
	afx_msg void OnBnClickedButtonOpenStrobe();
	afx_msg void OnButtonStartSubscribeEvent();
	afx_msg void OnButtonStopSubscribeEvent();
	afx_msg void OnBnClickedCheckOpenType1();
	afx_msg void OnBnClickedCheckOpenType2();

	afx_msg void OnStrobeset();
	afx_msg void OnBlackwhiltlist();
	afx_msg void OnTalk();
	afx_msg void OnLatticescreen();

	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
private:	
	void OnReceivePictureAlarm(Alarm* alarm/*EventParam* pEventParam*/);
	StuAlarmItem* GetAlarmItemByGUID(const char* szGUID);
	//void AddAlarmInfo(StuAlarmItem& item);
	void ClearPictureAlarmUIInfo();
	void ClearPictureAlarmRecordData();
	void ClearEventAlarmUIInfo() ;
	void InitPictureAlarmCtrl();
	void InitEventAlarmCtrl();
	void ClearPictureAlarmInEventCtrl();
	BOOL UpdateLicensePlateNumber(const DWORD dwEventType, const void* pEventInfo);
	void FillCWndWithDefaultColor(CWnd* cwnd);
	BOOL StorePicture(const char* szJpgFilePath, EventParam* pEventParam);
	BOOL DisplayImage(const char* szGUID, const char* szJpgFilePath, StuAlarmItem* pEventItem);	
	BOOL UpdatePictureAlarmListCtrlInfo(const char* szGUID, StuAlarmItem* pEventItem, bool bFilter = false, int nIndex = 0);
	BOOL DisplayLicensePlateImage(BYTE* pBuffer, size_t nSize);
	BOOL DisplayEventImage(const char* szJPGPath);
	void ChangeUI(int nState);	
	void DisplaySelectedPicInfo(int nItemIndex, CListCtrl* listCtrl);	
	BOOL StartListenEventAlarm();
	BOOL StopListenEventAlarm();
	BOOL OpenStrobe();
	void InitMenu();
	void InitPicturePath();
	void InitCtrls();
	void InitNetSdk();

	CPictureWnd				m_cPictureWnd;
	CFont*					m_Font;
	unsigned int			m_nPictureAlarmCount;
	unsigned int			m_nEventAlarmCount;
	BOOL					m_bSubIntelligent;
	BOOL					m_bOrdinary;
	CToolTipCtrl			m_tt;
public:
	bool		            m_bListenInvite;			
	bool		            m_bListenPosition;
	TalkDlg*	            pTalkDlg;
	
	
	afx_msg void OnLatticescreen2();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
