// IntelligentTrafficDlg.h : header file
//

#if !defined(_ACCESSCONTROLSYSTEM10DLG_)
#define _ACCESSCONTROLSYSTEM10DLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Lock.h"
#include "Card.h"

#define WM_EVENTALARM_INFO (WM_USER + 5)
#define MAX_MSG_SHOW	(100)
#define WM_FINGERPRINT (WM_USER + 6)
/////////////////////////////////////////////////////////////////////////////
// CAccessControlDlg dialog
class CAccessControlDlg : public CDialog
{
// Construction
public:
	CAccessControlDlg(CWnd* pParent = NULL);	// standard constructor
	~CAccessControlDlg();	

// Dialog Data
	//{{AFX_DATA(CAccessControlDlg)
	enum { IDD = IDD_ACCESSCONTROL_DIALOG };
	CIPAddressCtrl	m_ctrlIP;
	CString			m_strUserName;
	int				m_nPort;
	CString			m_strPasswd;	
	CListCtrl		m_listEventAlarm;
	CComboBox		m_cbChannel;
	CMenu			m_Menu;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAccessControlDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	//{{AFX_MSG(CAccessControlDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();		
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	afx_msg LRESULT OnEventAlarmInfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetFingerPrint(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonLogin();
	afx_msg void OnButtonLogout();
	afx_msg void OnButtonStartSubscribeEvent();
	afx_msg void OnButtonStopSubscribeEvent();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGeneralconfigDevinfo();
	afx_msg void OnGeneralconfigNet();
	afx_msg void OnBnClickedButtonOpenDoor();
	afx_msg void OnGeneralconfigDevtime();
	afx_msg void OnBnClickedButtonCloseDoor();
	afx_msg void OnMaintenanceChangepassword();
	afx_msg void OnMaintenanceReboot();
	afx_msg void OnMaintenanceReset();
	afx_msg void OnMaintenanceUpgrade();
	afx_msg void OnMaintenanceAutomatrix();
	afx_msg void OnAccessconfigTimeschedule();
	afx_msg void OnAccessconfigDoorconifg();
	afx_msg void OnAccessconfigOpendoorgroup();
	afx_msg void OnAccessconfigFirstenter();
	afx_msg void OnAccessconfigAblock();
	afx_msg void OnAccessconfigOpendoorroute();
	afx_msg void OnAccessconfigAccesspassword();
	afx_msg void OnQueryrecordDoorrecord();
	afx_msg void OnQueryrecordAlarmrecord();
	afx_msg void OnQueryrecordLog();
	afx_msg void OnUsermanager();
	afx_msg void OnBnClickedButtonOpenAlways();
	afx_msg void OnBnClickedButtonCloseAlways();
	afx_msg void OnBnClickedButtonGetDoorstate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
private:	
	void			ClearEventAlarmUIInfo() ;
	void			InitEventAlarmCtrl();
	void			ChangeUI(int nState);	
	void			InitMenu();
	void			InitCtrls();
	void			InitNetSdk();
	BOOL			StartListenEventAlarm();
	BOOL			StopListenEventAlarm();

	unsigned int	m_nEventAlarmCount;
	//BOOL			m_bSubscribe;

	BOOL			m_bListenEvent;
	BOOL			m_bListenFingerPrint;
public:
	CCard*	        pCardDlg;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
