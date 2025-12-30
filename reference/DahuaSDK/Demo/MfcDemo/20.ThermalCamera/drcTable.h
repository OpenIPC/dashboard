#pragma once
//#ifdef DLL_IMPLEMENT
//#define DLL_API __declspec(dllexport)
//#else
//#define DLL_API __declspec(dllimport)
//#endif
//#define DLL_API
#ifndef _DRC_TABLE_H_
#define _DRC_TABLE_H_
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* 函数名  : drcTable
* 描  述  : raw数据转换成Y数据
* 输  入  : - pRaw: 14bit裸数据
* 输  入  : - height: 输入裸数据的高
* 输  入  : - width:  输入裸数据的宽
* 输  入  : - sel:  默认填0
* 输  入  : - lut:  当sel为1时，该参数为本地drc表，默认填0
* 输  出  : pYData：转换后的Y数据
* 返回值  : 0  : 成功
*           -1: 失败
*******************************************************************************/
/*DLL_API*/ int /*__stdcall*/ drcTable(unsigned short *pRaw, unsigned short width, unsigned short height, int sel, unsigned char *pYData, unsigned short* lut);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif