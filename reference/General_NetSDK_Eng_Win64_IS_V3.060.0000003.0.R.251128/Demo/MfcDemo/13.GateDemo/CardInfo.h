#pragma once

struct tagCardInfo
{
	char	szCardNo[DH_MAX_CARDNO_LEN];
	char	szUserID[DH_MAX_USERID_LEN];
	char	szCardName[DH_MAX_CARDNAME_LEN];
	char	szPsw[DH_MAX_CARDPWD_LEN];
	int		nRecNo;

	NET_ACCESSCTLCARD_STATE			emStatus;				// 卡状态
	NET_ACCESSCTLCARD_TYPE			emType;					// 卡类型
	int								nUserTime;				// 使用次数
	BOOL							bFirstEnter;            // 是否拥有首卡权限
	BOOL							bIsValid;               // 是否有效,TRUE有效;FALSE无效

	NET_TIME        stuValidStartTime;                      // 开始有效期, 设备暂不支持时分秒
	NET_TIME        stuValidEndTime;                        // 结束有效期, 设备暂不支持时分秒
};
