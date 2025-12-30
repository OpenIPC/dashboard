// UserManage.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "netsdkdemodlg.h"
#include "UserManage.h"
#include "User_AddUser.h"
#include "USER_AddGroup.h"
#include "USER_ModifyUser.h"
#include "USER_ModifyGroup.h"
#include "USER_ModifyPsw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUserManage dialog


CUserManage::CUserManage(CWnd* pParent /*=NULL*/)
	: CDialog(CUserManage::IDD, pParent)
{
   m_pUserInfo = new USER_MANAGE_INFO_NEW;
   
   InitStuUserInfo(m_pUserInfo);
	//{{AFX_DATA_INIT(CUserManage)
	//}}AFX_DATA_INIT
	m_dev = NULL;	
}


void CUserManage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserManage)
	DDX_Control(pDX, IDC_USERLIST_TREE, m_userList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserManage, CDialog)
	//{{AFX_MSG_MAP(CUserManage)
	ON_BN_CLICKED(IDC_ADD_USER, OnAddUser)
	ON_BN_CLICKED(IDC_MODIFY_USER, OnModifyUser)
	ON_BN_CLICKED(IDC_ADD_GROUP, OnAddGroup)
	ON_BN_CLICKED(IDC_DELETE_USER, OnDeleteUser)
	ON_BN_CLICKED(IDC_MODIFY_PSW, OnModifyPsw)
	ON_BN_CLICKED(IDC_DELETE_GROUP, OnDeleteGroup)
	ON_BN_CLICKED(IDC_MODIFY_GROUP, OnModifyGroup)
	ON_NOTIFY(TVN_SELCHANGED, IDC_USERLIST_TREE, OnSelchangedUserlistTree)
	//}}AFX_MSG_MAP
    ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserManage message handlers

BOOL CUserManage::OnInitDialog() 
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
    RefreshUserAndRight();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserManage::OnDestroy()
{
    CDialog::OnDestroy();

    m_userList.DeleteAllItems();
    m_rightList.DeleteAllItems();
    if (m_pUserInfo)
    {
        delete m_pUserInfo;
    }
}


void CUserManage::SetDeviceId(DeviceNode *dev)
{
	m_dev = dev;
}


void CUserManage::OnAddUser() 
{
	HTREEITEM node = m_userList.GetSelectedItem();
	if (!node)
	{
		return;
	}

    CUser_AddUser dlg;
    int nIndex = (DWORD)m_userList.GetItemData(node);
   
    if (isParentNode(node))  // 父节点
    {
          dlg.SetEnvrmt(m_pUserInfo, nIndex, m_dev);   //将用户组下标传入
    }
    else  // 子节点
    {
        int nGroupIndex = GetGroupIndex(m_pUserInfo->userList[nIndex].dwGroupID);
        if (nGroupIndex < 0 )
        {
            return;
        }
        dlg.SetEnvrmt(m_pUserInfo, nGroupIndex, m_dev);  
    }
    
	if (0 == dlg.DoModal())
	{
		RefreshUserAndRight();
	}
}

void CUserManage::OnModifyUser() 
{
    HTREEITEM node = m_userList.GetSelectedItem();
    if (!node)
    {
        return;
    }
    CUser_ModifyUser dlg;
    int nIndex = (DWORD)m_userList.GetItemData(node);

    if (!isParentNode(node))  // 子节点
    {
         dlg.SetEnvrmt(m_pUserInfo, nIndex, m_dev);  
    }
    else   // 父节点
    {
        return; 
    }

    if (0 == dlg.DoModal())
    {
        RefreshUserAndRight();
    }
}

void CUserManage::OnDeleteUser() 
{
	HTREEITEM node = m_userList.GetSelectedItem();
	if (!node)
	{
		return;
	}

    int nIndex = (DWORD)m_userList.GetItemData(node);

    if (!isParentNode(node))  // 子节点
    {
        BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 4, (void *)&m_pUserInfo->userList[nIndex], 0, NULL, MAX_TIMEOUT);
        if (!bRet)
        {
            ((CNetSDKDemoDlg *)AfxGetMainWnd())->LastError();
        }
    }
    else   // 父节d点
    {
        return; 
    }
    RefreshUserAndRight();
}

void CUserManage::OnModifyPsw() 
{
	HTREEITEM node = m_userList.GetSelectedItem();
	if (!node)
	{
		return;
	}

    CUser_ModifyPsw dlg;
    int nIndex = (DWORD)m_userList.GetItemData(node);

    if (!isParentNode(node))  // 子节点
    {
         dlg.SetEnvrmt(m_pUserInfo, nIndex, m_dev);
    }
    else   // 父节点
    {
        return; 
    }
  
    if (0 == dlg.DoModal())
    {
        RefreshUserAndRight();
    }
}

void CUserManage::OnAddGroup()
{
	CUser_AddGroup dlg;
	dlg.SetEnvrmt(m_pUserInfo, m_dev);
	if (0 == dlg.DoModal())
	{
		RefreshUserAndRight();
	}
}

void CUserManage::OnDeleteGroup() 
{
	HTREEITEM node = m_userList.GetSelectedItem();
	if (!node)
	{
		return;
	}

    int nIndex = (DWORD)m_userList.GetItemData(node);
    if (isParentNode(node))  // 父节点
    {
        BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 8, (void *)&m_pUserInfo->groupListEx[nIndex], 0, NULL, MAX_TIMEOUT);
        if (!bRet)
        {
            ((CNetSDKDemoDlg *)AfxGetMainWnd())->LastError();
        }
        RefreshUserAndRight();
    }
    else   // 子节点
    {
        return; 
    }

}

void CUserManage::OnModifyGroup() 
{
	HTREEITEM node = m_userList.GetSelectedItem();
	if (!node)
	{
		return;
	}

    CUser_ModifyGroup dlg;
    int nIndex = (DWORD)m_userList.GetItemData(node);

    if (isParentNode(node))  // 父节点
    {
        dlg.SetEnvrmt(m_pUserInfo, nIndex, m_dev);
    }
    else   // 子节点 
    {
        return; 
    }

    if (0 == dlg.DoModal())
    {
        RefreshUserAndRight();
    }
}

void CUserManage::RefreshRightList(DWORD *dwList)
{
	if (m_dev->Info.nDVRType == NET_NB_SERIAL)
	{
		return;
	}

	int count = m_rightList.GetCount();
	HTREEITEM node = m_rightList.GetRootItem();

	for (int i=0; i<count; i++)
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
            if (dwList[i] == m_pUserInfo->rightList[j].dwID)
            {
                m_rightList.SetCheck(node, TRUE);         
                break;
            }
            node = m_rightList.GetNextItem(node, TVGN_NEXT);               
        }
    }
}

// 选择用户或者用户组，显示权限
void CUserManage::OnSelchangedUserlistTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM node = m_userList.GetSelectedItem();
	if (!node)
	{
		return;
	}
    
  int nIndex = m_userList.GetItemData(node);
    if (NULL == m_userList.GetParentItem(node))
    {
        // 用户组 父节点
        RefreshRightList(m_pUserInfo->groupListEx[nIndex].rights);
    }
    else
    {
        // 用户子节点
        RefreshRightList(m_pUserInfo->userList[nIndex].rights);
    }
}


// 显示用户和权限
void CUserManage::RefreshUserAndRight()
{
    m_userList.DeleteAllItems();
    m_rightList.DeleteAllItems();
    
    // 将结构体重新初始化
    InitStuUserInfo(m_pUserInfo);

    BOOL bRet = CLIENT_QueryUserInfoNew(m_dev->LoginID, m_pUserInfo, NULL, MAX_TIMEOUT);
    if (!bRet)
    {
        MessageBox(ConvertString("failed to get user info"));
        return;
    }
   
    // 添加用户组和用户
    HTREEITEM hRoot;
    HTREEITEM hLeaf;
    for (int i = 0; i < m_pUserInfo->dwGroupNum; i++)
    {
         hRoot = m_userList.InsertItem(m_pUserInfo->groupListEx[i].name, 0, 0, TVI_ROOT);                    
        
        // 将用户组下标存入 TreeCtrl节点
        m_userList.SetItemData(hRoot, (DWORD)i);

        for (int j = 0; j < m_pUserInfo->dwUserNum; j++)
        {
            if (m_pUserInfo->userList[j].dwGroupID == m_pUserInfo->groupListEx[i].dwID)
            {
                 hLeaf = m_userList.InsertItem(m_pUserInfo->userList[j].name, 0, 0, hRoot);
            
                 // 将用户下标存入 TreeCtrl节点
                m_userList.SetItemData(hLeaf, (DWORD)j);
            }
        }
        m_userList.Expand(hRoot, TVE_EXPAND);
    }

    // 将设备获取的权限写入 TreeCtrl
    CString strRight;
    for (int i = 0; i < m_pUserInfo->dwRightNum; i++)
    {
        strRight.Format("%d: %s: %s", m_pUserInfo->rightList[i].dwID, m_pUserInfo->rightList[i].name, m_pUserInfo->rightList[i].memo);
        hRoot = m_rightList.InsertItem(strRight, 0, 0, TVI_ROOT);
    }
}


int CUserManage::GetGroupIndex(int nGroupId)
{
    int nIndex = -1;
    for (int i = 0; i < m_pUserInfo->dwGroupNum; i++)
    {
        if (nGroupId == m_pUserInfo->groupListEx[i].dwID)
        {
            nIndex = i;
            break;
        }
    }
    return nIndex;
}


void CUserManage::InitStuUserInfo(USER_MANAGE_INFO_NEW* pInfo)
{
    if (pInfo == NULL)
    {
        return;
    }

    memset(pInfo, 0, sizeof(USER_MANAGE_INFO_NEW));
    pInfo->dwSize = sizeof(USER_MANAGE_INFO_NEW);

    for(int i =0; i < DH_MAX_GROUP_NUM; ++i)
    {
        pInfo->groupListEx[i].dwSize = sizeof(USER_GROUP_INFO_EX2);
    }
    for(int i =0; i < DH_NEW_MAX_RIGHT_NUM; ++i)
    {
        pInfo->rightList[i].dwSize = sizeof(OPR_RIGHT_NEW);
    }

    for(int i =0; i < DH_MAX_GROUP_NUM; ++i)
    {
        pInfo->groupList[i].dwSize = sizeof(USER_GROUP_INFO_NEW);
    }

    for(int i =0; i < DH_MAX_USER_NUM; ++i)
    {
        pInfo->userList[i].dwSize = sizeof(USER_INFO_NEW);
    }
}

bool CUserManage::isParentNode(HTREEITEM node)
{
    if( NULL == m_userList.GetParentItem(node))
    {
        // 头结点
        return true;
    }
    else
    {
        // 子节点
        return false;
    }

}