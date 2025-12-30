// RuleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "RuleDlg.h"
#include "GlobalDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRuleDlg dialog


CRuleDlg::CRuleDlg(CWnd* pParent /*=NULL*/,LLONG lLoginId, int nChannel)
	: CDialog(CRuleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRuleDlg)
	m_nRuleAmount = 0;
	m_nPresetNum = 0;
	m_nRule = 0;
	m_szCustomName = _T("");
	m_nCooddinatecount = 0;
	m_nX = 0;
	m_nY = 0;
	m_nPeriod = 0;
	m_nAlarmid = 0;
	m_fHysteresis = 0.0f;
	m_fThreshold = 0.0f;
	m_nDrationTime = 0;
	m_fObjectEmissivity = 0.0f;
	m_nObjectDistance = 0;
	m_nRefalectedTemp = 0;
	m_nAlarmAmount = 0;
	//}}AFX_DATA_INIT
    m_iLoginID = lLoginId;
    m_nChannel = nChannel;
}


void CRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRuleDlg)
	DDX_Control(pDX, IDC_COMBO_ALARMID, m_Combo_AlarmID);
	DDX_Control(pDX, IDC_COMBO_LOCALCONFIG, m_Combo_LocalConfig);
	DDX_Control(pDX, IDC_COMBO_RULEID, m_Combo_Ruleid);
	DDX_Control(pDX, IDC_COMBO_ALARMENABLE, m_Combo_AlarmEnable);
	DDX_Control(pDX, IDC_COMBO_CONDITION, m_Combo_Condition);
	DDX_Control(pDX, IDC_COMBO_RESULTTYPE, m_Combo_ResultType);
	DDX_Control(pDX, IDC_COMBO_POINTNUM, m_Combo_PointNum);
	DDX_Control(pDX, IDC_COMBO_MODETYPE, m_Combo_ModeType);
	DDX_Control(pDX, IDC_COMBO_ENABLE, m_ComboEnable);
	DDX_Text(pDX, IDC_EDIT_RULEAMOUNT, m_nRuleAmount);
	DDX_Text(pDX, IDC_EDIT_PRESETNUM, m_nPresetNum);
	DDX_Text(pDX, IDC_EDIT_RULENUM, m_nRule);
	DDX_Text(pDX, IDC_EDIT_CUSTOMNAME, m_szCustomName);
	DDX_Text(pDX, IDC_EDIT_COORDINATECOUNT, m_nCooddinatecount);
	DDX_Text(pDX, IDC_EDIT_X, m_nX);
	DDX_Text(pDX, IDC_EDIT_Y, m_nY);
	DDX_Text(pDX, IDC_EDIT_PERIOD, m_nPeriod);
	DDX_Text(pDX, IDC_EDIT_ALARMID, m_nAlarmid);
	DDX_Text(pDX, IDC_EDIT_HYSTERESIS, m_fHysteresis);
	DDX_Text(pDX, IDC_EDIT_THRESHOLD, m_fThreshold);
	DDX_Text(pDX, IDC_EDIT_DURATION, m_nDrationTime);
	DDX_Text(pDX, IDC_EDIT_COEFFICIENT, m_fObjectEmissivity);
	DDX_Text(pDX, IDC_EDIT_DISTANCE, m_nObjectDistance);
	DDX_Text(pDX, IDC_EDIT_REFALECTED, m_nRefalectedTemp);
	DDX_Text(pDX, IDC_EDIT_ALARMAMOUNT, m_nAlarmAmount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRuleDlg, CDialog)
	//{{AFX_MSG_MAP(CRuleDlg)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	ON_CBN_SELCHANGE(IDC_COMBO_RULEID, OnSelchangeComboRuleid)
	ON_CBN_SELCHANGE(IDC_COMBO_POINTNUM, OnSelchangeComboPointnum)
	ON_CBN_SELCHANGE(IDC_COMBO_ALARMID, OnSelchangeComboAlarmid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRuleDlg message handlers

BOOL CRuleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    GetRuleConfig(0,0,0);
    UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CRuleDlg::GetRuleConfig(int RuleID,int pointID,int AlarmID)
{	
    int nBufferLen = 40*512;
    CFG_RADIOMETRY_RULE_INFO *stuRuleOutInfo = new CFG_RADIOMETRY_RULE_INFO;
	if (NULL == stuRuleOutInfo)
	{
		return FALSE;
	}

    memset(stuRuleOutInfo,0,sizeof(CFG_RADIOMETRY_RULE_INFO));
    char szJsonBuf[40*512] = {0};    
    int nerror = 0;
    BOOL bRet = FALSE;
	bRet = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_THERMOMETRY_RULE,m_nChannel,szJsonBuf,nBufferLen,&nerror,3000);
    if (0 != bRet)
    {
        DWORD dwRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_THERMOMETRY_RULE,szJsonBuf,(char*)stuRuleOutInfo,sizeof(CFG_RADIOMETRY_RULE_INFO),&dwRetLen);
        if (!bRet)
        {
			bRet = FALSE;
            MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
			goto e_clearup;            
        }
        else
        {
            if (RuleID>stuRuleOutInfo->nCount)
            {
				bRet = FALSE;
				goto e_clearup;                
            }
            else
            {
                int index = 0;
                CString str;
                m_nRuleAmount = stuRuleOutInfo->nCount;
                m_Combo_Ruleid.ResetContent();
				int i = 0;
                for (i = 0;i<m_nRuleAmount;++i)
                {
                    str.Format("%d",i);
                    index = m_Combo_Ruleid.AddString(str);
                    m_Combo_Ruleid.SetItemData(index,i);
                }
                m_Combo_Ruleid.SetCurSel(0);
                m_ComboEnable.SetCurSel(stuRuleOutInfo->stRule[RuleID].bEnable);
                m_nPresetNum = stuRuleOutInfo->stRule[RuleID].nPresetId;
                m_nRule = stuRuleOutInfo->stRule[RuleID].nRuleId;
                m_szCustomName = stuRuleOutInfo->stRule[RuleID].szName;
                m_Combo_ModeType.SetCurSel(stuRuleOutInfo->stRule[RuleID].nMeterType);
                m_nPeriod = stuRuleOutInfo->stRule[RuleID].nSamplePeriod;
                m_nCooddinatecount = stuRuleOutInfo->stRule[RuleID].nCoordinateCnt;
                m_Combo_LocalConfig.SetCurSel(stuRuleOutInfo->stRule[RuleID].stLocalParameters.bEnable);
                m_fObjectEmissivity = stuRuleOutInfo->stRule[RuleID].stLocalParameters.fObjectEmissivity;
                m_nObjectDistance = stuRuleOutInfo->stRule[RuleID].stLocalParameters.nObjectDistance;
                m_nRefalectedTemp = stuRuleOutInfo->stRule[RuleID].stLocalParameters.nRefalectedTemp;
                m_Combo_PointNum.ResetContent();
                for(i = 0;i<m_nCooddinatecount;++i)
                {
                    index = 0;
                    str.Format("%d",i);
                    index = m_Combo_PointNum.AddString(str);
                    m_Combo_PointNum.SetItemData(index,i);
                }
                m_Combo_PointNum.SetCurSel(0);
                m_nX = stuRuleOutInfo->stRule[RuleID].stCoordinates[pointID].nX;
                m_nY = stuRuleOutInfo->stRule[RuleID].stCoordinates[pointID].nY;
                m_nAlarmAmount = stuRuleOutInfo->stRule[RuleID].nAlarmSettingCnt;
                m_Combo_AlarmID.ResetContent();
                for(i = 0;i<m_nAlarmAmount;++i)
                {
                    index = 0;
                    str.Format("%d",i);
                    index = m_Combo_AlarmID.AddString(str);
                    m_Combo_AlarmID.SetItemData(index,i);
                }
                m_Combo_AlarmID.SetCurSel(0);
                m_nAlarmid = stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].nId;  
                m_Combo_AlarmEnable.SetCurSel(stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].bEnable);
                m_Combo_ResultType.SetCurSel(stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].nResultType);
                m_Combo_Condition.SetCurSel(stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].nAlarmCondition);
                m_fThreshold = stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].fThreshold;
                m_fHysteresis = stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].fHysteresis;
                m_nDrationTime = stuRuleOutInfo->stRule[RuleID].stAlarmSetting[AlarmID].nDuration;

				bRet = TRUE;
            }
        }
    } 
    else
    {
        MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
		bRet = FALSE;
		goto e_clearup;      
    }

e_clearup:
    if (stuRuleOutInfo != NULL)
    {
        delete stuRuleOutInfo;
        stuRuleOutInfo = NULL;
    }
    return bRet;
}

BOOL CRuleDlg::SetRuleConfig()
{
    UpdateData();
    CFG_RADIOMETRY_RULE_INFO *stuRuleOutInfo = new CFG_RADIOMETRY_RULE_INFO;
	if(NULL == stuRuleOutInfo)
	{
		return FALSE;
	}

    memset(stuRuleOutInfo,0,sizeof(*stuRuleOutInfo));
    stuRuleOutInfo->nCount = m_nRuleAmount;
    int i = m_Combo_Ruleid.GetCurSel();
    int j = m_Combo_PointNum.GetCurSel();
    int l = m_Combo_AlarmID.GetCurSel();
    stuRuleOutInfo->stRule[i].bEnable = m_ComboEnable.GetCurSel();
    stuRuleOutInfo->stRule[i].nPresetId = m_nPresetNum;
    stuRuleOutInfo->stRule[i].nRuleId = m_nRule;
    strncpy(stuRuleOutInfo->stRule[i].szName,(LPCTSTR)m_szCustomName,sizeof(stuRuleOutInfo->stRule[i].szName) - 1);
    stuRuleOutInfo->stRule[i].nMeterType = m_Combo_ModeType.GetCurSel();
    stuRuleOutInfo->stRule[i].nSamplePeriod = m_nPeriod;
    stuRuleOutInfo->stRule[i].nCoordinateCnt = m_nCooddinatecount;
    stuRuleOutInfo->stRule[i].stLocalParameters.bEnable = m_Combo_LocalConfig.GetCurSel();
    stuRuleOutInfo->stRule[i].stLocalParameters.fObjectEmissivity = m_fObjectEmissivity;
    stuRuleOutInfo->stRule[i].stLocalParameters.nObjectDistance = m_nObjectDistance;
    stuRuleOutInfo->stRule[i].stLocalParameters.nRefalectedTemp =  m_nRefalectedTemp;
    stuRuleOutInfo->stRule[i].stCoordinates[j].nX = m_nX;
    stuRuleOutInfo->stRule[i].stCoordinates[j].nY = m_nY;
    m_nAlarmid = stuRuleOutInfo->stRule[i].stAlarmSetting[l].nId = m_nAlarmid;  
    stuRuleOutInfo->stRule[i].stAlarmSetting[l].bEnable = m_Combo_AlarmEnable.GetCurSel();
    stuRuleOutInfo->stRule[i].stAlarmSetting[l].nResultType = m_Combo_ResultType.GetCurSel();
    stuRuleOutInfo->stRule[i].stAlarmSetting[l].nAlarmCondition = m_Combo_Condition.GetCurSel();
    stuRuleOutInfo->stRule[i].stAlarmSetting[l].fThreshold = m_fThreshold;
    stuRuleOutInfo->stRule[i].stAlarmSetting[l].fHysteresis = m_fHysteresis;
    stuRuleOutInfo->stRule[i].stAlarmSetting[l].nDuration = m_nDrationTime;
    char szJsonBuf[512 * 40] = {0};
    BOOL bRet = CLIENT_PacketData(CFG_CMD_THERMOMETRY, stuRuleOutInfo, sizeof(*stuRuleOutInfo), szJsonBuf, sizeof(szJsonBuf));
    if (!bRet)
    {
        DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
    } 
    else
    {
        int nerror = 0;
        int nrestart = 0;
        int nChannelID = -1;
        bRet = CLIENT_SetNewDevConfig(m_iLoginID, CFG_CMD_THERMOMETRY, nChannelID, szJsonBuf, sizeof(szJsonBuf), &nerror, &nrestart, 3000);
        if (!bRet)
        {
            DWORD dwError = CLIENT_GetLastError() & 0x7fffffff;
        }
        
    }
    UpdateData(FALSE);
	delete stuRuleOutInfo;
	stuRuleOutInfo = NULL;
    return FALSE;
}



void CRuleDlg::OnBtnGet() 
{
    
	GetRuleConfig(0,0,0);
    UpdateData(FALSE);
}

void CRuleDlg::OnBtnSet() 
{
	SetRuleConfig();
}

void CRuleDlg::OnSelchangeComboRuleid() 
{
    UpdateData();
	int i = m_Combo_Ruleid.GetCurSel();
    int j = m_Combo_PointNum.GetCurSel();
    int l = m_Combo_AlarmID.GetCurSel();
    GetRuleConfig(i,j,l);
    m_Combo_Ruleid.SetCurSel(i);
    m_Combo_PointNum.SetCurSel(j);
    m_Combo_AlarmID.SetCurSel(l);
    UpdateData(FALSE);
}

void CRuleDlg::OnSelchangeComboPointnum() 
{
    UpdateData();
    int i = m_Combo_Ruleid.GetCurSel();
    int j = m_Combo_PointNum.GetCurSel();
    int l = m_Combo_AlarmID.GetCurSel();
    GetRuleConfig(i,j,l);
    m_Combo_Ruleid.SetCurSel(i);
    m_Combo_PointNum.SetCurSel(j);
    m_Combo_AlarmID.SetCurSel(l);
    UpdateData(FALSE);
}

void CRuleDlg::OnSelchangeComboAlarmid() 
{
    UpdateData();
    int i = m_Combo_Ruleid.GetCurSel();
    int j = m_Combo_PointNum.GetCurSel();
    int l = m_Combo_AlarmID.GetCurSel();
    GetRuleConfig(i,j,l);
    m_Combo_Ruleid.SetCurSel(i);
    m_Combo_PointNum.SetCurSel(j);
    m_Combo_AlarmID.SetCurSel(l);
    UpdateData(FALSE);
}
