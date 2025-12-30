// ModifyUserInfo.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "ModifyUserInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyUserInfo dialog


CModifyUserInfo::CModifyUserInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyUserInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyUserInfo)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwUserID = 0;
	m_usersInfo = 0;
	m_hLoginID = 0;
}


void CModifyUserInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyUserInfo)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyUserInfo, CDialog)
	//{{AFX_MSG_MAP(CModifyUserInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyUserInfo message handlers

void CModifyUserInfo::OnOK() 
{
	// TODO: Add extra validation here
	if(!IsValid())
	{
		MessageBox(ConvertString("Please Input!", DLG_USERMANAGER), ConvertString("Prompt"));
		return;
	}
	USER_INFO_NEW userInfo = {sizeof(userInfo)};
	userInfo.dwID = m_usersInfo->userList[m_dwUserID].dwID;
	GetDlgItem(IDC_MODIFY_USER_EDIT_NEWNAME)->GetWindowText(userInfo.name, sizeof(userInfo.name) - 1);
	strncpy(userInfo.passWord, m_usersInfo->userList[m_dwUserID].passWord, sizeof(userInfo.passWord) - 1);
	GetDlgItem(IDC_MODIFY_USER_EDIT_NEWMEMO)->GetWindowText(userInfo.memo, sizeof(userInfo.memo) - 1);
	userInfo.dwFouctionMask = ((CButton*)GetDlgItem(IDC_MODIFY_USER_CHECK_NEWREUSEABLE))->GetCheck();
	userInfo.dwGroupID = m_usersInfo->userList[m_dwUserID].dwGroupID;
	
	int nCountItem = m_treeLevels.GetCount();
	HTREEITEM hItem = m_treeLevels.GetRootItem();
	int nIndex = 0;
    bool bSlect =false;
	for (int i=0; i<nCountItem && hItem; i++)
	{
		if (m_treeLevels.GetCheck(hItem))
		{
			userInfo.rights[nIndex] = m_treeLevels.GetItemData(hItem);
			nIndex++;
            bSlect = true;
		}
		hItem = m_treeLevels.GetNextItem(hItem, TVGN_NEXT);
	}
	userInfo.dwRightNum = nIndex;
    if (bSlect == false)
    {
        MessageBox(ConvertString("Please select at least one right"));
        return;
    }
	
	BOOL bRet = CLIENT_OperateUserInfoNew(m_hLoginID, 5, (void *)&userInfo, (void *)&m_usersInfo->userList[m_dwUserID], 0, 10000);
	if (!bRet)
	{
		int nError = CLIENT_GetLastError();
		if(nError == NET_RETURN_DATA_ERROR)	MessageBox(ConvertString("checkout return data error!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_ILLEGAL_PARAM) MessageBox(ConvertString("Input error!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_NOEXIST) MessageBox(ConvertString("The user is not existing!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_EXIST) MessageBox(ConvertString("The user is existing!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_RIGHTOVER) MessageBox(ConvertString("The user's right is out of the group's right!", DLG_USERMANAGER), ConvertString("Prompt"));
		else MessageBox(ConvertString("Modify user info failed!", DLG_USERMANAGER), ConvertString("Prompt"));
	}
	else
	{
		CDialog::OnOK();
	}
}

BOOL CModifyUserInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_USERMANAGER);
	// TODO: Add extra initialization here
	//Create tree
	CRect rect;
	GetDlgItem(IDC_MODIFY_USER_MODIFY_FRAME)->GetClientRect(&rect);
	GetDlgItem(IDC_MODIFY_USER_MODIFY_FRAME)->ClientToScreen(&rect);
	ScreenToClient(&rect);
	BOOL bCreate = m_treeLevels.Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER
		| TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES 
		| TVS_DISABLEDRAGDROP, rect, this, 0x1005);
	
	SetWindowLong(m_treeLevels.m_hWnd, GWL_STYLE, TVS_CHECKBOXES);
	m_treeLevels.ShowWindow(SW_SHOW);

	if (!m_usersInfo || !m_hLoginID)
	{
		return TRUE;
	}

	//Set group name 
	int i = 0;
	DWORD dwGroupID = m_usersInfo->userList[m_dwUserID].dwGroupID;
	for(i=0;i<(int)(m_usersInfo->dwGroupNum);i++)
	{
		if(m_usersInfo->groupListEx[i].dwID == dwGroupID)
		{
			//CString str = m_usersInfo->groupListEx[i].name;
			SetDlgItemText(IDC_MODIFY_USER_EDIT_GROUP, m_usersInfo->groupListEx[i].name);
			//char szName[17] = {0};
			//memcpy(szName, m_usersInfo->userList[m_dwUserID].name, 16);
			//str = szName;
			SetDlgItemText(IDC_MODIFY_USER_EDIT_USERNAME,m_usersInfo->userList[m_dwUserID].name);
			SetDlgItemText(IDC_MODIFY_USER_EDIT_NEWNAME,m_usersInfo->userList[m_dwUserID].name);
		//	str = m_usersInfo->userList[m_dwUserID].memo;
			SetDlgItemText(IDC_MODIFY_USER_EDIT_MEMO,m_usersInfo->userList[m_dwUserID].memo);
			SetDlgItemText(IDC_MODIFY_USER_EDIT_NEWMEMO,m_usersInfo->userList[m_dwUserID].memo);
			DWORD dwSpecial = m_usersInfo->dwFouctionMask;
			if(dwSpecial == 0)
			{
				GetDlgItem(IDC_MODIFY_USER_CHECK_NEWREUSEABLE)->EnableWindow(FALSE);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_MODIFY_USER_CHECK_REUSEABLE))->SetCheck(m_usersInfo->userList[m_dwUserID].dwFouctionMask);
				((CButton*)GetDlgItem(IDC_MODIFY_USER_CHECK_NEWREUSEABLE))->SetCheck(m_usersInfo->userList[m_dwUserID].dwFouctionMask);
			}
			//Initialize right list 
			InitLevelTree(m_usersInfo->userList[m_dwUserID].dwRightNum,m_usersInfo->userList[m_dwUserID].rights, m_usersInfo->groupListEx[i].dwRightNum, m_usersInfo->groupListEx[i].rights);
			break;
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyUserInfo::InitModifyUserInfoDlg(USER_MANAGE_INFO_NEW *info, DWORD dwID, LLONG hLoginID)
{
	m_usersInfo = info;
	m_dwUserID = dwID;
	m_hLoginID = hLoginID;
}

void CModifyUserInfo::InitLevelTree(DWORD dwNum, DWORD *dwList, DWORD dwGroupNum, DWORD *dwGroupList)
{
	m_treeLevels.DeleteAllItems();
	//Right information 
	int i = 0;
	for(i=0;i<(int)m_usersInfo->dwRightNum;i++)
	{
		CString str;
		str.Format("%d: %s : %s      ",m_usersInfo->rightList[i].dwID,m_usersInfo->rightList[i].name,
			m_usersInfo->rightList[i].memo);
		HTREEITEM hRoot = m_treeLevels.InsertItem(str,TVI_ROOT,TVI_LAST);
		m_treeLevels.SetItemData(hRoot,m_usersInfo->rightList[i].dwID);
	}
	
	// 只显示组所拥有的权限, 删除无效权限
	int nCountItem = m_treeLevels.GetCount();
	HTREEITEM hItem = m_treeLevels.GetRootItem();
	for(i=0;i<(int)dwGroupNum;i++)
	{
		hItem = m_treeLevels.GetRootItem();
		for(int j=0;j<nCountItem && hItem;j++)
		{
			DWORD dwData = m_treeLevels.GetItemData(hItem);
			if(dwData == dwGroupList[i])
			{
				m_treeLevels.SetCheck(hItem,TRUE);
				break;
			}
			hItem = m_treeLevels.GetNextItem(hItem,TVGN_NEXT);
		}
	}

	hItem = m_treeLevels.GetRootItem();
	HTREEITEM hItemTemp = hItem;
	for (i = 0; i < nCountItem; ++i)
	{
		hItemTemp = hItem;
		BOOL bb = m_treeLevels.GetCheck(hItem);
		if (!m_treeLevels.GetCheck(hItem))
		{
			hItem = m_treeLevels.GetNextItem(hItem,TVGN_NEXT);
			m_treeLevels.DeleteItem(hItemTemp);
		}
		else
		{
			hItem = m_treeLevels.GetNextItem(hItem,TVGN_NEXT);
		}
	}

	//Update right list 
	nCountItem = m_treeLevels.GetCount();
	hItem = m_treeLevels.GetRootItem();
	for (i = 0; i < nCountItem; ++i)
	{
		m_treeLevels.SetCheck(hItem,FALSE);
		hItem = m_treeLevels.GetNextItem(hItem,TVGN_NEXT);
	}

	hItem = m_treeLevels.GetRootItem();
	for(i=0;i<(int)dwNum;i++)
	{
		hItem = m_treeLevels.GetRootItem();

		for(int j=0;j<nCountItem && hItem;j++)
		{
			DWORD dwData = m_treeLevels.GetItemData(hItem);
			if(dwData == dwList[i])
			{
				m_treeLevels.SetCheck(hItem,TRUE);
				break;
			}
			hItem = m_treeLevels.GetNextItem(hItem,TVGN_NEXT);
		}
	}
}

BOOL CModifyUserInfo::IsValid()
{
	BOOL bValid = TRUE;
	CString str1;
	GetDlgItemText(IDC_MODIFY_USER_EDIT_NEWNAME,str1);
	if(str1.IsEmpty())
	{
		bValid = FALSE;
	}
	return bValid;
}

