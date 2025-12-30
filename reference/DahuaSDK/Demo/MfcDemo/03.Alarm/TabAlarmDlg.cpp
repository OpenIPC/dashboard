// TabAlarmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClientDemo1.h"
#include "TabAlarmDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabAlarmDlg dialog


CTabAlarmDlg::CTabAlarmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTabAlarmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTabAlarmDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nChannelCount = 16;
	m_nInputAlarmCount = 16;
}


void CTabAlarmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTabAlarmDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTabAlarmDlg, CDialog)
	//{{AFX_MSG_MAP(CTabAlarmDlg)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabAlarmDlg message handlers

BOOL CTabAlarmDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTabAlarmDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

//Set general alamr output display 
void CTabAlarmDlg::SetCommonAlarmInfo(WORD wInputAlarm,WORD wVideoLost,WORD wMotionAlarm,
									  int nChannelCount,int nInputAlarmCount)
{
	SetCheckBox(nChannelCount,nInputAlarmCount);
	int i = 0;
	double x = 2;
	int nInputAlarmID = 1029;
	int nVideoLost = 1077;
	int nMotionID = 1061;
	//External input channel alarm 
	if(0 != wInputAlarm)
	{
		for(i=0;i<m_nInputAlarmCount;i++)
		{
			WORD wAI = (WORD)pow(x,i);
			WORD wInput = wInputAlarm | (~wAI);
			WORD wReverse = ~wInput;
			if(0 == wReverse)
			{
				int nIsChecked = ((CButton*)GetDlgItem(nInputAlarmID + i))->GetCheck();
				if(1 == nIsChecked)
				{
					((CButton*)GetDlgItem(nInputAlarmID + i))->SetCheck(0);
				}
				else
				{
					((CButton*)GetDlgItem(nInputAlarmID + i))->SetCheck(1);
				}
			}
		}
	}
	//Video loss alarm 
	if(0 != wVideoLost)
	{
		for(i=0;i<m_nChannelCount;i++)
		{
			WORD wAV = (WORD)pow(x,i);
			WORD wVideo = wVideoLost | (~wAV);
			WORD wReverse = ~wVideo;
			if(0 == wReverse)
			{
				int nIsChecked = ((CButton*)GetDlgItem(nVideoLost + i))->GetCheck();
				if(1 == nIsChecked)
				{
					((CButton*)GetDlgItem(nVideoLost + i))->SetCheck(0);
				}
				else
				{
					((CButton*)GetDlgItem(nVideoLost + i))->SetCheck(1);
				}
			}
		}
	}
	//Motion detection alarm 
	if(0 != wMotionAlarm)
	{
		for(i=0;i<m_nChannelCount;i++)
		{
			WORD wAV = (WORD)pow(x,i);
			WORD wMotion = wMotionAlarm | (~wAV);
			WORD wReverse = ~wMotion;
			if(0 == wReverse)
			{
				int nIsChecked = ((CButton*)GetDlgItem(nMotionID + i))->GetCheck();
				if(1 == nIsChecked)
				{
					((CButton*)GetDlgItem(nMotionID + i))->SetCheck(0);
				}
				else
				{
					((CButton*)GetDlgItem(nMotionID + i))->SetCheck(1);
				}
			}
		}
	}
}

//Set CheckBox according to channel amount 
void CTabAlarmDlg::SetCheckBox(int nChannelCount, int nInputAlarmCount)
{
	//Video input channel setup 
	if(m_nChannelCount != nChannelCount)
	{
		int i = 0;
		int nShelterID = 1045;
		int nMotionID = 1061;
		int nVideoLost = 1077;
		int nSoundID = 1093;
		if(m_nInputAlarmCount > nInputAlarmCount)
		{
			for(i=nInputAlarmCount;i<m_nInputAlarmCount;i++)
			{
				((CButton*)GetDlgItem(nShelterID + i))->SetCheck(0);
				GetDlgItem(nShelterID + i)->EnableWindow(FALSE);
				((CButton*)GetDlgItem(nMotionID + i))->SetCheck(0);
				GetDlgItem(nMotionID + i)->EnableWindow(FALSE);
				((CButton*)GetDlgItem(nVideoLost + i))->SetCheck(0);
				GetDlgItem(nVideoLost + i)->EnableWindow(FALSE);
				((CButton*)GetDlgItem(nSoundID + i))->SetCheck(0);
				GetDlgItem(nSoundID + i)->EnableWindow(FALSE);
			}
		}
		else
		{
			for(i=m_nInputAlarmCount;i<nInputAlarmCount;i++)
			{
				((CButton*)GetDlgItem(nShelterID + i))->SetCheck(0);
				GetDlgItem(nShelterID + i)->EnableWindow(TRUE);
				((CButton*)GetDlgItem(nMotionID + i))->SetCheck(0);
				GetDlgItem(nMotionID + i)->EnableWindow(TRUE);
				((CButton*)GetDlgItem(nVideoLost + i))->SetCheck(0);
				GetDlgItem(nVideoLost + i)->EnableWindow(TRUE);
				((CButton*)GetDlgItem(nSoundID + i))->SetCheck(0);
				GetDlgItem(nSoundID + i)->EnableWindow(TRUE);
			}
		}
		m_nChannelCount = nChannelCount;
	}
	//External input channel setup 
	if(m_nInputAlarmCount != nInputAlarmCount)
	{
		int i = 0;
		int nID = 1029;
		if(m_nInputAlarmCount > nInputAlarmCount)
		{
			for(i=nInputAlarmCount;i<m_nInputAlarmCount;i++)
			{
				((CButton*)GetDlgItem(nID + i))->SetCheck(0);
				GetDlgItem(nID + i)->EnableWindow(FALSE);
			}
		}
		else
		{
			for(i=m_nInputAlarmCount;i<nInputAlarmCount;i++)
			{
				((CButton*)GetDlgItem(nID + i))->SetCheck(0);
				GetDlgItem(nID + i)->EnableWindow(TRUE);
			}
		}
		m_nInputAlarmCount = nInputAlarmCount;
	}
}

//Display camera masking alarm 
void CTabAlarmDlg::SetShelterAlarmInfo(WORD wVideoShelter)
{
	int i = 0;
	double x = 2;
	int nShelterAlarmID = 1045;
	//Camera masking alarm 
	for(i=0;i<m_nChannelCount;i++)
	{
		WORD wAS = (WORD)pow(x,i);
		WORD wShelter = wVideoShelter | (~wAS);
		WORD wReverse = ~wShelter;
		if(0 == wReverse)
		{
			int nIsChecked = ((CButton*)GetDlgItem(nShelterAlarmID + i))->GetCheck();
			if(1 == nIsChecked)
			{
				((CButton*)GetDlgItem(nShelterAlarmID + i))->SetCheck(0);
			}
			else
			{
				((CButton*)GetDlgItem(nShelterAlarmID + i))->SetCheck(1);
			}
		}
	}
}

//Display audio alarm 
void CTabAlarmDlg::SetSoundAlarmInfo(WORD wSoundAlarm)
{
	int i = 0;
	double x = 2;
	int nSoundAlarmID = 1093;
	//Audio alarm 
	for(i=0;i<m_nChannelCount;i++)
	{
		WORD wAS = (WORD)pow(x,i);
		WORD wSound = wSoundAlarm | (~wAS);
		WORD wReverse = ~wSound;
		if(0 == wReverse)
		{
			int nIsChecked = ((CButton*)GetDlgItem(nSoundAlarmID + i))->GetCheck();
			if(1 == nIsChecked)
			{
				((CButton*)GetDlgItem(nSoundAlarmID + i))->SetCheck(0);
			}
			else
			{
				((CButton*)GetDlgItem(nSoundAlarmID + i))->SetCheck(1);
			}
		}
	}
}

//Display HDD space is insufficient. 
void CTabAlarmDlg::SetDiskFullAlarmInfo(BOOL bDiskFull)
{
	if(bDiskFull)
	{
		((CButton*)GetDlgItem(IDC_ALARM_DISKFULL))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_ALARM_DISKFULL))->SetCheck(0);
	}
}

//Display HDD malfucntion alarm 
void CTabAlarmDlg::SetDiskErrorAlarmInfo(DWORD dwDiskError)
{
	double x = 2;
	CString strDiskError;
	for(int i=0;i<32;i++)
	{
		DWORD dwAD = (DWORD)pow(x,i);
		DWORD dwDisk = dwDiskError | (~dwAD);
		DWORD wReverse = ~dwDisk;
		if(0 == wReverse)
		{
			CString str;
			str.Format(" %d ",i);
			strDiskError = strDiskError +str;
		}
	}
	SetDlgItemText(IDC_ALARM_DISKERROR,strDiskError);
}

void CTabAlarmDlg::CleanAllInfo(void) 
{
	CleanColum(IDC_ALARM_SHELTER0, 16);
	CleanColum(IDC_ALARM_INPUT0, 16);
	CleanColum(IDC_ALARM_MOTION0, 16);
	CleanColum(IDC_ALARM_VIDEOLOST0, 16);
	CleanColum(IDC_ALARM_SOUND0, 16);
	CleanColum(IDC_ALARM_DISKFULL,1);
}

void CTabAlarmDlg::CleanColum(UINT uID, int nLen) 
{
	for(int i = 0; i < nLen; i++)
	{
		((CButton*)GetDlgItem(uID + i))->SetCheck(FALSE);
	}
}


































































































