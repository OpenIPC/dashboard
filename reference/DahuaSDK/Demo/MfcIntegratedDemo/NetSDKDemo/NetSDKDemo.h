// NetSDKDemo.h : main header file for the NETSDKDEMO application
//

#if !defined(AFX_NETSDKDEMO_H__F984CDA1_DB9B_44E5_ADD8_44A8BB6D6E9D__INCLUDED_)
#define AFX_NETSDKDEMO_H__F984CDA1_DB9B_44E5_ADD8_44A8BB6D6E9D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'StdAfx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "../../../Include/Common/dhnetsdk.h"
/////////////////////////////////////////////////////////////////////////////
// CNetSDKDemoApp:
// See NetSDKDemo.cpp for the implementation of this class
//

#define MAX_TIMEOUT 5000

//老报警接口
typedef struct 
{
	NET_CLIENT_STATE cState;
	DWORD dError;
	DWORD dFull;
	BYTE  shelter[16];
	BYTE  soundalarm[16];
	BYTE  almDecoder[16];
}DEV_STATE;

//设备信息列表结构
typedef struct _DeviceNode {
	char UserNanme[40]; //登录用户名
	char Name[24];   //设备名称
	char IP[129];     //设备IP地址字符
	LLONG LoginID;    //设备登录iD
	//NET_DEVICEINFO Info;  //设备信息
	NET_DEVICEINFO_Ex Info;	//设备信息
	int nChnNum;        //设备通道数
	DEV_STATE State; //设备状态
	DWORD TotalKbps;     //设备当前总码流
	DWORD Max_Kbps;     //设置最大网络流量
	BOOL	bIsOnline;
}DeviceNode;

//当前画面显示内容的类型
typedef enum _SplitType{
	SPLIT_TYPE_NULL = 0,   //空白
	SPLIT_TYPE_MONITOR,    //网络预览
	SPLIT_TYPE_NETPLAY,    //网络回放
	SPLIT_TYPE_MULTIPLAY,  //网络预览
	SPLIT_TYPE_FILEPLAY,    //本地文件播放
	SPLIT_TYPE_CYCLEMONITOR,   //轮循预览
	SPLIT_TYPE_PBBYTIME		//通过时间回放
}SplitType;

//视频参数结构
typedef union _VideoParam{
		BYTE  bParam[4];
		DWORD dwParam; //视频参数
}VideoParam;

//画面分割通道显示信息(可以定义成type/param，param自定义)
typedef struct _SplitInfoNode
{
	SplitType Type;     //显示类型 空白/预览/网络回放/本地回放等
	LLONG iHandle;  //用于记录通道id(预览通道ID/播放文件iD等)
//	DWORD nTimeCount;   //时间计数,用于码流统计 
//	DWORD nKBPS;       //码流统计//sdk增加接口后不用应用层统计
	int isSaveData;  //数据是否保存(直接sdk保存)
	FILE *SavecbFileRaw; //保存回调原始数据
	FILE *SavecbFileStd; //保存回调mp4数据
	FILE *SavecbFileYUV; //保存回调yuv数据
	FILE *SavecbFilePcm;  //保存回调pcm数据
	VideoParam nVideoParam;  //视频参数
	void *Param;  //信息参数,对于不同的显示有不同的参数
}SplitInfoNode;


class CNetSDKDemoApp : public CWinApp
{
public:
	CNetSDKDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetSDKDemoApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNetSDKDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/*
class CDeviceListProtector //全局变量保护类
{
public:
	CDeviceListProtector(CPtrList **list):m_ptrdevicelist(list)
	{
		*m_ptrdevicelist = new CPtrList(sizeof(DeviceNode));
		*list = *m_ptrdevicelist;
	}
	~CDeviceListProtector()
	{
		int count = (*m_ptrdevicelist)->GetCount();
		for (int i = 0; i < count; i++)
		{
			delete (DeviceNode *)((*m_ptrdevicelist)->GetTail());
			(*m_ptrdevicelist)->RemoveTail();
		}
		(*m_ptrdevicelist)->RemoveAll();
		delete (*m_ptrdevicelist);
	}
private:
	CPtrList **m_ptrdevicelist;
};

class CCSLock
{
public:
	CCSLock(CRITICAL_SECTION& cs):m_cs(cs)
	{
		EnterCriticalSection(&m_cs);
	}
	
	~CCSLock()
	{
		LeaveCriticalSection(&m_cs);
	}
private:
	CRITICAL_SECTION& m_cs;
};
*/

//extern CRITICAL_SECTION g_cs;

//全局变量，用于保存程序所在目录的路径名
extern CString g_strWorkDir;

//全局变量，用于保存设备列表
//extern CPtrList  *g_ptrdevicelist ;

//全局函数，将系统时间格式转换为私有定义的网络时间格式
void g_systimetoprivatetime(SYSTEMTIME *systime , NET_TIME *privatetime);

//全局函数，将私有定义的网络时间格式转换为系统时间格式
void g_privatetimetosystime(NET_TIME *privatetime , SYSTEMTIME *systime);

//全局函数，将私有定义的网络时间转换为字符串显示
CString g_TimeOutString(NET_TIME *privatetime );

//全局函数, 计算两时间点之差
DWORD g_IntervalTime(NET_TIME *stime, NET_TIME *etime );

void g_SetWndStaticText(CWnd * pWnd);

char* g_GetIniPath(void);

CString ConvertString(CString strText);

void ConvertComboBox(CComboBox &stuComboBox);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETSDKDEMO_H__F984CDA1_DB9B_44E5_ADD8_44A8BB6D6E9D__INCLUDED_)
