// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__579D5D2C_B21B_4584_90A4_DC47B24C2734__INCLUDED_)
#define AFX_STDAFX_H__579D5D2C_B21B_4584_90A4_DC47B24C2734__INCLUDED_

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
#endif // _AFX_NO_AFXCMN_SUPPO


//{{AFX_INSERT_LOCATION}}
#include <string>
#include "../../../Include/Common/dhnetsdk.h"
#include "../../../Include/Common/dhconfigsdk.h"
#include "dhplay/play.h"

#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win64/dhconfigsdk.lib")
	#pragma comment(lib, "dhplay/win64/play.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win32/dhconfigsdk.lib")
	#pragma comment(lib, "dhplay/win32/play.lib")
#endif
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__579D5D2C_B21B_4584_90A4_DC47B24C2734__INCLUDED_)
