// DeviceControlAndTimeSynchronization.h : main header file for the DeviceControlAndTimeSynchronization application
//

#if !defined(AFX_DeviceControlAndTimeSynchronization_H__7BF89A7A_46C4_4B95_BA2C_30BC2720A3CD__INCLUDED_)
#define AFX_DeviceControlAndTimeSynchronization_H__7BF89A7A_46C4_4B95_BA2C_30BC2720A3CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDeviceControlAndTimeSynchronizationApp:
// See DeviceControlAndTimeSynchronization.cpp for the implementation of this class
//

class CDeviceControlAndTimeSynchronizationApp : public CWinApp
{
public:
	CDeviceControlAndTimeSynchronizationApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceControlAndTimeSynchronizationApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDeviceControlAndTimeSynchronizationApp)
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

#endif // !defined(AFX_DeviceControlAndTimeSynchronization_H__7BF89A7A_46C4_4B95_BA2C_30BC2720A3CD__INCLUDED_)
