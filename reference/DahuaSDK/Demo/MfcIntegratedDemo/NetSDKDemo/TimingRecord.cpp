// TimingRecord.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "TimingRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTimingRecord dialog


CTimingRecord::CTimingRecord(CWnd* pParent /*=NULL*/)
	: CDialog(CTimingRecord::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTimingRecord)
	m_bAllDayRecord = FALSE;
	m_hour1 = 0;
	m_hour2 = 0;
	m_hour3 = 0;
	m_hour4 = 0;
	m_minute1 = 0;
	m_minute2 = 0;
	m_minute3 = 0;
	m_minute4 = 0;
	m_shour1 = 0;
	m_shour2 = 0;
	m_shour3 = 0;
	m_shour4 = 0;
	m_sminute1 = 0;
	m_sminute2 = 0;
	m_sminute3 = 0;
	m_sminute4 = 0;
	//}}AFX_DATA_INIT
}


void CTimingRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTimingRecord)
	DDX_Control(pDX, IDC_RECORDTYPESEL, m_alldaytypesel);
	DDX_Control(pDX, IDC_RECORDTYPESEL4, m_recordtype4sel);
	DDX_Control(pDX, IDC_RECORDTYPESEL3, m_recordtype3sel);
	DDX_Control(pDX, IDC_RECORDTYPESEL2, m_recordtype2sel);
	DDX_Control(pDX, IDC_RECORDTYPESEL1, m_recordtype1sel);
	DDX_Control(pDX, IDC_RECORDDAYSEL, m_recorddaysel);
	DDX_Control(pDX, IDC_COPYDAYSEL, m_copydaysel);
	DDX_Check(pDX, IDC_ALLDAY_CHECK, m_bAllDayRecord);
	DDX_Text(pDX, IDC_HOUR1, m_hour1);
	DDX_Text(pDX, IDC_HOUR2, m_hour2);
	DDX_Text(pDX, IDC_HOUR3, m_hour3);
	DDX_Text(pDX, IDC_HOUR4, m_hour4);
	DDX_Text(pDX, IDC_MINUTE1, m_minute1);
	DDX_Text(pDX, IDC_MINUTE2, m_minute2);
	DDX_Text(pDX, IDC_MINUTE3, m_minute3);
	DDX_Text(pDX, IDC_MINUTE4, m_minute4);
	DDX_Text(pDX, IDC_SHOUR1, m_shour1);
	DDX_Text(pDX, IDC_SHOUR2, m_shour2);
	DDX_Text(pDX, IDC_SHOUR3, m_shour3);
	DDX_Text(pDX, IDC_SHOUR4, m_shour4);
	DDX_Text(pDX, IDC_SMINUTE1, m_sminute1);
	DDX_Text(pDX, IDC_SMINUTE2, m_sminute2);
	DDX_Text(pDX, IDC_SMINUTE3, m_sminute3);
	DDX_Text(pDX, IDC_SMINUTE4, m_sminute4);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTimingRecord, CDialog)
	//{{AFX_MSG_MAP(CTimingRecord)
	ON_CBN_SELCHANGE(IDC_RECORDDAYSEL, OnSelchangeRecorddaysel)
	ON_BN_CLICKED(IDC_ALLDAY_CHECK, OnAlldayCheck)
	ON_BN_CLICKED(IDC_BTNOK, OnBtnok)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTimingRecord message handlers




CTimingRecord::SetRecordInfo(NET_DEV_RECORD *recinfo)
{
	m_recordinfo = recinfo;
}

BOOL CTimingRecord::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_nDate = 0;
	m_recorddaysel.SetCurSel(m_nDate);
	m_nCopyDate = 0;
	m_copydaysel.SetCurSel(m_nCopyDate);
	m_bAllDayRecord = m_recordinfo->struRecAllDay[m_nDate].wAllDayRecord;
	m_alldaytypesel.SetCurSel(m_recordinfo->struRecAllDay[m_nDate].byRecordType);		
	m_recordtype1sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][0].byRecordType);
	m_recordtype2sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][1].byRecordType);
	m_recordtype3sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][2].byRecordType);
	m_recordtype4sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][3].byRecordType);
	m_hour1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStartHour;
	m_shour1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStopHour;
	m_hour2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStartHour;
	m_shour2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStopHour;
	m_hour3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStartHour;
	m_shour3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStopHour;
	m_hour4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStartHour;
	m_shour4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStopHour;
	m_minute1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStartMin;
	m_sminute1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStopMin;
	m_minute2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStartMin;
	m_sminute2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStopMin;
	m_minute3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStartMin;
	m_sminute3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStopMin;
	m_minute4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStartMin;
	m_sminute4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStopMin;
	EnableControl(!m_bAllDayRecord);
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CTimingRecord::EnableControl(BOOL bEnable)
{
	GetDlgItem(IDC_HOUR1)->EnableWindow(bEnable);
	GetDlgItem(IDC_HOUR2)->EnableWindow(bEnable);
	GetDlgItem(IDC_HOUR3)->EnableWindow(bEnable);
	GetDlgItem(IDC_HOUR4)->EnableWindow(bEnable);
	
	GetDlgItem(IDC_MINUTE1)->EnableWindow(bEnable);
	GetDlgItem(IDC_MINUTE2)->EnableWindow(bEnable);
	GetDlgItem(IDC_MINUTE3)->EnableWindow(bEnable);
	GetDlgItem(IDC_MINUTE4)->EnableWindow(bEnable);
	
	GetDlgItem(IDC_SHOUR1)->EnableWindow(bEnable);
	GetDlgItem(IDC_SHOUR2)->EnableWindow(bEnable);
	GetDlgItem(IDC_SHOUR3)->EnableWindow(bEnable);
	GetDlgItem(IDC_SHOUR4)->EnableWindow(bEnable);
	
	GetDlgItem(IDC_SMINUTE1)->EnableWindow(bEnable);
	GetDlgItem(IDC_SMINUTE2)->EnableWindow(bEnable);
	GetDlgItem(IDC_SMINUTE3)->EnableWindow(bEnable);
	GetDlgItem(IDC_SMINUTE4)->EnableWindow(bEnable);
	
	GetDlgItem(IDC_RECORDTYPESEL1)->EnableWindow(bEnable);
	GetDlgItem(IDC_RECORDTYPESEL2)->EnableWindow(bEnable);
	GetDlgItem(IDC_RECORDTYPESEL3)->EnableWindow(bEnable);
	GetDlgItem(IDC_RECORDTYPESEL4)->EnableWindow(bEnable);
	
	GetDlgItem(IDC_RECORDTYPESEL)->EnableWindow(!bEnable);
}

void CTimingRecord::OnSelchangeRecorddaysel() 
{
	m_nDate = m_recorddaysel.GetCurSel();	
	m_bAllDayRecord = m_recordinfo->struRecAllDay[m_nDate].wAllDayRecord;
	m_alldaytypesel.SetCurSel(m_recordinfo->struRecAllDay[m_nDate].byRecordType);
	m_recordtype1sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][0].byRecordType);
	m_recordtype2sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][1].byRecordType);
	m_recordtype3sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][2].byRecordType);
	m_recordtype4sel.SetCurSel(m_recordinfo->struRecordSched[m_nDate][3].byRecordType);
	m_hour1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStartHour;
	m_shour1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStopHour;
	m_hour2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStartHour;
	m_shour2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStopHour;
	m_hour3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStartHour;
	m_shour3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStopHour;
	m_hour4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStartHour;
	m_shour4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStopHour;
	m_minute1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStartMin;
	m_sminute1 = m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStopMin;
	m_minute2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStartMin;
	m_sminute2 = m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStopMin;
	m_minute3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStartMin;
	m_sminute3 = m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStopMin;
	m_minute4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStartMin;
	m_sminute4 = m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStopMin;
	EnableControl(!m_bAllDayRecord);
	GetDlgItem(IDC_BTNCOPY)->EnableWindow(FALSE);
	UpdateData(FALSE);	
}

void CTimingRecord::OnAlldayCheck() 
{
	UpdateData(TRUE);
	EnableControl(!m_bAllDayRecord);
	
}

BOOL CTimingRecord::TimeTest()
{
	int i,j;
	CString sTemp;
	CString sTime[4][2];
	sTime[0][0].Format("%02d%02d",m_hour1,m_minute1);
	sTime[0][1].Format("%02d%02d",m_shour1,m_sminute1);
	sTime[1][0].Format("%02d%02d",m_hour2,m_minute2);
	sTime[1][1].Format("%02d%02d",m_shour2,m_sminute2);
	sTime[2][0].Format("%02d%02d",m_hour3,m_minute3);
	sTime[2][1].Format("%02d%02d",m_shour3,m_sminute3);
	sTime[3][0].Format("%02d%02d",m_hour4,m_minute4);
	sTime[3][1].Format("%02d%02d",m_shour4,m_sminute4);
	for(i=0;i<4;i++)
	{
		if ((sTime[i][0].Left(2)>"24")||(sTime[i][0].Left(2)>"24")||(sTime[i][0].Left(2)<"00")||(sTime[i][0].Left(2)<"00"))
		{
			sTemp.Format("%d",(i+1));
			sTemp = MSG_TIMINGREC_STARTTIMEERROR + sTemp;
			AfxMessageBox(sTemp);
			return FALSE;
		}
		if ((sTime[i][0]>"2400")||(sTime[i][1]>"2400")||(sTime[i][0]<"0000")||(sTime[i][1]<"0000"))
		{
			sTemp.Format("%d",(i+1));
			sTemp = MSG_TIMINGREC_STARTTIMEERROR + sTemp;
			AfxMessageBox(sTemp);
			return FALSE;
		}
		if ((sTime[i][0].Right(2)>"59")||(sTime[i][0].Right(2)>"59")||(sTime[i][0].Right(2)<"00")||(sTime[i][0].Right(2)<"00"))
		{
			sTemp.Format("%d",(i+1));
			sTemp = MSG_TIMINGREC_STARTTIMEERROR + sTemp;
			AfxMessageBox(sTemp);
			return FALSE;
		}
		if ((sTime[i][1].Left(2)>"24")||(sTime[i][1].Left(2)>"24")||(sTime[i][1].Left(2)<"00")||(sTime[i][1].Left(2)<"00"))
		{
			sTemp.Format("%d",(i+1));
			sTemp = MSG_TIMINGREC_STOPTIMEERROR + sTemp;
			AfxMessageBox(sTemp);
			return FALSE;
		}
		if ((sTime[i][1].Right(2)>"59")||(sTime[i][1].Right(2)>"59")||(sTime[i][1].Right(2)<"00")||(sTime[i][1].Right(2)<"00"))
		{
			sTemp.Format("%d",(i+1));
			sTemp = MSG_TIMINGREC_STOPTIMEERROR + sTemp;
			AfxMessageBox(sTemp);
			return FALSE;
		}
		if(sTime[i][0]>=sTime[i][1])
		{
			if((sTime[i][0]!="0000") || (sTime[i][1]!="0000"))
			{
				sTemp.Format("%d",(i+1));
				sTemp = MSG_TIMINGREC_STOPTIMEERROR + sTemp;
				AfxMessageBox(sTemp);
				return FALSE;
			}
		}
		for (j=i+1;j<4;j++)
		{
			if((sTime[j][0]>sTime[i][0]) && (sTime[j][0]<sTime[i][1]))
			{
				sTemp.Format("%d,%d",(i+1),(j+1));
				sTemp = MSG_TIMINGREC_TIMEOVERLAY + sTemp;
				AfxMessageBox(sTemp);
				return FALSE;
			}else if((sTime[j][1]>sTime[i][0]) && (sTime[j][1]<sTime[i][1]))
			{
				sTemp.Format("%d,%d",(i+1),(j+1));
				sTemp = MSG_TIMINGREC_TIMEOVERLAY + sTemp;
				AfxMessageBox(sTemp);
				return FALSE;
			}else if((sTime[j][0]<=sTime[i][0]) && (sTime[j][1]>=sTime[i][1]))
			{
				if((sTime[j][0]!="0000") && (sTime[j][1]!="0000"))
				{
					sTemp.Format("%d,%d",(i+1),(j+1));
					sTemp = MSG_TIMINGREC_TIMEOVERLAY + sTemp;
					AfxMessageBox(sTemp);
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

void CTimingRecord::OnBtnok() 
{
	if (!UpdateData(TRUE))
	{
		return;
	}
	if(!TimeTest())
	{
		return;
	}
	
	m_recordinfo->struRecAllDay[m_nDate].wAllDayRecord = m_bAllDayRecord;
	if(m_bAllDayRecord)
	{		
		m_recordinfo->struRecAllDay[m_nDate].byRecordType = m_alldaytypesel.GetCurSel();
	}
	else
	{
		m_recordinfo->struRecordSched[m_nDate][0].byRecordType = m_recordtype1sel.GetCurSel();
		m_recordinfo->struRecordSched[m_nDate][1].byRecordType = m_recordtype2sel.GetCurSel();
		m_recordinfo->struRecordSched[m_nDate][2].byRecordType = m_recordtype3sel.GetCurSel();
		m_recordinfo->struRecordSched[m_nDate][3].byRecordType = m_recordtype4sel.GetCurSel();			
		m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStartHour = m_hour1;
		m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStopHour = m_shour1;
		m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStartHour = m_hour2;
		m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStopHour = m_shour2;
		m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStartHour = m_hour3;
		m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStopHour = m_shour3;
		m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStartHour = m_hour4;
		m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStopHour = m_shour4;
		
		m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStartMin = m_minute1;
		m_recordinfo->struRecordSched[m_nDate][0].struRecordTime.byStopMin = m_sminute1;
		m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStartMin = m_minute2;
		m_recordinfo->struRecordSched[m_nDate][1].struRecordTime.byStopMin = m_sminute2;
		m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStartMin = m_minute3;
		m_recordinfo->struRecordSched[m_nDate][2].struRecordTime.byStopMin = m_sminute3;
		m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStartMin = m_minute4;
		m_recordinfo->struRecordSched[m_nDate][3].struRecordTime.byStopMin = m_sminute4;
	}
	GetDlgItem(IDC_BTNCOPY)->EnableWindow(TRUE);	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
