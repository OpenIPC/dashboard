#ifndef _DEMO_MATRIX_UTIL_H_
#define _DEMO_MATRIX_UTIL_H_

#pragma warning(disable: 4786)
#include <map>
#include <vector>
#include <list>
#include "LanguageManager.h"

#include "../../../../Include/Common/dhnetsdk.h"
#include "../../../../Include/Common/dhconfigsdk.h"

#if defined(_WIN64)
	#pragma comment(lib, "../../../../Lib/Win64/dhnetsdk.lib")
	#pragma comment(lib, "../../../../Lib/Win64/dhconfigsdk.lib")
#else
	#pragma comment(lib, "../../../../Lib/Win32/dhnetsdk.lib")
	#pragma comment(lib, "../../../../Lib/Win32/dhconfigsdk.lib")
#endif

#ifndef _countof
#define _countof(array)		sizeof(array) / sizeof(array[0])
#endif

#define APP_NAME			"MonitorWall"
#define MAX_RECT_VALUE		8192.0

#define ID_MENU_GROUP_ID	13500

#define WAIT_TIME			5000

//**********************************************************************
// 结构定义
//**********************************************************************

enum
{
	TREE_ITEM_DEVICE,			// 设备结点
	TREE_ITEM_CHANNEL,			// 通道结点
};

class CEncoder;
// 节点信息
struct TreeItemData
{
	UINT					uItemType;					// 结点类型
	CEncoder*				pDevice;					// 设备信息
	int						nChannel;					// 通道信息, 通道结点有效

	TreeItemData(UINT uType = TREE_ITEM_DEVICE, CEncoder* pDev = NULL, int nChn = -1)
		: uItemType(uType)
		, pDevice(pDev)
		, nChannel(nChn)
	{
	}

	virtual bool operator==(const TreeItemData& other)
	{
		return other.uItemType == uItemType
			&& other.pDevice == pDevice
			&& other.nChannel == nChannel;
	}
};

struct TreeItemDataDscon : public TreeItemData
{
	int					nInputType;		// 0-analogMatrix, 1-remoteDevice
	char				szDevID[128];	// analogMatrixID or remoteDeviceID

	TreeItemDataDscon(int _nInputType, const char* szName, CEncoder* pDev = NULL, int nChn = -1)
		: TreeItemData(TREE_ITEM_CHANNEL, pDev, nChn)
		, nInputType(_nInputType)
	{
		ZeroMemory(szDevID, sizeof(szDevID));
		strncpy(szDevID, szName, min(strlen(szName), sizeof(szDevID) - 1));
	}
};

//**********************************************************************
// MesssageBox
//**********************************************************************
int MsgBoxFmt( HWND hWnd, UINT uType, LPCTSTR lpszCaption, LPCTSTR lpszTextFmt, ... );
int _MsgBoxErr(HWND hWnd, UINT uType, LPCTSTR lpszCaption, LPCTSTR lpszMsg);

#define MsgBox0(str)					MsgBoxFmt(m_hWnd, MB_ICONINFORMATION, LMIC("Prompt"), LMIC(str))
#define MsgBox1(str, p1)				MsgBoxFmt(m_hWnd, MB_ICONINFORMATION, LMIC("Prompt"), LMIC(str), p1)
#define MsgBox2(str, p1, p2)			MsgBoxFmt(m_hWnd, MB_ICONINFORMATION, LMIC("Prompt"), LMIC(str), p1, p2)
#define MsgBox3(str, p1, p2, p3)		MsgBoxFmt(m_hWnd, MB_ICONINFORMATION, LMIC("Prompt"), LMIC(str), p1, p2, p3)
#define MsgBox4(str, p1, p2, p3, p4)	MsgBoxFmt(m_hWnd, MB_ICONINFORMATION, LMIC("Prompt"), LMIC(str), p1, p2, p3, p4)
#define MsgBox							MsgBox0
#define MsgBoxErr(str)					_MsgBoxErr(m_hWnd, MB_ICONINFORMATION, LMIC("Prompt"), str)


#endif
