// DlgCfgSnapLinkage.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgSnapLinkage.h"
#include "SubDlgSensorInfoDescription.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSnapLinkage dialog


CDlgCfgSnapLinkage::CDlgCfgSnapLinkage(CWnd* pParent /* = NULL */, LLONG lLoginID /* = NULL */, UINT32 uiVideoIn /* = 0 */, UINT32 uiAnalogAlarmIn /* = 72 */)
	: CDialog(CDlgCfgSnapLinkage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgSnapLinkage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginID;
    m_uiVideoIn = uiVideoIn;
    m_uiAnalogAlarmIn = uiAnalogAlarmIn;
    memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgSnapLinkage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgSnapLinkage)
	DDX_Control(pDX, IDC_CFG_SNAPLINKAGE_CMB_CHN, m_cmbChn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgSnapLinkage, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgSnapLinkage)
	ON_CBN_SELCHANGE(IDC_CFG_SNAPLINKAGE_CMB_CHN, OnSelchangeCfgSnaplinkageCmbChn)
	ON_BN_CLICKED(IDC_CFG_SNAPLINKAGE_BTN_CHNS, OnCfgSnaplinkageBtnChns)
	ON_BN_CLICKED(IDC_CFG_SNAPLINKAGE_BTN_GET, OnCfgSnaplinkageBtnGet)
	ON_BN_CLICKED(IDC_CFG_SNAPLINKAGE_BTN_SET, OnCfgSnaplinkageBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
// Private method

void CDlgCfgSnapLinkage::InitDlg()
{
    m_uiVideoIn = m_uiVideoIn <= 0 ? 4 : m_uiVideoIn;

	// channel
	m_cmbChn.ResetContent();
	for (int i = 0; i < m_uiVideoIn; i++)
    {
        CString csItem;
        csItem.Format("%s %02d", ConvertString("Channel"), i + 1);
        m_cmbChn.InsertString(-1, csItem);
    }
	m_cmbChn.SetCurSel(0);
}

BOOL CDlgCfgSnapLinkage::SetConfigToDevice()
{	
	int nSel = m_cmbChn.GetCurSel();
	if (-1 == nSel)
	{
		return FALSE;
	}
	char szJsonBuf[1024 * 40] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_SNAPLIKAGE, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString("packet SnapLinkage error...", DLG_CFG_SNAPLINKAGE), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
		int nerror = 0;
		int nrestart = 0;
		
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_SNAPLIKAGE, nSel, szJsonBuf, strlen(szJsonBuf), &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig SnapLinkage failed:", DLG_CFG_SNAPLINKAGE), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString("SetConfig SnapLinkage ok!", DLG_CFG_SNAPLINKAGE), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgSnapLinkage::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 40] = {0};
	int nerror = 0;
	int nSelChn = m_cmbChn.GetCurSel() == -1 ? 0 : m_cmbChn.GetCurSel();
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_SNAPLIKAGE, nSelChn, szJsonBuf, 
		sizeof(szJsonBuf), &nerror, SDK_API_WAIT);
	
	if (bRet)
	{
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_SNAPLIKAGE, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString("parse SnapLinkage error...", DLG_CFG_SNAPLINKAGE), ConvertString("Prompt"));
			return FALSE;
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", 
			ConvertString("QueryConfig SnapLinkage error:", DLG_CFG_SNAPLINKAGE), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgSnapLinkage::DlgToStu()
{
}

void CDlgCfgSnapLinkage::StuToDlg()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSnapLinkage message handlers

BOOL CDlgCfgSnapLinkage::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_SNAPLINKAGE);
	// TODO: Add extra initialization here
	InitDlg();
    OnCfgSnaplinkageBtnGet();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgSnapLinkage::OnSelchangeCfgSnaplinkageCmbChn() 
{
    // TODO: Add your control notification handler code here
    int nSel = m_cmbChn.GetCurSel();
    if (-1 == nSel)
    {
        return;
    }
    if (GetConfigFromDevice())
    {
        StuToDlg();
	}	
}

void CDlgCfgSnapLinkage::OnCfgSnaplinkageBtnChns() 
{
	// TODO: Add your control notification handler code here
    std::vector<int> vecChn;
    int i = 0;
    for (; i < m_stuInfo.nChannelNum; i++)
    {
        vecChn.push_back(m_stuInfo.anAnalogAlarm[i]);
    }
    
    CSubDlgSensorInfoDescription dlg(this, m_uiAnalogAlarmIn);
    dlg.SetID(vecChn);
    if (IDOK == dlg.DoModal())
    {
        vecChn.clear();
        vecChn = dlg.GetID();
        std::vector<int>::iterator it = vecChn.begin();
        for (i = 0; i < __min(vecChn.size(), DH_MAX_ZONE_NUM) && it != vecChn.end(); i++, it++)
        {
            m_stuInfo.anAnalogAlarm[i] = *it;
        }
        m_stuInfo.nChannelNum = __min(vecChn.size(), DH_MAX_ZONE_NUM);
	}
}

void CDlgCfgSnapLinkage::OnCfgSnaplinkageBtnGet() 
{
    // TODO: Add your control notification handler code here
    if (GetConfigFromDevice())
    {
        StuToDlg();
	}	
}

void CDlgCfgSnapLinkage::OnCfgSnaplinkageBtnSet() 
{
    // TODO: Add your control notification handler code here
    DlgToStu();
	SetConfigToDevice();		
}
