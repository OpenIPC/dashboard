#if !defined(AFX_DLGACCESS_H__C42FC239_7919_4CD8_B97D_04286D02B69F__INCLUDED_)
#define AFX_DLGACCESS_H__C42FC239_7919_4CD8_B97D_04286D02B69F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAccess.h : header file
//
#define MAX_ACCESS_NUM    8

typedef struct
{
	CFG_ACCESS_MODE emAccessMode;
	char*			pszName;
}Demo_AccessMode;

const Demo_AccessMode stuDemoAccessMode[] = 
{
	{ACCESS_MODE_HANDPROTECTED, "HandProtected"},
	{ACCESS_MODE_SAFEROOM, "SafeRoom"},
	{ACCESS_MODE_OTHER, "Other"}
};

typedef struct
{
	CFG_ACCESS_STATE emAccessState;
	char*			pszName;
}Demo_AccessState;

const Demo_AccessState stuDemoAccessState[] = 
{
	{ACCESS_STATE_NORMAL, "Normal"},
	{ACCESS_STATE_CLOSEALWAYS, "CloseAlways"},
	{ACCESS_STATE_OPENALWAYS, "OpenAlways"}
};
/////////////////////////////////////////////////////////////////////////////
// CDlgAccess dialog

class CDlgAccess : public CDialog
{
// Construction
public:
	CDlgAccess(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAccess)
	enum { IDD = IDD_DLG_ACCESS };
	CComboBox	m_cmbEnableMode;
	CComboBox	m_cbChannelId;
	CComboBox	m_cbAccessState;
	CComboBox	m_cbAccessMode;
	CComboBox	m_cbSampleRate;
	CComboBox	m_cbSampleBit;
	CComboBox	m_cbBitRate;
	CComboBox	m_cbAudioFormat;
	CString	m_strClosedoor;
	CString	m_strInUse;
	CString	m_strNotClose;
	CString	m_strNotInUse;
	CString	m_strOpenDoor;
	CString	m_strWaiting;
	CString	m_strAccessChName;
	BOOL	m_bSnapshot;
	int		m_nUnlockHoldTime;
	int		m_nUnlockReloadTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAccess)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAccess)
	afx_msg void OnButtonGetcap();
	afx_msg void OnButtonGetgeneralconfig();
	afx_msg void OnButtonSetgeneralconfig();
	afx_msg void OnButtonGetEventcfg();
	afx_msg void OnButtonSeteventcfg();
	afx_msg void OnSelchangeComboAudioformat();
	afx_msg void OnSelchangeComboChannelid();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SaveChannelInfo(int nChannelId);
	void ShowChannelInfo(int nChannelId);

	void InitAccessEventCfgDlg();
	void AccessEventCfgToDlg();
	void DlgToAccessEventCfg();
	BOOL SetAccessEventCfg();
	BOOL GetAccessEventCfg();

private: 
	LLONG		m_hLogHandle;            // 登陆id
	int			m_nAccessChNum;          // 门禁通道数
	int			m_nPreChannelId;         // 之前选中通道号

	CFG_CAP_SPEAK		m_stSpeakCaps;
	CFG_ACCESS_EVENT_INFO m_stuAccessEvent[MAX_ACCESS_NUM]; // 门禁事件配置信息
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGACCESS_H__C42FC239_7919_4CD8_B97D_04286D02B69F__INCLUDED_)
