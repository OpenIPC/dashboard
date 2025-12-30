#if !defined(AFX_SYSTEMCFG_H__BC17C965_B5B7_4D3C_9EC6_529E4E203364__INCLUDED_)
#define AFX_SYSTEMCFG_H__BC17C965_B5B7_4D3C_9EC6_529E4E203364__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SystemCFG.h : header file
//

#include "ServerConfig.h"
#include "ChannelConfig.h"
#include "SerialConfig.h"
#include "AlarmConfig.h"
#include "UserConfig.h"
#include "ATM_Config.h"
#include "ExceptionConfig.h"

/////////////////////////////////////////////////////////////////////////////
// CSystemCFG dialog

class CSystemCFG : public CDialog
{
// Construction
public:
	CSystemCFG(CWnd* pParent = NULL);   // standard constructor

	void SetDevice(DeviceNode *nDev);
	DeviceNode *GetDevice();

// Dialog Data
	//{{AFX_DATA(CSystemCFG)
	enum { IDD = IDD_SYSTEMCFG };
	CTabCtrl	m_config_tab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemCFG)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemCFG)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeConfigTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBtnquit();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CServerConfig m_ServerConfig;
	CChannelConfig m_ChannelConfig;
	CSerialConfig m_SerialConfig;
	CAlarmConfig m_AlarmConfig;
	CUserConfig m_UserConfig;
	CATM_Config m_ATMConfig;
	CExceptionConfig m_ExceptionConfig;

	DeviceNode *m_Device;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEMCFG_H__BC17C965_B5B7_4D3C_9EC6_529E4E203364__INCLUDED_)
