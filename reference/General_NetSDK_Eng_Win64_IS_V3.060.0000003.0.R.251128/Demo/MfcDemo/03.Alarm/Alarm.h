// Alarm.h : main header file for the Alarm application
//

#if !defined(AFX_Alarm_H__1EBA1713_8D02_439C_820A_8FB592364BA0__INCLUDED_)
#define AFX_Alarm_H__1EBA1713_8D02_439C_820A_8FB592364BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAlarmApp:
// See Alarm.cpp for the implementation of this class
//

class CAlarmApp : public CWinApp
{
public:
	CAlarmApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlarmApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAlarmApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void g_SetWndStaticText(CWnd * pWnd);
CString ConvertString(CString strText);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Alarm_H__1EBA1713_8D02_439C_820A_8FB592364BA0__INCLUDED_)
