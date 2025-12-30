// VehicleGPS.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVehicleGPSApp:
// See VehicleGPS.cpp for the implementation of this class
//

class CVehicleGPSApp : public CWinApp
{
public:
	CVehicleGPSApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CVehicleGPSApp theApp;