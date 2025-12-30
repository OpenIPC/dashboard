//{{AFX_INCLUDES()
#include "setdeviceconfig.h"
//}}AFX_INCLUDES
#if !defined(AFX_SYSTEMCONFIG_H__ADA7A521_3E2E_4106_BE36_349E583A5B7F__INCLUDED_)
#define AFX_SYSTEMCONFIG_H__ADA7A521_3E2E_4106_BE36_349E583A5B7F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SystemConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSystemConfig dialog

class CSystemConfig : public CDialog
{
// Construction
	LLONG m_deviceID;
public:
	In_DeviceInfo m_di;
	CSystemConfig(CWnd* pParent = NULL);   // standard constructor

	void setDeviceId(LLONG deviceID);
	void ShowDefModal(In_DeviceInfo& di );
// Dialog Data
	//{{AFX_DATA(CSystemConfig)
	enum { IDD = IDD_SYSTEM_CONFIG };
	CSetDeviceConfig	m_ocx_systemset;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSystemConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSystemConfig)
	virtual BOOL OnInitDialog();
	afx_msg void OnOnBlockAskDataSetdeviceconfig1(long DeviceID, long lConfigType, long FAR* lpConfigbuf, long FAR* lpMaxLen, long FAR* lpConfigBufLen, long FAR* lpResult);
	afx_msg void OnOnSaveDeviceDataSetdeviceconfig1(long DeviceID, long lConfigType, long FAR* lpConfigbuf, long lConfigbuflen, long FAR* lpResult);
	afx_msg void OnOnDeviceSetInfoSetdeviceconfig1(long FAR* lpInfoType);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYSTEMCONFIG_H__ADA7A521_3E2E_4106_BE36_349E583A5B7F__INCLUDED_)
