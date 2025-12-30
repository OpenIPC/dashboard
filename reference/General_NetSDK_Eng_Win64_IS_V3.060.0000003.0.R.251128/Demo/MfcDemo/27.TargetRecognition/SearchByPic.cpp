// SearchByPic.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "SearchByPic.h"
#include <atlconv.h>
#include <assert.h>

#define WM_UPDATE_BUTTON_UI (WM_USER + 142)
#define WM_SETCOUNT_UI		(WM_USER + 143)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_FILE_PATH_LEN 256
#define FILESEARCH_TIMER_MSEC 62.5

void CALLBACK FaceFindState(LLONG lLoginID, LLONG lAttachHandle, NET_CB_FACE_FIND_STATE* pstStates, int nStateNum, LDWORD dwUser)
{
	if (pstStates == NULL || nStateNum == 0 || dwUser == 0)
	{
		return;
	}

	CSearchByPic *dlg = (CSearchByPic*)dwUser;
	dlg->m_nProgress = pstStates->nProgress;
	dlg->m_nTotalCount = pstStates->nCurrentCount;
	HWND hwnd = dlg->GetSafeHwnd();

	if (pstStates->nProgress == 100)
	{
		::PostMessage(hwnd, WM_UPDATE_BUTTON_UI, WPARAM(dwUser) , 0);
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
// CSearchByPic dialog


CSearchByPic::CSearchByPic(const LLONG lLoginHandle, const int m_nChnCnt, char *pSoftPath /*= NULL*/, CWnd* pParent /*=NULL*/)
: CDialog(CSearchByPic::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSearchByPic)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginHandle;
	memset(m_szFilePath, 0, sizeof(m_szFilePath));
	m_nProgress = 0;
	m_nChannelCnt = m_nChnCnt;
	m_pSoftPath = NULL;
	if (pSoftPath)
	{
		m_pSoftPath = pSoftPath;
	}

	m_nTotalCount = 0;
	m_dwThreadID = 0;
	m_lFindHandle = 0;
	m_lAttachHandle = 0;
	m_nTimer = 0;
	m_lEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CSearchByPic::~CSearchByPic()
{
	if (m_lEvent)
	{
		CloseHandle(m_lEvent);
		m_lEvent = 0;
	}

	StopFindHandle();
	StopAttchHandle();

	ClearCandidateInfo();
}

void CSearchByPic::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchByPic)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SEARCH_CHANNEL, m_comboChannel);
	//	DDX_Control(pDX, IDC_SEARCH_DBTYPE, m_comDbType);
	DDX_Control(pDX, IDC_SEARCH_PROGRESS, m_ctlSearchPro);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_OPEN_SEARCH_PIC, m_pPicShow);
	DDX_Control(pDX, IDC_RESULT_PIC2, m_pResultPic2);
	DDX_Control(pDX, IDC_RESULT_PIC, m_pResultPic1);
}


BEGIN_MESSAGE_MAP(CSearchByPic, CDialog)
	//{{AFX_MSG_MAP(CSearchByPic)
	ON_BN_CLICKED(IDC_OPEN_PIC, OnOpenPic)
	ON_BN_CLICKED(IDC_SEARCH_PIC, OnSearchPic)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_SEARCHPIC_STOP, &CSearchByPic::OnBnClickedButtonSearchpicStop)
	ON_MESSAGE(WM_UPDATE_BUTTON_UI, &CSearchByPic::OnDeviceAttch)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchByPic message handlers

void CSearchByPic::ClearCandidateInfo()
{
	FaceLock locker(m_Mutex);
	std::list<CANDIDATE_INFOEX*>::iterator it = m_lstCandidateInfo.begin();
	while(it != m_lstCandidateInfo.end())
	{
		CANDIDATE_INFOEX *pstCandidateInfo = *it;
		if (pstCandidateInfo)
		{
			if (pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath)
			{
				delete[] pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath;
				pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath = NULL;
			}
			delete pstCandidateInfo;
			pstCandidateInfo = NULL;
		}
		m_lstCandidateInfo.erase(it++);
	}
	locker.UnLock();
}

BOOL CSearchByPic::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// TODO: Add extra initialization here
	CString str;
	int nIndex = 0;
	m_comboChannel.ResetContent();
	for(int i=0;i < m_nChannelCnt;i++)
	{
		str.Format("%d",i + 1);
		nIndex = m_comboChannel.AddString(str);
		m_comboChannel.SetItemData(nIndex,i);
	}
	m_comboChannel.SetCurSel(0);

	CComboBox *pSimilarity = (CComboBox*)GetDlgItem(IDC_SEARCH_SIMILARITY_COMBOX);
	for (int i = 1; i <= 100; i++)
	{
		str.Format("%d", i);
		pSimilarity->AddString(str);
	}
	pSimilarity->SetCurSel(54);

	int nYear = 0;
	int nMonth = 0;
	int nDay = 0;

	CTime curTime = CTime::GetCurrentTime();
	nYear = curTime.GetYear();
	nMonth = curTime.GetMonth();
	nDay = curTime.GetDay();

	if (nMonth > 1)
	{
		str.Format("%d", nYear);
		SetDlgItemText(IDC_SEARCH_StartYear, str);

		str.Format("%d", nMonth-1);
		SetDlgItemText(IDC_SEARCH_StartMonth, str);
	}
	else
	{
		str.Format("%d", nYear-1);
		SetDlgItemText(IDC_SEARCH_StartYear, str);

		str.Format("%d", 12);
		SetDlgItemText(IDC_SEARCH_StartMonth, str);
	}

	str.Format("%d", nDay);
	SetDlgItemText(IDC_SEARCH_StartDay, str);

	str.Format("%d", nYear);
	SetDlgItemText(IDC_SEARCH_EndYear, str);
	str.Format("%d", nMonth);
	SetDlgItemText(IDC_SEARCH_EndMonth, str);
	str.Format("%d", nDay);
	SetDlgItemText(IDC_SEARCH_EndDay, str);

	m_ctlSearchPro.SetRange32(0,200);
	m_ctlSearchPro.SetPos(0);

	GetDlgItem(IDC_SEARCH_PIC)->EnableWindow(FALSE);

	CString filepath(m_pSoftPath);
	CString filename("BackGround.jpg");
	CString strFile = filepath + filename;
	char *pFileDst = strFile.GetBuffer(256);
	
	m_pResultPic1.SetImageFile(pFileDst);
	m_pResultPic2.SetImageFile(pFileDst);

	if (0 == m_nTimer)
	{
		m_nTimer = SetTimer(1, FILESEARCH_TIMER_MSEC, NULL);
	}

	GetDlgItem(IDC_BUTTON_SEARCHPIC_STOP)->EnableWindow(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CSearchByPic::OnOpenPic() 
{
	ClearShowPersonInfoSearchByPic();

	USES_CONVERSION;

	char *pFilePath = NULL;
	CString PicPath;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, 
	"JPG files(*.jpg, *.JPG) | *.jpg; *.JPG |", NULL);
	if (dlg.DoModal() == IDOK)
	{
		PicPath = dlg.GetPathName();
		pFilePath = PicPath.GetBuffer(512);
		memcpy(m_szFilePath, pFilePath, sizeof(m_szFilePath));

		m_pPicShow.SetImageFile(PicPath);

		PicPath.ReleaseBuffer();
		ClearShowPersonInfoSearchByPic();
		GetDlgItem(IDC_SEARCH_PIC)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SEARCHPIC_STOP)->EnableWindow(FALSE);
	}
}

DWORD WINAPI DoFindTargetRecognition(LPVOID lpParam)
{
	if (lpParam == 0)
	{
		return 0;
	}

	CSearchByPic *dlg = (CSearchByPic*)lpParam;

	HWND hwnd = dlg->GetSafeHwnd();
	::PostMessage(hwnd, WM_UPDATE_BUTTON_UI, WPARAM(lpParam) , 0);

	return 1;
}

void CSearchByPic::ClearShowPersonInfoSearchByPic()
{
	m_ctlSearchPro.SetRange32(0,200);
	m_ctlSearchPro.SetPos(0);

	m_nProgress = 0;

	SetDlgItemText(IDC_RESULT_NAME, "");
	SetDlgItemText(IDC_RESULT_SEX, "");
	SetDlgItemText(IDC_RESULT_YEAR, "");
	SetDlgItemText(IDC_RESULT_MONTH, "");
	SetDlgItemText(IDC_RESULT_DAY, "");
	SetDlgItemText(IDC_RESULT_AGE, "");
	SetDlgItemText(IDC_RESULT_CARDTYPE, "");
	SetDlgItemText(IDC_RESULT_CARDID, "");
	SetDlgItemText(IDC_RESULT_GLASSES, "");
	SetDlgItemText(IDC_RESULT_EYE, "");
	SetDlgItemText(IDC_RESULT_Mouth, "");
	SetDlgItemText(IDC_RESULT_Mask, "");
	SetDlgItemText(IDC_RESULT_Beard, "");
	SetDlgItemText(IDC_RESULT_Attractive, "");
	SetDlgItemText(IDC_EDIT_PIC_SIMILARITY1, "");

	SetDlgItemText(IDC_RESULT_NAME2, "");
	SetDlgItemText(IDC_RESULT_SEX2, "");
	SetDlgItemText(IDC_RESULT_YEAR2, "");
	SetDlgItemText(IDC_RESULT_MONTH2, "");
	SetDlgItemText(IDC_RESULT_DAY2, "");
	SetDlgItemText(IDC_RESULT_AGE2, "");
	SetDlgItemText(IDC_RESULT_CARDTYPE2, "");
	SetDlgItemText(IDC_RESULT_CARDID2, "");
	SetDlgItemText(IDC_RESULT_GLASSES2, "");
	SetDlgItemText(IDC_RESULT_EYE2, "");
	SetDlgItemText(IDC_RESULT_Mouth2, "");
	SetDlgItemText(IDC_RESULT_Mask2, "");
	SetDlgItemText(IDC_RESULT_Beard2, "");
	SetDlgItemText(IDC_RESULT_Attractive2, "");
	SetDlgItemText(IDC_EDIT_PIC_SIMILARITY2, "");

	FillCWndWithDefaultColor(GetDlgItem(IDC_RESULT_PIC));
	FillCWndWithDefaultColor(GetDlgItem(IDC_RESULT_PIC2));

	CString filepath(m_pSoftPath);
	CString filename("BackGround.jpg");
	CString strFile = filepath + filename;
	char *pFileDst = strFile.GetBuffer(256);
	m_pResultPic1.SetImageFile(pFileDst);
	m_pResultPic2.SetImageFile(pFileDst);
	strFile.ReleaseBuffer();
}

void CSearchByPic::ShowPersonInfoSearchByPic(FACERECOGNITION_PERSON_INFOEX *pstPersonInfo, int index, int nbySimilarity)
{
	CString str;

	str.Format("%d", nbySimilarity);
	SetDlgItemText(IDC_EDIT_PIC_SIMILARITY1+(index*300), str);

	str.Format("%s", pstPersonInfo->szPersonName);
	SetDlgItemText(IDC_RESULT_NAME+(index*300), str);

	if (1 == pstPersonInfo->bySex)
	{
		str.Format("%s", ConvertString("Male"));
	}
	else if (2 == pstPersonInfo->bySex)
	{
		str.Format("%s", ConvertString("Female"));
	}
	else
	{
		str.Format("%s", ConvertString("Unknown"));
	}
	SetDlgItemText(IDC_RESULT_SEX+(index*300), str);

	if (pstPersonInfo->wYear != 0  || pstPersonInfo->byMonth != 0 || pstPersonInfo->byDay != 0)
	{
		str.Format("%d", pstPersonInfo->wYear);
		SetDlgItemText(IDC_RESULT_YEAR+(index*300), str);

		str.Format("%d", pstPersonInfo->byMonth);
		SetDlgItemText(IDC_RESULT_MONTH+(index*300), str);

		str.Format("%d", pstPersonInfo->byDay);
		SetDlgItemText(IDC_RESULT_DAY+(index*300), str);
	}

	str.Format("%d", pstPersonInfo->byAge);
	SetDlgItemText(IDC_RESULT_AGE+(index*300), str);

	if (1 == pstPersonInfo->byIDType)
	{
		SetDlgItemText(IDC_RESULT_CARDTYPE+(index*300), ConvertString("IC"));
	}
	else if (2 == pstPersonInfo->byIDType)
	{
		SetDlgItemText(IDC_RESULT_CARDTYPE+(index*300), ConvertString("Passport"));
	}

	str.Format("%s", pstPersonInfo->szID);
	SetDlgItemText(IDC_RESULT_CARDID+(index*300), str);

	if (1 == pstPersonInfo->byGlasses)
	{
		str.Format("%s", ConvertString("Not Wear"));
	}
	else if (2 == pstPersonInfo->byGlasses)
	{
		str.Format("%s", ConvertString("Wear"));
	}
	else
	{
		str.Format("%s", ConvertString("Unknown"));
	}
	SetDlgItemText(IDC_RESULT_GLASSES+(index*300), str);

	if (EM_EYE_STATE_NODISTI == pstPersonInfo->emEye)
	{
		str.Format("%s", ConvertString("Unidentification"));
	}
	else if (EM_EYE_STATE_CLOSE == pstPersonInfo->emEye)
	{
		str.Format("%s", ConvertString("Close Eyes"));
	}
	else if (EM_EYE_STATE_OPEN == pstPersonInfo->emEye)
	{
		str.Format("%s", ConvertString("Open Eyes"));
	}
	else
	{
		str.Format("%s", ConvertString("Unknown"));
	}
	SetDlgItemText(IDC_RESULT_EYE+(index*300), str);

	if (EM_MOUTH_STATE_NODISTI == pstPersonInfo->emMouth)
	{
		str.Format("%s", ConvertString("Unidentification"));
	}
	else if (EM_MOUTH_STATE_CLOSE == pstPersonInfo->emMouth)
	{
		str.Format("%s", ConvertString("Close Mouth"));
	}
	else if (EM_MOUTH_STATE_OPEN == pstPersonInfo->emMouth)
	{
		str.Format("%s", ConvertString("Open Mouth"));
	}
	else
	{
		str.Format("%s", ConvertString("Unknown"));
	}
	SetDlgItemText(IDC_RESULT_Mouth+(index*300), str);

	if (EM_MASK_STATE_NODISTI == pstPersonInfo->emMask)
	{
		str.Format("%s", ConvertString("Unidentification"));
	}
	else if (EM_MASK_STATE_NOMASK == pstPersonInfo->emMask)
	{
		str.Format("%s", ConvertString("Not Wear"));
	}
	else if (EM_MASK_STATE_WEAR == pstPersonInfo->emMask)
	{
		str.Format("%s", ConvertString("Wear"));
	}
	else
	{
		str.Format("%s", ConvertString("Unknown"));
	}
	SetDlgItemText(IDC_RESULT_Mask+(index*300), str);

	if (EM_BEARD_STATE_NODISTI == pstPersonInfo->emBeard)
	{
		str.Format("%s", ConvertString("Unidentification"));
	}
	else if (EM_BEARD_STATE_NOBEARD == pstPersonInfo->emBeard)
	{
		str.Format("%s", ConvertString("No Beard"));
	}
	else if (EM_BEARD_STATE_HAVEBEARD == pstPersonInfo->emBeard)
	{
		str.Format("%s", ConvertString("Have Beard"));
	}
	else
	{
		str.Format("%s", ConvertString("Unknown"));
	}
	SetDlgItemText(IDC_RESULT_Beard+(index*300), str);

	str.Format("%d", pstPersonInfo->nAttractive);
	SetDlgItemText(IDC_RESULT_Attractive+(index*300), str);
}

void CSearchByPic::AddCandidateInfoToList(CANDIDATE_INFOEX * pstAddInfo)
{
	if (NULL == pstAddInfo)
	{
		return;
	}

	int i = 0;
	BOOL bHasAdd = FALSE;

	FaceLock locker(m_Mutex);
	std::list<CANDIDATE_INFOEX*>::iterator it = m_lstCandidateInfo.begin();

	for (; it != m_lstCandidateInfo.end(); ++it)
	{
		CANDIDATE_INFOEX *pstCandidateInfo = *it;
		if (pstCandidateInfo)
		{
			if (pstAddInfo->bySimilarity > pstCandidateInfo->bySimilarity)
			{

				m_lstCandidateInfo.insert(it, pstAddInfo);
				bHasAdd = TRUE;
				break;
			}
		}
		i++;
	}
	if (FALSE == bHasAdd)
	{
		m_lstCandidateInfo.push_back(pstAddInfo);
	}

	locker.UnLock();
}

void CSearchByPic::DoFindTargetRecognitionFunc()
{
	int i = 0;
	int nToken = 0;
	int nTotalCount = 0;
	BOOL bRet = FALSE;
	CANDIDATE_INFOEX *pstCandidateInfo = NULL;
	std::list<CANDIDATE_INFOEX*>::iterator it;
	DH_IN_DOWNLOAD_REMOTE_FILE stuInFile = {sizeof(stuInFile)};
	DH_OUT_DOWNLOAD_REMOTE_FILE stuOutFile = {sizeof(stuOutFile)};
	NET_IN_DOFIND_FACERECONGNITION stuInDoFind = {sizeof(stuInDoFind)};
	NET_OUT_DOFIND_FACERECONGNITION stuOutDoFind = {sizeof(stuOutDoFind)};
	const int nMaxFileLength = 256;

	for (int i=0; i<2; ++i)
	{
		stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].nFilePathLen = nMaxFileLength;
		stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath = new char[nMaxFileLength];
		if (NULL ==  stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath)
		{
			goto ERROR_OUT;
		}
		memset(stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath, 0, nMaxFileLength);
	}

	stuOutDoFind.bUseCandidatesEx = TRUE;

	if (m_nProgress == 100) // when device query process is 100 begin to Dofind
	{
		StopAttchHandle();
		int nBegin = 0;
		stuInDoFind.nCount = 20; // 20 items per query
		stuInDoFind.lFindHandle = m_lFindHandle;
		stuInDoFind.emDataType = EM_NEEDED_PIC_TYPE_HTTP_URL;
		int nFindCount = 0;
		while (1)
		{
			//stop search button
			DWORD dRet = WaitForSingleObject(m_lEvent, 500);
			if (dRet == WAIT_OBJECT_0)
			{
				ResetEvent(m_lEvent);
				goto ERROR_OUT;

			}

			stuInDoFind.nBeginNum = nBegin;
			if (m_lFindHandle)
			{
				bRet = CLIENT_DoFindFaceRecognition(&stuInDoFind, &stuOutDoFind, DEFAULT_WAIT_TIME);
				if (!bRet)
				{
					MessageBox(ConvertString("Failed to do find target recognition!"), "");
					goto ERROR_OUT;
				}
			}

			for (i = 0; i < stuOutDoFind.nCadidateExNum; i++)
			{
				pstCandidateInfo = new CANDIDATE_INFOEX;
				if (NULL == pstCandidateInfo)
				{
					AfxMessageBox(ConvertString("Memory error"));
					goto ERROR_OUT;
				}
				memset(pstCandidateInfo, 0, sizeof(CANDIDATE_INFOEX));
				memcpy(pstCandidateInfo, &(stuOutDoFind.stuCandidatesEx[i]), sizeof(CANDIDATE_INFOEX));
				pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath = new char[MAX_FILE_PATH_LEN];
				if (pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath &&
					stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath)
				{
					memset(pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath, 0, MAX_FILE_PATH_LEN);
					strncpy(pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath, stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath, strlen(stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath)+1);
				}
				AddCandidateInfoToList(pstCandidateInfo);
			}

			nFindCount += stuOutDoFind.nCadidateExNum;

			m_ctlSearchPro.SetRange32(0,200);
			m_nProgress += (((float)nFindCount/(float)m_nTotalCount)*100);

			nBegin += 20;

			if (stuOutDoFind.nCadidateExNum < 20) // stop query
			{
				StopFindHandle();
				break;
			}
		}

		DoUpdateCount();

		FaceLock locker(m_Mutex);
		it = m_lstCandidateInfo.begin();
		for (i = 0; (it != m_lstCandidateInfo.end()) && (i < 2); ++it, i++)
		{
			CANDIDATE_INFOEX *pstCandidateInfo = *it;
			if (pstCandidateInfo)
			{
				FACERECOGNITION_PERSON_INFOEX *pstPersonInfo = &(pstCandidateInfo->stPersonInfo);
				int nbySimilarity = pstCandidateInfo->bySimilarity;
				ShowPersonInfoSearchByPic(pstPersonInfo, i, nbySimilarity);
				if (strlen(pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath) > 0)
				{
					DH_IN_DOWNLOAD_REMOTE_FILE stuInFile = {sizeof(stuInFile)};
					DH_OUT_DOWNLOAD_REMOTE_FILE stuOutFile = {sizeof(stuOutFile)};

					char szSrcPath[FACE_MAX_PATH_LEN] = {0};
					char szFileDst[FACE_MAX_PATH_LEN] = {0};

					strncpy(szSrcPath, pstCandidateInfo->stPersonInfo.szFacePicInfo[0].pszFilePath, sizeof(szSrcPath)-1);
					_snprintf(szFileDst, sizeof(szFileDst) - 1,"Face\\RemoteFace\\RemoteFace%d.jpg", i);

					CString filepath(m_pSoftPath);
					CString filename(szFileDst);
					CString strFile = filepath + filename;
					char *pFileDst = strFile.GetBuffer(256);

					stuInFile.pszFileName = szSrcPath;
					stuInFile.pszFileDst = pFileDst;

					bRet = CLIENT_DownloadRemoteFile(m_lLoginId, &stuInFile, &stuOutFile, DEFAULT_WAIT_TIME);
					if (!bRet)
					{
						MessageBox(ConvertString("Download remote face picture failed!"), "");
						goto ERROR_OUT;
					}
					if (i == 0)
					{
						m_pResultPic1.SetImageFile(pFileDst);
					}
					else if (i == 1)
					{
						m_pResultPic2.SetImageFile(pFileDst);
					}

					strFile.ReleaseBuffer();
				}
			}
		}
		locker.UnLock();
	}

ERROR_OUT:
	for (int i=0; i<2; ++i)
	{
		if (stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath)
		{
			delete[] stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath;
			stuOutDoFind.stuCandidatesEx[i].stPersonInfo.szFacePicInfo[0].pszFilePath = NULL;
		}
	}


	ClearCandidateInfo();
	StopAttchHandle();
	StopFindHandle();
	GetDlgItem(IDC_BUTTON_SEARCHPIC_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_SEARCH_PIC)->EnableWindow(TRUE);
	GetDlgItem(IDC_OPEN_PIC)->EnableWindow(TRUE);
	return;
}

void CSearchByPic::OnSearchPic() 
{
	StopFindHandle();
	StopAttchHandle();
	ClearShowPersonInfoSearchByPic();
	ClearCandidateInfo();
	GetDlgItem(IDC_OPEN_PIC)->EnableWindow(FALSE);
	if (strlen(m_szFilePath) <= 0)
	{
		return;
	}

	BOOL bRet = FALSE;
	LLONG lAttachHandle = 0;
	int nToken = 0;

	NET_THREAD_PARAM stuThreadParam = {0};

	NET_IN_FACE_FIND_STATE stuInFindState = {sizeof(stuInFindState)};
	NET_OUT_FACE_FIND_STATE stuOutFindState = {sizeof(stuOutFindState)};

	NET_IN_STARTFIND_FACERECONGNITION stuInParam = {sizeof(stuInParam)};
	NET_OUT_STARTFIND_FACERECONGNITION stuOutParam = {sizeof(stuOutParam)};
	stuInParam.stFilterInfo.dwSize = sizeof(stuInParam.stFilterInfo);
	stuInParam.stMatchOptions.dwSize = sizeof(stuInParam.stMatchOptions);

	stuInParam.bPersonExEnable = TRUE;

	int id = m_comboChannel.GetCurSel();
	if(CB_ERR != id)
	{
		stuInParam.nChannelID = m_comboChannel.GetItemData(id);
	}

	CComboBox *pSimilarity = (CComboBox*)GetDlgItem(IDC_SEARCH_SIMILARITY_COMBOX);
	int nSimlilarity = pSimilarity->GetCurSel() + 1;

	stuInParam.stMatchOptions.nSimilarity = nSimlilarity;

	CDateTimeCtrl *pCDataStart = (CDateTimeCtrl*)GetDlgItem(IDC_STARTDATE_SEARCHPIC);
	CDateTimeCtrl *pCDataEnd = (CDateTimeCtrl*)GetDlgItem(IDC_ENDDATE_SEARCHPIC);

	SYSTEMTIME DataStart = {0};
	SYSTEMTIME DataEnd = {0};

	pCDataStart->GetTime(&DataStart);
	pCDataEnd->GetTime(&DataEnd);

	stuInParam.stFilterInfo.stStartTime.dwYear = DataStart.wYear;
	stuInParam.stFilterInfo.stStartTime.dwMonth = DataStart.wMonth;
	stuInParam.stFilterInfo.stStartTime.dwDay = DataStart.wDay;
	stuInParam.stFilterInfo.stStartTime.dwHour = 0;
	stuInParam.stFilterInfo.stStartTime.dwMinute = 0;
	stuInParam.stFilterInfo.stStartTime.dwSecond = 0;

	stuInParam.stFilterInfo.stEndTime.dwYear = DataEnd.wYear;
	stuInParam.stFilterInfo.stEndTime.dwMonth = DataEnd.wMonth;
	stuInParam.stFilterInfo.stEndTime.dwDay = DataEnd.wDay;
	stuInParam.stFilterInfo.stEndTime.dwHour = 23;
	stuInParam.stFilterInfo.stEndTime.dwMinute = 59;
	stuInParam.stFilterInfo.stEndTime.dwSecond = 59;

	BOOL bNeedUpdateButton = TRUE;

	FILE *fPic = fopen(m_szFilePath, "rb");
	if (fPic == NULL)
	{
		return;
	}
	
	fseek(fPic, 0, SEEK_END);
	int nLength = ftell(fPic);
	if (nLength <= 0)
	{
		bNeedUpdateButton = FALSE;
		goto FREE_RETURN;
	}
	rewind(fPic);

	stuInParam.nBufferLen = nLength;

	stuInParam.pBuffer = new char[stuInParam.nBufferLen];
	if (NULL == stuInParam.pBuffer)
	{
		MessageBox(ConvertString("Memory error"), "");
		bNeedUpdateButton = FALSE;
		goto FREE_RETURN;
	}

	memset(stuInParam.pBuffer, 0, stuInParam.nBufferLen);
	int nReadLen = fread(stuInParam.pBuffer, 1, stuInParam.nBufferLen, fPic);
	fclose(fPic);
	fPic = NULL;
	if (nReadLen <= 0)
	{
		bNeedUpdateButton = FALSE;
		goto FREE_RETURN;
	}

	stuInParam.stPersonInfoEx.wFacePicNum = 1;
	stuInParam.stPersonInfoEx.szFacePicInfo[0].dwOffSet = 0;
	stuInParam.stPersonInfoEx.szFacePicInfo[0].dwFileLenth = nLength;

	bRet = CLIENT_StartFindFaceRecognition(m_lLoginId, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);

	if (!bRet)
	{
		MessageBox(ConvertString("Failed to start search target recognition!"), "");
		bNeedUpdateButton = FALSE;
		goto FREE_RETURN;
	}

	m_lFindHandle = stuOutParam.lFindHandle;

	if (-1 == stuOutParam.nTotalCount)
	{
		stuInFindState.nTokenNum = 1;
		nToken = stuOutParam.nToken;
		stuInFindState.nTokens = &nToken;
		stuInFindState.cbFaceFindState = FaceFindState;
		stuInFindState.dwUser = (LDWORD)this;
		m_nProgress = 0;

		m_lAttachHandle = CLIENT_AttachFaceFindState(m_lLoginId, &stuInFindState, &stuOutFindState, DEFAULT_WAIT_TIME);

		if (0 == m_lAttachHandle)
		{
			MessageBox(ConvertString("Query By Picture fail"), "");
			StopFindHandle();
			bNeedUpdateButton = FALSE;
			goto FREE_RETURN;
		}
	}
	else
	{
		MessageBox(ConvertString("Failed to start search target recognition!"), "");
	}

FREE_RETURN:
	if (stuInParam.pBuffer)
	{
		delete[] stuInParam.pBuffer;
		stuInParam.pBuffer = NULL;
	}

	if (fPic)
	{
		fclose(fPic);
		fPic = NULL;
	}
	if (bNeedUpdateButton)
	{
		GetDlgItem(IDC_SEARCH_PIC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SEARCHPIC_STOP)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_OPEN_PIC)->EnableWindow(TRUE);
	}
}

void CSearchByPic::OnBnClickedButtonSearchpicStop()
{
	m_nProgress = 0;
	StopFindHandle();
	StopAttchHandle();

	SetEvent(m_lEvent);
	GetDlgItem(IDC_BUTTON_SEARCHPIC_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_SEARCH_PIC)->EnableWindow(TRUE);
	GetDlgItem(IDC_OPEN_PIC)->EnableWindow(TRUE);
}

LRESULT CSearchByPic::OnDeviceAttch(WPARAM wParam, LPARAM lParam)
{	
	if (wParam == NULL)
	{
		return 0;
	}
	CSearchByPic *dlg = (CSearchByPic *)wParam;
	dlg->DoFindTargetRecognitionFunc();

	return 0;
}

void CSearchByPic::DoUpdateCount()
{
	CString strCount = "";
	strCount.Format("%d", m_nTotalCount);
	if (m_nTotalCount == 0)
	{
		m_nProgress = 200;
	}
	SetDlgItemText(IDC_EDIT_SEARCHBUPIC_COUNT, strCount);
}

BOOL CSearchByPic::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CSearchByPic::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == m_nTimer)
	{
		m_ctlSearchPro.SetRange32(0, 200);
		m_ctlSearchPro.SetPos((int)(m_nProgress) );
	}

	CDialog::OnTimer(nIDEvent);
}

void CSearchByPic::StopAttchHandle()
{
	if (m_lAttachHandle)
	{
		CLIENT_DetachFaceFindState(m_lAttachHandle);
		m_lAttachHandle = 0;
	}
}

void CSearchByPic::StopFindHandle()
{
	if (m_lFindHandle)
	{
		CLIENT_StopFindFaceRecognition(m_lFindHandle);
		m_lFindHandle = 0;
	}	
}