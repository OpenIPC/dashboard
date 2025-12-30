// PlayBack.h : main header file for the CLIENTDEMO3 application
//

#if !defined(AFX_CLIENTDEMO3_H__BB715EF5_70CB_4ACC_BCDA_F3D0383C0503__INCLUDED_)
#define AFX_CLIENTDEMO3_H__BB715EF5_70CB_4ACC_BCDA_F3D0383C0503__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// PlayBackApp:
// See PlayBack.cpp for the implementation of this class
//

class PlayBackApp : public CWinApp
{
public:
	PlayBackApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PlayBackApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(PlayBackApp)
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

#endif // !defined(AFX_CLIENTDEMO3_H__BB715EF5_70CB_4ACC_BCDA_F3D0383C0503__INCLUDED_)
