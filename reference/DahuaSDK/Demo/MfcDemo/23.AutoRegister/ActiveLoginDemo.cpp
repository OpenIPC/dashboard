// ActiveLoginDemo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ActiveLoginDemo.h"
#include "ActiveLoginDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoApp

BEGIN_MESSAGE_MAP(CActiveLoginDemoApp, CWinApp)
	//{{AFX_MSG_MAP(CActiveLoginDemoApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoApp construction

CActiveLoginDemoApp::CActiveLoginDemoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CActiveLoginDemoApp object

CActiveLoginDemoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CActiveLoginDemoApp initialization

BOOL CActiveLoginDemoApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
    
    
    if(CoInitialize(NULL)!= 0)
    {
		AfxMessageBox(ConvertString("Initialization Com Fail!",DLG_MAIN));
/*        AfxMessageBox("初始化Com支持库失败!");*/
        exit(1);
    }
	CActiveLoginDemoDlg dlg;
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

CString ConvertString(const CString& strText, const char* pszSeg /* = NULL */)
{
    CString strIniPath, strRet;
    char szVal[256] = {0};
    
    if (!pszSeg)
    {
        pszSeg = DLG_MAIN;
    }
    
    GetPrivateProfileString(pszSeg, strText, "", szVal, sizeof(szVal), "./langchn.ini");
    strRet = szVal;
    if(strRet.GetLength()==0)
    {
        //If there is no corresponding string in ini file then set it to be default value(English).
        strRet=strText;
    }
    
    return strRet;
}

int CActiveLoginDemoApp::ExitInstance() 
{
    CoUninitialize();	
	return CWinApp::ExitInstance();
}

//Set static text in the dialogue box(English->current language) 
void g_SetWndStaticText(CWnd * pWnd, const char* pszSeg /* = NULL */)
{
	CString strCaption,strText;

	//Set main window title 
	pWnd->GetWindowText(strCaption);
	if(strCaption.GetLength()>0)
	{
		strText=ConvertString(strCaption, pszSeg);
		pWnd->SetWindowText(strText);

	}

	//Set sub-window title 
	CWnd * pChild=pWnd->GetWindow(GW_CHILD);
	CString strClassName;
	while(pChild)
	{
		//////////////////////////////////////////////////////////////////////////		
		//Added by Jackbin 2005-03-11
		strClassName = ((CRuntimeClass*)pChild->GetRuntimeClass())->m_lpszClassName;
		if(strClassName == "CEdit")
		{
			//The next sub-window 
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}

		//////////////////////////////////////////////////////////////////////////	

		//Set current language text in the sub-window 
		pChild->GetWindowText(strCaption);
		strText=ConvertString(strCaption, pszSeg);
		pChild->SetWindowText(strText);

		//The next sub-window 
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}