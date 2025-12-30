// NetSDKDemoDlg.h : header file
//

#if !defined(AFX_NETSDKDEMODLG_H__874C22A9_455C_4113_9954_FDC1528EE15D__INCLUDED_)
#define AFX_NETSDKDEMODLG_H__874C22A9_455C_4113_9954_FDC1528EE15D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "searchrecord.h"
#include "clientState.h"
#include "exbutton.h"
#include "systemconfig.h"
#include "PlayWnd.h"
#include "ScreenPannel.h"
#include "PTZPannel.h"
#include "NormalBtnPannel.h"
#include "AdvanceBtnPannel.h"
#include "SelectPannel.h"
#include "RuntimeMsgPannel.h"
#include "AudioBroadcastDlg.h"
#include "downloadbytime.h"
/////////////////////////////////////////////////////////////////////////////
// CNetSDKDemoDlg dialog

#define TIMER_KBPS 1999
#define CUR_MAXCHAN 16
#define	CUR_SPLIT	SPLIT16

//画面分割类型
enum{
	SPLIT1 = 0,
	SPLIT4,
	SPLIT9,
	SPLIT16,	
	SPLIT_TOTAL
};

//播放控制类型
enum{
	STATUS_STOP,
	STATUS_PAUSE,
	STATUS_PLAY,
	STATUS_STEP,
};

//设置视频参数
enum{
	VIDEO_BRIGHT = 0,
	VIDEO_CONTRAST,
	VIDEO_HUE,
	VIDEO_SATURATION,
	VIDEO_TOTAL
};

//云台控制操作
enum{
	PTZ_START,
	PTZ_STOP
};
//预览信息参数
typedef struct _SplitMonitorParam
{
	DeviceNode *pDevice;  //设备指针
	int iChannel;   //对应的设备中的通道序号
//	BOOL  isTalkOpen;  //是否打开语音对讲
}SplitMonitorParam;

//网络回放信息参数
typedef struct _SplitNetPlayParam
{
//	DWORD iFilePos;  //文件信息列表position/
	FileInfoNode *pFileInfo;   //文件信息指针
	int   iStatus;  //播放状态 正常/暂停/快放/慢放
	int   iPlayPos;  //当前播放的位置
}SplitNetPlayParam;

//通过时间回放录象参数
typedef struct _SplitPBByTimeParam
{
	DeviceNode *pDevice;
	NET_TIME starttime;
	NET_TIME endtime;
	int nChannel;
	int npos;
	int iStatus;
}SplitPBByTimeParam;

//多画面预览信息参数
typedef struct _SplitMulitPlayParam
{
	DeviceNode *pDevice;  //设备指针
}SplitMultiPlayParam;

//轮循预览信息参数
typedef struct _CycleChannelInfo
{
	DWORD dwDeviceID;
	int iChannel;
}CycleChannelInfo;
typedef struct _SplitCycleParam
{
	DeviceNode *pDevice;	//设备指针
	CPtrList *pChannelList;	//轮循通道列表
	int iChannelCount;		//通道数
	int iCurPosition;		//当前预览通道在数组里的位置
	UINT iInterval;			//画面切换间隔时间（秒）
}SplitCycleParam;	
/*Begin: Add by yehao(10857) For Task.NO.11071 2006-12-26*/
typedef struct _TalkHandleInfo
{
	DeviceNode *pDevice;
	LLONG		lHandle;
} TalkHandleInfo, *LPTalkHandleInfo;
/*End: yehao(10857) Task.NO.11071 */

class CNetSDKDemoDlg : public CDialog
{
// Construction
	int m_connectwaittime;  //设置连接等待时间
	int m_curScreen;   //当前显示画面序号
	int m_curSoundSplit;  //当前声音输出所在的画面序号

	//内部根据设计需要的子功能函数
	void *GetCurDeviceInfo();  //获得当前设备列表选择的设备的信息指针
	void *FindDeviceInfo(LLONG lLoginID, char *sDVRIP,LONG nDVRPort); //根据设备id获得设备信息指针
	int GetHandleSplit(LLONG lPlayHandle);   //根据id,获得当前画面序号
	BOOL CheckCurSplitAndClose();  //检查当前画面并关闭图像显示;
	BOOL IsCycling(DWORD deviceID, int channelNo);   //是否在轮训通道
	int  GetCurSplitStart(int nScreen, int nSplit);   //获取当前画面分割模式的指定画面的起始画面序号
	int  PlayStop(int iScreen, BOOL bDis = FALSE);    //播放停止

	//各类更新操作函数
	void UpdateVideoCtrl(int nMode);      //刷新视频参数控制区
	void UpdateDeviceList();     //刷新设备列表
	void UpdateDeviceState(void *pDevice, char *pBuf, DWORD dwBufLen); //更新设备的状态
	
public:
	CNetSDKDemoDlg(CWnd* pParent = NULL);	// standard constructor


	//用于回调的接口	
	void ReceiveRealData(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, LLONG lParam = 0);
	BOOL ReceiveMessage(LLONG lLoginID, LONG lCommand, char *pchDVRIP, LONG nDVRPort,char *pBuf, DWORD dwBufLen);
	void AddDisplayInfo(LLONG lLoginID, LLONG lPlayHandle, HDC hDC);  //叠加字符或图片
	void ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize);	
	//用于删除轮循画面的参数
	void DeleteCycleParam(int nScreen);

	//设备断开时回调函数，可以用来处理断开后设备列表的更新及设备的删除操作
	friend void CALLBACK DisConnectFunc(LLONG lLoginID, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser);	
	//回放数据回调函数，demo里将数据保存成文件
	friend int CALLBACK PBDataCallBack(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, 
		DWORD dwBufSize, LDWORD dwUser);
	friend void CALLBACK RealDataCallBackEx(LLONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, 
		DWORD dwBufSize, LLONG lParam, LDWORD dwUser);
	//消息回调处理函数,是对整个sdk应用的回调,用来处理回调出来的报警信息
	friend BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf,DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort);
	//自定义画板回调,可以用来显示通道名称
	friend void CALLBACK ChannelAutoDraw(LLONG lLoginID, LLONG lPlayHandle, HDC hDC, LDWORD dwUser);
		//播放进度状态回调函数,用来处理进度条显示
	friend void CALLBACK PlayCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser);

	//报警状态页面
	CClientState  m_ClientStateDlg;
	
//	CSystemConfig m_SysConfig;
	// Dialog Data
	//{{AFX_DATA(CNetSDKDemoDlg)
	enum { IDD = IDD_NETSDKDEMO_DIALOG };
	CExButton	m_ptz_righttop;
	CExButton	m_ptz_rightdown;
	CExButton	m_ptz_lefttop;
	CExButton	m_ptz_leftdown;
	CExButton	m_zoom_wide;
	CExButton	m_zoom_tele;
	CExButton	m_focus_near;
	CExButton	m_focus_far;
	CExButton	m_iris_open;
	CExButton	m_iris_close;
	CExButton	m_ptz_up;
	CExButton	m_ptz_right;
	CExButton	m_ptz_left;
	CExButton	m_ptz_down;
	CComboBox	m_channelsel;
	CTreeCtrl	m_devicelist;
	int		m_play_frame;
	UINT	m_testvalue;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetSDKDemoDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNetSDKDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDblclkTreeDevicelist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void OpenSingleChannel(DeviceNode *pInfo,int nCh, int screenNo, DH_RealPlayType subtype);	//打开一个预览通道
	void OpenAllChannel(DeviceNode *pInfo, DH_RealPlayType subtype);					//打开选中设备的所有预览通道
	void PlayRecordFile(FileInfoNode* playfile, int scrNo);	//打开一个录象文件的播放
	BOOL ProcessCloseScreen(int scrNo, BOOL bDis = FALSE);  //执行关闭画面
	BOOL ProcessCloseAllChannel(DeviceNode *pInfo);			//执行关闭选中设备的所有通道
	void SwitchFullScreen();		//切换全屏 
//	void ReturnOrignalSplit(void);	//恢复正常画面分割
	void UpdateScreen(int nSplit);	//刷新屏幕,主要是刷新视频区域

	void AddDevice();				//对“添加设备”按键的响应函数 
	void DeleteDevice();			//对“删除设备”按键的响应函数
	void OpenChannel();				//对“打开通道”按键的响应函数
	void OpenMultiplay();			//对“多画面预览”按键的响应函数
	void CloseScreen();				//对“关闭画面”按键的响应函数
	void CloseAllChannel();			//对“关闭设备通道”按键的响应函数
	void SearchRecord();			//对“录象查询”按键的响应函数
	void CycleMonitor();			//对“轮循”按键的响应函数
	void Playbackbytime();			//对“按时间回放”按键的响应函数
	void Downloadbytime();			//对“按时间下载”按键的响应函数
	void Sysconfig();				//对“系统配置”按键的响应函数
	void Sysconfig2();				//对“系统配置<X>”按键的响应函数

	BOOL OpenSound(BOOL bOpen);		//打开\关闭音频
	BOOL OpenTalk(BOOL bOpen);		//打开\关闭对讲
	void UpdateDevice();			//对“设备升级”按键的响应函数
	void Recordstate();				//对“录象状态”按键的响应函数
	void RebootDevice();			//对“重启设备”按键的响应函数
	void AlarmIOctrl();				//对“报警IO控制”按键的响应函数
	void ShutdownDevice();			//对“关闭设备”按键的响应函数
	void DeviceState();				//对“设备状态”按键的响应函数
	void CaptureScreen();			//对“抓图”按键的响应函数
	void SetIframe();				//对“强制I帧”按键的响应函数
	void ShowFluxInfo();			//对“显示码流”按键的响应函数
	void Transcom();				//对“透明通道”按键的响应函数
	void DDNS_QueryIP();				//对“DDNS查询IP”按键的响应函数
	void Getversion();				//对“SDK版本”按键的响应函数
	void OpenAudioBroadcastDlg();	//打开语音广播的操作界面
	void ptzctrl();					//对“云台菜单”按键的响应函数
	void Extptzctrl();				//对“云台扩展菜单”按键的响应函数

	void SaveRealdata(int nCheck);			//保存实时预览数据
	void SavecbdataRaw(int nCheck);				//保存回调数据－原始数据
	void SavecbdataStd(int nCheck);			//保存回调数据－标准数据
	void Savecbdatapcm(int nCheck);			//保存回调数据－pcm音频数据
	void Savecbdatayuv(int nCheck);			//保存回调数据－yuv视频数据
				
	void PtzControl(int type, BOOL stop, int param);   //云台控制操作
	void ProcessDeleteDevice(DeviceNode *pDevice,  BOOL bDelList, BOOL bDis = false); //删除设备,nDelList是否删除设备列表, bDis表示是否继线
	
	void NextCycleMonitor(UINT nID);	//打开轮循列表中下一个通道

	void CtrlColor_Bright(int pos);			//控制画面亮度
	void CtrlColor_Contrast(int pos);		//控制画面对比度
	void CtrlColor_Saturation(int pos);		//控制画面饱和度
	void CtrlColor_Hue(int pos);			//控制画面色度

	BOOL SeekPlayPositon(int nPos);		//播放进度条的拖动响应函数
	BOOL PlayCtrl_Play();				//"播放"\"暂停"按建响应函数
	BOOL PlayCtrl_Stop();				//"停止"按键响应函数
	BOOL PlayCtrl_Fast();				//"快放"按键响应函数
	BOOL PlayCtrl_Slow();				//"慢放"按键响应函数
	BOOL PlayCtrl_Step();				//"单帧"按键响应函数
	BOOL PlayCtrl_Frame(int frame);		//"按帧"按键响应函数

	void GetDeviceWorkstate();			//"设备工作状态"按键响应函数
	void DeviceControldisk();			//"硬盘控制"按键响应函数
	void DeviceUserinfo();				//"用户信息"按键响应函数
	
	void SeleteNormalPannel();			//显示“常规”功能页面
	void SeleteAdvancePannel();			//显示“高级”功能页面
	void SeleteSaveDataPannel();		//显示“数据保存”功能页面
	void SeletePTZPannel();				//显示“云台控制”功能页面
	void SeleteColorPannel();			//显示“颜色调整”功能页面
	void SeletePlayCtrlPannel();		//显示“播放控制”功能页面
	void SeleteDevListPannel();			//显示“设备列表”功能页面
	
	void UpdatePannelPosition();			//更新各子窗口位置

	void LastError();						//显示函数执行出错原因

	void SwitchMultiWnd(int nSplit);

	int	 GetCurWinID(void);

	void SetCurWindId(int nCuWinID)
	{
		m_curScreen = nCuWinID;
	}

	int UpdateDeviceListCallback_Imp(const DeviceNode& node);
	int WriteAlarmLogFunc_Imp(const AlarmNode& node);

	void TestProc(); //for test

	//for stream parser
//	void HandleParserMsg(PARSERHANDLE hHandle, PARSERMSG msg, 
//		unsigned long dwParam1, unsigned long dwParam2);

public:
	int GetCurScreen(){return m_curScreen;}
	void UpdateCurScreenInfo();  //刷新当前画面的相关信息显示
	void SetSplit(int split);
	int  GetSplit();
	//获取画面窗口信息
	BOOL GetSplitInfo_Main(int nIndex, SplitInfoNode* info);
	BOOL SetSplitInfo_Main(int nIndex, SplitInfoNode* info);
	/*Begin: Add by yehao(10857) 2006-12-20*/
	static void CALLBACK AudioDataCallBack(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag, LDWORD dwUser);
	LLONG GetTalkHandle();
	/*End: yehao(10857) */
private:
	/*Begin: Add by yehao(10857) For Task.NO.11071 2006-12-26*/
	BOOL RecordStart();
	BOOL RecordStop();
	/*End: yehao(10857) Task.NO.11071 */
private:
	BOOL		m_bPTZCtrl;		//标识PTZ控制状态
	
//	BOOL		m_bFullSCRN;		//标识当前是否全屏状态
	CRect		m_clientRect;		//程序界面区域位置
	CRect		m_screenRect;		//储存屏幕区域的位置
	CRect		m_pannelRect;		//储存功能面板区域的位置
	CRect		m_selectRect;		//储存“选择页面”面板的位置
	CRect		m_btnAreaRect;		//储存功能按键区域的位置
	CRect		m_runtimeMsgRect;			//储存“运行时信息”面板的位置

	CScreenPannel		m_screenPannel;		//播放屏幕底板－子窗口
	CPTZPannel			m_ptzPannel;		//云台控制面板－子窗口
	CNormalBtnPannel	m_normalBtnPannel;	//常规功能按键面板－子窗口
	CAdvanceBtnPannel	m_advanceBtnPannel;	//高级功能按键面板－子窗口
	CSelectPannel		m_selectPannel;		//显示“选择页面”按键的面板－子窗口
	CRuntimeMsgPannel	m_runtimeMsgPannel;	//显示运行时信息的面板－子窗口
	/*Begin: Add by yehao(10857) 2006-12-22*/
	list<DeviceNode *>	m_broadcastDevList;
	CAudioBroadcastDlg	m_audioBroadcastDlg;
	BOOL				m_bRecord;
	TalkHandleInfo		m_talkhandle;
	unsigned int		m_uRecordCount;
	/*End: yehao(10857) 2006-12-22*/
	CBrush				m_myBrush;


	//download test
	CDownloadByTime m_dbByTime;

	//specified alarm test
	FILE	*m_almLogFile_Comm;
	FILE	*m_almLogFile_Shelter;
	FILE	*m_almLogFile_dFull;
	FILE	*m_almLogFile_dError;
	FILE	*m_almLogFile_SoundDec;
	
	DEV_STATE	m_lastAlarm;

	//listen device test
	LLONG	m_lListenDevice;
	LLONG	m_lListenChannel;
	typedef struct 
	{
		int state;
		char ip[16];
		WORD port;
		char serial[48];
	}LISTEN_DATA;
	LISTEN_DATA m_mylsdata;
};

 
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETSDKDEMODLG_H__874C22A9_455C_4113_9954_FDC1528EE15D__INCLUDED_)
