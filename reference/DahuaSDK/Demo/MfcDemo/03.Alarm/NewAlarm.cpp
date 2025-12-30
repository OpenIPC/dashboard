// NewAlarm.cpp : implementation file
//

#include "stdafx.h"
#include "ClientDemo1.h"
#include "NewAlarm.h"
#include "resource.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewAlarm dialog
void CNewAlarm::SetAlarmInfo(BYTE *dwAlarm,CDlgChn* dlgAlarm, int nAlarmCount)
{
	dlgAlarm->SetChnState(dwAlarm, nAlarmCount);
}

void CNewAlarm::SetSoundAlarmInfo(DH_NEW_SOUND_ALARM_STATE &stuSoundAlarm,DWORD	dwID)
{
	char szBuf[2048] ={0};
	int nCount =stuSoundAlarm.channelcount;
	for(int i=0;i<nCount;i++)
	{
		char szTempBuf[256];
		memset(szTempBuf,0,sizeof(char)*256);
		sprintf(szTempBuf
			,"channel:%d type:%d volume:%d \n"
			,stuSoundAlarm.SoundAlarmInfo[i].channel
			,stuSoundAlarm.SoundAlarmInfo[i].alarmType
			,stuSoundAlarm.SoundAlarmInfo[i].volume);
		strcat(szBuf,szTempBuf);
	}
	SetDlgItemText(dwID,szBuf);
}
void CNewAlarm::SetCheckBox(int nChannelCount, int nInputAlarmCount)
{
		
}

void CNewAlarm::SetAlarmDecoderAlarm(ALARM_DECODER_ALARM & stuAlarmDecoderAlarm)
{
	if(stuAlarmDecoderAlarm.nAlarmDecoderNum <= 0)
	{
		return ;
	}
	UINT uID[8];//7:Now it supports max 8 alarm decoders.
	uID[0] = IDC_ARRAY_DECODER00;
	uID[1] = IDC_ARRAY_DECODER10;
	uID[2] = IDC_ARRAY_DECODER20;
	uID[3] = IDC_ARRAY_DECODER30;
	uID[4] = IDC_ARRAY_DECODER40;
	uID[5] = IDC_ARRAY_DECODER50;
	uID[6] = IDC_ARRAY_DECODER60;
	uID[7] = IDC_ARRAY_DECODER70;
	
	//Clear all CheckBox.
	for(int iClear = 0; iClear < 8; iClear++)
	{
		for(int jCheck = 0; jCheck < 8; jCheck++)
		{
			((CButton *)GetDlgItem(uID[iClear] + jCheck))->SetCheck(0);
		}
	}

	WORD wAlarmData = 0;//To get alarm data 
	for(int i = 0; i < stuAlarmDecoderAlarm.nAlarmDecoderNum; i++)
	{
		wAlarmData = (unsigned short)stuAlarmDecoderAlarm.stuAlarmDecoder[i].dwAlarmDecoder;
		for(int j = 0; j < 8; j++)//8:Bit means the 8 input ports of alarm decoder 
		{
			((CButton *)GetDlgItem(uID[i] + j))->SetCheck(wAlarmData & 0x0001);//Get the alarm information from the lowest bit 
			wAlarmData >>= 0x0001;
		}
	}
}

void CNewAlarm::SetAlarmAlarmArmDisarmState(ALARM_ARM_DISARM_STATE_INFO & stuAlarmArmDisarmstate)
{
	((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(stuAlarmArmDisarmstate.bState);
}
CNewAlarm::CNewAlarm(CWnd* pParent /*=NULL*/)
	: CDialog(CNewAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewAlarm)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNewAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewAlarm)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewAlarm, CDialog)
	//{{AFX_MSG_MAP(CNewAlarm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewAlarm message handlers

BOOL CNewAlarm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);

	m_dlgDiskError.Create(IDD_DIALOG_CHN, this);
	m_dlgDiskFull.Create(IDD_DIALOG_CHN, this);
	m_dlgDynamic.Create(IDD_DIALOG_CHN, this);
	m_dlgShelter.Create(IDD_DIALOG_CHN, this);
	m_dlgVideoLost.Create(IDD_DIALOG_CHN, this);
	m_dlgOutput.Create(IDD_DIALOG_CHN, this);

	m_dlgDiskError.Attach(IDC_STATIC_DISKERROR, this);
	m_dlgDiskFull.Attach(IDC_STATIC_DISKFULL, this);
	m_dlgDynamic.Attach(IDC_STATIC_DYNAMIC, this);
	m_dlgShelter.Attach(IDC_STATIC_SHELTER, this);
	m_dlgVideoLost.Attach(IDC_STATIC_VIDEOLOST, this);
	m_dlgOutput.Attach(IDC_STATIC_OUTPUT, this);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewAlarm::CleanAllInfo()
{
	BYTE alarmInfo[32] = {0};
	m_dlgDiskError.SetChnState(alarmInfo, 32);
	m_dlgDiskFull.SetChnState(alarmInfo, 32);
	m_dlgDynamic.SetChnState(alarmInfo, 32);
	m_dlgShelter.SetChnState(alarmInfo, 32);
	m_dlgVideoLost.SetChnState(alarmInfo, 32);
	m_dlgOutput.SetChnState(alarmInfo, 32);

	CleanColum(IDC_ARRAY_DECODER00, 8);
	CleanColum(IDC_ARRAY_DECODER10, 8);
	CleanColum(IDC_ARRAY_DECODER20, 8);
	CleanColum(IDC_ARRAY_DECODER30, 8);
	CleanColum(IDC_ARRAY_DECODER40, 8);
	CleanColum(IDC_ARRAY_DECODER50, 8);
	CleanColum(IDC_ARRAY_DECODER60, 8);
	CleanColum(IDC_ARRAY_DECODER70, 8);
} 

void CNewAlarm::CleanColum(UINT uID, int nLen) 
{
	for(int i = 0; i < nLen; i++)
	{
	//	((CButton*)GetDlgItem(uID + i))->SetCheck(FALSE);
	}
}

































































































