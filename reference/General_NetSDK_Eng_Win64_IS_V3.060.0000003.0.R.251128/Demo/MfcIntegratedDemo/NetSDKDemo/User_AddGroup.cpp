// User_AddGroup.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "User_AddGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUser_AddGroup dialog


CUser_AddGroup::CUser_AddGroup(CWnd* pParent /*=NULL*/)
	: CDialog(CUser_AddGroup::IDD, pParent)
{
	m_user_info = 0;
	m_dev = 0;
	//{{AFX_DATA_INIT(CUser_AddGroup)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUser_AddGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUser_AddGroup)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUser_AddGroup, CDialog)
	//{{AFX_MSG_MAP(CUser_AddGroup)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUser_AddGroup message handlers

BOOL CUser_AddGroup::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	CRect rect;
	GetDlgItem(IDC_RLIST_FRAME)->GetClientRect(&rect);
	GetDlgItem(IDC_RLIST_FRAME)->ClientToScreen(&rect);
	ScreenToClient(&rect);
	
	BOOL bCreate = m_rightList.Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER
		| TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES 
		| TVS_DISABLEDRAGDROP, rect, this, 0x1005);
	
	SetWindowLong(m_rightList.m_hWnd, GWL_STYLE, TVS_CHECKBOXES);
	
	m_rightList.ShowWindow(SW_SHOW);
	
	if (!m_user_info || !m_dev)
	{
		return TRUE;
	}
	
	CString strRight;
    HTREEITEM hRoot;
	for (int i = 0; i < m_user_info->dwRightNum; i++)
	{
        strRight.Format("%d: %s: %s", m_user_info->rightList[i].dwID, m_user_info->rightList[i].name, m_user_info->rightList[i].memo);
		hRoot = m_rightList.InsertItem(strRight, 0, 0, TVI_ROOT);
		m_rightList.SetItemData(hRoot, m_user_info->rightList[i].dwID);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUser_AddGroup::SetEnvrmt(USER_MANAGE_INFO_NEW *info, DeviceNode *dev)
{
	m_user_info = info;
	m_dev = dev;
}

void CUser_AddGroup::OnBtnOk() 
{
	USER_GROUP_INFO_EX2 ugInfo = { sizeof(USER_GROUP_INFO_EX2)};
	ugInfo.dwID = m_user_info->dwGroupNum + 1;
	GetDlgItem(IDC_NAME_EDIT)->GetWindowText(ugInfo.name, DH_NEW_USER_NAME_LENGTH);
	GetDlgItem(IDC_MEMO_EDIT)->GetWindowText(ugInfo.memo, DH_MEMO_LENGTH);
	
	int count = m_rightList.GetCount();
	HTREEITEM node = m_rightList.GetRootItem();
	int rIndex = 0;
    bool bSelect = false;
	for (int i=0; i<count && node; i++)
	{
		if (m_rightList.GetCheck(node))
		{
			ugInfo.rights[rIndex] = m_rightList.GetItemData(node);
			rIndex++;
            bSelect  = true;
		}
		node = m_rightList.GetNextItem(node, TVGN_NEXT);
	}
    if (bSelect == false)
    {
        MessageBox(ConvertString("Please select at least one right"));
        return;
    }
	ugInfo.dwRightNum = rIndex;
	BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 7/*type: add user group*/, &ugInfo, 0, NULL, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Add group failed!"));
	}
	else
	{
		EndDialog(0);
	}
}
