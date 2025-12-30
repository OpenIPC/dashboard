// CardManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VTODemo.h"
#include "AddCardDlg.h"
#include "GetFingerDlg.h"

// CAddCardDlg 对话框
#define QUERY_COUNT 20

IMPLEMENT_DYNAMIC(CAddCardDlg, CDialog)

CAddCardDlg::CAddCardDlg(LLONG lLoginID, EM_Operate em_Operate, CardInfo& cardinfo, CWnd* pParent /*=NULL*/)
	: CDialog(CAddCardDlg::IDD, pParent)
{
	m_lLoginID = lLoginID; 
	m_nFingerprintLen = 0;
	m_emOperate = em_Operate;
	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	
	if(em_Operate == EM_Operate_modify)
	{
		if(cardinfo.nFingerLen)
		{
			memcpy(m_byFingerprintData, cardinfo.szFingerData, cardinfo.nFingerLen);
			m_nFingerprintLen = cardinfo.nFingerLen;
		}
	}

	memset(m_szFilePath, 0, sizeof(m_szFilePath));

	m_stuCardInfo = cardinfo;
}

CAddCardDlg::~CAddCardDlg()
{
}

BOOL CAddCardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	SetDlgItemText(IDC_EDIT_CARDNO, m_stuCardInfo.szCardNo);
	SetDlgItemText(IDC_EDIT_ROOMNO, m_stuCardInfo.szHomeNo);
	
	if(m_emOperate == EM_Operate_modify)
	{
		GetDlgItem(IDC_EDIT_CARDNO)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_ROOMNO)->EnableWindow(FALSE);
	}
	return TRUE;
}

void CAddCardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_IMAGE, m_addPic);
}


BEGIN_MESSAGE_MAP(CAddCardDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OPENPICTURE, &CAddCardDlg::OnBnClickedBtnOpenpicture)

	ON_BN_CLICKED(IDC_BTN_OK, &CAddCardDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_GET, &CAddCardDlg::OnBnClickedBtnGet)
END_MESSAGE_MAP()


// CAddCardDlg 消息处理程序

void CAddCardDlg::OnBnClickedBtnOpenpicture()
{
	// TODO: 在此添加控件通知处理程序代码
	char *pFilePath = NULL;
	CString PicPath;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, 
		"JPG files(*.jpg, *.JPG) | *.jpg; *.JPG |", NULL);
	if (dlg.DoModal() == IDOK)
	{
		PicPath = dlg.GetPathName();
		pFilePath = PicPath.GetBuffer(512);
		memcpy(m_szFilePath, pFilePath, sizeof(m_szFilePath));

		m_addPic.SetImageFile(PicPath);

		PicPath.ReleaseBuffer();
	}
}

BOOL CAddCardDlg::AddModifyCard()
{
	memset(&m_stuCardInfo, 0, sizeof(m_stuCardInfo));
	BOOL bRet = GetCardInfoFromUI();
	if(FALSE == bRet)
	{
		return FALSE;
	}

	if(m_szFilePath[0] == '\0' && m_emOperate == EM_Operate_add)
	{
		MessageBox(ConvertString("Face can't be empty"), ConvertString("Prompt"));
		return FALSE;
	}
	BOOL bAdd = FALSE;
	if(m_emOperate == EM_Operate_add)
	{
		bAdd = InsertCardInfo();
		if(bAdd)
		{
			MessageBox(ConvertString("Add card successfully"), ConvertString("Prompt"));
		}
	}
	else
	{
		bAdd = ModifyCardInfo();
		if(bAdd)
		{
			MessageBox(ConvertString("Modify card successfully"), ConvertString("Prompt"));
		}
	}
	if(!bAdd)
	{
		return FALSE;
	}

	
	return TRUE;
}

BOOL CAddCardDlg::GetCardInfoFromUI()
{
	CString strCardNo;
	CString strRoomNo;

	GetDlgItemText(IDC_EDIT_CARDNO, strCardNo);
	GetDlgItemText(IDC_EDIT_ROOMNO, strRoomNo);

	if(strCardNo == "" || strRoomNo == "")
	{
		MessageBox(ConvertString("CardNo or RoomNo can not empty"), ConvertString("Prompt"));
		return FALSE;
	}

	strncpy(m_stuCardInfo.szCardNo, strCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(m_stuCardInfo.szHomeNo, strRoomNo, DH_COMMON_STRING_16 - 1);

	return TRUE;
}
BOOL CAddCardDlg::InsertCardInfo()
{
	if(!isCardEmpty())
	{
		return FALSE;
	}

	BOOL bRet = FALSE;
	NET_CTRL_RECORDSET_INSERT_PARAM stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));

	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.stuCtrlRecordSetInfo.dwSize = sizeof(stuInParam.stuCtrlRecordSetInfo);
	stuInParam.stuCtrlRecordSetResult.dwSize = sizeof(stuInParam.stuCtrlRecordSetResult);
	stuInParam.stuCtrlRecordSetInfo.emType = NET_RECORD_ACCESSCTLCARD;

	NET_RECORDSET_ACCESS_CTL_CARD* pStuCardInfo = new NET_RECORDSET_ACCESS_CTL_CARD;
	if(pStuCardInfo == NULL)
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		return FALSE;
	}
	memset(pStuCardInfo, 0, sizeof(NET_RECORDSET_ACCESS_CTL_CARD));

	pStuCardInfo->dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	strncpy(pStuCardInfo->szCardNo, m_stuCardInfo.szCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(pStuCardInfo->szUserID, m_stuCardInfo.szHomeNo, DH_MAX_USERID_LEN - 1);

	pStuCardInfo->nDoorNum = 1;
	pStuCardInfo->sznDoors[0] = 0;

	pStuCardInfo->bEnableExtended = true;
	pStuCardInfo->stuFingerPrintInfoEx.nCount = 1;
	pStuCardInfo->stuFingerPrintInfoEx.nLength = m_nFingerprintLen;
	pStuCardInfo->stuFingerPrintInfoEx.nPacketLen = m_nFingerprintLen;
	pStuCardInfo->stuFingerPrintInfoEx.pPacketData = new char[m_nFingerprintLen];
	if(NULL == pStuCardInfo->stuFingerPrintInfoEx.pPacketData)
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		delete pStuCardInfo;
		pStuCardInfo = NULL;
		return FALSE;
	}
	memset(pStuCardInfo->stuFingerPrintInfoEx.pPacketData, 0, m_nFingerprintLen);
	memcpy(pStuCardInfo->stuFingerPrintInfoEx.pPacketData, m_byFingerprintData, m_nFingerprintLen);
	
	stuInParam.stuCtrlRecordSetInfo.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	stuInParam.stuCtrlRecordSetInfo.pBuf = (void*)pStuCardInfo;

	bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_INSERTEX, &stuInParam, DEFAULT_WAIT_TIME);

	if(pStuCardInfo)
	{
		if(pStuCardInfo->stuFingerPrintInfoEx.pPacketData)
		{
			delete []pStuCardInfo->stuFingerPrintInfoEx.pPacketData;
			pStuCardInfo->stuFingerPrintInfoEx.pPacketData = NULL;
		}

		delete pStuCardInfo;
		pStuCardInfo = NULL;
	}
	if(!bRet)
	{
		MessageBox(ConvertString("CLIENT_ControlDevice insert card fail"), ConvertString("Prompt"));
		return FALSE;
	}
	
//======================Add face info=============================
	if(m_szFilePath[0] == '\0' && m_emOperate == EM_Operate_modify)
	{
		DeleteCardByRecNo(stuInParam.stuCtrlRecordSetResult.nRecNo);
		goto FREE_RETURN;
	}
	NET_IN_ADD_FACE_INFO stuFaceInParam;
	memset(&stuFaceInParam, 0, sizeof(NET_IN_ADD_FACE_INFO));
	stuFaceInParam.dwSize = sizeof(stuFaceInParam);
	strncpy(stuFaceInParam.szUserID, m_stuCardInfo.szHomeNo, DH_MAX_USERID_LEN - 1);
	stuFaceInParam.stuFaceInfo.nFacePhoto = 1;

	FILE* fPic = fopen(m_szFilePath, "rb");
	if(fPic == NULL)
	{
		DeleteCardByRecNo(stuInParam.stuCtrlRecordSetResult.nRecNo);
		MessageBox(ConvertString("Open picture fail"), ConvertString("Prompt"));
		return FALSE;
	}

	fseek(fPic, 0, SEEK_END);
	int nLength = ftell(fPic);
	if(nLength <= 0)
	{
		DeleteCardByRecNo(stuInParam.stuCtrlRecordSetResult.nRecNo);
		MessageBox(ConvertString("Picture empty"), ConvertString("Prompt"));
		goto FREE_RETURN;
	}
	rewind(fPic);

	stuFaceInParam.stuFaceInfo.nRoom = 1;
	strncpy(stuFaceInParam.stuFaceInfo.szRoomNo[0], m_stuCardInfo.szHomeNo, sizeof(stuFaceInParam.stuFaceInfo.szRoomNo[0])-1);
	stuFaceInParam.stuFaceInfo.nFacePhotoLen[0] = nLength;
	stuFaceInParam.stuFaceInfo.pszFacePhoto[0] = new char[nLength];

	if (NULL == stuFaceInParam.stuFaceInfo.pszFacePhoto[0])
	{
		DeleteCardByRecNo(stuInParam.stuCtrlRecordSetResult.nRecNo);
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		goto FREE_RETURN;
	}

	memset(stuFaceInParam.stuFaceInfo.pszFacePhoto[0], 0, nLength);
	int nReadLen = fread(stuFaceInParam.stuFaceInfo.pszFacePhoto[0], 1, nLength, fPic);
	if (nReadLen <= 0)
	{
		DeleteCardByRecNo(stuInParam.stuCtrlRecordSetResult.nRecNo);
		goto FREE_RETURN;
	}

	NET_OUT_ADD_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(NET_OUT_ADD_FACE_INFO));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginID, EM_FACEINFO_OPREATE_ADD, (void*)&stuFaceInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		DeleteCardByRecNo(stuInParam.stuCtrlRecordSetResult.nRecNo);
		MessageBox(ConvertString("CLIENT_FaceInfoOpreate add face fail"), ConvertString("Prompt"));
	}

FREE_RETURN:
	if (fPic)
	{
		fclose(fPic);
		fPic = NULL;
	}
	if (stuFaceInParam.stuFaceInfo.pszFacePhoto[0] != NULL)
	{
		delete[] stuFaceInParam.stuFaceInfo.pszFacePhoto[0];
		stuFaceInParam.stuFaceInfo.pszFacePhoto[0] = NULL;
	}

	LLONG lError = CLIENT_GetLastError();
	return bRet;
}


void CAddCardDlg::OnBnClickedBtnOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_OK)->EnableWindow(FALSE);
	AddModifyCard();
	memset(m_szFilePath, 0, sizeof(m_szFilePath));	
	GetDlgItem(IDC_BTN_OK)->EnableWindow(TRUE);
	GetDlgItem(IDC_GETFINGER_STATUS)->ShowWindow(FALSE);
	m_addPic.SetImageFile(NULL);
	Invalidate(TRUE);
	OnOK();
}

void CAddCardDlg::OnBnClickedBtnGet()
{
	// TODO: 在此添加控件通知处理程序代码
	CGetFingerDlg *pdlg = new CGetFingerDlg();
	int nResponse = pdlg->DoModal();
	GetDlgItem(IDC_GETFINGER_STATUS)->ShowWindow(TRUE);
	if (nResponse == IDOK)
	{
		m_nFingerprintLen = pdlg->GetFingerPrintLen();
		if (m_nFingerprintLen <= 0)
		{
			MessageBox(ConvertString("No fingerprint data"), ConvertString("Prompt"));
			return;
		}

		pdlg->GetFingerPrintData(m_byFingerprintData, m_nFingerprintLen);	
		GetDlgItem(IDC_GETFINGER_STATUS)->SetWindowText(ConvertString("Collection Succeed"));
	}
	else
	{
		GetDlgItem(IDC_GETFINGER_STATUS)->SetWindowText(ConvertString("Collection Failed"));
	}
	delete pdlg;
	pdlg = NULL;
}

BOOL CAddCardDlg::ModifyCardInfo()
{
	BOOL bRet = FALSE;
	FILE* fPic = NULL;
	NET_CTRL_RECORDSET_PARAM stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	NET_IN_ADD_FACE_INFO stuFaceInParam;
	memset(&stuFaceInParam, 0, sizeof(NET_IN_ADD_FACE_INFO));

	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;

	NET_RECORDSET_ACCESS_CTL_CARD* pStuCardInfo = new NET_RECORDSET_ACCESS_CTL_CARD;
	if(pStuCardInfo == NULL)
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		return FALSE;
	}
	memset(pStuCardInfo, 0, sizeof(NET_RECORDSET_ACCESS_CTL_CARD));

	pStuCardInfo->dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	strncpy(pStuCardInfo->szCardNo, m_stuCardInfo.szCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(pStuCardInfo->szUserID, m_stuCardInfo.szHomeNo, DH_MAX_USERID_LEN - 1);

	pStuCardInfo->nRecNo = m_stuCardInfo.nRecNo;
	pStuCardInfo->nDoorNum = 1;
	pStuCardInfo->sznDoors[0] = 0;

	pStuCardInfo->bEnableExtended = true;
	pStuCardInfo->stuFingerPrintInfoEx.nCount = 1;
	pStuCardInfo->stuFingerPrintInfoEx.nLength = m_nFingerprintLen;
	pStuCardInfo->stuFingerPrintInfoEx.nPacketLen = m_nFingerprintLen;
	pStuCardInfo->stuFingerPrintInfoEx.pPacketData = new char[m_nFingerprintLen];
	if(NULL == pStuCardInfo->stuFingerPrintInfoEx.pPacketData)
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		delete pStuCardInfo;
		pStuCardInfo = NULL;
		return FALSE;
	}
	memset(pStuCardInfo->stuFingerPrintInfoEx.pPacketData, 0, m_nFingerprintLen);
	memcpy(pStuCardInfo->stuFingerPrintInfoEx.pPacketData, m_byFingerprintData, m_nFingerprintLen);

	stuInParam.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	stuInParam.pBuf = (void*)pStuCardInfo;

	bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_UPDATEEX, &stuInParam, DEFAULT_WAIT_TIME);
	if(pStuCardInfo)
	{
		if(pStuCardInfo->stuFingerPrintInfoEx.pPacketData)
		{
			delete []pStuCardInfo->stuFingerPrintInfoEx.pPacketData;
			pStuCardInfo->stuFingerPrintInfoEx.pPacketData = NULL;
		}

		delete pStuCardInfo;
		pStuCardInfo = NULL;
	}
	if(!bRet)
	{
		MessageBox(ConvertString("CLIENT_ControlDevice modify card fail"), ConvertString("Prompt"));
		return FALSE;
	}

//=========================Modify face info==============================
	if(m_szFilePath[0] == '\0' && m_emOperate == EM_Operate_modify)
	{
		goto FREE_RETURN;
	}
	stuFaceInParam.dwSize = sizeof(stuFaceInParam);
	strncpy(stuFaceInParam.szUserID, m_stuCardInfo.szHomeNo, DH_MAX_USERID_LEN - 1);
	stuFaceInParam.stuFaceInfo.nFacePhoto = 1;
	
	if(m_szFilePath[0] == '\0')
	{
		goto FREE_RETURN;
	}
	fPic = fopen(m_szFilePath, "rb");
	if(fPic == NULL)
	{
		MessageBox(ConvertString("Open picture fail"), ConvertString("Prompt"));
		return FALSE;
	}

	fseek(fPic, 0, SEEK_END);
	int nLength = ftell(fPic);
	if(nLength <= 0)
	{
		MessageBox(ConvertString("Picture empty"), ConvertString("Prompt"));
		goto FREE_RETURN;
	}
	rewind(fPic);

	stuFaceInParam.stuFaceInfo.nRoom = 1;
	strncpy(stuFaceInParam.stuFaceInfo.szRoomNo[0], m_stuCardInfo.szHomeNo, sizeof(stuFaceInParam.stuFaceInfo.szRoomNo[0]) - 1);
	stuFaceInParam.stuFaceInfo.nFacePhotoLen[0] = nLength;
	stuFaceInParam.stuFaceInfo.pszFacePhoto[0] = new char[nLength];

	if (NULL == stuFaceInParam.stuFaceInfo.pszFacePhoto[0])
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		goto FREE_RETURN;
	}

	memset(stuFaceInParam.stuFaceInfo.pszFacePhoto[0], 0, nLength);
	int nReadLen = fread(stuFaceInParam.stuFaceInfo.pszFacePhoto[0], 1, nLength, fPic);
	fclose(fPic);
	fPic = NULL;
	if (nReadLen <= 0)
	{
		goto FREE_RETURN;
	}

	NET_OUT_ADD_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(NET_OUT_ADD_FACE_INFO));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginID, EM_FACEINFO_OPREATE_UPDATE, (void*)&stuFaceInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("CLIENT_FaceInfoOpreate modify face fail"), ConvertString("Prompt"));
	}

FREE_RETURN:
	if (fPic)
	{
		fclose(fPic);
		fPic = NULL;
	}
	if (stuFaceInParam.stuFaceInfo.pszFacePhoto[0] != NULL)
	{
		delete[] stuFaceInParam.stuFaceInfo.pszFacePhoto[0];
		stuFaceInParam.stuFaceInfo.pszFacePhoto[0] = NULL;
	}

	LLONG lError = CLIENT_GetLastError();
	return bRet;
}

BOOL CAddCardDlg::PreTranslateMessage(MSG* pMsg)
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

BOOL CAddCardDlg::isCardEmpty()
{
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
		goto FREE_RETURN;
	}
	memset(pStuCardInfo, 0, sizeof(FIND_RECORD_ACCESSCTLCARD_CONDITION));
	pStuCardInfo->dwSize = sizeof(FIND_RECORD_ACCESSCTLCARD_CONDITION);

	strCardNo.Format("%s", m_stuCardInfo.szCardNo);
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
		MessageBox(ConvertString("CLIENT_FindRecord fail"), ConvertString("Prompt"));
		goto FREE_RETURN;
	}
	LLONG m_lFindHandle = stuOutParam.lFindeHandle;


FREE_RETURN:
	if (pStuCardInfo)
	{
		delete pStuCardInfo;
		pStuCardInfo = NULL;
	}

	if(!bRet)
	{
		return FALSE;
	}
//==============================DoFindNextRecord=======================================
	int nRet = -1;
	bRet = TRUE;
	NET_IN_FIND_NEXT_RECORD_PARAM stuNextInParam = {sizeof(stuNextInParam)};
	NET_OUT_FIND_NEXT_RECORD_PARAM	stuNextOutParam = {sizeof(stuNextOutParam)};

	// 内存申请失败，直接返回
	NET_RECORDSET_ACCESS_CTL_CARD* pAccessCardInfo = new NET_RECORDSET_ACCESS_CTL_CARD[QUERY_COUNT];
	if (NULL == pAccessCardInfo)
	{
		MessageBox(ConvertString("NEW Memory fail"), ConvertString("Prompt"));
		bRet = FALSE;
		goto ERROR_OUT;
	}

	memset(&stuNextInParam, 0, sizeof(stuNextInParam));
	stuNextInParam.dwSize = sizeof(stuNextInParam);
	stuNextInParam.lFindeHandle = m_lFindHandle;
	stuNextInParam.nFileCount = QUERY_COUNT;

	memset(&stuNextOutParam, 0, sizeof(NET_OUT_FIND_NEXT_RECORD_PARAM));
	stuNextOutParam.dwSize = sizeof(NET_OUT_FIND_NEXT_RECORD_PARAM);

	memset(pAccessCardInfo, 0, stuNextInParam.nFileCount * sizeof(NET_RECORDSET_ACCESS_CTL_CARD));

	for (int i = 0; i < stuNextInParam.nFileCount; i++)
	{
		pAccessCardInfo[i].dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	}
	stuNextOutParam.pRecordList = (void*)pAccessCardInfo;
	stuNextOutParam.nMaxRecordNum = stuNextInParam.nFileCount;

	nRet = CLIENT_FindNextRecord(&stuNextInParam, &stuNextOutParam, DEFAULT_WAIT_TIME);

	if(nRet > 0 && stuNextOutParam.nRetRecordNum > 0)
	{
		MessageBox(ConvertString("Card already existed!"), ConvertString("Prompt"));
		bRet = FALSE;
	}

ERROR_OUT:
	if (pAccessCardInfo)
	{
		delete[] pAccessCardInfo;
		pAccessCardInfo = NULL;
	}

	if (m_lFindHandle)
	{
		CLIENT_FindRecordClose(m_lFindHandle);
		m_lFindHandle = 0;
	}

	return bRet;
}

BOOL CAddCardDlg::DeleteCardByRecNo(int nRecNo)
{
	NET_CTRL_RECORDSET_PARAM stuInParam = {sizeof(stuInParam)};
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;
	stuInParam.pBuf = &nRecNo;
	stuInParam.nBufLen = sizeof(int);

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_REMOVE, &stuInParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("CLIENT_ControlDevice  delete cardinfo fail"), ConvertString("Prompt"));
	}

	return bRet;
}