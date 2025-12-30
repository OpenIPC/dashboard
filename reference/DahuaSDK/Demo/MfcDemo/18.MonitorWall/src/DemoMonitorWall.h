// DemoMonitorWall.h : main header file for the DEMOMONITORWALL application
//

#if !defined(AFX_DEMOMONITORWALL_H__BE72B78D_ADBA_4347_A3E2_80029DC31F57__INCLUDED_)
#define AFX_DEMOMONITORWALL_H__BE72B78D_ADBA_4347_A3E2_80029DC31F57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define MODULE_DIR			CDemoMonitorWallApp::m_strModuleDir
#define MAX_BUF_LEN         (2 * 1024 * 1024)

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallApp:
// See DemoMonitorWall.cpp for the implementation of this class
//

class CDemoMonitorWallApp : public CWinApp
{
public:
	CDemoMonitorWallApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDemoMonitorWallApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	static CString m_strModuleDir;

	//{{AFX_MSG(CDemoMonitorWallApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEMOMONITORWALL_H__BE72B78D_ADBA_4347_A3E2_80029DC31F57__INCLUDED_)
