#pragma once

typedef struct tagCardInfo
{
	char	szCardNo[DH_MAX_CARDNO_LEN];
	char	szHomeNo[DH_COMMON_STRING_16]; //NET_FACE_RECORD_INFO
	int		nRecNo;
	char	szFingerData[10*1024];
	int		nFingerLen;
}CardInfo;
