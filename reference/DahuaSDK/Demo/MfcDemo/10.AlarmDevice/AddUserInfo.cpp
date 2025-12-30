// AddUserInfo.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "AddUserInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddUserInfo dialog


CAddUserInfo::CAddUserInfo(CWnd* pParent /*=NULL*/)
	: CDialog(CAddUserInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddUserInfo)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwGroupID = 0;
	m_usersInfo = 0;
	m_hLoginID = 0;
}


void CAddUserInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddUserInfo)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddUserInfo, CDialog)
	//{{AFX_MSG_MAP(CAddUserInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddUserInfo message handlers
//Add user
void CAddUserInfo::OnOK() 
{
	// TODO: Add extra validation here
	if(!IsValid())
	{
		MessageBox(ConvertString("Please Input!", DLG_USERMANAGER), ConvertString("Prompt"));
		return;
	}
	USER_INFO_NEW userInfo = {sizeof(userInfo)};
	userInfo.dwID = m_usersInfo->dwUserNum + 1;
	GetDlgItem(IDC_ADD_USER_EDIT_USERNAME)->GetWindowText(userInfo.name, DH_NEW_USER_NAME_LENGTH+1);
	GetDlgItem(IDC_ADD_USER_EDIT_PASSWORD)->GetWindowText(userInfo.passWord, DH_NEW_USER_PSW_LENGTH+1);
	GetDlgItem(IDC_ADD_USER_EDIT_USERMEMO)->GetWindowText(userInfo.memo, DH_MEMO_LENGTH);
	userInfo.dwFouctionMask = ((CButton*)GetDlgItem(IDC_ADD_USER_CHECK_REUSEABLE))->GetCheck();
	userInfo.dwGroupID = m_dwGroupID;
	
	int nCountItem = m_treeLevels.GetCount();
	HTREEITEM hItem = m_treeLevels.GetRootItem();
	int nIndex = 0;
    bool    bSlect = false;
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
	
	BOOL bRet = CLIENT_OperateUserInfoNew(m_hLoginID, 3/*type: add user*/, &userInfo, 0, 0, 1000 * 10);
	if (!bRet)
	{
		int nError = CLIENT_GetLastError();
		if(nError == NET_RETURN_DATA_ERROR)	MessageBox(ConvertString("checkout return data error!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_ILLEGAL_PARAM) MessageBox(ConvertString("Input error!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_GROUP_NOEXIST) MessageBox(ConvertString("The group isn't existing!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_EXIST) MessageBox(ConvertString("The user is existing!", DLG_USERMANAGER), ConvertString("Prompt"));
		else if(nError == NET_USER_RIGHTOVER) MessageBox(ConvertString("The user's right is out of the group's right!", DLG_USERMANAGER), ConvertString("Prompt"));
		else MessageBox(ConvertString("Add user failed!", DLG_USERMANAGER), ConvertString("Prompt"));
	}
	else
	{
		CDialog::OnOK();
	}
}

//Initialize right list 
void CAddUserInfo::InitAddUserInfoDlg(USER_MANAGE_INFO_NEW *info, DWORD dwID, LLONG hLoginID)
{
	m_usersInfo = info;
	m_dwGroupID = dwID;
	m_hLoginID = hLoginID;
}

BOOL CAddUserInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_USERMANAGER);
	// TODO: Add extra initialization here
	//Create tree 
	CRect rect;
	GetDlgItem(IDC_ADD_USER_RLIST_FRAME)->GetClientRect(&rect);
	GetDlgItem(IDC_ADD_USER_RLIST_FRAME)->ClientToScreen(&rect);
	ScreenToClient(&rect);
	BOOL bCreate = m_treeLevels.Create(WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER
		| TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES 
		| TVS_DISABLEDRAGDROP, rect, this, 0x1005);
	
	SetWindowLong(m_treeLevels.m_hWnd, GWL_STYLE, TVS_CHECKBOXES);
	m_treeLevels.ShowWindow(SW_SHOW);
	
	//Set group name 
	int i = 0;
	for(i=0;i<(int)(m_usersInfo->dwGroupNum);i++)
	{
		if(m_usersInfo->groupListEx[i].dwID == m_dwGroupID)
		{
			CString str = m_usersInfo->groupListEx[i].name;
			SetDlgItemText(IDC_ADD_USER_EDIT_GROUP,str);
			DWORD dwSpecial = m_usersInfo->dwFouctionMask;
			if(dwSpecial == 0)
			{
				GetDlgItem(IDC_ADD_USER_CHECK_REUSEABLE)->EnableWindow(FALSE);
			}
			//Initialize right list 
			InitLevelTree(m_usersInfo->groupListEx[i].dwRightNum,m_usersInfo->groupListEx[i].rights);
			break;
		}
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Initialize right list 
void CAddUserInfo::InitLevelTree(DWORD dwNum, DWORD *dwList)
{
	m_treeLevels.DeleteAllItems();
	//Right information 
	int i = 0;
	for(i=0;i<(int)m_usersInfo->dwRightNum;i++)
	{
		CString str;
		str.Format("%d: %s : %s",m_usersInfo->rightList[i].dwID,m_usersInfo->rightList[i].name,
			m_usersInfo->rightList[i].memo);
		HTREEITEM hRoot = m_treeLevels.InsertItem(str,TVI_ROOT,TVI_LAST);
		m_treeLevels.SetItemData(hRoot,m_usersInfo->rightList[i].dwID);
	}

	//Update right list 
	int nCountItem = m_treeLevels.GetCount();
	HTREEITEM hItem = m_treeLevels.GetRootItem();
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
	
	// 只显示组所拥有的权限
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
}

BOOL CAddUserInfo::IsValid()
{
	BOOL bValid = TRUE;
	CString str1;
	GetDlgItemText(IDC_ADD_USER_EDIT_USERNAME,str1);
	if(str1.IsEmpty())
	{
		bValid = FALSE;
	}
	return bValid;
}