// PersonOpreateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "PersonOpreateDlg.h"
#include "AddPersonDlg.h"
#include <atlconv.h>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPersonOpreateGlg dialog


CPersonOpreateGlg::CPersonOpreateGlg(const LLONG lLoginHandle, const char* pszGroupId, const char* pszGroupName, char* pstSoftPath, CWnd* pParent /*=NULL*/)
	: CDialog(CPersonOpreateGlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_lLoginID = lLoginHandle;
	m_nCurPos = 0;
	m_nPersonCount = 0;
	m_pstPersonSelectInfo = NULL;
	m_pszSoftPath = pstSoftPath;
	strncpy(m_szGroupId, pszGroupId, sizeof(m_szGroupId)-1);
	strncpy(m_szGroupName, pszGroupName, sizeof(m_szGroupName)-1);
	memset(m_szSelectPersonFile, 0, sizeof(m_szSelectPersonFile));
	m_lFindPersonHandle = 0;
	m_pPersonInfoList = NULL;

	if (NULL == m_pPersonInfoList)
	{
		m_pPersonInfoList = new NET_PERSON_LIST_INFO[10];
		if (m_pPersonInfoList)
		{
			memset(m_pPersonInfoList, 0, sizeof(NET_PERSON_LIST_INFO)*10);
			for (int i = 0; i < 10; i++)
			{
				NET_PERSON_LIST_INFO *pstPersonListInfo = m_pPersonInfoList + i;
				pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath = new char[256];
				if (pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath)
				{
					memset(pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath, 0, 256);
				}
			}
		}
	}
	//{{AFX_DATA_INIT(CPersonOpreateGlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPersonOpreateGlg::~CPersonOpreateGlg()
{
	DeleteFile(m_szSelectPersonFile);

	if (m_lFindPersonHandle)
	{
		CLIENT_StopFindFaceRecognition(m_lFindPersonHandle);
		m_lFindPersonHandle = 0;
	}

	if (m_pPersonInfoList)
	{
		for (int i = 0; i < 10; i++)
		{
			NET_PERSON_LIST_INFO *pstPersonListInfo = m_pPersonInfoList + i;
			if (pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath)
			{
				delete[] pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath;
				pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath = NULL;
			}
		}

		delete[] m_pPersonInfoList;
		m_pPersonInfoList = NULL;
		m_pstPersonSelectInfo= NULL;
	}
}

BOOL CPersonOpreateGlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	SetDlgItemText(IDC_PersonGroupID, m_szGroupId);
	SetDlgItemText(IDC_PersonGroupName, m_szGroupName);

	m_cbSexType.ResetContent();
	m_cbSexType.InsertString(-1, ConvertString("Unlimited"));
	m_cbSexType.InsertString(-1, ConvertString("Male"));
	m_cbSexType.InsertString(-1, ConvertString("Female"));
	m_cbSexType.SetCurSel(0);

	m_cbCardType.ResetContent();	
	m_cbCardType.InsertString(-1, ConvertString("Unlimited"));
	m_cbCardType.InsertString(-1, ConvertString("IC"));
	m_cbCardType.InsertString(-1, ConvertString("Passport"));	
	m_cbCardType.SetCurSel(0);

	GetDlgItem(IDC_BUTTON_PER)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);

	for (int i = 0; i < 10; i++)
	{
		GetDlgItem(IDC_RADIO_PERSON1+i)->EnableWindow(FALSE);
	}

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	FillCWndWithDefaultColor(GetDlgItem(IDC_SELECT_PersonPicture));
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CPersonOpreateGlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPersonOpreateGlg)
	DDX_Control(pDX, IDC_COMBO_SEX, m_cbSexType);
	DDX_Control(pDX, IDC_COMBO_CARDTYPE, m_cbCardType);
	//DDX_DateTimeCtrl(pDX, IDC_PersonBirthDayStart, m_BirthDayStart);
	//DDX_DateTimeCtrl(pDX, IDC_PersonBirthDayEnd, m_BirthDayEnd);

	DDX_Control(pDX, IDC_PersonBirthDayStart, m_BirthDayStart);
	DDX_Control(pDX, IDC_PersonBirthDayEnd, m_BirthDayEnd);
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_SELECT_PersonPicture, m_pPersonPic);
}


BEGIN_MESSAGE_MAP(CPersonOpreateGlg, CDialog)
	//{{AFX_MSG_MAP(CPersonOpreateGlg)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_PER, OnButtonPer)
	ON_BN_CLICKED(IDC_BUTTON_PERSON_ADD, OnButtonPersonAdd)
	ON_BN_CLICKED(IDC_BUTTON_PERSON_EDIT, OnButtonPersonEdit)
	ON_BN_CLICKED(IDC_RADIO_PERSON1, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON2, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON3, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON4, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON5, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON6, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON7, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON8, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON9, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_RADIO_PERSON10, OnSelectPersonInfo)
	ON_BN_CLICKED(IDC_BUTTON_PERSON_DEL, OnButtonPersonDel)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CPersonOpreateGlg::OnBnClickedSearch)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////


void CPersonOpreateGlg::ClearShowCandidateInfo()
{
	CString str;
	for(int i = 0; i < 10; i++)
	{
		int nId = IDC_EDIT_NAME1+i;
		SetDlgItemText(nId, "");

		nId = IDC_EDIT_INDEX1+i;
		SetDlgItemText(nId, "");
			
		nId = IDC_EDIT_SEX1 + i;
		SetDlgItemText(nId, "");
			
		nId = IDC_EDIT_CITY1 + i;
		SetDlgItemText(nId, "");
			
		nId = IDC_EDIT_CARDTYPE1 + i;
		SetDlgItemText(nId, "");
			
		nId = IDC_EDIT_CARDNUMBER1 + i;
		SetDlgItemText(nId, "");

		nId = IDC_BIRTHDAY1 + i;
		SetDlgItemText(nId, "");

		nId = IDC_BIRTHDAY_MONTH1 + i;
		SetDlgItemText(nId, "");

		nId = IDC_BIRTHDAY_DAY1 + i;
		SetDlgItemText(nId, "");

		nId = IDC_RADIO_PERSON1+i;
		((CButton*)GetDlgItem(nId))->SetCheck(0);
		GetDlgItem(nId)->EnableWindow(FALSE);

		FillCWndWithDefaultColor(GetDlgItem(IDC_SELECT_PersonPicture));
		m_pPersonPic.SetImageFile(NULL);
	}	
}

void CPersonOpreateGlg::ShowCandidateInfo(int nCount)
{
	if (m_pPersonInfoList && nCount <= 10)
	{
		CString str;
		for (int i = 0; i < nCount; i++)
		{
			NET_PERSON_LIST_INFO *pstPersonListInfo = m_pPersonInfoList + i;
			CANDIDATE_INFOEX *pstCandidateInfo = &(pstPersonListInfo->stuCandidate);
			FACERECOGNITION_PERSON_INFOEX *pstPersonInfo = &(pstCandidateInfo->stPersonInfo);

			int nId = IDC_EDIT_NAME1+i;
			str.Format("%s", pstPersonInfo->szPersonName);
			SetDlgItemText(nId, str);

			nId = IDC_EDIT_INDEX1 + i;
			str.Format("%d", pstPersonListInfo->nNumber);
			SetDlgItemText(nId, str);

			nId = IDC_EDIT_SEX1 + i;
			if (1 == pstPersonInfo->bySex)
			{
				SetDlgItemText(nId, ConvertString("Male"));
			}
			else if (2 == pstPersonInfo->bySex)
			{
				SetDlgItemText(nId, ConvertString("Female"));
			}

			char szBirthday[11] = {0};
			_snprintf(szBirthday, sizeof(szBirthday)-1, "%04d-%02d-%02d", pstPersonInfo->wYear, pstPersonInfo->byMonth, pstPersonInfo->byDay);
			nId = IDC_BIRTHDAY1 + i;
			str.Format("%s", szBirthday);
			SetDlgItemText(nId, str);

			nId = IDC_EDIT_CARDTYPE1 + i;
			if (1 == pstPersonInfo->byIDType)
			{
				SetDlgItemText(nId, ConvertString("IC"));
			}
			else if (2 == pstPersonInfo->byIDType)
			{
				SetDlgItemText(nId, ConvertString("Passport"));
			}

			nId = IDC_EDIT_CARDNUMBER1 + i;
			str.Format("%s", pstPersonInfo->szID);
			SetDlgItemText(nId, str);

			nId = IDC_RADIO_PERSON1+i;
			GetDlgItem(nId)->EnableWindow(TRUE);
		}
	}
}

void CPersonOpreateGlg::SearchPersonInfoFromGroup()
{
	if (0 == m_nPersonCount)
	{
		return;
	}

	BOOL bRet = FALSE;
	
	NET_IN_DOFIND_FACERECONGNITION stuInDoFind = {sizeof(stuInDoFind)};
	NET_OUT_DOFIND_FACERECONGNITION stuOutDoFind = {sizeof(stuOutDoFind)};
	stuOutDoFind.bUseCandidatesEx = TRUE;
	
	const int nQueryNum = 10;
	const int nFilePathLen = 256;
	for (int n = 0; n < nQueryNum; n++)
	{
		stuOutDoFind.stuCandidatesEx[n].stPersonInfo.szFacePicInfo[0].pszFilePath = new char[nFilePathLen];
		stuOutDoFind.stuCandidatesEx[n].stPersonInfo.szFacePicInfo[0].nFilePathLen = nFilePathLen;
	}

	stuInDoFind.lFindHandle = m_lFindPersonHandle;
	stuInDoFind.emDataType = EM_NEEDED_PIC_TYPE_HTTP_URL;
	stuInDoFind.nCount = nQueryNum;
	int nBegin = m_nCurPos;
	int nCountIndex = m_nCurPos;
	
	stuInDoFind.nBeginNum = m_nCurPos;
	bRet = CLIENT_DoFindFaceRecognition(&stuInDoFind, &stuOutDoFind, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Failed to do find target recognition!"), "");
		goto FREE_DATA;
	}

	for (int i = 0; i < stuOutDoFind.nCadidateExNum; i++)
	{
		NET_PERSON_LIST_INFO *pstPersonListInfo = m_pPersonInfoList+i;
		char*  pszFilePath = pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath;

		memset(pstPersonListInfo, 0, sizeof(*pstPersonListInfo));
		pstPersonListInfo->nIndex = IDC_RADIO_PERSON1 + nCountIndex;
		pstPersonListInfo->nNumber = nCountIndex+1;
		memcpy(&(pstPersonListInfo->stuCandidate),&(stuOutDoFind.stuCandidatesEx[i]), sizeof(CANDIDATE_INFOEX));
		pstPersonListInfo->stuCandidate.stPersonInfo.wFacePicNum = 1;
		pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath = pszFilePath;
		if (pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath)
		{
			memset(pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath, 0, nFilePathLen);
			strncpy(pstPersonListInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath, stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath, nFilePathLen - 1);
		}
		nCountIndex++;
	}

	if (stuOutDoFind.nCadidateExNum < stuInDoFind.nCount)
	{
		GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
	}

	ShowCandidateInfo(stuOutDoFind.nCadidateExNum);

FREE_DATA:
	for (int n = 0; n < nQueryNum; n++)
	{
		if (stuOutDoFind.stuCandidatesEx[n].stPersonInfo.szFacePicInfo[0].pszFilePath)
		{
			delete stuOutDoFind.stuCandidatesEx[n].stPersonInfo.szFacePicInfo[0].pszFilePath;
		}
		stuOutDoFind.stuCandidatesEx[n].stPersonInfo.szFacePicInfo[0].pszFilePath = NULL;
	}
	
}

void CPersonOpreateGlg::StartFindPersonInfo(BOOL bDelete)
{
	if (0 != m_lFindPersonHandle)
	{
		BOOL bRet = CLIENT_StopFindFaceRecognition(m_lFindPersonHandle);
		if (!bRet)
		{
			MessageBox(ConvertString("Failed to stop find!"), "");
		}
		m_lFindPersonHandle = 0;
		m_nCurPos = 0;
	}

	ClearShowCandidateInfo();
	GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_PER)->EnableWindow(FALSE);

	if (NULL == m_pPersonInfoList)
	{
		return;
	}

	NET_IN_STARTFIND_FACERECONGNITION stuInParam = {sizeof(stuInParam)};
	NET_OUT_STARTFIND_FACERECONGNITION stuOutParam = {sizeof(stuOutParam)};

	stuInParam.stMatchOptions.dwSize = sizeof(stuInParam.stMatchOptions);
	stuInParam.stFilterInfo.dwSize = sizeof(stuInParam.stFilterInfo);
	stuInParam.bPersonExEnable = TRUE;

	stuInParam.stFilterInfo.nRangeNum = 1;
	stuInParam.stFilterInfo.szRange[0] = (BYTE)NET_FACE_DB_TYPE_BLACKLIST;

	CString strName;
	GetDlgItemText(IDC_EDIT_NAME, strName);
	char *pcName = (LPSTR)(LPCSTR)strName;
	if (strlen(pcName) > 0)
	{
		strncpy(stuInParam.stPersonInfoEx.szPersonName, pcName, sizeof(stuInParam.stPersonInfoEx.szPersonName)-1);
	}
	stuInParam.stPersonInfoEx.bySex = m_cbSexType.GetCurSel();

	SYSTEMTIME DataStart = {0};
	CDateTimeCtrl *pCDataStart = (CDateTimeCtrl*)GetDlgItem(IDC_PersonBirthDayStart);
	DWORD dwResult = pCDataStart->GetTime(&DataStart);
	if (dwResult == GDT_VALID)
	{
		stuInParam.stFilterInfo.stBirthdayRangeStart.dwYear = DataStart.wYear;
		stuInParam.stFilterInfo.stBirthdayRangeStart.dwMonth = DataStart.wMonth;
		stuInParam.stFilterInfo.stBirthdayRangeStart.dwDay = DataStart.wDay;
	}

	SYSTEMTIME DataEnd = {0};
	dwResult = m_BirthDayEnd.GetTime(&DataEnd);
	if (dwResult == GDT_VALID)
	{
		stuInParam.stFilterInfo.stBirthdayRangeEnd.dwYear = DataEnd.wYear;
		stuInParam.stFilterInfo.stBirthdayRangeEnd.dwMonth = DataEnd.wMonth;
		stuInParam.stFilterInfo.stBirthdayRangeEnd.dwDay = DataEnd.wDay;
	}

	stuInParam.stPersonInfoEx.byIDType = m_cbCardType.GetCurSel();

	CString strCard;
	GetDlgItemText(IDC_EDIT_CARD_NUM, strCard);
	char *pcCard = (LPSTR)(LPCSTR)strCard;
	if (strlen(pcCard) > 0)
	{
		strncpy(stuInParam.stPersonInfoEx.szID, pcCard, sizeof(stuInParam.stPersonInfoEx.szID)-1);
	}
	strncpy(stuInParam.stFilterInfo.szGroupId[0], m_szGroupId, sizeof(m_szGroupId)-1);
	stuInParam.stFilterInfo.nGroupIdNum = 1;

	strncpy(stuInParam.stPersonInfoEx.szGroupID, m_szGroupId, sizeof(stuInParam.stPersonInfoEx.szGroupID)-1);

	BOOL bRet = CLIENT_StartFindFaceRecognition(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Failed to start search target recognition!"), "");
		return;
	}

	m_nPersonCount = stuOutParam.nTotalCount;

	if ((stuOutParam.nTotalCount == 0) && (FALSE == bDelete))
	{
		MessageBox(ConvertString("No person!"), "");
		return;
	}

	if (stuOutParam.nTotalCount > 10)
	{
		GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);
	}

	m_lFindPersonHandle = stuOutParam.lFindHandle;
}

void CPersonOpreateGlg::OnOK() 
{	
	BOOL bValid = UpdateData(TRUE);
	if (!bValid)
	{
		return;
	}

	StartFindPersonInfo();
	SearchPersonInfoFromGroup();

	FillCWndWithDefaultColor(GetDlgItem(IDC_SELECT_PersonPicture));
}

void CPersonOpreateGlg::OnButtonNext() 
{
	m_nCurPos += 10;
	ClearShowCandidateInfo();

	SearchPersonInfoFromGroup();

	GetDlgItem(IDC_BUTTON_PER)->EnableWindow(TRUE);
}

void CPersonOpreateGlg::OnButtonPer() 
{
	if (m_nCurPos >= 10)
	{
		m_nCurPos -= 10;
		ClearShowCandidateInfo();
		SearchPersonInfoFromGroup();
		GetDlgItem(IDC_BUTTON_NEXT)->EnableWindow(TRUE);
		if (0 == m_nCurPos)
		{
			GetDlgItem(IDC_BUTTON_PER)->EnableWindow(FALSE);
		}
	}
}

void CPersonOpreateGlg::OnButtonPersonAdd() 
{
	CAddPersonDlg AddPerson(m_lLoginID, m_szGroupId, m_szGroupName, FACE_PERSON_ADD);
	int nResponse = AddPerson.DoModal();
 	//if (nResponse == IDOK)
 	{
		StartFindPersonInfo();

		SearchPersonInfoFromGroup();
 	}
	m_pstPersonSelectInfo = NULL;
}

void CPersonOpreateGlg::OnButtonPersonEdit() 
{
	if (NULL == m_pstPersonSelectInfo)
	{
		MessageBox(ConvertString("Please select a person!"), "");
		return;
	}
	
	CANDIDATE_INFOEX *pstCandidateInfo = &(m_pstPersonSelectInfo->stuCandidate);
	CAddPersonDlg AddPerson(m_lLoginID, m_szGroupId, m_szGroupName, FACE_PERSON_EDIT, pstCandidateInfo);
	int nResponse = AddPerson.DoModal();
// 	if (nResponse != IDOK)
// 	{
// 		return;
// 	}

	StartFindPersonInfo();
	ClearShowCandidateInfo();

	SearchPersonInfoFromGroup();
	m_pstPersonSelectInfo = NULL;
}

void CPersonOpreateGlg::OnSelectPersonInfo()
{
	BOOL bSelect = FALSE;
	int nIndex = 0;
	int i = 0;

	for (nIndex = IDC_RADIO_PERSON1; (nIndex < (nIndex+PERPAGE_INDEX_COUNT)) && (i < 10); nIndex++, i++)
	{
		if (((CButton *)GetDlgItem(nIndex))->GetCheck())
		{
			m_pstPersonSelectInfo = m_pPersonInfoList + i;
			bSelect = TRUE;
			break;			
		}
	}

	if (FALSE == bSelect)
	{
		return;
	}

	char szSrcPath[256] = {0};
	char szFileDst[256] = {0};
	strncpy(szSrcPath, m_pstPersonSelectInfo->stuCandidate.stPersonInfo.szFacePicInfo[0].pszFilePath, sizeof(szSrcPath)-1);
	strncpy(szFileDst, "Face\\RemoteFace\\SepectPerson.jpg", sizeof(szFileDst)-1);
						
	CString filepath(m_pszSoftPath);
	CString filename(szFileDst);
	CString strFile = filepath + filename;
	char *pFileDst = strFile.GetBuffer(256);

	DeleteFile(pFileDst);
	
	DH_IN_DOWNLOAD_REMOTE_FILE stuInFile = {sizeof(stuInFile)};
	DH_OUT_DOWNLOAD_REMOTE_FILE stuOutFile = {sizeof(stuOutFile)};
	stuInFile.pszFileName = szSrcPath;
	stuInFile.pszFileDst = pFileDst;
	BOOL bRet = CLIENT_DownloadRemoteFile(m_lLoginID, &stuInFile, &stuOutFile, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Download remote face picture failed!"), "");
		return;
	}

	m_pPersonPic.SetImageFile(pFileDst);
	
	memset(m_szSelectPersonFile, 0, sizeof(m_szSelectPersonFile));
	strncpy(m_szSelectPersonFile, pFileDst, sizeof(m_szSelectPersonFile)-1);
}


void CPersonOpreateGlg::OnButtonPersonDel() 
{
	if (NULL == m_pstPersonSelectInfo)
	{
		MessageBox(ConvertString("Please select a person!"), "");
		return;
	}
	
	CANDIDATE_INFOEX *pstCandidateInfo = &(m_pstPersonSelectInfo->stuCandidate);
	NET_IN_OPERATE_FACERECONGNITIONDB stuInParam = {sizeof(stuInParam)};
	NET_OUT_OPERATE_FACERECONGNITIONDB stuOutParam = {sizeof(stuOutParam)};
	stuInParam.emOperateType = NET_FACERECONGNITIONDB_DELETE;
	stuInParam.bUsePersonInfoEx = TRUE;
	strncpy(stuInParam.stPersonInfoEx.szUID, m_pstPersonSelectInfo->stuCandidate.stPersonInfo.szUID, sizeof(stuInParam.stPersonInfoEx.szUID)-1);
	strncpy(stuInParam.stPersonInfoEx.szGroupID, m_szGroupId, sizeof(stuInParam.stPersonInfoEx.szGroupID)-1);
	BOOL bRet = CLIENT_OperateFaceRecognitionDB(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		MessageBox(ConvertString("Failed to delete this person info!"), "");
	}

	StartFindPersonInfo(TRUE);
	ClearShowCandidateInfo();

	SearchPersonInfoFromGroup();
	m_pstPersonSelectInfo = NULL;
}

void CPersonOpreateGlg::OnBnClickedSearch()
{
	OnOK();
}


BOOL CPersonOpreateGlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
