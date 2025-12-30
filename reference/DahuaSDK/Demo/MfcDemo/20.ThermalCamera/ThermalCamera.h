// ThermalCamera.h : main header file for the ThermalCamera application
//

#if !defined(AFX_ThermalCamera_H__FD88D889_3B9E_44AD_9CB9_0AB461FD66F5__INCLUDED_)
#define AFX_ThermalCamera_H__FD88D889_3B9E_44AD_9CB9_0AB461FD66F5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define DEVICE_PARAM			"DeviceParam"		// for setting device basic params
#define DLG_MAIN                "MainDlg"

#define DISABLE         0
#define ENABLE          1
#define UNKNOWN         0
#define SPOT            1
#define LINE            2
#define AREA            3
/////////////////////////////////////////////////////////////////////////////
// CThermalCameraApp:
// See ThermalCamera.cpp for the implementation of this class
//


class CThermalCameraApp : public CWinApp
{
public:
	CThermalCameraApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThermalCameraApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CThermalCameraApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ThermalCamera_H__FD88D889_3B9E_44AD_9CB9_0AB461FD66F5__INCLUDED_)
