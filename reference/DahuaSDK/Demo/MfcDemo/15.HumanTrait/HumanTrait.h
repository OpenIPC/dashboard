// HumanTrait.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CHumanTraitApp:
// See HumanTrait.cpp for the implementation of this class
//

class CHumanTraitApp : public CWinApp
{
public:
	CHumanTraitApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CHumanTraitApp theApp;

void g_SetWndStaticText(CWnd * pWnd);
CString ConvertString(CString strText);
