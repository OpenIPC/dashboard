// User_AddUser.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "User_AddUser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUser_AddUser dialog


CUser_AddUser::CUser_AddUser(CWnd* pParent /*=NULL*/)
	: CDialog(CUser_AddUser::IDD, pParent)
{
	m_user_info = 0;
	m_dev = 0;
	m_nGroupIndex = 0;
	//{{AFX_DATA_INIT(CUser_AddUser)
	//}}AFX_DATA_INIT
}


void CUser_AddUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUser_AddUser)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUser_AddUser, CDialog)
	//{{AFX_MSG_MAP(CUser_AddUser)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUser_AddUser message handlers


BOOL CUser_AddUser::OnInitDialog() 
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
	
	if (!m_user_info || !m_dev)
	{
		return TRUE;
	}

    m_rightList.ShowWindow(SW_SHOW);

	// 将权限添加到tree list 里面
    CString strRight;
    HTREEITEM hRoot;
	for (int i = 0; i < m_user_info->dwRightNum; i++)
	{
        strRight.Format("%d: %s: %s", m_user_info->rightList[i].dwID, m_user_info->rightList[i].name, m_user_info->rightList[i].memo);
		hRoot = m_rightList.InsertItem(strRight, 0, 0, TVI_ROOT);
		m_rightList.SetItemData(hRoot, m_user_info->rightList[i].dwID);
	}
    
    // 将用户组所有的权限勾选
    RefreshRightList(m_user_info->groupListEx[m_nGroupIndex].rights);
   
    // 显示用户组名
    GetDlgItem(IDC_GROUP_EDIT)->SetWindowText(m_user_info->groupListEx[m_nGroupIndex].name);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUser_AddUser::SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD nIndex, DeviceNode *dev)
{
	m_user_info = info;
	m_nGroupIndex = nIndex;  
	m_dev = dev;
}

void CUser_AddUser::OnBtnOk() 
{
	//construct user info
	USER_INFO_NEW urInfo = { sizeof(USER_INFO_NEW)};
	urInfo.dwID = m_user_info->dwUserNum + 1;
	GetDlgItem(IDC_NAME_EDIT)->GetWindowText(urInfo.name, DH_NEW_USER_NAME_LENGTH);
	GetDlgItem(IDC_PSW_EDIT)->GetWindowText(urInfo.passWord, DH_NEW_USER_PSW_LENGTH);
	GetDlgItem(IDC_MEMO_EDIT)->GetWindowText(urInfo.memo, DH_MEMO_LENGTH);
	urInfo.dwFouctionMask = ((CButton*)GetDlgItem(IDC_CHECK_REUSEABLE))->GetCheck();
	urInfo.dwGroupID = m_user_info->groupListEx[m_nGroupIndex].dwID;

	int count = m_rightList.GetCount();
	HTREEITEM node = m_rightList.GetRootItem();
	int rIndex = 0;
    bool bSelect = false;
	for (int i=0; i<count && node; i++)
	{
		if (m_rightList.GetCheck(node))
		{
			urInfo.rights[rIndex] = m_rightList.GetItemData(node);
			rIndex++;
            bSelect = true;
		}
		node = m_rightList.GetNextItem(node, TVGN_NEXT);
	}
	urInfo.dwRightNum = rIndex;
    if (bSelect == false)
    {
        MessageBox(ConvertString("Please select at least one right"));
        return;
    }


	BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 3, &urInfo, 0, NULL, MAX_TIMEOUT);
	if (!bRet)
    {
        int nError = CLIENT_GetLastError();
        if(nError == NET_RETURN_DATA_ERROR)	MessageBox(ConvertString("checkout return data error!"), ConvertString("Prompt"));
        else if(nError == NET_ILLEGAL_PARAM) MessageBox(ConvertString("Input error!"), ConvertString("Prompt"));
        else if(nError == NET_GROUP_NOEXIST) MessageBox(ConvertString("The group isn't existing!"), ConvertString("Prompt"));
        else if(nError == NET_USER_EXIST) MessageBox(ConvertString("The user is existing!"), ConvertString("Prompt"));
        else if(nError == NET_USER_RIGHTOVER) MessageBox(ConvertString("The user's right is out of the group's right!"), ConvertString("Prompt"));
        else MessageBox(ConvertString("Add user failed!"), ConvertString("Prompt"));
	}
	else
	{
		EndDialog(0);
	}
}

void CUser_AddUser::RefreshRightList(DWORD *dwList)
{
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
            if (dwList[i] == m_user_info->rightList[j].dwID)
            {
                m_rightList.SetCheck(node, TRUE);         
                break;
            }
            node = m_rightList.GetNextItem(node, TVGN_NEXT);               
        }
    }
}
