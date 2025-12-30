// QueryPointInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "QueryPointInfoDlg.h"
#include "GlobalDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQueryPointInfoDlg dialog


CQueryPointInfoDlg::CQueryPointInfoDlg(CWnd* pParent /*=NULL*/,LLONG iLoginId,int nChannel)
	: CDialog(CQueryPointInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQueryPointInfoDlg)
	m_nX = 0;
	m_nY = 0;
	m_type = _T("unknown");
	m_unit = _T("unknown");
	m_temp = 0.0f;
	//}}AFX_DATA_INIT
    m_lLoginID = iLoginId;
    m_nChannel = nChannel;
}


void CQueryPointInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQueryPointInfoDlg)
	DDX_Text(pDX, IDC_EDIT_X, m_nX);
//	DDV_MinMaxInt(pDX, m_nX, 0, 8191);
	DDX_Text(pDX, IDC_EDIT_Y, m_nY);
//	DDV_MinMaxInt(pDX, m_nY, 0, 8191);
	DDX_Text(pDX, IDC_EDIT_MEASURETYPE, m_type);
	DDX_Text(pDX, IDC_EDIT_UNIT, m_unit);
	DDX_Text(pDX, IDC_EDIT_TEMP, m_temp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQueryPointInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CQueryPointInfoDlg)
	ON_BN_CLICKED(IDC_POINTQUERY, OnPointquery)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQueryPointInfoDlg message handlers

void CQueryPointInfoDlg::OnPointquery() 
{
    BOOL bret = UpdateData();   //添加了updataData是为了获取控件的值，如果是正确的，返回一个true，如果是错误的，返回一个false
	if (!bret)
	{
		return;
	}
    NET_IN_RADIOMETRY_GETPOINTTEMPER stuInGetPointTemper = {sizeof(stuInGetPointTemper)};
    stuInGetPointTemper.dwSize = sizeof(stuInGetPointTemper);
    stuInGetPointTemper.nChannel = m_nChannel;
    stuInGetPointTemper.stCoordinate.nx = m_nX;
    stuInGetPointTemper.stCoordinate.ny = m_nY;
    NET_OUT_RADIOMETRY_GETPOINTTEMPER stuOutPointTemper = {sizeof(stuOutPointTemper)};
    BOOL ret = CLIENT_QueryDevInfo(m_lLoginID,NET_QUERY_DEV_RADIOMETRY_POINT_TEMPER,&stuInGetPointTemper,&stuOutPointTemper,NULL,1000);
    if (0 != ret)
    {
        int iMeterType = stuOutPointTemper.stPointTempInfo.nMeterType;
        if (NET_RADIOMETRY_METERTYPE_UNKNOWN == iMeterType)
        {
            m_type = ConvertString("Unknown");
        }
        else if (NET_RADIOMETRY_METERTYPE_SPOT == iMeterType)
        {
            m_type = ConvertString("Spot");
        }
        else if (NET_RADIOMETRY_METERTYPE_LINE == iMeterType)
        {
            m_type = ConvertString("Line");
        }
        else if (NET_RADIOMETRY_METERTYPE_AREA == iMeterType)
        {
            m_type = ConvertString("Area");
        }
        int iMeterUnit = stuOutPointTemper.stPointTempInfo.nTemperUnit;
        if (TEMPERATURE_UNIT_UNKNOWN == iMeterUnit)
        {
            m_unit = ConvertString("Unknown");
        }
        else if (TEMPERATURE_UNIT_CENTIGRADE == iMeterUnit)
        {
            m_unit = ConvertString("Celsius");
        }
        else if (TEMPERATURE_UNIT_FAHRENHEIT == iMeterUnit)
        {
            m_unit = ConvertString("Fahrenheit");
        }
        m_temp = stuOutPointTemper.stPointTempInfo.fTemperAver;
    }
    else
    {
        MessageBox(ConvertString("get info error"), ConvertString("Prompt"));
    }
    UpdateData(FALSE);
}

BOOL CQueryPointInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
