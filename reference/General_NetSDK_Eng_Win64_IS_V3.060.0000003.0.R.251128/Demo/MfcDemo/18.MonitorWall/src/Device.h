// Device.h: interface for the CDevice class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "Util.h"

class CDevice;
class CDeviceFactory
{
public:
	enum { Decoder, Encoder };

	static CDevice* Create(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL, LPCTSTR lpszDevClass = "Unknown", UINT nType = Decoder);
};

// 解码设备通道
class CDecChannel
{
public:
	CDecChannel(int _nChannelID = -1, int _nCardID = -1, int _nCardSlot = -1, int _nMaxSources = 0);
	
	LPCTSTR ToString() const;
	
public:
	int						m_nChannelID;				// 通道号
	int						m_nCardID;					// 卡号
	int						m_nCardSlot;				// 卡槽
	int						m_nMaxSources;				// 支持的最大显示源配置数
	UINT					m_nSplitModes[32];			// 支持的分割模式, 以0结尾
	BOOL					m_bPartOfSpliceScreen;		// 融合屏的组成部分, 即本通道和其他通道一起组成一个融合屏
	BOOL					m_bSpliceScreen;			// 是否融合屏
	CString					m_strName;					// 融合屏名称
};
typedef std::map<int, CDecChannel> CDecChnMap;

// 编码设备通道
class CEncChannel
{
public:
	CEncChannel(int _nChannelID = -1, int _nCardID = -1, int _nCardSlot = -1);
	
	LPCTSTR ToString() const;
	
public:
	int						m_nChannelID;				// 通道号
	int						m_nCardID;					// 卡号
	int						m_nCardSlot;				// 卡槽
	CString					m_strName;					// 通道名
	CString					m_strAnalogMatrix;			// 所属模拟矩阵, 大屏有效
	CString					m_strRemoteDevice;			// 所属远程设备ID, 大屏有效
	CString					m_strDevID;					// 设备ID, AnalogMatrixID or RemoteDeviceID
};
typedef std::map<int, CEncChannel> CEncChnMap;

// 基本设备信息
class CDevice
{	
public:
	CDevice(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL, LPCTSTR lpszDevClass = "Unknown");
	
	LPCTSTR GetDeviceClass() const { return m_pszDeviceClass; }

	LLONG	GetHandle() const { return m_lLoginID; }

	LPCTSTR GetIP() const { return m_strIP; }
	int		GetPort() const { return m_nPort; }
	LPCTSTR GetUser() const { return m_strUser; }
	LPCTSTR GetPwd() const { return m_strPwd; }

	void	SetHandle(LLONG lLoginID) { m_lLoginID = lLoginID; }
	void	SetLoginInfo(LPCTSTR lpszIP, int nPort, LPCTSTR lpszUser, LPCTSTR lpszPwd);
	
protected:
	LLONG		m_lLoginID;
	UINT		m_nPort;
	CString		m_strIP;
	CString		m_strUser;
	CString		m_strPwd;
	LPCTSTR		m_pszDeviceClass;	// 设备类型
};

#define DECLARE_DEVICE_CLASS(s)	static LPCTSTR DCID() { return s; }

// 解码设备
class CDecoder : public CDevice
{
public:
	CDecoder(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL, LPCTSTR lpszDevClass = NULL);
	
	DECLARE_DEVICE_CLASS("Decoder");

	virtual BOOL Update();
	virtual BOOL UpdateCompositeChannel();
	virtual BOOL UpdateNormalChannel();

	CDecChannel* GetDecodeChannel(int nChannel); 
	const CDecChannel* GetDecodeChannel(int nChannel) const; 

	CDecChnMap&	 GetDecChnMap() { return m_mapDecChn; }
	const CDecChnMap& GetDecChnMap() const { return m_mapDecChn; }
	
protected:
	CDecChnMap	m_mapDecChn;		// 解码通道信息
	DH_PRODUCTION_DEFNITION m_stuProdDef;
	int			m_nSpliceCount;		// 拼接屏数量
};

// 解码矩阵
class CDecoderMatrix : public CDecoder
{
public:
	CDecoderMatrix(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);

	DECLARE_DEVICE_CLASS("Matrix");

	virtual BOOL UpdateNormalChannel();
};

// 解码NVR
class CDecoderNVR : public CDecoder
{
public:
	CDecoderNVR(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);

	DECLARE_DEVICE_CLASS("NVR");

	virtual BOOL UpdateCompositeChannel();
};

// 解码NVD
class CDecoderNVD : public CDecoder
{
public:
	CDecoderNVD(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);
	
	DECLARE_DEVICE_CLASS("NVD");

    virtual BOOL UpdateNormalChannel();
	//virtual BOOL UpdateCompositeChannel();
};

// 解码大屏控制器
class CDecoderDSCON : public CDecoder
{
public:
	CDecoderDSCON(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);
	
	DECLARE_DEVICE_CLASS("DSCON");
	
	virtual BOOL UpdateNormalChannel();
};







/************************************************************************/
/*                              编码设备                                */
/************************************************************************/

// 编码设备
class CEncoder : public CDevice
{
public:
	CEncoder(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL, LPCTSTR lpszDevClass = NULL);

	DECLARE_DEVICE_CLASS("Encoder");

	virtual BOOL Update();
	virtual BOOL UpdateNormalChannel();
	
	CEncChannel* GetEncodeChannel(int nChannel); 
	CEncChnMap&	 GetEncChnMap() { return m_mapEncChn; }

	void		Online(BOOL bOnline = TRUE) { m_bOnline = bOnline; }
	BOOL		IsOnline() const { return m_bOnline; }
	
protected:
	CEncChnMap	m_mapEncChn;		// 编码通道信息
	BOOL		m_bOnline;			// 在线状态

public:
	CString		m_strDevClass;		// 设备类型, 添加设备时由用户输入
	CString		m_strDevModel;		// 设备型号, 添加设备时由用户输入
};

// 编码矩阵
class CEncoderMatrix : public CEncoder
{
public:
	CEncoderMatrix(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);
	
	DECLARE_DEVICE_CLASS("Matrix");

	virtual BOOL UpdateNormalChannel();
	
protected:
};

// 编码NVR
class CEncoderNVR : public CEncoder
{
public:
	CEncoderNVR(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);
	
	DECLARE_DEVICE_CLASS("NVR");
	
protected:
};

// 编码NVD
class CEncoderNVD : public CEncoder
{
public:
	CEncoderNVD(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);
	
	DECLARE_DEVICE_CLASS("NVD");

	virtual BOOL UpdateNormalChannel();
	
protected:
};

// 编码大屏控制器
class CEncoderDSCON : public CEncoder
{
public:
	CEncoderDSCON(LLONG lLoginID, LPCTSTR lpszIP, int nPort = 37777, LPCTSTR lpszUser = NULL, LPCTSTR lpszPwd = NULL);
	
	DECLARE_DEVICE_CLASS("DSCON");
	
	virtual BOOL UpdateNormalChannel();
	
protected:
};

#endif
