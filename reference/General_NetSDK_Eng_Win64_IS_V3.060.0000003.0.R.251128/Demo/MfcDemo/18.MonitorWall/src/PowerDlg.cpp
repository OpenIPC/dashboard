// PowerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "demomonitorwall.h"
#include "PowerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPowerDlg dialog


CPowerDlg::CPowerDlg(LLONG lLoginID, CWnd* pParent /*=NULL*/)
	: CDialog(CPowerDlg::IDD, pParent)
	, m_lLoginID(lLoginID)
{
	//{{AFX_DATA_INIT(CPowerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPowerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPowerDlg)
	DDX_Control(pDX, IDC_COMBO_POWER, m_cbPower);
	DDX_Control(pDX, IDC_COMBO_OUTPUT, m_cbOutput);
	DDX_Control(pDX, IDC_COMBO_MONITORWALL, m_cbWall);
	DDX_Control(pDX, IDC_COMBO_BLOCK, m_cbBlock);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPowerDlg, CDialog)
	//{{AFX_MSG_MAP(CPowerDlg)
	ON_CBN_SELCHANGE(IDC_COMBO_MONITORWALL, OnSelchangeComboMonitorwall)
	ON_CBN_SELCHANGE(IDC_COMBO_BLOCK, OnSelchangeComboBlock)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPowerDlg message handlers

BOOL CPowerDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_lLoginID == 0)
	{
		MsgBox0("Please login first.");
		OnCancel();
		return FALSE;
	}

	InitUI();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPowerDlg::InitUI()
{				
	m_vecWall.clear();

	const int nMaxJsonBuf = MAX_BUF_LEN;
	char* pszJsonBuf = new char[nMaxJsonBuf];
	ZeroMemory(pszJsonBuf, nMaxJsonBuf);
	int nRetLen = 0;

	// 取电视墙配置
	if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_MONITORWALL, -1, pszJsonBuf, nMaxJsonBuf, NULL))
	{
		const int nMaxMonitorWall = 4;
		AV_CFG_MonitorWall* pstuWall = new AV_CFG_MonitorWall[nMaxMonitorWall];
		for (int i = 0; i < nMaxMonitorWall; ++i)
		{
			pstuWall[i].nStructSize = sizeof(AV_CFG_MonitorWall);
			for (int j = 0; j < AV_CFG_Max_Block_In_Wall; ++j)
			{
				pstuWall[i].stuBlocks[j].nStructSize = sizeof(AV_CFG_MonitorWallBlock);
				for (int k = 0; k < AV_CFG_Max_TV_In_Block; ++k)
				{
					pstuWall[i].stuBlocks[j].stuTVs[k].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
				}
			}
		}
		
		if (CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf, pstuWall, pstuWall->nStructSize * nMaxMonitorWall, &nRetLen))
		{
			for (int i = 0; i < nRetLen / pstuWall->nStructSize; ++i)
			{
				m_vecWall.push_back(pstuWall[i]);
			}
		}						
		delete[] pstuWall;
	}

// 	// 取融合屏配置
// 	if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_SPLICESCREEN, -1, pszJsonBuf, nMaxJsonBuf, NULL))
// 	{
// 		const int nMaxSpliceScreen = 128;
// 		AV_CFG_SpliceScreen* pstuScreen = new AV_CFG_SpliceScreen[nMaxSpliceScreen];
// 		for (int i = 0; i < nMaxSpliceScreen; ++i)
// 		{
// 			pstuScreen[i].nStructSize = sizeof(AV_CFG_SpliceScreen);
// 		}
// 		
// 		int nRetLen = 0;
// 		if (CLIENT_ParseData(CFG_CMD_SPLICESCREEN, pszJsonBuf, pstuScreen, pstuScreen->nStructSize * nMaxSpliceScreen, &nRetLen))
// 		{
// 			for (int i = 0; i < nRetLen / pstuScreen->nStructSize; ++i)
// 			{
// 				m_vecSplice.push_back(pstuScreen[i]);
// 			}
// 		}						
// 		delete[] pstuScreen;
// 	}

	delete[] pszJsonBuf;

	// 
	m_cbWall.ResetContent();
	CString strText;

	for (int i = 0; i < m_vecWall.size(); ++i)
	{
		const AV_CFG_MonitorWall& wall = m_vecWall[i];
		strText.Format("%s (%d)", wall.szName, i);
		int nIndex = m_cbWall.AddString(strText);
		m_cbWall.SetItemData(nIndex, i);
	}
	m_cbWall.SetCurSel(0);
	OnSelchangeComboMonitorwall();
	m_cbPower.SetCurSel(0);
}

AV_CFG_MonitorWallBlock* CPowerDlg::GetSpliceScreen( LPCTSTR lpszMonitorName, int nBlockID )
{
	if (lpszMonitorName != NULL && nBlockID >= 0)
	{
		for (int i = 0; i < m_vecWall.size(); ++i)
		{
			AV_CFG_MonitorWall& stuWall = m_vecWall[i];
			if (strcmp(stuWall.szName, lpszMonitorName) == 0 && nBlockID < stuWall.nBlockCount)
			{
				return stuWall.stuBlocks + nBlockID;
			}
		}
	}
	return NULL;
}

void CPowerDlg::OnSelchangeComboMonitorwall() 
{
	m_cbBlock.ResetContent();
	
	int nIndex = m_cbBlock.AddString("All");
	m_cbBlock.SetItemData(nIndex, -1);
	
	nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		CString strText;
		int nWallID = m_cbWall.GetItemData(nIndex);
		
		if (nWallID >= 0 && nWallID < m_vecWall.size())
		{
			const AV_CFG_MonitorWall& wall = m_vecWall[nWallID];
			for (int i = 0; i < wall.nBlockCount; ++i)
			{
				const AV_CFG_MonitorWallBlock* pBlock = wall.stuBlocks + i;
				if (pBlock && *pBlock->szCompositeID != 0)
				{
					strText.Format("%s (%d)", pBlock->szCompositeID, i);
					nIndex = m_cbBlock.AddString(strText);
					m_cbBlock.SetItemData(nIndex, i);
				}
			}
		}
	}
	m_cbBlock.SetCurSel(0);
	OnSelchangeComboBlock();
}

void CPowerDlg::OnSelchangeComboBlock() 
{
	m_cbOutput.ResetContent();
	int nIndex = m_cbOutput.AddString("All");
	m_cbOutput.SetItemData(nIndex, -1);
	
	int nWallID = -1;
	int nBlockID = -1;
	
	nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		nWallID = m_cbWall.GetItemData(nIndex);
	}
	
	nIndex = m_cbBlock.GetCurSel();
	if (nIndex >= 0)
	{
		nBlockID = m_cbBlock.GetItemData(nIndex);
	}
	
	if (nWallID >= 0 && nWallID < m_vecWall.size())
	{
		const AV_CFG_MonitorWall& wall = m_vecWall[nWallID];
		if (nBlockID >= 0 && nBlockID < wall.nBlockCount)
		{
			CString strText;
			const AV_CFG_MonitorWallBlock& block = wall.stuBlocks[nBlockID];
			for (int i = 0; i < block.nTVCount; ++i)
			{
				strText.Format("%s (%d)", block.stuTVs[i].szName, i);
				nIndex = m_cbOutput.AddString(strText);
				m_cbOutput.SetItemData(nIndex, i);
			}
		}
	}
	m_cbOutput.SetCurSel(0);
}

void CPowerDlg::OnOK() 
{
	int nWallID = -1;
	int nBlockID = -1;
	int nOutputID = -1;
	
	int nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		nWallID = m_cbWall.GetItemData(nIndex);
	}
	
	nIndex = m_cbBlock.GetCurSel();
	if (nIndex >= 0)
	{
		nBlockID = m_cbBlock.GetItemData(nIndex);
	}

	nIndex = m_cbOutput.GetCurSel();
	if (nIndex >= 0)
	{
		nOutputID = m_cbOutput.GetItemData(nIndex);
	}

	BOOL bPowerOn = (m_cbPower.GetCurSel() == 0 ? TRUE : FALSE);

	std::string strBlockID;
	if (nWallID >= 0 && nWallID < m_vecWall.size())
	{
		const AV_CFG_MonitorWall& wall = m_vecWall[nWallID];
		if (nBlockID >= 0 && nBlockID < wall.nBlockCount)
		{
			const AV_CFG_MonitorWallBlock& block = wall.stuBlocks[nBlockID];
			strBlockID = block.szCompositeID;
		}
	}
	
	if (nWallID >= 0)
	{
		DH_IN_WM_POWER_CTRL stuInParam = { sizeof(DH_IN_WM_POWER_CTRL) };
		stuInParam.bPowerOn = bPowerOn;
		stuInParam.nMonitorWallID = nWallID;
		stuInParam.pszBlockID = strBlockID.c_str();
		stuInParam.nTVID = nOutputID;

		if (!CLIENT_PowerControl(m_lLoginID, &stuInParam, NULL))
		{
			MsgBoxErr("Power control error.");
		}
	}
	
	//CDialog::OnOK();
}