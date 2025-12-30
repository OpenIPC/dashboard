// CollectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "demomonitorwall.h"
#include "CollectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCollectionDlg dialog


CCollectionDlg::CCollectionDlg(LLONG lLoginID, CWnd* pParent /*=NULL*/)
	: CDialog(CCollectionDlg::IDD, pParent)
	, m_lLoginID(lLoginID)
{
	//{{AFX_DATA_INIT(CCollectionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCollectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCollectionDlg)
	DDX_Control(pDX, IDC_COMBO_MONITORWALL, m_cbWall);
	DDX_Control(pDX, IDC_LIST_COLLECTION, m_lbCollection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCollectionDlg, CDialog)
	//{{AFX_MSG_MAP(CCollectionDlg)
	ON_BN_CLICKED(IDC_BTN_LOAD, OnBtnLoad)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBtnSave)
	ON_LBN_SELCHANGE(IDC_LIST_COLLECTION, OnSelchangeListCollection)
	ON_CBN_SELCHANGE(IDC_COMBO_MONITORWALL, OnSelchangeComboMonitorwall)
	ON_BN_CLICKED(IDC_BTN_RENAME, OnBtnRename)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCollectionDlg message handlers

BOOL CCollectionDlg::OnInitDialog() 
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

void CCollectionDlg::InitUI()
{					
	const int nMaxJsonBuf = MAX_BUF_LEN;
	char* pszJsonBuf = new char[nMaxJsonBuf];
	ZeroMemory(pszJsonBuf, nMaxJsonBuf);
	int nRetLen = 0;
	int i = 0;
	
	// 取电视墙配置
	std::vector<AV_CFG_MonitorWall> vecWall;
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
				vecWall.push_back(pstuWall[i]);
			}
		}						
		delete[] pstuWall;
	}
	
	delete[] pszJsonBuf;

	// 取电视墙预案
	for (i = 0; i < vecWall.size(); ++i)
	{
		DH_IN_WM_GET_COLLECTIONS stuInParam = { sizeof(DH_IN_WM_GET_COLLECTIONS), i };
		DH_OUT_WM_GET_COLLECTIONS stuOutParam = { sizeof(DH_OUT_WM_GET_COLLECTIONS) };
		stuOutParam.nMaxCollectionsCount = 64;
		stuOutParam.pCollections = new DH_MONITORWALL_COLLECTION[stuOutParam.nMaxCollectionsCount];
		for (int j = 0; j < stuOutParam.nMaxCollectionsCount; ++j)
		{
			stuOutParam.pCollections[j].dwSize = sizeof(DH_MONITORWALL_COLLECTION);
			for (int k = 0; k < DH_MAX_BLOCK_NUM; ++k)
			{
				stuOutParam.pCollections[j].stuBlocks[k].dwSize = sizeof(DH_BLOCK_COLLECTION);
				for (int m = 0; m < DH_MAX_SPLIT_WINDOW; ++m)
				{
					stuOutParam.pCollections[j].stuBlocks[k].stuWnds[m].dwSize = sizeof(DH_WINDOW_COLLECTION);
				}
			}
		}
		if (!CLIENT_GetMonitorWallCollections(m_lLoginID, &stuInParam, &stuOutParam))
		{
			MsgBoxErr("Get collections error.");
			delete[] stuOutParam.pCollections;
			break;
		}

		m_vecMonitorWallCollections.push_back(&stuOutParam);	
		delete[] stuOutParam.pCollections;
	}
	
	// 
	m_cbWall.ResetContent();
	CString strText;
	
	for (i = 0; i < vecWall.size(); ++i)
	{
		const AV_CFG_MonitorWall& wall = vecWall[i];
		strText.Format("%s (%d)", wall.szName, i);
		int nIndex = m_cbWall.AddString(strText);
		m_cbWall.SetItemData(nIndex, i);
	}

	m_cbWall.SetCurSel(0);
	OnSelchangeComboMonitorwall();
}

void CCollectionDlg::OnBtnLoad() 
{
	int nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		int nWallID = m_cbWall.GetItemData(nIndex);
		CString strName;
		GetDlgItemText(IDC_EDIT_COLLECTION_NAME, strName);
		if (!strName.IsEmpty())
		{
			DH_IN_WM_LOAD_COLLECTION stuInParam = { sizeof(DH_IN_WM_LOAD_COLLECTION) };
			stuInParam.nMonitorWallID = nWallID;
			stuInParam.pszName = strName;
			if (!CLIENT_LoadMonitorWallCollection(m_lLoginID, &stuInParam, NULL))
			{
				MsgBoxErr("Load monitor wall collection error.");
			}
		}
	}
}

void CCollectionDlg::OnBtnSave() 
{
	int nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		int nWallID = m_cbWall.GetItemData(nIndex);
		CString strName;
		GetDlgItemText(IDC_EDIT_COLLECTION_NAME, strName);
		if (!strName.IsEmpty())
		{
			DH_IN_WM_SAVE_COLLECTION stuInParam = { sizeof(DH_IN_WM_SAVE_COLLECTION) };
			stuInParam.nMonitorWallID = nWallID;
			stuInParam.pszName = strName;
			if (!CLIENT_SaveMonitorWallCollection(m_lLoginID, &stuInParam, NULL))
			{
				MsgBoxErr("Save monitor wall collection error.");
			}
		}
	}
}

void CCollectionDlg::OnSelchangeListCollection() 
{
	int nIndex = m_lbCollection.GetCurSel();
	if (nIndex >= 0)
	{
		CString strText;
		m_lbCollection.GetText(nIndex, strText);
		SetDlgItemText(IDC_EDIT_COLLECTION_NAME, strText);
	}	
}

void CCollectionDlg::OnSelchangeComboMonitorwall() 
{	
	m_lbCollection.ResetContent();
	SetDlgItemText(IDC_EDIT_COLLECTION_NAME, NULL);

	int nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		int nWallID = m_cbWall.GetItemData(nIndex);
		if (nWallID >= 0 && nWallID < m_vecMonitorWallCollections.size())
		{
			const CMonitorWallCollections& collections = m_vecMonitorWallCollections[nWallID];
			for (int i = 0; i < collections.m_vecCollections.size(); ++i)
			{
				m_lbCollection.AddString(collections.m_vecCollections[i].szName);
			}

			if (m_lbCollection.GetCount() > 0)
			{
				CString strText;
				m_lbCollection.SetCurSel(0);
				m_lbCollection.GetText(0, strText);
				SetDlgItemText(IDC_EDIT_COLLECTION_NAME, strText);
			}
		}
	}
}

void CCollectionDlg::OnBtnRename() 
{
	int nIndex = m_cbWall.GetCurSel();
	if (nIndex >= 0)
	{
		int nWallID = m_cbWall.GetItemData(nIndex);
		CString strNameOld, strNameNew;
		GetDlgItemText(IDC_EDIT_COLLECTION_NAME, strNameOld);
		GetDlgItemText(IDC_EDIT_COLLECTION_NEW_NAME, strNameNew);
		//if (!strNameOld.IsEmpty() && !strNameNew.IsEmpty())
		{
			DH_IN_WM_RENAME_COLLECTION stuInParam = { sizeof(DH_IN_WM_RENAME_COLLECTION) };
			stuInParam.nMonitorWallID = nWallID;
			stuInParam.pszNewName = strNameNew;
			stuInParam.pszOldName = strNameOld;
			if (!CLIENT_RenameMonitorWallCollection(m_lLoginID, &stuInParam, NULL))
			{
				MsgBoxErr("Rename monitor wall collection error.");
				return;
			}

			// 更新显示
			nIndex = m_lbCollection.GetCurSel();
			m_lbCollection.InsertString(nIndex, strNameNew);
			m_lbCollection.DeleteString(nIndex + 1);
			SetDlgItemText(IDC_EDIT_COLLECTION_NAME, strNameNew);
		}
	}	
}
