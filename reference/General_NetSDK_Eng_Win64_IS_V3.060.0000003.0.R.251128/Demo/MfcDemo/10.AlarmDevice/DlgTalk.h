#if !defined(AFX_DLGTALK_H__4BD8DAD0_62D4_466B_83EF_EB8A0CD5D604__INCLUDED_)
#define AFX_DLGTALK_H__4BD8DAD0_62D4_466B_83EF_EB8A0CD5D604__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTalk.h : header file
//

#include <string>
#include "FlashLabel.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgTalk dialog
#define RECEIVER_AUDIO_PORT 50002

class CDlgTalk : public CDialog
{
// Construction
public:
	CDlgTalk(CWnd* pParent = NULL, LLONG lLoginId = NULL, NET_DEVICE_TYPE emType = NET_AF6016, int nChannelNum = 1);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgTalk)
	enum { IDD = IDD_DLG_TALK };
	CFlashLabel	m_wndInviteFlag;
	CComboBox	m_cbChannelId;
	CComboBox	m_cbAudioFormate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTalk)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgTalk)
	afx_msg void OnButtonTalkStarttalk();
	afx_msg void OnButtonTalkStoptalk();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBtnBroadcast();
	afx_msg void OnCheckTalkAttachEvent();
	afx_msg void OnBtnTalkRefuse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	NET_DEVICE_TYPE	m_emDeviceType;						// 设备类型
	LLONG			m_lLoginID;							// 设备登陆句柄
	DHDEV_TALKFORMAT_LIST m_stuAudioEncodeFormate;		// 设备语音编码格式
	int				m_nChannelNum;						// 设备视频通道个数
	LLONG			m_hTalkingHandle;					// 语音对讲句柄
	BOOL			m_bAnswered;						// 设备是否已经响应对讲

private:
	std::string ConvertAudioTypeToString( DH_TALK_CODING_TYPE emType);
	DH_TALK_CODING_TYPE ConvertStringToAudioType( const std::string& strType);

public:
	BOOL		GetAnswerState() { return m_bAnswered; }
	void		SetAnswerState(AV_BOOL bAnswered) { m_bAnswered = bAnswered; }
	void		InitDlg(LLONG hLogHandle, AV_int32 nChannelNum);
	
	void AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag);
    void OnEventNotify(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort);

	friend void __stdcall AudioDataCallBack(LLONG hTalkHandle,  AV_BYTE byDataType, AV_BYTE *pBuf, AV_int32 nBufSize, void* pReserved, void* pUserParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTALK_H__4BD8DAD0_62D4_466B_83EF_EB8A0CD5D604__INCLUDED_)
