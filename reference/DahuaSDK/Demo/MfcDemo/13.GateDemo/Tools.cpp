#include "stdafx.h"
#include "Tools.h"


void SetDate( HWND hwnd,NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(hwnd, nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwYear = stuTime.wYear;
	stNetTime.dwMonth = stuTime.wMonth;
	stNetTime.dwDay = stuTime.wDay;
}

void SetTime(HWND hwnd, NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(hwnd, nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwHour = stuTime.wHour;
	stNetTime.dwMinute = stuTime.wMinute;
	stNetTime.dwSecond = stuTime.wSecond;
}
