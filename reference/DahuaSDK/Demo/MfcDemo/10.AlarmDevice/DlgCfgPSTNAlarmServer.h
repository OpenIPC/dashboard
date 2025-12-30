#if !defined(AFX_DLGCFGPSTNALARMSERVER_H__D970B711_AFCD_4B04_91DA_16B015A0802A__INCLUDED_)
#define AFX_DLGCFGPSTNALARMSERVER_H__D970B711_AFCD_4B04_91DA_16B015A0802A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CDlgCfgPSTNAlarmServer.h : header file
//

typedef struct  
{
	CFG_EM_SIGNALTRANSMIT_MODE emMode;
	char* pszMode;
}Demo_PSTN_Mode;

const Demo_PSTN_Mode DemoMode[] = 
{
	{CFG_EM_SIGNALTRANSMIT_ERROR, "UnKnowMode"},
	{CFG_EM_SIGNALTRANSMIT_DTMF_5S, "DTMF 5/S"},
	{CFG_EM_SIGNALTRANSMIT_DTMF_10S, "DTMF 10/S"},
}; 

typedef struct
{
	CFG_EM_PSTN_PROTOCOL_TYPE emType;
	char* pszType;
}Demo_PSTN_Type;

const Demo_PSTN_Type DemoType[] =
{	
	{CFG_EM_PSTN_PROTOCOL_ERROR, "UnKnowType"},
	{CFG_EM_PSTN_PROTOCOL_CID, "CID"},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPSTNAlarmServer dialog

class CDlgCfgPSTNAlarmServer : public CDialog
{
	// Construction
public:
	CDlgCfgPSTNAlarmServer(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor
	
	// Dialog Data
	//{{AFX_DATA(CDlgCfgPSTNAlarmServer)
	enum { IDD = IDD_DLG_CFG_PSTN_ALARM_SERVER };
	CComboBox	m_cmbServer;
	CComboBox	m_cmbProtocol;
	CEdit	m_edNumber;
	CEdit	m_edName;
	CEdit	m_edAccount;
	CComboBox	m_cmbMode;
	CButton	m_chkEnable;
	UINT	m_nDelay;
	UINT	m_nTryCount;
	//}}AFX_DATA
	
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgPSTNAlarmServer)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CDlgCfgPSTNAlarmServer)
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeComboServer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	BOOL getCfgFromDevice();
	BOOL setCfgToDevice();
	void ShowDlgInfo(int nSerNum);
	BOOL getDlgInfo();
	void getPSTNAbility();
private:
	
	LLONG						m_lLoginId;
	CFG_PSTN_ALARM_CENTER_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGPSTNALARMSERVER_H__D970B711_AFCD_4B04_91DA_16B015A0802A__INCLUDED_)
