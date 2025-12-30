// CapturePicture.h : main header file for the CAPTUREPICTURE application
//

#if !defined(AFX_CAPTUREPICTURE_H__F93E4FC1_CC3E_4E0B_A10B_5215DA81B963__INCLUDED_)
#define AFX_CAPTUREPICTURE_H__F93E4FC1_CC3E_4E0B_A10B_5215DA81B963__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCapturePictureApp:
// See CapturePicture.cpp for the implementation of this class
//

class CCapturePictureApp : public CWinApp
{
public:
	CCapturePictureApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCapturePictureApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCapturePictureApp)
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

#endif // !defined(AFX_CAPTUREPICTURE_H__F93E4FC1_CC3E_4E0B_A10B_5215DA81B963__INCLUDED_)
