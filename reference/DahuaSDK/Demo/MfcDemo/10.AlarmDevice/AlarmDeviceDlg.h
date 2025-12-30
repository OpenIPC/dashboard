// AlarmDeviceDlg.h : header file
//

#if !defined(AFX_CLIENTDEMODLG_H__475AE43D_B618_4F15_9DE9_2628AA826F52__INCLUDED_)
#define AFX_CLIENTDEMODLG_H__475AE43D_B618_4F15_9DE9_2628AA826F52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VideoNodeInfo.h"
#include "ExButton.h"
#include "ptzScreen.h"
#include <afxmt.h>

#define WM_DEVICE_DISCONNECTED	(WM_USER+200)
/////////////////////////////////////////////////////////////////////////////
// CClientDemoDlg dialog

class CAlarmDeviceDlg : public CDialog
{
// Construction
public:
	
	//PTZ extensive control function 
	void PtzExtControl(DWORD dwCommand, DWORD dwParam = 0);
	//PTZ control function 
	void PtzControl(int type, BOOL stop);
	//Log in handle 
	LLONG m_LoginID;
	//Display function execution error reason.
	void LastError();
	//To get real-time data
	void ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, LDWORD dwBufSize, LLONG lParam);
	//Callback function when device disconnected
	friend void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);
	//Callback monitor data and then save
	friend void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, 
		DWORD dwBufSize, LLONG lParam, LDWORD dwUser);
	
	friend void CALLBACK  MessageProcFunc(int nWndID, UINT message,	LDWORD dwUser);
	friend BOOL CALLBACK  GetParamsFunc(int nWndID, int type, LDWORD dwUser);
	friend void CALLBACK SetParamsFunc(int nWndID, int type, LDWORD dwUser);
	friend void CALLBACK RectEventFunc(RECT WinRect,CPoint &pointStart,CPoint &pointEnd,LDWORD dwUser);
	CAlarmDeviceDlg(CWnd* pParent = NULL);	// standard constructor

	
// Dialog Data
	//{{AFX_DATA(CClientDemoDlg)
	enum { IDD = IDD_CLIENTDEMO_DIALOG };
	CComboBox	m_cmbControlQueryType;
	CComboBox	m_cmbConfig;
	CComboBox	m_comboPlayMode;
	CComboBox	m_auxNosel;
	CComboBox	m_comboPTZData;
	CExButton	m_iris_open;
	CExButton	m_iris_close;
	CExButton	m_focus_far;
	CExButton	m_focus_near;
	CExButton	m_zoom_tele;
	CExButton	m_zoom_wide;
	CExButton	m_ptz_rightdown;
	CExButton	m_ptz_rightup;
	CExButton	m_ptz_leftdown;
	CExButton	m_ptz_leftup;
	CExButton	m_ptz_right;
	CExButton	m_ptz_left;
	CExButton	m_ptz_down;
	CExButton	m_ptz_up;
	CComboBox	m_comboChannel;
	CComboBox	m_comboDispNum;
	CIPAddressCtrl	m_DvrIPAddr;
	CString	m_DvrUserName;
	CString	m_DvrPassword;
	DWORD	m_DvrPort;
	int		m_presetData;  // [0,100]
	int		m_crviseGroup;
	int		m_moveNo;
	int		m_posX;
	int		m_posY;
	int		m_posZoom;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientDemoDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CClientDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBTLogin();
	afx_msg void OnBUTTONPlay();
	afx_msg void OnBTLeave();
	afx_msg void OnDestroy();
	afx_msg void OnButtonStop();
	afx_msg void OnBtnPtzexctrl();
	afx_msg void OnPresetSet();
	afx_msg void OnPresetAdd();
	afx_msg void OnPresetDele();
	afx_msg void OnStartCruise();
	afx_msg void OnCruiseAddPoint();
	afx_msg void OnCruiseDelPoint();
	afx_msg void OnCruiseDelGroup();
	afx_msg void OnModeSetBegin();
	afx_msg void OnModeStart();
	afx_msg void OnModeSetDelete();
	afx_msg void OnLineSetLeft();
	afx_msg void OnLineSetRight();
	afx_msg void OnLineStart();
	afx_msg void OnFastGo();
	afx_msg void OnExactGo();
	afx_msg void OnResetZero();
	afx_msg void OnRotateStart();
	afx_msg void OnRotateStop();
	afx_msg void OnAuxOpen();
	afx_msg void OnAuxClose();
	afx_msg void OnBtnPtzmenu();
	afx_msg void OnSelchangeCOMBODispNum();
	afx_msg void OnCloseupCOMBODispNum();
	afx_msg void OnBtnConfig();
	afx_msg void OnBtnCap();
	afx_msg void OnBtnSubscribe();
	afx_msg void OnBtnControlQuery();
	afx_msg void OnBtnTalk();	
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void MultiPlayServerMode(int iDispNum, HWND hWnd);
	void ShowLoginErrorReason(int nError);
	void ServerPlayMode(int iDispNum,int iChannel,HWND hWnd);
	void DirectPlayMode(int iDispNum,int iChannel,HWND hWnd);
	void MultiPlayMode(int iDispNum,HWND hWnd);
	void StopPlayForMultiMode(int iDispNum);
	void StopPlayForServerMode(int iDispNum);
	void StopPlayForDirectMode(int iDispNum);
	void StopPlayForMultiServer(int iDispNum);
	
	void InitNetSDK();
	void InitPTZControl();
	void InitComboBox();
	HWND GetDispHandle(int nNum);
	CString GetDvrIP();
	void IsValid();
	long GetStreamPort(LLONG lRealHandle);
	void SetPlayVideoInfo(int iDispNum,int iChannel,enum RealPlayMode ePlayMode);
	void CloseDispVideo(int iDispNum);

	void UpdataScreenPos(void);
	void MessageProc(int nWndID, UINT message);
	BOOL GetParams(int nWndID, int type);
	void SetParams(int nWndID, int type);
	BOOL GetExitDecode(int nCurWndID);
	BOOL GetExitCycle(int nCurWndID);
	void SetExitDecode(int nCurWndID);
	void SetExitCycle(int nCurWndID);

	UINT32 GetAnalogAlarmIn();
	
	//Device channel amount 
	int m_nChannelCount;
	CRect m_rectSmall;
	CRect m_rectLarge;
	//9-window control information 
	CVideoNodeInfo m_videoNodeInfo[9];
	LLONG m_DispHanle[9];

	CPtzScreen m_ptzScreen;
	CRect			m_screenRect;
	CRect			m_clientRect;
	int				m_CurScreen;
	BOOL			m_bWndExitDecode[16];
	BOOL			m_bWndExitCycle[16];
	CCriticalSection m_cs;
	CCriticalSection m_csPos;

private:
	void AlarmCfg();
	void AlarmChassisIntrusion();
	void AlarmExAlarmBox();
	void AlarmExAlarmChannel();
	void AlarmExAlarmInput();
	void AlarmExAlarmOutput();
	void AlarmKeyboard();
	void AlarmSubSystemCfg();
	void AlarmOutCfg();
	void AlarmPowerFault();
	void BatteryLowPower();
	void CommGlobalCfg();
	void Locales();
	void Mobile();
	void NetworkCfg();
	void NTP();
	void PhoneEventNotify();
	void PSTN_Alarm_Server();
	void SerialCfg();
	void Urgency();
	void VW_SensorInfo();
	void WirelessCfg();
    void SnapLinkage();
    void AirConditionCfg();
    void NetAbortCfg();
    void IPConflictCfg();
    void MacConflictCfg();
    void PSTNBreakLineCfg();
	void NetCollectionCfg();
	void AlarmSlotBondCfg();
    void RCEmergencyCallCfg();
    void AlarmBellCfg();
    void PSTNPersonServerCfg();
    void ArmLinkCfg();

	void AccessManager();
	void AlarmBell();
	void AlarmChannelState();
	void PlaySound();
	void FileManagerCfg();
	void FileName();
	void Log();
	void ModifyPsw();
	void OpenAccess();
	void PowerState();
    void RestoreCfg();
	void Version();
	void UserList();
	void Upgrade();
	void SetBypassMode();
	void DeviceTime();
// 	void AttachCfgChange();

	void AlarmCenterCfg();
	void ActivatedDefenceArea();
	void AutoMainTain();
	void SetNetworkParam();
	void SetArmMode();
	void UserManager();
	
	void SensorSamplingCfg();
	void STPCfg();
	void GetAnalogAlarmChannels();
	void AnalogAlarmCfg();
	void AnalogAlarmData();
	void AlarmChannels();
	void AlarmSubSystem();
    void AlarmOutSet();
    void AirConditionOperate();
	void GetLowRateWPAN();

private:

	// 本地报警输入通道数
	UINT32			m_uiAlarmIn;
	// 本地报警输出通道数
	UINT32			m_uiAlarmOut;
	// 本地模拟量报警通道
	UINT32			m_uiAnalogAlarmIn;
	// 设备类型
	NET_DEVICE_TYPE	m_emDeviceType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTDEMODLG_H__475AE43D_B618_4F15_9DE9_2628AA826F52__INCLUDED_)
