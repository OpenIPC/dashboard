// IntelligentTraffic.h : main header file for the IntelligentTraffic application
//

#if !defined(AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_)
#define AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

class CParkingSpaceDetectorApp : public CWinApp
{
public:
	CParkingSpaceDetectorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParkingSpaceDetectorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
// Implementation

	//{{AFX_MSG(CParkingSpaceDetectorApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_REALLOADPICTURE_H__D79C176B_B614_478F_A246_6DACDBE3869F__INCLUDED_)
