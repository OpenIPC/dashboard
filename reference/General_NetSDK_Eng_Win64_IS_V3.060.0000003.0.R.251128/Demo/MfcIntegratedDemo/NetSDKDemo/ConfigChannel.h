#if !defined(AFX_CONFIGCHANNEL_H__20B8E78B_0C12_4D1C_9538_75858A35D3B5__INCLUDED_)
#define AFX_CONFIGCHANNEL_H__20B8E78B_0C12_4D1C_9538_75858A35D3B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigChannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigChannel dialog

class CConfigChannel : public CDialog
{
// Construction
public:
	CConfigChannel(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigChannel)
	enum { IDD = IDD_CONFIG_CHANNEL };
	CButton	m_gainchk;
	CComboBox	m_channelsel;
	CButton	m_osdshowchk;
	CComboBox	m_resolutionsel;
	CComboBox	m_encodetypesel;
	CComboBox	m_blindtypesel;
	CComboBox	m_audiofmtsel;
	CComboBox	m_osdsel;
	CComboBox	m_imgqualitysel;
	CComboBox	m_framesel;
	CComboBox	m_bitratectrlsel;
	CButton	m_videochk;
	CButton	m_audiochk;
	CComboBox	m_streamsel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigChannel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigChannel)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboChanno();
	afx_msg void OnSelchangeComboStream();
	afx_msg void OnSelchangeComboOsd();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUndoAll();
	afx_msg void OnApply();
	afx_msg void OnDropdownComboChanno();
	afx_msg void OnChangeEditBrightness();
	afx_msg void OnChangeEditContrast();
	afx_msg void OnChangeEditSaturation();
	afx_msg void OnChangeEditHue();
	afx_msg void OnChangeEditGain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetConfig();
	void SetDevice(DeviceNode *pDev);
	
private:
	DeviceNode *m_pDev;
	DHDEV_CHANNEL_CFG m_chnCFG[16];
	DHDEV_SYSTEM_ATTR_CFG m_sysCFG;
	BOOL m_bInited;
	int m_chnIdx;
	int m_strmIdx;
	int m_osdIdx;
	int m_dontsave;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGCHANNEL_H__20B8E78B_0C12_4D1C_9538_75858A35D3B5__INCLUDED_)
