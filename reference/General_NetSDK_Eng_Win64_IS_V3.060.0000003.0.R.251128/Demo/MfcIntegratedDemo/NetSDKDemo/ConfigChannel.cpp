// ConfigChannel.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigChannel.h"
#include "NetSDKDemoDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigChannel dialog


CConfigChannel::CConfigChannel(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigChannel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigChannel)
	//}}AFX_DATA_INIT
	m_pDev = 0;;
	memset(&m_chnCFG, 0, 16*sizeof(DHDEV_CHANNEL_CFG));
	memset(&m_sysCFG, 0, sizeof(DHDEV_SYSTEM_ATTR_CFG));
	m_bInited = FALSE;
	m_chnIdx = -1;
	m_strmIdx = -1;
	m_osdIdx = -1;
	m_dontsave = 0;
}


void CConfigChannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigChannel)
	DDX_Control(pDX, IDC_CHECK_GAIN, m_gainchk);
	DDX_Control(pDX, IDC_COMBO_CHANNO, m_channelsel);
	DDX_Control(pDX, IDC_CHECK_OSDSHOW, m_osdshowchk);
	DDX_Control(pDX, IDC_COMBO_RESOLUTION, m_resolutionsel);
	DDX_Control(pDX, IDC_COMBO_ENCODETYPE, m_encodetypesel);
	DDX_Control(pDX, IDC_COMBO_BLINDTYPE, m_blindtypesel);
	DDX_Control(pDX, IDC_COMBO_AUDIOFMT, m_audiofmtsel);
	DDX_Control(pDX, IDC_COMBO_OSD, m_osdsel);
	DDX_Control(pDX, IDC_COMBO_IMGQUALITY, m_imgqualitysel);
	DDX_Control(pDX, IDC_COMBO_FRAME, m_framesel);
	DDX_Control(pDX, IDC_COMBO_BRCTRL, m_bitratectrlsel);
	DDX_Control(pDX, IDC_CHECK_VIDEO, m_videochk);
	DDX_Control(pDX, IDC_CHECK_AUDIO, m_audiochk);
	DDX_Control(pDX, IDC_COMBO_STREAM, m_streamsel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigChannel, CDialog)
	//{{AFX_MSG_MAP(CConfigChannel)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNO, OnSelchangeComboChanno)
	ON_CBN_SELCHANGE(IDC_COMBO_STREAM, OnSelchangeComboStream)
	ON_CBN_SELCHANGE(IDC_COMBO_OSD, OnSelchangeComboOsd)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_UNDO_ALL2, OnUndoAll)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_CBN_DROPDOWN(IDC_COMBO_CHANNO, OnDropdownComboChanno)
	ON_EN_CHANGE(IDC_EDIT_BRIGHTNESS, OnChangeEditBrightness)
	ON_EN_CHANGE(IDC_EDIT_CONTRAST, OnChangeEditContrast)
	ON_EN_CHANGE(IDC_EDIT_SATURATION, OnChangeEditSaturation)
	ON_EN_CHANGE(IDC_EDIT_HUE, OnChangeEditHue)
	ON_EN_CHANGE(IDC_EDIT_GAIN, OnChangeEditGain)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigChannel message handlers

void GetMaskedIndex(BYTE *des, BYTE src, DWORD dwMask)
{
	if (!des )
	{
		return;
	}
	char idx = -1;
	for (int i = 0; i< 32; i++)
	{
		if ((dwMask&(0x01<<i))) 
		{
			idx++;
		}

		if (idx >= 0 && i >= src)
		{
			break;
		}
	}
	*des = (idx >= 0)?idx:0;
}

void GetOrigIndex(BYTE *des, BYTE src, DWORD dwMask)
{
	if (!des)
	{
		return;
	}
	char idx = -1;
	int i = 0;
	for (i = 0; i< 32; i++)
	{
		if ((dwMask&(0x01<<i))) 
		{
			idx++;
		}

		if (idx >= 0 && idx >= src)
		{
			break;
		}
	}
	*des = (idx >= 0)?i:0;
}

BOOL CConfigChannel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	if (!m_pDev)
	{
		return TRUE;
	}

	CString strChn;
	for (int i = 0; i < m_pDev->Info.nChanNum; i++)
	{
		strChn.Format(ConvertString("Channel_%d"), i+1);
		m_channelsel.InsertString(i, strChn);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigChannel::GetConfig()
{
	if (!m_pDev || m_bInited)
	{
		return;
	}
	
	while(m_resolutionsel.GetCount() > 0)
	{
		m_resolutionsel.DeleteString(0);
	}
	
	while(m_encodetypesel.GetCount() > 0)
	{
		m_encodetypesel.DeleteString(0);
	}

	BOOL bRet = FALSE;
	DWORD retlen = 0;
	bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_DEVICECFG, 0,
								&m_sysCFG, sizeof(DHDEV_SYSTEM_ATTR_CFG), &retlen, CONFIG_WAITTIME);
	if (!bRet || retlen != sizeof(DHDEV_SYSTEM_ATTR_CFG)) 
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}

	bRet = CLIENT_GetDevConfig(m_pDev->LoginID, DH_DEV_CHANNELCFG, -1/*all channel*/,
								&m_chnCFG, 16*sizeof(DHDEV_CHANNEL_CFG), &retlen, CONFIG_WAITTIME);
	if (!bRet || retlen != m_pDev->Info.nChanNum*sizeof(DHDEV_CHANNEL_CFG))
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		m_bInited = TRUE;
	}

	//compute resolution and encode type name list
	char *rsName[8] = {"D1", "HD1", "BCIF", "CIF", "QCIF", "VGA", "QVGA", "SVCD"};
	char *ecName[8] = {"DIVX_MPEG4", "MS_MPEG4", "MPEG2", "MPEG1", "H263", "MJPG", "FCC_MPEG4", "H264"};

	BYTE rsIdx = 0;
	for (int mi = 0; mi < 8; mi++)
	{
		if (m_sysCFG.stDspEncodeCap.dwImageSizeMask & (0x01<<mi))
		{
			m_resolutionsel.InsertString(rsIdx, rsName[mi]);
			rsIdx++;
		}
	}

	BYTE ecIdx = 0;
	for (int ei = 0; ei < 8; ei++)
	{
		if (m_sysCFG.stDspEncodeCap.dwEncodeModeMask & (0x01<<ei))
		{
			m_encodetypesel.InsertString(ecIdx, ecName[ei]);
			ecIdx++;
		}
	}

	//show config information
	if (m_channelsel.GetCount() > 0)
	{
		m_channelsel.SetCurSel(0);
		OnSelchangeComboChanno();
	}
}

void CConfigChannel::SetDevice(DeviceNode *pDev)
{
	m_pDev = pDev;
}

void CConfigChannel::OnSelchangeComboChanno() 
{
	if (!m_bInited)
	{
		return;
	}

	int strmIdx,osdIdx;
	//store current config
	if (m_chnIdx < 0)
	{
		goto skipsave;
	}

	strmIdx = m_streamsel.GetCurSel();
	if (strmIdx < 0)
	{
		return;
	}

	osdIdx = m_osdsel.GetCurSel();
	if (osdIdx < 0)
	{
		return;
	}

	GetDlgItemText(IDC_EDIT_CHANNAME, m_chnCFG[m_chnIdx].szChannelName, DH_CHAN_NAME_LEN+1);
	BYTE rsOIdx, ecOIdx;
	GetOrigIndex(&rsOIdx, m_resolutionsel.GetCurSel(), m_sysCFG.stDspEncodeCap.dwImageSizeMask);
	GetOrigIndex(&ecOIdx, m_encodetypesel.GetCurSel(), m_sysCFG.stDspEncodeCap.dwEncodeModeMask);	
	if (strmIdx < 3) //main stream - for record
	{
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byVideoEnable= m_videochk.GetCheck();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byAudioEnable= m_audiochk.GetCheck();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byBitRateControl= m_bitratectrlsel.GetCurSel();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byFramesPerSec= m_framesel.GetCurSel();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byEncodeMode= ecOIdx;
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byImageSize= rsOIdx;
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byImageQlty= m_imgqualitysel.GetCurSel()+1;
	}
	else if (strmIdx < 6) //assistant - for net-transfor
	{
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byVideoEnable= m_videochk.GetCheck();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byAudioEnable= m_audiochk.GetCheck();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byBitRateControl= m_bitratectrlsel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byFramesPerSec= m_framesel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byEncodeMode= ecOIdx;
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byImageSize= rsOIdx;
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byImageQlty= m_imgqualitysel.GetCurSel()+1;
	}
	m_chnCFG[m_chnIdx].stColorCfg[0].byBrightness = GetDlgItemInt(IDC_EDIT_BRIGHTNESS);
	m_chnCFG[m_chnIdx].stColorCfg[0].bySaturation = GetDlgItemInt(IDC_EDIT_SATURATION);
	m_chnCFG[m_chnIdx].stColorCfg[0].byContrast = GetDlgItemInt(IDC_EDIT_CONTRAST);
	m_chnCFG[m_chnIdx].stColorCfg[0].byHue = GetDlgItemInt(IDC_EDIT_HUE);
	m_chnCFG[m_chnIdx].stColorCfg[0].byGainEn = m_gainchk.GetCheck();
	m_chnCFG[m_chnIdx].stColorCfg[0].byGain = GetDlgItemInt(IDC_EDIT_GAIN);

	//show new config
skipsave:
	int chnIdx = m_channelsel.GetCurSel();
	if (chnIdx < 0)
	{
		return;
	}
	m_chnIdx = chnIdx;

	char tmpName[DH_CHAN_NAME_LEN+1] = {0};
	memcpy(tmpName, m_chnCFG[chnIdx].szChannelName, DH_CHAN_NAME_LEN);
	SetDlgItemText(IDC_EDIT_CHANNAME, tmpName);
	
	SetDlgItemInt(IDC_EDIT_BRIGHTNESS, m_chnCFG[chnIdx].stColorCfg[0].byBrightness);
	SetDlgItemInt(IDC_EDIT_SATURATION, m_chnCFG[chnIdx].stColorCfg[0].bySaturation);
	SetDlgItemInt(IDC_EDIT_CONTRAST, m_chnCFG[chnIdx].stColorCfg[0].byContrast);
	SetDlgItemInt(IDC_EDIT_HUE, m_chnCFG[chnIdx].stColorCfg[0].byHue);
	SetDlgItemInt(IDC_EDIT_GAIN, m_chnCFG[chnIdx].stColorCfg[0].byGain);
	m_gainchk.SetCheck(m_chnCFG[chnIdx].stColorCfg[0].byGainEn?1:0);

	//refresh every area
	if (m_streamsel.GetCount())
	{
		m_streamsel.SetCurSel(0);
	}
	m_strmIdx = -1; //prevent from unwanted saving
	OnSelchangeComboStream();
	
	if (m_osdsel.GetCount() > 0)
	{
		m_osdsel.SetCurSel(0);
	}
	m_osdIdx = -1; //prevent from unwanted saving
	OnSelchangeComboOsd();
}

void CConfigChannel::OnSelchangeComboStream() 
{
	if (!m_bInited || m_chnIdx < 0)
	{
		return;
	}

	if (m_strmIdx < 0)
	{
		goto skipsave;
	}
	//store current config
	BYTE rsOIdx, ecOIdx;
	GetOrigIndex(&rsOIdx, m_resolutionsel.GetCurSel(), m_sysCFG.stDspEncodeCap.dwImageSizeMask);
	GetOrigIndex(&ecOIdx, m_encodetypesel.GetCurSel(), m_sysCFG.stDspEncodeCap.dwEncodeModeMask);
	if (m_strmIdx < 3 && m_strmIdx >= 0) //main stream - for record
	{
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byVideoEnable= m_videochk.GetCheck();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byAudioEnable= m_audiochk.GetCheck();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byBitRateControl= m_bitratectrlsel.GetCurSel();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byFramesPerSec= m_framesel.GetCurSel();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byEncodeMode= ecOIdx;
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byImageSize= rsOIdx;
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[m_strmIdx].byImageQlty= m_imgqualitysel.GetCurSel()+1;
	}
	else if (m_strmIdx < 6) //assistant - for net-transfor
	{
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byVideoEnable= m_videochk.GetCheck();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byAudioEnable= m_audiochk.GetCheck();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byBitRateControl= m_bitratectrlsel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byFramesPerSec= m_framesel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byEncodeMode= ecOIdx;
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byImageSize= rsOIdx;
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[m_strmIdx-3].byImageQlty= m_imgqualitysel.GetCurSel()+1;
	}

	//show new config
skipsave:

	int strmIdx = m_streamsel.GetCurSel();
	if (strmIdx < 0)
	{
		return;
	}
	m_strmIdx = strmIdx;

	//get masked resolution and encodetype indexs
	BYTE rsIdx, ecIdx;
	GetMaskedIndex(&rsIdx, m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byImageSize, m_sysCFG.stDspEncodeCap.dwImageSizeMask);	
	GetMaskedIndex(&ecIdx, m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byEncodeMode ,m_sysCFG.stDspEncodeCap.dwEncodeModeMask);

	if (strmIdx < 3) //main stream - for record
	{
		m_videochk.SetCheck(m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byVideoEnable);
		m_audiochk.SetCheck(m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byAudioEnable);
		m_bitratectrlsel.SetCurSel(m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byBitRateControl);
		m_framesel.SetCurSel(m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byFramesPerSec);
		m_encodetypesel.SetCurSel(ecIdx);
		m_resolutionsel.SetCurSel(rsIdx);
		m_imgqualitysel.SetCurSel(m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byImageQlty-1);
	}
	else if (strmIdx < 6) //assistant - for net-transfor
	{
		m_videochk.SetCheck(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byVideoEnable);
		m_audiochk.SetCheck(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byAudioEnable);
		m_bitratectrlsel.SetCurSel(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byBitRateControl);
		m_framesel.SetCurSel(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byFramesPerSec);
		m_encodetypesel.SetCurSel(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byEncodeMode);
		m_resolutionsel.SetCurSel(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byImageSize);
		m_imgqualitysel.SetCurSel(m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byImageQlty-1);
	}
}

void CConfigChannel::OnSelchangeComboOsd() 
{
	if (!m_bInited || m_chnIdx < 0)
	{
		return;
	}

	//store current osd config
	if (m_osdIdx >= 0)
	{
		//later
	}

	//show new one
	int osdIdx = m_osdsel.GetCurSel();
	if (osdIdx < 0)
	{
		return;
	}
	m_osdIdx = osdIdx;

	switch(osdIdx) 
	{
	case 0: // channel name osd
		m_osdshowchk.SetCheck(m_chnCFG[m_chnIdx].stChannelOSD.bShow);
		SetDlgItemInt(IDC_EDIT_FORE_RED, m_chnCFG[m_chnIdx].stChannelOSD.rgbaFrontground & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_GREEN, (m_chnCFG[m_chnIdx].stChannelOSD.rgbaFrontground>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_BLUE, (m_chnCFG[m_chnIdx].stChannelOSD.rgbaFrontground>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_TRANS, (m_chnCFG[m_chnIdx].stChannelOSD.rgbaFrontground>>24) & 0xFF);

		SetDlgItemInt(IDC_EDIT_BACK_RED, m_chnCFG[m_chnIdx].stChannelOSD.rgbaBackground & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_GREEN, (m_chnCFG[m_chnIdx].stChannelOSD.rgbaBackground>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_BLUE, (m_chnCFG[m_chnIdx].stChannelOSD.rgbaBackground>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_TRANS, (m_chnCFG[m_chnIdx].stChannelOSD.rgbaBackground>>24) & 0xFF);

		SetDlgItemInt(IDC_EDIT_RECT_LFET, m_chnCFG[m_chnIdx].stChannelOSD.rcRect.left & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_TOP, (m_chnCFG[m_chnIdx].stChannelOSD.rcRect.top>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_RIGHT, (m_chnCFG[m_chnIdx].stChannelOSD.rcRect.right>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_BOTTOM, (m_chnCFG[m_chnIdx].stChannelOSD.rcRect.bottom>>24) & 0xFF);

		GetDlgItem(IDC_STATIC_BLINDTYPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_BLINDTYPE)->ShowWindow(SW_HIDE);
		break;
	case 1:
		m_osdshowchk.SetCheck(m_chnCFG[m_chnIdx].stChannelOSD.bShow);
		SetDlgItemInt(IDC_EDIT_FORE_RED, m_chnCFG[m_chnIdx].stTimeOSD.rgbaFrontground & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_GREEN, (m_chnCFG[m_chnIdx].stTimeOSD.rgbaFrontground>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_BLUE, (m_chnCFG[m_chnIdx].stTimeOSD.rgbaFrontground>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_TRANS, (m_chnCFG[m_chnIdx].stTimeOSD.rgbaFrontground>>24) & 0xFF);
		
		SetDlgItemInt(IDC_EDIT_BACK_RED, m_chnCFG[m_chnIdx].stTimeOSD.rgbaBackground & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_GREEN, (m_chnCFG[m_chnIdx].stTimeOSD.rgbaBackground>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_BLUE, (m_chnCFG[m_chnIdx].stTimeOSD.rgbaBackground>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_TRANS, (m_chnCFG[m_chnIdx].stTimeOSD.rgbaBackground>>24) & 0xFF);
		
		SetDlgItemInt(IDC_EDIT_RECT_LFET, m_chnCFG[m_chnIdx].stTimeOSD.rcRect.left & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_TOP, (m_chnCFG[m_chnIdx].stTimeOSD.rcRect.top>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_RIGHT, (m_chnCFG[m_chnIdx].stTimeOSD.rcRect.right>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_BOTTOM, (m_chnCFG[m_chnIdx].stTimeOSD.rcRect.bottom>>24) & 0xFF);

		GetDlgItem(IDC_STATIC_BLINDTYPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_COMBO_BLINDTYPE)->ShowWindow(SW_HIDE);
		break;
	case 2:
		m_osdshowchk.SetCheck(m_chnCFG[m_chnIdx].stChannelOSD.bShow);
		SetDlgItemInt(IDC_EDIT_FORE_RED, m_chnCFG[m_chnIdx].stBlindCover[0].rgbaFrontground & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_GREEN, (m_chnCFG[m_chnIdx].stBlindCover[0].rgbaFrontground>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_BLUE, (m_chnCFG[m_chnIdx].stBlindCover[0].rgbaFrontground>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_FORE_TRANS, (m_chnCFG[m_chnIdx].stBlindCover[0].rgbaFrontground>>24) & 0xFF);
		
		SetDlgItemInt(IDC_EDIT_BACK_RED, m_chnCFG[m_chnIdx].stBlindCover[0].rgbaBackground & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_GREEN, (m_chnCFG[m_chnIdx].stBlindCover[0].rgbaBackground>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_BLUE, (m_chnCFG[m_chnIdx].stBlindCover[0].rgbaBackground>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_BACK_TRANS, (m_chnCFG[m_chnIdx].stBlindCover[0].rgbaBackground>>24) & 0xFF);
		
		SetDlgItemInt(IDC_EDIT_RECT_LFET, m_chnCFG[m_chnIdx].stBlindCover[0].rcRect.left & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_TOP, (m_chnCFG[m_chnIdx].stBlindCover[0].rcRect.top>>8) & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_RIGHT, (m_chnCFG[m_chnIdx].stBlindCover[0].rcRect.right>>16) & 0xFF);
		SetDlgItemInt(IDC_EDIT_RECT_BOTTOM, (m_chnCFG[m_chnIdx].stBlindCover[0].rcRect.bottom>>24) & 0xFF);		

		m_blindtypesel.SetCurSel(m_chnCFG[m_chnIdx].byBlindEnable);

		GetDlgItem(IDC_STATIC_BLINDTYPE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_COMBO_BLINDTYPE)->ShowWindow(SW_SHOW);
		break;
	default:
		break;
	}
	
}

///////////////////
/************************************************************************
 *	Í¼ÏñÍ¨µÀ
************************************************************************/
/*																		*/

void CConfigChannel::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		GetConfig();
	}
}

void CConfigChannel::OnUndoAll() 
{
	m_bInited = FALSE;
	m_resolutionsel.Clear();
	m_encodetypesel.Clear();
	m_chnIdx = -1;
	m_strmIdx = -1;
	m_osdIdx = -1;
	GetConfig();
}

void CConfigChannel::OnApply() 
{
	if (!m_bInited || m_chnIdx < 0)
	{
		return;
	}

	int strmIdx = m_streamsel.GetCurSel();
	if (strmIdx < 0)
	{
		return;
	}

	int osdIdx = m_osdsel.GetCurSel();
	if (osdIdx < 0)
	{
		return;
	}

	GetDlgItemText(IDC_EDIT_CHANNAME, m_chnCFG[m_chnIdx].szChannelName, DH_CHAN_NAME_LEN+1);
	BYTE rsOIdx, ecOIdx;
	GetOrigIndex(&rsOIdx, m_resolutionsel.GetCurSel(), m_sysCFG.stDspEncodeCap.dwImageSizeMask);
	GetOrigIndex(&ecOIdx, m_encodetypesel.GetCurSel(), m_sysCFG.stDspEncodeCap.dwEncodeModeMask);
	if (strmIdx < 3) //main stream - for record
	{
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byVideoEnable= m_videochk.GetCheck();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byAudioEnable= m_audiochk.GetCheck();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byBitRateControl= m_bitratectrlsel.GetCurSel();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byFramesPerSec= m_framesel.GetCurSel();
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byEncodeMode= ecOIdx;
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byImageSize= rsOIdx;
		m_chnCFG[m_chnIdx].stMainVideoEncOpt[strmIdx].byImageQlty= m_imgqualitysel.GetCurSel()+1;
	}
	else if (strmIdx < 6) //assistant - for net-transfor
	{
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byVideoEnable= m_videochk.GetCheck();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byAudioEnable= m_audiochk.GetCheck();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byBitRateControl= m_bitratectrlsel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byFramesPerSec= m_framesel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byEncodeMode= m_encodetypesel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byImageSize= m_resolutionsel.GetCurSel();
		m_chnCFG[m_chnIdx].stAssiVideoEncOpt[strmIdx-3].byImageQlty= m_imgqualitysel.GetCurSel()+1;
	}
	m_chnCFG[m_chnIdx].stColorCfg[0].byBrightness = GetDlgItemInt(IDC_EDIT_BRIGHTNESS);
	m_chnCFG[m_chnIdx].stColorCfg[0].bySaturation = GetDlgItemInt(IDC_EDIT_SATURATION);
	m_chnCFG[m_chnIdx].stColorCfg[0].byContrast = GetDlgItemInt(IDC_EDIT_CONTRAST);
	m_chnCFG[m_chnIdx].stColorCfg[0].byHue = GetDlgItemInt(IDC_EDIT_HUE);
	m_chnCFG[m_chnIdx].stColorCfg[0].byGainEn = m_gainchk.GetCheck();
	m_chnCFG[m_chnIdx].stColorCfg[0].byGain = GetDlgItemInt(IDC_EDIT_GAIN);

	BOOL bRet = CLIENT_SetDevConfig(m_pDev->LoginID, DH_DEV_CHANNELCFG, -1/*all channel*/,
								&m_chnCFG, 16*sizeof(DHDEV_CHANNEL_CFG), CONFIG_WAITTIME);
	if (!bRet)
	{
		((CNetSDKDemoDlg*)AfxGetMainWnd())->LastError();
		return;
	}
	else
	{
		MessageBox(ConvertString(MSG_CONFIG_SUCCESS), "OK");
	}
}

void CConfigChannel::OnDropdownComboChanno() 
{
	// TODO: Add your control notification handler code here
}

void CConfigChannel::OnChangeEditBrightness() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	int nSize = GetDlgItemInt(IDC_EDIT_BRIGHTNESS);
	if (nSize < 0 || nSize >100)
	{
		SetDlgItemText(IDC_EDIT_BRIGHTNESS, "50");
	}
}

void CConfigChannel::OnChangeEditContrast() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	int nSize = GetDlgItemInt(IDC_EDIT_CONTRAST);
	if (nSize < 0 || nSize >100)
	{
		SetDlgItemText(IDC_EDIT_CONTRAST, "50");
	}
}

void CConfigChannel::OnChangeEditSaturation() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	int nSize = GetDlgItemInt(IDC_EDIT_SATURATION);
	if (nSize < 0 || nSize >100)
	{
		SetDlgItemText(IDC_EDIT_SATURATION, "50");
	}
}

void CConfigChannel::OnChangeEditHue() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	int nSize = GetDlgItemInt(IDC_EDIT_HUE);
	if (nSize < 0 || nSize >100)
	{
		SetDlgItemText(IDC_EDIT_HUE, "50");
	}
}

void CConfigChannel::OnChangeEditGain() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	int nSize = GetDlgItemInt(IDC_EDIT_GAIN);
	if (nSize < 0 || nSize >100)
	{
		SetDlgItemText(IDC_EDIT_GAIN, "50");
	}
}
