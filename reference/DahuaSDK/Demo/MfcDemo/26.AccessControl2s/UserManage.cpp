// UserManage.cpp : 实现文件
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "UserManage.h"
#include "UserInfo.h"

#define WM_UI_LIST_LOAD		(WM_USER + 300)
#define WM_UI_CTRL_ENABLE	(WM_USER + 301)

DWORD WINAPI DoFindUserInfo(LPVOID lpParam)
{
	if (lpParam == 0)
	{
		return 0;
	}

	UserManage *dlg = (UserManage*)lpParam;
	if (dlg)
	{
		dlg->DoFindNextUserRecord();
	}
	return 1;
}

IMPLEMENT_DYNAMIC(UserManage, CDialog)

UserManage::UserManage(CWnd* pParent /*=NULL*/)
	: CDialog(UserManage::IDD, pParent)
{
	hParentWnd = pParent->GetSafeHwnd();
	m_bIsDoFindNext = FALSE;
	m_dwThreadID = 0;
	m_nUserInfoIndex = -1;
	m_ThreadHandle = 0;
	pUserInfoDlg = NULL;
}

UserManage::~UserManage()
{
	m_bIsDoFindNext = FALSE;
	if (m_ThreadHandle)
	{
		int dwRet = WaitForSingleObject(m_ThreadHandle, INFINITE);
		if (dwRet == WAIT_OBJECT_0)
		{
			CloseHandle(m_ThreadHandle);
			m_ThreadHandle = 0;
		}
	}
}

void UserManage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL_USER, m_list_user);
}


BEGIN_MESSAGE_MAP(UserManage, CDialog)
	//{{AFX_MSG_MAP(CAccessControlDlg)
	ON_MESSAGE(WM_UI_LIST_LOAD, &UserManage::ListLoad)
	ON_MESSAGE(WM_UI_CTRL_ENABLE, &UserManage::CtrlEnable)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_BUT_USER_ADD, &UserManage::OnBnClickedButUserAdd)
	ON_BN_CLICKED(IDC_BUT_USER_MODIFY, &UserManage::OnBnClickedButUserModify)
	ON_BN_CLICKED(IDC_BUT_USER_DELETE, &UserManage::OnBnClickedButUserDelete)
	ON_BN_CLICKED(IDC_BUT_USER_GET, &UserManage::OnBnClickedButUserGet)
	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_USER, &UserManage::OnNMClickListctrlUser)
END_MESSAGE_MAP()



BOOL UserManage::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
 	if (Device::GetInstance().GetLoginState())
 	{
 		OnBnClickedButUserGet();
 	}
 	else
 	{
 		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		UIState(FALSE);
 	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void UserManage::InitDlg()
{
	m_list_user.SetExtendedStyle(m_list_user.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_list_user.SetExtendedStyle(m_list_user.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("NO.", 60));
	vecTitles.push_back(ColDesObj("UserID", 60));
	vecTitles.push_back(ColDesObj("Name", 100));
	vecTitles.push_back(ColDesObj("Type", 120));

	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_list_user.InsertColumn(j, &lvc);
	}
}

void UserManage::OnBnClickedButUserGet()
{
	UIState(FALSE);
	m_ThreadHandle = CreateThread(NULL, 0, DoFindUserInfo, this, 0, (LPDWORD)&m_dwThreadID);
}

void UserManage::OnBnClickedButUserAdd()
{
	UIState(FALSE);
	//UserInfo dlg(this);
	pUserInfoDlg = new UserInfo(this);
	if (NULL != pUserInfoDlg)
	{
		pUserInfoDlg->SetDlgMode(1);
		if (IDOK == pUserInfoDlg->DoModal())
		{
			const NET_ACCESS_USER_INFO* pstuUserInfo = pUserInfoDlg->GetUserInfo();
			if (pstuUserInfo != NULL)
			{
				NET_ACCESS_USER_INFO stuUserInfoAdd;
				memset(&stuUserInfoAdd,0,sizeof(NET_ACCESS_USER_INFO));
				memcpy(&stuUserInfoAdd,pstuUserInfo,sizeof(NET_ACCESS_USER_INFO));
				m_UserInfoVector.push_back(stuUserInfoAdd);

				NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
				BOOL bret = Device::GetInstance().UserInsert(1, &stuUserInfoAdd, stuFailCode);
				if (bret)
				{
					OnBnClickedButUserGet();
					//进入修改页面 去添加人脸、指纹、卡信息
					pUserInfoDlg->SetDlgMode(2);
					pUserInfoDlg->SetDlgHWND(hParentWnd);
					pUserInfoDlg->SetUserInfo(&stuUserInfoAdd);
					if (IDOK == pUserInfoDlg->DoModal())
					{
						const NET_ACCESS_USER_INFO* pstuUserInfo = pUserInfoDlg->GetUserInfo();
						if (pstuUserInfo != NULL)
						{
							memcpy(&stuUserInfoAdd, pstuUserInfo, sizeof(NET_ACCESS_USER_INFO));

							NET_EM_FAILCODE stuFailCodeI = NET_EM_FAILCODE_NOERROR;
							BOOL bretI = Device::GetInstance().UserInsert(1, &stuUserInfoAdd, stuFailCodeI);
							if (bretI)
							{
								OnBnClickedButUserGet();
							}
							else
							{
								if(stuFailCodeI != NET_EM_FAILCODE_UNKNOWN && stuFailCodeI != NET_EM_FAILCODE_NOERROR)
								{
									CString csInfo;
									csInfo.Format("%s", FailCodeType2Str(stuFailCodeI).c_str());	
									MessageBox(csInfo, ConvertString("Prompt"));
								}
								UIState(TRUE);
							}
						}
						else
						{
							UIState(TRUE);
						}
					}
					else
					{
						UIState(TRUE);
					}

				}
				else
				{
					if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
					{
						CString csInfo;
						csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
						MessageBox(csInfo, ConvertString("Prompt"));
					}
					UIState(TRUE);
				}
			}
			else
			{
				UIState(TRUE);
			}
		}
		else
		{
			UIState(TRUE);
		}

		delete pUserInfoDlg;
		pUserInfoDlg = NULL;
	}
	else
	{
		UIState(TRUE);
	}
}

void UserManage::OnBnClickedButUserModify()
{
	if (m_nUserInfoIndex == -1)
	{
		return;
	}
	UIState(FALSE);
	//UserInfo dlg(this);
	pUserInfoDlg = new UserInfo(this);
	if (NULL != pUserInfoDlg)
	{
		pUserInfoDlg->SetDlgMode(2);
		pUserInfoDlg->SetDlgHWND(hParentWnd);
		pUserInfoDlg->SetUserInfo(&m_UserInfoVector[m_nUserInfoIndex]);
		if (IDOK == pUserInfoDlg->DoModal())
		{
			const NET_ACCESS_USER_INFO* pstuUserInfo = pUserInfoDlg->GetUserInfo();
			if (pstuUserInfo != NULL)
			{
				memcpy(&m_UserInfoVector[m_nUserInfoIndex], pstuUserInfo, sizeof(NET_ACCESS_USER_INFO));

				NET_EM_FAILCODE stuFailCodeI = NET_EM_FAILCODE_NOERROR;
				BOOL bretI = Device::GetInstance().UserInsert(1, &m_UserInfoVector[m_nUserInfoIndex], stuFailCodeI);
				if (bretI)
				{
					OnBnClickedButUserGet();
				}
				else
				{
					if(stuFailCodeI != NET_EM_FAILCODE_UNKNOWN && stuFailCodeI != NET_EM_FAILCODE_NOERROR)
					{
						CString csInfo;
						csInfo.Format("%s", FailCodeType2Str(stuFailCodeI).c_str());	
						MessageBox(csInfo, ConvertString("Prompt"));
					}
					UIState(TRUE);
				}
			}
			else
			{
				UIState(TRUE);
			}
		}
		else
		{
			UIState(TRUE);
		}

		delete pUserInfoDlg;
		pUserInfoDlg = NULL;
	}
	else
	{
		UIState(TRUE);
	}
}

void UserManage::OnBnClickedButUserDelete()
{
	if (m_nUserInfoIndex == -1)
	{
		return;
	}
	UIState(FALSE);
	NET_IN_ACCESS_USER_SERVICE_REMOVE stuUserRemoveIn = {sizeof(stuUserRemoveIn)};
	stuUserRemoveIn.nUserNum = 1;
	NET_ACCESS_USER_INFO stuUserInfo;
	memset(&stuUserInfo,0,sizeof(NET_ACCESS_USER_INFO));
	memcpy(&stuUserInfo,&m_UserInfoVector[m_nUserInfoIndex],sizeof(NET_ACCESS_USER_INFO));
	strncpy(stuUserRemoveIn.szUserID[0],stuUserInfo.szUserID,DH_MAX_USERID_LEN-1);

	NET_OUT_ACCESS_USER_SERVICE_REMOVE stuUserRemoveOut = {sizeof(stuUserRemoveOut)};
	NET_EM_FAILCODE stuFailCodeR = NET_EM_FAILCODE_NOERROR;
	stuUserRemoveOut.nMaxRetNum = 1;
	stuUserRemoveOut.pFailCode = &stuFailCodeR;
	BOOL bretR = Device::GetInstance().UserRemove(&stuUserRemoveIn, stuUserRemoveOut);
	if (bretR)
	{
		OnBnClickedButUserGet();
		m_nUserInfoIndex = -1;
	}
	else
	{
		if(stuFailCodeR != NET_EM_FAILCODE_UNKNOWN && stuFailCodeR != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCodeR).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
		UIState(TRUE);
	}
}

void UserManage::DoFindNextUserRecord()
{
	BOOL bUserFind = Device::GetInstance().UserStartFind();
	if (bUserFind)
	{
		m_UserInfoVector.clear();
		//
		int nStartNo = 0;
		m_bIsDoFindNext = TRUE;
		while (m_bIsDoFindNext)
		{
			NET_ACCESS_USER_INFO* pUserInfo = new NET_ACCESS_USER_INFO[10];
			if (pUserInfo)
			{
				memset(pUserInfo,0,sizeof(NET_ACCESS_USER_INFO)*10);
				int nRecordNum = 0;
				m_bIsDoFindNext = Device::GetInstance().UserFindNext(nStartNo,10,pUserInfo,nRecordNum);

				for (int i=0;i<nRecordNum;i++)
				{
					NET_ACCESS_USER_INFO stuUserInfo;
					memset(&stuUserInfo,0,sizeof(NET_ACCESS_USER_INFO));
					memcpy(&stuUserInfo,&pUserInfo[i],sizeof(NET_ACCESS_USER_INFO));
					m_UserInfoVector.push_back(stuUserInfo);
				}
				nStartNo += nRecordNum;

				delete []pUserInfo;
				pUserInfo = NULL;
			}
			else
			{
				m_bIsDoFindNext = FALSE;
			}
		}

		bUserFind = Device::GetInstance().UserStopFind();
		//ListReload();
		::PostMessage(this->GetSafeHwnd(), WM_UI_LIST_LOAD, NULL, NULL);
	}
	CloseHandle(m_ThreadHandle);
	m_ThreadHandle = 0;
	//UIState(TRUE);
	::PostMessage(this->GetSafeHwnd(), WM_UI_CTRL_ENABLE, NULL, NULL);
}

void UserManage::ListReload()
{
	if (m_list_user)
	{

		m_list_user.SetRedraw(FALSE);
		m_list_user.DeleteAllItems();
		for (int i = 0; i < m_UserInfoVector.size(); i++)
		{
			NET_ACCESS_USER_INFO stuUserInfo;
			memset(&stuUserInfo,0,sizeof(NET_ACCESS_USER_INFO));
			memcpy(&stuUserInfo,&m_UserInfoVector[i],sizeof(NET_ACCESS_USER_INFO));

			CString strIndex;
			strIndex.Format("%d",i + 1);
			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = i;
			m_list_user.InsertItem(&lvi);
			m_list_user.SetItemText(i, 0, strIndex);
			CString str;
			str.Format("%s",stuUserInfo.szUserID);
			m_list_user.SetItemText(i, 1, str);
			str.Format("%s",stuUserInfo.szName);
			m_list_user.SetItemText(i, 2, str);
			std::string dstr = UserType2Str(stuUserInfo.emUserType);
			m_list_user.SetItemText(i, 3, dstr.c_str());
		}
		m_list_user.SetRedraw(TRUE);
	}
}

void UserManage::OnNMClickListctrlUser(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pNMHDR)
	{
		HD_NOTIFY *phdn = (HD_NOTIFY*)pNMHDR;
		m_nUserInfoIndex = phdn->iItem;
	}
	*pResult = 0;
}

void UserManage::UIState(BOOL b)
{
	if (CWnd::GetSafeHwnd() )
	{
		GetDlgItem(IDC_BUT_USER_GET)->EnableWindow(b);
		GetDlgItem(IDC_BUT_USER_ADD)->EnableWindow(b);
		GetDlgItem(IDC_BUT_USER_MODIFY)->EnableWindow(b);
		GetDlgItem(IDC_BUT_USER_DELETE)->EnableWindow(b);
	}
}
LRESULT UserManage::ListLoad(WPARAM wParam, LPARAM lParam)
{
	ListReload();
	return 0;
}
LRESULT UserManage::CtrlEnable(WPARAM wParam, LPARAM lParam)
{
	UIState(TRUE);
	return 0;
}