// SystemConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "SystemConfig.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSystemConfig dialog


CSystemConfig::CSystemConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemConfig::IDD, pParent)
{
	m_deviceID = -1;
	//{{AFX_DATA_INIT(CSystemConfig)
	//}}AFX_DATA_INIT
	memset(&m_di,0,sizeof(m_di));
}


void CSystemConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSystemConfig)
	DDX_Control(pDX, IDC_SETDEVICECONFIG1, m_ocx_systemset);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSystemConfig, CDialog)
	//{{AFX_MSG_MAP(CSystemConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSystemConfig message handlers

void CSystemConfig::setDeviceId(LLONG deviceID)
{
	m_deviceID = deviceID;
}
void CSystemConfig::ShowDefModal(In_DeviceInfo& di )
{
	m_di = di;
	
	DoModal();
	return;
}

BOOL CSystemConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	m_ocx_systemset.SetConfigPageHide(8);

#ifdef LANG_ENG
	m_ocx_systemset.CreateDeviceFram(0);
#else
	m_ocx_systemset.CreateDeviceFram(1);
#endif

	m_ocx_systemset.SetDeviceShowType((long *)(&m_di));
	
	return TRUE; 
}

BEGIN_EVENTSINK_MAP(CSystemConfig, CDialog)
    //{{AFX_EVENTSINK_MAP(CSystemConfig)
	ON_EVENT(CSystemConfig, IDC_SETDEVICECONFIG1, 13 , OnOnBlockAskDataSetdeviceconfig1, VTS_I4 VTS_I4 VTS_PI4 VTS_PI4 VTS_PI4 VTS_PI4)
	ON_EVENT(CSystemConfig, IDC_SETDEVICECONFIG1, 8 , OnOnSaveDeviceDataSetdeviceconfig1, VTS_I4 VTS_I4 VTS_PI4 VTS_I4 VTS_PI4)
	ON_EVENT(CSystemConfig, IDC_SETDEVICECONFIG1, 18 , OnOnDeviceSetInfoSetdeviceconfig1, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

//////////////////////for test///////////////////////



//串口配置
typedef struct {
	BYTE Com_Version[8]; /*!< 版本号 */
	BYTE Function; /*!< 串口功能 0-普通, 1-控制键盘, 2-透明串口, 3-ATM机*/
	BYTE DataBits; /*!< 数据位 取值为5,6,7,8 */
	BYTE StopBits; /*!< 停止位 0-1位, 1-1.5位, 2-2位 */
	BYTE Parity; /*!< 校验位 0-no, 1-odd, 2-even */
	DWORD BaudBase; /*!< 波特率 0-9 分别表示: {300,600,1200,2400,4800,9600,19200,38400,57600,115200}*/
} CONFIG_COMM;

//云台配置
typedef struct
{
	DWORD baudrate; // 具体值参考串口配置
	BYTE databits;
	BYTE parity;
	BYTE stopbits;
} PTZ_ATTR;

typedef struct {
	BYTE Ptz_Version[8]; /*!< 版本号 */
	PTZ_ATTR PTZAttr; /*!< 串口属性 */
	WORD DestAddr; /*!< 目的地址 0-255 */
	WORD Protocol; /*!< 协议类型 保存协议的下标，动态变化 */
	WORD MonitorAddr; /*!< 预览器地址 0-64 */
	BYTE Reserved[10]; /*!< 保留 */
} CONFIG_PTZ;

//录象（定时）配置
#define N_WEEKS 7
#define N_TSECT 6
typedef struct { // 定时时段
	BYTE StartHour; //开始时间
	BYTE StartMin;
	BYTE StartSec;
	BYTE EndHour; // 结束时间
	BYTE EndMin;
	BYTE EndSec;
	BYTE State; //第二位是定时，第三位是动态检测，第四位是报警
	BYTE Reserve;
} TSECT;
typedef struct {
	BYTE RecVersion[8]; /*!< 版本号 */
	TSECT Sector[N_WEEKS][N_TSECT]; /*!< 定时时段，一个星期七天，每天N_TSECT时间段 */
	BYTE PreRecord; /*!< 预录开关, 0-关闭, 1-打开 */
	BYTE Redundancy; /*!< 冗余开关，0-无效, 1-有效 */
	BYTE Reserved[2]; /*!< 保留 */
} CONFIG_RECORD;

//图像配置
// 使用中的编码信息结构定义（请用此结构）
typedef struct {
	BYTE CapVersion[8]; /*!< 版本号 */
	BYTE CifMode; /*!< 分辨率CIF1, CIF2, CIF4 */
	BYTE VideoType; /*!< 编码模式：MPEG1 or MPEG4 */
	BYTE EncodeMode; /*!< 码流控制 */
	BYTE ImgQlty; /*!< 码流的画质 */
	BYTE Frames; /*!< 帧率 */
	BYTE Brightness; /*!< 亮度 */
	BYTE Contrast; /*!< 对比度 */
	BYTE Saturation; /*!< 饱和度 */
	BYTE Hue; /*!< 色度 */
	BYTE AudioEn; /*!< 音频使能 */
	BYTE Gain; /*!< 增益 */
	BYTE CoverEnable; /*!< 区域遮盖开关 */
	RECT Cover; /*!< 区域遮盖范围 */
} OLD_CONFIG_CAPTURE;
//! 拟采用的编码信息结构（暂时没有使用）

typedef struct 
{
	BYTE CapVersion[8]; /*!< 版本号 */
	BYTE CifMode; /*!< 分辨率 参照枚举capture_size_t*/
	BYTE VideoType; /*!< 编码模式 参照枚举capture_comp_t */
	BYTE EncodeMode; /*!< 码流控制 参照枚举capture_brc_t */
	BYTE ImgQlty; /*!< 码流的画质 1-6 */
	BYTE Frames; /*!< 帧率 档次N制1-6, P制1-5 */
	BYTE Brightness; /*!< 亮度 0-100 */
	BYTE Contrast; /*!< 对比度 0-100 */
	BYTE Saturation; /*!< 饱和度 0-100 */
	BYTE Hue; /*!< 色度 0-100 */
	BYTE AudioEn; /*!< 音频使能 0-关闭, 1-打开 */
	BYTE Gain; /*!< 增益 0-100 */
	BYTE CoverEnable; /*!< 区域遮盖开关 0-关闭, 1-打开*/
	RECT Cover; /*!< 区域遮盖范围 */
	BYTE TimeTilteEn; /*!< 时间标题使能 0-关闭, 1-打开 */
	BYTE ChTitleEn; /*!< 通道标题使能 0-关闭, 1-打开 */
	BYTE Reserver[2]; /*!< 保留 */
} CONFIG_CAPTURE;

typedef struct
{
    unsigned char Version[8]; // 8字节的版本信息
    char HostName[16]; // 主机名
    unsigned long HostIP; // IP 地址
    unsigned long Submask; // 子网掩码
    unsigned long GateWayIP; // 网关 IP
    unsigned long DNSIP; // DNS IP
	
    // 外部接口
    unsigned long AlarmServerIP; // 报警中心IP
    unsigned short  AlarmServerPort; // 报警中心端口
    unsigned long SMTPServerIP; // SMTP server IP
    unsigned short  SMTPServerPort; // SMTP server port
    unsigned long LogServerIP; // Log server IP
    unsigned short  LogServerPort; // Log server port
	
    // 本机服务端口
    unsigned short  HttpPort; // HTTP服务端口号
    unsigned short  HttpsPort; // HTTPS服务端口号
    unsigned short  TCPPort; // TCP 侦听端口
    unsigned short  TCPMaxConn; // TCP 最大连接数
    unsigned short  SSLPort; // SSL 侦听端口
    unsigned short  UDPPort; // UDP 侦听端口
    unsigned long McastIP; // 组播IP
    unsigned short  McastPort; // 组播端口
    
    // 其他
    unsigned char  MonMode; // 预览协议 0-TCP, 1-UDP, 2-MCAST
    unsigned char  PlayMode; // 回放协议 0-TCP, 1-UDP, 2-MCAST
    unsigned char  AlmSvrStat; // 报警中心状态 0-关闭, 1-打开
}CONFIG_NET;

//alarm
typedef struct
{
	DWORD dwVersion;
	BYTE dbReserve[4];
}VER_CONFIG;
#define NAME_LEN 32

#define ALARM_SECTS 2
typedef struct 
{
	VER_CONFIG sVerConfig; /*! 配置文件版本信息 */
	DWORD AlarmMask; /*! 报警输出掩码 */
	DWORD RecordMask; /*! 录象通道掩码 */
	DWORD TimeDelay; /*! 录象延时：10～300 sec */
	BYTE SensorType; /*! 传感器类型：常开 or 常闭 */
	BYTE TimePreRec; /*! 预录时间 或者 预录开关 */
	BYTE AlarmTipEn; /*! 报警提示 */
	TSECT sAlarmSects[ALARM_SECTS]; /*! 报警输出和提示的时间段 */
	DWORD dwSectMask[ALARM_SECTS]; /*! 报警输出和提示的时间段的掩码 */
	DWORD dwInputTypeMask; /*! 报警输入类型掩码 */
	BYTE dbTourEn; /*! 是否允许报警轮训 */
	BYTE Mailer; /*! 发送邮件掩码（1-8位表示最多8通道的图像） */
	BYTE SendAlarmMsg; /*! 发送报警信息（给报警服务器等） */
	BYTE adbReserved; /*! 保留字段 */
} CONFIG_ALARM, *pCONFIG_ALARM;

//////////////////////end test///////////////////////

void CSystemConfig::OnOnBlockAskDataSetdeviceconfig1(long DeviceID, long lConfigType, long FAR* lpConfigbuf, long FAR* lpMaxLen, long FAR* lpConfigBufLen, long FAR* lpResult) 
{
	BOOL nRet = FALSE;
	NET_TIME nTime;
	if(DeviceID == 0)
	{
		return;
	}
	switch(lConfigType)
	{
	case 1:
		nRet = CLIENT_QueryDeviceTime(DeviceID, &nTime, 2000);
		if(nRet)
		{
			*(LONG *)lpConfigBufLen =  sizeof(nTime);//g_TimeOutString(&nTime).GetBuffer(0)
			memcpy((char *)lpConfigbuf,(char *)&nTime,sizeof(nTime));
		}
		else if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;
	case 2:   //系统信息5表示字符信息
		nRet = CLIENT_QuerySystemInfo(DeviceID, 5, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 2000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;
	case 3:
		nRet = CLIENT_QueryConfig(DeviceID, 10, (char *)lpConfigbuf,*(LONG *)lpMaxLen, (int *)lpConfigBufLen, 2000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;
	case 4:
		nRet = CLIENT_QueryComProtocol(DeviceID, 0, (char *)lpConfigbuf,*(LONG *)lpMaxLen, (int *)lpConfigBufLen, 2000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;
	case 5:
		nRet = CLIENT_QueryComProtocol(DeviceID, 1, (char *)lpConfigbuf,*(LONG *)lpMaxLen, (int *)lpConfigBufLen, 2000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;	
	case 6:
		nRet = CLIENT_QueryConfig(DeviceID, 210, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 2000);
		break;
	case 31:
	case 33:
	case 34: //DDNS
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 10000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		{
			//for test
			char temp[1000] = {0};
			memcpy(temp, lpConfigbuf, *lpConfigBufLen);
			//end test
		}
		break;
	case 50:
		nRet = CLIENT_QueryLog(DeviceID, (char *)lpConfigbuf,*(LONG *)lpMaxLen, (int *)lpConfigBufLen);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;
	case 51:
		ZeroMemory(lpConfigbuf,*lpMaxLen);
		nRet = 1;	
		break;

/////////////////////////////////for test////////////////////////
		
	case 20: //general
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		break;
	case 21: //comm
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		if (*lpConfigBufLen == sizeof(CONFIG_COMM))
		{
			CONFIG_COMM *temp = (CONFIG_COMM *)lpConfigbuf;
			int i = 0;
		}
		//test
		break;
	case 22:
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		if (*lpConfigBufLen == sizeof(CONFIG_NET))
		{
			CONFIG_NET *ptmpNetCfg = (CONFIG_NET *)lpConfigbuf;
			int i = 0;
		}
		break;
	case 23: //record
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		if (*lpConfigBufLen == 16 * sizeof(CONFIG_RECORD))
		{
			int length = *lpConfigBufLen;
			while (length/sizeof(CONFIG_RECORD) > 0)
			{
				CONFIG_RECORD *temp = (CONFIG_RECORD *)lpConfigbuf;
				length -= sizeof(CONFIG_RECORD);
			}
		}
		break;
	case 24: //picture
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		if (*lpConfigBufLen == 16 * sizeof(OLD_CONFIG_CAPTURE))
		{
			int length = *lpConfigBufLen;
			while (length/sizeof(OLD_CONFIG_CAPTURE) > 0)
			{
				OLD_CONFIG_CAPTURE *temp = (OLD_CONFIG_CAPTURE *)lpConfigbuf;
				length -= sizeof(OLD_CONFIG_CAPTURE);
			}
		}
		break;
	case 25: //ptz
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		if (*lpConfigBufLen == 16 * sizeof(CONFIG_PTZ))
		{
			int length = *lpConfigBufLen;
			while (length/sizeof(CONFIG_PTZ) > 0)
			{
				CONFIG_PTZ *temp = (CONFIG_PTZ *)lpConfigbuf;
				length -= sizeof(CONFIG_PTZ);
			}
		}
		break;
	case 27: //alarm
		nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		}
		if (*lpConfigBufLen == 16 * sizeof(CONFIG_ALARM))
		{
			int length = *lpConfigBufLen;
			while (length/sizeof(CONFIG_ALARM) > 0)
			{
				CONFIG_ALARM *temp = (CONFIG_ALARM *)lpConfigbuf;
				length -= sizeof(CONFIG_ALARM);
			}
		}
		break;		
//////////////////////////////test end///////////////////////////////
	default:
		if(lConfigType >= 20 && lConfigType <= 27)
		{
			nRet = CLIENT_QueryConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, *(LONG *)lpMaxLen, (int *)lpConfigBufLen, 5000);
			if (!nRet)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
			}
		}
	}
	if(nRet)
	{
		*lpResult = 1;		
	}
	else
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		*lpResult = 0;		
	}
}

void CSystemConfig::OnOnSaveDeviceDataSetdeviceconfig1(long DeviceID, long lConfigType, long FAR* lpConfigbuf, long lConfigbuflen, long FAR* lpResult) 
{
	BOOL nRet = FALSE;
	if(DeviceID < 0)
	{
		return;
	}
	switch(lConfigType)
	{
	case 1:
		nRet = CLIENT_SetupDeviceTime(DeviceID, (NET_TIME *)lpConfigbuf);
		break;
	case 3:
		nRet = 1;
		nRet = CLIENT_SetupConfig(DeviceID, 10, (char *)lpConfigbuf, lConfigbuflen, 5000);
		 if (!nRet)
		{
			 ((CNetSDKDemoDlg *)GetParent())->LastError();
		}
		break;
	case 31:
	case 33:
	case 34:
		nRet = CLIENT_SetupConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, lConfigbuflen, 5000);
		if (!nRet)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();
		}
		break;
	case 51:
		nRet = CLIENT_SetupConfig(DeviceID, 200, (char *)lpConfigbuf, lConfigbuflen, 10000);
		break;
	default:
		if((lConfigType >= 20 && lConfigType <= 27))
		{
			nRet = CLIENT_SetupConfig(DeviceID, lConfigType - 20, (char *)lpConfigbuf, lConfigbuflen, 5000);
			if (!nRet)
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();
			}
		}
	}
	if(nRet)
	{
		*lpResult = 1;		
	}
	else
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();
		*lpResult = 0;
	}
}

void CSystemConfig::OnOnDeviceSetInfoSetdeviceconfig1(long FAR* lpInfoType) 
{
	if(!*lpInfoType)
	{
		CDialog ::OnCancel();
	}
}