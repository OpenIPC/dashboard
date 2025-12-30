// DlgCfgSensorInfoOfVW.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgSensorInfoOfVW.h"
#include "SubDlgSensorInfoDescription.h"
#include <VECTOR>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorInfoOfVW dialog


CDlgCfgSensorInfoOfVW::CDlgCfgSensorInfoOfVW(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */, int nChannel /* = 4 */)
	: CDialog(CDlgCfgSensorInfoOfVW::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgSensorInfoOfVW)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginID = lLoginID;
	m_nChannel = nChannel;
	memset(&m_stuVW, 0, sizeof(m_stuVW));
	m_stuVW.nStructSize = sizeof(m_stuVW);
	// only "stuSensorInfo" part is usfull for AE6016
	for (int i = 0; i < AV_CFG_Max_Video_Widget_Sensor_Info; i++)
	{
		m_stuVW.stuSensorInfo[i].nStructSize = sizeof(AV_CFG_VideoWidgetSensorInfo);
		m_stuVW.stuSensorInfo[i].stuRect.nStructSize = sizeof(AV_CFG_Rect);
		for (int j = 0; j < AV_CFG_Max_Description_Num; j++)
		{
			m_stuVW.stuSensorInfo[i].stuDescription[j].nStructSize = sizeof(AV_CFG_VideoWidgetSensorInfo_Description);
		}
	}
}


void CDlgCfgSensorInfoOfVW::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgSensorInfoOfVW)
	DDX_Control(pDX, IDC_SENSORINFO_CHK_PREVIEWBLEND, m_chkPreviewBlend);
	DDX_Control(pDX, IDC_SENSORINFO_CHK_ENCODEBLEND, m_chkEncodeBlend);
	DDX_Control(pDX, IDC_SENSORINFO_CMB_CHN, m_cmbChn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgSensorInfoOfVW, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgSensorInfoOfVW)
	ON_CBN_SELCHANGE(IDC_SENSORINFO_CMB_CHN, OnSelchangeSensorinfoCmbChn)
	ON_BN_CLICKED(IDC_SENSORINFO_BTN_DESCRIPTION, OnSensorinfoBtnDescription)
	ON_BN_CLICKED(IDC_SENSORINFO_BTN_GET, OnSensorinfoBtnGet)
	ON_BN_CLICKED(IDC_SENSORINFO_BTN_SET, OnSensorinfoBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorInfoOfVW private method

void CDlgCfgSensorInfoOfVW::InitDlg()
{
	int i = 0;
	for (; i < m_nChannel; i++)
	{
		CString csInfo;
		csInfo.Format("%s%02d", ConvertString("Channel", DLG_CFG_SENSORINFO), i + 1);
		m_cmbChn.InsertString(-1, csInfo);
	}
	m_cmbChn.SetCurSel(0);

	// Preview blend
	m_chkPreviewBlend.SetCheck(BST_UNCHECKED);

	// Encode blend
	m_chkEncodeBlend.SetCheck(BST_UNCHECKED);

	// Zone
	SetDlgItemInt(IDC_SENSORINFO_EDT_LTX, 0, FALSE);
	SetDlgItemInt(IDC_SENSORINFO_EDT_LTY, 0, FALSE);
	SetDlgItemInt(IDC_SENSORINFO_EDT_RBX, 0, FALSE);
	SetDlgItemInt(IDC_SENSORINFO_EDT_RBY, 0, FALSE);

	// Description...
}

void CDlgCfgSensorInfoOfVW::DlgToStu()
{
	// only support the first one
	AV_CFG_VideoWidgetSensorInfo& stuInfo = m_stuVW.stuSensorInfo[0];

	// Preview blend
	if (m_chkPreviewBlend.GetCheck())
	{
		stuInfo.bPreviewBlend = TRUE;
	}
	else
	{
		stuInfo.bPreviewBlend = FALSE;
	}

	// Encode blend
	if (m_chkEncodeBlend.GetCheck())
	{
		stuInfo.bEncodeBlend = TRUE;
	}
	else
	{
		stuInfo.bEncodeBlend = FALSE;
	}

	// Zone
	stuInfo.stuRect.nLeft = GetDlgItemInt(IDC_SENSORINFO_EDT_LTX);
	stuInfo.stuRect.nTop = GetDlgItemInt(IDC_SENSORINFO_EDT_LTY);
	stuInfo.stuRect.nRight = GetDlgItemInt(IDC_SENSORINFO_EDT_RBX);
	stuInfo.stuRect.nBottom = GetDlgItemInt(IDC_SENSORINFO_EDT_RBY);

	// Description...
}

void CDlgCfgSensorInfoOfVW::StuToDlg()
{
	const AV_CFG_VideoWidgetSensorInfo& stuInfo = m_stuVW.stuSensorInfo[0];

	// Preview blend
	if (stuInfo.bPreviewBlend)
	{
		m_chkPreviewBlend.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkPreviewBlend.SetCheck(BST_UNCHECKED);
	}
	
	// Encode blend
	if (stuInfo.bEncodeBlend)
	{
		m_chkEncodeBlend.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkEncodeBlend.SetCheck(BST_UNCHECKED);
	}
	
	// Zone
	SetDlgItemInt(IDC_SENSORINFO_EDT_LTX, stuInfo.stuRect.nLeft, FALSE);
	SetDlgItemInt(IDC_SENSORINFO_EDT_LTY, stuInfo.stuRect.nTop, FALSE);
	SetDlgItemInt(IDC_SENSORINFO_EDT_RBX, stuInfo.stuRect.nRight, FALSE);
	SetDlgItemInt(IDC_SENSORINFO_EDT_RBY, stuInfo.stuRect.nBottom, FALSE);	
	
	// Description...
}

BOOL CDlgCfgSensorInfoOfVW::GetConfig()
{
	int nChn = m_cmbChn.GetCurSel();
	if (-1 == nChn)
	{
		return FALSE;
	}
	char szBuf[1024*10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_VIDEOWIDGET, nChn, szBuf, sizeof(szBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		CString csInfo;
		csInfo.Format("%s:%08x", ConvertString("Get config of VideoWidget error", DLG_CFG_SENSORINFO), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}

	int nRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_VIDEOWIDGET, szBuf, &m_stuVW, sizeof(m_stuVW), &nRetLen);
	if (!bRet || nRetLen != sizeof(m_stuVW))
	{
		MessageBox(ConvertString("Parse config of VideoWidget error", DLG_CFG_SENSORINFO), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgSensorInfoOfVW::SetConfig()
{
	int nChn = m_cmbChn.GetCurSel();
	if (-1 == nChn)
	{
		return FALSE;
	}
	char szBuf[1024*10] = {0};
	int nErr = 0;
	int nRestart = 0;
	BOOL bRet = CLIENT_PacketData(CFG_CMD_VIDEOWIDGET, &m_stuVW, sizeof(m_stuVW), szBuf, sizeof(szBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("Packet config of VideoWidget error", DLG_CFG_SENSORINFO), ConvertString("Prompt"));
		return FALSE;
	}
	bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_VIDEOWIDGET, nChn, szBuf, strlen(szBuf), &nErr, &nRestart, SDK_API_WAIT);
	if (!bRet)
	{
		CString csInfo;
		csInfo.Format("%s:%08x", ConvertString("Set config of VideoWidget error", DLG_CFG_SENSORINFO), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}
	else
	{
		MessageBox(ConvertString("Set config of VideoWidget ok", DLG_CFG_SENSORINFO), ConvertString("Prompt"));
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSensorInfoOfVW message handlers

BOOL CDlgCfgSensorInfoOfVW::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_SENSORINFO);
	// TODO: Add extra initialization here
	InitDlg();
	if (GetConfig())
	{
		StuToDlg();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgSensorInfoOfVW::OnSelchangeSensorinfoCmbChn() 
{
	// TODO: Add your control notification handler code here
	int nChn = m_cmbChn.GetCurSel();
	if (-1 == nChn)
	{
		return;
	}
	if (GetConfig())
	{
		StuToDlg();
	}	
}

void CDlgCfgSensorInfoOfVW::OnSensorinfoBtnDescription() 
{
	// TODO: Add your control notification handler code here
	std::vector<int> vecChn;
	int i = 0;
	for (; i < m_stuVW.stuSensorInfo[0].nDescriptionNum; i++)
	{
		vecChn.push_back(m_stuVW.stuSensorInfo[0].stuDescription[i].nSensorID);
	}

	CSubDlgSensorInfoDescription dlg;
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		vecChn.clear();
		vecChn = dlg.GetID();
		std::vector<int>::iterator it = vecChn.begin();
		for (i = 0; i < __min(vecChn.size(), AV_CFG_Max_Description_Num) && it != vecChn.end(); i++, it++)
		{
			m_stuVW.stuSensorInfo[0].stuDescription[i].nSensorID = *it;
		}
		m_stuVW.stuSensorInfo[0].nDescriptionNum = __min(vecChn.size(), AV_CFG_Max_Description_Num);
	}
}

void CDlgCfgSensorInfoOfVW::OnSensorinfoBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfig())
	{
		StuToDlg();
	}	
}

void CDlgCfgSensorInfoOfVW::OnSensorinfoBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfig();
}
