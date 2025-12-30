// ChannelConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ChannelConfig.h"
#include "SetHide.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChannelConfig dialog
#include "TimingRecord.h"

CChannelConfig::CChannelConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CChannelConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChannelConfig)
	m_rec_ratectrl = 0;
	m_net_ratectrl = 0;
	m_channelname = _T("");
	m_bHide = FALSE;
	m_bHideAlarm = FALSE;
	m_bMotion = FALSE;
	m_bOSD = FALSE;
	m_bChannelName = FALSE;
	m_bTimingRecord = FALSE;
	m_bVLostAlarm = FALSE;
	m_channelname_X = 0;
	m_channelname_Y = 0;
	m_osd_X = 0;
	m_osd_Y = 0;
	m_bOSDWeek = FALSE;
	//}}AFX_DATA_INIT
}


void CChannelConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChannelConfig)
	DDX_Control(pDX, IDC_OSD_TIMEFORMATSEL, m_osd_timeformatsel);
	DDX_Control(pDX, IDC_OSD_DISTYPESEL, m_osd_distypesel);
	DDX_Control(pDX, IDC_PRERECORDTIME, m_prerecordsel);
	DDX_Control(pDX, IDC_RECORDDELAY, m_recorddelaysel);
	DDX_Control(pDX, IDC_FRAMERATESEL2, m_net_frameratesel);
	DDX_Control(pDX, IDC_FRAMERATESEL, m_rec_frameratesel);
	DDX_Control(pDX, IDC_CHANNELSEL, m_channelsel);
	DDX_Control(pDX, IDC_COPYCHANNELSEL, m_copychannelsel);
	DDX_Control(pDX, IDC_RATECTRLSEL2, m_net_ratectrlsel);
	DDX_Control(pDX, IDC_RATECTRLSEL, m_rec_ratectrlsel);
	DDX_Control(pDX, IDC_RATETYPESEL2, m_net_ratetypesel);
	DDX_Control(pDX, IDC_RATETYPESEL, m_rec_ratetypesel);
	DDX_Control(pDX, IDC_STREAMTYPESEL2, m_net_streamtypesel);
	DDX_Control(pDX, IDC_STREAMTYPESEL, m_rec_streamtypesel);
	DDX_Control(pDX, IDC_SIZESEL2, m_net_imgsizesel);
	DDX_Control(pDX, IDC_SIZESEL, m_rec_imgsizesel);
	DDX_Control(pDX, IDC_IMAGEQUALITY2, m_net_imgqualitysel);
	DDX_Control(pDX, IDC_IMAGEQUALITY, m_rec_imgqualitysel);
	DDX_Text(pDX, IDC_RATECTRL, m_rec_ratectrl);
	DDX_Text(pDX, IDC_RATECTRL2, m_net_ratectrl);
	DDX_Text(pDX, IDC_CHANNELNAME, m_channelname);
	DDX_Check(pDX, IDC_HIDE_CHECK, m_bHide);
	DDX_Check(pDX, IDC_HIDEALARM_CHECK, m_bHideAlarm);
	DDX_Check(pDX, IDC_MOTION_CHECK, m_bMotion);
	DDX_Check(pDX, IDC_OSD_CHECK, m_bOSD);
	DDX_Check(pDX, IDC_CHANNELNAME_CHECK, m_bChannelName);
	DDX_Check(pDX, IDC_TIMINGRECORD_CHECK, m_bTimingRecord);
	DDX_Check(pDX, IDC_VLOSTALARM_CHECK, m_bVLostAlarm);
	DDX_Text(pDX, IDC_CHANNELNAME_X, m_channelname_X);
	DDX_Text(pDX, IDC_CHANNELNAME_Y, m_channelname_Y);
	DDX_Text(pDX, IDC_OSD_X, m_osd_X);
	DDX_Text(pDX, IDC_OSD_Y, m_osd_Y);
	DDX_Check(pDX, IDC_OSDWEEK_CHECK, m_bOSDWeek);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChannelConfig, CDialog)
	//{{AFX_MSG_MAP(CChannelConfig)
	ON_CBN_SELCHANGE(IDC_CHANNELSEL, OnSelchangeChannelsel)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_SETTIMINGRECORD, OnSettimingrecord)
	ON_BN_CLICKED(IDC_TIMINGRECORD_CHECK, OnTimingrecordCheck)
	ON_BN_CLICKED(IDC_HIDE_CHECK, OnHideCheck)
	ON_BN_CLICKED(IDC_MOTION_CHECK, OnMotionCheck)
	ON_BN_CLICKED(IDC_HIDEALARM_CHECK, OnHidealarmCheck)
	ON_BN_CLICKED(IDC_VLOSTALARM_CHECK, OnVlostalarmCheck)
	ON_BN_CLICKED(IDC_OSD_CHECK, OnOsdCheck)
	ON_BN_CLICKED(IDC_CHANNELNAME_CHECK, OnChannelnameCheck)
	ON_BN_CLICKED(IDC_SETHIDE, OnSethide)
	ON_BN_CLICKED(IDC_RESTORE, OnRestore)
	ON_CBN_SELCHANGE(IDC_RATECTRLSEL, OnSelchangeRatectrlsel)
	ON_CBN_SELCHANGE(IDC_RATECTRLSEL2, OnSelchangeRatectrlsel2)
	ON_BN_CLICKED(IDC_SAVECHANNELNAME, OnSavechannelname)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChannelConfig message handlers

BOOL CChannelConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// TODO: Add extra initialization here
	CRect rc(0, 0, 0, 0);
	GetParent()->GetClientRect(&rc);
	((CTabCtrl*)GetParent())->AdjustRect(FALSE, &rc);
	MoveWindow(&rc);

	//初始化界面
	m_rec_imgqualitysel.InsertString(0, MSG_CHANNELCFG_PQWORST);
	m_rec_imgqualitysel.InsertString(1, MSG_CHANNELCFG_PQWORSE);
	m_rec_imgqualitysel.InsertString(2, MSG_CHANNELCFG_PQNORMAL);
	m_rec_imgqualitysel.InsertString(3, MSG_CHANNELCFG_PQGOOD);
	m_rec_imgqualitysel.InsertString(4, MSG_CHANNELCFG_PQBETTER);
	m_rec_imgqualitysel.InsertString(5, MSG_CHANNELCFG_PQBEST);

	m_net_imgqualitysel.InsertString(0, MSG_CHANNELCFG_PQWORST);
	m_net_imgqualitysel.InsertString(1, MSG_CHANNELCFG_PQWORSE);
	m_net_imgqualitysel.InsertString(2, MSG_CHANNELCFG_PQNORMAL);
	m_net_imgqualitysel.InsertString(3, MSG_CHANNELCFG_PQGOOD);
	m_net_imgqualitysel.InsertString(4, MSG_CHANNELCFG_PQBETTER);
	m_net_imgqualitysel.InsertString(5, MSG_CHANNELCFG_PQBEST);

	m_rec_streamtypesel.InsertString(0, MSG_CHANNELCFG_VSTREAM);
	m_rec_streamtypesel.InsertString(1, MSG_CHANNELCFG_CSTREAM);

	m_net_streamtypesel.InsertString(0, MSG_CHANNELCFG_VSTREAM);
	m_net_streamtypesel.InsertString(1, MSG_CHANNELCFG_CSTREAM);

	m_rec_ratetypesel.InsertString(0, MSG_CHANNELCFG_LMRATE);
	m_rec_ratetypesel.InsertString(1, MSG_CHANNELCFG_VRRATE);

	m_net_ratetypesel.InsertString(0, MSG_CHANNELCFG_LMRATE);
	m_net_ratetypesel.InsertString(1, MSG_CHANNELCFG_VRRATE);
	

	CString sTemp;
	char cTemp[100];
	m_nChanSel = 0;
	m_copychannelsel.ResetContent();
	m_channelsel.ResetContent();
	sTemp.Format(ConvertString(_T(NAME_CHANNELCFG_ALLCHANNEL)));
	m_copychannelsel.AddString(sTemp);

	for(int i=0; i < m_Device->Info.byChanNum; i++)
	{
		sTemp.Format("%d", i + 1);
		sTemp = ConvertString(NAME_CHANNELCFG_CHANNEL) + sTemp;
		m_channelsel.AddString(sTemp);
		m_copychannelsel.AddString(sTemp);
		bSetChannelPara[i] = FALSE;
		bGetChannelPara[i] = FALSE;
		bRemoteRecord[i] = FALSE;
		if(GetChanPara(i))
		{
			bGetChannelPara[i] = TRUE;
		}
		else
		{
			sTemp.Format("%d",i+1);
			sTemp = MSG_CHANNELCFG_GETCHANNELCFGFAILED + sTemp;
			AfxMessageBox(sTemp);
		}
	}
	char buf[16*DH_CHAN_NAME_LEN];
	int chancount;	
	int bRet = CLIENT_QueryChannelName(m_Device->LoginID, (char *)buf, 512, &chancount);
	if (bRet) 
	{
		for (int i = 0; i < 16; i++)
		{
			strcpy((char *)m_piccfg[i].sChanName, buf+ 32*i);
		}
	}
	else
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
	}
	if (m_channelsel.GetCount() > 0)
	{
		m_channelsel.SetCurSel(m_nChanSel);
	}
	m_copychannelsel.SetCurSel(m_nChanSel);
//	m_nChanNum = m_nChanSel+m_lStartChannel;
	if(bGetChannelPara[m_nChanSel])
	{
		//录像参数
		if((m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate >> 31) && 0x01)
		{
			GetDlgItem(IDC_RATECTRL)->EnableWindow(TRUE);
			m_rec_ratectrlsel.SetCurSel(22);		//自定义码流
			m_rec_ratectrl = m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate & 0x7fffffff;
		}
		else
		{
			m_rec_ratectrlsel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate-2);			
			m_rec_ratectrl = 0;
			GetDlgItem(IDC_RATECTRL)->EnableWindow(FALSE);
		}
		
		m_rec_imgsizesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byResolution);
		m_rec_streamtypesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byStreamType);
		m_rec_imgqualitysel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byPicQuality - 1);
		m_rec_ratetypesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byBitrateType);
		m_rec_frameratesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.dwVideoFrameRate);		
//2005-07-08 网传参数
		if ((m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate >> 31) && 0x01)
		{
			GetDlgItem(IDC_RATECTRL2)->EnableWindow(TRUE);
			m_net_ratectrlsel.SetCurSel(22);		//自定义码流
			m_net_ratectrl = m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate & 0x7fffffff;
		}
		else
		{
			m_net_ratectrlsel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate-2);			
			m_net_ratectrl = 0;
			GetDlgItem(IDC_RATECTRL2)->EnableWindow(FALSE);
		}
		
		m_net_imgsizesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byResolution);
		m_net_streamtypesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byStreamType);
		m_net_imgqualitysel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byPicQuality - 1);
		m_net_ratetypesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byBitrateType);
		m_net_frameratesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.dwVideoFrameRate);		
//end		
		m_bTimingRecord = m_record[m_nChanSel].dwRecord;
//		GetDlgItem(IDC_SETTIMINGRECORD)->EnableWindow(m_bTimingRecord);
		m_recorddelaysel.SetCurSel(m_record[m_nChanSel].dwRecordTime);
		m_prerecordsel.SetCurSel(m_record[m_nChanSel].dwPreRecordTime);
		ZeroMemory(cTemp, 100);
		memcpy(cTemp, m_piccfg[m_nChanSel].sChanName, NAME_LEN);
		m_channelname.Format("%s",cTemp);
		m_bHide = m_piccfg[m_nChanSel].dwEnableHide;
		GetDlgItem(IDC_SETHIDE)->EnableWindow(m_bHide);
		m_bMotion = m_piccfg[m_nChanSel].struMotion.byEnableHandleMotion;
		GetDlgItem(IDC_SETMOTION)->EnableWindow(m_bMotion);
		if(m_piccfg[m_nChanSel].struHideAlarm.dwEnableHideAlarm > 0)
		{
			m_bHideAlarm = TRUE;
		}
		else
		{
			m_bHideAlarm = FALSE;
		}

		GetDlgItem(IDC_SETHIDEALARM)->EnableWindow(m_bHideAlarm);
		m_bChannelName = m_piccfg[m_nChanSel].dwShowChanName;
		m_channelname_X = m_piccfg[m_nChanSel].wShowNameTopLeftX;
		m_channelname_Y = m_piccfg[m_nChanSel].wShowNameTopLeftY;
		m_bOSD = m_piccfg[m_nChanSel].dwShowOsd;
		m_osd_X = m_piccfg[m_nChanSel].wOSDTopLeftX;
		m_osd_Y = m_piccfg[m_nChanSel].wOSDTopLeftY;
		m_bOSDWeek = m_piccfg[m_nChanSel].byDispWeek;
		m_osd_distypesel.SetCurSel(m_piccfg[m_nChanSel].byOSDAttrib-1);
		m_osd_timeformatsel.SetCurSel(m_piccfg[m_nChanSel].byOSDType);
		EnableChanname(m_bChannelName);
		EnableOSD(m_bOSD);
		m_bVLostAlarm = m_piccfg[m_nChanSel].struVILost.byEnableHandleVILost;
		GetDlgItem(IDC_SETVLOSTALARM)->EnableWindow(m_bVLostAlarm);
		GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
		GetDlgItem(IDC_RESTORE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
		GetDlgItem(IDC_RESTORE)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CChannelConfig::GetChanPara(int nIndex)
{ 
	DWORD dwReturned;	
	if(!CLIENT_GetDevConfig(m_Device->LoginID, DEV_GET_PICCFG, nIndex ,&m_piccfg[nIndex], sizeof(NET_DEV_PICCFG), &dwReturned), CONFIG_WAITTIME)
	{
			return FALSE;
	}
	if(!CLIENT_GetDevConfig(m_Device->LoginID,DEV_GET_COMPRESSCFG, nIndex,&m_compressioncfg[nIndex], sizeof(NET_DEV_COMPRESSIONCFG), &dwReturned, CONFIG_WAITTIME))
	{
			return FALSE;
	}
/*
	if(!CLIENT_GetDevConfig(m_Device->LoginID,NET_DVR_GET_RECORDCFG, nIndex + 1,&m_record[nIndex], sizeof(NET_DVR_RECORD), &dwReturned, CONFIG_WAITTIME))	
	{
		CString sTemp;
		sTemp.Format("%d : ERROR: NET_DVR_GET_RECORDCFG = %d \n", m_nChanNum, NET_DVR_GetLastError());
		return FALSE;
	}
	*/

	return TRUE;
}

void CChannelConfig::EnableChanname(BOOL bEnable) 
{
	GetDlgItem(IDC_CHANNELNAME_X)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHANNELNAME_Y)->EnableWindow(bEnable);
}

void CChannelConfig::EnableOSD(BOOL bEnable) 
{
	GetDlgItem(IDC_OSD_X)->EnableWindow(bEnable);
	GetDlgItem(IDC_OSD_Y)->EnableWindow(bEnable);
	GetDlgItem(IDC_OSD_DISTYPESEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_OSD_TIMEFORMATSEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_OSDWEEK_CHECK)->EnableWindow(bEnable);
}

void CChannelConfig::OnSelchangeChannelsel() 
{
	m_nChanSel = m_channelsel.GetCurSel();
	char cTemp[100];
	if(!bGetChannelPara[m_nChanSel])
	{		
		if(GetChanPara(m_nChanSel))
		{
			char buf[512];
			int chancount;	
			int bRet = CLIENT_QueryChannelName(m_Device->LoginID, (char *)buf, 512, &chancount);
			if (bRet) 
			{
				for (int i = 0; i < 16; i++)
				{
					strcpy((char *)m_piccfg[i].sChanName, buf+ 32*i);
				}
			}
			else
			{
				((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
			}
			bGetChannelPara[m_nChanSel] = TRUE;
		}
		else
		{
			GetDlgItem(IDC_APPLY)->EnableWindow(FALSE);
			GetDlgItem(IDC_RESTORE)->EnableWindow(FALSE);
			CString sTemp;
			sTemp.Format("%d",m_nChanSel + 1);
			sTemp = MSG_CHANNELCFG_GETCHANNELCFGFAILED + sTemp;
			AfxMessageBox(sTemp);
			return;
		}
	}
	if((m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate >> 31) && 0x01)
	{
		GetDlgItem(IDC_RATECTRL)->EnableWindow(TRUE);
		m_rec_ratectrlsel.SetCurSel(22);		//自定义码流
		m_rec_ratectrl = m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate & 0x7fffffff;
	}
	else
	{
		m_rec_ratectrlsel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate-2);			
		m_rec_ratectrl = 0;
		GetDlgItem(IDC_RATECTRL)->EnableWindow(FALSE);
	}	
	m_rec_imgsizesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byResolution);
	m_rec_streamtypesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byStreamType);
	m_rec_imgqualitysel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byPicQuality - 1);
	m_rec_ratetypesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.byBitrateType);
	m_rec_frameratesel.SetCurSel(m_compressioncfg[m_nChanSel].struRecordPara.dwVideoFrameRate);		
			
	//双码流中网传参数
	if ((m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate >> 31) && 0x01)
	{
		GetDlgItem(IDC_RATECTRL2)->EnableWindow(TRUE);
		m_net_ratectrlsel.SetCurSel(22);		//自定义码流
		m_net_ratectrl = m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate & 0x7fffffff;
	}
	else
	{
		m_net_ratectrlsel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate-2);			
		m_net_ratectrl = 0;
		GetDlgItem(IDC_RATECTRL2)->EnableWindow(FALSE);
	}
	
	m_net_imgsizesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byResolution-1);
	m_net_streamtypesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byStreamType);
	m_net_imgqualitysel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byPicQuality - 1);
	m_net_ratetypesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.byBitrateType);
	m_net_frameratesel.SetCurSel(m_compressioncfg[m_nChanSel].struNetPara.dwVideoFrameRate);		
	
	m_bTimingRecord = m_record[m_nChanSel].dwRecord;
//	GetDlgItem(IDC_SETTIMINGRECORD)->EnableWindow(m_bTimingRecord);
	m_recorddelaysel.SetCurSel(m_record[m_nChanSel].dwRecordTime);
	m_prerecordsel.SetCurSel(m_record[m_nChanSel].dwPreRecordTime);
	ZeroMemory(cTemp, 100);
	memcpy(cTemp, m_piccfg[m_nChanSel].sChanName, NAME_LEN);
	m_channelname.Format("%s",cTemp);
	m_bHide = m_piccfg[m_nChanSel].dwEnableHide;
	GetDlgItem(IDC_SETHIDE)->EnableWindow(m_bHide);
	m_bMotion = m_piccfg[m_nChanSel].struMotion.byEnableHandleMotion;
	GetDlgItem(IDC_SETMOTION)->EnableWindow(m_bMotion);
	if(m_piccfg[m_nChanSel].struHideAlarm.dwEnableHideAlarm > 0)
	{
		m_bHideAlarm = TRUE;
	}
	else
	{
		m_bHideAlarm = FALSE;
	}
	
	GetDlgItem(IDC_SETHIDEALARM)->EnableWindow(m_bHideAlarm);
	m_bChannelName = m_piccfg[m_nChanSel].dwShowChanName;
	m_channelname_X = m_piccfg[m_nChanSel].wShowNameTopLeftX;
	m_channelname_Y = m_piccfg[m_nChanSel].wShowNameTopLeftY;
	m_bOSD = m_piccfg[m_nChanSel].dwShowOsd;
	m_osd_X = m_piccfg[m_nChanSel].wOSDTopLeftX;
	m_osd_Y = m_piccfg[m_nChanSel].wOSDTopLeftY;
	m_bOSDWeek = m_piccfg[m_nChanSel].byDispWeek;
	m_osd_distypesel.SetCurSel(m_piccfg[m_nChanSel].byOSDAttrib-1);
	m_osd_timeformatsel.SetCurSel(m_piccfg[m_nChanSel].byOSDType);
	EnableChanname(m_bChannelName);
	EnableOSD(m_bOSD);
	m_bVLostAlarm = m_piccfg[m_nChanSel].struVILost.byEnableHandleVILost;
	GetDlgItem(IDC_SETVLOSTALARM)->EnableWindow(m_bVLostAlarm);
	if(bRemoteRecord[m_nChanSel])
	{
		GetDlgItem(IDC_SETTIMINGRECORD)->SetWindowText(ConvertString(NAME_CHANNELCFG_STOPREC));		
	}
	else
	{
		GetDlgItem(IDC_SETTIMINGRECORD)->SetWindowText(ConvertString(NAME_CHANNELCFG_STARTREC));
	}
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_RESTORE)->EnableWindow(TRUE);
	
	UpdateData(FALSE);
}

void CChannelConfig::SetDevice (DeviceNode *nDev)
{
	m_Device = nDev;
}

void CChannelConfig::OnApply() 
{
	if(!UpdateData(TRUE))
	{
		return;
	}	
	int nTemp = 0;
	char ctemp[NAME_LEN];
	memset(ctemp,0,NAME_LEN);
	strcpy(ctemp,m_channelname.GetBuffer(0));
	memcpy(m_piccfg[m_nChanSel].sChanName,ctemp,NAME_LEN);
	m_compressioncfg[m_nChanSel].dwSize = sizeof(NET_DEV_COMPRESSIONCFG);
	m_compressioncfg[m_nChanSel].struRecordPara.byStreamType = m_rec_streamtypesel.GetCurSel();
	m_compressioncfg[m_nChanSel].struRecordPara.byPicQuality = m_rec_imgqualitysel.GetCurSel() + 1;
	if(m_rec_ratectrlsel.GetCurSel() == 22)	//自定义码流
	{
		nTemp = m_rec_ratectrl;		
		if(nTemp < 32*1024)
			nTemp = 32*1024;
		if(nTemp > 8192*1024)
			nTemp =  8192*1024;
		nTemp |= 0x80000000;
		m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate = nTemp;
	}
	else
	{
		m_compressioncfg[m_nChanSel].struRecordPara.dwVideoBitrate = m_rec_ratectrlsel.GetCurSel()+2;
	}
	m_compressioncfg[m_nChanSel].struRecordPara.byResolution = m_rec_imgsizesel.GetCurSel();
	m_compressioncfg[m_nChanSel].struRecordPara.dwVideoFrameRate = m_rec_frameratesel.GetCurSel();
	m_compressioncfg[m_nChanSel].struRecordPara.byBitrateType = m_rec_ratetypesel.GetCurSel();
	//2005-07-08 网传参数
	m_compressioncfg[m_nChanSel].struNetPara.byStreamType = m_net_streamtypesel.GetCurSel();
	m_compressioncfg[m_nChanSel].struNetPara.byPicQuality = m_net_imgqualitysel.GetCurSel() + 1;
	if (m_net_ratectrlsel.GetCurSel() == 22)	//自定义码流
	{
		nTemp = m_net_ratectrl;		
		if (nTemp < 32*1024)
			nTemp = 32*1024;
		if (nTemp > 8192*1024)
			nTemp =  8192*1024;
		nTemp |= 0x80000000;
		m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate = nTemp;
	}
	else
	{
		m_compressioncfg[m_nChanSel].struNetPara.dwVideoBitrate = m_net_ratectrlsel.GetCurSel()+2;
	}
	m_compressioncfg[m_nChanSel].struNetPara.byResolution = m_net_imgqualitysel.GetCurSel();
	m_compressioncfg[m_nChanSel].struNetPara.dwVideoFrameRate = m_net_frameratesel.GetCurSel();
	m_compressioncfg[m_nChanSel].struNetPara.byBitrateType = m_net_ratetypesel.GetCurSel();
	
	m_record[m_nChanSel].dwSize = sizeof(NET_DEV_RECORD);
	m_record[m_nChanSel].dwRecord = m_bTimingRecord;
	m_record[m_nChanSel].dwRecordTime = m_recorddelaysel.GetCurSel();
	m_record[m_nChanSel].dwPreRecordTime = m_prerecordsel.GetCurSel();
	
	m_piccfg[m_nChanSel].dwSize = sizeof(NET_DEV_PICCFG);
	m_piccfg[m_nChanSel].dwEnableHide = m_bHide;
	m_piccfg[m_nChanSel].struMotion.byEnableHandleMotion = m_bMotion;
	if(!m_bHideAlarm)
	{
		m_piccfg[m_nChanSel].struHideAlarm.dwEnableHideAlarm = m_bHideAlarm;
	}
	m_piccfg[m_nChanSel].dwShowChanName = m_bChannelName;
	m_piccfg[m_nChanSel].wShowNameTopLeftX = m_channelname_X;
	m_piccfg[m_nChanSel].wShowNameTopLeftY = m_channelname_Y;
	m_piccfg[m_nChanSel].dwShowOsd = m_bOSD;
	m_piccfg[m_nChanSel].wOSDTopLeftX = m_osd_X;
	m_piccfg[m_nChanSel].wOSDTopLeftY = m_osd_Y;
	m_piccfg[m_nChanSel].byDispWeek = m_bOSDWeek;
	m_piccfg[m_nChanSel].byOSDAttrib = m_osd_distypesel.GetCurSel()+1;
	m_piccfg[m_nChanSel].byOSDType = m_osd_timeformatsel.GetCurSel();
	m_piccfg[m_nChanSel].struVILost.byEnableHandleVILost = m_bVLostAlarm;
	
	bSetChannelPara[m_nChanSel] = TRUE;
	GetDlgItem(IDC_APPLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_RESTORE)->EnableWindow(TRUE);

	for(int i=0; i<m_Device->Info.byChanNum; i++)
	{
		int m_nChanNum = i;
		if(bSetChannelPara[i])
		{
			if(!CLIENT_SetDevConfig(m_Device->LoginID, DEV_SET_PICCFG, m_nChanNum,&(m_piccfg[i]), sizeof(NET_DEV_PICCFG), CONFIG_WAITTIME))
			{
				AfxMessageBox(ConvertString("channel config save failed!"));	
				return ;
			}
			if(!CLIENT_SetDevConfig(m_Device->LoginID,DEV_SET_COMPRESSCFG, m_nChanNum,&(m_compressioncfg[i]), sizeof(NET_DEV_COMPRESSIONCFG), CONFIG_WAITTIME))
			{
				AfxMessageBox(ConvertString("channel config save failed!"));	
				return ;
			}
			if(!CLIENT_SetDevConfig(m_Device->LoginID,DEV_SET_RECORDCFG, m_nChanNum,&(m_record[i]), sizeof(NET_DEV_RECORD), CONFIG_WAITTIME))	
			{
				AfxMessageBox(ConvertString("channel config save failed!"));	
				return ;
			}
			bSetChannelPara[i] = FALSE;
		}
	}
}

void CChannelConfig::OnSettimingrecord() 
{
	CTimingRecord dlg;
	
	dlg.SetRecordInfo(&(m_record[m_nChanSel]));
	if(dlg.DoModal() == IDOK)
		bSetChannelPara[m_nChanSel] = TRUE;
	
}

void CChannelConfig::OnTimingrecordCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	GetDlgItem(IDC_SETTIMINGRECORD)->EnableWindow(m_bTimingRecord);

}

void CChannelConfig::OnHideCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	GetDlgItem(IDC_SETHIDE)->EnableWindow(m_bHide);	
}

void CChannelConfig::OnMotionCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	GetDlgItem(IDC_SETMOTION)->EnableWindow(m_bMotion);	
}

void CChannelConfig::OnHidealarmCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	GetDlgItem(IDC_SETHIDEALARM)->EnableWindow(m_bHideAlarm);
}

void CChannelConfig::OnVlostalarmCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	GetDlgItem(IDC_SETVLOSTALARM)->EnableWindow(m_bVLostAlarm);
}

void CChannelConfig::OnOsdCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	EnableOSD(m_bOSD);
}

void CChannelConfig::OnChannelnameCheck() 
{
	if (!UpdateData(TRUE)) 
	{
		return;
	}
	EnableChanname(m_bChannelName);
}

void CChannelConfig::OnSethide() 
{
	CSetHide Dlg;
	int i = 0;
	
	for(i=0; i<MAX_SHELTERNUM; i++)
	{
		Dlg.SetShelter(i, &m_piccfg[ m_nChanSel].struShelter[i]);
	}
//	Dlg.m_iChannel = m_nChanNum;
//	Dlg.m_lServerID = m_lServerID;
	if(Dlg.DoModal() == IDOK)
	{
		for(i=0; i<MAX_SHELTERNUM; i++)
		{
			Dlg.GetShelter(i, &m_piccfg[m_nChanSel].struShelter[i]);
		}
		bSetChannelPara[m_nChanSel] = TRUE;
	}
}

void CChannelConfig::OnRestore() 
{
	OnInitDialog();
}

void CChannelConfig::OnSelchangeRatectrlsel() 
{
	if(m_rec_ratectrlsel.GetCurSel() == 22)	//自定义码流
	{
		GetDlgItem(IDC_RATECTRL)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_RATECTRL)->EnableWindow(FALSE);
	}
}

void CChannelConfig::OnSelchangeRatectrlsel2() 
{
	if(m_net_ratectrlsel.GetCurSel() == 22)	//自定义码流
	{
		GetDlgItem(IDC_RATECTRL2)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_RATECTRL2)->EnableWindow(FALSE);
	}
}

void CChannelConfig::OnSavechannelname() 
{
	UpdateData();
	char *buf = new char[512];
	memset(buf, 0 , 512);
	strcpy((char *)m_piccfg[m_nChanSel].sChanName, m_channelname.GetBuffer(0));
	for(int i = 0; i < 16; i ++)
	{
		memcpy(buf + 32*i, (char *)m_piccfg[i].sChanName, 32);
	}
	BOOL ret = CLIENT_SetupChannelName(m_Device->LoginID, buf, 512);
	if (!ret)
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
	}
}
