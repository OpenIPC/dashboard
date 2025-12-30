// ThermalCameraDlg.h : header file
//

#if !defined(AFX_ThermalCameraDLG_H__71B42769_37E6_4FA9_9549_1128E09D744F__INCLUDED_)
#define AFX_ThermalCameraDLG_H__71B42769_37E6_4FA9_9549_1128E09D744F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CThermalCameraDlg dialog

class CThermalCameraDlg : public CDialog
{
// Construction
public:
	CThermalCameraDlg(CWnd* pParent = NULL);	// standard constructor
public:
    CMenu m_Menu;
    LLONG m_LoginID;
    int m_nChannelCount;
    int m_nChannelNum;
// Dialog Data
	//{{AFX_DATA(CThermalCameraDlg)
	enum { IDD = IDD_ThermalCamera_DIALOG };
	CButton	m_BtnStop;
	CButton	m_BtnPlay;
	CComboBox	m_comboPlayMode;
	CIPAddressCtrl	m_DvrIPAddr;
	CString	m_DvrUserName;
	CString	m_DvrPassword;
	DWORD	m_DvrPort;
	BOOL	m_bOriginal;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThermalCameraDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
public:
    void InitNetSDK();
    //Callback function when device disconnected
	friend void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
    //To get real-time data
	void ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LLONG lParam);
    //Callback monitor data and then save
    friend void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, 
		DWORD dwBufSize, LLONG lParam, LDWORD dwUser);	
    CString GetDvrIP();
    void LastError();
    void ShowLoginErrorReason(int nError);
    
protected:
	HICON m_hIcon;
    LLONG m_DispHanle[2];

	// Generated message map functions
	//{{AFX_MSG(CThermalCameraDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAlarm();
	afx_msg void OnQuery();
	afx_msg void OnPreperty();
	afx_msg void OnGlobal();
	afx_msg void OnStatistcis();
	afx_msg void OnBTNLogin();
	afx_msg void OnBtnLeave();
	afx_msg void OnBUTTONPlay();
	afx_msg void OnButtonStop();
	afx_msg void OnPresetinfo();
	afx_msg void OnPointinfo();
	afx_msg void OnIteminfo();
	afx_msg void OnGetRandomRegionTemper();
	afx_msg void OnRule();
	afx_msg void OnBtnPtzmenu();
	afx_msg void OnHeatmap();
	afx_msg void OnThermalSet();
	afx_msg void OnFireWarnSet();
	afx_msg void OnCalibrate();
	afx_msg void OnPTZ();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    void ServerPlayMode(int iDispNum,int iChannel,HWND hWnd);
	void DirectPlayMode(int iDispNum,int iChannel,HWND hWnd);
    void CloseDispVideo(int iDispNum);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
enum RealPlayMode{
    DirectMode,
        ServerMode,
        MultiMode,
        MultiServerMode,
};
void g_SetWndStaticText(CWnd * pWnd);
CString ConvertString(CString strText, const char* pszSeg  = NULL );
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
//Play mode 
//Direct mode, server mode and multiple-window preview mode 

#endif // !defined(AFX_ThermalCameraDLG_H__71B42769_37E6_4FA9_9549_1128E09D744F__INCLUDED_)
