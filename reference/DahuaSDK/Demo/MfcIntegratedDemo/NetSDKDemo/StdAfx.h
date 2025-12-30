// StdAfx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__2449B94D_19D1_4DAB_9854_6D1075BF3EE6__INCLUDED_)
#define AFX_STDAFX_H__2449B94D_19D1_4DAB_9854_6D1075BF3EE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//#define LANG_ENG
#define LANG_ENG

#ifdef LANG_ENG
#include "messagetext_eng.h"
#else
//#include "messagetext.h"
#endif

#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win64/dhconfigsdk.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win32/dhconfigsdk.lib")
#endif

#include "../../../Include/Common/dhnetsdk.h"
#include "../../../Include/Common/dhconfigsdk.h"
#include "DevMgr.h"

typedef struct _In_DeviceInfo
{
    LLONG                lDeviceID;   	//设备的ID
    char                cDeviceName[32]; //设备名称
    unsigned long       unTypeIndex;      //设备类型
    unsigned long       unNetType;       //网络模式,0带包头模式、1非带包头模式
    unsigned long       unVideoCodeType; //视频编码格式:8为MPEG4,9为H264
    unsigned long       unChannelNum;   //当前设备通道数
    unsigned long       unAlarmInNum;   //当前设备报警输入通道数
    unsigned long       unAlarmOutNum;  //当前设备报警输出通道数
}In_DeviceInfo;

#define CONFIG_WAITTIME	500


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__2449B94D_19D1_4DAB_9854_6D1075BF3EE6__INCLUDED_)
