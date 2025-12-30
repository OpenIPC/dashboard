#if !defined(AFX_AUDIOSOURCE_H__16169EC8_29F5_412B_9BCA_BAC89B282FA9__INCLUDED_)
#define AFX_AUDIOSOURCE_H__16169EC8_29F5_412B_9BCA_BAC89B282FA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioSource.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAudioSource dialog

class CAudioSource : public CDialog
{
// Construction
public:
	CAudioSource(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAudioSource)
	enum { IDD = IDD_AUDIOSOURCE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
    int m_nChannel;
    LLONG m_LoginID;
    NET_ENCODE_AUDIO_SOURCE_INFO    m_stuAudioSource;
    BOOL GetVideoIn();
    BOOL SetVideoIn();
    void StuToDlg();
    void DlgToStu();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioSource)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAudioSource)
	virtual BOOL OnInitDialog();
	afx_msg void OnSet();
	afx_msg void OnGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOSOURCE_H__16169EC8_29F5_412B_9BCA_BAC89B282FA9__INCLUDED_)
