// RealPlayAndPTZControl.h : main header file for the RealPlayAndPTZControl application
//

#if !defined(AFX_RealPlayAndPTZControl_H__98F68F83_D38E_403B_9EEA_0C951855AD74__INCLUDED_)
#define AFX_RealPlayAndPTZControl_H__98F68F83_D38E_403B_9EEA_0C951855AD74__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRealPlayAndPTZControlApp:
// See RealPlayAndPTZControl.cpp for the implementation of this class
//

class CRealPlayAndPTZControlApp : public CWinApp
{
public:
	CRealPlayAndPTZControlApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRealPlayAndPTZControlApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRealPlayAndPTZControlApp)
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

#endif // !defined(AFX_RealPlayAndPTZControl_H__98F68F83_D38E_403B_9EEA_0C951855AD74__INCLUDED_)
