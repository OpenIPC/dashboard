// DlgRestoreCfgNew.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgRestoreCfgNew.h"
#

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgRestoreCfg dialog

const char* const g_szCfgToRestore[] = {
	CFG_CMD_ENCODE,
	CFG_CMD_RECORD,
	CFG_CMD_ALARMINPUT,
	CFG_CMD_NETALARMINPUT,
	CFG_CMD_MOTIONDETECT,
	CFG_CMD_VIDEOLOST,
	CFG_CMD_VIDEOBLIND,
	CFG_CMD_STORAGENOEXIST,
	CFG_CMD_STORAGEFAILURE,
	CFG_CMD_STORAGELOWSAPCE,
	CFG_CMD_NETABORT,
	CFG_CMD_IPCONFLICT,
	CFG_CMD_SNAPCAPINFO,
	CFG_CMD_NAS,
	CFG_CMD_PTZ,
	CFG_CMD_PTZ_AUTO_MOVEMENT,
	CFG_CMD_WATERMARK,
	CFG_CMD_ANALYSEGLOBAL,
	CFG_CMD_ANALYSEMODULE,
	CFG_CMD_ANALYSERULE,
	CFG_CMD_ANALYSESOURCE,
	CFG_CMD_RAINBRUSH,
	CFG_CMD_INTELLECTIVETRAFFIC,
	CFG_CMD_TRAFFICGLOBAL,
	CFG_CMD_DEV_GENERRAL,
	CFG_CMD_ATMMOTION,
	CFG_CMD_DEVICESTATUS,
	CFG_CMD_HARDDISKTANK,
	CFG_CMD_RAIDGROUP,
	CFG_CMD_STORAGEPOOLGROUP,
	CFG_CMD_STORAGEPOSITIONGROUP,
	CFG_CMD_VIDEOINDEVGROUP,
	CFG_CMD_DEVRECORDGROUP,
	CFG_CMD_IPSSERVER,
	CFG_CMD_SNAPSOURCE,
	CFG_CMD_DHRADER,
	CFG_CMD_TRANSRADER,
	CFG_CMD_LANDUNRADER,
	CFG_CMD_LANDUNCOILS,
	CFG_CMD_MATRIX_SPOT,
	CFG_CMD_HDVR_DSP,
	CFG_CMD_HDVR_ATTR_CFG,
	CFG_CMD_CHANNEL_HOLIDAY,
	CFG_CMD_HEALTH_MAIL,
	CFG_CMD_CAMERAMOVE,
	CFG_CMD_SPLITTOUR,
	CFG_CMD_VIDEOENCODEROI,
	CFG_CMD_VIDEO_INMETERING,
	CFG_CMD_TRAFFIC_FLOWSTAT,
	CFG_CMD_HDMIMATRIX,
	CFG_CMD_VIDEOINOPTIONS,
	CFG_CMD_RTSP,
	CFG_CMD_TRAFFICSNAPSHOT,
	CFG_CMD_TRAFFICSNAPSHOT_MULTI,
	CFG_CMD_MULTICAST,
	CFG_CMD_VIDEODIAGNOSIS_PROFILE,
	CFG_CMD_VIDEODIAGNOSIS_TASK,
	CFG_CMD_VIDEODIAGNOSIS_PROJECT,
	CFG_CMD_VIDEODIAGNOSIS_REALPROJECT,
	CFG_CMD_VIDEODIAGNOSIS_GLOBAL,
	CFG_CMD_VIDEODIAGNOSIS_TASK_ONE,
	CFG_CMD_TRAFFIC_WORKSTATE,
	CFG_CMD_STORAGEDEVGROUP,
	CFG_CMD_RECORDTOGROUP,
	CFG_CMD_INTELLITRACKSCENE,
	CFG_CMD_IVSFRAM_RULE,
	CFG_CMD_RECORD_STORAGEPOINT,
	CFG_CMD_MD_SERVER,
	CFG_CMD_CHANNELTITLE,
	CFG_CMD_RECORDMODE,
	CFG_CMD_VIDEOOUT,
	CFG_CMD_REMOTEDEVICE,
	CFG_CMD_REMOTECHANNEL,
	CFG_CMD_MONITORTOUR,
	CFG_CMD_MONITORCOLLECTION,
	CFG_CMD_DISPLAYSOURCE,
	CFG_CMD_RAID,
	CFG_CMD_RECORDSOURCE,
	CFG_CMD_VIDEOCOLOR,
	CFG_CMD_VIDEOWIDGET,
	CFG_CMD_STORAGEGROUP,
	CFG_CMD_LOCALS,
	CFG_CMD_LANGUAGE,
	CFG_CMD_ACCESSFILTER,
	CFG_CMD_AUTOMAINTAIN,
	CFG_CMD_REMOTEEVENT,
	CFG_CMD_MONITORWALL,
	CFG_CMD_SPLICESCREEN,
	CFG_CMD_TEMPERATUREALARM,
	CFG_CMD_FANSPEEDALARM,
	CFG_CMD_RECORDBACKUP,
	CFG_CMD_COMM,
	CFG_CMD_NETWORK,
	CFG_CMD_NASEX,
	CFG_CMD_LDAP,
	CFG_CMD_ACTIVE_DIR,
	CFG_CMD_FLASH,
	CFG_CMD_AUDIO_ANALYSERULE,
	CFG_CMD_JUDICATURE,
	CFG_CMD_GOODS_WEIGHT,
	CFG_CMD_VIDEOIN,
	CFG_CMD_ENCODEPLAN,
	CFG_CMD_PICINPIC,
	CFG_CMD_BURNFULL,
	CFG_CMD_MASTERSLAVE_GLOBAL,
	CFG_CMD_ANALYSEWHOLE,
	CFG_CMD_VIDEO_IN_BOUNDARY,
	CFG_CMD_MONITORWALL_COLLECTION,
	CFG_CMD_ANALOGMATRIX,
	CFG_CMD_ANALOG_MATRIX_PROTOCOL,
	CFG_CMD_VIDEO_OUT_TITLE,
	CFG_CMD_DISK_FLUX_ALARM,
	CFG_CMD_NET_FLUX_ALARM,
	CFG_CMD_DVRIP,
	CFG_CMD_PARKINGSPACE_SCREEN,
	CFG_CMD_PARKINGSPACE_STATUS,
	CFG_CMD_CLIENT_CUSTOM_DATA,
	CFG_CMD_BURN_RECORD_FORMAT,
	CFG_CMD_MULTIBURN,
	CFG_CMD_ENCODE_ENCRYPT,
	CFG_CMD_VIDEO_IN_ZOOM,
	CFG_CMD_SNAP,
	CFG_CMD_REMOTE_STORAGE_LIMIT,
	CFG_CMD_SPECIAL_DIR,
	CFG_CMD_AUTO_STARTUP_DELAY,
	CFG_CMD_CANFILTER,
	CFG_CMD_VIDEOIN_FOCUS,
	CFG_CMD_ENCODE_ADAPT,
	CFG_CMD_VIDEOANALYSE_CALIBRATE,
	CFG_CMD_PTZ_PRESET,
	CFG_CMD_TIMESPEEDLIMIT,
	CFG_CMD_VOICEALERT,
	CFG_CMD_DEVICEKEEPALIVE,
	CFG_CMD_AUDIO_SPIRIT,
	CFG_CMD_COMPOSE_CHANNEL,
	CFG_CMD_LOWER_MATRIX,
	CFG_CMD_INFRARED_BOARD_TEMPLATE,
	CFG_CMD_INFRARED_BOARD,
	CFG_CMD_VIDEOIN_EXPOSURE,
	CFG_CMD_ACCESS_GENERAL,
	CFG_CMD_ACCESS_EVENT,
	CFG_CMD_WIRELESS,
	CFG_CMD_ALARMSERVER,
	CFG_CMD_COMMGLOBAL,
	CFG_CMD_ANALOGALARM,
	CFG_CMD_ALARMOUT,
	CFG_CMD_NTP,
	CFG_CMD_ALARMBELL,
	CFG_CMD_MOBILE,
	CFG_CMD_PHONEEVENTNOTIFY,
	CFG_CMD_PSTN_ALARM_SERVER,
	CFG_CMD_AUDIO_OUTPUT_VOLUME,
	CFG_CMD_ALARMKEYBOARD,
	CFG_CMD_POWERFAULT,
	CFG_CMD_CHASSISINTRUSION,
	CFG_CMD_EXALARMBOX,
	CFG_CMD_EXALARMOUTPUT,
	CFG_CMD_EXALARMINPUT,
	CFG_CMD_ACCESSTIMESCHEDULE,
	CFG_CMD_URGENCY,
	CFG_CMD_SENSORSAMPLING,
	CFG_CMD_STP,
	CFG_CMD_ALARM_SUBSYSTEM,
	CFG_CMD_BATTERY_LOW_POWER,
	CFG_CMD_SNAPLIKAGE,
	CFG_CMD_AUDIOINPUT,
	CFG_CMD_EMAIL,
	CFG_CMD_AIRCONDITION,
	CFG_CMD_MACCONFLICT,
	CFG_CMD_PSTN_BREAK_LINE,
	CFG_CMD_NET_COLLECTION,
	CFG_CMD_ALARM_SLOT_BOND,
	"UnknownCFG"
};


CDlgRestoreCfgNew::CDlgRestoreCfgNew(CWnd* pParent /* = NULL */, LLONG hLoginId /* = NULL */)
	: CDialog(CDlgRestoreCfgNew::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgRestoreCfg)
		// NOTE: the ClassWizard will add member initialization here
	m_hLoginId = hLoginId;
	m_pbtnCheck = NULL;
	//}}AFX_DATA_INIT
}


void CDlgRestoreCfgNew::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgRestoreCfg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgRestoreCfgNew, CDialog)
	//{{AFX_MSG_MAP(CDlgRestoreCfg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgRestoreCfg private handlers

void CDlgRestoreCfgNew::InitDlg()
{
	int nCfgCount = sizeof(g_szCfgToRestore) / sizeof(g_szCfgToRestore[0]);
	int nColumnMaxCount = 5;
	int nRowMaxCount = (nCfgCount%nColumnMaxCount == 0 ? nCfgCount/nColumnMaxCount : (nCfgCount/nColumnMaxCount + 1));
	int nBtnWidth = 150;	// 100 * 1.5 
	int nBtnHeight = 20;	// 10 * 1.5
	int nInterval = 6;

	{		
		CRect rcBtn;
		GetDlgItem(IDOK)->GetClientRect(&rcBtn);

		int nWndWidth = nBtnWidth * nColumnMaxCount + nInterval * (nColumnMaxCount + 1);
		int nWndHeight = nBtnHeight * nRowMaxCount + nInterval * (nRowMaxCount + 1)
			+ (rcBtn.Height()*2 + nInterval * 4);
		MoveWindow(0, 0, nWndWidth, nWndHeight);
		
		GetDlgItem(IDOK)->MoveWindow(nWndWidth / 2 - rcBtn.Width() - nInterval, nWndHeight - rcBtn.Height()*2 - nInterval * 2, 
			rcBtn.Width(), rcBtn.Height());
		GetDlgItem(IDCANCEL)->MoveWindow(nWndWidth / 2 + nInterval, nWndHeight - rcBtn.Height()*2 - nInterval * 2,
			rcBtn.Width(), rcBtn.Height());

		CenterWindow();
	}

	m_pbtnCheck = new CButton[nCfgCount];

	for (int i = 0; i < nCfgCount; i++)
	{
		m_pbtnCheck[i].Create("", WS_CHILD | WS_VISIBLE | /*BS_CHECKBOX |*/ BS_AUTOCHECKBOX, 
			CRect(0, 0, 0, 0), this, emBtnID_base + i);
		m_pbtnCheck[i].SetWindowText(g_szCfgToRestore[i]);

		int nSpaceX = i - i/nColumnMaxCount * nColumnMaxCount;
		int nSpaceY = (i - i%nColumnMaxCount) / nColumnMaxCount;//i - i/nRowMaxCount * nRowMaxCount;

		m_pbtnCheck[i].MoveWindow(nInterval * (1 + nSpaceX) + nBtnWidth * nSpaceX, 
			nInterval * (1 + nSpaceY) + nBtnHeight * nSpaceY, 
			nBtnWidth, nBtnHeight, TRUE);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDlgRestoreCfg message handlers

BOOL CDlgRestoreCfgNew::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_RECOVERCONFIG);
	// TODO: Add extra initialization here
	InitDlg();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgRestoreCfgNew::OnOK() 
{
	// TODO: Add extra validation here	
	CString csCfgNames;
	int nCount = 0;
	for (int i = 0; i < sizeof(g_szCfgToRestore)/sizeof(g_szCfgToRestore[0]); i++)
	{
		if (m_pbtnCheck[i].GetCheck())
		{
			CString csName;
			m_pbtnCheck[i].GetWindowText(csName);
			if (nCount == 0)
			{
				csCfgNames += csName;
			}
			else
			{
				csCfgNames += "&&";
				csCfgNames += csName;
			}
			nCount++;
		}
	}
	
    NET_CTRL_RESTORE_DEFAULT stCtrParam = {sizeof(stCtrParam)};
    stCtrParam.szCfgNames = csCfgNames.GetBuffer(0);

	// 可同时恢复多个配置
	// 如果返回TRUE，表示所有指定的配置都恢复成默认
	// 如果返回FALSE，表示至少有一个配置恢复失败，所有指定的配置都不变化
    BOOL bRet = CLIENT_ControlDevice(m_hLoginId , DH_CTRL_RESTOREDEFAULT_EX , &stCtrParam );
	CString csOut;
	if (bRet)
	{
		MessageBox(ConvertString("Set to default ok", DLG_RECOVERCONFIG), ConvertString("Prompt"));
	} 
	else
	{
		csOut.Format("%s:%08x", ConvertString("Set to default failed", DLG_RECOVERCONFIG),  CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return;
	}
	
	CDialog::OnOK();
}

void CDlgRestoreCfgNew::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_pbtnCheck)
	{
		for (int i = 0; i < sizeof(g_szCfgToRestore) / sizeof(g_szCfgToRestore[0]); i++)
		{
			if (m_pbtnCheck[i].GetSafeHwnd())
			{
				m_pbtnCheck[i].DestroyWindow();
			}
		}
		delete []m_pbtnCheck;
		m_pbtnCheck = NULL;
	}
}
