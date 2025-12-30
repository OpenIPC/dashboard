// HeatMap.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HeatMap.h"
#include "HeatMapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHeatMapApp

BEGIN_MESSAGE_MAP(CHeatMapApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CHeatMapApp construction

CHeatMapApp::CHeatMapApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CHeatMapApp object

CHeatMapApp theApp;


// CHeatMapApp initialization

BOOL CHeatMapApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CHeatMapDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
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

TCHAR* g_GetIniPath(void)
{
    static char pszIniPath[512] = {0};
    if( strlen(pszIniPath) == 0 )
    {
        char szDirBuf[512] = {0};
        GetCurrentDirectory(512, szDirBuf);
        _snprintf_s(pszIniPath, 512 - 1, "%s\\langchn.ini", szDirBuf);
    }
    return pszIniPath;
}

CString ConvertString(CString strText)
{
    char val[200] = {0};
    CString strIniPath,strRet;
    GetPrivateProfileString("String",strText,"", val,200,g_GetIniPath());
    strRet = val;
    if(strRet.GetLength()==0)
    {
        //If there is no corresponding string in ini file ,then set it to be default value.
        strRet=strText;
    }
    return strRet;
}
//Set static text in dialogue box (English->current language)
void g_SetWndStaticText(CWnd * pWnd)
{
    CString strCaption,strText;

    //Set main window title
    pWnd->GetWindowText(strCaption);
    if(strCaption.GetLength()>0)
    {
        strText=ConvertString(strCaption);
        pWnd->SetWindowText(strText);
    }

    //Set small window title 
    CWnd * pChild=pWnd->GetWindow(GW_CHILD);
    CString strClassName;
    while(pChild)
    {
        //////////////////////////////////////////////////////////////////////////		
        //Added by Jackbin 2005-03-11
        strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
        if(strClassName == "CEdit")
        {
            //Next small window 
            pChild=pChild->GetWindow(GW_HWNDNEXT);
            continue;
        }

        //////////////////////////////////////////////////////////////////////////	

        //Set small window current language text 
        pChild->GetWindowText(strCaption);
        strText=ConvertString(strCaption);
        pChild->SetWindowText(strText);

        //Next small window 
        pChild=pChild->GetWindow(GW_HWNDNEXT);
    }
}

