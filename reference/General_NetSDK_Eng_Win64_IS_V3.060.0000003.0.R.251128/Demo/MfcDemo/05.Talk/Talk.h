// Talk.h : main header file for the Talk application
//

#if !defined(AFX_Talk_H__4A9C93F8_AE11_4E01_980E_38D5B7FA4C55__INCLUDED_)
#define AFX_Talk_H__4A9C93F8_AE11_4E01_980E_38D5B7FA4C55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "../../../Include/Common/dhnetsdk.h"
#include <iostream>

#define DEMO_SDK_WAITTIME 3000
/////////////////////////////////////////////////////////////////////////////
// CTalkApp:
// See Talk.cpp for the implementation of this class
//

class CTalkApp : public CWinApp
{
public:
	CTalkApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTalkApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTalkApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void g_SetWndStaticText(CWnd * pWnd);
CString ConvertString(CString strText);
std::string I2Str(const int v);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_Talk_H__4A9C93F8_AE11_4E01_980E_38D5B7FA4C55__INCLUDED_)
