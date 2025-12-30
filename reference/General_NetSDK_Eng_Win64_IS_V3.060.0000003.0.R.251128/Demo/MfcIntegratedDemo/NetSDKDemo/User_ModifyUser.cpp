// User_ModifyUser.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "User_ModifyUser.h"
//#include <atlbase.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyUser dialog


CUser_ModifyUser::CUser_ModifyUser(CWnd* pParent /*=NULL*/)
	: CDialog(CUser_ModifyUser::IDD, pParent)
{
	m_user_info = 0;
	m_dev = 0;
	m_nUserIndex = 0;
	//{{AFX_DATA_INIT(CUser_ModifyUser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUser_ModifyUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUser_ModifyUser)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUser_ModifyUser, CDialog)
	//{{AFX_MSG_MAP(CUser_ModifyUser)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUser_ModifyUser message handlers

BOOL CUser_ModifyUser::OnInitDialog() 
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

	DWORD gpId = m_user_info->userList[m_nUserIndex].dwGroupID;
	for (i = 0; i < m_user_info->dwGroupNum; i++)
	{
		if (m_user_info->groupListEx[i].dwID == gpId)
		{
			GetDlgItem(IDC_GROUP_EDIT)->SetWindowText(m_user_info->groupListEx[i].name);
			break;
		}
	}

	GetDlgItem(IDC_NAME_OLD_EDIT)->SetWindowText(m_user_info->userList[m_nUserIndex].name);
	GetDlgItem(IDC_NAME_NEW_EDIT)->SetWindowText(m_user_info->userList[m_nUserIndex].name);
	GetDlgItem(IDC_MEMO_OLD_EDIT)->SetWindowText(m_user_info->userList[m_nUserIndex].memo);
	GetDlgItem(IDC_MEMO_NEW_EDIT)->SetWindowText(m_user_info->userList[m_nUserIndex].memo);
    ((CButton*)GetDlgItem(IDC_CHECK_REUSEABLE))->SetCheck(m_user_info->userList[m_nUserIndex].dwFouctionMask);
	((CButton*)GetDlgItem(IDC_CHECK_REUSEABLE2))->SetCheck(m_user_info->userList[m_nUserIndex].dwFouctionMask);

	RefreshRightList(m_user_info->userList[m_nUserIndex].rights);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUser_ModifyUser::SetEnvrmt(USER_MANAGE_INFO_NEW *info, DWORD userIdx, DeviceNode *dev)
{
	m_dev = dev;
	m_user_info = info;
	m_nUserIndex = userIdx;
}

void CUser_ModifyUser::RefreshRightList(DWORD *dwList)
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

void CUser_ModifyUser::OnBtnOk() 
{
	USER_INFO_NEW newInfo = { sizeof(USER_INFO_NEW)};
	newInfo.dwID = m_user_info->userList[m_nUserIndex].dwID;
	GetDlgItem(IDC_NAME_NEW_EDIT)->GetWindowText(newInfo.name, DH_NEW_USER_NAME_LENGTH);
	strncpy(newInfo.passWord, m_user_info->userList[m_nUserIndex].passWord, DH_NEW_USER_PSW_LENGTH-1);
	GetDlgItem(IDC_MEMO_NEW_EDIT)->GetWindowText(newInfo.memo, DH_MEMO_LENGTH);
	newInfo.dwGroupID = m_user_info->userList[m_nUserIndex].dwGroupID; //这里需验证-linjy
	newInfo.dwFouctionMask = ((CButton*)GetDlgItem(IDC_CHECK_REUSEABLE2))->GetCheck();
	
	int count = m_rightList.GetCount();
	HTREEITEM node = m_rightList.GetRootItem();
	int rIndex = 0;
    bool bSelect = false;
	for (int i=0; i<count && node; i++)
	{
		if (m_rightList.GetCheck(node))
		{
			newInfo.rights[rIndex] = m_rightList.GetItemData(node);
			rIndex++;
            bSelect  = true;
		}
		node = m_rightList.GetNextItem(node, TVGN_NEXT);
	}
	newInfo.dwRightNum = rIndex;
    if (bSelect == false)
    {
        MessageBox(ConvertString("Please select at least one right"));
        return;
    }

	BOOL bRet = CLIENT_OperateUserInfoNew(m_dev->LoginID, 5, (void *)&newInfo, (void *)&m_user_info->userList[m_nUserIndex], NULL, MAX_TIMEOUT);
	if (!bRet)
	{
        int nError = CLIENT_GetLastError();
        if(nError == NET_RETURN_DATA_ERROR)	MessageBox(ConvertString("checkout return data error!"), ConvertString("Prompt"));
        else if(nError == NET_ILLEGAL_PARAM) MessageBox(ConvertString("Input error!"), ConvertString("Prompt"));
        else if(nError == NET_USER_NOEXIST) MessageBox(ConvertString("The user is not existing!"), ConvertString("Prompt"));
        else if(nError == NET_USER_EXIST) MessageBox(ConvertString("The user is existing!"), ConvertString("Prompt"));
        else if(nError == NET_USER_RIGHTOVER) MessageBox(ConvertString("The user's right is out of the group's right!"), ConvertString("Prompt"));
        else MessageBox(ConvertString("Modify user info failed!"), ConvertString("Prompt"));
	}
	else
	{
		EndDialog(0);
	}
}
