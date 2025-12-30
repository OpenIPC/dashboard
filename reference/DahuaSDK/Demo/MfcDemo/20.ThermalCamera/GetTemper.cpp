// GetTemper.cpp : 实现文件
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "GetTemper.h"
#include "GlobalDlg.h"
#include "ThermalCameraDlg.h"

// CGetTemper 对话框

IMPLEMENT_DYNAMIC(CGetTemper, CDialog)

CGetTemper::CGetTemper(CWnd* pParent /*=NULL*/,LLONG iLoginId,int nChannelCount)
	: CDialog(CGetTemper::IDD, pParent)
{
	m_lLoginID = iLoginId;
	m_nChannelCount = nChannelCount;
	m_nX1 = 0;
	m_nY1 = 0;
	m_nX2 = 8191;
	m_nY2 = 0;
	m_nX3 = 0;
	m_nY3 = 8191;
	m_nX4 = 8191;
	m_nY4 = 8191;
	m_unit = _T("unknown");
	m_dbTemperAver = 0;
	m_dbTemperMax = 0;
	m_dbTemperMin = 0;
	m_nTemperMaxPointX = 0;
	m_nTemperMaxPointY = 0;
	m_nTemperMinPointX = 0;
	m_nTemperMinPointY = 0;
}


CGetTemper::~CGetTemper()
{
}

void CGetTemper::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COM_CHANNEL, m_combo_ch);
	DDX_Text(pDX, IDC_EDIT_POINT1_X, m_nX1);
	DDX_Text(pDX, IDC_EDIT_POINT1_Y, m_nY1);
	DDX_Text(pDX, IDC_EDIT_POINT2_X, m_nX2);
	DDX_Text(pDX, IDC_EDIT_POINT2_Y, m_nY2);
	DDX_Text(pDX, IDC_EDIT_POINT3_X, m_nX3);
	DDX_Text(pDX, IDC_EDIT_POINT3_Y, m_nY3);
	DDX_Text(pDX, IDC_EDIT_POINT4_X, m_nX4);
	DDX_Text(pDX, IDC_EDIT_POINT4_Y, m_nY4);

	DDX_Text(pDX, IDC_EDIT6, m_unit);
	DDX_Text(pDX, IDC_EDIT7, m_dbTemperAver);
	DDX_Text(pDX, IDC_EDIT8, m_dbTemperMax);
	DDX_Text(pDX, IDC_EDIT9, m_dbTemperMin);
	DDX_Text(pDX, IDC_EDIT10, m_nTemperMaxPointX);
	DDX_Text(pDX, IDC_EDIT11, m_nTemperMaxPointY);
	DDX_Text(pDX, IDC_EDIT12, m_nTemperMinPointX);
	DDX_Text(pDX, IDC_EDIT13, m_nTemperMinPointY);

}


BEGIN_MESSAGE_MAP(CGetTemper, CDialog)
	ON_BN_CLICKED(IDC_GETTEMPER, &CGetTemper::OnBnClickedGettemper)
END_MESSAGE_MAP()


BOOL CGetTemper::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	for (int i = 0 ;i<m_nChannelCount;i++)
	{
		CString str;
		str.Format("%d",i);
		m_combo_ch.AddString(str);

	}
	if (m_nChannelCount>0)
	{
		m_combo_ch.SetCurSel(0);
	}
	return TRUE;
}

// CGetTemper 消息处理程序

void CGetTemper::OnBnClickedGettemper()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bRet = UpdateData();   //添加了updataData是为了获取控件的值，如果是正确的，返回一个true，如果是错误的，返回一个false
	if (!bRet)
	{
		return;
	}

	NET_IN_RADIOMETRY_RANDOM_REGION_TEMPER stuIn = {sizeof(stuIn)};
	stuIn.nChannel = m_combo_ch.GetCurSel();
	stuIn.nPointNum = 4;
	stuIn.stuPolygon[0].nx = m_nX1;
	stuIn.stuPolygon[0].ny = m_nY1;
	stuIn.stuPolygon[1].nx = m_nX2;
	stuIn.stuPolygon[1].ny = m_nY2;
	stuIn.stuPolygon[2].nx = m_nX3;
	stuIn.stuPolygon[2].ny = m_nY3;
	stuIn.stuPolygon[3].nx = m_nX4;
	stuIn.stuPolygon[3].ny = m_nY4;
	NET_OUT_RADIOMETRY_RANDOM_REGION_TEMPER stuOut = {sizeof(stuOut)};
	bRet = CLIENT_RadiometryGetRandomRegionTemper(m_lLoginID, &stuIn, &stuOut, 5000);
	if (bRet)
	{
		m_dbTemperAver = (double)stuOut.stuRegionTempInfo.nTemperAver / 100;
		m_dbTemperMax = (double)stuOut.stuRegionTempInfo.nTemperMax / 100;
		m_dbTemperMin = (double)stuOut.stuRegionTempInfo.nTemperMin / 100;
		if (0 == stuOut.stuRegionTempInfo.emTemperatureUnit)
		{
			m_unit = ConvertString("Centigrade");
		}
		else if (1 == stuOut.stuRegionTempInfo.emTemperatureUnit)
		{
			m_unit = ConvertString("Fahrenheit");
		}
		m_nTemperMaxPointX = stuOut.stuRegionTempInfo.stuTemperMaxPoint.nx;
		m_nTemperMaxPointY = stuOut.stuRegionTempInfo.stuTemperMaxPoint.ny;
		m_nTemperMinPointX = stuOut.stuRegionTempInfo.stuTemperMinPoint.nx;
		m_nTemperMinPointY = stuOut.stuRegionTempInfo.stuTemperMinPoint.ny;
	}
	else
	{
		MessageBox(ConvertString("get info error"), ConvertString("Prompt"));
	}
	UpdateData(FALSE);

}
