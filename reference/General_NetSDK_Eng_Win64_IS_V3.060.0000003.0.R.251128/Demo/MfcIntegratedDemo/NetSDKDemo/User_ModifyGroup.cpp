// User_ModifyGroup.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "User_ModifyGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyGroup dialog


CUser_ModifyGroup::CUser_ModifyGroup(CWnd* pParent /*=NULL*/)
	: CDialog(CUser_ModifyGroup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUser_ModifyGroup)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_user_info = NULL;
	m_dev = NULL;
	m_nGroupIndex = 0;
}


void CUser_ModifyGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUser_ModifyGroup)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUser_ModifyGroup, CDialog)
	//{{AFX_MSG_MAP(CUser_ModifyGroup)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyGroup message handlers

BOOL CUser_ModifyGroup::OnInitDialog() 
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
	
	int i;
	CString strRight;
    HTREEITEM hRoot;
	for (i = 0; i < m_user_info->dwRightNum; i++)
	{
        strRight.Format("%d: %s: %s", m_user_info->rightList[i].dwID, m_user_info->rightList[i].name, m_user_info->rightList[i].memo);
		hRoot = m_rightList.InsertItem(strRight, 0, 0, TVI_ROOT);
		m_rightList.SetItemData(hRoot, m_user_info->rightList[i].dwID);
	}
	
	RefreshRightList(m_user_info->groupListEx[m_nGroupIndex].rights);
	
	GetDlgItem(IDC_GPNAME_OLD_EDIT)->SetWindowText(m_user_info->groupListEx[m_nGroupIndex].name);
	GetDlgItem(IDC_GPNAME_NEW_EDIT)->SetWindowText(m_user_info->groupListEx[m_nGroupIndex].name);
	GetDlgItem(IDC_GPMEMO_OLD_EDIT)->SetWindowText(m_user_info->groupListEx[m_nGroupIndex].memo);
	GetDlgItem(IDC_GPMEMO_NEW_EDIT)->SetWindowText(m_user_info->groupListEx[m_nGroupIndex].memo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUser_ModifyGroup::SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD gpIdx, DeviceNode *dev)
{
	m_dev = dev;
	m_user_info = info;
	m_nGroupIndex = gpIdx;
}

void CUser_ModifyGroup::OnBtnOk() 
{
	USER_GROUP_INFO_EX2 gpInfo = { sizeof(USER_GROUP_INFO_EX2)};
	gpInfo.dwID = m_user_info->groupListEx[m_nGroupIndex].dwID;
	GetDlgItem(IDC_GPNAME_NEW_EDIT)->GetWindowText(gpInfo.name, DH_NEW_USER_NAME_LENGTH);
	GetDlgItem(IDC_GPMEMO_NEW_EDIT)->GetWindowText(gpInfo.memo, DH_MEMO_LENGTH);
	
	int count = m_rightList.GetCount();
	HTREEITEM node = m_rightList.GetRootItem();
	int rIndex = 0;
    bool bSelect = false;
	for (int i=0; i<count && node; i++)
	{
		if (m_rightList.GetCheck(node))
		{
			gpInfo.rights[rIndex] = m_rightList.GetItemData(node);
			rIndex++;
            bSelect  = true;
		}
		node = m_rightList.GetNextItem(node, TVGN_NEXT);
	}
	gpInfo.dwRightNum = rIndex;
    if (bSelect == false)
    {
        MessageBox(ConvertString("Please select at least one right"));
        return;
    }
	BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 9, &gpInfo, &m_user_info->groupListEx[m_nGroupIndex], NULL, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Modify group failed!"));
	}
	else
	{
		EndDialog(0);
	}
}

void CUser_ModifyGroup::RefreshRightList(DWORD *dwList)
{
    int count = m_rightList.GetCount();
    HTREEITEM node = m_rightList.GetRootItem();

    for (int i = 0; i<count; i++)
    {
        m_rightList.SetCheck(node, FALSE);
        node = m_rightList.GetNextItem(node, TVGN_NEXT);
    }

    for (int i = 0; i < count; i++)
    {
        if(dwList[i] == 0)
            continue;
        node = m_rightList.GetRootItem();
        for (int j = 0;j < count; j++)
        {
            if (dwList[i] == m_user_info->rightList[j].dwID)
            {
                m_rightList.SetCheck(node, TRUE);         
                break;
            }
            node = m_rightList.GetNextItem(node, TVGN_NEXT);               
        }
    }
}
