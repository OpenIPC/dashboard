// DispatchGroupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "DispatchGroupDlg.h"


// CDispatchGroupDlg dialog

IMPLEMENT_DYNAMIC(CDispatchGroupDlg, CDialog)

CDispatchGroupDlg::CDispatchGroupDlg(LLONG lLoginId, int nChnCount, NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CDispatchGroupDlg::IDD, pParent)
{
	m_lLoginID = lLoginId;
	m_nChnCount = nChnCount;
	memset(&m_stuDispatchGroupInfo, 0, sizeof(m_stuDispatchGroupInfo));
	memcpy(&m_stuDispatchGroupInfo, pstGroupInfo, sizeof(m_stuDispatchGroupInfo));
}

CDispatchGroupDlg::~CDispatchGroupDlg()
{
	CleanDispatchList();
}

BOOL CDispatchGroupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	SetDlgItemText(IDC_dispatch_groupId, m_stuDispatchGroupInfo.szGroupId);
	SetDlgItemText(IDC_dispatch_groupName, m_stuDispatchGroupInfo.szGroupName);

	CComboBox *pSimiarity = (CComboBox*)GetDlgItem(IDC_DISPATCH_SIMILARITY_COM);
	for(int i = 1; i <=100; i++)
	{
		CString str;
		str.Format("%d", i);
		pSimiarity->AddString(str);
	}
	pSimiarity->SetCurSel(79);

	SetDlgItemText(IDC_dispatch_similarity, "80");

	int nIndex = 0;
	int i = 0;
	m_comboDispatchChannel.ResetContent();
	for(i=1;i<= m_nChnCount;i++)
	{
		CString str;
		str.Format("%d",i);
		nIndex = m_comboDispatchChannel.AddString(str);
		m_comboDispatchChannel.SetItemData(nIndex,i);
	}
	m_comboDispatchChannel.SetCurSel(0);

	RefreshDispatchInfo(m_stuDispatchGroupInfo.szGroupId, TRUE);

	return TRUE;
}

void CDispatchGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_dispatch_channel, m_comboDispatchChannel);
	DDX_Control(pDX, IDC_LIST_DISPATCH, m_DispatchInfoList);
}


BEGIN_MESSAGE_MAP(CDispatchGroupDlg, CDialog)
	ON_BN_CLICKED(IDC_dispatch_add, &CDispatchGroupDlg::OnBnClickeddispatchadd)
	ON_BN_CLICKED(IDC_dispatch_delete, &CDispatchGroupDlg::OnBnClickeddispatchdelete)
	ON_BN_CLICKED(IDC_dispatch_dispatch, &CDispatchGroupDlg::OnBnClickeddispatchdispatch)
	ON_BN_CLICKED(IDC_dispatch_refresh, &CDispatchGroupDlg::OnBnClickeddispatchrefresh)
END_MESSAGE_MAP()


void CDispatchGroupDlg::CleanDispatchList()
{
	std::list<NET_DISPATCH_INFO*>::iterator it = m_lstDispatchChannelInfo.begin();
	while(it != m_lstDispatchChannelInfo.end())
	{
		NET_DISPATCH_INFO *pstInfo = *it;
		if (pstInfo)
		{
			m_lstDispatchChannelInfo.erase(it++);
			delete pstInfo;
			pstInfo = NULL;
		}
	}
}

void CDispatchGroupDlg::OnBnClickeddispatchadd()
{
	int nChannel = 0;

	int nIndex = m_comboDispatchChannel.GetCurSel();
	if(CB_ERR != nIndex)
	{
		nChannel = m_comboDispatchChannel.GetItemData(nIndex) - 1;
	}

	BOOL bNeedAddToList = TRUE;

	CComboBox *pSimilarity = (CComboBox*)GetDlgItem(IDC_DISPATCH_SIMILARITY_COM);
	int nSimilarity = pSimilarity->GetCurSel() + 1;

	nIndex = 0;
	std::list<NET_DISPATCH_INFO*>::iterator it = m_lstDispatchChannelInfo.begin();
	for (; it != m_lstDispatchChannelInfo.end(); ++it)
	{
		NET_DISPATCH_INFO *pstDispatchInfo = *it;
		if (pstDispatchInfo)
		{
			if (pstDispatchInfo->nChannel == nChannel)
			{
				bNeedAddToList = FALSE;
				if (nSimilarity == pstDispatchInfo->nSimilarity)
				{
					return;
				}

				pstDispatchInfo->nSimilarity = nSimilarity;
				pstDispatchInfo->bDispatch = FALSE;

				break;
			}
		}
		nIndex++;
	}

	if (TRUE == bNeedAddToList)
	{
		nIndex = m_DispatchInfoList.GetCount();
		NET_DISPATCH_INFO *pstDispatchInfo = new NET_DISPATCH_INFO;
		if (NULL == pstDispatchInfo)
		{
			MessageBox(ConvertString("Memory error"), "");
			return;
		}
		memset(pstDispatchInfo, 0, sizeof(*pstDispatchInfo));
		pstDispatchInfo->nChannel = nChannel;
		pstDispatchInfo->nSimilarity = nSimilarity;
		pstDispatchInfo->bDispatch = FALSE;
		m_lstDispatchChannelInfo.push_back(pstDispatchInfo);
	}

	CString str;
	str.Format("%4d                   %4d                %s", nChannel+1, nSimilarity, ConvertString("Not Dispatch"));
	m_DispatchInfoList.DeleteString(nIndex);
	m_DispatchInfoList.InsertString(nIndex, str);
}


NET_DISPATCH_INFO* CDispatchGroupDlg::SearchDispatchInfoByChannel(const int nChannel)
{
	std::list<NET_DISPATCH_INFO*>::iterator it = m_lstDispatchChannelInfo.begin();
	for (; it != m_lstDispatchChannelInfo.end(); ++it)
	{
		NET_DISPATCH_INFO *pstDispatchInfo = *it;
		if (pstDispatchInfo && (nChannel == pstDispatchInfo->nChannel))
		{
			return pstDispatchInfo;
		}
	}

	return NULL;
}

void CDispatchGroupDlg::DeleteDispatchInfoByChannel(const int nChannel)
{
	std::list<NET_DISPATCH_INFO*>::iterator it = m_lstDispatchChannelInfo.begin();
	for (; it != m_lstDispatchChannelInfo.end(); ++it)
	{
		NET_DISPATCH_INFO *pstDispatchInfo = *it;
		if (pstDispatchInfo && (nChannel == pstDispatchInfo->nChannel) )
		{
				m_lstDispatchChannelInfo.erase(it++);
				delete pstDispatchInfo;
				pstDispatchInfo = NULL;
				break;
		}
	}
}

void CDispatchGroupDlg::RefreshDispatchInfo(const char *szGroupId, BOOL bRefreshShow)
{
	int i = 0;
	BOOL bRet = FALSE;
	NET_IN_FIND_GROUP_INFO stuInParam = {sizeof(stuInParam)};
	NET_OUT_FIND_GROUP_INFO stuOutParam = {sizeof(stuOutParam)};
	stuOutParam.nMaxGroupNum = 50;
	NET_FACERECONGNITION_GROUP_INFO *pGroupInfo = NULL;
	stuOutParam.pGroupInfos = new NET_FACERECONGNITION_GROUP_INFO[stuOutParam.nMaxGroupNum];
	if (NULL == stuOutParam.pGroupInfos)
	{
		MessageBox(ConvertString("Memory error"), "");
		bRet = FALSE;
		goto e_clear;
	}

	memset(stuOutParam.pGroupInfos, 0, sizeof(NET_FACERECONGNITION_GROUP_INFO)*stuOutParam.nMaxGroupNum);
	for (i = 0; i < stuOutParam.nMaxGroupNum; i++)
	{
		pGroupInfo = stuOutParam.pGroupInfos + i;
		pGroupInfo->dwSize = sizeof(*pGroupInfo);
	}
	bRet = CLIENT_FindGroupInfo(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Failed to find group infos!"), "");
		bRet = FALSE;
		goto e_clear;
	}
	for (i = 0; i < stuOutParam.nRetGroupNum; i++)
	{
		pGroupInfo = stuOutParam.pGroupInfos + i;
		if (0 == strcmp(szGroupId, pGroupInfo->szGroupId))
		{
			memcpy(&m_stuDispatchGroupInfo, pGroupInfo, sizeof(m_stuDispatchGroupInfo));

		}
	}

	if (bRefreshShow)
	{
		CleanDispatchList();
		m_DispatchInfoList.ResetContent();
		for (i = 0; i < m_stuDispatchGroupInfo.nRetChnCount; i++)
		{
			CString str;
			str.Format("%4d                   %4d                %s", m_stuDispatchGroupInfo.nChannel[i]+1, m_stuDispatchGroupInfo.nSimilarity[i], ConvertString("Dispatch"));
			m_DispatchInfoList.AddString(str);
			NET_DISPATCH_INFO *pstDispatchInfo = new NET_DISPATCH_INFO;
			if (pstDispatchInfo)
			{
				memset(pstDispatchInfo, 0, sizeof(*pstDispatchInfo));
				pstDispatchInfo->nChannel = m_stuDispatchGroupInfo.nChannel[i];
				pstDispatchInfo->nSimilarity = m_stuDispatchGroupInfo.nSimilarity[i];
				pstDispatchInfo->bDispatch = TRUE;
				m_lstDispatchChannelInfo.push_back(pstDispatchInfo);
			}
		}
	}

e_clear:
	if (stuOutParam.pGroupInfos)
	{
		delete[] stuOutParam.pGroupInfos;
		stuOutParam.pGroupInfos = NULL;
	}
}

void CDispatchGroupDlg::OnBnClickeddispatchdelete()
{
	int i = 0;
	int nIndex = 0;
	int nResult = -2;
	CString str;
	char szNumber[5] = {0};
	int nChannel = 10;
	char *pcChannel = NULL;

	NET_IN_FACE_RECOGNITION_DEL_DISPOSITION_INFO stuInParam = {sizeof(stuInParam)};
	NET_OUT_FACE_RECOGNITION_DEL_DISPOSITION_INFO stuOutParam = {sizeof(stuOutParam)};

	strncpy_s(stuInParam.szGroupId,sizeof(stuInParam.szGroupId), m_stuDispatchGroupInfo.szGroupId, sizeof(stuInParam.szGroupId)-1);
	
	int nCount = m_DispatchInfoList.GetCount();
	for (i = 0; (i < nCount) && (nIndex < DH_MAX_CAMERA_CHANNEL_NUM); i++)
	{
		if (m_DispatchInfoList.GetSel(i))
		{
			m_DispatchInfoList.GetText(i, str);

			pcChannel = (LPSTR)(LPCSTR)str;
			sscanf(pcChannel, "%4d", &nChannel);
			nChannel = nChannel - 1;
			NET_DISPATCH_INFO *pstDispatchInfo = SearchDispatchInfoByChannel(nChannel);
			if (pstDispatchInfo)
			{
				stuInParam.nDispositionChn[nIndex] = pstDispatchInfo->nChannel;
				nIndex++;
			}
		}
	}

	if (nIndex > 0)
	{
		stuInParam.nDispositionChnNum = nIndex;
		BOOL bRet = CLIENT_FaceRecognitionDelDisposition(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
		if (!bRet)
		{
			MessageBox(ConvertString("Delete dispatch error!"), "");
			return;
		}

		for (i = 0; i < stuOutParam.nReportCnt; i++)
		{
			if (TRUE == stuOutParam.bReport[i])
			{
				DeleteDispatchInfoByChannel(stuInParam.nDispositionChn[i]);

				int nListCount = m_DispatchInfoList.GetCount();
				for (int j = 0; j < nListCount; j++)
				{
					m_DispatchInfoList.GetText(j, str);

					pcChannel = (LPSTR)(LPCSTR)str;
					sscanf(pcChannel, "%4d", &nChannel);
					if (stuInParam.nDispositionChn[i] == nChannel-1)
					{
						m_DispatchInfoList.DeleteString(j);
						break;
					}
				}
			}
		}

		RefreshDispatchInfo(m_stuDispatchGroupInfo.szGroupId);
	}
}

void CDispatchGroupDlg::OnBnClickeddispatchdispatch()
{
	int nCount = 0;
	NET_IN_FACE_RECOGNITION_PUT_DISPOSITION_INFO stuInParam = {sizeof(stuInParam)};
	NET_OUT_FACE_RECOGNITION_PUT_DISPOSITION_INFO stuOutParam = {sizeof(stuOutParam)};

	strncpy(stuInParam.szGroupId, m_stuDispatchGroupInfo.szGroupId, sizeof(stuInParam.szGroupId)-1);
	std::list<NET_DISPATCH_INFO*>::iterator it = m_lstDispatchChannelInfo.begin();
	for (; (it != m_lstDispatchChannelInfo.end()) && (nCount < DH_MAX_CAMERA_CHANNEL_NUM); ++it)
	{
		NET_DISPATCH_INFO *pstDispatchInfo = *it;
		if (pstDispatchInfo && (FALSE == pstDispatchInfo->bDispatch))
		{
			stuInParam.stuDispositionChnInfo[nCount].nChannelID = pstDispatchInfo->nChannel;
			stuInParam.stuDispositionChnInfo[nCount].nSimilary = pstDispatchInfo->nSimilarity;
			nCount++;
		}
	}

	if (0 < nCount)
	{
		stuInParam.nDispositionChnNum = nCount;
		BOOL bRet = CLIENT_FaceRecognitionPutDisposition(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
		if (!bRet)
		{
			MessageBox(ConvertString("Dispatch error!"), "");
			return;
		}

		for (int i = 0; i < stuOutParam.nReportCnt; i++)
		{
			if (TRUE == stuOutParam.bReport[i]) // 表示布控成功
			{
				int nListCount = m_DispatchInfoList.GetCount();
				for (int j = 0; j < nListCount; j++)
				{
					int nChannel = 10;
					CString str;
					m_DispatchInfoList.GetText(j, str);
					char *pcChannel = (LPSTR)(LPCSTR)str;
					sscanf(pcChannel, "%4d", &nChannel);
					nChannel = nChannel - 1;
					if (nChannel == stuInParam.stuDispositionChnInfo[i].nChannelID)
					{
						m_DispatchInfoList.DeleteString(j);
						str.Format("%4d                   %4d                %s", stuInParam.stuDispositionChnInfo[i].nChannelID+1, stuInParam.stuDispositionChnInfo[i].nSimilary, ConvertString("Dispatch"));
						m_DispatchInfoList.InsertString(j, str);
					}
				}
			}
		}

		RefreshDispatchInfo(m_stuDispatchGroupInfo.szGroupId);
	}
}

void CDispatchGroupDlg::OnBnClickeddispatchrefresh()
{
	RefreshDispatchInfo(m_stuDispatchGroupInfo.szGroupId, TRUE);
}


BOOL CDispatchGroupDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
