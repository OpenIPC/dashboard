// ShowDemoDlg.h : header file
//

#if !defined(AFX_SHOWDEMODLG_H__F3A6015F_7DC8_4AF7_886E_DAF57908FDFD__INCLUDED_)
#define AFX_SHOWDEMODLG_H__F3A6015F_7DC8_4AF7_886E_DAF57908FDFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CShowDemoDlg dialog

class CShowDemoDlg : public CDialog
{
// Construction
public:
	CShowDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CShowDemoDlg)
	enum { IDD = IDD_SHOWDEMO_DIALOG };
	CStatic	m_BitRate;
	CEdit	m_EditBitRate;
	CEdit	m_EditSnapFrameRate;
	CButton	m_VideoEnable;
	CComboBox	m_Cmb_Pack;
	CComboBox	m_Cmb_FreQuency;
	CComboBox	m_Cmb_Depth;
	CComboBox	m_Cmb_AudioMode;
	CButton	m_AudioEnable;
	CButton	m_SnapEnable;
	CComboBox	m_Cmb_SnapType;
	CComboBox	m_Cmb_SnapQuality;
	CComboBox	m_Cmb_SnapFrameRate;
	CComboBox	m_Cmb_SnapCompression;
	CComboBox	m_Cmb_SnapResolution;
	CComboBox	m_Cmb_AudioFormat;
	CComboBox	m_Cmb_BitRate;
	CComboBox	m_Cmb_FrameType;
	CComboBox	m_Cmb_Resolution;
	CComboBox	m_Cmb_FrameRate;
	CStatic	m_ImageQuality;
	CComboBox	m_Cmb_ImageQuality;
	CComboBox	m_Cmb_Compression;
	CComboBox	m_Cmb_BitRate_Ctl;
	CComboBox	m_Cmb_Format;
	CComboBox	m_Cmb_ChannelCount;
	CIPAddressCtrl	m_DvrIPAddr;
	CString	m_DvrUserName;
	CString	m_DvrPassword;
	int		m_DvrPort;
	int		m_nPicTime;
	int		m_nPicHour;
	int		m_nTrigPic;
	int		m_nPacketReriod;
	int		m_nInterval;
	int		m_nSvc;
	int		m_nSnapFrameRate;
	int		m_nEditBitRate;
	int		m_nMinFps;
	int		m_nMaxFps;
	//}}AFX_DATA

    LLONG m_LoginID;
    LLONG m_DispHanle;
    int m_nChannel;             //当前正在播放的通道号
    NET_OUT_ENCODE_CFG_CAPS m_stuCaps;
    CFG_ENCODE_INFO m_stuEncodeInfo;
    NET_ENCODE_SNAP_INFO m_stuSnapInfo;
    NET_ENCODE_SNAP_TIME_INFO m_stuSnapTimeInfo;
    NET_ENCODE_AUDIO_INFO m_stuEncodeAudioInfo;
    NET_ENCODE_AUDIO_COMPRESSION_INFO m_stuEncodeAudioCompression;
    NET_ENCODE_VIDEO_INFO m_stuEncodeVideoInfo;
    NET_ENCODE_VIDEO_PROFILE_INFO m_stuVideoProFile;
    NET_ENCODE_VIDEO_SVC_INFO m_stuVideoSVC;
    NET_ENCODE_VIDEO_PACK_INFO m_stuPack;

    CString GetDvrIP();
    void AddChannelCount(int nChannelCount);
    void ShowLoginErrorReason(int nError);
    void InitEncodeCtrl(NET_OUT_ENCODE_CFG_CAPS& stuCaps,int nStreamtype =0);
    void InitSnapCtrl(NET_OUT_ENCODE_CFG_CAPS& stuCaps,int nStreamtype =0);
    void GetEncodeCaps(int nChannel = 0, bool bFirst = false, int streamType = 0, CFG_VIDEO_COMPRESSION cmp = VIDEO_FORMAT_MPEG4, CFG_H264_PROFILE_RANK profile = PROFILE_BASELINE, int nWidth = 0, int nHeight = 0, int FPS = 1);
    void InitDlg();
    void OnSnapStuToDlg();
    void OnSnapDlgToStu();
    BOOL GetSnapConfig(NET_EM_SNAP_TYPE emType);
    BOOL SetSnapConfig();

    BOOL GetAudioConfig(NET_EM_FORMAT_TYPE emType);
    BOOL SetAudioConfig();
    void OnAudioStuToDlg();
    void OnAudioDlgToStu();
    
    BOOL GetVideoConfig(NET_EM_FORMAT_TYPE emType);
    BOOL SetVideoConfig();
    void OnVideoStuToDlg();
    void OnVideoDlgToStu();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowDemoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    void ResolutionSizetoInt(int nResolution, int *nWidth, int *nHeight);
	int ResolutionInttoSize(int nWidth, int nHeight);
    int AudioDepthToMasK(int nAudioDepth);
    void ResolutionSizeToInt(int nSize, int& nWidth, int& nHeight);
    void updateEncodeCaps();
	// Generated message map functions
	//{{AFX_MSG(CShowDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnLogout();
	afx_msg void OnSelchangeChannel();
	afx_msg void OnSelchangeCmbBitrateCtl();
	afx_msg void OnSelchangeCmbFormat();
	afx_msg void OnSelchangeCmbSnaptype();
	afx_msg void OnSetsnap();
	afx_msg void OnSetaudio();
	afx_msg void OnSetvideo();
	afx_msg void OnClose();
	afx_msg void OnSelchangeCmbResolution();
	afx_msg void OnSelchangeCmbCompression();
	afx_msg void OnSelchangeCmbFramerate();
	afx_msg void OnSelchangeCmbSnapFramerate();
	afx_msg void OnSelchangeCmbBitrate();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
};
CString ConvertString(CString strText);
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWDEMODLG_H__F3A6015F_7DC8_4AF7_886E_DAF57908FDFD__INCLUDED_)
