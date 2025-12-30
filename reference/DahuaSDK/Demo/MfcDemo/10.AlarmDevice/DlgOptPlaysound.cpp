// DlgOptPlaysound.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgOptPlaysound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgOptPlaysound dialog


CDlgOptPlaysound::CDlgOptPlaysound(CWnd* pParent /*=NULL*/, LLONG hLoginID)
	: CDialog(CDlgOptPlaysound::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgOptPlaysound)
	m_csFilePath = _T("");
	m_hLoginID = hLoginID;
	m_edShowVolume = _T("");
	//}}AFX_DATA_INIT
	memset(&m_stuAudioOutput, 0 , sizeof(CFG_AUDIO_OUTPUT_VOLUME));
}


void CDlgOptPlaysound::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgOptPlaysound)
	DDX_Control(pDX, IDC_PLAYSOUND_COMBO_AUDIO_CHANNEL, m_cmbAudioChannel);
	DDX_Control(pDX, IDC_PLAYSOUND_SLIDER_VOLUME, m_slVolume);
	DDX_Text(pDX, IDC_EDIT_FILE, m_csFilePath);
	DDX_Text(pDX, IDC_PLAYSOUND_STATIC_SHOW_VALUE, m_edShowVolume);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgOptPlaysound, CDialog)
	//{{AFX_MSG_MAP(CDlgOptPlaysound)
	ON_BN_CLICKED(IDC_BTN_PLAY, OnBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PLAYSOUND_SLIDER_VOLUME, OnCustomdrawSliderVolume)
	ON_BN_CLICKED(IDC_PLAYSOUND_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_PLAYSOUND_BUTTON_SET, OnButtonSet)
	ON_CBN_SELCHANGE(IDC_PLAYSOUND_COMBO_AUDIO_CHANNEL, OnSelchangeComboAudioChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOptPlaysound message handlers

BOOL CDlgOptPlaysound::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_OPT_PLAYSOUND);

	if (!m_hLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
		return FALSE;
	}
	GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
	// TODO: Add extra initialization here
	m_stuAudioOutput.nAudioOutputCount = MAX_AUDIO_OUTPUT_NUM;
	OnButtonGet();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgOptPlaysound::OnBtnPlay() 
{
	// TODO: Add your control notification handler code here
	if (!m_hLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
		return;
	}

	UpdateData(TRUE);

	NET_CTRL_START_PLAYAUDIO stuPlayAudioParam = {sizeof(NET_CTRL_START_PLAYAUDIO)};
	strncpy(stuPlayAudioParam.szAudioPath, m_csFilePath, DH_MAX_AUDIO_PATH - 1);

	BOOL bRet = CLIENT_ControlDevice((LLONG)m_hLoginID, DH_CTRL_START_PLAYAUDIO, &stuPlayAudioParam, SDK_API_WAIT);

	if (!bRet)
	{
		MessageBox(ConvertString("Play sound failed!", DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
		return;
	}
	
	MessageBox(ConvertString("Play sound successfully!", DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
	GetDlgItem(IDC_BTN_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);
	
}

void CDlgOptPlaysound::OnBtnStop() 
{
	// TODO: Add your control notification handler code here
	if (!m_hLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
		return;
	}
	
	UpdateData(TRUE);

	NET_CTRL_START_PLAYAUDIO stuPlayAudioParam = {sizeof(NET_CTRL_START_PLAYAUDIO)};	
	BOOL bRet = CLIENT_ControlDevice((LLONG)m_hLoginID, DH_CTRL_STOP_PLAYAUDIO, &stuPlayAudioParam, SDK_API_WAIT);

	MessageBox(ConvertString("Stop play sound successfully!", DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
	GetDlgItem(IDC_BTN_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);
}

void CDlgOptPlaysound::OnCustomdrawSliderVolume(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int nCurSel = m_cmbAudioChannel.GetCurSel();
	if (-1 != nCurSel)
	{
		_itoa(m_slVolume.GetPos(), m_edShowVolume.GetBuffer(3), 10);
		m_stuAudioOutput.szAudioOutputVolume[nCurSel] = m_slVolume.GetPos();
		UpdateData(FALSE);
	}
	*pResult = 0;
}

void CDlgOptPlaysound::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	char szOutBuffer[1024 * 3] = {0};
	int nerror = 0;
	int nSelChn = -1; // 获取全部配置内容
	BOOL bRet = CLIENT_GetNewDevConfig((LLONG)m_hLoginID, CFG_CMD_AUDIO_OUTPUT_VOLUME, nSelChn, szOutBuffer, 1024 * 3, &nerror, SDK_API_WAIT);
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_AUDIO_OUTPUT_VOLUME, szOutBuffer, (void*)&m_stuAudioOutput, sizeof(m_stuAudioOutput), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse AudioOutputVolume error..."), DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
			return ;
		}

		// 通道数以实际返回为准
		m_cmbAudioChannel.ResetContent();
		for (int i = 0; i < m_stuAudioOutput.nAudioOutputCount; ++i)
		{
			CString csTemp;
			csTemp.Format("%s %02d", ConvertString("AudioOutputChannel", DLG_OPT_PLAYSOUND), i + 1);
			m_cmbAudioChannel.InsertString(-1, csTemp);	
		}
		ShowInfo(0);
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Get AudioOutputVolume config error:", DLG_OPT_PLAYSOUND), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return ;
	}
	return ;
}

void CDlgOptPlaysound::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	char szJsonBuf[1024 * 3] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_AUDIO_OUTPUT_VOLUME, &m_stuAudioOutput, sizeof(m_stuAudioOutput), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet AudioOutputVolume error..."), DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
		return ;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig((LLONG)m_hLoginID, CFG_CMD_AUDIO_OUTPUT_VOLUME, -1, szJsonBuf, sizeof(m_stuAudioOutput), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig AudioOutputVolume failed:", DLG_OPT_PLAYSOUND), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig AudioOutputVolume ok!"), DLG_OPT_PLAYSOUND), ConvertString("Prompt"));
		}
	}
	return ;
}

void CDlgOptPlaysound::OnSelchangeComboAudioChannel() 
{
	// TODO: Add your control notification handler code here
	int nCurSel = m_cmbAudioChannel.GetCurSel();
	ShowInfo(nCurSel);
}

void CDlgOptPlaysound::ShowInfo(int nCursel)
{
	m_cmbAudioChannel.SetCurSel(nCursel);
	m_slVolume.SetPos(m_stuAudioOutput.szAudioOutputVolume[nCursel]);
	_itoa(m_slVolume.GetPos(), m_edShowVolume.GetBuffer(3), 10);
	UpdateData(FALSE);
}