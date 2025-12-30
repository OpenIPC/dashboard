// ActiveLoginDemo.h : main header file for the ACTIVELOGINDEMO application
//

#if !defined(AFX_ACTIVELOGINDEMO_H__20FAB3AC_4DCE_4E01_BBB5_BE129F0D13D1__INCLUDED_)
#define AFX_ACTIVELOGINDEMO_H__20FAB3AC_4DCE_4E01_BBB5_BE129F0D13D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#define		MAX_CHANNEL  9
#define     ROOT_DIR        -100            // 跟目录标示
#define     SECOND_DIR      -10             // 二级目录标示
#define     DEVICE_DIR      -1              // 设备目录标示
#define     CHANNEL_DIR      0              // 通道目录标示

#define DLG_MAIN					"MainDlg"			
#define DLG_NAME					"NameDlg"
#define	DLG_DEVICEINFO				"DeviceInfoDlg"
#define	DLG_PTZ						"PTZControlDlg"

#define CHANNEL						ConvertString("Channel",DLG_MAIN)
#define UNREGISTER					ConvertString("UnRegister",DLG_MAIN)
#define NONAME						ConvertString("No Name",DLG_MAIN)
/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoApp:
// See ActiveLoginDemo.cpp for the implementation of this class
//
class CActiveLoginDemoApp : public CWinApp
{
public:
	CActiveLoginDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActiveLoginDemoApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CActiveLoginDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CString ConvertString(const CString& strText, const char* pszSeg = NULL);

void g_SetWndStaticText(CWnd * pWnd, const char* pszSeg = NULL);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTIVELOGINDEMO_H__20FAB3AC_4DCE_4E01_BBB5_BE129F0D13D1__INCLUDED_)
