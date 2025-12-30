// Device.cpp: implementation of the CDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "demomonitorwall.h"
#include "Device.h"

CDevice* CDeviceFactory::Create( LLONG lLoginID, 
								LPCTSTR lpszIP, 
								int nPort /*= 37777*/, 
								LPCTSTR lpszUser /*= NULL*/, 
								LPCTSTR lpszPwd /*= NULL*/, 
								LPCTSTR lpszDevClass, 
								UINT nType /*= Decoder*/ )
{
#define BEGIN_CREATE_DEVICE if(0) ;
#define NEW_DEVICE(className) { pDev = new className(lLoginID, lpszIP, nPort, lpszUser, lpszPwd); }
#define CREATE_DEVICE(className) else if (_stricmp(lpszDevClass, className::DCID()) == 0) NEW_DEVICE(className)
#define END_CREATE_DEVICE(className)	else NEW_DEVICE(className)

	//*****************************************************************************

	CDevice* pDev = NULL;
	if (lpszDevClass == NULL) lpszDevClass = "";
	if (nType == Decoder)
	{
		BEGIN_CREATE_DEVICE
		CREATE_DEVICE(CDecoderMatrix)
		CREATE_DEVICE(CDecoderNVR)
		CREATE_DEVICE(CDecoderNVD)
		CREATE_DEVICE(CDecoderDSCON)
		END_CREATE_DEVICE(CDecoder)
	}
	else
	{
		BEGIN_CREATE_DEVICE
		CREATE_DEVICE(CEncoderMatrix)
		CREATE_DEVICE(CEncoderNVR)
		CREATE_DEVICE(CEncoderNVD)
		CREATE_DEVICE(CEncoderDSCON)
		END_CREATE_DEVICE(CEncoder)
	}
	return pDev;
}

/************************************************************************/
/*                            解码通道                                  */
/************************************************************************/

CDecChannel::CDecChannel( int _nChannelID /*= -1*/, int _nCardID /*= -1*/, int _nCardSlot /*= -1*/, int _nMaxSources /*= 0*/ ) 
: m_nChannelID(_nChannelID)
, m_nCardID(_nCardID)
, m_nCardSlot(_nCardSlot)
, m_nMaxSources(_nMaxSources)
, m_bSpliceScreen(FALSE)
, m_bPartOfSpliceScreen(FALSE)
{
	ZeroMemory(m_nSplitModes, sizeof(m_nSplitModes));
}

LPCTSTR CDecChannel::ToString() const
{
	static CString strName;
	if (m_nCardID >= 0 && m_nCardSlot >= 0)
	{
		strName.Format("TV%d(Card%d_%d)", m_nChannelID, m_nCardID, m_nCardSlot);
	}
	else
	{
		// 融合屏
		if (m_bSpliceScreen)
		{
			strName.Format("STV%d", m_nChannelID);
			if (!m_strName.IsEmpty())
			{
				strName += "-";
				strName += m_strName;
			}
		}
		else
		{
			strName.Format("TV%d", m_nChannelID);
		}
	}
	return strName;
}

/************************************************************************/
/*                            编码通道									*/
/************************************************************************/
CEncChannel::CEncChannel( int _nChannelID /*= -1*/, int _nCardID /*= -1*/, int _nCardSlot /*= -1*/)
: m_nChannelID(_nChannelID)
, m_nCardID(_nCardID)
, m_nCardSlot(_nCardSlot)
{

}

LPCTSTR CEncChannel::ToString() const
{
	static CString strName;

	if (!m_strName.IsEmpty())
	{
		return m_strName;
	}
	else if (m_nCardID >= 0 && m_nCardSlot >= 0)
	{
		strName.Format("%s%d(%s%d_%d)", LMIC("Chn"), m_nChannelID, LMIC("Card"), m_nCardID, m_nCardSlot);
	}
	else
	{
		strName.Format("%s%d", LMIC("Chn"), m_nChannelID);
	}
	return strName;
}

/************************************************************************/
/*                              基本设备                                */
/************************************************************************/

CDevice::CDevice( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/, LPCTSTR lpszDevClass /*= "Unknown"*/ )
: m_lLoginID(lLoginID)
, m_strIP(lpszIP)
, m_nPort(nPort)
, m_strUser(lpszUser)
, m_strPwd(lpszPwd)
, m_pszDeviceClass(lpszDevClass)
{
	
}

void CDevice::SetLoginInfo( LPCTSTR lpszIP, int nPort, LPCTSTR lpszUser, LPCTSTR lpszPwd )
{
	m_strIP = lpszIP;
	m_nPort = nPort;
	m_strUser = lpszUser;
	m_strPwd = lpszPwd;
}


/************************************************************************/
/*                              解码设备                                */
/************************************************************************/

CDecoder::CDecoder( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/, LPCTSTR lpszDevClass /*= NULL*/ )
: CDevice(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, lpszDevClass == NULL ? DCID() : lpszDevClass)
, m_nSpliceCount(0)
{
	ZeroMemory(&m_stuProdDef, sizeof(m_stuProdDef));
}

BOOL CDecoder::Update()
{
	ASSERT(m_lLoginID);
	//BOOL bResult = FALSE;		
	m_mapDecChn.clear();
	ZeroMemory(&m_stuProdDef, sizeof(m_stuProdDef));
	m_stuProdDef.dwSize = sizeof(m_stuProdDef);
	m_nSpliceCount = 0;
	
	if (CLIENT_QueryProductionDefinition(m_lLoginID, &m_stuProdDef))
	{
		// 普通通道
 		UpdateNormalChannel();

		// 融合屏通道
		if (m_stuProdDef.nVideoOutputCompositeChannels > 0)
		{
			UpdateCompositeChannel();
		}

		return TRUE;
	}
	return FALSE;
}

BOOL CDecoder::UpdateCompositeChannel()
{
	const int nMaxJsonBuf = MAX_BUF_LEN;
	char* pszJsonBuf = new char[nMaxJsonBuf];
	ZeroMemory(pszJsonBuf, nMaxJsonBuf);
	
	// 取融合屏虚拟通道信息
    const int nMaxCompositeNum = 256;
	DH_COMPOSITE_CHANNEL* pstuComposite = new DH_COMPOSITE_CHANNEL[nMaxCompositeNum];
	ZeroMemory(pstuComposite, sizeof(DH_COMPOSITE_CHANNEL) * nMaxCompositeNum);
	for (int i = 0; i < nMaxCompositeNum; ++i)
	{
		pstuComposite[i].dwSize = sizeof(DH_COMPOSITE_CHANNEL);
	}
	
	int nRetLen = 0;
	if (CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_COMPOSITE_CHN, 
		(char*)pstuComposite, pstuComposite->dwSize * nMaxCompositeNum, &nRetLen))
	{
		m_nSpliceCount = nRetLen / pstuComposite->dwSize;
		for (int i = 0; i < m_nSpliceCount; ++i)
		{					
			if (*pstuComposite[i].szCompositeID == 0)
				continue;

			// 获取分割能力
			DH_SPLIT_CAPS stuCaps = { sizeof(DH_SPLIT_CAPS) };
			CLIENT_GetSplitCaps(m_lLoginID, pstuComposite[i].nVirtualChannel, &stuCaps);
			
			// 将拼接屏添加到输入通道列表中
			CDecChannel stuChannel(pstuComposite[i].nVirtualChannel, -1, -1, stuCaps.nMaxSourceCount);
			stuChannel.m_bSpliceScreen = TRUE;
			stuChannel.m_strName = pstuComposite[i].szCompositeID;
			for (int k = 0; k < min(stuCaps.nModeCount, _countof(stuChannel.m_nSplitModes)); ++k)
			{
				stuChannel.m_nSplitModes[k] = stuCaps.emSplitMode[k];
			}
			m_mapDecChn[pstuComposite[i].nVirtualChannel] = stuChannel;
		}
	}
	delete[] pstuComposite;
	
	// 取电视墙配置, 把组成拼接屏的各个TV标识出来
	if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_MONITORWALL, -1, pszJsonBuf, nMaxJsonBuf, NULL))
	{
		const int nMaxMonitorWall = 1;
		AV_CFG_MonitorWall* pstuWall = new AV_CFG_MonitorWall[nMaxMonitorWall];
		for (int i = 0; i < nMaxMonitorWall; ++i)
		{
			pstuWall[i].nStructSize = sizeof(AV_CFG_MonitorWall);
			for (int j = 0; j < AV_CFG_Max_Block_In_Wall; ++j)
			{
				pstuWall[i].stuBlocks[j].nStructSize = sizeof(AV_CFG_MonitorWallBlock);
				for (int k = 0; k < AV_CFG_Max_TV_In_Block; ++k)
				{
					pstuWall[i].stuBlocks[j].stuTVs[k].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
				}
			}
		}
		
		int nRetLen = 0;
		if (CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf, pstuWall, pstuWall->nStructSize * nMaxMonitorWall, &nRetLen))
		{
			for (int i = 0; i < pstuWall->nBlockCount; ++i)
			{
				const AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[i];
				for (int j = 0; j < block.nTVCount; ++j)
				{
					const AV_CFG_MonitorWallTVOut& tv = block.stuTVs[j];
					CDecChannel* pChn = GetDecodeChannel(tv.nChannelID);
					if (pChn)
					{
						pChn->m_bPartOfSpliceScreen = TRUE;
					}
				}
			}
		}						
		delete[] pstuWall;
	}
	delete[] pszJsonBuf;

	return TRUE;
}

BOOL CDecoder::UpdateNormalChannel()
{
	// 查询子卡信息
	DH_MATRIX_CARD_LIST stuCards = {sizeof(stuCards)};
	for (int i = 0; i < _countof(stuCards.stuCards); ++i)
	{
		stuCards.stuCards[i].dwSize = sizeof(DH_MATRIX_CARD);
	}
	
	if (CLIENT_QueryMatrixCardInfo(m_lLoginID, &stuCards))
	{
		for (int i = 0; i < stuCards.nCount; ++i)
		{
			const DH_MATRIX_CARD& stuCard = stuCards.stuCards[i];
			if (!stuCard.bEnable)
				continue;
			
			if ((stuCard.dwCardType & DH_MATRIX_CARD_DECODE) && stuCard.nVideoDecChn > 0)
			{
				for (int j = stuCard.nVideoDecChnMin; j <= stuCard.nVideoDecChnMax; ++j)
				{
					DH_SPLIT_CAPS stuCaps = { sizeof(DH_SPLIT_CAPS) };
					CLIENT_GetSplitCaps(m_lLoginID, j, &stuCaps);
					
					CDecChannel stuChannel(j, i, j - stuCard.nVideoDecChnMin, stuCaps.nMaxSourceCount);
					for (int k = 0; k < min(stuCaps.nModeCount, _countof(stuChannel.m_nSplitModes)); ++k)
					{
						stuChannel.m_nSplitModes[k] = stuCaps.emSplitMode[k];
					}
					m_mapDecChn[j] = stuChannel;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < m_stuProdDef.nVideoOutChannel; ++i)
		{					
			DH_SPLIT_CAPS stuCaps = { sizeof(DH_SPLIT_CAPS) };
			CLIENT_GetSplitCaps(m_lLoginID, i, &stuCaps);
			
			CDecChannel stuChannel(i, -1, -1, stuCaps.nMaxSourceCount);
			for (int k = 0; k < min(stuCaps.nModeCount, _countof(stuChannel.m_nSplitModes)); ++k)
			{
				stuChannel.m_nSplitModes[k] = stuCaps.emSplitMode[k];
			}
			m_mapDecChn[i] = stuChannel;
		}
	}	
	return TRUE;
}

CDecChannel* CDecoder::GetDecodeChannel( int nChannel )
{
	CDecChnMap::iterator it = m_mapDecChn.find(nChannel);
	if (it != m_mapDecChn.end())
	{
		return &it->second;
	}
	return NULL;
}

const CDecChannel* CDecoder::GetDecodeChannel( int nChannel ) const
{
	CDecChnMap::const_iterator it = m_mapDecChn.find(nChannel);
	if (it != m_mapDecChn.end())
	{
		return &it->second;
	}
	return NULL;
}

/************************************************************************/
/*                              解码矩阵                                */
/************************************************************************/

CDecoderMatrix::CDecoderMatrix( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CDecoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

BOOL CDecoderMatrix::UpdateNormalChannel()
{
	// 矩阵需要查询子卡信息
	DH_MATRIX_CARD_LIST stuCards = {sizeof(stuCards)};
	for (int i = 0; i < _countof(stuCards.stuCards); ++i)
	{
		stuCards.stuCards[i].dwSize = sizeof(DH_MATRIX_CARD);
	}
	
	if (CLIENT_QueryMatrixCardInfo(m_lLoginID, &stuCards))
	{
		for (int i = 0; i < stuCards.nCount; ++i)
		{
			const DH_MATRIX_CARD& stuCard = stuCards.stuCards[i];
			if (!stuCard.bEnable)
				continue;
			
			if ((stuCard.dwCardType & DH_MATRIX_CARD_DECODE) && stuCard.nVideoDecChn > 0)
			{
				for (int j = stuCard.nVideoDecChnMin; j <= stuCard.nVideoDecChnMax; ++j)
				{
					DH_SPLIT_CAPS stuCaps = { sizeof(DH_SPLIT_CAPS) };
					CLIENT_GetSplitCaps(m_lLoginID, j, &stuCaps);
					
					CDecChannel stuChannel(j, i, j - stuCard.nVideoDecChnMin, stuCaps.nMaxSourceCount);
					for (int k = 0; k < min(stuCaps.nModeCount, _countof(stuChannel.m_nSplitModes)); ++k)
					{
						stuChannel.m_nSplitModes[k] = stuCaps.emSplitMode[k];
					}
					m_mapDecChn[j] = stuChannel;
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

/************************************************************************/
/*                              解码NVR                                */
/************************************************************************/

CDecoderNVR::CDecoderNVR( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CDecoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

BOOL CDecoderNVR::UpdateCompositeChannel()
{
	return TRUE;
}

/************************************************************************/
/*                              解码NVD                                 */
/************************************************************************/

CDecoderNVD::CDecoderNVD( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CDecoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}
/*
BOOL CDecoderNVD::UpdateCompositeChannel()
{
	const int nMaxJsonBuf = 64 * 1024;
	char* pszJsonBuf = new char[nMaxJsonBuf];
	ZeroMemory(pszJsonBuf, nMaxJsonBuf);	
	
	// 取融合屏配置
	if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_SPLICESCREEN, -1, pszJsonBuf, nMaxJsonBuf, NULL))
	{
		int nMaxSpliceScreen = m_stuProdDef.nVideoOutputCompositeChannels;
		AV_CFG_SpliceScreen* pstuScreen = new AV_CFG_SpliceScreen[nMaxSpliceScreen];
		for (int i = 0; i < nMaxSpliceScreen; ++i)
		{
			pstuScreen[i].nStructSize = sizeof(AV_CFG_SpliceScreen);
		}
		
		int nRetLen = 0;
		if (CLIENT_ParseData(CFG_CMD_SPLICESCREEN, pszJsonBuf, pstuScreen, pstuScreen->nStructSize * nMaxSpliceScreen, &nRetLen))
		{
			m_nSpliceCount = nRetLen / pstuScreen->nStructSize;			
			for (int i = 0; i < m_nSpliceCount; ++i)
			{
				int nChannel = m_stuProdDef.nVideoOutChannel + i;
				DH_SPLIT_CAPS stuCaps = { sizeof(DH_SPLIT_CAPS) };
				CLIENT_GetSplitCaps(m_lLoginID, nChannel, &stuCaps);
				
				CDecChannel stuChannel(nChannel, -1, -1, stuCaps.nMaxSourceCount);
				stuChannel.m_bSpliceScreen = TRUE;
				stuChannel.m_strName = pstuScreen[i].szName;
				for (int k = 0; k < min(stuCaps.nModeCount, _countof(stuChannel.m_nSplitModes)); ++k)
				{
					stuChannel.m_nSplitModes[k] = stuCaps.emSplitMode[k];
				}
				m_mapDecChn[nChannel] = stuChannel;
			}
		}

		// 取电视墙配置
		if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_MONITORWALL, -1, pszJsonBuf, nMaxJsonBuf, NULL))
		{
			const int nMaxMonitorWall = 1;
			AV_CFG_MonitorWall* pstuWall = new AV_CFG_MonitorWall[nMaxMonitorWall];
			for (int i = 0; i < nMaxMonitorWall; ++i)
			{
				pstuWall[i].nStructSize = sizeof(AV_CFG_MonitorWall);
				for (int j = 0; j < AV_CFG_Max_Block_In_Wall; ++j)
				{
					pstuWall[i].stuBlocks[j].nStructSize = sizeof(AV_CFG_MonitorWallBlock);
					for (int k = 0; k < AV_CFG_Max_TV_In_Block; ++k)
					{
						pstuWall[i].stuBlocks[j].stuTVs[k].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
					}
				}
			}
			
			int nRetLen = 0;
			if (CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf, pstuWall, pstuWall->nStructSize * nMaxMonitorWall, &nRetLen))
			{							
				for (int i = 0; i < pstuWall->nBlockCount; ++i)
				{
					int j = 0;
					// block是否融合
					bool bComposite = false;
					for (j = 0; j < m_nSpliceCount; ++j)
					{
						const AV_CFG_SpliceScreen* pScreen = pstuScreen + j;
						if (strcmp(pScreen->szWallName, pstuWall->szName) == 0 && pScreen->nBlockID == i)
						{
							bComposite = true;
							break;
						}
					}

					if (!bComposite) continue;

					const AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[i];					

					// 将block中的tv在output map中排除
					for (j = 0; j < block.nTVCount; ++j)
					{
						const AV_CFG_MonitorWallTVOut& tv = block.stuTVs[j];
					
						CDecChannel* pChn = GetDecodeChannel(tv.nChannelID);
						if (pChn)
						{
							pChn->m_bPartOfSpliceScreen = TRUE;
						}
					}
				}
			}						
			delete[] pstuWall;
		}
		delete[] pstuScreen;
	}
	delete[] pszJsonBuf;
	
	return TRUE;
}
*/
BOOL CDecoderNVD::UpdateNormalChannel()
{
    for (int i = 0; i < m_stuProdDef.nVideoOutChannel; ++i)
    {					
        DH_SPLIT_CAPS stuCaps = { sizeof(DH_SPLIT_CAPS) };
        CLIENT_GetSplitCaps(m_lLoginID, i, &stuCaps);
        
        CDecChannel stuChannel(i, -1, -1, stuCaps.nMaxSourceCount);
        for (int k = 0; k < min(stuCaps.nModeCount, _countof(stuChannel.m_nSplitModes)); ++k)
        {
            stuChannel.m_nSplitModes[k] = stuCaps.emSplitMode[k];
        }
        m_mapDecChn[i] = stuChannel;
	}
    return TRUE;
}

/************************************************************************/
/*                               解码DSCON	                            */
/************************************************************************/

CDecoderDSCON::CDecoderDSCON( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CDecoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

BOOL CDecoderDSCON::UpdateNormalChannel()
{	
	// 查询子卡信息
	DH_MATRIX_CARD_LIST stuCards = {sizeof(stuCards)};
	for (int i = 0; i < _countof(stuCards.stuCards); ++i)
	{
		stuCards.stuCards[i].dwSize = sizeof(DH_MATRIX_CARD);
	}
	
	if (CLIENT_QueryMatrixCardInfo(m_lLoginID, &stuCards))
	{
		for (int i = 0; i < stuCards.nCount; ++i)
		{
			const DH_MATRIX_CARD& stuCard = stuCards.stuCards[i];
			if (!stuCard.bEnable)
				continue;
			
			if (stuCard.dwCardType & DH_MATRIX_CARD_OUTPUT)
			{
				for (int j = stuCard.nVideoOutChnMin; j <= stuCard.nVideoOutChnMax; ++j)
				{					
					CDecChannel stuChannel(j, i, j - stuCard.nVideoOutChnMin);
					m_mapDecChn[j] = stuChannel;
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}





/************************************************************************/
/*                              编码设备                                */
/************************************************************************/

CEncoder::CEncoder( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/, LPCTSTR lpszDevClass /*= NULL*/ )
: CDevice(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, lpszDevClass == NULL ? DCID() : lpszDevClass)
{
	m_bOnline = FALSE;
	m_strDevClass = "";
	m_strDevModel = "";
}

BOOL CEncoder::Update()
{
	ASSERT(m_lLoginID);
	return UpdateNormalChannel();
}

BOOL CEncoder::UpdateNormalChannel()
{
	m_mapEncChn.clear();

    int nRetLen = 0;
    NET_DEV_CHN_COUNT_INFO stuChnInfo = { sizeof(stuChnInfo) };
    stuChnInfo.stuVideoIn.dwSize = sizeof(stuChnInfo.stuVideoIn);
    if (CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_DEV_CHN_COUNT, (char*)&stuChnInfo, stuChnInfo.dwSize, &nRetLen))
    {
        for (int i = 0; i < stuChnInfo.stuVideoIn.nMaxTotal; ++i)
        {
            CEncChannel stuChannel(i, -1, -1);
            m_mapEncChn[i] = stuChannel;
        }
		return TRUE;
    }
    return FALSE;
}

CEncChannel* CEncoder::GetEncodeChannel( int nChannel )
{
	CEncChnMap::iterator it = m_mapEncChn.find(nChannel);
	if (it != m_mapEncChn.end())
	{
		return &it->second;
	}
	return NULL;
}

/************************************************************************/
/*                              编码矩阵                                */
/************************************************************************/

CEncoderMatrix::CEncoderMatrix( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CEncoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

BOOL CEncoderMatrix::UpdateNormalChannel()
{
	BOOL bRet = FALSE;
	DH_IN_MATRIX_GET_CAMERAS stuInParam = { sizeof(DH_IN_MATRIX_GET_CAMERAS) };
	DH_OUT_MATRIX_GET_CAMERAS stuOutParam = { sizeof(DH_OUT_MATRIX_GET_CAMERAS) };
	stuOutParam.nMaxCameraCount = 4096;
	stuOutParam.pstuCameras = new DH_MATRIX_CAMERA_INFO[stuOutParam.nMaxCameraCount];
	ZeroMemory(stuOutParam.pstuCameras, sizeof(DH_MATRIX_CAMERA_INFO) * stuOutParam.nMaxCameraCount);
	for (int i = 0; i < stuOutParam.nMaxCameraCount; ++i)
	{
		stuOutParam.pstuCameras[i].dwSize = sizeof(DH_MATRIX_CAMERA_INFO);
		stuOutParam.pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
	}
	
	if (CLIENT_MatrixGetCameras(m_lLoginID, &stuInParam, &stuOutParam))
	{
		for (int i = 0; i < stuOutParam.nRetCameraCount; ++i)
		{
			DH_MATRIX_CAMERA_INFO& stuInfo = stuOutParam.pstuCameras[i];
			CEncChannel stuChannel(stuInfo.nUniqueChannel);
			stuChannel.m_strName.Format("%s_%s_%d_%d", stuInfo.szName, stuInfo.szDevID, stuInfo.nChannelID, stuInfo.nUniqueChannel);
			stuChannel.m_strDevID = stuInfo.szDevID;
			m_mapEncChn[stuChannel.m_nChannelID] = stuChannel;
		}
		bRet = TRUE;
	}
	delete[] stuOutParam.pstuCameras;
	return bRet;

	/*
	// 矩阵需要查询子卡信息
	DH_MATRIX_CARD_LIST stuCards = {sizeof(stuCards)};
	for (int i = 0; i < _countof(stuCards.stuCards); ++i)
	{
		stuCards.stuCards[i].dwSize = sizeof(DH_MATRIX_CARD);
	}
	
	if (CLIENT_QueryMatrixCardInfo(m_lLoginID, &stuCards))
	{
		for (int i = 0; i < stuCards.nCount; ++i)
		{
			const DH_MATRIX_CARD& stuCard = stuCards.stuCards[i];
			if (!stuCard.bEnable)
				continue;
			
			if (stuCard.dwCardType & DH_MATRIX_CARD_ENCODE)
			{
				for (int j = stuCard.nVideoEncChnMin; j <= stuCard.nVideoEncChnMax; ++j)
				{					
					CEncChannel stuChannel(j, i, j - stuCard.nVideoEncChnMin);
					m_mapEncChn[j] = stuChannel;
				}
			}
		}
		return TRUE;
	}
	return FALSE;
	//*/
}

/************************************************************************/
/*                              编码NVR	                                */
/************************************************************************/

CEncoderNVR::CEncoderNVR( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CEncoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

/************************************************************************/
/*                               编码NVD	                            */
/************************************************************************/

CEncoderNVD::CEncoderNVD( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CEncoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

BOOL CEncoderNVD::UpdateNormalChannel()
{
	return TRUE;
}


/************************************************************************/
/*                               编码DSCON	                            */
/************************************************************************/

CEncoderDSCON::CEncoderDSCON( LLONG lLoginID, LPCTSTR lpszIP, int nPort /*= 37777*/, LPCTSTR lpszUser /*= NULL*/, LPCTSTR lpszPwd /*= NULL*/ )
: CEncoder(lLoginID, lpszIP, nPort, lpszUser, lpszPwd, DCID())
{
	
}

BOOL CEncoderDSCON::UpdateNormalChannel()
{
	//*
	BOOL bRet = FALSE;
	DH_IN_MATRIX_GET_CAMERAS stuInParam = { sizeof(DH_IN_MATRIX_GET_CAMERAS) };
	DH_OUT_MATRIX_GET_CAMERAS stuOutParam = { sizeof(DH_OUT_MATRIX_GET_CAMERAS) };
	stuOutParam.nMaxCameraCount = 1024;
	stuOutParam.pstuCameras = new DH_MATRIX_CAMERA_INFO[stuOutParam.nMaxCameraCount];
	ZeroMemory(stuOutParam.pstuCameras, sizeof(DH_MATRIX_CAMERA_INFO) * stuOutParam.nMaxCameraCount);
	for (int i = 0; i < stuOutParam.nMaxCameraCount; ++i)
	{
		stuOutParam.pstuCameras[i].dwSize = sizeof(DH_MATRIX_CAMERA_INFO);
		stuOutParam.pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
	}

	if (CLIENT_MatrixGetCameras(m_lLoginID, &stuInParam, &stuOutParam))
	{
		for (int i = 0; i < stuOutParam.nRetCameraCount; ++i)
		{
			DH_MATRIX_CAMERA_INFO& stuInfo = stuOutParam.pstuCameras[i];
			CEncChannel stuChannel(stuInfo.nUniqueChannel);
			stuChannel.m_strName.Format("%s_%s_%d_%d", stuInfo.szName, stuInfo.szDevID, stuInfo.nChannelID, stuInfo.nUniqueChannel);
			stuChannel.m_strDevID = stuInfo.szDevID;
			m_mapEncChn[stuChannel.m_nChannelID] = stuChannel;
		}
		bRet = TRUE;
	}
	delete[] stuOutParam.pstuCameras;
	return bRet;
	//*/

	/*
	int nChnIndex = 0;
	const int nMaxJsonBuf = 64 * 1024;
	char* pszJsonBuf = new char[nMaxJsonBuf];
	ZeroMemory(pszJsonBuf, nMaxJsonBuf);	
	BOOL bRet = FALSE;
	
	// 取模拟矩阵配置
	if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_ANALOGMATRIX, -1, pszJsonBuf, nMaxJsonBuf, NULL))
	{
		int i = 0;
		int nAnalogMatrixCount = 128;
		CFG_ANALOG_MATRIX_INFO* pstuMatrixs = new CFG_ANALOG_MATRIX_INFO[nAnalogMatrixCount];
		ZeroMemory(pstuMatrixs, sizeof(CFG_ANALOG_MATRIX_INFO) * nAnalogMatrixCount);
		for (i = 0; i < nAnalogMatrixCount; ++i)
		{
			pstuMatrixs[i].nChnInSrcCount = 128;
			pstuMatrixs[i].pstuChnInSrc = new CFG_ANALOG_MATRIX_INPUT_CHN_SRC[pstuMatrixs[i].nChnInSrcCount];
			ZeroMemory(pstuMatrixs[i].pstuChnInSrc, sizeof(CFG_ANALOG_MATRIX_INPUT_CHN_SRC) * pstuMatrixs[i].nChnInSrcCount);
		}
		
		int nRetLen = 0;
		if (CLIENT_ParseData(CFG_CMD_ANALOGMATRIX, pszJsonBuf, pstuMatrixs, sizeof(CFG_ANALOG_MATRIX_INFO) * nAnalogMatrixCount, &nRetLen))
		{
			int nCount = nRetLen / sizeof(CFG_ANALOG_MATRIX_INFO);			
			for (int i = 0; i < nCount; ++i)
			{
				CFG_ANALOG_MATRIX_INFO* pDev = pstuMatrixs + i;
				for (int j = 0; j < pDev->nRetChnInSrcCount; ++j)
				{
					CFG_ANALOG_MATRIX_INPUT_CHN_SRC* pstuSrc = pDev->pstuChnInSrc + j;
					CEncChannel stuChannel(j);
					stuChannel.m_strName.Format("[A]%s", pstuSrc->szName);
					stuChannel.m_strAnalogMatrix = pDev->szName;
					m_mapEncChn[nChnIndex++] = stuChannel;
				}
			}
			bRet = TRUE;
		}

		for (i = 0; i < nAnalogMatrixCount; ++i)
		{
			delete[] pstuMatrixs[i].pstuChnInSrc;
		}
		delete[] pstuMatrixs;
	}

	// 获取远程设备信息
	if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_REMOTEDEVICE, -1, pszJsonBuf, nMaxJsonBuf, NULL))
	{
		int i = 0;
		int nRemoteDeviceCount = 128;
		AV_CFG_RemoteDevice* pstuRemoteDevices = new AV_CFG_RemoteDevice[nRemoteDeviceCount];
		ZeroMemory(pstuRemoteDevices, sizeof(AV_CFG_RemoteDevice) * nRemoteDeviceCount);
		for (i = 0; i < nRemoteDeviceCount; ++i)
		{
			pstuRemoteDevices[i].nStructSize = sizeof(AV_CFG_RemoteDevice);
		}
		
		int nRetLen = 0;
		if (CLIENT_ParseData(CFG_CMD_REMOTEDEVICE, pszJsonBuf, pstuRemoteDevices, sizeof(AV_CFG_RemoteDevice) * nRemoteDeviceCount, &nRetLen))
		{
			int nCount = nRetLen / sizeof(AV_CFG_RemoteDevice);			
			for (int i = 0; i < nCount; ++i)
			{
				AV_CFG_RemoteDevice* pDev = pstuRemoteDevices + i;
				for (int j = 0; j < pDev->nVideoChannel; ++j)
				{
					CEncChannel stuChannel(j);
					stuChannel.m_strName.Format("[R]%s_%d", pDev->szID, j);
					stuChannel.m_strRemoteDevice = pDev->szID;
					m_mapEncChn[nChnIndex++] = stuChannel;
				}
			}
			bRet = TRUE;
		}
		delete[] pstuRemoteDevices;

	}

	delete pszJsonBuf;
	return bRet;
	//*/
}