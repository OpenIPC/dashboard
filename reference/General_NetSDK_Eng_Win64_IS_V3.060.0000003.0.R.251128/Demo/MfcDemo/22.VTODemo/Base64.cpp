
#include "stdafx.h"
#include "Base64.h"
static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};
static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64EncodeLen(int len)
{
	return ((len + 2) / 3 * 4) + 1;	
}

/// base46 encode
 int  base64Encode(char* coded_dst, const char* plain_src, int len_plain_src)
{
	int i;
	char *p;

	p = coded_dst;
	for (i = 0; i < len_plain_src - 2; i += 3)
	{
		*p++ = basis_64[(plain_src[i] >> 2) & 0x3F];
		*p++ = basis_64[((plain_src[i] & 0x3) << 4) |
		                ((int) (plain_src[i + 1] & 0xF0) >> 4)];
		*p++ = basis_64[((plain_src[i + 1] & 0xF) << 2) |
		                ((int) (plain_src[i + 2] & 0xC0) >> 6)];
		*p++ = basis_64[plain_src[i + 2] & 0x3F];
	}
	if (i < len_plain_src)
	{
		*p++ = basis_64[(plain_src[i] >> 2) & 0x3F];
		if (i == (len_plain_src - 1))
		{
			*p++ = basis_64[((plain_src[i] & 0x3) << 4)];
			*p++ = '=';
		}
		else
		{
			*p++ = basis_64[((plain_src[i] & 0x3) << 4) |
			                ((int) (plain_src[i + 1] & 0xF0) >> 4)];
			*p++ = basis_64[((plain_src[i + 1] & 0xF) << 2)];
		}
		*p++ = '=';
	}

	*p++ = '\0';
	return p - coded_dst;
}

/// cal dst buf len
 int  base64DecodeLen(const char* coded_src)
{
	int nbytesdecoded;
	register const unsigned char *bufin;
	register int nprbytes;

	bufin = (const unsigned char *) coded_src;
	while (pr2six[*(bufin++)] <= 63)
		;

	nprbytes = (bufin - (const unsigned char *) coded_src) - 1;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;

	return nbytesdecoded + 1;
}

/// base64 decode
 int  base64Decode(char* plain_dst, const char* coded_src)
{
	int nbytesdecoded;
	register const unsigned char *bufin;
	register unsigned char *bufout;
	register int nprbytes;

	bufin = (const unsigned char *) coded_src;
	while (pr2six[*(bufin++)] <= 63)
		;
	nprbytes = (bufin - (const unsigned char *) coded_src) - 1;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;

	bufout = (unsigned char *) plain_dst;
	bufin = (const unsigned char *) coded_src;

	while (nprbytes > 4)
	{
		*(bufout++) =
		    (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
		*(bufout++) =
		    (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
		*(bufout++) =
		    (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
	}

	/* Note: (nprbytes == 1) would be an error, so just ingore that case */
	if (nprbytes > 1)
	{
		*(bufout++) =
		    (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
	}
	if (nprbytes > 2)
	{
		*(bufout++) =
		    (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
	}
	if (nprbytes > 3)
	{
		*(bufout++) =
		    (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
	}

	*(bufout++) = '\0';
	nbytesdecoded -= (4 - nprbytes) & 3;
	return nbytesdecoded;
}


 /// 计算存放编码后数据的缓冲区字节数
 int base64EncodeLen(int len);

 
 int  base64Encode(char* dst, const char* src, int len);


 bool base64Encode(const char *src, int len, CString &strDst)
 {
	 if (src == NULL || len == 0)
	 {
		return false;
	 }
	
	 int nDstLen = base64EncodeLen(len);
	 char *pDst = new char[nDstLen];
	 if (NULL == pDst)
	 {
		 return false;
	 }

	base64Encode(pDst, src, len);
	strDst=pDst;
	delete []pDst;
	return true;
 }



