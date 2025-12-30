#include "stdafx.h"
#include "Util.h"

CString ConvertString(CString strText)
{
	char val[200] = {0};
	CString strIniPath,strRet;

	char szSoftPath[256] = {0};
	int filelen = GetModuleFileName(NULL, szSoftPath, 256);
	int k = filelen;
	while (szSoftPath[k] != '\\')
	{
		k--;
	}
	szSoftPath[k+1] = '\0';
	CString str = "\\langchn.ini";
	str = szSoftPath + str;
	
	memset(val,0,200);
	GetPrivateProfileString("String",strText,"",
		val,200, str/*"./langchn.ini"*/);
	strRet = val;
	if(strRet.GetLength()==0)
	{
		//If there is no corresponding string in ini file then set it to be default value(English)
		strRet=strText;
	}
	return strRet;
}

void g_SetWndStaticText(CWnd * pWnd)
{
	CString strCaption,strText;
	
	//Set main widnow title 
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
			//The next small window 
			pChild=pChild->GetWindow(GW_HWNDNEXT);
			continue;
		}
		
		//////////////////////////////////////////////////////////////////////////	
		
		//Set current language text in small window
		pChild->GetWindowText(strCaption);
		strText=ConvertString(strCaption);
		pChild->SetWindowText(strText);
		
		//The next small window 
		pChild=pChild->GetWindow(GW_HWNDNEXT);
	}
}