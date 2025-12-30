#if !defined(AFX_CHANNELCONFIG_H__90F90254_247E_4B7F_93E7_EF497100555A__INCLUDED_)
#define AFX_CHANNELCONFIG_H__90F90254_247E_4B7F_93E7_EF497100555A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChannelConfig.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChannelConfig dialog

class CChannelConfig : public CDialog
{
// Construction
public:
	CChannelConfig(CWnd* pParent = NULL);   // standard constructor

	void SetDevice (DeviceNode *nDev);

	BOOL GetChanPara(int nIndex);

	void EnableChanname(BOOL bEnable);
	void EnableOSD(BOOL bEnable);
	BOOL bSetChannelPara[MAX_CHANNUM];

// Dialog Data
	//{{AFX_DATA(CChannelConfig)
	enum { IDD = IDD_CHILD_CHANNELCFG };
	CComboBox	m_osd_timeformatsel;
	CComboBox	m_osd_distypesel;
	CComboBox	m_prerecordsel;
	CComboBox	m_recorddelaysel;
	CComboBox	m_net_frameratesel;
	CComboBox	m_rec_frameratesel;
	CComboBox	m_channelsel;
	CComboBox	m_copychannelsel;
	CComboBox	m_net_ratectrlsel;
	CComboBox	m_rec_ratectrlsel;
	CComboBox	m_net_ratetypesel;
	CComboBox	m_rec_ratetypesel;
	CComboBox	m_net_streamtypesel;
	CComboBox	m_rec_streamtypesel;
	CComboBox	m_net_imgsizesel;
	CComboBox	m_rec_imgsizesel;
	CComboBox	m_net_imgqualitysel;
	CComboBox	m_rec_imgqualitysel;
	UINT	m_rec_ratectrl;
	UINT	m_net_ratectrl;
	CString	m_channelname;
	BOOL	m_bHide;
	BOOL	m_bHideAlarm;
	BOOL	m_bMotion;
	BOOL	m_bOSD;
	BOOL	m_bChannelName;
	BOOL	m_bTimingRecord;
	BOOL	m_bVLostAlarm;
	UINT	m_channelname_X;
	UINT	m_channelname_Y;
	UINT	m_osd_X;
	UINT	m_osd_Y;
	BOOL	m_bOSDWeek;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChannelConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChannelConfig)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeChannelsel();
	afx_msg void OnApply();
	afx_msg void OnSettimingrecord();
	afx_msg void OnTimingrecordCheck();
	afx_msg void OnHideCheck();
	afx_msg void OnMotionCheck();
	afx_msg void OnHidealarmCheck();
	afx_msg void OnVlostalarmCheck();
	afx_msg void OnOsdCheck();
	afx_msg void OnChannelnameCheck();
	afx_msg void OnSethide();
	afx_msg void OnRestore();
	afx_msg void OnSelchangeRatectrlsel();
	afx_msg void OnSelchangeRatectrlsel2();
	afx_msg void OnSavechannelname();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DeviceNode *m_Device;
	int m_nChanSel;

	NET_DEV_COMPRESSIONCFG m_compressioncfg[MAX_CHANNUM];
	BOOL bGetChannelPara[MAX_CHANNUM];
	BOOL bRemoteRecord[MAX_CHANNUM];

	NET_DEV_PICCFG m_piccfg[MAX_CHANNUM];
	NET_DEV_RECORD	m_record[MAX_CHANNUM];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANNELCONFIG_H__90F90254_247E_4B7F_93E7_EF497100555A__INCLUDED_)
