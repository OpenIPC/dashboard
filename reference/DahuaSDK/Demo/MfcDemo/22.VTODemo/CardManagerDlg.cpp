// CardManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VTODemo.h"
#include "CardManagerDlg.h"
#include "AddCardDlg.h"
#include "Base64.h"
#include "CMyListCtl.h"


#define  QUERY_COUNT 20

#define WM_USER_DOTHREAD		(WM_USER + 210)
#define	WM_USER_THREADCLOSE		(WM_USER + 211)

struct updateLcInfo
{
	int nBegin;
	int nShowNum;
};

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

// CCardManagerDlg 对话框

IMPLEMENT_DYNAMIC(CCardManagerDlg, CDialog)

CCardManagerDlg::CCardManagerDlg(LLONG loginID,CWnd* pParent /*=NULL*/)
	: CDialog(CCardManagerDlg::IDD, pParent)
{
	m_lLoginHandle = loginID;

	InitializeCriticalSection(&m_csCardInfoVector);

	m_bIsDoFindNext = FALSE;
	m_ThreadHandle = NULL;
	m_lFindHandle = 0;
	m_dwThreadID = 0;
}

CCardManagerDlg::~CCardManagerDlg()
{
}

void CCardManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_listCtrl);
}


BEGIN_MESSAGE_MAP(CCardManagerDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_CARDSEARCH, &CCardManagerDlg::OnBnClickedBtnCardsearch)
	ON_BN_CLICKED(IDC_BTN_CARDADD, &CCardManagerDlg::OnBnClickedBtnCardadd)
	ON_BN_CLICKED(IDC_BTN_CARDMODIFY, &CCardManagerDlg::OnBnClickedBtnCardmodify)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CCardManagerDlg::OnBnClickedBtnDelete)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CCardManagerDlg::OnBnClickedBtnClear)

	ON_MESSAGE(WM_USER_DOTHREAD, &CCardManagerDlg::DoThreadFindCard)
	ON_MESSAGE(WM_USER_THREADCLOSE, &CCardManagerDlg::DoCloseThread)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CCardManagerDlg 消息处理程序

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
void CCardManagerDlg::OnBnClickedBtnCardsearch()
{
	// TODO: 在此添加控件通知处理程序代码
	CloseFindhandle();
	ClearCardInfoVector();
	m_listCtrl.DeleteAllItems();
	m_nRecNoVec.clear();

	m_bIsDoFindNext = TRUE;

	GetDlgItem(IDC_BTN_CARDSEARCH)->EnableWindow(FALSE);

	CString strCardNo = "";
	BOOL bRet = FALSE;

	NET_OUT_FIND_RECORD_PARAM stuOutParam;
	memset(&stuOutParam, 0, sizeof(stuOutParam));
	stuOutParam.dwSize = sizeof(NET_OUT_FIND_RECORD_PARAM);

	NET_IN_FIND_RECORD_PARAM stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;

	FIND_RECORD_ACCESSCTLCARD_CONDITION *pStuCardInfo = new FIND_RECORD_ACCESSCTLCARD_CONDITION;
	if (pStuCardInfo == NULL)
	{
		MessageBox(ConvertString("New Memory fail"), ConvertString("Prompt"));
		GetDlgItem(IDC_BTN_CARDSEARCH)->EnableWindow(TRUE);
		goto FREE_RETURN;
	}
	memset(pStuCardInfo, 0, sizeof(FIND_RECORD_ACCESSCTLCARD_CONDITION));
	pStuCardInfo->dwSize = sizeof(FIND_RECORD_ACCESSCTLCARD_CONDITION);

	GetDlgItemText(IDC_EDIT1, strCardNo);
	if (strCardNo != "")
	{
		char *pCardNo = (LPSTR)(LPCSTR)strCardNo;
		strncpy(pStuCardInfo->szCardNo, pCardNo, sizeof(pStuCardInfo->szCardNo) - 1);
		pStuCardInfo->abCardNo = TRUE;
	}

	stuInParam.pQueryCondition = (void*)pStuCardInfo;

	bRet = CLIENT_FindRecord(m_lLoginHandle, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("CLIENT_FindRecord fail"), ConvertString("Prompt"));
		GetDlgItem(IDC_BTN_CARDSEARCH)->EnableWindow(TRUE);
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

void CCardManagerDlg::OnBnClickedBtnCardadd()
{
	// TODO: 在此添加控件通知处理程序代码
	CardInfo cardInfo;
	memset(&cardInfo, 0, sizeof(cardInfo));
	CAddCardDlg dlg(m_lLoginHandle, EM_Operate_add, cardInfo);
	dlg.DoModal();
}

void CCardManagerDlg::OnBnClickedBtnCardmodify()
{
	// TODO: 在此添加控件通知处理程序代码
	CardInfo cardInfo;
	memset(&cardInfo, 0, sizeof(cardInfo));

	int nSelectCount = m_listCtrl.GetSelectedCount();
	if (0 == nSelectCount)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		return;
	}

	int nIndex = m_listCtrl.GetSelectionMark();
	DWORD_PTR dwDataPtr = NULL;
	
	dwDataPtr = m_listCtrl.GetItemData(nIndex);
	memcpy(&cardInfo, (void *)dwDataPtr, sizeof(cardInfo));
	for(int i = 0;i < m_nRecNoVec.size(); i++)
	{
		if(m_nRecNoVec[i] == cardInfo.nRecNo)
		{
			MessageBox(ConvertString("This item has been changed, please search again!"), ConvertString("Prompt"));
			return ;
		}
	}

	CAddCardDlg dlg(m_lLoginHandle, EM_Operate_modify, cardInfo);
	if(IDCANCEL != dlg.DoModal())
	{
		m_nRecNoVec.push_back(cardInfo.nRecNo);
	}
}

void CCardManagerDlg::OnBnClickedBtnDelete()
{
	GetDlgItem(IDC_BTN_DELETE)->EnableWindow(FALSE);
	CMyListCtl *pModifyInfo = (CMyListCtl*)GetDlgItem(IDC_LIST2);
	int nSel = pModifyInfo->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		GetDlgItem(IDC_BTN_DELETE)->EnableWindow(TRUE);
		return;
	}

	tagCardInfo *pCardInfo = m_cardInfoVector[nSel];
	for(int i = 0;i < m_nRecNoVec.size(); i++)
	{
		if(m_nRecNoVec[i] == pCardInfo->nRecNo)
		{
			MessageBox(ConvertString("This item has been changed, please search again!"), ConvertString("Prompt"));
			GetDlgItem(IDC_BTN_DELETE)->EnableWindow(TRUE);
			return ;
		}
	}
	m_nRecNoVec.push_back(pCardInfo->nRecNo);

	BOOL bDeleteCard = DeleteCardInfo(pCardInfo);
	//删除人员信息失败
	if (FALSE == bDeleteCard)
	{
		GetDlgItem(IDC_BTN_DELETE)->EnableWindow(TRUE);
		return;
	}

	BOOL bDeleteFace = DeleteFaceInfo(*pCardInfo);
	//删除人脸图片失败
	if (FALSE == bDeleteFace)
	{
		GetDlgItem(IDC_BTN_DELETE)->EnableWindow(TRUE);
		return;
	}

	MessageBox(ConvertString("Delete successfully"), ConvertString("Prompt"));
	GetDlgItem(IDC_BTN_DELETE)->EnableWindow(TRUE);
}

void CCardManagerDlg::OnBnClickedBtnClear()
{
	// TODO: 在此添加控件通知处理程序代码
	INT_PTR nRes = MessageBox(ConvertString("All cards will be removed!"), ConvertString("Prompt"), MB_OKCANCEL);
	if(nRes == IDCANCEL)
	{
		return;
	}
	GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(FALSE);
	BOOL bFaceClear = ClearFaceInfo();
	if (FALSE == bFaceClear)
	{
		GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(TRUE);
		return;
	}

	BOOL bCardClear = ClearCardInfo();
	if (FALSE == bCardClear)
	{
		GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(TRUE);
		return;
	}
	
	GetDlgItem(IDC_BTN_CLEAR)->EnableWindow(TRUE);
	MessageBox(ConvertString("Clear successfully"), ConvertString("Prompt"));
	OnBnClickedBtnCardsearch();
}
void CCardManagerDlg::DoFindNextRecord()
{
	int nBegin = 0;
	NET_IN_FIND_NEXT_RECORD_PARAM stuInParam= {sizeof(stuInParam)};
	NET_OUT_FIND_NEXT_RECORD_PARAM	stuOutParam={sizeof(stuOutParam)};
	bool isSuccess = true;
	char *szFingerPrintBuf[QUERY_COUNT] = {0};

	// 内存申请失败，直接返回
	NET_RECORDSET_ACCESS_CTL_CARD* pAccessCardInfo = new NET_RECORDSET_ACCESS_CTL_CARD[QUERY_COUNT];
	if (NULL == pAccessCardInfo)
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		goto ERROR_OUT;
	}

	for (int i = 0; i < QUERY_COUNT; i++)
	{
		pAccessCardInfo[i].stuFingerPrintInfoEx.pPacketData = NULL;
	}
		
	for (int i = 0; i < QUERY_COUNT; i++)
	{
		szFingerPrintBuf[i] = new char[10*1024];
		if (NULL == szFingerPrintBuf[i])
		{
			goto ERROR_OUT;
		}
	}

	HWND hWnd = this->GetSafeHwnd();
	if (NULL == hWnd)
	{
		MessageBox(ConvertString("hWnd is NULL"), ConvertString("Prompt"));
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
			/* find fingerprint data */
			for (int n = 0; n < stuOutParam.nRetRecordNum; n++)
			{
				NET_CTRL_RECORDSET_PARAM stuFingerParam;
				memset(&stuFingerParam, 0, sizeof(stuFingerParam));
				stuFingerParam.dwSize = sizeof(stuFingerParam);
				stuFingerParam.emType = NET_RECORD_ACCESSCTLCARD;

				NET_RECORDSET_ACCESS_CTL_CARD *pstuCardInfo = pAccessCardInfo + n;
				pstuCardInfo->bEnableExtended = TRUE;

				pstuCardInfo->stuFingerPrintInfoEx.pPacketData = szFingerPrintBuf[n];
				pstuCardInfo->stuFingerPrintInfoEx.nPacketLen = 10*1024;
				stuFingerParam.pBuf = pstuCardInfo;

				stuFingerParam.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
				int len = 0;
				CLIENT_QueryDevState(m_lLoginHandle, DH_DEVSTATE_DEV_RECORDSET_EX, (char*)&stuFingerParam, sizeof(NET_CTRL_RECORDSET_PARAM), &len, DEFAULT_WAIT_TIME);
			}

			EnterCriticalSection(&m_csCardInfoVector);
			for (int n = 0; n < stuOutParam.nRetRecordNum; n++)
			{
				tagCardInfo *pCardInfo = new tagCardInfo;
				if (pCardInfo == NULL)
				{
					MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
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

			updateLcInfo *pUpdateInfo = new updateLcInfo;
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
		for (int i = 0; i < QUERY_COUNT; i++)
		{
			if(pAccessCardInfo[i].stuFingerPrintInfoEx.pPacketData)
			{
				delete []pAccessCardInfo[i].stuFingerPrintInfoEx.pPacketData;
				pAccessCardInfo[i].stuFingerPrintInfoEx.pPacketData = NULL;
			}
		}

		delete[] pAccessCardInfo;
		pAccessCardInfo = NULL;
	}
	CloseFindhandle();

	::PostMessage(hWnd, WM_USER_THREADCLOSE, NULL, NULL);
}
void CCardManagerDlg::CopyCardInfo(tagCardInfo *pCardInfoDest, NET_RECORDSET_ACCESS_CTL_CARD *pCardInfoSrc)
{
	if (pCardInfoDest == NULL || pCardInfoSrc == NULL)
	{
		return;
	}

	pCardInfoDest->nRecNo = pCardInfoSrc->nRecNo;
	strncpy(pCardInfoDest->szCardNo, pCardInfoSrc->szCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(pCardInfoDest->szHomeNo, pCardInfoSrc->szUserID, DH_COMMON_STRING_16 - 1);
	if (pCardInfoSrc->stuFingerPrintInfoEx.nRealPacketLen)
	{
		memcpy(pCardInfoDest->szFingerData, pCardInfoSrc->stuFingerPrintInfoEx.pPacketData, pCardInfoSrc->stuFingerPrintInfoEx.nRealPacketLen);
	}
	pCardInfoDest->nFingerLen = pCardInfoSrc->stuFingerPrintInfoEx.nRealPacketLen;
}
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
	GetDlgItem(IDC_BTN_CARDSEARCH)->EnableWindow(TRUE);
}
void CCardManagerDlg::UpdateLcCardInfo(int nBeginNum, int nShowNum)
{
	CString strIndex = "";
	CString strCardNo = "";
	CString strHomeNo = "";
	CString strFingerData= "";

	int nSel = 0;
	for(int n = 0; n < nShowNum; n++)
	{
		nSel = nBeginNum + n;
		tagCardInfo *pInfo = m_cardInfoVector[nSel];

		strIndex.Format("%d", pInfo->nRecNo);
		strCardNo.Format("%s", pInfo->szCardNo);
		strHomeNo.Format("%s", pInfo->szHomeNo);
		strFingerData= "";
		base64Encode(pInfo->szFingerData, pInfo->nFingerLen, strFingerData);

		m_listCtrl.InsertItem(nSel, strIndex);
		m_listCtrl.SetItemText(nSel, 1, strCardNo);
		m_listCtrl.SetItemText(nSel, 2, strHomeNo);
		m_listCtrl.SetItemText(nSel, 3, strFingerData);
		m_listCtrl.SetItemData(nSel, (DWORD_PTR)(void *)m_cardInfoVector[nSel]);
	}
}
BOOL CCardManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  在此添加额外的初始化
	g_SetWndStaticText(this);

	m_listCtrl.SetExtendedStyle(m_listCtrl.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_listCtrl.InsertColumn(0,ConvertString("RecNo"),LVCFMT_LEFT,80,-1);
	m_listCtrl.InsertColumn(1,ConvertString("CardNo"),LVCFMT_LEFT,300,0);
	m_listCtrl.InsertColumn(2, ConvertString("RoomNo"), LVCFMT_LEFT,165,1);
	m_listCtrl.InsertColumn(3, ConvertString("FingerData"), LVCFMT_LEFT,10000,2);

	m_listCtrl.GetHeaderCtrl()->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CCardManagerDlg::DeleteCardInfo(tagCardInfo *pCardInfo)
{
	NET_CTRL_RECORDSET_PARAM stuInParam ;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;
	stuInParam.pBuf = &pCardInfo->nRecNo;
	stuInParam.nBufLen = sizeof(int);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginHandle, DH_CTRL_RECORDSET_REMOVE, &stuInParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("CLIENT_ControlDevice  delete cardinfo fail"), ConvertString("Prompt"));
	}

	return bRet;
}

BOOL CCardManagerDlg::DeleteFaceInfo(const tagCardInfo& cardInfo)
{
	BOOL bRet = FALSE;
	NET_IN_REMOVE_FACE_INFO stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	strncpy(stuInParam.szUserID, cardInfo.szHomeNo, DH_MAX_USERID_LEN - 1);

	NET_OUT_REMOVE_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(stuOutParam));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginHandle, EM_FACEINFO_OPREATE_REMOVE, (void*)&stuInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("CLIENT_FaceInfoOpreate delete face fail"), ConvertString("Prompt"));
	}

	return bRet;
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

	bRet = CLIENT_FaceInfoOpreate(m_lLoginHandle, EM_FACEINFO_OPREATE_CLEAR, (void*)&stuInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("CLIENT_FaceInfoOpreate clear face fail"), ConvertString("Prompt"));
	}

	return bRet;
}

BOOL CCardManagerDlg::ClearCardInfo()
{
	NET_CTRL_RECORDSET_PARAM stuInParam ;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;

	BOOL bRet = CLIENT_ControlDevice(m_lLoginHandle, DH_CTRL_RECORDSET_CLEAR, &stuInParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("CLIENT_ControlDevice  clear cardinfo fail"), ConvertString("Prompt"));
	}

	return bRet;
}
BOOL CCardManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_KEYDOWN == pMsg->message)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			return true;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CCardManagerDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	
	CloseThread();

	if (m_lFindHandle)
	{
		CLIENT_FindRecordClose(m_lFindHandle);
		m_lFindHandle = 0;
	}
	
	DeleteCriticalSection(&m_csCardInfoVector);
	
}
