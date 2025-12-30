// DlgSetArmMode.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgSetArmMode.h"
#include "SubDlgSensorInfoDescription.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetArmMode dialog


CDlgSetArmMode::CDlgSetArmMode(CWnd* pParent /*=NULL*/, LLONG lLoginId /*=0*/)
	: CDialog(CDlgSetArmMode::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSetArmMode)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
    memset(&m_stuOut, 0, sizeof(m_stuOut));
    m_stuOut.dwSize = sizeof(m_stuOut);
}


void CDlgSetArmMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetArmMode)
	DDX_Control(pDX, IDC_SETARMMODE_COMBO_SCENE, m_cmbScene);
	DDX_Control(pDX, IDC_SETARMMODE_COMBO_MODE, m_cmbMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSetArmMode, CDialog)
	//{{AFX_MSG_MAP(CDlgSetArmMode)
	ON_BN_CLICKED(IDC_SETARMMODE_BTN_SET, OnSetarmmodeBtnSet)
	ON_BN_CLICKED(IDC_SETARMMODE_BTN_ALARMSOURCE, OnSetarmmodeBtnAlarmSource)
	ON_BN_CLICKED(IDC_SETARMMODE_BTN_ALARMLINK, OnSetarmmodeBtnAlarmLink)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetArmMode message handlers

BOOL CDlgSetArmMode::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	g_SetWndStaticText(this, DLG_SETAMRMODE);
	
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_SETAMRMODE), ConvertString("Prompt"));
		return TRUE;
	}
	
	m_cmbMode.ResetContent();
	m_cmbMode.InsertString(-1, ConvertString("Unknown", DLG_SETAMRMODE));
	m_cmbMode.InsertString(-1, ConvertString("Disarming", DLG_SETAMRMODE));
	m_cmbMode.InsertString(-1, ConvertString("Arming", DLG_SETAMRMODE));
    m_cmbMode.InsertString(-1, ConvertString("ForceArming", DLG_SETAMRMODE));
	m_cmbMode.SetCurSel(0);

	m_cmbScene.ResetContent();
	m_cmbScene.InsertString(-1, ConvertString("Unknown", DLG_SETAMRMODE));
	m_cmbScene.InsertString(-1, ConvertString("Outdoor", DLG_SETAMRMODE));
	m_cmbScene.InsertString(-1, ConvertString("Indoor", DLG_SETAMRMODE));
    m_cmbScene.InsertString(-1, ConvertString("Whole", DLG_SETAMRMODE));
	m_cmbScene.InsertString(-1, ConvertString("RightNow", DLG_SETAMRMODE));
	m_cmbScene.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
void CDlgSetArmMode::OnSetarmmodeBtnSet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_SETAMRMODE), ConvertString("Prompt"));
		return;
	}

	CString csPsw;
	GetDlgItemText(IDC_SETARMMODE_EDT_PASSWORD, csPsw);
	
	CTRL_ARM_DISARM_PARAM_EX stuParam = {sizeof(stuParam)};
    stuParam.stuIn.dwSize = sizeof(stuParam.stuIn);
    stuParam.stuOut.dwSize = sizeof(stuParam.stuOut);
	
	stuParam.stuIn.emState = (NET_ALARM_MODE)(m_cmbMode.GetCurSel() - 1);
	stuParam.stuIn.emSceneMode = (NET_SCENE_MODE)m_cmbScene.GetCurSel();	
	stuParam.stuIn.szDevPwd = csPsw.GetBuffer(0);
	BOOL bRet = CLIENT_ControlDevice(m_lLoginId, DH_CTRL_ARMED_EX, &stuParam, SDK_API_WAIT);
	if (bRet)
    {
        MessageBox(ConvertString("Set Arm Mode successfully.", DLG_SETAMRMODE), ConvertString("Prompt"));
        m_stuOut.dwSourceNum = 0;
        m_stuOut.dwLinkNum = 0;
	}
    else
    {
        CString csOut;
        csOut.Format("%s %08x", ConvertString("Set Arm Mode failed:", DLG_SETAMRMODE), CLIENT_GetLastError());
        MessageBox(csOut, ConvertString("Prompt"));

        int i = 0;
        
        m_stuOut.dwSourceNum = stuParam.stuOut.dwSourceNum;
        for (; i < sizeof(m_stuOut.nSource)/sizeof(m_stuOut.nSource[0]); i++)
        {
            m_stuOut.nSource[i] = stuParam.stuOut.nSource[i];
        }
        
        m_stuOut.dwLinkNum = stuParam.stuOut.dwLinkNum;
        for (i = 0; i < sizeof(m_stuOut.nLink)/sizeof(m_stuOut.nLink[0]); i++)
        {
            m_stuOut.nLink[i] = stuParam.stuOut.nLink[i];
        }
    }
}

void CDlgSetArmMode::OnSetarmmodeBtnAlarmSource() 
{
	// TODO: Add your control notification handler code here
    std::vector<int> vecChn;
    int i = 0;
    for (; i < m_stuOut.dwSourceNum; i++)
    {
        vecChn.push_back(m_stuOut.nSource[i]);
    }
    
    CSubDlgSensorInfoDescription dlg(this, ARM_DISARM_ZONE_MAX);
	dlg.SetID(vecChn);
    dlg.DoModal();
}

void CDlgSetArmMode::OnSetarmmodeBtnAlarmLink() 
{
	// TODO: Add your control notification handler code here    
    std::vector<int> vecChn;
    int i = 0;
    for (; i < m_stuOut.dwLinkNum; i++)
    {
        vecChn.push_back(m_stuOut.nLink[i]);
    }
    
    CSubDlgSensorInfoDescription dlg(this, ARM_DISARM_ZONE_MAX);
    dlg.SetID(vecChn);
    dlg.DoModal();
}
