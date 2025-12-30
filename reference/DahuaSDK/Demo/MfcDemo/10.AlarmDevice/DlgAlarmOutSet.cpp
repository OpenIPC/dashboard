// DlgAlarmOutSet.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgAlarmOutSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmOutSet dialog


CDlgAlarmOutSet::CDlgAlarmOutSet(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */, int nAlarmOut /* = 0 */)
	: CDialog(CDlgAlarmOutSet::IDD, pParent)
    , m_nAlarmOut(0)
{
	//{{AFX_DATA_INIT(CDlgAlarmOutSet)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginID;
    m_nAlarmOut = nAlarmOut;
}


void CDlgAlarmOutSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarmOutSet)
	DDX_Control(pDX, IDC_ALARMOUTSET_CHK_MODE, m_chkAction);
	DDX_Control(pDX, IDC_ALARMOUTSET_CMB_CHN, m_cmbChannel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAlarmOutSet, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarmOutSet)
	ON_BN_CLICKED(IDC_ALARMOUTSET_BTN_SET, OnAlarmoutsetBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmOutSet private method

void CDlgAlarmOutSet::InitDlg()
{
    for (int i = 0; i < m_nAlarmOut; i++)
    {
        CString csInfo;
        csInfo.Format("%d", i+1);
        m_cmbChannel.InsertString(-1, csInfo);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmOutSet message handlers

BOOL CDlgAlarmOutSet::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_ALARMOUTSET);
	// TODO: Add extra initialization here
	InitDlg();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAlarmOutSet::OnAlarmoutsetBtnSet() 
{
	// TODO: Add your control notification handler code here
    ALARMCTRL_PARAM stuParam = {sizeof(stuParam)};
    stuParam.nAction = m_chkAction.GetCheck();
    stuParam.nAlarmNo = m_cmbChannel.GetCurSel();
	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_TRIGGER_ALARM_OUT, &stuParam, SDK_API_WAIT);
    if (bRet)
    {
        MessageBox(ConvertString("set alarm out ok!", DLG_ALARMOUTSET), ConvertString("Prompt"));
    } 
    else
    {
        CString csInfo;
        csInfo.Format("%s:0x%08x", ConvertString("set alarm out failed", DLG_ALARMOUTSET), CLIENT_GetLastError());
        MessageBox(csInfo, ConvertString("Prompt"));
    }
}
