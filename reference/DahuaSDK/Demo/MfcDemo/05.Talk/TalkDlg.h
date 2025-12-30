// CTalkDlg.h : header file
//

#if !defined(AFX_TalkDLG_H__9D297D32_70F7_43FB_B811_A7924197BDA8__INCLUDED_)
#define AFX_TalkDLG_H__9D297D32_70F7_43FB_B811_A7924197BDA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTalkDlg dialog

class CTalkDlg : public CDialog
{
// Construction
public:
	CTalkDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTalkDlg)
	enum { IDD = IDD_Talk_DIALOG };
	CComboBox	m_ctlTransmitType;
	CComboBox	m_ctlSpeakerChl;
	CComboBox	m_ctlEncodeType;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTalkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

public:
	void AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag);
	void ShowTalkErrorReason(int nError);

protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTalkDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();	
	afx_msg void OnClose();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();
	afx_msg void OnBtnStarttalk();
	afx_msg void OnBtnStoptalk();	
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeComboTransmitType();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
private:
	BOOL m_bRecordStatus;
	LLONG m_hTalkHandle;
	DHDEV_TALKDECODE_INFO m_curTalkMode;
	void ShowLoginErrorReason(int nError);

	LLONG m_lLoginHandle;
	LONG m_lTalkChnNum;
	LONG m_lDevChnNum;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TalkDLG_H__9D297D32_70F7_43FB_B811_A7924197BDA8__INCLUDED_)
