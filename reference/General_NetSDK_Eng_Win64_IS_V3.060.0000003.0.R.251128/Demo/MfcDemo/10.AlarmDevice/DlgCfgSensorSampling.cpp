// DlgCfgSensorSampling.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgSensorSampling.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorSampling dialog


CDlgCfgSensorSampling::CDlgCfgSensorSampling(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgCfgSensorSampling::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgSensorSampling)
	m_edtDetectionPeriod = 0;
	m_edtStorageitem = 0;
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgSensorSampling::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgSensorSampling)
	DDX_Text(pDX, IDC_SENSORSAMPLING_EDT_DETECTIONPERIOD, m_edtDetectionPeriod);
	DDX_Text(pDX, IDC_SENSORSAMPLING_EDT_STORAGEITEM, m_edtStorageitem);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgSensorSampling, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgSensorSampling)
	ON_BN_CLICKED(IDC_SENSORSAMPLING_BTN_GET, OnSensorsamplingBtnGet)
	ON_BN_CLICKED(IDC_SENSORSAMPLING_BTN_SET, OnSensorsamplingBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorSampling message handlers

BOOL CDlgCfgSensorSampling::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this, DLG_CFG_SENSORSANMPLING);
	// TODO: Add extra initialization here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_CFG_SENSORSANMPLING), ConvertString("Prompt"));
		return FALSE;
	}
	
	if (getCfgFromDevice())
	{
		StuToDlg();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgSensorSampling::OnSensorsamplingBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (getCfgFromDevice())
	{
		StuToDlg();
	}
}

void CDlgCfgSensorSampling::OnSensorsamplingBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	setCfgToDevice();
}

void CDlgCfgSensorSampling::DlgToStu()
{
	UpdateData(TRUE);
	m_stuInfo.nDetectionPeriod = m_edtDetectionPeriod;
	m_stuInfo.nStorageItem = m_edtStorageitem;
}

void CDlgCfgSensorSampling::StuToDlg()
{
	m_edtDetectionPeriod = m_stuInfo.nDetectionPeriod;
	m_edtStorageitem = m_stuInfo.nStorageItem;
	UpdateData(FALSE);
}

BOOL CDlgCfgSensorSampling::getCfgFromDevice()
{
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_CFG_SENSORSANMPLING), ConvertString("Prompt"));
		return FALSE;
	}
	
	char szOutBuffer[1024] = {0};
	int nerror = 0;

	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_SENSORSAMPLING, -1, szOutBuffer, 1024, &nerror, SDK_API_WAIT);
	if (bRet)
	{
		DWORD dwRetLen = 0;		
		bRet = CLIENT_ParseData(CFG_CMD_SENSORSAMPLING, szOutBuffer, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse SensorSampling error..."), DLG_CFG_SENSORSANMPLING), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("get config SensorSampling error:", DLG_CFG_SENSORSANMPLING),CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgSensorSampling::setCfgToDevice()
{
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_CFG_SENSORSANMPLING), ConvertString("Prompt"));
		return FALSE;
	}
	
	char szJsonBuf[1024] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_SENSORSAMPLING, (void*)&m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet SensorSampling error..."), DLG_CFG_SENSORSANMPLING), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		bRet = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_SENSORSAMPLING, -1, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig SensorSampling failed:", DLG_CFG_SENSORSANMPLING), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
        else
        {    
			MessageBox(ConvertString(CString("SetConfig SensorSampling ok!"), DLG_CFG_SENSORSANMPLING), ConvertString("Prompt"));
        }
	}
	return TRUE;
}