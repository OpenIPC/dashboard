#if !defined(_TALKDLG_)
#define _TALKDLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxwin.h"

#define DEMO_SDK_WAITTIME 3000
#define WM_EVENTALARM_INVITE (WM_USER + 300)

class TalkDlg : public CDialog
{
	DECLARE_DYNAMIC(TalkDlg)

public:
	TalkDlg(CWnd* pParent = NULL);
	virtual ~TalkDlg();

	enum { IDD = IDD_DIALOG_TALK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonBegintalk();
	afx_msg void OnBnClickedButtonStoptalk();
	void AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag);
	LRESULT InviteEvent(WPARAM wParam, LPARAM lParam);
protected:
	BOOL                   m_bRecordStatus;
	LLONG                  m_hTalkHandle;
	HWND				   hParentWnd;

public:
	afx_msg void OnBnClickedCheckCallevent();
};
#endif