// TargetRecognition.h : main header file for the TARGETRECOGNITION application
//

#if !defined(AFX_TARGETRECOGNITION_H__2026399B_215E_4E6B_97FC_56A3F02A3127__INCLUDED_)
#define AFX_TARGETRECOGNITION_H__2026399B_215E_4E6B_97FC_56A3F02A3127__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTargetRecognitionApp:
// See TargetRecognition.cpp for the implementation of this class
//


class CTargetRecognitionApp : public CWinApp
{
public:
	CTargetRecognitionApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTargetRecognitionApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTargetRecognitionApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TARGETRECOGNITION_H__2026399B_215E_4E6B_97FC_56A3F02A3127__INCLUDED_)
