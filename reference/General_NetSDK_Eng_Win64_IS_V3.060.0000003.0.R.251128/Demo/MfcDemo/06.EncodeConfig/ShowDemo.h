// ShowDemo.h : main header file for the SHOWDEMO application
//

#if !defined(AFX_SHOWDEMO_H__C3D18D42_2877_487C_8D99_DC9C56F26E16__INCLUDED_)
#define AFX_SHOWDEMO_H__C3D18D42_2877_487C_8D99_DC9C56F26E16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#define TIMEOUT         5000
/////////////////////////////////////////////////////////////////////////////
// CShowDemoApp:
// See ShowDemo.cpp for the implementation of this class
//

class CShowDemoApp : public CWinApp
{
public:
	CShowDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
    
// Implementation

	//{{AFX_MSG(CShowDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWDEMO_H__C3D18D42_2877_487C_8D99_DC9C56F26E16__INCLUDED_)
