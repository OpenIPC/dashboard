#ifndef _DHPLAY_H
#define _DHPLAY_H

#if (defined(WIN32) || defined(WIN64))
	#ifdef dhplay_EXPORTS
		#define PLAYSDK_API __declspec(dllexport)
	#else
		#define PLAYSDK_API __declspec(dllimport)
	#endif
	#define CALLMETHOD __stdcall
	#define CALLBACK   __stdcall
#else
	#define CALLMETHOD
	#define CALLBACK
	#define PLAYSDK_API //extern "C"
#endif

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#ifdef __OBJC__
#include "objc/objc.h"
#else
#define BOOL        int
#endif
#define BYTE		unsigned char
#define PBYTE		BYTE*
#define LPBYTE		BYTE*
#ifndef LONG
#define LONG		int
#endif
#ifndef DWORD
#define DWORD		unsigned int
#endif
#define WORD		unsigned short
#define COLORREF	DWORD
#define HDC			void*
#define HWND		void*
#define LPSTR		char*
#define UINT		unsigned int
#define TRUE		1
#define FALSE		0
#define ULONGLONG	unsigned long long
#define LONGLONG	long long

typedef struct _SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/***********************************************************************************************/
/* 常量定义																					   */
/***********************************************************************************************/

/* 通道号 */
#define  FUNC_MAX_PORT					511		// 最大通道号，通道号范围[0,512)

/* 音频幅值 */
#define MIN_WAVE_COEF					-100	// PLAY_AdjustWaveAudio使用
#define MAX_WAVE_COEF					100		// PLAY_AdjustWaveAudio使用

/* 音频采集长度 */
#define MIN_AUDIO_RECORD_LEN			320		// 最小音频采集长度
#define MAX_AUDIO_RECORD_LEN			4096	// 最大音频采集长度

/* 显示相关 */
#define MAX_DISPLAY_WND					64      // 设置的区域序号的最大值

/* 播放缓存相关 */
#define BUF_VIDEO_SRC					1		// 视频源缓冲
#define BUF_AUDIO_SRC					2		// 音频源缓冲
#define BUF_VIDEO_RENDER				3		// 解码后视频数据缓冲
#define BUF_AUDIO_RENDER				4		// 解码后音频数据缓冲

/* 文件跳转方式 */
#define BY_FRAMENUM						1		// 按帧号方式(PLAY_GetKeyFramePos使用)
#define BY_FRAMETIME					2		// 按时间方式(PLAY_GetKeyFramePos使用)

/* 数据流相关 */
#define SOURCE_BUF_MAX					1024*100000	// 数据流缓冲区最大长度
#define SOURCE_BUF_MIN					1024*1024	// 数据流缓冲区最小长度

#define STREAME_REALTIME				0		// 实时流模式
#define STREAME_FILE					1		// 文件流模式

/* 音频位宽 */
#define T_AUDIO16						101		// 16位音频数据类型
#define T_AUDIO8						100		// 8位音频数据类型

/* 解码后的视频格式 */
#define T_UYVY							1		// UYVY类型的YUV数据，现在不支持
#define T_IYUV							3		// IYUV(I420)类型YUV数据
#define T_NV12							5		// NV12类型yuv数据，现在只支持bmp32抓图
#define T_RGB32							7		// RGB32类型，现在不支持

/* avi转码 */
#define AVI_MEDIACHANGE_FRAMERATE		1		// 帧率改变
#define AVI_MEDIACHANGE_RESOLUTION		2		// 分辨率改变

/* 水印相关 */
#define WATERMARK_DATA_TEXT				0		// 文字
#define WATERMARK_DATA_JPEG_BMP			1		// JPEG或者BMP图片
#define WATERMARK_DATA_FRAMEDATA		3		// 帧数据

/* 错误码 */
#define  DH_PLAY_NOERROR							0	//无错误
#define  DH_PLAY_COMMON_ERROR						1	//普通错误
#define	 DH_PLAY_PARA_INVALID						2	//参数无效
#define  DH_PLAY_ORDER_ERROR						3	//调用顺序不对
#define	 DH_PLAY_PORT_OPEN							4	//通道已经被打开
#define	 DH_PLAY_PORT_CLOSE							5	//通道已经被关闭
#define	 DH_PLAY_PORT_INVALID						6	//通道号无效
#define	 DH_PLAY_PORT_EXIST							7	//通道已经存在
#define  DH_PLAY_OPEN_FILE_ERROR					8	//打开文件失败
#define  DH_PLAY_INTERFACE_NOT_SUPPORT				9	//接口不支持
#define  DH_PLAY_HWND_INVALID						10	//窗口句柄无效
#define  DH_PLAY_PLAY_ERROR							11	//播放失败
#define  DH_PLAY_SPEED_INVALID						12	//速度无效
#define  DH_PLAY_NOT_FILE							13	//非文件模式
#define  DH_PLAY_NOT_STREAM							14	//非流模式
#define  DH_PLAY_NO_FRAME							15	//当前没有帧可用
#define  DH_PLAY_INDEX_NOT_COMPLETE					16	//索引没有建立完成
#define  DH_PLAY_INDEX_COMPLETE						17	//已建立文件索引
#define  DH_PLAY_GET_FILE_SIZE_ERROR				18	//获取文件大小失败
#define  DH_PLAY_CREATE_THREAD_FAIL					19	//创建线程失败
#define  DH_PLAY_CREATE_EVENT_FAIL					20	//创建句柄失败
#define  DH_PLAY_SOUND_SHARE_MODE					21	//处于共享声音模式
#define  DH_PLAY_INCLUDE_SOUND_SHARE_PORT			22	//该端口已包含
#define  DH_PLAY_NOT_INCLUDE_SOUND_SHARE_PORT		23	//该端口未包含
#define  DH_PLAY_CREATE_DIR_ERROR					24	//创建路径失败
#define  DH_PLAY_CREATE_FILE_ERROR					25	//创建文件失败

#define  DH_PLAY_CONVERT_YUV_ERROR					26	//转到yuv失败
#define  DH_PLAY_CONVERT_JPG_ERROR					27	//转到jpg失败
#define  DH_PLAY_CONVERT_BMP_ERROR					28	//转到bmp失败
#define  DH_PLAY_CONVERT_TIFF_ERROR					29	//转到tiff失败
#define  DH_PLAY_HW_CATCH_ERROR					    30	//硬解码抓拍失败
#define  DH_PLAY_CREATE_VIDEO_RENDER_ERROR			31	//创建视频渲染失败
#define  DH_PLAY_NOT_SUPPORT_REF_VALUE				32	//不支持外部索引设置
#define  DH_PLAY_FORMAT_NOT_SUPPORT					33	//格式不支持
#define  DH_PLAY_CREATE_RECORD_ERROR				34	//创建录像失败
#define  DH_PLAY_OPEN_RECORD_ERROR					35	//打开录像失败
#define  DH_PLAY_FRAMERATE_ERROR					36	//帧率错误
#define  DH_PLAY_CREATE_AUDIO_RECORD_ERROR			37	//创建音频录像失败
#define  DH_PLAY_OPEN_AUDIO_RECORD_ERROR			38	//打开音频录像失败
#define	 DH_PLAY_AES_ALLOC_ERROR					39	//调用aes_alloc失败

#define  DH_PLAY_BUF_OVER				            40  //缓冲区已满
#define  DH_PLAY_ALLOC_MEMORY_ERROR		            41  //分配内存失败


/***********************************************************************************************/
/* 枚举定义																					   */
/***********************************************************************************************/
																								
/* 渲染模式 */
typedef enum
{
	RENDER_NOTSET = 0,				// 未设置
	RENDER_GDI,						// GDI渲染
	RENDER_X11 = RENDER_GDI,		// 非windows平台X11渲染
	RENDER_DDRAW,					// ddraw渲染	
	RENDER_OPENGL = RENDER_DDRAW,	// 非windows平台opengl渲染
    RENDER_D3D,						// D3D渲染,默认等同于D3D9渲染
	RENDER_D3D9 = RENDER_D3D,       // D3D9渲染
	RENDER_WGL,						// windows平台opengl渲染
	RENDER_D3D11                    // D3D11渲染
}RenderType;

/* 解码模式 */
typedef enum
{
	DECODE_NOTSET = 0,		// 未设置
	DECODE_SW,				// 软解
	DECODE_HW,				// 硬解码拷贝模式(如使用windows平台，使用d3d9接口)
	DECODE_HW_FAST,			// 硬解码直接显示模式(如使用windows平台，使用d3d9接口)
	DECODE_MSDK,			// 硬解码，调用intel media sdk，已废弃
	DECODE_HW_FAST_D3D11,		// 硬解码直接显示模式,使用d3d11接口,仅限windows平台
}DecodeType;

/* 图片格式 */
typedef enum __tPicFormats
{
	PicFormat_BMP = 0,				        // BMP32类型
	PicFormat_JPEG,							// JPEG类型
	PicFormat_JPEG_70,						// 70%质量的JPEG类型
	PicFormat_JPEG_50,						// 50%质量的JPEG类型
	PicFormat_JPEG_30,						// 30%质量的JPEG类型
	PicFormat_JPEG_10,						// 10%质量的JPEG类型
	PicFormat_BMP24,                        // BMP24类型
	PicFormat_TIFF							// TIFF类型
} tPicFormats;

/* 获取媒体信息格式 */
typedef enum _CMD_TYPE
{
	PLAY_CMD_GetTime = 1,			// 编码中时间信息，为公历年月日
	PLAY_CMD_GetFileRate = 2,		// 帧率信息
	PLAY_CMD_GetMediaInfo = 3,		// 媒体信息
	PLAY_CMD_GetRenderNum = 4,		// 当前要渲染的帧号
	PLAY_CMD_GetRenderTime = 5,		// 当前要渲染的时间，绝对时间
	PLAY_CMD_GetSrcTime	= 6,		// 编码中时间信息，为1970年7月1日后后持续的秒数
	PLAY_CMD_GetCurRenderNum = 7,   // 当前帧号，PLAY_CMD_GetRenderNum是上一帧的帧号
    PLAY_CMD_GetRenderTimeStamp = 8 // 当前要渲染的时间戳，相对时间戳
} CMD_TYPE_E;

/* 音视频同步策略 */
typedef enum
{
	AV_SYNC_VIDEO_MASTER,		  // 以视频为基准（非音视频同步方式）
	AV_SYNC_AUDIO_TIME_STAMP      // 以音频时间戳为基准
}AV_SYNC_TYPE;

/***********************************************************************************************/
/* 结构体定义																				   */
/***********************************************************************************************/

/* 局部显示区域 */
typedef struct _tagRECT
{
	LONG left;								
	LONG top;								
	LONG right;								
	LONG bottom;							
}DISPLAYRECT;

/* 时间结构 */
#ifndef _DHTIME_
#define _DHTIME_
typedef struct _DHTIME								
{
	DWORD second	:6;						//	秒	0-59		
	DWORD minute	:6;						//	分	0-59		
	DWORD hour		:5;						//	时	0-23		
	DWORD day		:5;						//	日	1-31		
	DWORD month		:4;						//	月	1-12		
	DWORD year		:6;						//	年	2000-2063	
} DHTIME,*pDHTIME;
#endif

/* 文件信息结构，本地使用 */
#define UUID_MAX_LEN		96		//分区唯一标识长度
#define MAX_DEV_NAME_LEN	32		//分区名称长度
typedef struct _FILE_INFO
{
	UINT			channel;				//通道号
	BYTE			type;					//文件类型,按照数值计算
	BYTE			lock_flag;				//锁定类型(类型定义见dhfs_file_state)
	BYTE			video_audio;			//视频或音频
	BYTE			image;					//bit0~3图像质量(bit4~6保留, bit7 表示夏令时)

	DHTIME			start_time;				//开始时间
	DHTIME			end_time;				//结束时间

	UINT			file_length;			//文件长度，单位为KB

	UINT			first_clus_no;			//文件首簇号
	UINT			ud_no;					//用户数据号

	char			part[MAX_DEV_NAME_LEN];	//文件所在的分区名称, 如:"dev/sda_0"
	char			uuid[UUID_MAX_LEN];		//文件所在分区uuid,每个分区的唯一标识,重新卸载|挂载后会更新

	UINT		rev[9];
}FILE_INFO,*pFILE_INFO;

/* 帧位置信息 */
typedef struct
{
	LONGLONG		nFilePos;				// 指定帧在文件中的偏移位置
	LONG			nFrameLen;				// 帧长度
	LONG			nFrameNum;				// 帧序号
	LONG			nFrameTime;				// 帧时间
	LONG			nErrorFrameNum;			// 保留，暂无使用
	SYSTEMTIME*		pErrorTime;				// 保留，暂无使用
	LONG			nErrorLostFrameNum;		// 保留，暂无使用
	LONG			nErrorFrameSize;		// 保留，暂无使用
}FRAME_POS,*PFRAME_POS;

/* 帧信息 */
typedef struct
{
	LONG			nWidth;					// 画面宽，单位像素。如果是音频数据则为0
	LONG			nHeight;				// 画面高，如果是音频数据则为0
	LONG			nStamp;					// 时标信息，单位毫秒
	LONG			nType;					// 视频帧类型，T_AUDIO16，T_RGB32，T_IYUV
	LONG			nFrameRate;				// 视频表示帧率，音频表示采样率
}FRAME_INFO;

/* 帧信息扩展字段 */
typedef struct
{
	#define FRAME_TYPE_VIDEO 0				// 视频帧
	#define FRAME_TYPE_AUDIO 1				// 音频帧
	int				nFrameType;				// 视频帧类型，见上面定义
	int				nFrameSeq;				// 帧序号
	int				nStamp;					// 时标信息，单位毫秒
	int				nWidth;					// 画面宽，单位像素。如果是音频数据则为0
	int 			nHeight;				// 画面高，如果是音频数据则为0
	int				nFrameRate;				// 编码时产生的图像帧率
	int				nChannels;				// 音频通道数
	int				nBitPerSample;			// 音频采样位数
	int				nSamplesPerSec;			// 音频采样频率
	int				nRemainData;			// 缓冲剩余数据量
	SYSTEMTIME		nDataTime;				// 时间
	int				nFrameSubType;			// 视频帧子类型
	int				nReserved[58];			// 保留字段
}FRAME_INFO_EX;

/* 解码后的帧信息 */
typedef struct 
{
	int				nFrameType;					// 帧类型，定义见FRAME_INFO_EX里nFrameType字段
	void*			pAudioData;				    // 音频数据，如果是音频帧
	int				nAudioDataLen;			    // 音频数据长度
	void*			pVideoData[3];			    // 分别表示视频的YUV三个分量
	int				nStride[3];				    // 分别表示YUV三个分量的跨距
	int				nWidth[3];				    // 分别表示YUV三个分量的宽度
	int				nHeight[3];					// 分别表示YUV三个分量的高度
    SYSTEMTIME		nDataTime;				    // 帧时间
	int				nReserved[60];				// 保留字段
}FRAME_DECODE_INFO;

/* 媒体信息 */
typedef struct 
{
	int			lWidth;					//	画面宽，单位像素
	int			lHeight;				//	画面高
	int			lFrameRate;				//	帧率
	int			lChannel;				//	音频通道数
	int			lBitPerSample;			//	音频采样位数
	int			lSamplesPerSec;			//	音频采样频率
}MEDIA_INFO;

/***********************************************************************************************/
/* 接口																						   */
/***********************************************************************************************/

/************************************************************************/
//> 获取版本号
/************************************************************************/

/**
 * 获取播放库SDK主版本号，次版本号和SVN版本号。
 *
 * @return 最高1位表示当前的主版本号；第2~3位表示次版本号；其余的表示SVN版本号。
 *         如：返回值34033722表示：主版本号是3，次版本号是40，SVN版本号是33722。
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetSdkVersion();

/************************************************************************/
//> 错误码
/************************************************************************/

/**
 * 获取错误码。
 *
 * @return 错误码
 */
PLAYSDK_API DWORD  CALLMETHOD PLAY_GetLastErrorEx();

/************************************************************************/
//> 日志开关
/************************************************************************/

/**
 * 日志调试开关，默认为Warn等级，打印较少。如果需要开启详细日志打印，建议为Debug等级。
 * Windows平台生成为当前库目录下的playsdk.log文件，其他非windows为控制台打印。
 *
 */
typedef enum
{
    LOG_LevelUnknown= 0, // 未知等级
    LOG_LevelFatal,	     // fatal等级，当设置为此等级时，有一种打印输出（fatal）都有输出
    LOG_LevelError,	     // error等级，当设置为此等级时，有两种打印输出（fatal，error）都有输出
    LOG_LevelWarn,	     // warn等级，当设置为此等级时，有三种打印输出（fatal，error，warn）都有输出
    LOG_LevelInfo,	     // info等级，当设置为此等级时，有四种打印输出（fatal，error，warn，info）都有输出
    LOG_LevelTrace,	     // Trace等级，当设置为此等级时，有五种打印输出（fatal，error，warn，info，trace）都有输出
    LOG_LevelDebug	     // Debug等级，当设置为此等级时，以上六种打印（fatal，error，warn，info，trace，debug）都有输出
}LOG_LEVEL;
PLAYSDK_API void CALLMETHOD PLAY_SetPrintLogLevel(LOG_LEVEL logLevel);

/************************************************************************/
//> 播放端口号
/************************************************************************/

/**
 * 获取空闲通道号，范围在101~511。
 *
 * @param[out] plPort 获取到的空闲通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetFreePort(LONG *plPort);

/**
 * 释放通道号。
 *
 * @param[in] lPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_ReleasePort(LONG lPort);

/************************************************************************/
//> 文件操作
/************************************************************************/

/**
 * 打开播放文件。
 *
 * @param[in] nPort 通道号
 * @param[in] sFileName 文件名
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_OpenFile(LONG nPort,LPSTR sFileName);

/**
 * 关闭文件。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_CloseFile(LONG nPort);

/**
 * 文件结束回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pUserData 用户数据
 */
typedef void (CALLBACK *fFileEndCBFun)(DWORD nPort, void* pUserData);

/**
 * 文件结束回调。
 *
 * @param[in] nPort 通道号
 * @param[out] pFileEndCBFun 文件结束函数回调指针
 * @param[in] pUserData 用户数据
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileEndCallBack(LONG nPort, fFileEndCBFun pFileEndCBFun, void* pUserData);

/************************************************************************/
//> 流操作
/************************************************************************/

/**
 * 设置流播放模式。
 *
 * @param[in] nPort 通道号
 * @param[in] nMode 播放模式，0实时流，1文件流
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetStreamOpenMode(LONG nPort,DWORD nMode);

/**
 * 获取播放模式。
 *
 * @param[in] nPort 通道号
 * @return LONG，STREAM_MODE_REALSTREAM或STREAM_MODE_FILESTREAM
 */
PLAYSDK_API LONG CALLMETHOD PLAY_GetStreamOpenMode(LONG nPort);

/**
 * 打开流。
 *
 * @param[in] nPort 通道号
 * @param[in] pFileHeadBuf 文件头数据
 * @param[in] nSize 文件头长度
 * @param[in] nBufPoolSize 设置播放器中存放数据流的缓冲区大小。范围是SOURCE_BUF_MIN~SOURCE_BUF_MAX
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_OpenStream(LONG nPort,PBYTE pFileHeadBuf,DWORD nSize,DWORD nBufPoolSize);

/**
 * 关闭流。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_CloseStream(LONG nPort);

/**
 * 输入数据流，PLAY_Play之后使用。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 流数据缓冲区地址
 * @param[in] nSize 流数据长度
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_InputData(LONG nPort,PBYTE pBuf,DWORD nSize);


/************************************************************************/
//> 声音相关
/************************************************************************/

/**
 * 以独占方式打开声音。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_PlaySound(LONG nPort);

/**
 * 关闭声音（独占方式）。
 *
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StopSound();

/**
 * 以共享方式打开声音。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_PlaySoundShare(LONG nPort);

/**
 * 关闭声音（共享方式）。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StopSoundShare(LONG nPort);

/**
 * 设置音量。
 *
 * @param[in] nPort 通道号
 * @param[in] nVolume 音量值，范围0-0xFFFF
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetVolume(LONG nPort,WORD nVolume);

/**
 * 获取音量。
 *
 * @param[in] nPort 通道号
 * @return WORD，音量值
 * @note 这里的音量是指声卡输出的主音量，会影响到其他的声音应用。
 */
PLAYSDK_API WORD CALLMETHOD PLAY_GetVolume(LONG nPort);

/**
 * 音频采集数据回调函数。
 *
 * @param[in] pDataBuffer 数据地址
 * @param[in] DataLength 数据长度
 * @param[in] pUserData 用户数据
 */
typedef void (CALLBACK *pCallFunction)(LPBYTE pDataBuffer, DWORD DataLength, void* pUserData);

/**
 * 打开音频采集功能。
 *
 * @param[in] pProc音频采集数据回调指针
 * @param[in] nBitsPerSample 音频采样位数
 * @param[in] nSamplesPerSec 音频采样率
 * @param[in] nLength 数据缓冲的长度，范围320-4096
 * @param[in] encodetype 对采集数据进行编码设置，0：pcm，1：g711a，2：g711u。pcm回调裸数据，g711a和g711u回调带新大华头的数据
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_OpenAudioRecord(pCallFunction pProc, LONG nBitsPerSample, LONG nSamplesPerSec, LONG nLength, LONG encodetype, void* pUserData);

/**
 * 关闭音频采集功能。
 *
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_CloseAudioRecord();

/**
 * 音频采集变声功能。
 *
 * @param[in] bStart 开启关闭音频采集变声，1-开启，0-关闭， 仅支持Andriod/IOS
 * @param[in] nPitch 声音变化的程度，范围：-50-50，0时声音不变
 * @param[in] bQuick 是快速实现，0-不使能，音质提高，cpu消耗增，1-使能，音质下降，cpu消耗减少 
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SpeechChange(BOOL bStart, int nPitch, BOOL bQuick);

/**
 * 设置音频采集缩放比例。
 *
 * @param[in] fRatio 缩放比例。大于0小于1为音频缩小；1为原始音频；大于1为音频放大
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetAudioRecScaling(float fRatio);

/**
 * 获取音频采集缩放比例。
 *
 * @param[out] pfRatio 缩放比例。大于0小于1为音频缩小；1为原始音频；大于1为音频放大
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetAudioRecScaling(float* pfRatio);

/**
 * 设置音频播放缩放比例。
 *
 * @param[in] nPort 通道号
 * @param[in] fRatio 缩放比例。大于0小于1为音频缩小；1为原始音频；大于1为音频放大。
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetAudioRenderScaling(LONG nPort, float fRatio);

/**
 * 获取音频播放缩放比例。
 *
 * @param[in] nPort 通道号
 * @param[out] pfRatio 缩放比例。大于0小于1为音频缩小；1为原始音频；大于1为音频放大
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetAudioRenderScaling(LONG nPort, float* pfRatio);

/**
 * 调整WAVE波形，可以改变声音的大小。它和PLAY_SetVolume的不同在于，它是调整声音数据，只对该路其作用，而PLAY_SetVolume是
 * 调整声卡音量， 对整个系统起作用。
 *
 * @param[in] nPort 通道号
 * @param[in] nCoefficient 调整参数，0是不调整，其他取值范围从MIN_WAVE_COEF到MAX_WAVE_COEF。若设定为MIN_WAVE_COEF表
 *                         示音量最低，若设定为MAX_WAVE_COEF表示音量最大
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_AdjustWaveAudio(LONG nPort,LONG nCoefficient);

/************************************************************************/
//> 播放控制
/************************************************************************/

/**
 * 开启播放。播放开始，播放视频画面大小将根据hWnd窗口调整，要全屏显示，只要把hWnd窗口放大到全屏。开始解码线程，若送入的显示窗   
 * 口句柄为NULL，则不显示，但是不影响解码。
*
* @param[in] nPort 通道号
* @param[in] hWnd 播放视频的窗口句柄
* @return BOOL，成功返回TURE，失败返回FALSE
* @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_Play(LONG nPort, HWND hWnd);

/**
 * 关闭播放。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_Stop(LONG nPort);

/**
 * 暂停/恢复播放。
 *
 * @param[in] nPort 通道号
 * @param[in] nPause 1：暂停；0：恢复
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_Pause(LONG nPort,DWORD nPause);

/**
 * 慢速播放。每次调用将使当前播放速度慢一倍；要恢复正常播放调用PLAY_Play()，从当前位置开始正常播放。
 * 播放速度范围为[1/64,64]。
 *
 * @param[in] nPort 通道号
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_Slow(LONG nPort);

/**
 * 快速播放。每次调用将使当前播放速度加快一倍，要恢复正常播放调用PLAY_Play()，从当前位置开始正常播 
   放；高清码流在高倍速播放时，由于受到解码和显示的限制，可能达不到所设置的速度。
 * 播放速度范围为[1/64, 64]。
 *
 * @param[in] nPort 通道号
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_Fast(LONG nPort);

/**
 * 单帧播放。要恢复正常播放调用PLAY_ Play。
 *
 * @param[in] nPort 通道号
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_OneByOne(LONG nPort);

/**
 * 单帧倒放。要恢复正常播放调用PLAY_ Play。只支持文件播放，且必须在文件索引生成之后才能调用。
 *
 * @param[in] nPort 通道号
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_OneByOneBack(LONG nPort);

/**
 * 设置视频参数。
 *
 * @param[in] nPort 通道号
 * @param[in] nRegionNum 显示区域。如果只有一个显示区域(通常情况)设为0
 * @param[in] nBrightness 亮度。默认64；范围0-128
 * @param[in] nContrast 对比度。默认64；范围0-128
 * @param[in] nSaturation 饱和度。默认64；范围0-128
 * @param[in] nHue 色调。默认64；范围0-128
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetColor(LONG nPort, DWORD nRegionNum, int nBrightness, int nContrast, int nSaturation, int nHue);

/**
 * 获取视频参数。
 *
 * @param[in] nPort 通道号
 * @param[in] nRegionNum 显示区域
 * @param[out] pBrightness 亮度
 * @param[out] pContrast 对比度
 * @param[out] pSaturation 饱和度
 * @param[out] pHue 色调
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetColor(LONG nPort, DWORD nRegionNum, int *pBrightness, int *pContrast, int *pSaturation, int *pHue);

/**
 * 设置音视频播放策略。
 *
 * @param[in] nPort 通道号
 * @param[in] nAVSyncType 码流播放基准，见AV_SYNC_TYPE
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetAVSyncType(LONG nPort,AV_SYNC_TYPE nAVSyncType);

/**
 * 改变视频播放速度。在非1倍速下音频会被丢弃。
 *
 * @param[in] nPort 通道号
 * @param[in] fCoff 播放速度，范围是1/64-64。当播放速度较快时可能需要抽帧播放
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlaySpeed(LONG nPort, float fCoff);

/**
 * 设置播放方向。
 *
 * @param[in] nPort 通道号
 * @param[in] emDirection 播放方向。0：正放；1：倒放
 * @return 成功返回TURE；失败返回FALSE。
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlayDirection(LONG nPort, DWORD emDirection);

/**
 * 设置延迟播放时间。[nDelay,nThreshold]，小于nDelay稍微慢放，大于nThreshold稍微快放，两者之间正常播放。
 *
 * @param[in] nPort 通道号
 * @param[in] nDelay(ms) 延迟时间。缓冲达到该时间正常播放，小于该时间稍微慢放
 * @param[in] nThreshold(ms) 阈值时间。缓冲小于该时间正常播放，大于该时间稍微快放
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDelayTime(LONG nPort, int nDelay, int nThreshold);

/**
 * 设置播放策略，只对实时流有效。
 *
 * @param[in] nPort 通道号
 * @param[in] nStartTime(ms) 开始播放时间
 * @param[in] nSlowTime(ms) 阈值时间。缓冲小于该时间正常播放，大于该时间稍微快放
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlayMethod(LONG nPort, int nStartTime, int nSlowTime, int nFastTime, int nFailedTime);


/* 实时流数据缓冲模式 */
typedef enum
{
	CACHE_MODE_OFF = 0,		// 关闭实时流自适应缓冲模式
	ADAPTIVE_CACHE,			// 自适应缓冲
	REALTIME_FIRST,			// 实时优先
	FLUENCY_FIRST,			// 流畅优先 
} CACHE_MODE;

/**
 * 设置播放缓冲模式，只对实时流有效。
 *
 * @param[in] nPort 通道号
 * @param[in] cacheMode 实时数据缓冲模式
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetCacheMode(LONG nPort, CACHE_MODE cacheMode);

/**
 * 设置音频播放策略（只适用于对讲业务），需在PLAY_PlaySound之后调用
 *
 * @param[in] nPort 通道号
 * @param[in] nClearTime(ms) 缓冲清除阀值
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetAudioPlayMethod(LONG nPort, int nClearTime);

/**
 * 设置图像翻转。范围0-3。
 *
 * @param[in] nPort 通道号
 * @param[in] nrotateType 翻转类型。0：不旋转；1：旋转90度；2：旋转180度；3：旋转270度
 * @return 成功返回TURE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetRotateAngle(LONG nPort , int nrotateType);

/************************************************************************/
//> 文件定位
/************************************************************************/

/**
 * 设置文件播放指针的相对位置(百分比)。
 *
 * @param[in] nPort 通道号
 * @param[in] fRelativePos 文件长度的百分比
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlayPos(LONG nPort,float fRelativePos);

/**
 * 获取文件播放指针的相对位置(百分比)。
 *
 * @param[in] nPort 通道号
 * @return float，返回文件长度的相对位置，范围0-100%
 */
PLAYSDK_API float CALLMETHOD PLAY_GetPlayPos(LONG nPort);

/**
 * 设置文件当前播放时间。
 *
 * @param[in] nPort 通道号
 * @param[in] nTime 设置文件播放位置到指定时间，单位毫秒
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPlayedTimeEx(LONG nPort,DWORD nTime);

/**
 * 获取文件当前播放时间。
 *
 * @param[in] nPort 通道号
 * @return DWORD，文件当前播放的时间，单位毫秒
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetPlayedTimeEx(LONG nPort);

/**
 * 获取当前播放的帧序号，PLAY_GetPlayedFrames是总共解码的帧数。如果文件
 * 播放位置不被改变，那么这两个函数的返回值应该非常接近，除非码流丢失
 * 数据。
 *
 * @param[in] nPort 通道号
 * @return DWORD，当前播放的帧序号
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetCurrentFrameNum(LONG nPort);

/**
 * 设置文件当前帧播放帧号，此函数必须在文件索引生成之后才能调用。
 *
 * @param[in] nPort 通道号
 * @param[in] nFrameNum 帧序号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetCurrentFrameNum(LONG nPort,DWORD nFrameNum);

/**
 * 获取已播放的帧数。
 *
 * @param[in] nPort 通道号
 * @return DWORD，已经播放的视频帧数
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetPlayedFrames(LONG nPort);

/**
 * 获取当前文件播放时间。
 *
 * @param[in] nPort 通道号
 * @return DWORD，文件当前播放的时间，单位秒
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetPlayedTime(LONG nPort);

/**
 * 获取文件总时间。
 *
 * @param[in] nPort 通道号
 * @return DWORD，文件总的时间长度值，单位秒
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetFileTime(LONG nPort);

/**
 * 获取文件总帧数。
 *
 * @param[in] nPort 通道号
 * @return DWORD，文件中的总帧数
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetFileTotalFrames(LONG nPort);

/************************************************************************/
//> 文件索引
/************************************************************************/

/**
 * 建立索引回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pUserData 用户自定义参数
 */
typedef void(CALLBACK *fFileRefDoneCBFun)(DWORD nPort, void* pUserData);	

/**
 * 设置建立索引回调，在文件打开时生成文件索引。这个过程耗时比较长，大约
 * 每秒处理40M左右的数据，因为从硬盘读数据比较慢，建立索引的过程是在
 * 后台完成，需要使用索引的函数要等待这个过程结束，其他接口不受影响。
 *
 * @param[in] nPort 通道号
 * @param[out] pFileRefDoneCBFun 建立索引回调函数指针
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileRefCallBack(LONG nPort,
													fFileRefDoneCBFun pFileRefDoneCBFunc,
													void* pUserData);
/**
 * 建立索引回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] bIndexCreated 索引创建标志，TRUE索引创建成功；FALSE失败
 * @param[in] pUserData 用户自定义参数
 */
typedef void (CALLBACK *fFileRefDoneCBFunEx)(DWORD nPort, BOOL bIndexCreated, void* pUserData);

/**
 * 设置建立索引回调，在文件打开时生成文件索引。这个过程耗时比较长，大约
 * 每秒处理40M左右的数据，因为从硬盘读数据比较慢，建立索引的过程是在
 * 后台完成，需要使用索引的函数要等待这个过程结束，其他接口不受影响。
 *
 * @param[in] nPort 通道号
 * @param[out] pFileRefDoneCBFunEx 建立索引回调函数指针
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileRefCallBackEx(LONG nPort, fFileRefDoneCBFunEx pFileRefDoneCBFunEx, void* pUserData);

/**
 * 查找指定位置之前的关键帧位置，图像解码必须从关键帧开始，如果用户保存
 * 的文件不是从关键帧开始的，那么倒下一个关键帧之前的数据会被忽略。如
 * 果用户要截取文件中的一段数据，则应该考虑从关键帧开始截取。结束位置
 * 则关系不大，最多丢失3帧数据。在文件索引建立完全的前提下，与
 * PLAY_GetNextKeyFramePos联合使用，可以用来实现剪辑录像文件，剪辑精度
 * 与关键帧间隔有关。
 *
 * @param[in] nPort 通道号
 * @param[in] nValue 当前位置，可以是时间或帧号，类型由nType指定
 * @param[in] nType 指定nValue的类型。如果nType是BY_FRAMENUM，则nValue表示帧号；nType是BY_FRAMETIME，则nValue表示时间，单位ms。
 * @param[in] pFramePos 查找到的关键帧的文件位置信息结构指针，详见PFRAME_POS。
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetKeyFramePos(LONG nPort,DWORD nValue, DWORD nType, PFRAME_POS pFramePos);

/**
 * 查找指定位置之后的关键帧位置。
 *
 * @param[in] nPort 通道号
 * @param[in] nValue 当前位置，可以是时间或帧号，类型由nType指定
 * @param[in] nType 指定nValue的类型。如果nType是BY_FRAMENUM,则nValue表示帧号；nType是BY_FRAMETIME,则nValue表示时间,单位ms。
 * @param[in] pFramePos 查找到的关键帧的文件位置信息结构指针，详见PFRAME_POS。
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetNextKeyFramePos(LONG nPort,DWORD nValue, DWORD nType, PFRAME_POS pFramePos);

/**
 * 获取文件索引，以便下次打开同一个文件时直接使用这个信息。必须在索引建成后才能获得信息。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuffer 索引信息
 * @param[in/out] pSize 输入pBuffer的大小，输出索引信息大小。可以在第一次指定pSize=0，pBuffer=NULL，
 *                  从pSize的返回值获得需要的缓冲区大小。然后分配足够的缓冲，再调用一次。
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetRefValue(LONG nPort,BYTE *pBuffer, DWORD *pSize);

/**
 * 设置文件索引，索引信息及其长度必须准确。如果已经有了文件索引信息，可以
 * 不再调用生成索引的回调函数PLAY_SetFileRefCallBackEx，直接输入索引信息。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuffer 索引信息
 * @param[in] pSize 索引信息的长度
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetRefValue(LONG nPort,BYTE *pBuffer, DWORD nSize);

/************************************************************************/
//> 获取播放或解码信息
/************************************************************************/

/**
 * 设置解码回调流类型，在PLAY_Play之前调用。
 *
 * @param[in] nPort 通道号
 * @param[in] nStream 1 视频流；2 音频流；3 复合流
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecCBStream(LONG nPort,DWORD nStream);

/**
 * 解码回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pFrameDecodeInfo 解码后的音视频数据
 * @param[in] pFrameInfo 图像和声音信息，请参见FRAME_INFO结构体
 * @param[in] pUser 用户自定义参数
 */
typedef void (CALLBACK* fCBDecode)(LONG nPort, FRAME_DECODE_INFO* pFrameDecodeInfo, FRAME_INFO_EX* pFrameInfo, void* pUser);

/**
 * 设置解码回调，替换播放器中的显示部分，由用户自己控制显示，该函数在
 * PLAY_Play之前调用，在PLAY_Stop时自动失效，下次调用PLAY_Play之前
 * 需要重新设置。解码部分不控制速度，只要用户从回调函数中返回，解码器
 * 就会解码下一部分数据。适用于只解码不显示的情形。
 *
 * @param[in] nPort 通道号
 * @param[out] cbDec 解码回调函数指针，不能为NULL
 * @param[in] pUser 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecodeCallBack(LONG nPort, fCBDecode cbDec, void* pUser);

/**
 * 视频抓图回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 返回图像数据
 * @param[in] nSize 返回图像数据大小
 * @param[in] nWidth 画面宽，单位像素
 * @param[in] nHeight 画面高，单位像素
 * @param[in] nStamp 时标信息，单位毫秒
 * @param[in] nType 数据类型，T_RGB32，T_UYVY，详见宏定义说明
 * @param[in] nReceaved 对应用户自定义参数
 */
typedef void (CALLBACK* fDisplayCBFun)(LONG nPort,char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nStamp,LONG nType, void* pReserved);

/**
 * 设置视频抓图回调函数，如果要停止回调，可以把回调函数指针设为NULL，该函数可以在任何时候调用。
 *
 * @param[in] nPort 通道号
 * @param[out] DisplayCBFun 抓图回调函数，可以为NULL
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDisplayCallBack(LONG nPort, fDisplayCBFun DisplayCBFun, void* pUserData);

/**
 * 音频解码后的WAVE数据回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pAudioBuf wave格式音频数据
 * @param[in] nSize 音频数据长度
 * @param[in] nStamp 时标(ms)
 * @param[in] nType 音频类型T_AUDIO16， 采样率8000，单声道，每个采样点16位表示
 * @param[in] pUserData 用户自定义数据
 */
typedef void (CALLBACK * fAudioCBFun)(LONG nPort, char * pAudioBuf, LONG nSize, LONG nStamp, LONG nType, void* pUserData);

/**
 * 音频解码后的WAVE数据回调。
 *
 * @param[in] nPort 通道号
 * @param[out] AudioCBFun 音频解码回调函数
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetAudioCallBack(LONG nPort, fAudioCBFun AudioCBFun, void* pUserData);

/**
 * 解码回调。与PLAY_SetDecodeCallBack基本相同，不同的是解码回调的同时可以显示视频，建议不要在回调函数里面做长时间的逻辑处理，以免增加显示的延时。
 *
 * @param[in] nPort 通道号
 * @param[out] cbDec 解码回调函数指针，不能为NULL
 * @param[in] pUser 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
typedef fCBDecode fVisibleDecodeCallBackFunc;
PLAYSDK_API BOOL CALLMETHOD PLAY_SetVisibleDecodeCallBack(LONG nPort, fVisibleDecodeCallBackFunc cbDec, void* pUser);

/**
 * 解码回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 解码后的音视频数据
 * @param[in] nSize 解码后的音视频数据pBuf的长度
 * @param[in] pFrameInfo 图像和声音信息，请参见FRAME_INFO结构体
 * @param[in] pUserData 用户自定义参数
 * @param[in] nReserved2 保留参数
 */
typedef void (CALLBACK* fDecCBFun)(LONG nPort,char * pBuf,LONG nSize,FRAME_INFO * pFrameInfo, void* pUserData, LONG nReserved2);

/**
 * 设置解码回调，替换播放器中的显示部分，由用户自己控制显示，该函数在
 * PLAY_Play之前调用，在PLAY_Stop时自动失效，下次调用PLAY_Play之前
 * 需要重新设置。解码部分不控制速度，只要用户从回调函数中返回，解码器
 * 就会解码下一部分数据。适用于只解码不显示的情形。
 *
 * @param[in] nPort 通道号
 * @param[out] DecCBFun 解码回调函数指针，不能为NULL
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecCallBack(LONG nPort, fDecCBFun DecCBFun);

/**
 * 设置解码回调，替换播放器中的显示部分，由用户自己控制显示，该函数在
 * PLAY_Play之前调用，在PLAY_Stop时自动失效，下次调用PLAY_Play之前
 * 需要重新设置。解码部分不控制速度，只要用户从回调函数中返回，解码器
 * 就会解码下一部分数据。适用于只解码不显示的情形。
 *
 * @param[in] nPort 通道号
 * @param[out] DecCBFun 解码回调函数指针,不能为NULL
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecCallBackEx(LONG nPort, fDecCBFun DecCBFun, void* pUserData);

/**
 * 解码回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 解码后的音视频数据
 * @param[in] nSize 解码后的音视频数据pBuf的长度
 * @param[in] pFrameInfo 图像和声音信息,请参见FRAME_INFO结构体
 * @param[in] pUserData 用户自定义参数
 * @param[in] nReserved1 保留参数
 */
typedef void (CALLBACK* fVisibleDecCBFun)(LONG nPort,char * pBuf,LONG nSize,FRAME_INFO * pFrameInfo, void* pUserData, LONG nReserved1);

/**
 * 解码回调。与PLAY_SetDecCallBackEx基本相同，不同的是解码回调的同时可以
 * 显示视频，建议不要在回调函数里面做长时间的逻辑处理，以免增加显示的延时。
 *
 * @param[in] nPort 通道号
 * @param[out] DecCBFun 解码回调函数指针,不能为NULL
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetVisibleDecCallBack(LONG nPort, fVisibleDecCBFun DecCBFun, void* pUserData);

/**
 * 水印数据回调函数。
 *
 * @param[in] buf 水印数据buffer指针
 * @param[in] key 区分不同水印信息
 * @param[in] len 缓冲的最大长度
 * @param[in] reallen 缓冲的实际长度
 * @param[in] len 缓冲的最大长度
 * @param[in] reallen 缓冲的实际长度
 * @param[in] type 1没有错误；2水印错误；3帧数据错误；4帧号错误；5帧头错误；6没有帧头
 * @param[in] pUserData 用户自定义参数
 * @return int 无意义
 */
typedef int (CALLBACK* fGetWaterMarkInfoCallbackFunc)(char* buf, LONG key, LONG len, LONG reallen, LONG type, void* pUserData);	

/**
 * 设置水印数据回调。注意：水印校验回调设置后将不会进行解码显示。
 *
 * @param[in] nPort 通道号
 * @param[out] pFunc 水印回调函数指针
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetWaterMarkCallBack(LONG nPort, fGetWaterMarkInfoCallbackFunc pFunc, void* pUserData);

/**
 * 水印数据回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] buf 水印数据缓冲
 * @param[in] lTimeStamp 水印的时间戳
 * @param[in] lInfoType 水印信息类型,有三种类型,WATERMARK_DATA_TEXT,WATERMARK_DATA_JPEG_BMP,WATERMARK_DATA_FRAMEDATA
 * @param[in] len 缓冲的最大长度
 * @param[in] reallen 缓冲的实际长度
 * @param[in] lCheckResult 1没有错误；2水印错误；3帧数据错误；4帧号错误；5帧头错误；6没有帧头
 * @param[in] pUserData 用户自定义参数
 * @return int 无意义
 */
typedef int (CALLBACK* fGetWaterMarkInfoCallbackFuncEx)(LONG nPort, char* buf, LONG lTimeStamp, LONG lInfoType, LONG len, LONG reallen, LONG lCheckResult, void* pUserData);

/**
 * 设置水印数据回调。注意：水印校验回调设置后将不会进行解码显示。
 *
 * @param[in] nPort 通道号
 * @param[out] pFunc 水印回调函数指针
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetWaterMarkCallBackEx(LONG nPort, fGetWaterMarkInfoCallbackFuncEx pFunc, void* pUserData);

/**
 * 图像分辨率改变通知回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pUserData 用户自定义参数
 */
typedef void (CALLBACK *fEncChangeCBFun)(LONG nPort, void* pUserData);

/**
 * 设置图像分辨率改变通知回调，打开文件前使用。
 *
 * @param[in] nPort 通道号
 * @param[out] EncChangeCBFun 分辨率改变通知回调
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetEncTypeChangeCallBack(LONG nPort, fEncChangeCBFun EncChangeCBFun, void* pUserData);

/**
 * 图像分辨率改变通知回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pUserData 用户自定义参数
 * @param[in] nWidth 图像宽
 * @param[in] nHeight 图像高
 */
typedef void(CALLBACK *fEncChangeCBFunEx)(LONG nPort, void* pUserData,LONG nWidth, LONG nHeight);

/**
 * 设置图像分辨率改变通知回调，打开文件前使用。
 *
 * @param[in] nPort 通道号
 * @param[out] EncChangeCBFun 分辨率改变通知回调
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetEncTypeChangeCallBackEx(LONG nPort, fEncChangeCBFunEx EncChangeCBFun, void* pUserData);

/**
 * 查询信息。
 *
 * @param[in] nPort 通道号
 * @param[in] cmdType 指定状态查询指令，见CMD_TYPE_E
 * @param[in] buf 存放信息的缓冲
 * @param[in] buflen 缓冲长度
 * @param[out] returnlen 获取的信息的有效数据长度
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_QueryInfo(LONG nPort, int cmdType, char* buf, int buflen, int* returnlen);

/**
 * 获取视频实时码率。
 *
 * @param[in] nPort 通道号
 * @param[out] pBitRate 输出参数，返回视频码率(单位为k)
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetRealFrameBitRate(LONG nPort, double* pBitRate);

/**
 * 获取文件总帧数。
 *
 * @param[in] nPort 通道号
 * @return DWORD，获取当前帧率
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetCurrentFrameRate(LONG nPort);

/**
 * 获取用于显示的图像大小（如果是鱼眼，则为矫正后的鱼眼图像），根据此大小来设置显示窗口的区域，可以不用显卡做缩放
 * 工作，对于那些不支持硬件缩放的显卡来说非常有用。
 *
 * @param[in] nPort 通道号
 * @param[out] pWidth 图像宽
 * @param[out] pHeight 图像高
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPictureSize(LONG nPort,LONG *pWidth,LONG *pHeight);

/**
 * 帧数据回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 数据指针
 * @param[in] nSize 数据长度
 * @param[in] pMutexInfo 帧信息,指向DEMUX_INFO结构
 * @param[in] pMutexInfoEx 帧数据或裸数据，指向DemuInfoEx结构
 * @param[in] pUserData 用户自定义数据
 */
/* Demux帧类型 */
typedef enum
{
	FRAME_UNKNOWN = 0,			// 帧类型不可知
	FRAME_VIDEO,				// 帧类型是视频帧
	FRAME_AUDIO,				// 帧类型是音频帧
	FRAME_DATA					// 帧类型是数据帧
}FRAME_TYPE;

/* Demux帧子类型 */
typedef enum
{
	FRAME_SUB_TYPE_DATA_INVALID = 0,			// 数据无效
	FRAME_SUB_TYPE_VIDEO_I_FRAME = 1,			// I帧
	FRAME_SUB_TYPE_VIDEO_P_FRAME,				// P帧
	FRAME_SUB_TYPE_VIDEO_B_FRAME,				// B帧

	FRAME_SUB_TYPE_VIDEO_SMART_I_FRAME = 19,	// smart I帧
	FRAME_SUB_TYPE_VIDEO_SMART_P_FRAME, 		// smart P帧

	FRAME_SUB_TYPE_DATA_CIPHER_AUXILIARY = 26 	// 密码辅助帧
}FRAME_SUB_TYPE;

/* Demux视频编码格式 */
typedef enum
{
	ENCODE_VIDEO_UNKNOWN = 0,		// 视频编码格式不可知
	ENCODE_VIDEO_MPEG4,				// 视频编码格式是MPEG4
	ENCODE_VIDEO_HI_H264,			// 视频编码格式是海思H264
	ENCODE_VIDEO_JPEG,				// 视频编码格式是标准JPEG
	ENCODE_VIDEO_DH_H264,			// 视频编码格式是大华码流H264
	ENCODE_VIDEO_JPEG2000 = 6,		// 视频编码格式是标准JPEG2000
	ENCODE_VIDEO_AVS = 7,			// 视频编码格式是标准AVS
	ENCODE_VIDEO_STD_H264 = 8,		// 视频编码格式是标准H264
	ENCODE_VIDEO_MPEG2 = 9,			// 视频编码格式是MPEG2
	ENCODE_VIDEO_VNC = 10,			// 视频编码格式是VNC
	ENCODE_VIDEO_SVAC = 11,			// 视频编码格式是SVAC
	ENCODE_VIDEO_DH_H265 = 12		// 视频编码格式是H265
}ENCODE_VIDEO_TYPE;

/* Demux音频编码格式 */
typedef enum 
{
	ENCODE_AUDIO_UNKNOWN = 0,
	ENCODE_AUDIO_PCM = 7,			// 音频编码格式是PCM8
	ENCODE_AUDIO_G729,				// 音频编码格式是G729
	ENCODE_AUDIO_IMA,				// 音频编码格式是IMA
	ENCODE_PCM_MULAW,				// 音频编码格式是PCM MULAW
	ENCODE_AUDIO_G721,				// 音频编码格式是G721
	ENCODE_PCM8_VWIS,				// 音频编码格式是PCM8_VWIS
	ENCODE_MS_ADPCM,				// 音频编码格式是MS_ADPCM
	ENCODE_AUDIO_G711A,				// 音频编码格式是G711A
	ENCODE_AUDIO_AMR,				// 音频编码格式是AMR
	ENCODE_AUDIO_PCM16,				// 音频编码格式是PCM16
	ENCODE_AUDIO_G711U = 22,		// 音频编码格式是G711U
	ENCODE_AUDIO_G723 = 25,			// 音频编码格式是G723
	ENCODE_AUDIO_AAC,				// 音频编码格式是AAC
	ENCODE_AUDIO_G726_40,           // 音频编码格式是G726, 40kbps
	ENCODE_AUDIO_G726_32,           // 音频编码格式是G726, 32kbps
	ENCODE_AUDIO_G726_24,           // 音频编码格式是G726, 24kbps
	ENCODE_AUDIO_G726_16,           // 音频编码格式是G726, 16kbps
	ENCODE_AUDIO_MP2,				// 音频编码格式是mp2
	ENCODE_AUDIO_OGG,				// 音频编码格式是ogg vorbis
	ENCODE_AUDIO_MP3,				// 音频编码格式是mp3
	ENCODE_AUDIO_G722_1,		    // 音频编码格式是G722_1
	ENCODE_AUDIO_OPUS = 38          // 音频编码格式是OPUS
}ENCODE_AUDIO_TYPE;

/* 码流封装格式 */
typedef enum
{
	STREAM_TYPE_UNKNOWN = 0,        // 未知码流
	STREAM_TYPE_MPEG4,              // MPEG4
	STREAM_TYPE_DHPT =3,	        // 大华老码流：DHPT
	STREAM_TYPE_NEW,                // 大华老码流：NEW
	STREAM_TYPE_HB,                 // 大华老码流：HB
	STREAM_TYPE_AUDIO,              // 音频流
	STREAM_TYPE_PS,                 // MPEG-2：PS
	STREAM_TYPE_DHSTD,              // 大华最新的标准码流
	STREAM_TYPE_ASF,                // ASF
	STREAM_TYPE_3GPP,               // 3GP
	STREAM_TYPE_RAW,	            // 大华老码流：裸码流
	STREAM_TYPE_TS,                 // MPEG-2：TS
	STREAM_TYPE_SVC,                // svc
	STREAM_TYPE_AVI,                // AVI
	STREAM_TYPE_MP4,                // MP4
	STREAM_TYPE_CGI,                // CGI
	STREAM_TYPE_WAV,		        // WAV音频
	STREAM_TYPE_FLV,                // FLV

	STREAM_TYPE_MKV,                // mkv
	STREAM_TYPE_RTP,			    // RTP
	STREAM_TYPE_RAW_MPEG4,	        // MPEG4裸码流
	STREAM_TYPE_RAW_H264,	        // H264裸码流
	STREAM_TYPE_RAW_H265,	        // H265裸码流
	STREAM_TYPE_WMV,			    // WMV
	STREAM_TYPE_RAW_MPEG2,	        // MPEG2裸码流
	STREAM_TYPE_RAW_SVAC,	        // SVAC裸码流
	STREAM_TYPE_MOV,
	STREAM_TYPE_VOB,			    // VOB
	STREAM_TYPE_RAW_H263,
	STREAM_TYPE_RM,
	STREAM_TYPE_DHPS,		        // DHPS
}STREAM_TYPE;

/* Demux扩展信息 */
typedef struct 
{
	char*	pHead;				// 帧数据(带大华头)
	int		nLen;				// 帧数据长度
	char*	pBody;				// 裸数据(不带大华头)
	int		nBodyLen;			// 裸数据长度

	int		nRet;				// 0:继续解码 1:不解码，默认为0
	BYTE	nEncryptType;		// 加密类型，0:不加密 1:AES
    BYTE    nRotateAngle;       // 码流旋转角度 0:不旋转 1:90度旋转 2:180度旋转 3:270度旋转
	char    reserved1[2];
	int     nStreamType;        // 码流封装格式,详见STREAM_TYPE定义
	char	reserved[120];
}DemuInfoEx;

/* Demux基本信息 */
typedef struct
{
	int type;			 // 详见FRAME_TYPE定义
	int subtype;		 // 详见FRAME_SUB_TYPE定义
	int encode;			 // 详见ENCODE_VIDEO_TYPE，ENCODE_AUDIO_TYPE定义
	int sequence;		 // 帧序号
	int width;			 // 视频宽
	int height;			 // 视频高
	int rate;			 // 帧率
	int year;			 // 绝对时间年
	int month;           // 绝对时间月
	int day;             // 绝对时间日
	int hour;            // 绝对时间时
	int minute;          // 绝对时间分
	int secode;          // 绝对时间秒
	LONG timestamp;      // 相对时间戳
	int channels;        // 音频通道数
	int bitspersample;   // 音频采样深度
	int samplespersecond;// 音频采样率
}DEMUX_INFO;

typedef void (CALLBACK* fDemuxDecCBFun)(LONG nPort,char * pBuf,	LONG nSize,void * pMutexInfo,void* pMutexInfoEx, void* pUserData);

/**
 * 设置帧数据回调。
 *
 * @param[in]  nPort 通道号
 * @param[out] DecCBFun 帧数据回调函数
 * @param[in]  pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDemuxCallBack(LONG nPort, fDemuxDecCBFun DecCBFun, void* pUserData);

/**
 * H264信息获取回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 数据指针
 * @param[in] nLen 数据长度
 * @param[in] pUserData 用户自定义参数
 */
typedef int (CALLBACK *fH264InfoCBFun)(LONG nPort, char* pBuf, LONG nLen, void* pUserData);

/**
 * 设置H264信息获取回调函数。
 *
 * @param[in]  nPort 通道号
 * @param[out] pH264InfoCBFun H264信息获取回调函数
 * @param[in]  nUser 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_Set264EncodeInfoCallBack(LONG nPort, fH264InfoCBFun pH264InfoCBFun, void* nUser);

/**
 * 设置aes解密密钥。
 *
 * @param[in] nPort 通道号
 * @param[in] szKey 密钥的指针
 * @param[in] nKeylen 密钥的长度
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetSecurityKey(LONG nPort,const char* szKey,DWORD nKeylen);

/*加密类型*/
typedef enum
{
	ENCRYPT_UNKOWN = 0,
	ENCRYPT_AES,
	ENCRYPT_AES256,
	ENCRYPT_AES256_GDPR2,
	ENCRYPT_SM1_ECB,                     /*国密加密算法SM1_ECB*/       
	ENCRYPT_SM1_OFB,                     /*国密加密算法SM1_OFB*/ 
	ENCRYPT_SM4_ECB,                     /*国密加密算法SM4_ECB*/ 
	ENCRYPT_SM4_OFB,                     /*国密加密算法SM4_OFB*/  //不知道具尸是哪种国密加密，就填这种SP_ENCRYPT_SM4_OFB
}ENCRYPT_TYPE;

typedef struct
{
	unsigned char x[32];
	unsigned char y[32];
}PUBLICKEY_PARAM;

//解密参数
typedef struct
{
	char* Key;                     //AES:key；AES256(GDPR一期):key； AES256(GDPR二期)和国密:vkek；  		   
	int KeyLen;			
	char* KeyId;       	           //AES:不用填； AES256(GDPR一期):keyid； AES256(GDPR二期)和国密:vkekid ； 	 
	int KeyIdLen;	
	bool bSetPublicKey;            //是否设置公钥  pPublicKey
	PUBLICKEY_PARAM* pPublicKey;   //svac2.0 国密解密需要填;  其余都填NULL
	char Reserved[19];             //保留字段      
}DECRYPT_PARAM; 

/**
 * 设置aes解密密钥。
 *
 * @param[in] nPort 通道号
 * @param[in] nType  解密类型
 * @param[in] szKey 密钥的指针
 * @param[in] nKeylen 密钥的长度
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetSecurityKeyEx(LONG nPort, ENCRYPT_TYPE nType, DECRYPT_PARAM* key, unsigned int key_len);

/************************************************************************/
//> 显示操作
/************************************************************************/

/**
 * 设置或增加显示区域，可以做局部放大显示。
 *
 * @param[in] nPort 通道号
 * @param[in] nRegionNum 显示区域序号,0~(MAX_DISPLAY_WND-1),如果为0,则将设置的区域显示在主窗口中
 * @param[in] pSrcRect 局部显示区域
 * @param[in] hDestWnd 显示窗口句柄
 * @param[in] bEnable 打开(设置)或关闭显示区域
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDisplayRegion(LONG nPort,DWORD nRegionNum, DISPLAYRECT *pSrcRect, HWND hDestWnd, BOOL bEnable);

/**
 * 刷新显示。当用户暂停时如果刷新了窗口，则窗口中的图像因为刷新而消失，
 * 此时调用这个接口可以重新把图像显示出来。只有在暂停和单帧播放时才
 * 执行，其它情况会直接返回。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_RefreshPlay(LONG nPort);

/************************************************************************/
//> 缓冲区操作
/************************************************************************/

/**
 *  获取流播放模式下源缓冲区剩余数据大小(单位为字节)。
 *
 * @param[in] nPort 通道号
 * @return DWORD，当前源缓冲的大小单位BYTE(单位为字节)
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetSourceBufferRemain(LONG nPort);

/**
 * 清空流播放模式下源缓冲区的剩余数据。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_ResetSourceBuffer(LONG nPort);

/**
 * 清空流播放模式下源缓冲区的剩余数据。
 *
 * @param[in] nPort 通道号
 * @param[in] nBufType，如下：
 *			BUF_VIDEO_SRC 1 视频源缓冲
 *			BUF_AUDIO_SRC 2 音频源缓冲
 *			BUF_VIDEO_RENDER 3 解码后视频数据缓冲
 *			BUF_AUDIO_RENDER 4 解码后音频数据缓冲
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_ResetBuffer(LONG nPort,DWORD nBufType);

/**
 * 获取指定缓冲区的大小。
 *
 * @param[in] nPort 通道号
 * @param[in] nBufType，如下：
 *			BUF_VIDEO_SRC 1 视频源缓冲
 *			BUF_AUDIO_SRC 2 音频源缓冲
 *			BUF_VIDEO_RENDER 3 解码后视频数据缓冲
 *			BUF_AUDIO_RENDER 4 解码后音频数据缓冲
 * @return DWORD，根据不同参数返回缓冲区值，源缓冲区返回byte，解码后缓冲区返回帧数
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetBufferValue(LONG nPort,DWORD nBufType);

/************************************************************************/
//> 车载硬盘
/************************************************************************/

/**
 * 初始化硬盘(Windows32平台)。
 *
 * @return UINT，成功返回硬盘数量， 失败返回0
 */
PLAYSDK_API UINT CALLMETHOD PLAY_InitDisk();

/**
 * 进度回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] nPercent 进度百分比, -1表示异常
 * @param[in] pUserData 用户自定义参数
 */
typedef void (CALLBACK *fPercentCallbackFunc)(LONG nPort, int nPercent, void* pUserData);

/**
 * 进度回调。可以设定文件开始结束时间，如果指定，需要PLAY_SetFileRefCallBack回调后才有效，[0,0] 表示整个文件大小，不需要建立索引，在PLAY_OpenFile之后调用。
 *
 * @param[in] nPort 通道号
 * @param[in] nStartTime 文件开始时间， 单位为从1970/1/1开始经过的秒数
 * @param[in] nEndTime 文件结束时间， 单位为从1970/1/1开始经过的秒数
 * @param[out] pFunCallback 进度回调函数指针
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPercentCallBack(LONG nPort, LONG nStartTime, LONG nEndTime, fPercentCallbackFunc pFunCallback, void* pUserData);

typedef enum
{
	FORMAT_ALL_DATA, //清除所有数据
	FORMAT_KEY_DATA  //清除关键区数据
}FormatType;

/**
 * 格式化硬盘(Windows32平台)。
 *
 * @param[in] disk_no 要读取分区信息的磁盘号
 * @param[in] type 格式化选项
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_FormatDisk(UINT disk_no, FormatType type);

/**
 * 文件查询与定位(Windows32平台)。
 *
 * @param[in] channel 需要查询的录像的通道号
 * @param[in] start_time 录像查询的开始时间
 * @param[in] end_time 录像查询的结束时间
 * @param[in/out] pmax_ret_num 输入查询的最大文件个数，返回查询到的文件个数，文件系统最多一次返回pmax_ret_num传入的条数
 * @param[out] pfile_info 查询到的录像文件存放的位置
 * @param[in] type 查询的录像类型掩码，报警、动检、普通
 * @param[in] hint 暂时无意义，固定设置为0
 * @param[in] driver_type 驱动器类型掩码，指定在某些驱动器类型内查询
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 * 文件名字参考: "/part/uuid/%4d-%02d-%02d/%03d/%02d.%02d.%02d-%02d.%02d.%02d/cType/first_clus_no.dav"
 */
PLAYSDK_API UINT CALLMETHOD PLAY_QueryFileList(UINT nChannel,DHTIME start_time,DHTIME end_time,UINT *pmax_ret_num,pFILE_INFO pfile_info,UINT type, BYTE hint, UINT driver_type);

/************************************************************************/
//> 鱼眼
/************************************************************************/

typedef enum
{
	FISHEYEMOUNT_MODE_INVALID = 0,					// 安装模式无效
	FISHEYEMOUNT_MODE_CEIL = 1,						// 顶装
	FISHEYEMOUNT_MODE_WALL,							// 壁装
	FISHEYEMOUNT_MODE_FLOOR,						// 地装
	FISHEYEMOUNT_MODE_NUM
}FISHEYE_MOUNTMODE;

typedef enum
{
	FISHEYECALIBRATE_MODE_INVALID = 0,							// 矫正模式无效
	FISHEYECALIBRATE_MODE_OFF = 1,								// 关闭鱼眼算法库，外部关闭
	FISHEYECALIBRATE_MODE_ORIGINAL,								// 原始模式(正方形),带缩放比例
	FISHEYECALIBRATE_MODE_PANORAMA,								// 1p
	FISHEYECALIBRATE_MODE_PANORAMA_PLUS_ONE_EPTZ,				// 1p+1
	FISHEYECALIBRATE_MODE_DOUBLE_PANORAMA,						// 2p
	FISHEYECALIBRATE_MODE_ORIGINAL_PLUS_DOUBLE_PANORAMA,		// 1+2p
	FISHEYECALIBRATE_MODE_ORIGINAL_PLUS_THREE_EPTZ_REGION,		// 1+3
	FISHEYECALIBRATE_MODE_PANORAMA_PLUS_THREE_EPTZ_REGION,		// 1p+3
	FISHEYECALIBRATE_MODE_ORIGINAL_PLUS_TWO_EPTZ_REGION,		// 1+2
	FISHEYECALIBRATE_MODE_ORIGINAL_PLUS_FOUR_EPTZ_REGION,		// 1+4
	FISHEYECALIBRATE_MODE_PANORAMA_PLUS_FOUR_EPTZ_REGION,		// 1p+4
	FISHEYECALIBRATE_MODE_PANORAMA_PLUS_SIX_EPTZ_REGION,		// 1p+6
	FISHEYECALIBRATE_MODE_ORIGINAL_PLUS_EIGHT_EPTZ_REGION,		// 1+8
	FISHEYECALIBRATE_MODE_PANORAMA_PLUS_EIGHT_EPTZ_REGION,		// 1p+8
	FISHEYECALIBRATE_MODE_TWO_EPTZ_REGION_WITH_ORIGINAL,		// 1F+2	
	FISHEYECALIBRATE_MODE_FOUR_EPTZ_REGION_WITH_ORIGINAL,		// 1F+4	
	FISHEYECALIBRATE_MODE_DOUBLE_PANORAMA_WITH_ORIGINAL,		// 1F+2p
	FISHEYECALIBRATE_MODE_FOUR_EPTZ_REGION_WITH_PANORAMA,		// 1p(F)+4
	FISHEYECALIBRATE_MODE_TWO_EPTZ_REGION,		                // 2画面
	FISHEYECALIBRATE_MODE_SINGLE,								// 单画面
	FISHEYECALIBRATE_MODE_FOUR_EPTZ_REGION,						// 4画面
	FISHEYECALIBRATE_MODE_USER_DEFINED,				    		// 用户自定义
	FISHEYECALIBRATE_MODE_PHONE,								// 手机模式
	FISHEYECALIBRATE_MODE_ORIGINAL_PLUS_ONE_EPTZ_REGION,		// 1+1
	FISHEYECALIBRATE_MODE_ONE_EPTZ_REGION,						// 1画面
	FISHEYECALIBRATE_MODE_SEMI_SPHERE,							// 半圆
	FISHEYECALIBRATE_MODE_CYLINDER,								// 圆柱
	FISHEYECALIBRATE_MODE_LITTLE_PLANET,						// 小行星
	FISHEYECALIBRATE_MODE_DOUBLE_SPHERE,						// 双目球
	FISHEYECALIBRATE_MODE_DOUBLE_CYLINDER,						// 双目圆柱
	FISHEYECALIBRATE_MODE_DOUBLE_360,

	FISHEYECALIBRATE_MODE_NUM
}FISHEYE_CALIBRATMODE;

typedef enum
{
	FISHEYEEPTZ_CMD_INVALID = 0,	
	FISHEYEEPTZ_CMD_ZOOM_IN = 1,						// 放大
	FISHEYEEPTZ_CMD_ZOOM_OUT,							// 缩小
	FISHEYEEPTZ_CMD_UP,									// 向上移动
	FISHEYEEPTZ_CMD_DOWN,								// 向下移动
	FISHEYEEPTZ_CMD_LEFT,								// 向左移动
	FISHEYEEPTZ_CMD_RIGHT,								// 向右移动
	FISHEYEEPTZ_CMD_ROTATE_CLOCKWISE_AUTO,				// 自动顺时针旋转
	FISHEYEEPTZ_CMD_ROTATE_ANTICLOCKWISE_AUTO,			// 自动逆时针旋转
	FISHEYEEPTZ_CMD_STOP,								// 停止
	FISHEYEEPTZ_CMD_SHOW_REGION,						// 框选放大
	FISHEYEEPTZ_CMD_EXIT_SHOW_REGION,					// 退出框选放大
	FISHEYEEPTZ_CMD_DEFAULT,						    // 恢复默认
	FISHEYEEPTZ_CMD_ORIGIN_ROTATE,						// 圆旋转

	FISHEYEEPTZ_CMD_SET_CUR_REGION = 0x20,              // 配置指定窗口的位置信息
	FISHEYEEPTZ_CMD_GET_CUR_REGION,                     // 获取指定窗口的位置信息
	FISHEYEEPTZ_CMD_IS_IN_PANORAMA_REGION,              // 输入点是否在当前全景点链区域内
	FISHEYEEPTZ_CMD_TAP_VIEW,							// 显示指定位置,即点即看
	FISHEYEEPTZ_CMD_SET_FOCUS,	 				        // 设置窗口位置信息
	FISHEYEEPTZ_CMD_GET_FOCUS,							// 获取窗口位置信息

	FISHEYEEPTZ_CMD_PTZ_CALI,							// 鱼球联动标定
	FISHEYEEPTZ_CMD_GET_PTZ_RLT,						// 获取鱼球联动定位信息
    FISHEYEEPTZ_CMD_SET_CUR_REGION_PTZ,                 // 外部直接输入ptz模式
    FISHEYEEPTZ_CMD_GET_FOCUS_8192,                     // 获取窗口位置信息8192坐标系
	FISHEYEEPTZ_CMD_NUM
}FISHEYE_EPTZCMD;

typedef enum
{
	FISHEYE_SETPARAM,	// 设置参数
	FISHEYE_GETPARAM	// 获取参数
}FISHEYE_OPERATETYPE;

typedef enum
{
	/* 枪机类型 */
	IPCTYPE_200WN				= 0,	//
	IPCTYPE_130WN				= 1,
	IPCTYPE_D1WN				= 2,
	IPCTYPE_100WN				= 3,
	IPCTYPE_FE					= 4,	// 鱼眼

	//球机类型
	SPCTYPE_D6501				= 100,	// sony机芯65球机
	HSPCTYPE_D6A2030E			= 101,	// 大华机芯2030E，6A球机
	HSPCTYPE_D65A2030E			= 102	// 大华机芯2030E，65A球机
}CAM_TYPE;

/* 镜头类型 */
typedef enum
{
	LENTYPE_NORM				= 0,	// 无畸变镜头
	LENTYPE_Lens0361 			= 1,	// 3.6毫米枪机镜头
	LENTYPE_Lens2880			= 2,	// 130度广角枪机镜头
	LENTYPE_Lens0362 			= 3,	// 3.6毫米枪机镜头
	LENTYPE_Lens0401 			= 4,	// 4.0毫米枪机镜头

	LENTYPE_TEST1				= 100	// 调试用参数
}LEN_TYPE;

typedef struct
{
	int w;
	int h;
}FISHEYE_SIZE;

typedef struct
{
	short x;
	short y;
}FISHEYE_POINT2D;

typedef struct
{
	FISHEYE_MOUNTMODE     subMountMode;			    // 子图像安装模式, 仅当图像主校正模式为用户自定义模式时, 该值有效
	FISHEYE_CALIBRATMODE  subCalibrateMode;	        // 子图像校正模式, 仅当图像主校正模式为用户自定义模式时, 该值有效
	FISHEYE_SIZE          imgOutput;                // 子图像输出分辨率
	FISHEYE_POINT2D       upperLeft;                // 子图像偏移
	int					  reserved[3];				// 保留字节
}FISHEYE_SUBMODE;

typedef struct
{
	FISHEYE_SIZE          mainShowSize;		        // 主显示窗口分辨率, 算法据此输出适合的最优结果(不变形情况下尽量达到该比例)
	FISHEYE_SIZE          floatMainShowSize;		// 输出双buffer时使用，目前暂时还是用老的操作方法，浮动窗口的主显示窗口分辨率, 浮动圆的宽高比需要为1:1，浮动壁装全景的宽高比需要为16:9
	FISHEYE_SIZE          imgOutput;                // 输出图像分辨率(缩放前), 图像主校正模式为用户自定义模式时为外部输入, 其他模式为内部返回
	FISHEYE_SUBMODE*	  subMode;                  // 子模式信息, 图像主校正模式为用户自定义模式时为外部输入, 其他模式为内部返回
	int		              subModeNum;               // 子模式数, 图像主校正模式为用户自定义模式时为外部输入, 其他模式为内部返回
	int                   outputSizeRatio;		    // 暂不启用, 校正输出图像的缩放比,Q8格式,范围0-256, 256为保持最大输出分辨率
	int                   reserved[1];				// 保留字节
}FISHEYE_OUTPUTFORMAT;

typedef struct
{
	int x;
	int y;
	int hAngle;
	int vAngle;
	int available;
	int reserved[3];
}FISHEYE_REGIONPARAM;

typedef struct
{
	FISHEYE_REGIONPARAM   regionParam[9];
	int              circularOffset;
	int              panoramaOffset;
	int              useRegionParam;           // 为1时表明有效，使用该值进行初始化；没有保存信息时请置为0
	int              reserved[1];
}FISHEYE_MODEINITPARAM;

typedef struct
{	
	/*必设参数*/
	int zoom_type;							// 倍数控制模式----期望自适应变倍和根据框选目标大小变倍两种模式
	int hcam_wax;							// 期望倍数对应球机角度x（水平）
	int hcam_way;							// 期望倍数对应球机角度y（垂直）
	int hcam_wmul;							// 期望倍数设置（基准倍数）
	int cfg_type;							// 配置方式，默认为1：使用配置参数方式,1：使用参数配置方式，0：使用设备类型配置方式

	/* 主相机参数 */							         
	/* 镜头参数 */
	int prm_re;								// 投影半径
	int prm_mul;							// 投影倍率
	int prm_dx;								// x方向偏移
	int prm_dy;								// y方向偏移
	int prm_cw;								// CMOS宽（实际使用宽）
	int prm_ch;								// CMOS高（实际使用高）

	/* 主相机和从相机类型配置（cfg_type为0时设置该参数才有效）,默认130度、130万枪机和200W65球机 */
	LEN_TYPE mlen_type;						// 主相机镜头类型
	CAM_TYPE mcam_type;						// 主相机类型
	CAM_TYPE hcam_type;						// 从相机类型

	/* 球机参数 */
	int himg_width;                         // 从相机图像宽
	int himg_height;                        // 从相机图像高
	int prm_fax;                            // 球机水平视场角

	/* 可默认的参数 */
	/* 主相机参数 */
	int mcam_fc;							// 相机等效焦距
	int mcam_cw;							// 镜头靶面高
	int mcam_ch;							// 镜头靶面宽 
	int cam_height;                         // 相机架设高度（米），（暂时未使用）
	int prm_ma;								// 焦距参数

	/* 从相机参数 */
	/* 球机参数 */
	int prm_hw;								// CMOS宽
	int prm_hh;								// CMOS高
	int prm_fo;								// 等效焦距
	int prm_ca;								// 视野参数
	int prm_mmul;							// 最大倍率
}MHFPTZ_CONFIGPARAM;

typedef struct
{
	FISHEYE_SIZE     mainStreamSize;		    // 对应主码流原始宽高，当传入分辨率与之不等时表明为辅码流是此分辨率缩放而来
	int              originX;					// 输入图像中鱼眼圆的圆心横坐标, 归一化至0-8192坐标系
	int              originY;					// 输入图像中鱼眼圆的圆心纵坐标, 归一化至0-8192坐标系
	int              radius;					// 输入图像中鱼眼圆的半径, 归一化至0-8192坐标系
	int              lensDirection;		        // 旋转角度, Q7格式, 范围0-360*128, 一般配为0
	FISHEYE_MOUNTMODE     mainMountMode;		// 主安装模式
	FISHEYE_CALIBRATMODE  mainCalibrateMode;	// 图像主校正模式
	FISHEYE_MODEINITPARAM modeInitParam;        // 外部传入模式初始化各画面信息，适用于模式切换恢复到上一次的状态,
	FISHEYE_OUTPUTFORMAT *outputFormat;         // 输出图像信息
	MHFPTZ_CONFIGPARAM   *configParam;          // 鱼球联动配置参数
	int              enableAutoContrast;        // 开启自动对比度, 0关闭, 1开启, 该功能会增加算法耗时, 需要性能好的机器才建议开启
	int              alphaHistogram;            // 直方图IIR强度0-255, 默认128, 越大越参考当前帧
	int              alphaGray;                 // 灰度拉伸强度0-255, 默认245, 越大越对比度弱
	FISHEYE_SIZE     captureSize;		        // 对应当前模式下的抓图分辨率
	int              mhfptzIndex;               // IN 鱼球联动球机序号0,1,2....
	int              reserved[1];				// 保留字节
}FISHEYE_OPTPARAM;

typedef struct
{
	FISHEYE_EPTZCMD   ePtzCmd;			// 云台操作，说明见FISHEYE_EPtzCmd定义
	int          winId;				    // 要进行eptz的窗口编号，左上角winId为0，从左到右递增							
	int          arg1;
	int          arg2;
	int          arg3;
	int          arg4;
	int          arg5;
	int          arg6;
	int          reserved0[6];			   // 保留字节
	void*        pParam;                   // 鱼球联动
	void*        pResult;
	void*        pArg;   

	int          reserved1[7];			   // 保留字节
}FISHEYE_EPTZPARAM;

/**
 * 开启视频鱼眼算法功能，需要包含fisheye.dll库。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StartFisheye(LONG nPort);

/**
 * 设置/获取鱼眼参数。
 *
 * @param[in] nPort 通道号
 * @param[in] operatetype 操作类型
 * @param[in/out] pOptParam 鱼眼参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_OptFisheyeParams(LONG nPort, FISHEYE_OPERATETYPE operatetype, FISHEYE_OPTPARAM* pOptParam);

/**
 * 用于浮动模式下开启或关闭第二个鱼眼窗口。
 *
 * @param[in] nPort 通道号
 * @param[in] hDestWnd 显示窗口句柄
 * @param[in] pOptParam 第二个窗口对应的鱼眼参数
 * @param[in] bEnable 打开或关闭显示区域
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_FisheyeSecondRegion(LONG nPort, HWND hDestWnd, FISHEYE_OPTPARAM* pOptParam, BOOL bEnable);

/**
 * 开启eptz(电子云台），进行缩放移动。
 *
 * @param[in] nPort 通道号
 * @param[in/out] pEptzParam 调节参数
 * @param[in] bSecondRegion 是否为浮动模式下的第二个窗口，1为真，默认填0
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_FisheyeEptzUpdate(LONG nPort, FISHEYE_EPTZPARAM* pEptzParam, BOOL bSecondRegion);

/**
 * 停止视频鱼眼算法功能。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StopFisheye(LONG nPort);

/**
 * 鱼眼信息回调函数。
 *
 * @param[in] nPort 通道号
 * @param[in] byCorrectMode 矫正模式
 * @param[in] wRadius 半径
 * @param[in] wCircleX 圆心横坐标
 * @param[in] wCircleY 圆心纵坐标
 * @param[in] widthRatio 宽比率
 * @param[in] heightRatio 高比率
 * @param[in] gain 增益
 * @param[in] denoiseLevel 降噪等级
 * @param[in] InstallStyle 鱼眼安装方式
 * @param[in] pUserData 用户自定义参数
 */
typedef void (CALLBACK* fFishEyeInfoFun)( 
	LONG nPort,
	BYTE byCorrectMode,
	WORD wRadius,
	WORD wCircleX,
	WORD wCircleY,
	UINT widthRatio,
	UINT heightRatio,
	BYTE gain,
	BYTE denoiseLevel,
	BYTE installStyle,
	void* pUserData );

/**
 * 设置获取鱼眼信息回调。
 *
 * @param[in] nPort 通道号
 * @param[out] pFishEyeInfoFun 鱼眼信息回调函数指针，不能为NULL
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetFishEyeInfoCallBack(LONG nPort, fFishEyeInfoFun pFishEyeInfoFun, void* pUserData);

/************************************************************************/
//> 抓图
/************************************************************************/

/**
 * 抓图，将BMP图片保存为指定的文件。PLAY_SetDisplayCallBack设置的视频数
 * 据回调函数，只有在有视频数据解码出来时才调用，并由用户处理视频数
 * 据(如抓图)，如果不断有解码的数据，就不断调用这个回调函数。而PLAY_
 * CatchPic一次只抓一幅图，并能在暂停和单帧播放时实现抓图。建议：如果
 * 用户想实现抓图(一次抓一幅图)，调用PLAY_CatchPic，而如果想得到一段
 * 时间内的视频数据，调用PLAY_SetDisplayCallBack。
 *
 * @param[in] nPort 通道号
 * @param[in] sFileName 文件名
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_CatchPic(LONG nPort,char* sFileName);

/**
* 抓图，将图片保存为指定的文件。PLAY_SetDisplayCallBack设置的视频数
* 据回调函数，只有在有视频数据解码出来时才调用，并由用户处理视频数
* 据(如抓图)，如果不断有解码的数据，就不断调用这个回调函数。而PLAY_
* CatchPicEx一次只抓一幅图，并能在暂停和单帧播放时实现抓图。建议：如果
* 用户想实现抓图(一次抓一幅图)，调用PLAY_CatchPicEx，而如果想得到一段
* 时间内的视频数据，调用PLAY_SetDisplayCallBack。
 * @param[in] nPort 通道号
 * @param[in] sFileName 文件名
 * @param[in] ePicfomat  图片格式，见tPicFormats
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_CatchPicEx(LONG nPort,char* sFileName,tPicFormats ePicfomat);

/**
 * 图像格式转为BMP格式。
 *
 * @param[in] pBuf 图像数据指针
 * @param[in] nSize 图像数据大小
 * @param[in] nWidth 图像宽度
 * @param[in] nHeight 图像高度
 * @param[in] nType 数据类型，如T_YV12，T_UYVY
 * @param[in] sFileName 要保存的文件名。最好以BMP作为文件扩展名
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_ConvertToBmpFile(char * pBuf,LONG nSize,LONG nWidth,LONG nHeight,LONG nType, char *sFileName);

/*
 * 图像格式转为JPEG格式。
 *
 * @param[in] pYUVBuf 图像数据指针
 * @param[in] nWidth 图像宽度
 * @param[in] nHeight 图像高度
 * @param[in] YUVtype YUV数据类型，如T_YV12，T_UYVY
 * @param[in] quality 图片压缩质量，范围(0, 100]
 * @param[in] sFileName 文件名
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_ConvertToJpegFile(char *pYUVBuf, LONG nWidth, LONG nHeight, int YUVtype, int quality, char *sFileName);

/**
 * 抓取BMP图像。
 *
 * @param[in] nPort 通道号
 * @param[in] pBmpBuf 存放BMP图像数据的缓冲地址，由用户分配，不得小于bmp 图像大小，
 *               推荐大小：sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+w*h*4，
 *        其中w和h分别为图像宽高
 * @param[in] dwBufSize 缓冲区大小
 * @param[out] pBmpSize 获取到的实际bmp图像大小
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPicBMP(LONG nPort, PBYTE pBmpBuf, DWORD dwBufSize, DWORD* pBmpSize);

/**
 * 抓取BMP图像。
 *
 * @param[in] nPort 通道号
 * @param[in] pBmpBuf   存放BMP图像数据的缓冲地址，由用户分配，不得小于bmp 图像大小，
 *   推荐大小：sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+w*h*4，
 *   其中w和h分别为图像宽高。
 * @param[in] dwBufSize 缓冲区大小
 * @param[out] pBmpSize 获取到的实际bmp图像大小
 * @param[in] nWidth 指定的bmp的宽
 * @param[in] nHeight 指定的bmp的高
 * @param[in] nRgbType 指定RGB格式 0：RGB32；1：RGB24；
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPicBMPEx(LONG nPort, PBYTE pBmpBuf, DWORD dwBufSize, DWORD* pBmpSize, LONG nWidth, LONG nHeight, int nRgbType);

/**
 * 抓取JPEG图像。
 *
 * @param[in] nPort 通道号
 * @param[in] pJpegBuf 存放JPEG图像数据的缓冲地址，由用户分配，不得小于JPEG图像大小，
         推荐大小：w*h*3/2，其中w和h分别为图像宽高
 * @param[in] dwBufSize 缓冲区大小
 * @param[out] pJpegSize 获取到的实际JPEG图像大小
 * @param[in] quality JPEG图像的压缩质量，取值范围为(0,100]
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPicJPEG(LONG nPort, PBYTE pJpegBuf, DWORD dwBufSize, DWORD* pJpegSize, int quality);

/**
 * 抓取TIFF图像。
 *
 * @param[in] nPort 通道号
 * @param[in] pTiffBuf 存放TIFF图像数据的缓冲地址，由用户分配，不得小于tiff图像大小，
   推荐大小：w*h*3+1024，
   其中w和h分别为图像宽高
 * @param[in] dwBufSize 缓冲区大小
 * @param[out] pTiffSize 获取到的实际TIFF图像大小
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPicTIFF(LONG nPort, PBYTE pTiffBuf, DWORD dwBufSize, DWORD* pTiffSize);

/**
 * 抓图，可选择格式并指定宽高。
 *
 * @param[in] nPort 通道号
 * @param[in] sFileName 文件名
 * @param[in] lTargetWidth 指定的图像宽度
 * @param[in] lTargetHeight 指定的图像高度
 * @param[in] ePicfomat 图片格式，详见tPicFomats
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_CatchResizePic(LONG nPort, char* sFileName, LONG lTargetWidth, LONG lTargetHeight, tPicFormats ePicfomat);

/************************************************************************/
//> 画图回调
/************************************************************************/

/*
 *画图回调函数
 * 
 * @param[in] nPort 通道号
 * @param[in] hDc OffScreen表面设备上下文
 * @param[in] pUserData 用户自定义参数
 */
typedef void (CALLBACK* fDrawCBFun)(LONG nPort,HDC hDc, void* pUserData);

/*
 * 注册一个回调函数，获得当前表面的device context，你可以在这个DC上画图(或文字)，
 * 就好像在窗口的客户区DC上绘图，但这个DC不是窗口客户区的DC，而是DirectDraw里
 * 的Off-Screen表面的DC。注意，如果是使用overlay表面，这个接口无效，可以直接在
 * 窗口上绘图，只要不是透明色就不会被覆盖。
 *
 * @param[in] nPort 通道号
 * @param[out] DrawFun 画图回调函数
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_RigisterDrawFun(LONG nPort, fDrawCBFun DrawCBFun, void* pUserData); 

/*
 *画图回调函数
 * 
 * @param[in] nPort 通道号
 * @param[in] nReginNum 显示区域序号，范围[0,(MAX_DISPLAY_WND-1)]。如果nRegionNum为0，
 *   则将设置的区域显示在主窗口中。
 * @param[in] hDc OffScreen表面设备上下文
 * @param[in] pUserData 用户自定义参数
 */
typedef void (CALLBACK* fDrawCBFunEx)(LONG nPort,LONG nReginNum,HDC hDc, void* pUserData);

/*
 * 注册一个回调函数，获得当前表面的上下文(HDC)，这个DC不是窗口客户区的DC，
 * 而是DirectDraw里的Off-Screen表面的DC。注意。如果是使用overlay表面，
 * 这个接口无效，overlay方式可以直接在窗口上绘图，只要不是透明色就不会被覆盖。
 *
 * @param[in] nPort 通道号
 * @param[in] nReginNum 显示区域序号，范围[0,(MAX_DISPLAY_WND-1)]。如果nRegionNum为0，
 *   则将设置的区域显示在主窗口中
 * @param[out] DrawFunEx 画图回调函数
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_RigisterDrawFunEx(LONG nPort, LONG nReginNum, fDrawCBFunEx DrawFunEx, void* pUserData);

/************************************************************************/
//> 录制或转码功能
/************************************************************************/

/*
 * 开始预录。
 *
 * @param[in] nPort 解码通道
 * @param[in] sFileName 录像位置
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StartPrepareRecord(LONG nPort,const char* pFileName);

/*
 * 停止预录。
 *
 * @param[in] nPort 解码通道
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StopPrepareRecord(LONG nPort);

/*
 * 开始流数据录像，只对流模式有用，在PLAY_Play之后调用。
 *
 * @param[in] nPort 通道号
 * @param[in] sFileName 文件名
 * @param[in] idataType 取值DATA_RECORD_TYPE
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
typedef enum
{
	DATA_RECORD_ORIGINAL = 0,     // 录制原始视频流
	DATA_RECORD_AVI,              // 录制AVI
	DATA_RECORD_ASF,              // 录制ASF  
	DATA_RECORD_ORIGINAL_SEGMENT, // 录制分段的原始视频流（废弃，分段下载请使用PLAY_StartDataRecordEx接口）
	DATA_RECORD_RESIZE_AVI,       // 录制转变分辨率的AVI，使用PLAY_ResolutionScale设置宽高
	DATA_RECORD_MP4,              // 录制MP4
	DATA_RECORD_RESIZE_MP4,       // 录制转变分辨率的MP4，使用PLAY_ResolutionScale设置宽高
	DATA_RECORD_MP4_NOSEEK,       // 录制不回写MP4 
    DATA_RECORD_RESIZE_MP4_NOSEEK,// 录制转变分辨率的不回写MP4，使用PLAY_ResolutionScale设置宽高
	DATA_RECORD_TS,               // 录制TS
    DATA_RECORD_PS,               // 标准PS封装
    DATA_RECORD_RESIZE_DAV,       // 录制DAV(需要重新编解码) 
	DATA_RECORD_DAV,              // 录制DAV(不需要重新编解码)

	// 如果需要增加枚举，在此之前增加
	DATA_RECORD_COUNT			  // 录制类型数目 
}DATA_RECORD_TYPE;
PLAYSDK_API BOOL CALLMETHOD PLAY_StartDataRecord(LONG nPort, char *sFileName, int idataType);

/*
 * 保存原始码流，该接口需与PLAY_StartDataRecord或者PLAY_StartDataRecordEx配合着使用。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 流数据
 * @param[in] nSize 大小
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_WriteData(LONG nPort, PBYTE pBuf,DWORD nSize);

/*
 * 停止流数据录像。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StopDataRecord(LONG nPort);

/*
 * 回调函数 AVIConvertCallback。
 *
 * @param[in] nPort 通道号
 * @param[in] lMediaChangeType AVI_MEDIACHANGE_FRAMERATE表示帧率改变；AVI_MEDIACHANGE_RESOLUTION表示分辨率改变
 * @param[in] pUserData 用户自定义参数
 * @param[out] pIsNewFile TRUE 分段转换；FALSE 不分段；默认不分段
 * @param[out] sNewFileName 如果分段转换，新的录像文件名
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
typedef void (CALLBACK* fAVIConvertCallback)(LONG nPort, LONG lMediaChangeType, void* pUserData, BOOL *pIsNewFile, char *sNewFileName);

/*
 * 开始AVI转换并设置AVI转换状态回调。
 *
 * @param[in] nPort 通道号
 * @param[in] sFileName 文件名
 * @param[out] pAVIFunc 回调函数
 * @param[in] pUserData 用户自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StartAVIConvert(LONG nPort, char *sFileName, fAVIConvertCallback pAVIFunc, void* pUserData);

/*
 * 停止AVI转换。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_StopAVIConvert(LONG nPort);

/************************************************************************/
//> 解码及显示设置
/************************************************************************/

/*
 * 指定解码器(Windows平台)， PLAY_Play之前调用有效。
 *
 * @param[in] nPort 通道号
 * @param[in] decodeType 解码模式（仅限于H264， Hevc)
 * @param[in] renderType 渲染模式
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetEngine(LONG nPort,DecodeType decodeType,RenderType renderType);

/*
 * 设置图像质量，当设置成高质量时画面效果好，但CPU利用率高。在支持多路播放时，
 * 可以设为低质量，以降低CPU利用率。当某路放大播放时将该路设置成高质量，
 * 以达到好的画面效果。
 *
 * @param[in] nPort 通道号
 * @param[in] bHighQuality 为TRUE(1)时图像高质量，为FALSE(0)时低质量(默认值)
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetPicQuality(LONG nPort,BOOL bHighQuality);

/*
 * 获取图像质量。
 *
 * @param[in] nPort 通道号
 * @param[out] bHighQuality 输出参数，TRUE表示高质量，FALSE表示低质量
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_GetPictureQuality(LONG nPort,BOOL *bHighQuality);

/*
 * 垂直同步显示开关。此接口需在PLAY_Play之后调用，重新播放时需重新设置。
 * 在播放动态图像出现断层时，可以使用此接口打开垂直同步功能。
 *
 * @param[in] nPort 通道号
 * @param[in] bEnable TRUE打开垂直同步，FALSE关闭垂直同步
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_VerticalSyncEnable(LONG nPort, BOOL bEnable);

/*
 * 是否启用高清图像内部调整策略。
 *
 * @param[in] nPort 通道号
 * @param[in] bEnable 详见PLAY_STRATEGE_E定义
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
typedef enum _PLAY_STRATEGE
{
	PLAY_THROW_FRAME_NO = 0,						// 不抽帧
	PLAY_THROW_FRAME_FLAG_HIGHT = 1,				// 默认策略抽帧，大于1080P码流4倍数(含)以上抽帧，[1080P,720P]码流8倍数(含)以上抽帧，小于720P码流16倍数(含)以上抽帧
	PLAY_THROW_FRAME_FLAG_ALL = 2,					// 强制抽I帧解码
	PLAY_THROW_FRAME_FLAG_ADAPTION = 3,				// 自适应解码能力抽帧
	PLAY_THROW_FRAME_FLAG_ADAPTION_LOW_CPU = 4,		// 自适应解码能力抽帧，低cpu占用率
} PLAY_STRATEGE_E;
PLAYSDK_API BOOL CALLMETHOD PLAY_EnableLargePicAdjustment(LONG nPort, int bEnable);

/*
 * 设置解码线程数。
 *
 * @param[in] nPort 通道号
 * @param[in] nNum 线程数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecodeThreadNum(LONG nPort, DWORD nNum);

/*
 * 设置解码策略，错误码流花屏或跳秒。
 *
 * @param[in] nPort 通道号
 * @param[in] nStrategyType 
 *				16: 表示解码中遇到frame不连续，提前返回
 *               8: 表示解码中遇到码流数据不符合协议当成解码错误
 *				 0 : 表示解码中遇到码流数据不符合协议继续解码，错误码流会花屏
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDecodeStrategy(LONG nPort, int nStrategyType);

/*
 * 抗锯齿使能开关，仅支持Windows。
 *
 * @param[in] nPort 通道号
 * @param[in] bEnable TRUE打开抗锯齿处理，FALSE关闭抗锯齿处理
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_AntiAliasEnable(LONG nPort, BOOL bEnable);

/************************************************************************/
//> 私有数据
/************************************************************************/

/*
 * 显示私有数据，例如规则框，规则框报警，移动侦测等。
 *
 * @param[in] nPort 通道号
 * @param[in] bTrue TRUE：打开 FALSE：关闭
 * @param[in] nReserve 保留参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_RenderPrivateData(LONG nPort, BOOL bTrue, LONG nReserve);

/*
 * 数据帧回调 PLAY_SetDataCallBack。
 *
 * @param[in] nPort 通道号
 * @param[in] pBuf 带大华头的原始数据
 * @param[in] nSize  数据大小
 * @param[in] pUserData  用户自定义数据
 */
typedef void (CALLBACK* fDataCBFun)(LONG nPort,char * pBuf,LONG nSize, void* pUserData);

/*
 * 数据帧回调，回调带大华头的原始数据，外层自己实现解析。
 *
 * @param[in] nPort 通道号
 * @param[out] pDataCBFun 回调函数
 * @param[in] pUserData 用户自定义数据
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetDataCallBack(LONG nPort, fDataCBFun pDataCBFun, void* pUserData);

/*
 * IVS解码回调函数。
 *
 * @param[in] pIVSBuf 辅助帧数据(json或智能帧结构体数据)，当为智能帧结构体数据时，pIVSBuf为IVS Object结构体数组的起始地址
 * @param[in] nIVSType 辅助帧数据类型,详见IVS_TYPE
 *			取值为IVSINFOTYPE_RAWDATA时，对应原始json数据
 *			取值为IVSINFOTYPE_TRACK时，单个IVS object对应结构体 SP_IVS_OBJ_EX；
 *			取值为IVSINFOTYPE_TRACK_EX_B0时，单个IVS object对应结构体 SP_IVS_COMMON_OBJ；
 * @param[in] nIVSBufLen 辅助帧数据长度(json或智能帧结构体数据)，
 *			  当为智能帧结构体数据时，nIVSBufLen为IVS Object的个数乘以单个IVS object的长度，单个IVS object的长度可由type备注类型获得
 * @param[in] nFrameSeq 辅助帧id
 * @param[in] pReserved 保留参数
 * @param[in] pUserData 自定义参数
 */
/* IVS类型 */
typedef enum _IVS_TYPE
{
	IVSINFOTYPE_PRESETPOS			= 1,						
	IVSINFOTYPE_MOTINTRKS			= 2,				
	IVSINFOTYPE_MOTINTRKS_EX		= 3,			
	IVSINFOTYPE_LIGHT				= 4,			
	IVSINFOTYPE_RAWDATA				= 5,   // jason数据
	IVSINFOTYPE_TRACK				= 6,   // 智能分析信息 
	IVSINFOTYPE_TRACK_EX_B0			= 7,   // 智能结构化数据信息
	IVSINFOTYPE_MOTIONFRAME			= 9,	

	IVSINFOTYPE_VIDEO_CONCENTRATION = 10,
	IVSINFOTYPE_OVERLAY_PIC			= 11,	// 叠加图片帧
	IVSINFOTYPE_OSD_INFO			= 12,	// OSD辅助帧
	IVSINFOTYPE_GPS_INFO			= 13,  // GPS辅助帧
	IVSINFOTYPE_TAGGING_INFO		= 14,  // 景物点信息标注帧，辅助帧(0x13)
	IVSINFOTYPE_TRACK_A1			= 15,  // NVR浓缩信息轨迹点
	IVSINFOTYPE_DATA_WITH_LARGE_AMOUNT = 16,
    IVSINFOTYPE_TRACK_A1_EX         = 17,  // NVR浓缩信息轨迹点(扩展)
	IVSINFOTYPE_DATA_WITH_WATER_LEVEL_MONITOR = 18, //水位检测水位尺信息帧(0x17)
	IVSINFOTYPE_INTELFLOW			= 19,  // 智能客流量
	IVSINFOTYPE_DATA_WITH_SOUND_DECIBEL = 20,	//声音警报分贝值信息帧(0x18)
	IVSINFOTYPE_DATA_WITH_SMART_MOTION = 21,	//智能动检信息帧(0x19)
}IVS_TYPE;

/* IVSINFOTYPE_OVERLAY_PIC类型对应的结构 */
typedef struct 
{
	unsigned char nOverLayPicPurpose;	// 叠加图片用途
	unsigned char nOverLayPicAction;	// 叠加图片动作
	unsigned char nOverLayPicCodeFormat;// 叠加图片编码格式
}OVERLAY_PIC_INFO;

/* IVSINFOTYPE_OSD_INFO类型对应的结构 */
typedef struct 
{
	unsigned short nOsdTopLeftCornercoordinateX; // 字符叠加区域左上角x坐标
	unsigned short nOsdTopLeftCornercoordinateY; // 字符叠加区域左上角y坐标 
	unsigned char  nOsdWordSize;				 // 字号(字符大小)
	unsigned char  nOsdWordAlignment;			 // 字符对齐方式
	unsigned char  reverse[6];
	unsigned int   nOsdRgbaValue;				 // 字符颜色	
}OSD_DATA_INFO;

typedef void (CALLMETHOD *fIVSInfoCallbackFunc)(char* pIVSBuf, LONG nIVSType, LONG nIVSBufLen, LONG nFrameSeq, void* pReserved, void* pUserData);

/*
 * IVS信息获取函数。
 *
 * @param[in] nPort 通道号
 * @param[out] pFunc 解码回调函数
 * @param[in] pUserData 自定义参数
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetIVSCallBack(LONG nPort, fIVSInfoCallbackFunc pFunc, void* pUserData);

/*
 * GPS信息回调函数。
 *
 * @param[in] pBuf 辅助帧数据
 * @param[in] nLen 辅助帧长度 
 * @param[in] pUserData 自定义
 * @return int 无意义
 */
typedef int (CALLBACK *fGPSInfoCallbackFunc)(char* pBuf, LONG nLen, void* pUserData);

/*
 * GPS信息获取函数。
 *
 * @param[in] nPort 通道号
 * @param[out] pFunc 回调函数  
 * @param[in] pUserData 自定义
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetGPSCallBack(LONG nPort, fGPSInfoCallbackFunc pFunc, void* pUserData);

/* 统计信息类型 */
typedef enum _STATISTIC_TYPE
{
	TYPE_UNUSE			= 0,		// 未使用类型
	INPUT_DATA_INTERVAL,			// 输入流数据间隔,参数1为时间间隔，参数2为数据长度
	PRASE_VIDEO_INTERVAL,			// 视频帧解析间隔，参数1为时间间隔，参数2位帧号
	VIDEO_PTS_INTERVAL,				// 视频pts间隔，参数1为时间间隔，参数2为帧号
	DECODE_VIDEO_TIME,				// 视频解码耗时，参数1为接口耗时，参数2为帧号
	PLAY_VIDEO_INTERVAL,			// 视频帧播放间隔，参数1为时间间隔，参数2为帧号
	RENDER_VIDEO_TIME,				// 视频渲染耗时，参数1为接口耗时
	VIDEO_DECODE_ERROR = 1000,		// 视频解码报错，参数1无意义，参数2为帧号
}STATISTIC_TYPE;

/* 统计信息对应的结构 */
typedef struct statistic_info
{
	LONG			nPort;				// 通道号
	LONG			nStatisticType;		// 信息类型
	LONGLONG		nParam1;			// 参数1
	LONGLONG		nParam2;			// 参数2
	char			szReserved[16];		// 保留参数
} STATISTIC_INFO, *PSTATISTIC_INFO;

/*
 * 解码播放统计信息回调
 *
 * @param[in] pStatisticData 统计数据
 * @param[in] pUserData 自定义参数
 * @return int 无意义
 */
typedef int (CALLBACK *fStatisticCallbackFunc)(PSTATISTIC_INFO pStatisticData, void* pUserData);

/*
 * 统计信息获取函数。
 *
 * @param[in] nPort,通道号
 * @param[out] pFunc 回调函数  
 * @param[in] pUserData 自定义
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetStatisticCallBack(LONG nPort, fStatisticCallbackFunc pFunc, void* pUserData);

/*
 * 设置图像显示比例
 *
 * @param[in] nPort,通道号
 * @param[in] nWidthProportion  宽值，图像宽高比
 * @param[in] nHeightProportion 高值，图像宽高比	
 * @return BOOL，成功返回TRUE，失败返回FALSE
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetViewProportion(LONG nPort, int nWidthProportion, int nHeightProportion);

/************************************************************************/
//> 不推荐使用的接口
/************************************************************************/

/*
 * 获取错误码，此接口无效，请使用PLAY_GetLastErrorEx接口。
 *
 * @param[in] nPort,通道号
 * @return DWORD，获得错误码。
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_GetLastError(LONG nPort);

/*
 * 打开文件并自动分配通道号。
 *
 * @param[in] sFileName 文件名
 * @return DWORD，失败返回0，否则返回通道号
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_CreateFile(LPSTR sFileName);

/*
 * 关闭播放文件并释放自动分配的通道号。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_DestroyFile(LONG nPort);

/*
 * 打开流接口并自动分配通道号。
 *
 * @param[in] nBufPoolSize 置播放器中存放数据流的缓冲区大小，范围是[SOURCE_BUF_MIN,SOURCE_BUF_MAX]
 * @return DWORD,失败返回0，否则返回通道号
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API DWORD CALLMETHOD PLAY_CreateStream(DWORD nBufPoolSize);

/*
 * 关闭数据流，并释放自动分配的通道号。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_DestroyStream(LONG nPort);

/*
 * 与PLAY_OneByOneBack重复，请使用PLAY_OneByOneBack。
 *
 * @param[in] nPort 通道号
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_BackOne(LONG nPort);

/*
 * 设置分辨率改变通知消息。
 *
 * @param[in] nPort 通道号
 * @param[in] hWnd 消息发送的窗口
 * @param[in] nMsg 用户输入的消息，当解码时编码格式发生改变时用户在hWnd窗口过程中收到这个消息
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetEncChangeMsg(LONG nPort,HWND hWnd,UINT nMsg);

/*
 * 设置文件结束时要发送的消息。
 *
 * @param[in] nPort 通道号
 * @param[in] hWnd 消息发送窗口
 * @param[in] nMsg 用户自定义的输入的消息，当播放到文件结束时用户在hWnd窗口过程中
 * @return BOOL，成功返回TRUE，失败返回FALSE
 * @note 如果返回失败，可以调用PLAY_GetLastErrorEx接口获取错误码。
 */
PLAYSDK_API BOOL CALLMETHOD PLAY_SetFileEndMsg(LONG nPort,HWND hWnd,UINT nMsg);

#ifdef __cplusplus
}
#endif

#endif
