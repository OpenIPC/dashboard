// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__585EA279_CF76_487A_A9BC_05EEEF7E9794__INCLUDED_)
#define AFX_STDAFX_H__585EA279_CF76_487A_A9BC_05EEEF7E9794__INCLUDED_

#define _WIN32_WINNT 0x0500

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

#include <assert.h>

/************************************************************************/
/* NETSDK                                                            */
/************************************************************************/
#include "../../../Include/Common/dhnetsdk.h"
#include "../../../Include/Common/dhconfigsdk.h"

#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win64/dhconfigsdk.lib")
	#pragma comment(lib, "Gdilib/win64/GdiPlus.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win32/dhconfigsdk.lib")
	#pragma comment(lib, "Gdilib/win32/GdiPlus.lib")
#endif


#pragma warning (disable:4786)
#include <vector>
#include <list>
#include <map>

#define  SINGLE_QUERY_COUNT  100   
#define  MAX_TIMEOUT        5000

#define   MAX_EVENT_IN_LIST_CONTROL 100  


// Device reconnect
#define WM_REDEVICE_RECONNECT       (WM_USER + 2)   
// Intelligent event
#define WM_INTELLIGENT_EVENT         (WM_USER + 3) 
// Picture download pos
#define WM_PICTURE_DOWNLOAD_POS      (WM_USER + 4) 

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__585EA279_CF76_487A_A9BC_05EEEF7E9794__INCLUDED_)
