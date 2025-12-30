// StatistcisDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "StatistcisDlg.h"
#include "GlobalDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatistcisDlg dialog


CStatistcisDlg::CStatistcisDlg(CWnd* pParent /*=NULL*/,LLONG lLoginId)
	: CDialog(CStatistcisDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStatistcisDlg)
	m_Cycle = 0;
	m_ITemName = _T("");
	//}}AFX_DATA_INIT
    m_iLoginID = lLoginId;
	memset(&stuInfo, 0, sizeof(stuInfo));
}


void CStatistcisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatistcisDlg)
	DDX_Control(pDX, IDC_COMBO_TEMPSTANUM, m_Combo_TempStaNum);
	DDX_Control(pDX, IDC_COMBO_TEMP, m_Combo_Temp);
	DDX_Control(pDX, IDC_COMBO_MODETYPE, m_Combo_ModeType);
	DDX_Text(pDX, IDC_EDIT_CYCLE, m_Cycle);
	DDX_Text(pDX, IDC_EDIT_ITEMNAME, m_ITemName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStatistcisDlg, CDialog)
	//{{AFX_MSG_MAP(CStatistcisDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_TEMPSTANUM, OnSelchangeComboTempstanum)
	ON_BN_CLICKED(IDC_BTN_GET, OnBtnGet)
	ON_BN_CLICKED(IDC_BTN_SET, OnBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatistcisDlg message handlers

BOOL CStatistcisDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = 1;

	BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_TEMP_STATISTICS,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_TEMP_STATISTICS,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            return FALSE;
        }
        else
        {
            int m_Amout = stuInfo.nCount;
            int index = 0;
            for (int i=1;i<=m_Amout;i++)
            {
                CString str;
                str.Format("%d",i);
                index = m_Combo_TempStaNum.AddString(str);
                m_Combo_TempStaNum.SetItemData(index,i);
            }
            if (0 == stuInfo.stStatistics[0].bEnable)
            {
                m_Combo_Temp.SetCurSel(1);
            }
            else if (1 == stuInfo.stStatistics[0].bEnable)
            {
                m_Combo_Temp.SetCurSel(0);
            }
            if (NET_RADIOMETRY_METERTYPE_UNKNOWN == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(0);
            }
            else if (NET_RADIOMETRY_METERTYPE_SPOT == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(1);
            }
            else if (NET_RADIOMETRY_METERTYPE_LINE == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(2);
            }
            else if (NET_RADIOMETRY_METERTYPE_AREA == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(3);
            }
            m_Cycle = stuInfo.stStatistics[0].nPeriod;
            m_ITemName = stuInfo.stStatistics[0].szName;
        }
    }
    else
    {
        return FALSE;
    }
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStatistcisDlg::OnSelchangeComboTempstanum() 
{
	int i = m_Combo_TempStaNum.GetCurSel();
    if (0 == stuInfo.stStatistics[i].bEnable)
    {
        m_Combo_Temp.SetCurSel(1);
    }
    else if (1 == stuInfo.stStatistics[i].bEnable)
    {
        m_Combo_Temp.SetCurSel(0);
    }
    if (NET_RADIOMETRY_METERTYPE_UNKNOWN == stuInfo.stStatistics[i].nMeterType)
    {
        m_Combo_ModeType.SetCurSel(0);
    }
    else if (NET_RADIOMETRY_METERTYPE_SPOT == stuInfo.stStatistics[i].nMeterType)
    {
        m_Combo_ModeType.SetCurSel(1);
    }
    else if (NET_RADIOMETRY_METERTYPE_LINE == stuInfo.stStatistics[i].nMeterType)
    {
        m_Combo_ModeType.SetCurSel(2);
    }
    else if (NET_RADIOMETRY_METERTYPE_AREA == stuInfo.stStatistics[i].nMeterType)
    {
        m_Combo_ModeType.SetCurSel(3);
    }
    m_Cycle = stuInfo.stStatistics[i].nPeriod;
    m_ITemName = stuInfo.stStatistics[i].szName;

}

void CStatistcisDlg::OnBtnGet() 
{
    char szJsonBuf[1024 * 40] = {0};
    int nerror = 0;
    int nChannel = -1;
    
    BOOL ret = CLIENT_GetNewDevConfig(m_iLoginID,CFG_CMD_TEMP_STATISTICS,nChannel,szJsonBuf,1024*40,&nerror,3000);
    if (0 != ret)
    {
        DWORD dwRetLen = 0;
        ret = CLIENT_ParseData(CFG_CMD_TEMP_STATISTICS,szJsonBuf,(void*)&stuInfo,sizeof(stuInfo),&dwRetLen);
        if (!ret)
        {
            MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
            return ;
        }
        else
        {
            int m_Amout = stuInfo.nCount;
            int index = 0;
            m_Combo_TempStaNum.ResetContent();
            for (int i=1;i<=m_Amout;i++)
            {
                CString str;
                str.Format("%d",i);
                index = m_Combo_TempStaNum.AddString(str);
                m_Combo_TempStaNum.SetItemData(index,i);
            }
            if (0 == stuInfo.stStatistics[0].bEnable)
            {
                m_Combo_Temp.SetCurSel(1);
            }
            else if (1 == stuInfo.stStatistics[0].bEnable)
            {
                m_Combo_Temp.SetCurSel(0);
            }
            if (NET_RADIOMETRY_METERTYPE_UNKNOWN == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(0);
            }
            else if (NET_RADIOMETRY_METERTYPE_SPOT == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(1);
            }
            else if (NET_RADIOMETRY_METERTYPE_LINE == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(2);
            }
            else if (NET_RADIOMETRY_METERTYPE_AREA == stuInfo.stStatistics[0].nMeterType)
            {
                m_Combo_ModeType.SetCurSel(3);
            }
            m_Cycle = stuInfo.stStatistics[0].nPeriod;
            m_ITemName = stuInfo.stStatistics[0].szName;
        }
    }
    else
    {
        MessageBox(ConvertString("getconfig error"), ConvertString("Prompt"));
        return ;
    }
	UpdateData(FALSE);
}

void CStatistcisDlg::OnBtnSet() 
{
	
}
