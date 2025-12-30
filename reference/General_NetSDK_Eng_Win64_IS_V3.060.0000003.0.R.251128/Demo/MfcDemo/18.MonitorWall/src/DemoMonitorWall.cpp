// DemoMonitorWall.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DemoMonitorWall.h"
#include "DemoMonitorWallDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString CDemoMonitorWallApp::m_strModuleDir;

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallApp

BEGIN_MESSAGE_MAP(CDemoMonitorWallApp, CWinApp)
	//{{AFX_MSG_MAP(CDemoMonitorWallApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallApp construction

CDemoMonitorWallApp::CDemoMonitorWallApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDemoMonitorWallApp object

CDemoMonitorWallApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDemoMonitorWallApp initialization

BOOL CDemoMonitorWallApp::InitInstance()
{
	AfxEnableControlContainer();

	//
	GetModuleFileName(NULL, m_strModuleDir.GetBuffer(_MAX_PATH), _MAX_PATH);
	m_strModuleDir.ReleaseBuffer();
	int nPos = m_strModuleDir.ReverseFind('\\');
	if (nPos >= 0)
	{
		m_strModuleDir.Delete(nPos + 1, m_strModuleDir.GetLength() - nPos - 1);
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CDemoMonitorWallDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
