#include "StdAfx.h"
#include "Util.h"

int MsgBoxFmt( HWND hWnd, UINT uType, LPCTSTR lpszCaption, LPCTSTR lpszTextFmt, ... )
{
	TCHAR szText[1024] = {0};
	
	va_list args;
	va_start(args, lpszTextFmt);  
    _vstprintf(szText, lpszTextFmt, args);
	va_end(args);
	
	return MessageBox(hWnd, szText, lpszCaption, uType);
}

int _MsgBoxErr( HWND hWnd, UINT uType, LPCTSTR lpszCaption, LPCTSTR lpszMsg )
{
	CString str = LMIC(lpszMsg);
	str += " ";
	str += LMIC(_T("Code:"));

	CString strCode;
	strCode.Format(_T(" 0x%08X"), CLIENT_GetLastError());
	str += strCode;

	return MessageBox(hWnd, str, lpszCaption, uType);
}
