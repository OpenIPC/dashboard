// FaceDbOpreate.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "FaceDbOpreate.h"
#include "AddGroupDlg.h"
#include "PersonOpreateDlg.h"
#include "DispatchGroupDlg.h"
#include "SearchByPic.h"
#include "DispatchGroupChannelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFaceDbOpreate dialog
CFaceDbOpreate::CFaceDbOpreate(LLONG lLoginID, const int nChnCount, char *pSoftPath /*=NULL*/, CWnd* pParent /*=NULL*/)
	: CDialog(CFaceDbOpreate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaceDbOpreate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginID = lLoginID;
	m_nChnCount = nChnCount;
	m_pstSelectGroup = NULL;
	m_pSoftPath = NULL;
	if (pSoftPath)
	{
		m_pSoftPath = pSoftPath;
	}
	m_nSelectId = 0;
	m_nGroupCnt = 0;
	m_nGroupPos = 0;
}

CFaceDbOpreate::~CFaceDbOpreate()
{
	CleanGroupList();

	if (m_pstSelectGroup)
	{
		delete m_pstSelectGroup;
		m_pstSelectGroup = NULL;
	}
}

BOOL CFaceDbOpreate::GetGroupInfos()
{
	unsigned int i = 0;
	BOOL bRet = TRUE;
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
		COMMON_GROUP_INFO *pGroupInfoTmp = new COMMON_GROUP_INFO;
		if (NULL == pGroupInfoTmp)
		{
			MessageBox(ConvertString("Memory error"), "");
			bRet = FALSE;
			goto e_clear;
		}
		memset(pGroupInfoTmp, 0, sizeof(*pGroupInfoTmp));
		pGroupInfoTmp->stuGroupInfo.dwSize = sizeof(pGroupInfoTmp->stuGroupInfo);
		memcpy(&(pGroupInfoTmp->stuGroupInfo), pGroupInfo, sizeof(pGroupInfoTmp->stuGroupInfo));
		pGroupInfoTmp->nIndex = IDC_RADIO_group1 + i;
		m_lstGroupInfo.push_back(pGroupInfoTmp);
	}

	m_nGroupCnt = stuOutParam.nRetGroupNum; // total number of face libraries

e_clear:
	if (stuOutParam.pGroupInfos)
	{
		delete[] stuOutParam.pGroupInfos;
		stuOutParam.pGroupInfos = NULL;
	}
	return bRet;
}

void CFaceDbOpreate::ShowGroupInfo()
{
	unsigned int nIndex = 0;
	int i = IDC_RADIO_group1 + m_nGroupPos;
	int nMaxIndex = i + 10;
	CString str;
	std::list<COMMON_GROUP_INFO*>::iterator it = m_lstGroupInfo.begin();
	for(; (it != m_lstGroupInfo.end()) && (i < nMaxIndex); ++it)
	{
		if (*it)
		{

			COMMON_GROUP_INFO *pstGroupInfo = (*it);
			if (pstGroupInfo && (pstGroupInfo->nIndex == i))
			{
				int nId = IDC_DB_NAME1+nIndex;
				str.Format("%s", pstGroupInfo->stuGroupInfo.szGroupName);
				SetDlgItemText(nId, str);

				nId = IDC_GROUP_ID1+nIndex;
				str.Format("%s", pstGroupInfo->stuGroupInfo.szGroupId);
				SetDlgItemText(nId, str);

				nId = IDC_PERSON_NUM1 + nIndex;
				str.Format("%d", pstGroupInfo->stuGroupInfo.nGroupSize);
				SetDlgItemText(nId, str);

				nId = IDC_RADIO_group1 + nIndex;
				GetDlgItem(nId)->EnableWindow(TRUE);

				nIndex++;
				i++;
			}
		}	
	}
}

BOOL CFaceDbOpreate::OnInitDialog()
{
	BOOL bRet = FALSE;
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	GetDlgItem(IDC_group_PerPage)->EnableWindow(FALSE);
	GetDlgItem(IDC_group_NextPage)->EnableWindow(FALSE);

	ClearShowGroupInfo();
	if (bRet = GetGroupInfos())
	{
		if (m_nGroupCnt > PERPAGE_INDEX_COUNT)
		{
			GetDlgItem(IDC_group_NextPage)->EnableWindow(TRUE);
		}
		m_nGroupPos = 0;
		ShowGroupInfo();

		((CButton*)GetDlgItem(IDC_RADIO_group1))->SetCheck(1);

		OnSelectGroup();
	}

	CComboBox* ComboChannel = NULL;
	for(int i = 0; i < m_nChnCount; i++)
	{
		ComboChannel = (CComboBox*)GetDlgItem(IDC_COMBO_DISPATCH_CHANNEL);
		CString str = "";
		str.Format("%d", i + 1);
		ComboChannel->AddString(str);
	}

	return bRet;  // return TRUE  unless you set the focus to a control
}

void CFaceDbOpreate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaceDbOpreate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFaceDbOpreate, CDialog)
	//{{AFX_MSG_MAP(CFaceDbOpreate)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_Edit, OnBUTTONEdit)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonPersonOpreate)
	ON_BN_CLICKED(IDC_BUTTON_DISPATCH, OnButtonDispatch)
	ON_BN_CLICKED(IDC_RADIO_group1, OnSelectGroup)

	ON_BN_CLICKED(IDC_group_PerPage, OngroupPerPage)
	ON_BN_CLICKED(IDC_RADIO_group2, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group3, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group4, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group5, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group6, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group7, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group8, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group9, OnSelectGroup)
	ON_BN_CLICKED(IDC_RADIO_group10, OnSelectGroup)
	ON_BN_CLICKED(IDC_group_NextPage, OngroupNextPage)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_DISPATCH_CHANNEL, &CFaceDbOpreate::OnBnClickedButtonDispatchChannel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFaceDbOpreate message handlers

void CFaceDbOpreate::OnButtonDelete() 
{
	if (NULL == m_pstSelectGroup)
	{
		MessageBox(ConvertString("Please select a group!"), "");
		return;
	}

	NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo = &(m_pstSelectGroup->stuGroupInfo);

	NET_IN_OPERATE_FACERECONGNITION_GROUP stuInParam = {sizeof(stuInParam)};
	NET_OUT_OPERATE_FACERECONGNITION_GROUP stuOutParam = {sizeof(stuOutParam)};
	stuInParam.emOperateType = NET_FACERECONGNITION_GROUP_DELETE;
	NET_DELETE_FACERECONGNITION_GROUP_INFO stuDeleteInfo = {sizeof(stuDeleteInfo)};
	strncpy(stuDeleteInfo.szGroupId, pstGroupInfo->szGroupId, sizeof(stuDeleteInfo.szGroupId)-1);
	stuInParam.pOPerateInfo = &stuDeleteInfo;
	BOOL bRet = CLIENT_OperateFaceRecognitionGroup(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Failed to delete this group!"), "");
		return;
	}

	delete m_pstSelectGroup;
	m_pstSelectGroup = NULL;
	pstGroupInfo = NULL;

	CleanGroupList();
	if (GetGroupInfos())
	{
		ClearShowGroupInfo();
		ShowGroupInfo();
	}
}

void CFaceDbOpreate::OnButtonAdd() 
{
	CAddGroupDlg AddGroupDlg(m_lLoginID, 1);
	int nResponse = AddGroupDlg.DoModal();

	CleanGroupList();
	if (GetGroupInfos())
	{
		ClearShowGroupInfo();
		ShowGroupInfo();
	}
}

void CFaceDbOpreate::CleanGroupList()
{
	std::list<COMMON_GROUP_INFO*>::iterator it = m_lstGroupInfo.begin();
	while(it != m_lstGroupInfo.end())
	{
		COMMON_GROUP_INFO *pstGroupInfo = *it;
		if (pstGroupInfo)
		{
			m_lstGroupInfo.erase(it++);
			delete pstGroupInfo;
			pstGroupInfo = NULL;
		}
	}
}

void CFaceDbOpreate::ClearShowGroupInfo()
{
	CString str;
	for(int i =0; i < PERPAGE_INDEX_COUNT; i++)
	{
		int nId = IDC_GROUP_ID1+i;
		SetDlgItemText(nId, "");

		nId = IDC_DB_NAME1+i;
		SetDlgItemText(nId, "");
			
		nId = IDC_PERSON_NUM1 + i;
		SetDlgItemText(nId, "");

		nId = IDC_RADIO_group1 + i;
		((CButton*)GetDlgItem(nId))->SetCheck(0);
		GetDlgItem(nId)->EnableWindow(FALSE);
	}
}

void CFaceDbOpreate::OnButtonRefresh() 
{
	CleanGroupList();
	m_nGroupPos = 0;
	if (GetGroupInfos())
	{
		ClearShowGroupInfo();
		ShowGroupInfo();

		GetDlgItem(IDC_group_PerPage)->EnableWindow(FALSE);
		GetDlgItem(IDC_group_NextPage)->EnableWindow(TRUE);
		if (m_nGroupCnt <= PERPAGE_INDEX_COUNT)
		{
			GetDlgItem(IDC_group_NextPage)->EnableWindow(FALSE);
		}
		
		((CButton*)GetDlgItem(IDC_RADIO_group1))->SetCheck(1);
		OnSelectGroup();
	}
}

void CFaceDbOpreate::OnBUTTONEdit() 
{
	if (NULL == m_pstSelectGroup)
	{
		MessageBox(ConvertString("Please select a group!"), "");
		return;
	}

	NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo = &(m_pstSelectGroup->stuGroupInfo);
	CAddGroupDlg EditGroupDlg(m_lLoginID, 2, pstGroupInfo);
	int nResponse = EditGroupDlg.DoModal();

	//  refresh update info 
	char *pGroupName = EditGroupDlg.GetEditGroupName();
	if (NULL != pGroupName)
	{
		strncpy(m_pstSelectGroup->stuGroupInfo.szGroupName, pGroupName, sizeof(m_pstSelectGroup->stuGroupInfo.szGroupName)-1);

		int nId = m_nSelectId - IDC_RADIO_group1;
		CString str;
		str.Format("%s", pGroupName);
		SetDlgItemText(IDC_DB_NAME1+nId, str);
	}
}

void CFaceDbOpreate::OnButtonPersonOpreate() 
{
	if (NULL == m_pstSelectGroup)
	{
		MessageBox(ConvertString("Please select a group!"), "");
		return;
	}

	NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo = &(m_pstSelectGroup->stuGroupInfo);
	
	CPersonOpreateGlg PersonOpreateGlg(m_lLoginID, pstGroupInfo->szGroupId, pstGroupInfo->szGroupName, m_pSoftPath);
	PersonOpreateGlg.DoModal();
}

void CFaceDbOpreate::RsetGroupInfo()
{
	if (NULL == m_pstSelectGroup)
	{
		return;
	}

	int nIndex = 0;
	m_pstSelectGroup->stuGroupInfo.nRetChnCount = 0;
	m_pstSelectGroup->stuGroupInfo.nRetSimilarityCount = 0;

	std::list<COMMON_GROUP_INFO*>::iterator it = m_lstGroupInfo.begin();
	for (; (it != m_lstGroupInfo.end()); ++it)
	{
		if (*it)
		{
			COMMON_GROUP_INFO *pstGroupInfo = *it;
			if (m_pstSelectGroup->nIndex == pstGroupInfo->nIndex)
			{
				memcpy(pstGroupInfo, m_pstSelectGroup, sizeof(*pstGroupInfo));
				break;
			}
		}
	}

}

void CFaceDbOpreate::OnButtonDispatch() 
{
	if (NULL == m_pstSelectGroup)
	{
		MessageBox(ConvertString("Please select a group!"), "");
		return;
	}

	NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo = &(m_pstSelectGroup->stuGroupInfo);
	CDispatchGroupDlg DispatchGroupDlg(m_lLoginID, m_nChnCount, pstGroupInfo);
	DispatchGroupDlg.DoModal();

	NET_FACERECONGNITION_GROUP_INFO *pstDispatchGroupInfo = DispatchGroupDlg.GetDispatchGroupInfo();
	memcpy(pstGroupInfo, pstDispatchGroupInfo, sizeof(*pstGroupInfo));

	std::list<COMMON_GROUP_INFO*>::iterator it = m_lstGroupInfo.begin();
	for (; it != m_lstGroupInfo.end(); ++it)
	{
		COMMON_GROUP_INFO *pstInfo = *it;
		if (pstInfo)
		{
			if (0 == strcmp(pstInfo->stuGroupInfo.szGroupId, pstGroupInfo->szGroupId))
			{
				memcpy(&(pstInfo->stuGroupInfo), pstGroupInfo, sizeof(pstInfo->stuGroupInfo));
			}
		}
	}
}

void CFaceDbOpreate::OnSelectGroup() 
{
	int nIndex = m_nGroupPos;
	int nId = IDC_RADIO_group1;

	for (nIndex = m_nGroupPos+IDC_RADIO_group1; nIndex < (nIndex+PERPAGE_INDEX_COUNT); nIndex++)
	{
		if (((CButton *)GetDlgItem(nId))->GetCheck())
		{
			m_nSelectId = nId;
			std::list<COMMON_GROUP_INFO*>::iterator it = m_lstGroupInfo.begin();
			for (; it != m_lstGroupInfo.end(); ++it)
			{
				COMMON_GROUP_INFO *pstGroupInfo = *it;
				if ((NULL != pstGroupInfo) && (nIndex == pstGroupInfo->nIndex))
				{
					if (NULL == m_pstSelectGroup)
					{
						m_pstSelectGroup = new COMMON_GROUP_INFO;
					}
					if (NULL == m_pstSelectGroup)
					{
						MessageBox(ConvertString("Memory error"), "");
						return;
					}

					memset(m_pstSelectGroup, 0, sizeof(*m_pstSelectGroup));
					memcpy(m_pstSelectGroup, pstGroupInfo, sizeof(*m_pstSelectGroup));
					break;
				}
			}

			break;
		}
		nId++;
	}

	if (NULL == m_pstSelectGroup)
	{
		return;
	}

	NET_FACERECONGNITION_GROUP_INFO *pstSelectInfo = &(m_pstSelectGroup->stuGroupInfo);
	CString str;
	str.Format("%s", pstSelectInfo->szGroupName);
	SetDlgItemText(IDC_DB_NAME, str);
}


void CFaceDbOpreate::OngroupPerPage() 
{
	ClearShowGroupInfo();
	m_nGroupPos -= PERPAGE_INDEX_COUNT;
	GetDlgItem(IDC_group_NextPage)->EnableWindow(TRUE);
	if (0 == m_nGroupPos)
	{
		GetDlgItem(IDC_group_PerPage)->EnableWindow(FALSE);
	}
	ShowGroupInfo();
}

void CFaceDbOpreate::OngroupNextPage() 
{
	ClearShowGroupInfo();
	m_nGroupPos += PERPAGE_INDEX_COUNT;
	GetDlgItem(IDC_group_PerPage)->EnableWindow(TRUE);
	if (m_nGroupPos+PERPAGE_INDEX_COUNT >= m_nGroupCnt)
	{
		GetDlgItem(IDC_group_NextPage)->EnableWindow(FALSE);
	}
	ShowGroupInfo();
}

void CFaceDbOpreate::OnBnClickedButtonDispatchChannel()
{
	CComboBox *ComboxChannel = (CComboBox *)GetDlgItem(IDC_COMBO_DISPATCH_CHANNEL);
	int nSel = ComboxChannel->GetCurSel();
	if (CB_ERR == ComboxChannel->GetCurSel())
	{
		MessageBox(ConvertString("Please select a channel!"), "");
		return;
	}

	CDispatchGroupChannelDlg dlg(m_lLoginID, nSel + 1);
	dlg.DoModal();
}

BOOL CFaceDbOpreate::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
