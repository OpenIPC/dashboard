// stdafx.cpp : 只包括标准包含文件的源文件
// VTODemo.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

#if defined(_WIN64)

#pragma comment(lib, "../../../Lib/Win64/dhconfigsdk.lib")
#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
#else
#pragma comment(lib,"../../../Lib/Win32/dhconfigsdk.lib")
#pragma comment(lib,"../../../Lib/Win32/dhnetsdk.lib")
#endif