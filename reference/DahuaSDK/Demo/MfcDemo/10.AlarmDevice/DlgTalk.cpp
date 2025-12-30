// DlgTalk.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgTalk.h"
#include <WINSOCK2.H>
#pragma comment(lib, "ws2_32.lib")
#include "AudioBoardcastDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// 

//Audio data of audio talk callback function  
void CALLBACK AudioDataCallBack(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	CDlgTalk *dlg = (CDlgTalk *)dwUser;
	dlg->AudioData(lTalkHandle, pDataBuf,dwBufSize,byAudioFlag);
}

//Audio data of the audio talk callback 
void CDlgTalk::AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag)
{
	if(m_hTalkingHandle == lTalkHandle)
	{
		if(NET_TALK_DATA_LOCAL_AUDIO == byAudioFlag)
		{
			//It means it is the audio data from the local reocrd library
			long lSendLen = CLIENT_TalkSendData(lTalkHandle, pDataBuf, dwBufSize);
			if(lSendLen != (long)dwBufSize)
			{
				//Error occurred when sending the user audio data to the device
				
			}
		}
		else if(NET_TALK_DATA_RECV_AUDIO == byAudioFlag || NET_TALK_DATA_RECV_VIDEO == byAudioFlag)
		{
			//It means it has received the audio data from the device.
			CLIENT_AudioDecEx(lTalkHandle, pDataBuf, dwBufSize);
		}
		else if (NET_TALK_DATA_RESPOND == byAudioFlag)
		{
			OutputDebugString("device responded...\n");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgTalk dialog

CDlgTalk::CDlgTalk(CWnd* pParent /* = NULL */, LLONG lLoginId /* = NULL */, NET_DEVICE_TYPE emType /* = NET_AF6016 */, int nChannelNum /* = 1 */)
	: CDialog(CDlgTalk::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTalk)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_emDeviceType = emType;
	m_lLoginID = lLoginId;
	m_nChannelNum = nChannelNum;
	m_hTalkingHandle = 0;
	m_bAnswered = FALSE;

	memset(&m_stuAudioEncodeFormate, 0, sizeof(DHDEV_TALKFORMAT_LIST));
	InitDlg(m_lLoginID, m_nChannelNum);
}


void CDlgTalk::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTalk)
	DDX_Control(pDX, IDC_STATIC_TALK_INVITE_FLAG, m_wndInviteFlag);
	DDX_Control(pDX, IDC_COMBO_TALK_CHANNELID, m_cbChannelId);
	DDX_Control(pDX, IDC_COMBO_TALK_AUDIOFORMATE, m_cbAudioFormate);
	//}}AFX_DATA_MAP
}

BOOL CDlgTalk::OnInitDialog() 
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this, DLG_TALKING);

	unsigned int nIndex = 0;
	int ncbIndex = 0;	

	// 初始化语音编码格式
	std::string strAudioType;
	for (nIndex = 0; nIndex < m_stuAudioEncodeFormate.nSupportNum; nIndex++)
	{
		strAudioType = ConvertAudioTypeToString(m_stuAudioEncodeFormate.type[nIndex].encodeType);
		ncbIndex = m_cbAudioFormate.AddString(strAudioType.c_str());
		m_cbAudioFormate.SetItemData(ncbIndex, nIndex);
	}
	m_cbAudioFormate.SetCurSel(0);
	
	// 初始通道号
	char szTemp[64] = {0};
	m_cbChannelId.ResetContent();
	for (nIndex  = 0;  nIndex < m_nChannelNum; nIndex++)
	{
		_snprintf(szTemp, sizeof(szTemp), "%s %d", ConvertString("Channel", DLG_TALKING), nIndex + 1);
		ncbIndex = m_cbChannelId.AddString((LPCTSTR)szTemp);
		m_cbChannelId.SetItemData(ncbIndex, nIndex);
	}
	m_cbChannelId.SetCurSel(0);
	
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDlgTalk, CDialog)
	//{{AFX_MSG_MAP(CDlgTalk)
	ON_BN_CLICKED(IDC_BUTTON_TALK_STARTTALK, OnButtonTalkStarttalk)
	ON_BN_CLICKED(IDC_BUTTON_TALK_STOPTALK, OnButtonTalkStoptalk)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_TALK_BROADCAST, OnBtnBroadcast)
	ON_BN_CLICKED(IDC_CHECK_TALK_ATTACH_EVENT, OnCheckTalkAttachEvent)
	ON_BN_CLICKED(IDC_BTN_TALK_REFUSE, OnBtnTalkRefuse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTalk message handlers

void CDlgTalk::OnButtonTalkStarttalk() 
{
	// 首先对讲模式为客户端模式
	CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_CLIENT_MODE, NULL);

	// 设置语音对讲编码格式
	DHDEV_TALKDECODE_INFO stuParam;
	int ncbIndex = m_cbAudioFormate.GetCurSel();
	int nIndex = m_cbAudioFormate.GetItemData(ncbIndex);

	stuParam.encodeType = m_stuAudioEncodeFormate.type[nIndex].encodeType;
	stuParam.dwSampleRate = m_stuAudioEncodeFormate.type[nIndex].dwSampleRate;
	stuParam.nAudioBit = m_stuAudioEncodeFormate.type[nIndex].nAudioBit;
	stuParam.nPacketPeriod = m_stuAudioEncodeFormate.type[nIndex].nPacketPeriod;

	CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_ENCODE_TYPE, &stuParam);
	
	// 三代设备需要额外参数
	//if (m_emDeviceType == NET_AF6016)
	{
		NET_TALK_EX stuTalk = {sizeof(stuTalk)};
		stuTalk.nAudioPort = RECEIVER_AUDIO_PORT;	// 本地收发音频数据的端口
		stuTalk.nChannel = 0;
		stuTalk.nWaitTime = 5000;
		stuTalk.hVideoWnd = GetDlgItem(IDC_STATIC_TALK_VIDEO)->GetSafeHwnd();
		stuTalk.stuVideoFmt.dwCompression = DH_CAPTURE_COMP_DIVX_MPEG4;
		stuTalk.stuVideoFmt.nFrequency = 8000;
		CLIENT_SetDeviceMode(m_lLoginID, DH_TALK_MODE3, &stuTalk);
	}

	// 向设备发起对讲	
	m_hTalkingHandle  = CLIENT_StartTalkEx(m_lLoginID, AudioDataCallBack, (LDWORD)this);
	if (m_hTalkingHandle == 0)
	{
		// 对讲失败
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Start Talk error:", DLG_TALKING), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	}

	// 开启本地录音
	BOOL bRet = CLIENT_RecordStartEx(m_lLoginID);

	// 启动停止对讲按钮
	((CButton*)GetDlgItem(IDC_BUTTON_TALK_STOPTALK))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_TALK_STARTTALK))->EnableWindow(FALSE);

    m_wndInviteFlag.StopFlash();
}

void CDlgTalk::OnButtonTalkStoptalk() 
{
	// 关闭本地录音
	CLIENT_RecordStopEx(m_lLoginID);

	// 停止对讲
	CLIENT_StopTalk(m_hTalkingHandle);

	GetDlgItem(IDC_STATIC_TALK_VIDEO)->Invalidate(FALSE);

	// 启动对讲按钮
	((CButton*)GetDlgItem(IDC_BUTTON_TALK_STOPTALK))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_BUTTON_TALK_STARTTALK))->EnableWindow(TRUE);

	SetAnswerState(FALSE);
}

void CDlgTalk::InitDlg(LLONG hLogHandle, AV_int32 nChannelNum)
{
	m_lLoginID = hLogHandle;
	m_nChannelNum = nChannelNum;

	// 获取语音编码格式
	DHDEV_TALKFORMAT_LIST stuInfo = {0};
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginID, DH_DEVSTATE_TALK_ECTYPE, (char*)&stuInfo, sizeof(stuInfo),
		&nRetLen, 5000);
	if (bRet)
	{
		memcpy(&m_stuAudioEncodeFormate, &stuInfo, sizeof(DHDEV_TALKFORMAT_LIST));
	}
}

DH_TALK_CODING_TYPE CDlgTalk::ConvertStringToAudioType( const std::string& strType)
{
	DH_TALK_CODING_TYPE emType = DH_TALK_DEFAULT;
	
	if (strType == "PCM")
	{
		emType = DH_TALK_PCM;
	}
	else if (strType == "G.711A")
	{
		emType = DH_TALK_G711a;
	}
	else if (strType == "G.711Mu")
	{
		emType = DH_TALK_G711u;
	}

	return emType;
}

std::string CDlgTalk::ConvertAudioTypeToString( DH_TALK_CODING_TYPE emType)
{
	std::string strType = "PCM";
	
	switch (emType)
	{
	case DH_TALK_PCM:
		strType = "PCM";
		break;
	case DH_TALK_G711a:
		strType = "G711a";
		break;
	case DH_TALK_G711u:
		strType = "G711Mu";
		break;
	default:
		strType = ConvertString("Default", DLG_TALKING);//DH_TALK_DEFAULT
		break;
	}

	return strType;
}

void CDlgTalk::OnDestroy() 
{
	CDialog::OnDestroy();

    CLIENT_StopListen(m_lLoginID);
	
	// TODO: Add your message handler code here
	if (m_hTalkingHandle)
	{
		CLIENT_RecordStopEx(m_lLoginID);

		CLIENT_StopTalkEx(m_hTalkingHandle);
		m_hTalkingHandle = NULL;
	}
}

void CDlgTalk::OnBtnBroadcast() 
{
	CAudioBoardcastDlg dlg;
	dlg.DoModal();
}

BOOL CALLBACK EventCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    CDlgTalk* pThis = (CDlgTalk*)dwUser;
    if (pThis)
    {
        pThis->OnEventNotify(lCommand, lLoginID, pBuf, dwBufLen, pchDVRIP, nDVRPort);
    }
    return 0;
}

void CDlgTalk::OnEventNotify( LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort )
{
    if (lCommand == DH_ALARM_TALKING_INVITE && pBuf && dwBufLen > 0)
    {
        m_wndInviteFlag.StartFlash();
    }
    else if (lCommand == DH_ALARM_TALKING_HANGUP && pBuf && dwBufLen > 0)
    {      
        m_wndInviteFlag.StopFlash();
    }
}

void CDlgTalk::OnCheckTalkAttachEvent() 
{
	// 订阅事件
    if (IsDlgButtonChecked(IDC_CHECK_TALK_ATTACH_EVENT))
    {
        CLIENT_SetDVRMessCallBack(EventCallBack, (LDWORD)this);

        if (CLIENT_StartListenEx(m_lLoginID))
        {
            // 订阅成功
        }
        else
        {
            MessageBox(ConvertString("Attach event error."), ConvertString("Prompt"));
        }
    }
    // 取消订阅
    else
    {
        CLIENT_StopListen(m_lLoginID);
    }
}

void CDlgTalk::OnBtnTalkRefuse() 
{
    NET_CTRL_TALKING_REFUSE stuInfo = { sizeof(stuInfo) };
    if (CLIENT_ControlDevice(m_lLoginID, DH_CTRL_TALKING_REFUSE, &stuInfo))
    {
        m_wndInviteFlag.StopFlash();
    }
    else
    {
        MessageBox(ConvertString("Refuse talking error."), ConvertString("Prompt"));
    }
}
