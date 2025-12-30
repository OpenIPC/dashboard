// RecordManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GateDemo.h"
#include "RecordManagerDlg.h"


#define WM_USER_DOTHREAD (WM_USER + 401)
#define WM_USER_THREADCLOSE (WM_USER + 402)

struct updateLcInfo
{
	int nBegin;
	int nShowNum;
};

DWORD WINAPI DoFindRecordInfo(LPVOID lpParam)
{
	if (lpParam == 0)
	{
		return 0;
	}

	CRecordManagerDlg *dlg = (CRecordManagerDlg*)lpParam;
	dlg->DoFindNextRecord();

	return 1;
}


// CRecordManagerDlg 对话框

IMPLEMENT_DYNAMIC(CRecordManagerDlg, CDialog)

CRecordManagerDlg::CRecordManagerDlg(LLONG lLoginID, CWnd* pParent /*=NULL*/)
	: CDialog(CRecordManagerDlg::IDD, pParent)
{
	m_lLoginID = lLoginID;
	m_lFindHandle = 0;
	m_bIsDoFindNext = FALSE;
	m_ThreadHandle = 0;

	InitializeCriticalSection(&m_csRecordVector);
}

CRecordManagerDlg::~CRecordManagerDlg()
{
	ClearRecordVector();
	CloseFindhandle();
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

	DeleteCriticalSection(&m_csRecordVector);
}

void CRecordManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RECORD_LIST, m_lcRecordList);
	DDX_Control(pDX, IDC_RECORD_CARDNO_CHECK, m_btnCardNoCheck);
	DDX_Control(pDX, IDC_CHECK_QUERYTIME, m_btnQeuryTimeCheck);
	DDX_Control(pDX, IDC_STATIC_SHOW_RECORD_PIC, m_recordPic);
}

BEGIN_MESSAGE_MAP(CRecordManagerDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_RECORD_SEARCH, &CRecordManagerDlg::OnBnClickedBtnRecordSearch)
	ON_BN_CLICKED(IDC_RECORD_CARDNO_CHECK, &CRecordManagerDlg::OnBnClickedRecordCardnoCheck)
	ON_BN_CLICKED(IDC_CHECK_QUERYTIME, &CRecordManagerDlg::OnBnClickedCheckQuerytime)

	ON_MESSAGE(WM_USER_DOTHREAD, &CRecordManagerDlg::DoThreadFindRecord)
	ON_MESSAGE(WM_USER_THREADCLOSE, &CRecordManagerDlg::DoCloseThread)
	ON_BN_CLICKED(IDC_BTN_RECORD_DELETE, &CRecordManagerDlg::OnBnClickedBtnRecordDelete)
	ON_BN_CLICKED(IDC_BTN_RECORD_CLEAR, &CRecordManagerDlg::OnBnClickedBtnRecordClear)
	ON_NOTIFY(NM_CLICK, IDC_RECORD_LIST, &CRecordManagerDlg::OnLvnItemchangedRecordList)
	ON_BN_CLICKED(IDC_BTN_PIC_DOWNLOAD, &CRecordManagerDlg::OnBnClickedBtnPicDownload)
END_MESSAGE_MAP()


LRESULT CRecordManagerDlg::DoThreadFindRecord(WPARAM wParam, LPARAM lParam)
{
	updateLcInfo *ep = (updateLcInfo*)wParam;
	if (NULL == ep)
	{
		return 0;
	}

	this->UpdateLcRecordInfo(ep->nBegin, ep->nShowNum);

	delete ep;
	return 0;
}

LRESULT CRecordManagerDlg::DoCloseThread(WPARAM wParam, LPARAM lParam)
{
	this->CloseThread();

	return 0;
}

// CRecordManagerDlg 消息处理程序

BOOL CRecordManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	g_SetWndStaticText(this);
	InitControl();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CRecordManagerDlg::InitControl()
{
	m_lcRecordList.SetExtendedStyle(m_lcRecordList.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lcRecordList.InsertColumn(0,ConvertString("RecNo"),LVCFMT_LEFT,50,-1);
	m_lcRecordList.InsertColumn(1,ConvertString("CardNo"),LVCFMT_LEFT,100,0);
	m_lcRecordList.InsertColumn(2,ConvertString("UserID"),LVCFMT_LEFT,100,1);
	m_lcRecordList.InsertColumn(3, ConvertString("Status"), LVCFMT_LEFT,100,2);
	m_lcRecordList.InsertColumn(4,ConvertString("CardType"),LVCFMT_LEFT,100,3);
	m_lcRecordList.InsertColumn(5, ConvertString("OpenMethod"), LVCFMT_LEFT,100,4);
	m_lcRecordList.InsertColumn(6, ConvertString("Time(Formatting time of UTC)"), LVCFMT_LEFT,200,8);
}

void CRecordManagerDlg::OnBnClickedBtnRecordSearch()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_RECORD_SEARCH)->EnableWindow(FALSE);
	ClearRecordVector();

	m_lcRecordList.DeleteAllItems();

	NET_IN_FIND_RECORD_PARAM InParam;
	memset(&InParam, 0, sizeof(InParam));
	InParam.dwSize = sizeof(InParam);
	InParam.emType = NET_RECORD_ACCESSCTLCARDREC_EX;

	FIND_RECORD_ACCESSCTLCARDREC_CONDITION_EX recordInfo;
	memset(&recordInfo, 0, sizeof(recordInfo));
	recordInfo.dwSize = sizeof(recordInfo);

	CString strCardNo;
	int nCheck = 0;

	//启用卡号查询
	nCheck = m_btnCardNoCheck.GetCheck();
	if (BST_CHECKED == nCheck)
	{
		GetDlgItemText(IDC_EDIT_RECORD_CARDNO, strCardNo);
		if (strCardNo != "")
		{
			recordInfo.bCardNoEnable = TRUE;
			strncpy(recordInfo.szCardNo, strCardNo, sizeof(recordInfo.szCardNo) - 1);
		}
	}
	//启用时间段查询
	nCheck = m_btnQeuryTimeCheck.GetCheck();
	if (BST_CHECKED == nCheck)
	{
		recordInfo.bTimeEnable = TRUE;
		SetDate(recordInfo.stStartTime, IDC_DATE_RECORD_BEGIN);
		SetTime(recordInfo.stStartTime, IDC_TIME_RECORD_BEGIN);

		SetDate(recordInfo.stEndTime, IDC_DATE_RECORD_END);
		SetTime(recordInfo.stEndTime, IDC_TIME_RECORD_END);
	}

	InParam.pQueryCondition = (void*)&recordInfo;

	NET_OUT_FIND_RECORD_PARAM outParam;
	memset(&outParam, 0, sizeof(outParam));
	outParam.dwSize = sizeof(outParam);

	BOOL bRet = CLIENT_FindRecord(m_lLoginID, &InParam, &outParam, DEFAULT_WAIT_TIME);    
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Query record fail"), ConvertString("Prompt"));
		GetDlgItem(IDC_BTN_RECORD_SEARCH)->EnableWindow(TRUE);
		return;
	}
	else
	{
		m_lFindHandle = outParam.lFindeHandle;
	}

	m_bIsDoFindNext = TRUE;
	m_ThreadHandle = CreateThread(NULL, 0, DoFindRecordInfo, this, 0, 0);
}

void CRecordManagerDlg::DoFindNextRecord()
{
	int nBegin = 0;

	NET_IN_FIND_NEXT_RECORD_PARAM stuInParam;
	NET_OUT_FIND_NEXT_RECORD_PARAM	stuOutParam;
	bool isSuccess = true;

	// 内存申请失败，直接返回
	const int nQueryNum = 20;
	NET_RECORDSET_ACCESS_CTL_CARDREC* pAccessRecordInfo = NEW NET_RECORDSET_ACCESS_CTL_CARDREC[nQueryNum];
	if (NULL == pAccessRecordInfo)
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
		stuInParam.nFileCount = nQueryNum;

		memset(&stuOutParam, 0, sizeof(stuOutParam));
		stuOutParam.dwSize = sizeof(stuOutParam);

		memset(pAccessRecordInfo, 0, stuInParam.nFileCount * sizeof(NET_RECORDSET_ACCESS_CTL_CARDREC));

		for (int i = 0; i < stuInParam.nFileCount; i++)
		{
			pAccessRecordInfo[i].dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARDREC);
		}
		stuOutParam.pRecordList = (void*)pAccessRecordInfo;
		stuOutParam.nMaxRecordNum = stuInParam.nFileCount;

		int nRet = CLIENT_FindNextRecord(&stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);

		if (nRet > 0)
		{
			EnterCriticalSection(&m_csRecordVector);
			for (int n = 0; n < stuOutParam.nRetRecordNum; n++)
			{
				tagRecordInfo *pRecordInfo = NEW tagRecordInfo;
				if (pRecordInfo == NULL)
				{
					MessageBox(ConvertString("NEW Memory fail"), "");
					isSuccess = false;
					break;
				}
				memset(pRecordInfo, 0, sizeof(*pRecordInfo));
				CopyRecordInfo(pRecordInfo, &pAccessRecordInfo[n]);

				m_recordVector.push_back(pRecordInfo);
			}
			LeaveCriticalSection(&m_csRecordVector);

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
		if (stuOutParam.nRetRecordNum < nQueryNum)
		{
			break;
		}
	}

ERROR_OUT:
	// 如果失败了，清空 m_recordVector
	if (!isSuccess)
	{
		ClearRecordVector();
	}

	if (pAccessRecordInfo)
	{
		delete[] pAccessRecordInfo;
		pAccessRecordInfo = NULL;
	}
	CloseFindhandle();

	::PostMessage(hWnd, WM_USER_THREADCLOSE, NULL, NULL);
}

void CRecordManagerDlg::UpdateLcRecordInfo(int nBeginNum, int nShowNum)
{
	CString strRecNo = "";
	CString strCardNo = "";
	CString strUserId = "";
	CString strStatus = "";
	CString strCardType = "";
	CString strOpenMethod = "";
	CString strTime = "";

	int nSel = 0;
	for(int n = 0; n < nShowNum; n++)
	{
		nSel = nBeginNum + n;
		tagRecordInfo *pInfo = m_recordVector[nSel];

		strRecNo.Format("%d", pInfo->nRecNo);
		strCardNo.Format("%s", pInfo->szCardNo);
		strUserId.Format("%s", pInfo->szUserID);

		//开门成功
		if (pInfo->bStatus == TRUE)
		{
			strStatus = "success";
		}
		//开门失败
		else
		{
			strStatus = "fail";
		}

		switch (pInfo->emCardType)
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

		switch (pInfo->emMethod)
		{
		case NET_ACCESS_DOOROPEN_METHOD_CARD:
			strOpenMethod = "Card Method";
			break;
		case NET_ACCESS_DOOROPEN_METHOD_FACE_RECOGNITION:
			strOpenMethod = "Face Method";
			break;
		default:
			strOpenMethod = "Unknown";
			break;
		}
		strTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->stuTime.dwYear,
			pInfo->stuTime.dwMonth,
			pInfo->stuTime.dwDay,
			pInfo->stuTime.dwHour,
			pInfo->stuTime.dwMinute,
			pInfo->stuTime.dwSecond);
		
		m_lcRecordList.InsertItem(nSel, strRecNo);
		m_lcRecordList.SetItemText(nSel, 1, strCardNo);
		m_lcRecordList.SetItemText(nSel, 2, strUserId);
		m_lcRecordList.SetItemText(nSel, 3, ConvertString(strStatus));
		m_lcRecordList.SetItemText(nSel, 4, ConvertString(strCardType));
		m_lcRecordList.SetItemText(nSel, 5, ConvertString(strOpenMethod));
		m_lcRecordList.SetItemText(nSel, 6, strTime);
	}
}

void CRecordManagerDlg::ClearRecordVector()
{
	EnterCriticalSection(&m_csRecordVector);

	std::vector<tagRecordInfo*>::iterator it = m_recordVector.begin();
	while(it != m_recordVector.end())
	{
		tagRecordInfo *pRecordInfo = *it;
		if (pRecordInfo)
		{
			it = m_recordVector.erase(it);
			delete pRecordInfo;
			pRecordInfo = NULL;
		}
	}
	m_recordVector.clear();

	LeaveCriticalSection(&m_csRecordVector);
}

void CRecordManagerDlg::CloseThread()
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
	GetDlgItem(IDC_BTN_RECORD_SEARCH)->EnableWindow(TRUE);
}

void CRecordManagerDlg::CloseFindhandle()
{

	if (m_lFindHandle)
	{
		CLIENT_FindRecordClose(m_lFindHandle);
		m_lFindHandle = 0;
	}
}

void CRecordManagerDlg::OnBnClickedRecordCardnoCheck()
{
	int nCheck = m_btnCardNoCheck.GetCheck();
	if (BST_CHECKED == nCheck)
	{
		GetDlgItem(IDC_EDIT_RECORD_CARDNO)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_RECORD_CARDNO)->EnableWindow(FALSE);
	}
	// TODO: 在此添加控件通知处理程序代码
	
}


void CRecordManagerDlg::OnBnClickedCheckQuerytime()
{
	// TODO: 在此添加控件通知处理程序代码
	int nCheck = m_btnQeuryTimeCheck.GetCheck();
	if (BST_CHECKED == nCheck)
	{
		GetDlgItem(IDC_DATE_RECORD_BEGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_TIME_RECORD_BEGIN)->EnableWindow(TRUE);

		GetDlgItem(IDC_DATE_RECORD_END)->EnableWindow(TRUE);
		GetDlgItem(IDC_TIME_RECORD_END)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_DATE_RECORD_BEGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIME_RECORD_BEGIN)->EnableWindow(FALSE);

		GetDlgItem(IDC_DATE_RECORD_END)->EnableWindow(FALSE);
		GetDlgItem(IDC_TIME_RECORD_END)->EnableWindow(FALSE);
	}
}

void CRecordManagerDlg::CopyRecordInfo(tagRecordInfo *pRecordInfoDest, NET_RECORDSET_ACCESS_CTL_CARDREC *pRecordInfoSrc)
{
	if (pRecordInfoDest == NULL || pRecordInfoSrc == NULL)
	{
		return;
	}
	
	pRecordInfoDest->nRecNo = pRecordInfoSrc->nRecNo;
	strncpy(pRecordInfoDest->szCardNo, pRecordInfoSrc->szCardNo, sizeof(pRecordInfoDest->szCardNo) - 1);
	strncpy(pRecordInfoDest->szUserID, pRecordInfoSrc->szUserID, sizeof(pRecordInfoDest->szUserID) - 1);
	pRecordInfoDest->stuTime = pRecordInfoSrc->stuTime;
	pRecordInfoDest->bStatus = pRecordInfoSrc->bStatus;
	pRecordInfoDest->emCardType = pRecordInfoSrc->emCardType;
	strncpy(pRecordInfoDest->szSnapFtpUrl, pRecordInfoSrc->szSnapFtpUrl, sizeof(pRecordInfoSrc->szRecordURL) - 1);
	pRecordInfoDest->emMethod = pRecordInfoSrc->emMethod;
}
BOOL CRecordManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CRecordManagerDlg::SetDate(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwYear = stuTime.wYear;
	stNetTime.dwMonth = stuTime.wMonth;
	stNetTime.dwDay = stuTime.wDay;
}

void CRecordManagerDlg::SetTime(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwHour = stuTime.wHour;
	stNetTime.dwMinute = stuTime.wMinute;
	stNetTime.dwSecond = stuTime.wSecond;
}
void CRecordManagerDlg::OnBnClickedBtnRecordDelete()
{
	// TODO: 在此添加控件通知处理程序代码
	CListCtrl *pLcRecordInfo = (CListCtrl*)GetDlgItem(IDC_RECORD_LIST);
	int nSel = pLcRecordInfo->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		return;
	}

	tagRecordInfo *pRecordInfo = m_recordVector[nSel];

	NET_CTRL_RECORDSET_PARAM recordParam;
	memset(&recordParam, 0, sizeof(recordParam));
	recordParam.dwSize = sizeof(recordParam);
	recordParam.emType = NET_RECORD_ACCESSCTLCARDREC_EX;

	recordParam.pBuf = &pRecordInfo->nRecNo;
	recordParam.nBufLen = sizeof(int);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_REMOVE, (void*)&recordParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Delete record info fail"), ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Delete successfully"), ConvertString("Prompt"));
	}
}

void CRecordManagerDlg::OnBnClickedBtnRecordClear()
{
	// TODO: 在此添加控件通知处理程序代码
	NET_CTRL_RECORDSET_PARAM recordParam;
	memset(&recordParam, 0, sizeof(recordParam));
	recordParam.dwSize = sizeof(recordParam);
	recordParam.emType = NET_RECORD_ACCESSCTLCARDREC_EX;


	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_CLEAR, (void*)&recordParam );
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Clear record info fail"), ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Clear successfully"), ConvertString("Prompt"));
	}
}

void CRecordManagerDlg::OnLvnItemchangedRecordList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CListCtrl *pLcRecordInfo = (CListCtrl*)GetDlgItem(IDC_RECORD_LIST);
	int nSel = pLcRecordInfo->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		return;
	}

	tagRecordInfo *pRecordInfo = m_recordVector[nSel];


	//Get file path
	char path[1024];
	int filelen = GetModuleFileName(NULL,path,1024);
	int i = filelen;
	while(path[i]!='\\')
	{
		i--;
	}
	path[i + 1] = '\0';

	//Get file name
	CString filepath(path);
	CString filename = "record.JPG";
	CString strfile = filepath + filename;
	char *pFileName = strfile.GetBuffer(200);

	DH_IN_DOWNLOAD_REMOTE_FILE stuRemoteFileParm;
	memset(&stuRemoteFileParm, 0, sizeof(DH_IN_DOWNLOAD_REMOTE_FILE));
	stuRemoteFileParm.dwSize = sizeof(DH_IN_DOWNLOAD_REMOTE_FILE);
	stuRemoteFileParm.pszFileName = pRecordInfo->szSnapFtpUrl ;
	stuRemoteFileParm.pszFileDst = pFileName;
	DH_OUT_DOWNLOAD_REMOTE_FILE fileinfo = {sizeof(DH_OUT_DOWNLOAD_REMOTE_FILE)};

	BOOL BRet = CLIENT_DownloadRemoteFile(m_lLoginID, &stuRemoteFileParm, &fileinfo);
	if (BRet == FALSE)
	{
		LLONG nError = CLIENT_GetLastError();

		MessageBox(ConvertString("Download File fail"), ConvertString("Prompt"));
	}
	else
	{
		m_recordPic.SetImageFile(pFileName);
	}
	strfile.ReleaseBuffer();
	*pResult = 0;
}

void CRecordManagerDlg::OnBnClickedBtnPicDownload()
{
	CListCtrl *pLcRecordInfo = (CListCtrl*)GetDlgItem(IDC_RECORD_LIST);
	int nSel = pLcRecordInfo->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a card"), ConvertString("Prompt"));
		return;
	}
	
	CString strFilePathName;
	CFileDialog dlg(FALSE,"*.jpg","download.jpg",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
		"jpg(*.jpg)|*.jpg|All File(*.jpg)|*.*||",this);
	if(dlg.DoModal() == IDOK)
	{
		strFilePathName = dlg.GetPathName();
	} 
	else
	{
		return;
	}

	tagRecordInfo *pRecordInfo = m_recordVector[nSel];

	DH_IN_DOWNLOAD_REMOTE_FILE stuRemoteFileParm;
	memset(&stuRemoteFileParm, 0, sizeof(DH_IN_DOWNLOAD_REMOTE_FILE));
	stuRemoteFileParm.dwSize = sizeof(DH_IN_DOWNLOAD_REMOTE_FILE);
	stuRemoteFileParm.pszFileName = pRecordInfo->szSnapFtpUrl ;
	stuRemoteFileParm.pszFileDst = strFilePathName.GetBuffer();
	DH_OUT_DOWNLOAD_REMOTE_FILE fileinfo = {sizeof(DH_OUT_DOWNLOAD_REMOTE_FILE)};

	BOOL BRet = CLIENT_DownloadRemoteFile(m_lLoginID, &stuRemoteFileParm, &fileinfo);
	if (BRet == FALSE)
	{
		LLONG nError = CLIENT_GetLastError();

		MessageBox(ConvertString("Download File fail"), ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Download picture success"), ConvertString("Prompt"));
	}
	strFilePathName.ReleaseBuffer();
}
