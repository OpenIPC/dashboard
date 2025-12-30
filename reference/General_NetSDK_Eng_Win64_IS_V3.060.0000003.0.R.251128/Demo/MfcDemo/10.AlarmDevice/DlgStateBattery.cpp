// DlgStateBattery.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgStateBattery.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgStateBattery dialog


CDlgStateBattery::CDlgStateBattery(CWnd* pParent /*=NULL*/, LLONG hLoginID)
	: CDialog(CDlgStateBattery::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgStateBattery)
	m_csPercent = _T("");
	m_hLoginID = hLoginID;
	m_nBatteryNum = 0;

	memset(m_bPowerState, 0, sizeof(m_bPowerState));
	memset(m_bBatteryState, 0, sizeof(m_bBatteryState));
	memset(m_nBatteryPercent, 0, sizeof(m_nBatteryPercent));	
	m_nPowerNum = 0;
	m_nBatteryNum = 0;
	//}}AFX_DATA_INIT
}


void CDlgStateBattery::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgStateBattery)
	DDX_Control(pDX, IDC_CHK_CHARGE, m_chkIsCharging);
	DDX_Control(pDX, IDC_PGS_PERCENT, m_prgPercent);
	DDX_Control(pDX, IDC_CMB_POWER_INDEX, m_cbPowerIndex);
	DDX_Control(pDX, IDC_CMB_BATTERY_INDEX, m_cbBatteryIndex);
	DDX_Control(pDX, IDC_CMB_STATE, m_cbState);
	DDX_Text(pDX, IDC_BATTERY_EDT_PERCENT, m_csPercent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgStateBattery, CDialog)
	//{{AFX_MSG_MAP(CDlgStateBattery)
	ON_BN_CLICKED(IDC_BATTERY_BTN_GET, OnBtnGet)
	ON_CBN_SELCHANGE(IDC_CMB_BATTERY_INDEX, OnSelchangeCmbBatteryIndex)
	ON_CBN_SELCHANGE(IDC_CMB_POWER_INDEX, OnSelchangeCmbPowerIndex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgStateBattery message handlers

BOOL CDlgStateBattery::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_STATE_BATTERY);
	
	if (!m_hLoginID)
	{
		MessageBox(ConvertString("we haven't login a device yet!", DLG_STATE_BATTERY), ConvertString("Prompt"));
		return FALSE;
	}

	// TODO: Add extra initialization here
	m_nPowerNum = 0;
	m_nBatteryNum = 0;
	memset(m_bPowerState, 0, sizeof(m_bPowerState));
	memset(m_bBatteryState, 0, sizeof(m_bBatteryState));
	memset(m_nBatteryPercent, 0, sizeof(m_nBatteryPercent));

	m_cbState.InsertString(-1, ConvertString("Off", DLG_STATE_BATTERY));
	m_cbState.InsertString(-1, ConvertString("On", DLG_STATE_BATTERY));
	m_cbState.SetCurSel(0);
	m_cbState.EnableWindow(FALSE);

	m_chkIsCharging.EnableWindow(FALSE);

	BOOL bRet = GetPowerState();
	if (!bRet)
	{
		MessageBox(ConvertString("Query power state failed.", DLG_STATE_BATTERY), ConvertString("Prompt"));
	}

	UpdateData(FALSE);
	return TRUE;  
	// return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgStateBattery::OnBtnGet() 
{
	// TODO: Add your control notification handler code here
	BOOL bRet = GetPowerState();
	if (!bRet)
	{
		MessageBox(ConvertString("Query power state failed.", DLG_STATE_BATTERY), ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Query power state successfully.", DLG_STATE_BATTERY), ConvertString("Prompt"));
	}
}

void CDlgStateBattery::OnSelchangeCmbBatteryIndex() 
{
	// TODO: Add your control notification handler code here
	int nIndex = m_cbBatteryIndex.GetCurSel();
	if (nIndex >= 0 && nIndex < 16)
	{
		m_chkIsCharging.SetCheck(m_bBatteryState[nIndex]);
		if (m_nBatteryPercent[nIndex]<=100 && m_nBatteryPercent[nIndex]>=0)
		{
			m_prgPercent.SetPos(m_nBatteryPercent[nIndex]);
			m_csPercent.Empty();
			m_csPercent.Format("%d", m_nBatteryPercent[nIndex]);
		}
	}
	UpdateData(TRUE);

}

void CDlgStateBattery::OnSelchangeCmbPowerIndex() 
{
	// TODO: Add your control notification handler code here
	int nIndex = m_cbPowerIndex.GetCurSel();
	if (nIndex >= 0 && nIndex < 16)
	{
		m_cbState.SetCurSel(m_bPowerState[nIndex]);
	}
}

BOOL CDlgStateBattery::GetPowerState()
{
	int nRetLen = 0;
	DH_POWER_STATUS stuPowerStatus = { sizeof(DH_POWER_STATUS) };
	BOOL bRet = CLIENT_QueryDevState((LLONG)m_hLoginID, DH_DEVSTATE_POWER_STATE, (char*)&stuPowerStatus, sizeof(DH_POWER_STATUS), &nRetLen, SDK_API_WAIT);
	if (bRet)
	{
		m_nPowerNum = stuPowerStatus.nCount;
		m_nBatteryNum = stuPowerStatus.nBatteryNum;
		InitPower(m_nPowerNum, &stuPowerStatus);
		InitBattery(m_nBatteryNum, &stuPowerStatus);
	}
	else
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Query power state failed:", DLG_STATE_BATTERY), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
	return bRet;
}

void CDlgStateBattery::InitPower(INT nNum, DH_POWER_STATUS *pstuInpara)
{
	if (nNum > 0 && nNum <= DH_MAX_POWER_NUM)
	{
		m_cbPowerIndex.ResetContent();
		for (int i=0; i<nNum; i++)
		{
			CString csStr;
			csStr.Format("%s %d", ConvertString("Power", DLG_STATE_BATTERY), i+1);
			m_cbPowerIndex.InsertString(-1, csStr);
			m_bPowerState[i] = pstuInpara->stuPowers[i].bPowerOn;
		}
		m_cbPowerIndex.SetCurSel(0);
		m_cbState.SetCurSel(m_bPowerState[0]);
	}
}

void CDlgStateBattery::InitBattery(INT nNum, DH_POWER_STATUS *pstuInpara)
{
	if (nNum >0 && nNum <= DH_MAX_BATTERY_NUM)
	{
		m_cbBatteryIndex.ResetContent();
		for (int i=0; i<nNum; i++)
		{
			CString csStr;
			csStr.Format("%s %d",ConvertString("Battery", DLG_STATE_BATTERY), i+1);
			m_cbBatteryIndex.InsertString(-1, csStr);
			m_bBatteryState[i] = pstuInpara->stuBatteries[i].bCharging;
			m_nBatteryPercent[i] = pstuInpara->stuBatteries[i].nPercent;
		}
		m_cbBatteryIndex.SetCurSel(0);
		m_prgPercent.SetPos(m_nBatteryPercent[0]);
		m_csPercent.Empty();
		m_csPercent.Format("%d", m_nBatteryPercent[0]);
		m_chkIsCharging.SetCheck(m_bBatteryState[0]);
	}
}


