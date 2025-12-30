// DispatchGroupChannelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "DispatchGroupChannelDlg.h"


// CDispatchGroupChannelDlg dialog

IMPLEMENT_DYNAMIC(CDispatchGroupChannelDlg, CDialog)

CDispatchGroupChannelDlg::CDispatchGroupChannelDlg(LLONG lLoginId, int nChannel, CWnd* pParent /*=NULL*/)
: CDialog(CDispatchGroupChannelDlg::IDD, pParent)
{
	m_lLoginID = lLoginId;
	m_nChannel = nChannel;
	m_bNeedClearItem = TRUE;
	m_nStateDlg = 0;
}

CDispatchGroupChannelDlg::~CDispatchGroupChannelDlg()
{
	ClearDispatchGroupInfoList();
}

void CDispatchGroupChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DISPATCH, m_DispatchInfoList);
}


BEGIN_MESSAGE_MAP(CDispatchGroupChannelDlg, CDialog)
	ON_BN_CLICKED(IDC_dispatch_add, &CDispatchGroupChannelDlg::OnBnClickeddispatchadd)
	ON_BN_CLICKED(IDC_dispatch_dispatch, &CDispatchGroupChannelDlg::OnBnClickeddispatchdispatch)
	ON_BN_CLICKED(IDC_DISPATCH_FORCHANNEL_DELETE, &CDispatchGroupChannelDlg::OnBnClickedDispatchForchannelDelete)
	ON_BN_CLICKED(IDC_DISPATCH_FORCHANNEL_REFRESH, &CDispatchGroupChannelDlg::OnBnClickedDispatchForchannelRefresh)
END_MESSAGE_MAP()


BOOL CDispatchGroupChannelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	RefreshDispatchInfoListBox();
	InitComboxGroupID();

	CString str = "";

	CComboBox *pSimiarity = (CComboBox*)GetDlgItem(IDC_DISPATCHCHANNEL_SIMILARITY_COM);
	for(int i = 1; i <=100; i++)
	{
		str.Format("%d", i);
		pSimiarity->AddString(str);
	}

	pSimiarity->SetCurSel(79);

	str.Format("%d", m_nChannel);
	SetDlgItemText(IDC_DISPATCHCHANNEL_CHANNEL, str);

	GetDlgItem(IDC_dispatch_dispatch)->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDispatchGroupChannelDlg::OnBnClickeddispatchadd()
{
	m_nStateDlg = 1;
	if (m_bNeedClearItem == TRUE)
	{
		m_DispatchInfoList.ResetContent();
	}

	CComboBox *pSimilarity = (CComboBox*)GetDlgItem(IDC_DISPATCHCHANNEL_SIMILARITY_COM);
	int nSimilarity = pSimilarity->GetCurSel() + 1;
	
	int nNum = 0;
	CComboBox *pComboxGroupId = (CComboBox*)GetDlgItem(IDC_DISPATCHCHANNEL_GROUPID);
	int nIndex = pComboxGroupId->GetCurSel();
	if(CB_ERR != nIndex)
	{
		std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
		while(iter != m_lsDispatchGroupInfo.end())
		{
			if (nIndex == nNum++)
			{
				NET_DISPATGROUP_INFO *pInfo = *iter;
				
				if (pInfo->bAddToList == FALSE)
				{
					pInfo->bAddToList = TRUE;
					pInfo->nSimilarity = nSimilarity;
					CString str;
					str.Format("%s                   %4d                %s", pInfo->szGroupID, pInfo->nSimilarity, ConvertString("Not Dispatch"));

					int nCount = m_DispatchInfoList.GetCount();
					m_DispatchInfoList.InsertString(nCount, str);
					break;
				}
			}
			iter++;
		}
	}
	m_bNeedClearItem = FALSE;
	GetDlgItem(IDC_dispatch_dispatch)->EnableWindow(TRUE);
}

void CDispatchGroupChannelDlg::InitComboxGroupID()
{
	CComboBox *pComboxGroupId = (CComboBox*)GetDlgItem(IDC_DISPATCHCHANNEL_GROUPID);
	std::list<NET_DISPATGROUP_INFO*>::iterator it = m_lsDispatchGroupInfo.begin();
	for (; it != m_lsDispatchGroupInfo.end(); it++)
	{
		CString str = "";
		str.Format("%s", (*it)->szGroupID);
		pComboxGroupId->AddString(str);
	}
	pComboxGroupId->SetCurSel(0);
}

void CDispatchGroupChannelDlg::OnBnClickeddispatchdispatch()
{
	DispatchListGroupID();
	RefreshDispatchInfoListBox();
	
	m_nStateDlg = 0;
	m_bNeedClearItem = TRUE;

	GetDlgItem(IDC_dispatch_dispatch)->EnableWindow(FALSE);
}

void CDispatchGroupChannelDlg::DispatchListGroupID()
{
	NET_IN_SET_GROUPINFO_FOR_CHANNEL stuInParam;
	NET_OUT_SET_GROUPINFO_FOR_CHANNEL strOutParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	memset(&strOutParam, 0, sizeof(strOutParam));

	stuInParam.dwSize = sizeof(NET_IN_SET_GROUPINFO_FOR_CHANNEL);
	strOutParam.dwSize = sizeof(NET_OUT_SET_GROUPINFO_FOR_CHANNEL);
	stuInParam.nChannelID = m_nChannel - 1;

	int nGroupNum = 0;

	std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
	for (; iter != m_lsDispatchGroupInfo.end(); iter++)
	{
		NET_DISPATGROUP_INFO *pInfo = *iter;
		if (pInfo)
		{
			if (pInfo->bNeedRevoke)
			{
				pInfo->bAddToList = FALSE;
				pInfo->bNeedRevoke = FALSE;
			}

			pInfo->bDispatch = FALSE;
			if (pInfo && pInfo->bAddToList == TRUE)
			{
				//add
				pInfo->bAddToList = FALSE;

				strncpy(stuInParam.szGroupId[nGroupNum], pInfo->szGroupID, DH_COMMON_STRING_64 - 1);
				stuInParam.nSimilary[nGroupNum] = pInfo->nSimilarity;
				++nGroupNum;
			}
		}
	}
	if (nGroupNum == 0)
	{
		stuInParam.nGroupIdNum = 1;
		stuInParam.nSimilaryNum = 1;
	}
	else
	{
		stuInParam.nGroupIdNum = nGroupNum;
		stuInParam.nSimilaryNum = nGroupNum;
	}

	BOOL bRet = CLIENT_SetGroupInfoForChannel(m_lLoginID, &stuInParam, &strOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Dispatch error!"), "");
	}
}

void CDispatchGroupChannelDlg::ClearDispatchGroupInfoList()
{
	std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
	while(iter != m_lsDispatchGroupInfo.end())
	{
		NET_DISPATGROUP_INFO *pInfo = *iter;
		if (pInfo)
		{
			m_lsDispatchGroupInfo.erase(iter++);
			delete pInfo;
			pInfo = NULL;
		}
	}
}

void CDispatchGroupChannelDlg::RefreshDispatchInfoListBox()
{	
	m_DispatchInfoList.ResetContent();
	ClearDispatchGroupInfoList();

	int i = 0;

	NET_IN_FIND_GROUP_INFO stuInParam = {sizeof(stuInParam)};
	NET_OUT_FIND_GROUP_INFO stuOutParam = {sizeof(stuOutParam)};
	stuOutParam.nMaxGroupNum = 50;
	NET_FACERECONGNITION_GROUP_INFO *pGroupInfo = NULL;
	stuOutParam.pGroupInfos = new NET_FACERECONGNITION_GROUP_INFO[stuOutParam.nMaxGroupNum];

	if (NULL == stuOutParam.pGroupInfos)
	{
		MessageBox(ConvertString("Memory error"), "");
	}

	memset(stuOutParam.pGroupInfos, 0, sizeof(NET_FACERECONGNITION_GROUP_INFO)*stuOutParam.nMaxGroupNum);
	for (int i = 0; i < stuOutParam.nMaxGroupNum; i++)
	{
		pGroupInfo = stuOutParam.pGroupInfos + i;
		pGroupInfo->dwSize = sizeof(*pGroupInfo);
	}
	BOOL bRet = CLIENT_FindGroupInfo(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Failed to find group infos!"), "");
	}
	else
	{
		int nGroupNum = stuOutParam.nRetGroupNum;
		for (int i = 0; i < nGroupNum; i++)
		{
			NET_FACERECONGNITION_GROUP_INFO *pInfoTemp = &stuOutParam.pGroupInfos[i];

			NET_DISPATGROUP_INFO *pGroupInfo = new NET_DISPATGROUP_INFO;
			memset(pGroupInfo, 0, sizeof(NET_DISPATGROUP_INFO));
			pGroupInfo->bAddToList = FALSE;
			pGroupInfo->nSimilarity = 0;
			pGroupInfo->bNeedRevoke = FALSE;
			strncpy(pGroupInfo->szGroupID, pInfoTemp->szGroupId, DH_COMMON_STRING_64 - 1);

			int nRetChannelCount = pInfoTemp->nRetChnCount;
			for (int n = 0; n < nRetChannelCount; n++)
			{
				if (m_nChannel == (pInfoTemp->nChannel[n] + 1))
				{
					pGroupInfo->nSimilarity = pInfoTemp->nSimilarity[n];
					pGroupInfo->bDispatch = TRUE;

					CString str;
					str.Format("%s                   %4d                %s", pGroupInfo->szGroupID, pGroupInfo->nSimilarity, ConvertString("Dispatch"));
					m_DispatchInfoList.AddString(str);
				}
			}
			m_lsDispatchGroupInfo.push_back(pGroupInfo);
		}
	}

	if (stuOutParam.pGroupInfos)
	{
		delete[] stuOutParam.pGroupInfos;
		stuOutParam.pGroupInfos = NULL;
	}
}

void CDispatchGroupChannelDlg::OnBnClickedDispatchForchannelDelete()
{
	char *pGroupID = NULL;
	char szGroupID[DH_COMMON_STRING_64] = {0};
	CString str = "";
	int nCount = m_DispatchInfoList.GetCount();
	BOOL bIsDeleteDispatchInfo = FALSE;

	int nIndex = 0;
	for (int i = 0; i < nCount; i++)
	{
		int nSel = m_DispatchInfoList.GetSel(i);

		if (nSel > 0)
		{
			m_DispatchInfoList.GetText(i, str);
			
			pGroupID = (LPSTR)(LPCSTR)str;
			sscanf_s(pGroupID, "%s", szGroupID, sizeof(szGroupID));

			if (IsGroupIDDispatch(szGroupID))
			{
				RevokeDispatchGroupID(szGroupID);
				bIsDeleteDispatchInfo = TRUE;
				nIndex++;
			}
			else
			{
				DeleteDispatchInfoByGroupID(szGroupID);
				bIsDeleteDispatchInfo = FALSE;
				
				nIndex++;
			}
		}
	}

	if (nIndex > 0)
	{
		if (bIsDeleteDispatchInfo == FALSE)
		{
			m_DispatchInfoList.ResetContent();
			std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
			while(iter != m_lsDispatchGroupInfo.end())
			{
				NET_DISPATGROUP_INFO *pInfo = *iter++;
				if (pInfo && pInfo->bAddToList == TRUE)
				{
					CString str;
					str.Format("%s                   %4d                %s", pInfo->szGroupID, pInfo->nSimilarity, ConvertString("Not Dispatch"));
					m_DispatchInfoList.AddString(str);
				}
			}
		}
		else
		{
			DispatchListGroupID();
			RefreshDispatchInfoListBox();
		}
	}
}

void CDispatchGroupChannelDlg::DeleteDispatchInfoByGroupID(char *pGroupID)
{
	std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
	while(iter != m_lsDispatchGroupInfo.end())
	{
		NET_DISPATGROUP_INFO *pInfo = *iter++;
		if (pInfo && strncmp(pInfo->szGroupID, pGroupID, sizeof(pInfo->szGroupID)) == 0)
		{
			pInfo->bAddToList = FALSE;
		}
	}
	GetDlgItem(IDC_dispatch_dispatch)->EnableWindow(TRUE);
}

void CDispatchGroupChannelDlg::OnBnClickedDispatchForchannelRefresh()
{
	m_bNeedClearItem = TRUE;
	m_nStateDlg = 0;
	RefreshDispatchInfoListBox();
}

void CDispatchGroupChannelDlg::RevokeDispatchGroupID(char *pGroupID)
{
	std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
	while(iter != m_lsDispatchGroupInfo.end())
	{
		NET_DISPATGROUP_INFO *pInfo = *iter++;

		if (pInfo &&  pInfo->bDispatch )
		{
			pInfo->bAddToList = TRUE;
			if (strncmp(pInfo->szGroupID, pGroupID, sizeof(pInfo->szGroupID)) == 0 )
			{
				pInfo->bNeedRevoke = TRUE;
			}
		}
	}
}

BOOL CDispatchGroupChannelDlg::IsGroupIDDispatch(char *pGroupID)
{
	BOOL bRet = FALSE;
	std::list<NET_DISPATGROUP_INFO*>::iterator iter = m_lsDispatchGroupInfo.begin();
	while(iter != m_lsDispatchGroupInfo.end())
	{
		NET_DISPATGROUP_INFO *pInfo = *iter++;

		if ( pInfo->bDispatch 
				&& strncmp(pInfo->szGroupID, pGroupID, sizeof(pInfo->szGroupID)) == 0
				&& m_nStateDlg == 0)
		{
			bRet = TRUE;
			break;
		}
	}
	return bRet;
}
BOOL CDispatchGroupChannelDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
