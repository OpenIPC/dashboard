#include "StdAfx.h"
#include "Utils.h"

TCHAR* g_GetIniPath(void)
{
	static char pszIniPath[512] = {0};
	if( strlen(pszIniPath) == 0 )
	{
		char szDirBuf[512] = {0};
		GetCurrentDirectory(512, szDirBuf);
		_snprintf(pszIniPath, sizeof(pszIniPath), "%s\\langchn.ini", szDirBuf);
	}
	return pszIniPath;
}

CString ConvertString(CString strText)
{
	char szval[200] = {0};
	CString strIniPath,strRet;

	GetPrivateProfileString("String",strText,"",
		szval,200,g_GetIniPath());
	strRet = szval;
	if(strRet.GetLength()==0)
	{
		//If there is no corresponding string in ini file ,then set it to be default value.
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

void FillCWndWithDefaultColor(CWnd* cwnd) 
{
	if (NULL == cwnd)
	{
		return;
	}

	CDC* cdc = cwnd->GetDC();
	if (NULL == cdc)
	{
		return;
	}

	RECT rect;
	cwnd->GetClientRect(&rect);
	CBrush brush(RGB(105,105,105));
	cdc->FillRect(&rect, &brush);
	cwnd->ReleaseDC(cdc);
}