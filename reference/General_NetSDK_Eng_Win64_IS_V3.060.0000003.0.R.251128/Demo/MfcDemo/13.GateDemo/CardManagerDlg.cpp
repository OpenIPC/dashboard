// CardManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GateDemo.h"
#include "CardManagerDlg.h"
#include "AddCardDlg.h"
#include "ModifyCardDlg.h"
#include "CardInfo.h"

struct updateLcInfo
{
	int nBegin;
	int nShowNum;
};

// CCardManagerDlg 对话框
#define  QUERY_COUNT 20

#define WM_USER_DOTHREAD		(WM_USER + 210)
#define	WM_USER_THREADCLOSE		(WM_USER + 211)

IMPLEMENT_DYNAMIC(CCardManagerDlg, CDialog)

CCardManagerDlg::CCardManagerDlg(LLONG lLoginID, CWnd* pParent /*=NULL*/)
	: CDialog(CCardManagerDlg::IDD, pParent)
{
	m_lLoginID = lLoginID;
	m_lFindHandle = 0;
	m_ThreadHandle = 0;
	m_dwThreadID = 0;
	m_bIsDoFindNext = FALSE;

	InitializeCriticalSection(&m_csCardInfoVector);
}

CCardManagerDlg::~CCardManagerDlg()
{
	ClearCardInfoVector();

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

	DeleteCriticalSection(&m_csCardInfoVector);
}

void CCardManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CARD, m_lcCardInfo);
}


BEGIN_MESSAGE_MAP(CCardManagerDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_CARD_SEARCH, &CCardManagerDlg::OnBnClickedBtnCardSearch)
	ON_BN_CLICKED(IDC_BTN_CARD_ADD, &CCardManagerDlg::OnBnClickedBtnCardAdd)
	ON_BN_CLICKED(IDC_BTN_CARD_MODIFY, &CCardManagerDlg::OnBnClickedBtnCardModify)
	ON_BN_CLICKED(IDC_BTN_CARD_DELETE, &CCardManagerDlg::OnBnClickedBtnCardDelete)
	ON_BN_CLICKED(IDC_BTN_CLEAR_CARD, &CCardManagerDlg::OnBnClickedBtnClearCard)

	ON_MESSAGE(WM_USER_DOTHREAD, &CCardManagerDlg::DoThreadFindCard)
	ON_MESSAGE(WM_USER_THREADCLOSE, &CCardManagerDlg::DoCloseThread)
END_MESSAGE_MAP()


LRESULT CCardManagerDlg::DoThreadFindCard(WPARAM wParam, LPARAM lParam)
{
	//this->DoFindNextRecord();
	updateLcInfo *ep = (updateLcInfo*)wParam;
	if (NULL == ep)
	{
		return 0;
	}

	this->UpdateLcCardInfo(ep->nBegin, ep->nShowNum);

	delete ep;
	return 0;
}

LRESULT CCardManagerDlg::DoCloseThread(WPARAM wParam, LPARAM lParam)
{

	this->CloseThread();

	return 0;
}

void CCardManagerDlg::CloseThread()
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
	GetDlgItem(IDC_BTN_CARD_SEARCH)->EnableWindow(TRUE);
}

// CCardManagerDlg 消息处理程序

BOOL CCardManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);
	InitControl();

	return TRUE;  // return TRUE unless you set the focus to a control
}

DWORD WINAPI DoFindCardInfo(LPVOID lpParam)
{
	if (lpParam == 0)
	{
		return 0;
	}

	CCardManagerDlg *dlg = (CCardManagerDlg*)lpParam;
	dlg->DoFindNextRecord();

	return 1;
}

void CCardManagerDlg::OnBnClickedBtnCardSearch()
{
	CloseFindhandle();
	ClearCardInfoVector();
	m_lcCardInfo.DeleteAllItems();

	m_bIsDoFindNext = TRUE;

	GetDlgItem(IDC_BTN_CARD_SEARCH)->EnableWindow(FALSE);

	CString strCardNo = "";
	BOOL bRet = FALSE;

	NET_OUT_FIND_RECORD_PARAM stuOutParam;
	memset(&stuOutParam, 0, sizeof(stuOutParam));
	stuOutParam.dwSize = sizeof(NET_OUT_FIND_RECORD_PARAM);

	NET_IN_FIND_RECORD_PARAM stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;

	FIND_RECORD_ACCESSCTLCARD_CONDITION *pStuCardInfo = NEW FIND_RECORD_ACCESSCTLCARD_CONDITION;
	if (pStuCardInfo == NULL)
	{
		MessageBox(ConvertString("New Memory fail"), ConvertString("Prompt"));
		GetDlgItem(IDC_BTN_CARD_SEARCH)->EnableWindow(TRUE);
		goto FREE_RETURN;
	}
	memset(pStuCardInfo, 0, sizeof(FIND_RECORD_ACCESSCTLCARD_CONDITION));
	pStuCardInfo->dwSize = sizeof(FIND_RECORD_ACCESSCTLCARD_CONDITION);

	GetDlgItemText(IDC_EDIT_MANAGER_CARDNO, strCardNo);
	if (strCardNo != "")
	{
		char *pCardNo = (LPSTR)(LPCSTR)strCardNo;
		strncpy(pStuCardInfo->szCardNo, pCardNo, sizeof(pStuCardInfo->szCardNo) - 1);
		pStuCardInfo->abCardNo = TRUE;
	}
	
	stuInParam.pQueryCondition = (void*)pStuCardInfo;

	bRet = CLIENT_FindRecord(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Query CardInfo fail"), ConvertString("Prompt"));
		GetDlgItem(IDC_BTN_CARD_SEARCH)->EnableWindow(TRUE);
		goto FREE_RETURN;
	}
	m_lFindHandle = stuOutParam.lFindeHandle;

	m_ThreadHandle = CreateThread(NULL, 0, DoFindCardInfo, this, 0, (LPDWORD)&m_dwThreadID);

FREE_RETURN:
	if (pStuCardInfo)
	{
		delete pStuCardInfo;
		pStuCardInfo = NULL;
	}
}

void CCardManagerDlg::DoFindNextRecord()
{
	int nBegin = 0;
	NET_IN_FIND_NEXT_RECORD_PARAM stuInParam;
	NET_OUT_FIND_NEXT_RECORD_PARAM	stuOutParam;
	bool isSuccess = true;

	// 内存申请失败，直接返回
	NET_RECORDSET_ACCESS_CTL_CARD* pAccessCardInfo = NEW NET_RECORDSET_ACCESS_CTL_CARD[QUERY_COUNT];
	if (NULL == pAccessCardInfo)
	{
		MessageBox(ConvertString("NEW Memory fail"), "");
		goto ERROR_OUT;
	}

	HWND hWnd = this->GetSafeHwnd();
	if (NULL == hWnd)
	{
		MessageBox(ConvertString("hWnd is NULL"), "");
		isSuccess = false;
		goto ERROR_OUT;
	}

	while(m_bIsDoFindNext)
	{
		memset(&stuInParam, 0, sizeof(stuInParam));
		stuInParam.dwSize = sizeof(stuInParam);
		stuInParam.lFindeHandle = m_lFindHandle;
		stuInParam.nFileCount = QUERY_COUNT;
		
		memset(&stuOutParam, 0, sizeof(NET_OUT_FIND_NEXT_RECORD_PARAM));
		stuOutParam.dwSize = sizeof(NET_OUT_FIND_NEXT_RECORD_PARAM);

		memset(pAccessCardInfo, 0, stuInParam.nFileCount * sizeof(NET_RECORDSET_ACCESS_CTL_CARD));

		for (int i = 0; i < stuInParam.nFileCount; i++)
		{
			pAccessCardInfo[i].dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
		}
		stuOutParam.pRecordList = (void*)pAccessCardInfo;
		stuOutParam.nMaxRecordNum = stuInParam.nFileCount;

		int nRet = CLIENT_FindNextRecord(&stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);

		if (nRet > 0)
		{
			EnterCriticalSection(&m_csCardInfoVector);
			for (int n = 0; n < stuOutParam.nRetRecordNum; n++)
			{
				tagCardInfo *pCardInfo = NEW tagCardInfo;
				if (pCardInfo == NULL)
				{
					MessageBox(ConvertString("NEW Memory fail"), "");
					isSuccess = false;
					break;
				}
				memset(pCardInfo, 0, sizeof(tagCardInfo));

				CopyCardInfo(pCardInfo, &pAccessCardInfo[n]);
				
				m_cardInfoVector.push_back(pCardInfo);
			}
			LeaveCriticalSection(&m_csCardInfoVector);

			// 前面内存分配失败，此处直接退出While循环
			// 且前面已经打印过，此处不再打印
			if (!isSuccess)
			{
				break;
			}

			updateLcInfo *pUpdateInfo = NEW updateLcInfo;
			if (pUpdateInfo == NULL)
			{
				isSuccess = false;
				break;
			}
			pUpdateInfo->nBegin = nBegin;
			pUpdateInfo->nShowNum = stuOutParam.nRetRecordNum;

			::PostMessage(hWnd, WM_USER_DOTHREAD, (WPARAM)pUpdateInfo, NULL);

			nBegin += stuOutParam.nRetRecordNum;
		}
		
		// 查询结束，退出
		if (stuOutParam.nRetRecordNum < QUERY_COUNT)
		{
			break;
		}
	}

ERROR_OUT:
	// 如果失败了，清空 m_cardInfoVector
	if (!isSuccess)
	{
		ClearCardInfoVector();
	}

	if (pAccessCardInfo)
	{
		delete[] pAccessCardInfo;
		pAccessCardInfo = NULL;
	}
	CloseFindhandle();

	::PostMessage(hWnd, WM_USER_THREADCLOSE, NULL, NULL);
}

void CCardManagerDlg::UpdateLcCardInfo(int nBeginNum, int nShowNum)
{
	CString strIndex = "";
	CString strCardNo = "";
	CString strCardName = "";
	CString strUserId = "";
	CString strCardStatus = "";
	CString strCardPwd = "";
	CString strCardType = "";
	CString strUserTime = "";
	CString strIsValid;
	CString strIsFirstCard = "";
	CString strValidStartTime = "";
	CString strValidEndTime = "";
	
	int nSel = 0;
	for(int n = 0; n < nShowNum; n++)
	{
		nSel = nBeginNum + n;
		tagCardInfo *pInfo = m_cardInfoVector[nSel];

		strIndex.Format("%d", pInfo->nRecNo);
		strCardNo.Format("%s", pInfo->szCardNo);
		strCardName.Format("%s", pInfo->szCardName);
		strUserId.Format("%s", pInfo->szUserID);
		strCardPwd.Format("%s", pInfo->szPsw);
		strUserTime.Format("%d", pInfo->nUserTime);
		if (TRUE == pInfo->bIsValid)
		{
			strIsValid = "True";
		}
		else
		{
			strIsValid = "False";
		}

		if (TRUE == pInfo->bFirstEnter)
		{
			strIsFirstCard = "True";
		}
		else
		{
			strIsFirstCard = "False";
		}

		strValidStartTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->stuValidStartTime.dwYear, 
			pInfo->stuValidStartTime.dwMonth,
			pInfo->stuValidStartTime.dwDay,
			pInfo->stuValidStartTime.dwHour,
			pInfo->stuValidStartTime.dwMinute,
			pInfo->stuValidStartTime.dwSecond);

		strValidEndTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->stuValidEndTime.dwYear,
			pInfo->stuValidEndTime.dwMonth,
			pInfo->stuValidEndTime.dwDay,
			pInfo->stuValidEndTime.dwHour,
			pInfo->stuValidEndTime.dwMinute,
			pInfo->stuValidEndTime.dwSecond);

		switch (pInfo->emStatus)
		{
		case NET_ACCESSCTLCARD_STATE_NORMAL:
			strCardStatus = "Normal";
			break;
		case NET_ACCESSCTLCARD_STATE_LOSE:
			strCardStatus = "Lose";
			break;
		case NET_ACCESSCTLCARD_STATE_LOGOFF:
			strCardStatus = "Logoff";
			break;
		case NET_ACCESSCTLCARD_STATE_FREEZE:
			strCardStatus = "Freeze";
			break;
		case NET_ACCESSCTLCARD_STATE_ARREARAGE:
			strCardStatus = "Arrearage";
			break;
		case NET_ACCESSCTLCARD_STATE_OVERDUE:
			strCardStatus = "Overdue";
			break;
		case NET_ACCESSCTLCARD_STATE_PREARREARAGE:
			strCardStatus = "Prearrearage";
			break;
		case NET_ACCESSCTLCARD_STATE_UNKNOWN:
			strCardStatus = "Unknown";
			break;
		default:
			break;
		}

		switch (pInfo->emType)
		{
		case NET_ACCESSCTLCARD_TYPE_GENERAL:
			strCardType = "General";
			break;
		case NET_ACCESSCTLCARD_TYPE_VIP:
			strCardType = "Vip";
			break;
		case NET_ACCESSCTLCARD_TYPE_GUEST:
			strCardType = "Guest";
			break;
		case NET_ACCESSCTLCARD_TYPE_PATROL:
			strCardType = "Patrol";
			break;
		case NET_ACCESSCTLCARD_TYPE_BLACKLIST:
			strCardType = "Blocklist";
			break;
		case NET_ACCESSCTLCARD_TYPE_CORCE:
			strCardType = "Corce";
			break;
		case NET_ACCESSCTLCARD_TYPE_POLLING:
			strCardType = "Polling";
			break;
		case NET_ACCESSCTLCARD_TYPE_MOTHERCARD:
			strCardType = "Mothercard";
			break;
		case NET_ACCESSCTLCARD_TYPE_UNKNOWN:
			strCardType = "Unknown";
			break;
		default:
			break;
		}

		m_lcCardInfo.InsertItem(nSel, strIndex);
		m_lcCardInfo.SetItemText(nSel, 1, strCardNo);
		m_lcCardInfo.SetItemText(nSel, 2, strCardName);
		m_lcCardInfo.SetItemText(nSel, 3, strUserId);
		m_lcCardInfo.SetItemText(nSel, 4, ConvertString(strCardStatus));
		m_lcCardInfo.SetItemText(nSel, 5, strCardPwd);
		m_lcCardInfo.SetItemText(nSel, 6, ConvertString(strCardType));
		m_lcCardInfo.SetItemText(nSel, 7, strUserTime);
		m_lcCardInfo.SetItemText(nSel, 8, strIsValid);
		m_lcCardInfo.SetItemText(nSel, 9, strIsFirstCard);
		m_lcCardInfo.SetItemText(nSel, 10, strValidStartTime);
		m_lcCardInfo.SetItemText(nSel, 11, strValidEndTime);
	}
}

void CCardManagerDlg::InitControl()
{
	m_lcCardInfo.SetExtendedStyle(m_lcCardInfo.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lcCardInfo.InsertColumn(0,ConvertString("RecNo"),LVCFMT_LEFT,50,-1);
	m_lcCardInfo.InsertColumn(1,ConvertString("CardNo"),LVCFMT_LEFT,60,0);
	m_lcCardInfo.InsertColumn(2,ConvertString("CardName"),LVCFMT_LEFT,100,1);
	m_lcCardInfo.InsertColumn(3, ConvertString("UserID"), LVCFMT_LEFT,100,2);
	m_lcCardInfo.InsertColumn(4,ConvertString("CardStatus"),LVCFMT_LEFT,100,3);
	m_lcCardInfo.InsertColumn(5, ConvertString("CardPassword"), LVCFMT_LEFT,100,4);
	m_lcCardInfo.InsertColumn(6,ConvertString("CardType"),LVCFMT_LEFT,100,5);
	m_lcCardInfo.InsertColumn(7, ConvertString("UserTime"), LVCFMT_LEFT,80,6);
	m_lcCardInfo.InsertColumn(8,ConvertString("IsValid"),LVCFMT_LEFT,100,7);
	m_lcCardInfo.InsertColumn(9,ConvertString("IsFirstCard"),LVCFMT_LEFT,100,8);
	m_lcCardInfo.InsertColumn(10, ConvertString("ValidStartTime"), LVCFMT_LEFT,200,9);
	m_lcCardInfo.InsertColumn(11, ConvertString("ValidEndTime"), LVCFMT_LEFT,200,10);
}

void CCardManagerDlg::OnBnClickedBtnCardAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	CAddCardDlg dlg(m_lLoginID);
	dlg.DoModal();
}

void CCardManagerDlg::OnBnClickedBtnCardModify()
{
	CListCtrl *pModifyInfo = (CListCtrl*)GetDlgItem(IDC_LIST_CARD);
	int nSel = pModifyInfo->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		return;
	}

	tagCardInfo *cardInfo = m_cardInfoVector[nSel];

	CModifyCardDlg dlg(m_lLoginID, *cardInfo);
	dlg.DoModal();
}

BOOL CCardManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CCardManagerDlg::CloseFindhandle()
{
	if (m_lFindHandle)
	{
		CLIENT_FindRecordClose(m_lFindHandle);
		m_lFindHandle = 0;
	}
}

void CCardManagerDlg::ClearCardInfoVector()
{
	EnterCriticalSection(&m_csCardInfoVector);

	std::vector<tagCardInfo*>::iterator it = m_cardInfoVector.begin();
	while(it != m_cardInfoVector.end())
	{
		tagCardInfo *pStCardInfo = *it;
		if (pStCardInfo)
		{
			it = m_cardInfoVector.erase(it);
			delete pStCardInfo;
			pStCardInfo = NULL;
		}
	}
	m_cardInfoVector.clear();

	LeaveCriticalSection(&m_csCardInfoVector);
}

void CCardManagerDlg::CopyCardInfo(tagCardInfo *pCardInfoDest, NET_RECORDSET_ACCESS_CTL_CARD *pCardInfoSrc)
{
	if (pCardInfoDest == NULL || pCardInfoSrc == NULL)
	{
		return;
	}

	pCardInfoDest->nRecNo = pCardInfoSrc->nRecNo;
	strncpy(pCardInfoDest->szCardNo, pCardInfoSrc->szCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(pCardInfoDest->szCardName, pCardInfoSrc->szCardName, DH_MAX_CARDNAME_LEN - 1);
	strncpy(pCardInfoDest->szUserID, pCardInfoSrc->szUserID, DH_MAX_USERID_LEN - 1);
	strncpy(pCardInfoDest->szPsw, pCardInfoSrc->szPsw, DH_MAX_CARDPWD_LEN - 1);
	pCardInfoDest->emStatus = pCardInfoSrc->emStatus;
	pCardInfoDest->emType = pCardInfoSrc->emType;
	pCardInfoDest->nUserTime = pCardInfoSrc->nUserTime;
	pCardInfoDest->bFirstEnter = pCardInfoSrc->bFirstEnter;
	pCardInfoDest->bIsValid = pCardInfoSrc->bIsValid;
	pCardInfoDest->stuValidStartTime = pCardInfoSrc->stuValidStartTime;
	pCardInfoDest->stuValidEndTime = pCardInfoSrc->stuValidEndTime;
}
void CCardManagerDlg::OnBnClickedBtnCardDelete()
{
	CListCtrl *pModifyInfo = (CListCtrl*)GetDlgItem(IDC_LIST_CARD);
	int nSel = pModifyInfo->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		return;
	}

	tagCardInfo *pCardInfo = m_cardInfoVector[nSel];

	BOOL bDeleteFace = DeleteFaceInfo(*pCardInfo);
	//删除人脸图片失败
	if (FALSE == bDeleteFace)
	{
		return;
	}

	BOOL bDeleteCard = DeleteCardInfo(pCardInfo);
	//删除人员信息失败
	if (FALSE == bDeleteCard)
	{
		return;
	}

	MessageBox(ConvertString("Delete successfully"), ConvertString("Prompt"));
}

BOOL CCardManagerDlg::DeleteCardInfo(tagCardInfo *pCardInfo)
{
	NET_CTRL_RECORDSET_PARAM stuInParam ;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;
	stuInParam.pBuf = &pCardInfo->nRecNo;
	stuInParam.nBufLen = sizeof(int);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_REMOVE, &stuInParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Delete cardinfo fail"), ConvertString("Prompt"));
	}

	return bRet;
}

BOOL CCardManagerDlg::DeleteFaceInfo(const tagCardInfo& cardInfo)
{
	BOOL bRet = FALSE;
	NET_IN_REMOVE_FACE_INFO stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	strncpy(stuInParam.szUserID, cardInfo.szUserID, DH_MAX_USERID_LEN - 1);

	NET_OUT_REMOVE_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(stuOutParam));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginID, EM_FACEINFO_OPREATE_REMOVE, (void*)&stuInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Delete face fail"), ConvertString("Prompt"));
	}

	return bRet;
}

void CCardManagerDlg::OnBnClickedBtnClearCard()
{
	BOOL bFaceClear = ClearFaceInfo();
	BOOL bCardClear = ClearCardInfo();

	if (bFaceClear && bCardClear)
	{
		MessageBox(ConvertString("Clear successfully"), ConvertString("Prompt"));
	}
}

BOOL CCardManagerDlg::ClearFaceInfo()
{
	BOOL bRet = FALSE;
	NET_IN_CLEAR_FACE_INFO stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);

	NET_OUT_CLEAR_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(stuOutParam));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginID, EM_FACEINFO_OPREATE_CLEAR, (void*)&stuInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Clear face fail"), ConvertString("Prompt"));
	}

	return bRet;
}

BOOL CCardManagerDlg::ClearCardInfo()
{
	NET_CTRL_RECORDSET_PARAM stuInParam ;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_CLEAR, &stuInParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Clear cardinfo fail"), ConvertString("Prompt"));
	}

	return bRet;
}