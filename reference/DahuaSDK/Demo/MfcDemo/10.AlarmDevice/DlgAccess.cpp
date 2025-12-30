// DlgAccess.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAccess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAccess dialog


CDlgAccess::CDlgAccess(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgAccess::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAccess)
	m_strClosedoor = _T("");
	m_strInUse = _T("");
	m_strNotClose = _T("");
	m_strNotInUse = _T("");
	m_strOpenDoor = _T("");
	m_strWaiting = _T("");
	m_strAccessChName = _T("");
	m_bSnapshot = FALSE;
	m_nUnlockHoldTime = 0;
	m_nUnlockReloadTime = 0;
	//}}AFX_DATA_INIT

	m_hLogHandle = lLoginId;
	m_nPreChannelId = 0;
	m_nAccessChNum = 4; // 先默认是4个通道

	memset(&m_stSpeakCaps, 0, sizeof(m_stSpeakCaps));	
	memset(&m_stuAccessEvent, 0, sizeof(m_stuAccessEvent));
}


void CDlgAccess::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAccess)
	DDX_Control(pDX, IDC_ACCESS_CMB_ENABLEMODE, m_cmbEnableMode);
	DDX_Control(pDX, IDC_COMBO_CHANNELID, m_cbChannelId);
	DDX_Control(pDX, IDC_ACCESS_CMB_STATE, m_cbAccessState);
	DDX_Control(pDX, IDC_ACCESS_CMB_MODE, m_cbAccessMode);
	DDX_Control(pDX, IDC_COMBO_SAMPLERATE, m_cbSampleRate);
	DDX_Control(pDX, IDC_COMBO_SAMPLEBIT, m_cbSampleBit);
	DDX_Control(pDX, IDC_COMBO_BITRATE, m_cbBitRate);
	DDX_Control(pDX, IDC_COMBO_AUDIOFORMAT, m_cbAudioFormat);
	DDX_Text(pDX, IDC_EDIT_CLOSEDOOR, m_strClosedoor);
	DDX_Text(pDX, IDC_EDIT_INUSE, m_strInUse);
	DDX_Text(pDX, IDC_EDIT_NOTCLOSE, m_strNotClose);
	DDX_Text(pDX, IDC_EDIT_NOTINUSE, m_strNotInUse);
	DDX_Text(pDX, IDC_EDIT_OPENDOOR, m_strOpenDoor);
	DDX_Text(pDX, IDC_EDIT_WAITING, m_strWaiting);
	DDX_Text(pDX, IDC_EDIT_CHANNELNAME, m_strAccessChName);
	DDX_Check(pDX, IDC_CHECK_SNAPSHOT, m_bSnapshot);
	DDX_Text(pDX, IDC_EDIT_LOCKHOLDTIME, m_nUnlockHoldTime);
	DDX_Text(pDX, IDC_EDIT_LOCKRELOADTIME, m_nUnlockReloadTime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAccess, CDialog)
	//{{AFX_MSG_MAP(CDlgAccess)
	ON_BN_CLICKED(IDC_BUTTON_GETCAP, OnButtonGetcap)
	ON_BN_CLICKED(IDC_BUTTON_GETGENERALCONFIG, OnButtonGetgeneralconfig)
	ON_BN_CLICKED(IDC_BUTTON_SETGENERALCONFIG, OnButtonSetgeneralconfig)
	ON_BN_CLICKED(IDC_BUTTON_GET_EVENTCFG, OnButtonGetEventcfg)
	ON_BN_CLICKED(IDC_BUTTON_SETEVENTCFG, OnButtonSeteventcfg)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIOFORMAT, OnSelchangeComboAudioformat)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELID, OnSelchangeComboChannelid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAccess private method

void CDlgAccess::InitAccessEventCfgDlg()
{	
	int i = 0;

	m_cbChannelId.ResetContent();
	for (i = 0;  i < m_nAccessChNum; i++)
	{
		CString csTemp;
		csTemp.Format("%s %02d", ConvertString("Channel", DLG_ACCESS), i + 1);
		m_cbChannelId.InsertString(-1, csTemp);
	}
	m_cbChannelId.SetCurSel(-1);

	m_cbAccessMode.ResetContent();
	for (i = 0; i < sizeof(stuDemoAccessMode) / sizeof(stuDemoAccessMode[0]); i++)
	{
		m_cbAccessMode.InsertString(-1, ConvertString(stuDemoAccessMode[i].pszName, DLG_ACCESS));
	}
	m_cbAccessMode.SetCurSel(-1);

	m_cbAccessState.ResetContent();
	for (i = 0; i < sizeof(stuDemoAccessState) / sizeof(stuDemoAccessState[0]); i++)
	{
		m_cbAccessState.InsertString(-1, ConvertString(stuDemoAccessState[i].pszName, DLG_ACCESS));
	}
	m_cbAccessState.SetCurSel(-1);

	m_cmbEnableMode.ResetContent();
	m_cmbEnableMode.InsertString(-1, ConvertString("LowVoltage", DLG_ACCESS));
	m_cmbEnableMode.InsertString(-1, ConvertString("HighVoltage", DLG_ACCESS));
	m_cmbEnableMode.SetCurSel(-1);

	SetDlgItemText(IDC_EDIT_CHANNELNAME, "");
}

void CDlgAccess::AccessEventCfgToDlg()
{
	int nSelChn = m_cbChannelId.GetCurSel();
	if (-1 == nSelChn)
	{
		return;
	}
	
	m_cbAccessMode.SetCurSel((int)m_stuAccessEvent[nSelChn].emMode);
	m_cbAccessState.SetCurSel((int)m_stuAccessEvent[nSelChn].emState);
	m_cmbEnableMode.SetCurSel((int)m_stuAccessEvent[nSelChn].nEnableMode);
	SetDlgItemText(IDC_EDIT_CHANNELNAME, m_stuAccessEvent[nSelChn].szChannelName);
}

void CDlgAccess::DlgToAccessEventCfg()
{	
	int nSelChn = m_cbChannelId.GetCurSel();
	if (-1 == nSelChn)
	{
		return;
	}

	m_stuAccessEvent[nSelChn].emMode = (CFG_ACCESS_MODE)m_cbAccessMode.GetCurSel();
	m_stuAccessEvent[nSelChn].emState = (CFG_ACCESS_STATE)m_cbAccessState.GetCurSel();
	m_stuAccessEvent[nSelChn].nEnableMode = m_cmbEnableMode.GetCurSel();
	GetDlgItemText(IDC_EDIT_CHANNELNAME, m_stuAccessEvent[nSelChn].szChannelName, sizeof(m_stuAccessEvent[nSelChn].szChannelName));
}

BOOL CDlgAccess::SetAccessEventCfg()
{
	int nSelChn = m_cbChannelId.GetCurSel();
	if (-1 == nSelChn)
	{
		return FALSE;
	}
		
	char szJson[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ACCESS_EVENT, &m_stuAccessEvent[nSelChn], sizeof(m_stuAccessEvent[nSelChn]), szJson, sizeof(szJson));
	if (bRet)
	{
		int nErr = 0, nRestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_hLogHandle, CFG_CMD_ACCESS_EVENT, nSelChn, 
			szJson, strlen(szJson), &nErr, &nRestart, SDK_API_WAIT);
		if (bRet)
		{
			//memcpy(&m_stuAccessEvent[nSelChn], &stuIn, sizeof(CFG_ACCESS_EVENT_INFO));
			MessageBox(ConvertString("Set config of AccessEvent successfully!", DLG_ACCESS), ConvertString("Prompt"));
		} 
		else
		{
			CString csErr;
			//csErr.Format("Set config of AccessEvent error: %08x", AV_GetLastError());
			csErr.Format("%s %08x", ConvertString("Set config of AccessEvent error:", DLG_ACCESS), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
	} 
	else
	{
		MessageBox(ConvertString("Packet AccessEvent error!", DLG_ACCESS), ConvertString("Prompt"));
		return FALSE;
	}

	return TRUE;
}

BOOL CDlgAccess::GetAccessEventCfg()
{
	int nSelChn = m_cbChannelId.GetCurSel();
	if (-1 == nSelChn)
	{
		return FALSE;
	}
	
	char szJson[1024] = {0};

	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_hLogHandle, CFG_CMD_ACCESS_EVENT, nSelChn,
		szJson, sizeof(szJson), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_ACCESS_EVENT_INFO stuInfo = {0};
		DWORD dwRet = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ACCESS_EVENT, szJson, &stuInfo, sizeof(stuInfo), &dwRet);
		if (bRet && sizeof(CFG_ACCESS_EVENT_INFO) == dwRet)
		{
			memcpy(&m_stuAccessEvent[nSelChn], &stuInfo, sizeof(CFG_ACCESS_EVENT_INFO));
		} 
		else
		{
			MessageBox(ConvertString("Parse AlarmOut error!", DLG_ACCESS), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query config of AlarmOut error:", DLG_ACCESS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAccess message handlers

BOOL CDlgAccess::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ACCESS);
	// 初始化门禁状态
	int nIndex = m_cbAccessState.AddString(ConvertString("Normal", DLG_ACCESS));
	m_cbAccessState.SetItemData(nIndex, ACCESS_STATE_NORMAL);
	nIndex = m_cbAccessState.AddString(ConvertString("CloseAlways", DLG_ACCESS));
	m_cbAccessState.SetItemData(nIndex, ACCESS_STATE_CLOSEALWAYS);
	nIndex = m_cbAccessState.AddString(ConvertString("OpenAlways", DLG_ACCESS));
	m_cbAccessState.SetItemData(nIndex, ACCESS_STATE_OPENALWAYS);
	m_cbAccessState.SetCurSel(-1);
	
	// 初始化门禁模式
	nIndex = m_cbAccessMode.AddString(ConvertString("HandProtected", DLG_ACCESS));
	m_cbAccessMode.SetItemData(nIndex, ACCESS_MODE_HANDPROTECTED);
	nIndex = m_cbAccessMode.AddString(ConvertString("SafeRoom", DLG_ACCESS));
	m_cbAccessMode.SetItemData(nIndex, ACCESS_MODE_SAFEROOM);
	nIndex = m_cbAccessMode.AddString(ConvertString("Other", DLG_ACCESS));
	m_cbAccessMode.SetItemData(nIndex, ACCESS_MODE_OTHER);
	m_cbAccessMode.SetCurSel(-1);

	InitAccessEventCfgDlg();
	return TRUE;
}

void CDlgAccess::OnButtonGetcap() 
{
	char szJson[1024] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLogHandle, CFG_CAP_CMD_SPEAK, -1,
		szJson, sizeof(szJson), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		int nRet = 0;
		bRet = CLIENT_ParseData(CFG_CAP_CMD_SPEAK, szJson, &m_stSpeakCaps, sizeof(m_stSpeakCaps), &nRet);
		if (bRet && nRet == sizeof(m_stSpeakCaps))
		{
			int nCapsNum = m_stSpeakCaps.nAudioCapNum;
			char szType[][32] = {"PCM","ADPCM","G.711a","G.711Mu", "G.726", "G.729", "MPEG2","AMR","ACC"};
			
			m_cbAudioFormat.ResetContent();
			for (int nIndex = 0; nIndex < nCapsNum; ++nIndex)
			{
				int nType = m_stSpeakCaps.stuAudioCap[nIndex].emCompression;
				m_cbAudioFormat.AddString(szType[nType]);
				m_cbAudioFormat.SetItemData(nIndex, nIndex); // 记录数组下标
			}
			m_cbAudioFormat.SetCurSel(0);
			OnSelchangeComboAudioformat();
		}
		else
		{
			MessageBox(ConvertString("Parse speak cap failed!", DLG_ACCESS), ConvertString("Prompt"));
		}
	} 
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("get config speak cap failed:", DLG_ACCESS),CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
}

void CDlgAccess::OnButtonGetgeneralconfig() 
{
	// 初始化接口参数
	int nJsonBufLen = 1024 *8;
	char *pBuf = new char[nJsonBufLen];
	if (NULL == pBuf)
	{
		// 申请内存失败
		return;
	}
	memset(pBuf, 0, nJsonBufLen);

	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_hLogHandle, CFG_CMD_ACCESS_GENERAL, -1,
		pBuf, nJsonBufLen, &nErr, SDK_API_WAIT);
	if (bRet)
	{
		// 调用接口解析json串
		CFG_ACCESS_GENERAL_INFO stuAccessGeneral = {0};
		bRet = CLIENT_ParseData(CFG_CMD_ACCESS_GENERAL, pBuf, &stuAccessGeneral, sizeof(stuAccessGeneral), NULL);

		// 将解析出来的信息显示到界面上
		if (bRet)
		{
			m_strClosedoor = stuAccessGeneral.szCloseDoorAudioPath;
			m_strInUse = stuAccessGeneral.szInUsedAuidoPath;
			m_strNotClose = stuAccessGeneral.szNotClosedAudioPath;
			m_strOpenDoor = stuAccessGeneral.szOpenDoorAudioPath;
			m_strWaiting = stuAccessGeneral.szWaitingAudioPath;
			m_strNotInUse = stuAccessGeneral.szPauseUsedAudioPath;
			m_nUnlockHoldTime = stuAccessGeneral.nUnlockHoldTime;
			m_nUnlockReloadTime = stuAccessGeneral.nUnlockReloadTime;
		}
		else
		{
			MessageBox(ConvertString("Parse config of AccessGeneral error!", DLG_ACCESS), ConvertString("Prompt"));
		}
	}
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query config of AccessGeneral error:", DLG_ACCESS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}

	if (pBuf)
	{
		delete []pBuf;
	}

	UpdateData(FALSE);
}

void CDlgAccess::OnButtonSetgeneralconfig() 
{
	// 获取界面上的信息
	UpdateData(TRUE);

	CFG_ACCESS_GENERAL_INFO stuAccessGeneral = {0};

	strncpy(stuAccessGeneral.szCloseDoorAudioPath, m_strClosedoor, sizeof(stuAccessGeneral.szCloseDoorAudioPath) - 1);
	strncpy(stuAccessGeneral.szInUsedAuidoPath, m_strInUse, sizeof(stuAccessGeneral.szInUsedAuidoPath) - 1);
	strncpy(stuAccessGeneral.szNotClosedAudioPath, m_strNotClose, sizeof(stuAccessGeneral.szNotClosedAudioPath) - 1);
	strncpy(stuAccessGeneral.szOpenDoorAudioPath, m_strOpenDoor, sizeof(stuAccessGeneral.szOpenDoorAudioPath) - 1);
	strncpy(stuAccessGeneral.szWaitingAudioPath, m_strWaiting, sizeof(stuAccessGeneral.szWaitingAudioPath) - 1);
	strncpy(stuAccessGeneral.szPauseUsedAudioPath, m_strNotInUse, sizeof(stuAccessGeneral.szPauseUsedAudioPath) - 1);
	stuAccessGeneral.nUnlockHoldTime = m_nUnlockHoldTime;
	stuAccessGeneral.nUnlockReloadTime = m_nUnlockReloadTime;

	// 组json串
	int nJsonBufLen = 1024 * 8;
	char *pJsonBuf = new char[nJsonBufLen];
	if (NULL == pJsonBuf)
	{
		// 申请内存失败
		return;
	}
	BOOL bret = CLIENT_PacketData(CFG_CMD_ACCESS_GENERAL, &stuAccessGeneral, sizeof(stuAccessGeneral), pJsonBuf, nJsonBufLen);
	if (bret)
	{
		// 将修改内容设置到设备上
		int nErr = 0, nRestart = 0;
		bret = CLIENT_SetNewDevConfig(m_hLogHandle, CFG_CMD_ACCESS_GENERAL, -1,
			pJsonBuf, strlen(pJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (bret)
		{
			MessageBox(ConvertString("Set config of AccessGeneral ok!", DLG_ACCESS), ConvertString("Prompt"));
		}
		else
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("Set config of AccessGeneral error:", DLG_ACCESS), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
		}
	}
	else
	{
		MessageBox(ConvertString("Packet config of AccessGeneral error!", DLG_ACCESS), ConvertString("Prompt"));
	}

	if (pJsonBuf != NULL)
	{
		delete []pJsonBuf;
	}
}

void CDlgAccess::OnButtonGetEventcfg() 
{
	if (GetAccessEventCfg())
	{
		AccessEventCfgToDlg();
	}
}

void CDlgAccess::OnButtonSeteventcfg() 
{
	DlgToAccessEventCfg();
	SetAccessEventCfg();
}

// 改变音频类型处理函数
void CDlgAccess::OnSelchangeComboAudioformat() 
{
	// 得到当前能力索引号
	int nIndex = m_cbAudioFormat.GetCurSel();
	if (nIndex < 0)
	{
		return;
	}
	nIndex = m_cbAudioFormat.GetItemData(nIndex);

	// 显示相关信息
	// 码流大小
	int i =0;
	char szTemp[32] = {0};
	CFG_CAP_AUDIO_FORMAT& stuAudioCap = m_stSpeakCaps.stuAudioCap[nIndex];
	m_cbBitRate.ResetContent();
	for (i = 0; i < stuAudioCap.nPropertyNum; ++i)
	{
		m_cbBitRate.AddString(_itoa(stuAudioCap.stuProperty[i].nBitRate, szTemp, 10));
	}
	m_cbBitRate.SetCurSel(0);

	// 采样位深
	m_cbSampleBit.ResetContent();
	for (i = 0; i < stuAudioCap.nPropertyNum; ++i)
	{
		m_cbSampleBit.AddString(_itoa(stuAudioCap.stuProperty[i].nSampleBit, szTemp, 10));
	}
	m_cbSampleBit.SetCurSel(0);

	// 采样率
	m_cbSampleRate.ResetContent();
	for (i = 0; i < stuAudioCap.nPropertyNum; ++i)
	{
		m_cbSampleRate.AddString(_itoa(stuAudioCap.stuProperty[i].nSampleRate, szTemp, 10));
	}
	m_cbSampleRate.SetCurSel(0);

}

// 将界面内容保存到对应通道
void CDlgAccess::SaveChannelInfo(int nChannelId)
{
	UpdateData(TRUE);
	m_stuAccessEvent[nChannelId].bSnapshotEnable = m_bSnapshot;
	strncpy(m_stuAccessEvent[nChannelId].szChannelName, m_strAccessChName, sizeof(m_stuAccessEvent[nChannelId].szChannelName) - 1);
	int nIndex = m_cbAccessState.GetCurSel();
	m_stuAccessEvent[nChannelId].emState = (CFG_ACCESS_STATE)m_cbAccessState.GetItemData(nIndex);
	nIndex = m_cbAccessMode.GetCurSel();
	m_stuAccessEvent[nChannelId].emMode = (CFG_ACCESS_MODE)m_cbAccessMode.GetItemData(nIndex);
}

void CDlgAccess::ShowChannelInfo(int nChannelId)
{
	m_bSnapshot = m_stuAccessEvent[nChannelId].bSnapshotEnable;
	m_strAccessChName = m_stuAccessEvent[nChannelId].szChannelName;
	
	int i = 0;
	for (i = 0; i <= m_cbAccessState.GetCount(); ++i)
	{
		if (m_cbAccessState.GetItemData(i) == (DWORD)m_stuAccessEvent[nChannelId].emState)
		{
			m_cbAccessState.SetCurSel(i);
		}
	}
	
	for (i = 0; i <= m_cbAccessMode.GetCount(); ++i)
	{
		if (m_cbAccessMode.GetItemData(i) == (DWORD)m_stuAccessEvent[nChannelId].emMode)
		{
			m_cbAccessMode.SetCurSel(i);
		}
	}

	UpdateData(FALSE);
}

// 改变门禁通道号处理函数
void CDlgAccess::OnSelchangeComboChannelid() 
{
	if (GetAccessEventCfg())
	{
		AccessEventCfgToDlg();
	}
}
