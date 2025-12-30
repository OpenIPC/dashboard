// IntelligentTraffic.h : main header file for the IntelligentTraffic application
//

#if !defined(_ACCESSCONTROLSYSTEM10_)
#define _ACCESSCONTROLSYSTEM10_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "Utility.h"

class CAccessControlApp : public CWinApp
{
public:
	CAccessControlApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRealLoadPictureApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
// Implementation

	//{{AFX_MSG(CRealLoadPictureApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_)
