#if !defined(AFX_SERIALCONFIG_H__7BF1BF09_411C_4BBC_B638_F4648AF7BD44__INCLUDED_)
#define AFX_SERIALCONFIG_H__7BF1BF09_411C_4BBC_B638_F4648AF7BD44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SerialConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSerialConfig dialog

class CSerialConfig : public CDialog
{
// Construction
public:
	CSerialConfig(CWnd* pParent = NULL);   // standard constructor
	
	void SetDevice (DeviceNode *nDev);

	void SwitchMode(BOOL bEnable);

	BOOL bSetSerial232;
	BOOL bSetSerial485[MAX_CHANNUM];
// Dialog Data
	//{{AFX_DATA(CSerialConfig)
	enum { IDD = IDD_CHILD_SERIALCFG };
	CComboBox	m_baud485sel;
	CComboBox	m_baud232sel;
	CComboBox	m_stopbit485sel;
	CComboBox	m_stopbit232sel;
	CComboBox	m_PPPmodesel;
	CComboBox	m_operatemodesel;
	CComboBox	m_flow485sel;
	CComboBox	m_flow232sel;
	CComboBox	m_decodetypesel;
	CComboBox	m_dbmodesel;
	CComboBox	m_databit485sel;
	CComboBox	m_databit232sel;
	CComboBox	m_copychlsel;
	CComboBox	m_chlnosel;
	CComboBox	m_checkbit485sel;
	CComboBox	m_checkbit232sel;
	BOOL	m_bDialBack;
	CString	m_decodeIP;
	BOOL	m_bEncrypt;
	CString	m_IPmask;
	CString	m_localIP;
	CString	m_phoneNo;
	CString	m_remoteIP;
	CString	m_username;
	CString	m_userpsw;
	CString	m_verifypsw;
	BYTE	m_decodeAddress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSerialConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSerialConfig)
	virtual BOOL OnInitDialog();
	afx_msg void OnApply232();
	afx_msg void OnSelchangeOperatemodesel();
	afx_msg void OnApply485();
	afx_msg void OnSelchangeChlnosel();
	afx_msg void OnRestore232();
	afx_msg void OnRestore485();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DeviceNode *m_Device;
	DWORD dwReturned;

	int m_nChanSel;
	int m_nChanNum;
	int m_nCopyChanNum;
	BOOL bGet232Serial;
	BOOL bGet485Serial[MAX_CHANNUM];	

	NET_DEV_DECODERCFG	decodercfg[MAX_CHANNUM];		//485串口参数
	NET_DEV_RS232CFG	rs232cfg;		//232串口参数设置
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERIALCONFIG_H__7BF1BF09_411C_4BBC_B638_F4648AF7BD44__INCLUDED_)
