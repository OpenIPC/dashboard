#if !defined(AFX_AUDIOBOARDCASTDLG_H__97FFB2FB_61C7_4239_8876_BDD9FDC68767__INCLUDED_)
#define AFX_AUDIOBOARDCASTDLG_H__97FFB2FB_61C7_4239_8876_BDD9FDC68767__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioBoardcastDlg.h : header file
//

struct AudioDeviceInfo 
{
	LLONG			lLoginID;	// µÇÂ½¾ä±ú
	CString			strIP;		// Éè±¸IP
};

/////////////////////////////////////////////////////////////////////////////
// CAudioBoardcastDlg dialog

class CAudioBoardcastDlg : public CDialog
{
// Construction
public:
	CAudioBoardcastDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAudioBoardcastDlg)
	enum { IDD = IDD_DIALOG_AUDIO_BOARDCAST };
	CListBox	m_lbDevice;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioBoardcastDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
// 	typedef std::map<LLONG, AudioDeviceInfo> AudioDeviceMap;
// 	AudioDeviceMap	m_mapDev;

	// Generated message map functions
	//{{AFX_MSG(CAudioBoardcastDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDel();
	afx_msg void OnBtnStart();
	afx_msg void OnBtnStop();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOBOARDCASTDLG_H__97FFB2FB_61C7_4239_8876_BDD9FDC68767__INCLUDED_)
