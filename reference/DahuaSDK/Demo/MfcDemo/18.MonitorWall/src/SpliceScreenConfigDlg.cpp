// SpliceScreenConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "demomonitorwall.h"
#include "SpliceScreenConfigDlg.h"
#include "DemoMonitorWallDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpliceScreenConfigDlg dialog


CSpliceScreenConfigDlg::CSpliceScreenConfigDlg(LLONG lLoginID, CWnd* pParent /*=NULL*/)
	: CDialog(CSpliceScreenConfigDlg::IDD, pParent)
	, m_lLoginID(lLoginID)
{
	//{{AFX_DATA_INIT(CSpliceScreenConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSpliceScreenConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpliceScreenConfigDlg)
	DDX_Control(pDX, IDC_LIST_BLOCK_OUTPUTS, m_lbOutputs);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_SIZE_ROW, m_cbTVSizeRow);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_SIZE_COLUMN, m_cbTVSizeColumn);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_LAYOUT_COLUMN, m_cbTVLayoutColumn);
	DDX_Control(pDX, IDC_COMBO_OUTPUT_LAYOUT_ROW, m_cbTVLayoutRow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpliceScreenConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CSpliceScreenConfigDlg)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_OUTPUT_LAYOUT_ROW, OnSelchangeComboOutputLayoutOrSize)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, OnBtnDelete)
	ON_BN_CLICKED(IDC_BTN_CLEAR, OnBtnClear)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_1x1, OnRadioLayout)
	ON_BN_CLICKED(IDC_RADIO_SIZE_nxn, OnRadioSize)
	ON_EN_KILLFOCUS(IDC_EDIT_SPLICE_NAME, OnKillfocusEditSpliceName)
	ON_CLBN_CHKCHANGE(IDC_LIST_BLOCK_OUTPUTS, OnCheckChangeOutputs)
	ON_EN_KILLFOCUS(IDC_EDIT_SPLICE_ID, OnKillfocusEditSpliceId)
	ON_EN_CHANGE(IDC_EDIT_SPLICE_ID, OnChangeEditSpliceId)
	ON_EN_CHANGE(IDC_EDIT_SPLICE_NAME, OnChangeEditSpliceName)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_2x2, OnRadioLayout)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_3x3, OnRadioLayout)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_4x4, OnRadioLayout)
	ON_BN_CLICKED(IDC_RADIO_LAYOUT_nxn, OnRadioLayout)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTPUT_LAYOUT_COLUMN, OnSelchangeComboOutputLayoutOrSize)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTPUT_SIZE_ROW, OnSelchangeComboOutputLayoutOrSize)
	ON_CBN_SELCHANGE(IDC_COMBO_OUTPUT_SIZE_COLUMN, OnSelchangeComboOutputLayoutOrSize)
	ON_BN_CLICKED(IDC_RADIO_SIZE_1x1, OnRadioSize)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpliceScreenConfigDlg message handlers

BOOL CSpliceScreenConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	InitUI();

	LMIU(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpliceScreenConfigDlg::InitUI()
{
	m_wndWall.CreateEx(WS_EX_CLIENTEDGE, NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 465, 385), this, 22333);

	m_cbTVLayoutRow.SetCurSel(0);
	m_cbTVLayoutColumn.SetCurSel(0);
	m_cbTVSizeRow.SetCurSel(0);
	m_cbTVSizeColumn.SetCurSel(0);
	CheckDlgButton(IDC_RADIO_LAYOUT_2x2, TRUE);
	CheckDlgButton(IDC_RADIO_SIZE_1x1, TRUE);
	OnRadioLayout();
	OnRadioSize();
	
	const int nMaxNameLen = 60;
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SPLICE_NAME);
	pEdit->LimitText(nMaxNameLen);	
	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SPLICE_ID);
	pEdit->LimitText(nMaxNameLen);
	
	// fill outputs
	m_lbOutputs.ResetContent();
	const CDecoder* pDecoder = ((CDemoMonitorWallDlg*)GetParent())->GetDecoderInfo();
	CDecChnMap::const_iterator it = pDecoder->GetDecChnMap().begin();
	for (; it != pDecoder->GetDecChnMap().end(); ++it)
	{
		if (!it->second.m_bSpliceScreen)
		{
			int nIndex = m_lbOutputs.AddString(it->second.ToString());
			m_lbOutputs.SetItemData(nIndex, it->first);
		}
	}

	// get config
	const int nMaxJsonLen = MAX_BUF_LEN;
	char* pszJsonBuf = new char[nMaxJsonLen];
	memset(pszJsonBuf, 0, nMaxJsonLen);

	/*
	// 解码器使用老协议配置拼接屏
	if (_stricmp(CDecoderNVD::DCID(), pDecoder->GetDeviceClass()) == 0)
	{
		// 解码器的拼接屏ID无法设置
		GetDlgItem(IDC_EDIT_SPLICE_ID)->EnableWindow(FALSE);

		// 获取电视墙配置
		if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_MONITORWALL, -1, pszJsonBuf, nMaxJsonLen, NULL))
		{
			const int nMaxMonitorWall = 1;
			AV_CFG_MonitorWall* pstuWall = new AV_CFG_MonitorWall[nMaxMonitorWall];
			for (int i = 0; i < nMaxMonitorWall; ++i)
			{
				pstuWall[i].nStructSize = sizeof(AV_CFG_MonitorWall);
				for (int j = 0; j < AV_CFG_Max_Block_In_Wall; ++j)
				{
					pstuWall[i].stuBlocks[j].nStructSize = sizeof(AV_CFG_MonitorWallBlock);
					pstuWall[i].stuBlocks[j].stuRect.nStructSize = sizeof(AV_CFG_Rect);
					for (int k = 0; k < AV_CFG_Max_TV_In_Block; ++k)
					{
						pstuWall[i].stuBlocks[j].stuTVs[k].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
					}
				}
			}
			
			int nRetLen = 0;
			if (CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf, pstuWall, pstuWall->nStructSize * nMaxMonitorWall, &nRetLen))
			{
				m_wndWall.ClearBlock();
				m_wndWall.SetName(pstuWall->szName);
				m_wndWall.SetGridLayout(CSize(pstuWall->nColumn, pstuWall->nLine));

				// 获取拼接屏配置
				if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_SPLICESCREEN, -1, pszJsonBuf, nMaxJsonLen, NULL))
				{
					int nMaxSpliceScreen = 32;
					AV_CFG_SpliceScreen* pstuScreen = new AV_CFG_SpliceScreen[nMaxSpliceScreen];
					for (int i = 0; i < nMaxSpliceScreen; ++i)
					{
						pstuScreen[i].nStructSize = sizeof(AV_CFG_SpliceScreen);
					}					
					
					if (CLIENT_ParseData(CFG_CMD_SPLICESCREEN, pszJsonBuf, pstuScreen, pstuScreen->nStructSize * nMaxSpliceScreen, &nRetLen))
					{
						int nSpliceCount = nRetLen / pstuScreen->nStructSize;	// 拼接屏数量
						for (int k = 0; k < nSpliceCount; ++k)
						{
							AV_CFG_SpliceScreen* p = pstuScreen + k;

							// 判断该拼接屏是否包含在电视墙配置中
							if (_stricmp(p->szWallName, pstuWall->szName) == 0 &&
								p->nBlockID >= 0 && p->nBlockID < pstuWall->nBlockCount)
							{
								int j = 0;
								const AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[p->nBlockID];							

								WMBlock wmblock(&m_wndWall);
								wmblock.bNew = TRUE;
								sprintf(wmblock.szCompositeID, "%d", p->nBlockID);
								strcpy(wmblock.szName, p->szName);
								wmblock.stuTVSize.cx = block.nColumn;
								wmblock.stuTVSize.cy = block.nLine;
								wmblock.stuBlockSize.cx = (block.stuRect.nRight - block.stuRect.nLeft + 1) / block.nColumn;
								wmblock.stuBlockSize.cy = (block.stuRect.nBottom - block.stuRect.nTop + 1) / block.nLine;
								wmblock.stuLeftTop.x = block.stuRect.nLeft * m_wndWall.GetGridSize().cx;
								wmblock.stuLeftTop.y = block.stuRect.nTop * m_wndWall.GetGridSize().cy;
								for (j = 0; j < block.nTVCount; ++j)
								{
									wmblock.vecOutputs.push_back(block.stuTVs[j].nChannelID);
								}
								m_wndWall.AddBlock(wmblock);
							}
						}
					}
					delete[] pstuScreen;
				}
			}
			delete[] pstuWall;
		}			
	}
	// 其他设备使用新协议
	else
	*/
	{
		if (CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_MONITORWALL, -1, pszJsonBuf, nMaxJsonLen, NULL, WAIT_TIME))
		{
			AV_CFG_MonitorWall* pstuWall = new AV_CFG_MonitorWall();
            if( NULL == pstuWall )
            {
                return;
            }
            memset(pstuWall , 0 , sizeof(AV_CFG_MonitorWall));
            pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);

			for (int i = 0; i < AV_CFG_Max_Block_In_Wall; ++i)
			{
				pstuWall->stuBlocks[i].nStructSize = sizeof(AV_CFG_MonitorWallBlock);				
				pstuWall->stuBlocks[i].stuRect.nStructSize = sizeof(AV_CFG_MonitorWallTVOut);	
				for (int j = 0; j < AV_CFG_Max_TV_In_Block; ++j)
				{
					pstuWall->stuBlocks[i].stuTVs[j].nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
				}
			}
			
			int nRetLen = 0;
			if (CLIENT_ParseData(CFG_CMD_MONITORWALL, pszJsonBuf, pstuWall, sizeof(AV_CFG_MonitorWall), &nRetLen))
			{
				m_wndWall.ClearBlock();
				m_wndWall.SetName(pstuWall->szName);
				m_wndWall.SetGridLayout(CSize(pstuWall->nColumn, pstuWall->nLine));
				for (int i = 0; i < pstuWall->nBlockCount; ++i)
				{
					AV_CFG_MonitorWallBlock* pBlockInfo = pstuWall->stuBlocks + i;
					//if (pBlockInfo->nTVCount > 0)
					if (*pBlockInfo->szCompositeID)
					{
						WMBlock block(&m_wndWall);
						block.bNew = TRUE;
						strncpy(block.szCompositeID, pBlockInfo->szCompositeID, sizeof(block.szCompositeID) - 1);
						strncpy(block.szName, pBlockInfo->szName, sizeof(block.szName) - 1);
						block.stuTVSize.cx = pBlockInfo->nColumn;
						block.stuTVSize.cy = pBlockInfo->nLine;
						block.stuBlockSize.cx = (pBlockInfo->stuRect.nRight - pBlockInfo->stuRect.nLeft + 1) / pBlockInfo->nColumn;
						block.stuBlockSize.cy = (pBlockInfo->stuRect.nBottom - pBlockInfo->stuRect.nTop + 1) / pBlockInfo->nLine;
						block.stuLeftTop.x = pBlockInfo->stuRect.nLeft * m_wndWall.GetGridSize().cx;
						block.stuLeftTop.y = pBlockInfo->stuRect.nTop * m_wndWall.GetGridSize().cy;
						for (int j = 0; j < pBlockInfo->nTVCount; ++j)
						{
							block.vecOutputs.push_back(pBlockInfo->stuTVs[j].nChannelID);
						}
						m_wndWall.AddBlock(block);
					}
				}
			}
			else
			{
				MsgBox0("Parse config data (monitor wall) error.");
			}
            delete pstuWall;
		}
		else
		{
			MsgBoxErr("Get config (monitor wall) error.");
		}
	}
	delete[] pszJsonBuf;
   
}

void CSpliceScreenConfigDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	m_wndWall.DestroyWindow();	
}

void CSpliceScreenConfigDlg::OnSelchangeComboOutputLayoutOrSize() 
{
	// TODO: Add your control notification handler code here
	
}

void CSpliceScreenConfigDlg::OnRadioLayout() 
{
	BOOL bFixed = IsDlgButtonChecked(IDC_RADIO_LAYOUT_1x1)
		|| IsDlgButtonChecked(IDC_RADIO_LAYOUT_2x2)
		|| IsDlgButtonChecked(IDC_RADIO_LAYOUT_3x3)
		|| IsDlgButtonChecked(IDC_RADIO_LAYOUT_4x4);
	
	m_cbTVLayoutRow.EnableWindow(!bFixed);
	m_cbTVLayoutColumn.EnableWindow(!bFixed);
}

void CSpliceScreenConfigDlg::OnRadioSize() 
{
	BOOL bFixed = IsDlgButtonChecked(IDC_RADIO_SIZE_1x1)
		|| IsDlgButtonChecked(IDC_RADIO_SIZE_2x2)
		|| IsDlgButtonChecked(IDC_RADIO_SIZE_3x3)
		|| IsDlgButtonChecked(IDC_RADIO_SIZE_4x4);
	
	m_cbTVSizeRow.EnableWindow(!bFixed);
	m_cbTVSizeColumn.EnableWindow(!bFixed);
}

void CSpliceScreenConfigDlg::OnBtnAdd() 
{
	WMBlock block(&m_wndWall);
	// Layout
	if (IsDlgButtonChecked(IDC_RADIO_LAYOUT_1x1))
	{
		block.stuBlockSize.cx = 1;
		block.stuBlockSize.cy = 1;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_LAYOUT_2x2))
	{
		block.stuBlockSize.cx = 2;
		block.stuBlockSize.cy = 2;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_LAYOUT_3x3))
	{
		block.stuBlockSize.cx = 3;
		block.stuBlockSize.cy = 3;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_LAYOUT_4x4))
	{
		block.stuBlockSize.cx = 4;
		block.stuBlockSize.cy = 4;
	}
	else
	{
		block.stuBlockSize.cx = GetDlgItemInt(IDC_COMBO_OUTPUT_LAYOUT_COLUMN);
		block.stuBlockSize.cy = GetDlgItemInt(IDC_COMBO_OUTPUT_LAYOUT_ROW);
	}
	if (block.stuBlockSize.cx <= 0 || block.stuBlockSize.cy <= 0)
	{
		return;
	}

	// Size
	if (IsDlgButtonChecked(IDC_RADIO_SIZE_1x1))
	{
		block.stuTVSize.cx = 1;
		block.stuTVSize.cy = 1;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_SIZE_2x2))
	{
		block.stuTVSize.cx = 2;
		block.stuTVSize.cy = 2;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_SIZE_3x3))
	{
		block.stuTVSize.cx = 3;
		block.stuTVSize.cy = 3;
	}
	else if (IsDlgButtonChecked(IDC_RADIO_SIZE_4x4))
	{
		block.stuTVSize.cx = 4;
		block.stuTVSize.cy = 4;
	}
	else
	{
		block.stuTVSize.cx = GetDlgItemInt(IDC_COMBO_OUTPUT_SIZE_COLUMN);
		block.stuTVSize.cy = GetDlgItemInt(IDC_COMBO_OUTPUT_SIZE_ROW);
	}
	if (block.stuTVSize.cx <= 0 || block.stuTVSize.cy <= 0)
	{
		return;
	}

	block.bNew = TRUE;

	if (!m_wndWall.AddBlock(block))
	{
		MsgBox0("Add block error.");
	}
}

void CSpliceScreenConfigDlg::OnBtnDelete() 
{
	m_wndWall.RemoveBlock();
}

void CSpliceScreenConfigDlg::OnBtnClear() 
{
	m_wndWall.ClearBlock();
}

void CSpliceScreenConfigDlg::OnOK() 
{
	if (m_wndWall.IsBlockIDConflict())
	{
		MsgBox("Block ID conflict.");
		return;
	}

	BOOL bResult = FALSE;
	const int nMaxJsonLen = MAX_BUF_LEN;
	char* pszJsonBuf = new char[nMaxJsonLen];
	memset(pszJsonBuf, 0, sizeof(nMaxJsonLen));

	AV_CFG_MonitorWall* pstuWall = new AV_CFG_MonitorWall;
	memset(pstuWall, 0, sizeof(AV_CFG_MonitorWall));
	pstuWall->nStructSize = sizeof(AV_CFG_MonitorWall);
	strncpy(pstuWall->szName, m_wndWall.GetName(), sizeof(pstuWall->szName) - 1);
	pstuWall->nColumn = m_wndWall.GetGridLayout().cx;
	pstuWall->nLine = m_wndWall.GetGridLayout().cy;
	pstuWall->nBlockCount = __min(_countof(pstuWall->stuBlocks), m_wndWall.GetBlockCount());
	for (int i = 0; i < pstuWall->nBlockCount; ++i)
	{
		const WMBlock* pBlock = m_wndWall.GetBlock(i);
		AV_CFG_MonitorWallBlock& block = pstuWall->stuBlocks[i];
		block.nStructSize = sizeof(AV_CFG_MonitorWallBlock);
		strncpy(block.szName, pBlock->szName, sizeof(block.szName) - 1);
		block.nColumn = pBlock->stuTVSize.cx;
		block.nLine = pBlock->stuTVSize.cy;
		if (strlen(pBlock->szCompositeID))
		{
			strncpy(block.szCompositeID, pBlock->szCompositeID, sizeof(block.szCompositeID) - 1);
		}
		else
		{
			MsgBox1("Composite (%s) ID can't be empty.", block.szName);
            delete pstuWall;
    	    delete[] pszJsonBuf;
            return;
		}
//		block.bComposite = TRUE;
		block.stuRect.nStructSize = sizeof(AV_CFG_Rect);
		RECT rcGrid = m_wndWall.GridRect(pBlock);
		block.stuRect.nLeft = rcGrid.left;
		block.stuRect.nTop = rcGrid.top;
		block.stuRect.nRight = rcGrid.right;
		block.stuRect.nBottom = rcGrid.bottom;
		block.nTVCount = __min(_countof(block.stuTVs), pBlock->vecOutputs.size());
		for (int j = 0; j < block.nTVCount; ++j)
		{
			AV_CFG_MonitorWallTVOut& output = block.stuTVs[j];
			output.nStructSize = sizeof(AV_CFG_MonitorWallTVOut);
			int nOutputID = pBlock->vecOutputs[j];
			output.nChannelID = nOutputID;
			sprintf(output.szName, "output %d", nOutputID);
		}
		// 开关机时间测试
// 		for (int k = 0; k < WEEK_DAY_NUM; ++k)
// 		{
// 			block.stuTimeSection[k][0].nBeginHour = 2;
// 			block.stuTimeSection[k][0].nEndHour = 3;			
// 			block.stuTimeSection[k][1].nBeginHour = 12;
// 			block.stuTimeSection[k][1].nEndHour = 13;
// 		}
	}

	if (CLIENT_PacketData(CFG_CMD_MONITORWALL, pstuWall, sizeof(*pstuWall), pszJsonBuf, nMaxJsonLen))
	{
		bResult = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_MONITORWALL, -1, pszJsonBuf, strlen(pszJsonBuf), NULL, NULL, WAIT_TIME);
	}

	/*
	// NVD需要再设置融合屏(老协议)
	CDemoMonitorWallDlg* pParent = (CDemoMonitorWallDlg*)GetParent();
	if (pParent && 
		pParent->GetDecoderInfo() && 
		strcmp(pParent->GetDecoderInfo()->GetDeviceClass(), CDecoderNVD::DCID()) == 0)
	{
		AV_CFG_SpliceScreen* pstuScreen = new AV_CFG_SpliceScreen[pstuWall->nBlockCount];
		ZeroMemory(pstuScreen, sizeof(AV_CFG_SpliceScreen) * pstuWall->nBlockCount);
		for (int i = 0; i < pstuWall->nBlockCount; ++i)
		{
			pstuScreen[i].nStructSize = sizeof(AV_CFG_SpliceScreen);
			strcpy(pstuScreen[i].szWallName, pstuWall->szName);
			pstuScreen[i].nBlockID = i;
			strcpy(pstuScreen[i].szName, pstuWall->stuBlocks[i].szName);
		}

		if (CLIENT_PacketData(CFG_CMD_SPLICESCREEN, pstuScreen, sizeof(AV_CFG_SpliceScreen) * pstuWall->nBlockCount, pszJsonBuf, nMaxJsonLen))
		{
			bResult = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_SPLICESCREEN, -1, pszJsonBuf, strlen(pszJsonBuf), NULL, NULL, WAIT_TIME);
		}
		delete[] pstuScreen;
	}
	*/

	delete pstuWall;
	delete[] pszJsonBuf;

	if (bResult)
	{
		CDialog::OnOK();
	}
	else
	{
		MsgBox("Set config error.");
	}
}

LRESULT CSpliceScreenConfigDlg::DefWindowProc( UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message)
	{
	case WM_BLOCK_SEL_CHANGE:
		{
			WMBlock* pBlock = m_wndWall.GetBlock(lParam);
			if (pBlock)
			{
				SetDlgItemText(IDC_EDIT_SPLICE_NAME, pBlock->szName);	
				SetDlgItemText(IDC_EDIT_SPLICE_ID, pBlock->szCompositeID);
				EnableOutputs(pBlock);
				
				int i = 0;
				UINT nID = -1;
				// layout
				UINT arCheckBoxLayout[] = 
				{ 
					IDC_RADIO_LAYOUT_1x1, 
					IDC_RADIO_LAYOUT_2x2, 
					IDC_RADIO_LAYOUT_3x3,
					IDC_RADIO_LAYOUT_4x4, 
					IDC_RADIO_LAYOUT_nxn
				};
				if (pBlock->stuBlockSize.cx == pBlock->stuBlockSize.cy 
					&& pBlock->stuBlockSize.cx > 0 
					&& pBlock->stuBlockSize.cx < _countof(arCheckBoxLayout))
				{
					nID = arCheckBoxLayout[pBlock->stuBlockSize.cx - 1];
				}
				else
				{
					nID = IDC_RADIO_LAYOUT_nxn;
					((CComboBox*)GetDlgItem(IDC_COMBO_OUTPUT_LAYOUT_ROW))->SetCurSel(pBlock->stuBlockSize.cy - 1);
					((CComboBox*)GetDlgItem(IDC_COMBO_OUTPUT_LAYOUT_COLUMN))->SetCurSel(pBlock->stuBlockSize.cx - 1);					
				}
				for (i = 0; i < _countof(arCheckBoxLayout); ++i)
				{
					CheckDlgButton(arCheckBoxLayout[i], arCheckBoxLayout[i] == nID);
				}
				GetDlgItem(IDC_COMBO_OUTPUT_LAYOUT_ROW)->EnableWindow(nID == IDC_RADIO_LAYOUT_nxn);
				GetDlgItem(IDC_COMBO_OUTPUT_LAYOUT_COLUMN)->EnableWindow(nID == IDC_RADIO_LAYOUT_nxn);			

				// size
				UINT arCheckBoxTVSize[] = 
				{ 
					IDC_RADIO_SIZE_1x1, 
					IDC_RADIO_SIZE_2x2, 
					IDC_RADIO_SIZE_3x3,
					IDC_RADIO_SIZE_4x4, 
					IDC_RADIO_SIZE_nxn
				};
				if (pBlock->stuTVSize.cx == pBlock->stuTVSize.cy 
					&& pBlock->stuTVSize.cx > 0 
					&& pBlock->stuTVSize.cx < _countof(arCheckBoxTVSize))
				{
					nID = arCheckBoxTVSize[pBlock->stuTVSize.cx - 1];
				}
				else
				{
					nID = IDC_RADIO_SIZE_nxn;
					((CComboBox*)GetDlgItem(IDC_COMBO_OUTPUT_SIZE_ROW))->SetCurSel(pBlock->stuTVSize.cy - 1);
					((CComboBox*)GetDlgItem(IDC_COMBO_OUTPUT_SIZE_COLUMN))->SetCurSel(pBlock->stuTVSize.cx - 1);
				}	
				for (i = 0; i < _countof(arCheckBoxTVSize); ++i)
				{
					CheckDlgButton(arCheckBoxTVSize[i], arCheckBoxTVSize[i] == nID);
				}
				GetDlgItem(IDC_COMBO_OUTPUT_SIZE_ROW)->EnableWindow(nID == IDC_RADIO_SIZE_nxn);
				GetDlgItem(IDC_COMBO_OUTPUT_SIZE_COLUMN)->EnableWindow(nID == IDC_RADIO_SIZE_nxn);				
			}
		}
		break;
	default:
		return CDialog::DefWindowProc(message, wParam, lParam);
	}
	return TRUE;
}

void CSpliceScreenConfigDlg::EnableOutputs( const WMBlock* pBlockActive )
{
	int i = 0;
	for (i = 0; i < m_lbOutputs.GetCount(); ++i)
	{
		m_lbOutputs.Enable(i, TRUE);
		m_lbOutputs.SetCheck(i, FALSE);
	}

	for (i = 0; i < m_wndWall.GetBlockCount(); ++i)
	{
		const WMBlock* pBlock = m_wndWall.GetBlock(i);
		if (pBlock && pBlock != pBlockActive)
		{
			for (int j = 0; j < pBlock->vecOutputs.size(); ++j)
			{
				EnableOutput(pBlock->vecOutputs[j], FALSE);
			}
		}
	}

	for (i = 0; i < pBlockActive->vecOutputs.size(); ++i)
	{
		CheckOutput(pBlockActive->vecOutputs[i], TRUE);
	}
}

void CSpliceScreenConfigDlg::EnableOutput( int nChannel, BOOL bEnable /*= TRUE*/ )
{
	for (int i = 0; i < m_lbOutputs.GetCount(); ++i)
	{
		if (m_lbOutputs.GetItemData(i) == nChannel)
		{
			m_lbOutputs.Enable(i, bEnable);
			return;
		}
	}
}

void CSpliceScreenConfigDlg::CheckOutput( int nChannel, BOOL bCheck /*= TRUE*/ )
{
	for (int i = 0; i < m_lbOutputs.GetCount(); ++i)
	{
		if (m_lbOutputs.GetItemData(i) == nChannel)
		{
			m_lbOutputs.SetCheck(i, bCheck);
			return;
		}
	}
}

void CSpliceScreenConfigDlg::OnKillfocusEditSpliceName() 
{
	WMBlock* pBlock = m_wndWall.GetActiveBlock();
	if (pBlock)
	{
		GetDlgItemText(IDC_EDIT_SPLICE_NAME, pBlock->szName, sizeof(pBlock->szName));
		if (*pBlock->szName == 0)
		{
			GetDlgItem(IDC_EDIT_SPLICE_NAME)->SetFocus();
			MsgBox("Please input block name.");
		}
	}
}

void CSpliceScreenConfigDlg::OnKillfocusEditSpliceId() 
{
	WMBlock* pBlock = m_wndWall.GetActiveBlock();
	if (pBlock)
	{
		GetDlgItemText(IDC_EDIT_SPLICE_ID, pBlock->szCompositeID, sizeof(pBlock->szCompositeID));
// 		if (!m_wndWall.IsBlockIDEnable(pBlock))
// 		{
// 			GetDlgItem(IDC_EDIT_SPLICE_ID)->SetFocus();
// 			MsgBox("block id error.");
// 		}
	}
}

void CSpliceScreenConfigDlg::OnChangeEditSpliceName() 
{
	WMBlock* pBlock = m_wndWall.GetActiveBlock();
	if (pBlock)
	{
		GetDlgItemText(IDC_EDIT_SPLICE_NAME, pBlock->szName, sizeof(pBlock->szName));
	}	
}

void CSpliceScreenConfigDlg::OnChangeEditSpliceId() 
{
	WMBlock* pBlock = m_wndWall.GetActiveBlock();
	if (pBlock)
	{
		GetDlgItemText(IDC_EDIT_SPLICE_ID, pBlock->szCompositeID, sizeof(pBlock->szCompositeID));
		m_wndWall.RedrawWindow();
	}
}

void CSpliceScreenConfigDlg::OnCheckChangeOutputs()
{
	WMBlock* pBlock = m_wndWall.GetActiveBlock();
	if (pBlock)
	{
		pBlock->vecOutputs.clear();
		for (int i = 0; i < m_lbOutputs.GetCount(); ++i)
		{
			if (m_lbOutputs.GetCheck(i))
			{
				pBlock->vecOutputs.push_back(m_lbOutputs.GetItemData(i));
			}
		}
	}
}

void CSpliceScreenConfigDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnLButtonDblClk(nFlags, point);
}
