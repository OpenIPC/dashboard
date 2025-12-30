// QueryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "QueryDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg dialog


CQueryDlg::CQueryDlg(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CQueryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryDlg)
	m_StartDay = COleDateTime::GetCurrentTime();
	m_StartTime = 0;
	m_EndDay = COleDateTime::GetCurrentTime();
	m_EndTime = 0;
	m_nBeginNum = 0;
	m_nReallyNum = 0;
	m_nCount = 0;
	m_strTime = _T("unknown");
	m_nPresentID = 0;
	m_nRuleID = 0;
	m_strName = _T("unknown");
	m_nChannel = 0;
	m_nX = 0;
	m_nY = 0;
	m_strMeasuretype = _T("unknown");
	m_strUnit = _T("unknown");
	m_fAveTemp = 0.0f;
	m_fMaxTemp = 0.0f;
	m_fMidTemp = 0.0f;
	m_fMinTemp = 0.0f;
	m_fStdTemp = 0.0f;
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginId;
    m_nFinderHanle = 0;

	memset(&stOutDo, 0, sizeof(stOutDo));
	stOutDo.dwSize = sizeof(stOutDo);
}


void CQueryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryDlg)
	DDX_Control(pDX, IDC_COMBO_POINTNUM, m_Combo_PointNum);
	DDX_Control(pDX, IDC_COMBO_QUERYTYPE, m_Combo_Querytype);
	DDX_Control(pDX, IDC_COMBO_CHANNELNUM, m_Combo_ChannelNum);
	DDX_DateTimeCtrl(pDX, IDC_STARTDAY, m_StartDay);
	DDX_DateTimeCtrl(pDX, IDC_STARTTIME, m_StartTime);
	DDX_DateTimeCtrl(pDX, IDC_ENDDAY, m_EndDay);
	DDX_DateTimeCtrl(pDX, IDC_ENDTIME, m_EndTime);
	DDX_Text(pDX, IDC_EDIT_QUERYBEGINNUM, m_nBeginNum);
	DDX_Text(pDX, IDC_EDIT_AMOUNT, m_nReallyNum);
	DDX_Text(pDX, IDC_EDIT_COUNT, m_nCount);
	DDX_Text(pDX, IDC_EDIT_TIME, m_strTime);
	DDX_Text(pDX, IDC_EDIT_PRESETNUM, m_nPresentID);
	DDX_Text(pDX, IDC_EDIT_RULENUM, m_nRuleID);
	DDX_Text(pDX, IDC_EDIT_MEASURENAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_CHANNEL, m_nChannel);
	DDX_Text(pDX, IDC_EDIT_Y, m_nX);
	DDX_Text(pDX, IDC_EDIT_X, m_nY);
	DDX_Text(pDX, IDC_EDIT_MEASURETYPE, m_strMeasuretype);
	DDX_Text(pDX, IDC_EDIT_UNIT, m_strUnit);
	DDX_Text(pDX, IDC_EDIT_TEMP, m_fAveTemp);
	DDX_Text(pDX, IDC_EDIT_MAXTEMP, m_fMaxTemp);
	DDX_Text(pDX, IDC_EDIT_MIDTEMP, m_fMidTemp);
	DDX_Text(pDX, IDC_EDIT_MINTEMP, m_fMinTemp);
	DDX_Text(pDX, IDC_EDIT_STDTEMP, m_fStdTemp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryDlg)
	ON_BN_CLICKED(IDC_BTN_QUERY, OnBtnQuery)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryDlg message handlers

void CQueryDlg::OnBtnQuery() 
{
	UpdateData();
    NET_IN_RADIOMETRY_STARTFIND stInStart = {sizeof(stInStart)};
    NET_OUT_RADIOMETRY_STARTFIND stOutStart = {sizeof(stOutStart)};
    stInStart.nChannel = 1/*m_Combo_ChannelNum.GetCurSel()*/;
    stInStart.nMeterType = m_Combo_Querytype.GetCurSel();
    stInStart.stStartTime.dwYear = m_StartDay.GetYear();
    stInStart.stStartTime.dwMonth = m_StartDay.GetMonth();
    stInStart.stStartTime.dwDay = m_StartDay.GetDay();
    stInStart.stStartTime.dwHour = m_StartTime.GetHour();
    stInStart.stStartTime.dwMinute = m_StartTime.GetMinute();
    stInStart.stStartTime.dwSecond = m_StartTime.GetSecond();

    stInStart.stEndTime.dwYear = m_EndDay.GetYear();
    stInStart.stEndTime.dwMonth = m_EndDay.GetMonth();
    stInStart.stEndTime.dwDay = m_EndDay.GetDay();
    stInStart.stEndTime.dwHour = m_EndTime.GetHour();
    stInStart.stEndTime.dwMinute = m_EndTime.GetMinute();
    stInStart.stEndTime.dwSecond = m_EndTime.GetSecond();
    if (!m_lLoginID)
    {
        MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
        return;
    }
    BOOL ret = CLIENT_StartFind(m_lLoginID,NET_FIND_RADIOMETRY,&stInStart,&stOutStart,2000);
    NET_IN_RADIOMETRY_DOFIND stInDo = {sizeof(stInDo)};
    m_nFinderHanle = stOutStart.nFinderHanle;
    stInDo.nFinderHanle = m_nFinderHanle;
    stInDo.nCount = m_nCount;
    stInDo.nBeginNumber = m_nBeginNum;
    if ((stInDo.nCount>NET_RADIOMETRY_DOFIND_MAX) || (stInDo.nBeginNumber>stOutStart.nTotalCount))
    {
        return;
    }
    if (ret != 0)
    {
        for (int i=0;i<5;i++)
        {
            ret = CLIENT_DoFind(m_lLoginID,NET_FIND_RADIOMETRY,&stInDo,&stOutDo,2000);
            if (ret)
            {
                break;
            }
        }
    }
    if (ret)
    {
        m_nReallyNum = stOutDo.nFound;
        DWORD dwYear = stOutDo.stInfo[0].stTime.dwYear;
        DWORD dwMonth = stOutDo.stInfo[0].stTime.dwMonth;
        DWORD dwDay = stOutDo.stInfo[0].stTime.dwDay;
        DWORD dwHour = stOutDo.stInfo[0].stTime.dwHour;
        DWORD dwMinute = stOutDo.stInfo[0].stTime.dwMinute;
        DWORD dwSecond = stOutDo.stInfo[0].stTime.dwSecond;
        m_strTime.Format("%4d:%2d%:%2d:%2d:%2d:%2d",dwYear,dwMonth,dwDay,dwHour,dwMinute,dwSecond);
        m_nPresentID = stOutDo.stInfo[0].nPresetId;
        m_nRuleID = stOutDo.stInfo[0].nRuleId;
        m_strName = stOutDo.stInfo[0].szName;
        m_nChannel = stOutDo.stInfo[0].nChannel;
        m_nX = stOutDo.stInfo[0].stCoordinate.nx;
        m_nY = stOutDo.stInfo[0].stCoordinate.ny;
        if (stOutDo.stInfo[0].stTemperInfo.nMeterType == NET_RADIOMETRY_METERTYPE_UNKNOWN)
        {
            m_strMeasuretype = ConvertString("unknown");
        }
        else if (stOutDo.stInfo[0].stTemperInfo.nMeterType == NET_RADIOMETRY_METERTYPE_SPOT)
        {
            m_strMeasuretype = ConvertString("spot");
        }
        else if (stOutDo.stInfo[0].stTemperInfo.nMeterType == NET_RADIOMETRY_METERTYPE_LINE)
        {
            m_strMeasuretype = ConvertString("line");
        }
        else if (stOutDo.stInfo[0].stTemperInfo.nMeterType == NET_RADIOMETRY_METERTYPE_AREA)
        {
            m_strMeasuretype = ConvertString("area");
        }
        if (stOutDo.stInfo[0].stTemperInfo.nTemperUnit == TEMPERATURE_UNIT_UNKNOWN)
        {
            m_strUnit = ConvertString("unknown");
        }
        else if (stOutDo.stInfo[0].stTemperInfo.nTemperUnit == TEMPERATURE_UNIT_CENTIGRADE)
        {
            m_strUnit = ConvertString("Celsius");
        }
        else if (stOutDo.stInfo[0].stTemperInfo.nTemperUnit == TEMPERATURE_UNIT_FAHRENHEIT)
        {
            m_strUnit = ConvertString("Fahrenheit");
        }
        m_fAveTemp = stOutDo.stInfo[0].stTemperInfo.fTemperAver;
        m_fMaxTemp = stOutDo.stInfo[0].stTemperInfo.fTemperMax;
        m_fMinTemp = stOutDo.stInfo[0].stTemperInfo.fTemperMin;
        m_fMidTemp = stOutDo.stInfo[0].stTemperInfo.fTemperMin;
        m_fStdTemp = stOutDo.stInfo[0].stTemperInfo.fTemperStd;
    }
    else
    {
        MessageBox(ConvertString("Query Fail"),ConvertString("Prompt"));
    }
}

BOOL CQueryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Combo_Querytype.SetCurSel(0);
    m_Combo_ChannelNum.SetCurSel(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
