#if !defined(AFX_DLGEXALARMCHANNEL_H__3A3EC2C9_23C2_4128_87FA_538B782551A8__INCLUDED_)
#define AFX_DLGEXALARMCHANNEL_H__3A3EC2C9_23C2_4128_87FA_538B782551A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgExAlarmChannel.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmChannel dialog

class CDlgExAlarmChannel : public CDialog
{
// Construction
public:
	CDlgExAlarmChannel(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgExAlarmChannel)
	enum { IDD = IDD_DLG_EXALARMCHANNELS };
	CListCtrl	m_strExAlarmChannelOutList;
	CListCtrl	m_strExAlarmChannelInList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgExAlarmChannel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgExAlarmChannel)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnExalarmchannelsBtnGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	BOOL getInfoFromDevice();
	BOOL getInfo();
	void showInfo();

	LLONG	m_lLoginId;
	NET_EXALARMCHANNELS	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEXALARMCHANNEL_H__3A3EC2C9_23C2_4128_87FA_538B782551A8__INCLUDED_)
