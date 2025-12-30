// SearchDevice.h : main header file for the SEARCHDEVICE application
//

#if !defined(AFX_SEARCHDEVICE_H__46B492CC_1BFF_438D_BA51_2DD7BC3FF656__INCLUDED_)
#define AFX_SEARCHDEVICE_H__46B492CC_1BFF_438D_BA51_2DD7BC3FF656__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSearchDeviceApp:
// See SearchDevice.cpp for the implementation of this class
//

class CSearchDeviceApp : public CWinApp
{
public:
	CSearchDeviceApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchDeviceApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSearchDeviceApp)
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

#endif // !defined(AFX_SEARCHDEVICE_H__46B492CC_1BFF_438D_BA51_2DD7BC3FF656__INCLUDED_)
