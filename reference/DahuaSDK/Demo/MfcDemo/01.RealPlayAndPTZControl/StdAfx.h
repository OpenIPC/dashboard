// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__D30A0C81_2AD3_4EFC_8E13_A06D790FC362__INCLUDED_)
#define AFX_STDAFX_H__D30A0C81_2AD3_4EFC_8E13_A06D790FC362__INCLUDED_

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


#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win64/dhconfigsdk.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
	#pragma comment(lib, "../../../Lib/Win32/dhconfigsdk.lib")
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__D30A0C81_2AD3_4EFC_8E13_A06D790FC362__INCLUDED_)
