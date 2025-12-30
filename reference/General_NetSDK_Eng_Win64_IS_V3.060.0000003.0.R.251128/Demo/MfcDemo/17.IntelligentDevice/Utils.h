#ifndef _DEMO_SPEAK_UTIL_H_
#define _DEMO_SPEAK_UTIL_H_

#pragma warning(disable: 4786)
#include <map>
#include <vector>
#include <list>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <iterator>
#include <algorithm>
#include <stdlib.h>

#define DEFAULT_WAIT_TIME 5000

#ifndef _countof
#define _countof(array)		sizeof(array) / sizeof(array[0])
#endif

CString ConvertString(CString strText);

const COLORREF RESERVED_COLOR = RGB(255, 255, 255);

void	g_SetWndStaticText(CWnd * pWnd);
void	FillCWndWithDefaultColor(CWnd* cwnd);

#endif