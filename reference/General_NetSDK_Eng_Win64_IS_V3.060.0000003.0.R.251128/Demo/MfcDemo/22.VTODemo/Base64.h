//
//  "$Id$"
//
//  Copyright (c)1992-2007, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __DAHUA_BASE64_ORDER_H__
#define __DAHUA_BASE64_ORDER_H__

/// 计算存放编码后数据的缓冲区字节数
int base64EncodeLen(int len);

/// Base64 编码
/// \param [in] dst 存放编码后的文本缓存
/// \param [in] src 编码前的数据缓存
/// \param [in] len 编码前的数据字节数
/// \return 返回编码后的文本字节数, 不包含字符串结束符
int  base64Encode(char* dst, const char* src, int len);

/// 计算存放解码后数据的缓冲区字节数
int  base64DecodeLen(const char* src);

bool base64Encode(const char *src, int len, CString &strDst);

/// Base64 解码
/// \param [in] dst 解码前的文本缓存
/// \param [in] src 解码后的数据缓存
/// \return 返回解码后的数据字节数
int base64Decode(char* dst, const char* src);


#endif// __DAHUA_BASE64_ORDER_H__


