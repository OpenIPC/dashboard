// QueryDownLoad.cpp : implementation file
//

#include "StdAfx.h"
#include "TargetRecognition.h"
#include "QueryDownLoad.h"

#define QUERY_FILE_NUM	50
#define	PATH_MAX_LEN	1024
#define FILEDOWNLOAD_TIMER_MSEC 62.5
#define WM_DOWNLOAD_CALLBACK	(WM_USER + 301)

struct	strCallBackInfo
{
	LLONG lHandle;
	DWORD dwTotalSize;
	DWORD dwDownLoadSize;
};

void CALLBACK DownLoadPosCallBack(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, LDWORD dwUser)
{
	CQueryDownLoad *dlg = (CQueryDownLoad*)dwUser;
	if (dlg == NULL)
	{
		return;
	}
	HWND hwnd = dlg->GetSafeHwnd();

	strCallBackInfo *pInfo = new strCallBackInfo;
	pInfo->lHandle = lPlayHandle;
	pInfo->dwTotalSize = dwTotalSize;
	pInfo->dwDownLoadSize = dwDownLoadSize;

	::PostMessage(hwnd, WM_DOWNLOAD_CALLBACK, WPARAM(pInfo) , 0);
}

IMPLEMENT_DYNAMIC(CQueryDownLoad, CDialog)

CQueryDownLoad::CQueryDownLoad(int nChannelNum, LLONG lLoginid, char *pSoftPath, CWnd* pParent /*=NULL*/)
: CDialog(CQueryDownLoad::IDD, pParent)
, m_nChannelNum(nChannelNum)
, m_lLogin(lLoginid)
, m_nPage(0)
, m_lFindHandle(0)
, m_emQuryType(QUERY_VIDEO)
, m_playBackHandle(0)
, m_nTimer(0)
, m_dwPlayBackTotalSize(0)
, m_dwPlayBackCurValue(0)
, m_dwDownLoadCurValue(0)
, m_dwDownLoadTotalSize(0)
, m_lDownLoadHandle(0)
, m_nInfoCount(0)
, m_pSoftPath(NULL)
{
	if (pSoftPath)
	{
		m_pSoftPath = pSoftPath;
	}

	memset(&m_DateStart, 0, sizeof(m_DateStart));
	memset(&m_DateEnd, 0, sizeof(m_DateEnd));
	m_emSex = EM_DEV_EVENT_FACEDETECT_SEX_TYPE_UNKNOWN;
	m_emGlasses = EM_FACEDETECT_GLASSES_UNKNOWN;
	m_emMask = EM_MASK_STATE_UNKNOWN;
	m_emBeard = EM_BEARD_STATE_UNKNOWN;
	memset(&m_nAge, 0, sizeof(m_nAge) );
	memset(&m_nSimilaryRange, 0, sizeof(m_nSimilaryRange));
	m_nQueryType = 0;
	m_nAttributeFlag = 0;
	m_emEmotion = EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_UNKNOWN;
}

CQueryDownLoad::~CQueryDownLoad()
{
	ClearQueryResultInfoVector();
	CloseFindHandle();
	ClosePlayBackHandle();
	CloseDownLoadHandle();
}

void CQueryDownLoad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_QUERY_RESULT, m_lcQueryResult);
	DDX_Control(pDX, IDC_COMBO_CHANNEL_QUERY, m_comboQueryChannel);
	DDX_Control(pDX, IDC_COMBO_ALARM_TYPE, m_comboEventType);
	DDX_Control(pDX, IDC_COMBO_FILE_TYPE, m_comboFileType);
	DDX_Control(pDX, IDC_PROGRESS_QUERY2, m_PlayBackProcess);
	DDX_Control(pDX, IDC_PROGRESS_QUERY, m_ProcessDownLoad);
	DDX_Control(pDX, IDC_COMBO_QUERY_SEX, m_comboSex);
	DDX_Control(pDX, IDC_COMBO_QUERY_GLASSES, m_comboGlasses);
	DDX_Control(pDX, IDC_COMBO_QUERY_MASK, m_comboMask);
	DDX_Control(pDX, IDC_COMBO_QUERY_BREAD, m_comboBeard);
	DDX_Control(pDX, IDC_CHECK_ATTRIBUTE_ENABLE, m_AttributeEnable);
	DDX_Control(pDX, IDC_COMBO_QUERY_EMOTION, m_comboEmotion);
}


BEGIN_MESSAGE_MAP(CQueryDownLoad, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_START, &CQueryDownLoad::OnBnClickedButtonQueryStart)
	ON_CBN_SELCHANGE(IDC_COMBO_FILE_TYPE, &CQueryDownLoad::OnCbnSelchangeComboFileType)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_PREPAGE, &CQueryDownLoad::OnBnClickedButtonQueryPrepage)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_DOWNLOAD, &CQueryDownLoad::OnBnClickedButtonQueryDownload)
	ON_BN_CLICKED(IDC_BUTTON_QUERY_NEXTPAGE, &CQueryDownLoad::OnBnClickedButtonQueryNextpage)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_QUERY_RESULT, &CQueryDownLoad::OnLvnItemchangedListQueryResult)

	ON_MESSAGE(WM_DOWNLOAD_CALLBACK, &CQueryDownLoad::OnDoDownLoad)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_ATTRIBUTE_ENABLE, &CQueryDownLoad::OnBnClickedCheckAttributeEnable)
	ON_CBN_SELCHANGE(IDC_COMBO_ALARM_TYPE, &CQueryDownLoad::OnCbnSelchangeComboAlarmType)
END_MESSAGE_MAP()

BOOL CQueryDownLoad::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitControl();
	
	m_comboEventType.ResetContent();
	m_comboEventType.AddString(ConvertString("FaceEvent"));
	m_comboEventType.SetCurSel(0);

	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);

	m_PlayBackProcess.SetRange32(0,(int)(m_dwPlayBackTotalSize));
	m_PlayBackProcess.SetPos(0);

	m_ProcessDownLoad.SetRange32(0, (int)m_dwDownLoadTotalSize/1024);
	m_ProcessDownLoad.SetPos(0);
	if (0 == m_nTimer)
	{
		m_nTimer = SetTimer(1, FILEDOWNLOAD_TIMER_MSEC, NULL);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CQueryDownLoad::InitControl()
{
	m_lcQueryResult.SetExtendedStyle(m_lcQueryResult.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lcQueryResult.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,50,-1);
	m_lcQueryResult.InsertColumn(1,ConvertString("Channel"),LVCFMT_LEFT,50,0);
	m_lcQueryResult.InsertColumn(2,ConvertString("Start Time"),LVCFMT_LEFT,150,1);
	m_lcQueryResult.InsertColumn(3, ConvertString("End Time"), LVCFMT_LEFT,150,2);

	m_comboFileType.ResetContent();
	m_comboFileType.AddString(ConvertString("Video"));
	m_comboFileType.AddString(ConvertString("Picture"));
	m_comboFileType.SetCurSel(0);

	m_comboQueryChannel.ResetContent();
	m_comboQueryChannel.AddString(ConvertString("All"));
	for(int i=0;i<m_nChannelNum;i++)
	{
		CString str;
		str.Format("%d",i + 1);
		m_comboQueryChannel.AddString(str);
	}
	m_comboQueryChannel.SetCurSel(0);

	m_comboGlasses.ResetContent();
	m_comboGlasses.AddString(ConvertString("Unknown"));
	m_comboGlasses.AddString(ConvertString("Not Wear"));
	m_comboGlasses.AddString(ConvertString("Wear"));
	m_comboGlasses.SetItemData(0, (DWORD_PTR)EM_FACEDETECT_GLASSES_UNKNOWN);
	m_comboGlasses.SetItemData(1, (DWORD_PTR)EM_FACEDETECT_WITHOUT_GLASSES);
	m_comboGlasses.SetItemData(2, (DWORD_PTR)EM_FACEDETECT_WITH_GLASSES);
	m_comboGlasses.SetCurSel(0);

	m_comboMask.ResetContent();
	m_comboMask.AddString(ConvertString("Unknown"));
	m_comboMask.AddString(ConvertString("Not Wear"));
	m_comboMask.AddString(ConvertString("Wear"));
	m_comboMask.SetItemData(0, (DWORD_PTR)EM_MASK_STATE_UNKNOWN);
	m_comboMask.SetItemData(1, (DWORD_PTR)EM_MASK_STATE_NOMASK);
	m_comboMask.SetItemData(2, (DWORD_PTR)EM_MASK_STATE_WEAR);
	m_comboMask.SetCurSel(0);

	m_comboSex.ResetContent();
	m_comboSex.AddString(ConvertString("Unknown"));
	m_comboSex.AddString(ConvertString("Male"));
	m_comboSex.AddString(ConvertString("Female"));
	m_comboSex.SetItemData(0, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_SEX_TYPE_UNKNOWN);
	m_comboSex.SetItemData(1, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_SEX_TYPE_MAN);
	m_comboSex.SetItemData(2, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_SEX_TYPE_WOMAN);
	m_comboSex.SetCurSel(0);

	m_comboBeard.ResetContent();
	m_comboBeard.AddString(ConvertString("Unknown"));
	m_comboBeard.AddString(ConvertString("No Beard"));
	m_comboBeard.AddString(ConvertString("Have Beard"));
	m_comboBeard.SetItemData(0, (DWORD_PTR)EM_BEARD_STATE_UNKNOWN);
	m_comboBeard.SetItemData(1, (DWORD_PTR)EM_BEARD_STATE_NOBEARD);
	m_comboBeard.SetItemData(2, (DWORD_PTR)EM_BEARD_STATE_HAVEBEARD);
	m_comboBeard.SetCurSel(0);

	m_comboEmotion.ResetContent();
	m_comboEmotion.AddString(ConvertString("Unknown"));
	m_comboEmotion.AddString(ConvertString("Wear glasses"));
	m_comboEmotion.AddString(ConvertString("Smile"));
	m_comboEmotion.AddString(ConvertString("Anger"));
	m_comboEmotion.AddString(ConvertString("Sadness"));
	m_comboEmotion.AddString(ConvertString("Disgust"));
	m_comboEmotion.AddString(ConvertString("Fear"));
	m_comboEmotion.AddString(ConvertString("Surprise"));
	m_comboEmotion.AddString(ConvertString("Neutral"));
	m_comboEmotion.AddString(ConvertString("Laugh"));
	m_comboEmotion.AddString(ConvertString("No glasses"));
	m_comboEmotion.AddString(ConvertString("Happy"));
	m_comboEmotion.AddString(ConvertString("Confused"));
	m_comboEmotion.AddString(ConvertString("Scream"));
	m_comboEmotion.AddString(ConvertString("Wear Sunglasses"));

	m_comboEmotion.SetItemData(0, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_UNKNOWN);
	m_comboEmotion.SetItemData(1, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_WEAR_GLASSES);
	m_comboEmotion.SetItemData(2, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_SMILE);
	m_comboEmotion.SetItemData(3, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_ANGER);
	m_comboEmotion.SetItemData(4, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_SADNESS);
	m_comboEmotion.SetItemData(5, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_DISGUST);
	m_comboEmotion.SetItemData(6, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_FEAR);
	m_comboEmotion.SetItemData(7, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_SURPRISE);
	m_comboEmotion.SetItemData(8, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_NEUTRAL);
	m_comboEmotion.SetItemData(9, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_LAUGH);
	m_comboEmotion.SetItemData(10, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_NOGLASSES);
	m_comboEmotion.SetItemData(11, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_HAPPY);
	m_comboEmotion.SetItemData(12, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_CONFUSED);
	m_comboEmotion.SetItemData(13, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_SCREAM);
	m_comboEmotion.SetItemData(14, (DWORD_PTR)EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE_WEAR_SUNGLASSES);
	
	m_comboEmotion.SetCurSel(0);
}

void CQueryDownLoad::OnCbnSelchangeComboFileType()
{
	int nSel = m_comboFileType.GetCurSel();
	if (m_comboFileType.GetCurSel() == 0)
	{
		m_comboEventType.ResetContent();
		m_comboEventType.AddString(ConvertString("FaceEvent"));
		m_comboEventType.SetCurSel(0);

		GetDlgItem(IDC_CHECK_ATTRIBUTE_ENABLE)->EnableWindow(FALSE);

	}
	else if (m_comboFileType.GetCurSel() == 1)
	{
		m_comboEventType.ResetContent();

		m_comboEventType.AddString(ConvertString("FaceDetection"));
		m_comboEventType.AddString(ConvertString("TargetRecognition"));

		m_comboEventType.SetCurSel(0);

		GetDlgItem(IDC_CHECK_ATTRIBUTE_ENABLE)->EnableWindow(TRUE);
	}
}

void CQueryDownLoad::ReceivePlayPos(LLONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize)
{
	if (lPlayHandle == m_playBackHandle)
	{
		if (dwDownLoadSize == -1)
		{
			ClosePlayBackHandle();
			m_dwPlayBackCurValue = 100;
			m_dwPlayBackTotalSize = 100;
			ClosePlayBackHandle();
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_REALPLAY_PIC));
			return;
		}
		m_dwPlayBackTotalSize = dwTotalSize;
		m_dwPlayBackCurValue = dwDownLoadSize;
	}
	else if (lPlayHandle == m_lDownLoadHandle)
	{
		if (dwDownLoadSize == -1)
		{
			m_dwDownLoadTotalSize = 100;
			m_dwDownLoadCurValue = 100;
			CloseDownLoadHandle();
			MessageBox(ConvertString("Download file success!"), ConvertString("Prompt"));
			GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
			return;
		}
		m_dwDownLoadTotalSize = dwTotalSize;
		m_dwDownLoadCurValue = dwDownLoadSize;
	}
}

emQueryType CQueryDownLoad::getCurrentQueryType()
{
	emQueryType emRet = QUERY_VIDEO;

	if (m_comboFileType.GetCurSel() == 0)
	{
		emRet = QUERY_VIDEO;
	}

	if (m_comboFileType.GetCurSel() == 1 && m_comboEventType.GetCurSel() == 0)
	{
		emRet = QUERY_PICFD;
	}

	if (m_comboFileType.GetCurSel() == 1 && m_comboEventType.GetCurSel() == 1)
	{
		emRet = QUERY_PICFR;
	}

	return emRet;
}

void CQueryDownLoad::OnBnClickedButtonQueryStart()
{
	if (m_lLogin == NULL)
	{
		MessageBox(ConvertString("Invalid Login Handle"), ConvertString("Prompt"));
		return;
	}

	CloseFindHandle();
	ClearQueryResultInfoVector();
	m_lcQueryResult.DeleteAllItems();
	m_nInfoCount = 0;
	m_nPage = 0;

	int tcheck = m_AttributeEnable.GetCheck();
	if (tcheck == BST_CHECKED)
	{
		m_nQueryType = 1;
	}
	else
	{
		m_nQueryType = 0;
	}

	GetTimeFromUI();
	BOOL bRet = GetAttributeFromUI();
	if (FALSE == bRet)
	{
		GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);
		return ;
	}
	BOOL bFind = FALSE;

	m_emQuryType = getCurrentQueryType();
	//query picture by event
	if (m_nQueryType == 0)
	{
		if (m_emQuryType == QUERY_VIDEO)
		{
			bFind = FindFaceVideo();
		}
		else if(m_emQuryType == QUERY_PICFD)
		{
			bFind = FindFaceEventPicture(EVENT_IVS_FACEDETECT);
		}
		else if(m_emQuryType == QUERY_PICFR)
		{
			bFind = FindFaceEventPicture(EVENT_IVS_FACERECOGNITION);
		}
	}

	//query picture by attribute
	else
	{
		if(m_emQuryType == QUERY_PICFD)
		{
			bFind = FindFaceDetectionPicture();
		}
		else if(m_emQuryType == QUERY_PICFR)
		{
			bFind = FindTargetRecognitionPicture();
		}
	}

	if (bFind == TRUE)
	{
		int nSearchCount = DoFindNextPageEx(QUERY_FILE_NUM + 1);
		if (nSearchCount == (QUERY_FILE_NUM + 1) )
		{
			GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
		}
		if (nSearchCount > 0)
		{
			GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
		}
	}

	GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);
}

BOOL CQueryDownLoad::FindFaceVideo()
{
	NET_IN_MEDIA_QUERY_FILE strMidiaQueryParam;
	memset(&strMidiaQueryParam, 0, sizeof(strMidiaQueryParam));

	strMidiaQueryParam.dwSize = sizeof(strMidiaQueryParam);
	strMidiaQueryParam.nMediaType = 2;					//set video search type
	strMidiaQueryParam.nChannelID = m_comboQueryChannel.GetCurSel() - 1;	//-1 means all channel
	strMidiaQueryParam.nEventLists[0] = EVENT_IVS_FACERECOGNITION;
	strMidiaQueryParam.nEventLists[1] = EVENT_IVS_FACEDETECT;
	strMidiaQueryParam.nEventCount = 2;					//number of events include  FaceDetection and TargetRecognition
	strMidiaQueryParam.emFalgLists[0]= FLAG_TYPE_EVENT;	
	strMidiaQueryParam.nFalgCount = 1;					

	//set the begin time
	strMidiaQueryParam.stuStartTime.dwYear = m_DateStart.dwYear;
	strMidiaQueryParam.stuStartTime.dwMonth = m_DateStart.dwMonth;
	strMidiaQueryParam.stuStartTime.dwDay = m_DateStart.dwDay;
	strMidiaQueryParam.stuStartTime.dwHour = m_DateStart.dwHour;
	strMidiaQueryParam.stuStartTime.dwMinute = m_DateStart.dwMinute;
	strMidiaQueryParam.stuStartTime.dwSecond = m_DateStart.dwSecond;

	//set the end time
	strMidiaQueryParam.stuEndTime.dwYear = m_DateEnd.dwYear;
	strMidiaQueryParam.stuEndTime.dwMonth = m_DateEnd.dwMonth;
	strMidiaQueryParam.stuEndTime.dwDay = m_DateEnd.dwDay;
	strMidiaQueryParam.stuEndTime.dwHour = m_DateEnd.dwHour;
	strMidiaQueryParam.stuEndTime.dwMinute = m_DateEnd.dwMinute;
	strMidiaQueryParam.stuEndTime.dwSecond = m_DateEnd.dwSecond;

	m_lFindHandle = CLIENT_FindFileEx(m_lLogin, DH_FILE_QUERY_FILE, &strMidiaQueryParam, NULL,DEFAULT_WAIT_TIME);
	if (m_lFindHandle == 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CQueryDownLoad::FindFaceDetectionPicture()
{
	MEDIAFILE_FACE_DETECTION_PARAM stuMediaFaceDetectionParam;
	memset(&stuMediaFaceDetectionParam, 0, sizeof(stuMediaFaceDetectionParam));

	stuMediaFaceDetectionParam.dwSize = sizeof(stuMediaFaceDetectionParam);
	stuMediaFaceDetectionParam.stuDetail.dwSize = sizeof(MEDIAFILE_FACE_DETECTION_DETAIL_PARAM);
	stuMediaFaceDetectionParam.nChannelID = m_comboQueryChannel.GetCurSel() - 1;	//-1 means all channel

	//set the begin time
	stuMediaFaceDetectionParam.stuStartTime.dwYear = m_DateStart.dwYear;
	stuMediaFaceDetectionParam.stuStartTime.dwMonth = m_DateStart.dwMonth;
	stuMediaFaceDetectionParam.stuStartTime.dwDay = m_DateStart.dwDay;
	stuMediaFaceDetectionParam.stuStartTime.dwHour = m_DateStart.dwHour;
	stuMediaFaceDetectionParam.stuStartTime.dwMinute = m_DateStart.dwMinute;
	stuMediaFaceDetectionParam.stuStartTime.dwSecond = m_DateStart.dwSecond;

	//set the end time
	stuMediaFaceDetectionParam.stuEndTime.dwYear = m_DateEnd.dwYear;
	stuMediaFaceDetectionParam.stuEndTime.dwMonth = m_DateEnd.dwMonth;
	stuMediaFaceDetectionParam.stuEndTime.dwDay = m_DateEnd.dwDay;
	stuMediaFaceDetectionParam.stuEndTime.dwHour = m_DateEnd.dwHour;
	stuMediaFaceDetectionParam.stuEndTime.dwMinute = m_DateEnd.dwMinute;
	stuMediaFaceDetectionParam.stuEndTime.dwSecond = m_DateEnd.dwSecond;

	stuMediaFaceDetectionParam.emPicType = NET_FACEPIC_TYPE_UNKOWN;
	stuMediaFaceDetectionParam.bDetailEnable = FALSE;
	stuMediaFaceDetectionParam.bAgeEnable = FALSE;

	stuMediaFaceDetectionParam.emSex = m_emSex;
	stuMediaFaceDetectionParam.emGlasses = m_emGlasses;
	stuMediaFaceDetectionParam.emBeard = m_emBeard;
	stuMediaFaceDetectionParam.emMask = m_emMask;
	stuMediaFaceDetectionParam.bAgeEnable = TRUE;
	stuMediaFaceDetectionParam.nAgeRange[0] = m_nAge[0];
	stuMediaFaceDetectionParam.nAgeRange[1] = m_nAge[1];
	stuMediaFaceDetectionParam.nEmotionValidNum = 1;
	stuMediaFaceDetectionParam.emEmotion[0] = m_emEmotion;

	m_lFindHandle = CLIENT_FindFileEx(m_lLogin, DH_FILE_QUERY_FACE_DETECTION, &stuMediaFaceDetectionParam, NULL,DEFAULT_WAIT_TIME);
	if (m_lFindHandle == 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CQueryDownLoad::FindTargetRecognitionPicture()
{
	MEDIAFILE_FACERECOGNITION_PARAM stuMediaTargetRecognitionParam;
	memset(&stuMediaTargetRecognitionParam, 0, sizeof(stuMediaTargetRecognitionParam));

	stuMediaTargetRecognitionParam.dwSize = sizeof(stuMediaTargetRecognitionParam);
	stuMediaTargetRecognitionParam.nChannelId = m_comboQueryChannel.GetCurSel() - 1;	//-1 means all channel
	stuMediaTargetRecognitionParam.nAlarmType = NET_FACERECOGNITION_ALARM_TYPE_ALL;

	//set the begin time
	stuMediaTargetRecognitionParam.stStartTime.dwYear = m_DateStart.dwYear;
	stuMediaTargetRecognitionParam.stStartTime.dwMonth = m_DateStart.dwMonth;
	stuMediaTargetRecognitionParam.stStartTime.dwDay = m_DateStart.dwDay;
	stuMediaTargetRecognitionParam.stStartTime.dwHour = m_DateStart.dwHour;
	stuMediaTargetRecognitionParam.stStartTime.dwMinute = m_DateStart.dwMinute;
	stuMediaTargetRecognitionParam.stStartTime.dwSecond = m_DateStart.dwSecond;

	//set the end time
	stuMediaTargetRecognitionParam.stEndTime.dwYear = m_DateEnd.dwYear;
	stuMediaTargetRecognitionParam.stEndTime.dwMonth = m_DateEnd.dwMonth;
	stuMediaTargetRecognitionParam.stEndTime.dwDay = m_DateEnd.dwDay;
	stuMediaTargetRecognitionParam.stEndTime.dwHour = m_DateEnd.dwHour;
	stuMediaTargetRecognitionParam.stEndTime.dwMinute = m_DateEnd.dwMinute;
	stuMediaTargetRecognitionParam.stEndTime.dwSecond = m_DateEnd.dwSecond;
	stuMediaTargetRecognitionParam.bSimilaryRangeEnable = TRUE;
	stuMediaTargetRecognitionParam.nSimilaryRange[0] = m_nSimilaryRange[0];
	stuMediaTargetRecognitionParam.nSimilaryRange[1] = m_nSimilaryRange[1];

	stuMediaTargetRecognitionParam.abPersonInfoEx = TRUE;
	FACERECOGNITION_PERSON_INFOEX	stPersonInfoEx;
	memset(&stPersonInfoEx, 0, sizeof(stPersonInfoEx) );
	if (m_emGlasses == EM_FACEDETECT_WITH_GLASSES)
	{
		stPersonInfoEx.byGlasses = 2;
	}
	else if (m_emGlasses == EM_FACEDETECT_WITHOUT_GLASSES)
	{
		stPersonInfoEx.byGlasses = 1;
	}
	else
	{
		stPersonInfoEx.byGlasses = 0;
	}
	stPersonInfoEx.emBeard = m_emBeard;
	stPersonInfoEx.emMask = m_emMask;
	stPersonInfoEx.bySex = (BYTE)m_emSex;
	stPersonInfoEx.bAgeEnable = TRUE;
	stPersonInfoEx.nAgeRange[0] = m_nAge[0];
	stPersonInfoEx.nAgeRange[1] = m_nAge[1];
	stPersonInfoEx.nEmotionValidNum = 1;
	stPersonInfoEx.emEmotions[0] = m_emEmotion;
	stuMediaTargetRecognitionParam.stPersonInfoEx = stPersonInfoEx;

	m_lFindHandle = CLIENT_FindFileEx(m_lLogin, DH_FILE_QUERY_FACE, &stuMediaTargetRecognitionParam, NULL,DEFAULT_WAIT_TIME);
	if (m_lFindHandle == 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CQueryDownLoad::FindFaceEventPicture(DWORD dwEventType)
{
	NET_IN_MEDIA_QUERY_FILE strMidiaQueryParam;
	memset(&strMidiaQueryParam, 0, sizeof(strMidiaQueryParam));

	strMidiaQueryParam.dwSize = sizeof(strMidiaQueryParam);
	strMidiaQueryParam.nMediaType = 1;					//set video search type
	strMidiaQueryParam.nChannelID = m_comboQueryChannel.GetCurSel() - 1;	//-1 means all channel
	strMidiaQueryParam.nEventLists[0] = dwEventType;
	strMidiaQueryParam.nEventCount = 1;					//number of events include  FaceDetection and TargetRecognition
	strMidiaQueryParam.emFalgLists[0]= FLAG_TYPE_EVENT;	
	strMidiaQueryParam.nFalgCount = 1;					

	//set the begin time
	strMidiaQueryParam.stuStartTime.dwYear = m_DateStart.dwYear;
	strMidiaQueryParam.stuStartTime.dwMonth = m_DateStart.dwMonth;
	strMidiaQueryParam.stuStartTime.dwDay = m_DateStart.dwDay;
	strMidiaQueryParam.stuStartTime.dwHour = m_DateStart.dwHour;
	strMidiaQueryParam.stuStartTime.dwMinute = m_DateStart.dwMinute;
	strMidiaQueryParam.stuStartTime.dwSecond = m_DateStart.dwSecond;

	//set the end time
	strMidiaQueryParam.stuEndTime.dwYear = m_DateEnd.dwYear;
	strMidiaQueryParam.stuEndTime.dwMonth = m_DateEnd.dwMonth;
	strMidiaQueryParam.stuEndTime.dwDay = m_DateEnd.dwDay;
	strMidiaQueryParam.stuEndTime.dwHour = m_DateEnd.dwHour;
	strMidiaQueryParam.stuEndTime.dwMinute = m_DateEnd.dwMinute;
	strMidiaQueryParam.stuEndTime.dwSecond = m_DateEnd.dwSecond;

	m_lFindHandle = CLIENT_FindFileEx(m_lLogin, DH_FILE_QUERY_FILE, &strMidiaQueryParam, NULL,DEFAULT_WAIT_TIME);
	if (m_lFindHandle == 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

int CQueryDownLoad::DoFindNextPageEx(int nSearchCount)
{
	int nRetCount = -1;
	if (m_nQueryType == 0)
	{
		nRetCount = FindNextFaceEventType(nSearchCount);
	}
	else
	{
		switch (m_emQuryType)
		{
		case QUERY_VIDEO:
			{
				nRetCount = FindNextFaceVideo(nSearchCount);
				break;
			}
		case QUERY_PICFD:
			{
				nRetCount = FindNextFaceDetectionPicture(nSearchCount);
				break;
			}
		case QUERY_PICFR:
			{
				nRetCount = FindNextTargetRecognitionPicure(nSearchCount);
				break;
			}
		default:
			break;
		}
	}
	
	if (nRetCount > 0)
	{
		int nUpdateInfoCount = nRetCount;
		if (nRetCount == (QUERY_FILE_NUM + 1))
		{
			nUpdateInfoCount = QUERY_FILE_NUM;
		}
		UpdateLcQueryList(nUpdateInfoCount);
	}
	return nRetCount;
}

int CQueryDownLoad::FindNextFaceEventType(int nSearchCount)
{
	int nRetCount = -1;
	BOOL bRet = FALSE;
	NET_OUT_MEDIA_QUERY_FILE *pMediaFileInfo = NEW NET_OUT_MEDIA_QUERY_FILE[nSearchCount];

	if (pMediaFileInfo == NULL)
	{
		return FALSE;
	}

	memset(pMediaFileInfo, 0, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE));

	for (int i = 0; i < nSearchCount; i++)
	{
		pMediaFileInfo[i].dwSize = sizeof(NET_OUT_MEDIA_QUERY_FILE);
	}

	nRetCount = CLIENT_FindNextFileEx(m_lFindHandle, nSearchCount, (void*)pMediaFileInfo, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE), NULL,DEFAULT_WAIT_TIME);
	if (nRetCount < 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		bRet = FALSE;
	}

	else if (0 == nRetCount)
	{
		MessageBox(ConvertString("Query Number zero"), ConvertString("Prompt"));
		bRet = TRUE;
	}
	else
	{
		for (int i = 0; i < nRetCount; i++)
		{
			NET_OUT_MEDIA_QUERY_FILE* pInfo = &pMediaFileInfo[i];

			STR_RECORDFILE_INFO *recordfile = NEW STR_RECORDFILE_INFO;
			memset(recordfile, 0, sizeof(STR_RECORDFILE_INFO));
			strncpy(recordfile->filename, pInfo->szFilePath, sizeof(recordfile->filename) - 1);
			recordfile->size = pInfo->nFileSizeEx;

			recordfile->starttime = pInfo->stuStartTime;
			recordfile->endtime = pInfo->stuEndTime;
			recordfile->driveno = pInfo->nDriveNo;
			recordfile->startcluster = pInfo->nCluster;
			recordfile->nRecordFileType = 0;
			recordfile->ch = pInfo->nChannelID;

			m_QueryResultInfoVecotr.push_back(recordfile);
		}
		bRet = TRUE;
	}
	if (NULL != pMediaFileInfo)
	{
		delete []pMediaFileInfo;
		pMediaFileInfo = NULL;
	}
	return nRetCount;
}

int CQueryDownLoad::FindNextFaceVideo(int nSearchCount)
{
	int nRetCount = -1;
	BOOL bRet = FALSE;
	NET_OUT_MEDIA_QUERY_FILE *pMediaFileInfo = NEW NET_OUT_MEDIA_QUERY_FILE[nSearchCount];

	if (pMediaFileInfo == NULL)
	{
		return FALSE;
	}

	memset(pMediaFileInfo, 0, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE));

	for (int i = 0; i < nSearchCount; i++)
	{
		pMediaFileInfo[i].dwSize = sizeof(NET_OUT_MEDIA_QUERY_FILE);
	}

	nRetCount = CLIENT_FindNextFileEx(m_lFindHandle, nSearchCount, (void*)pMediaFileInfo, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE), NULL,DEFAULT_WAIT_TIME);
	if (nRetCount < 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		bRet = FALSE;
	}

	else if (0 == nRetCount)
	{
		MessageBox(ConvertString("Query Number zero"), ConvertString("Prompt"));
		bRet = TRUE;
	}
	else
	{
		for (int i = 0; i < nRetCount; i++)
		{
			NET_OUT_MEDIA_QUERY_FILE* pInfo = &pMediaFileInfo[i];

			STR_RECORDFILE_INFO *recordfile = NEW STR_RECORDFILE_INFO;
			memset(recordfile, 0, sizeof(STR_RECORDFILE_INFO));
			strncpy(recordfile->filename, pInfo->szFilePath, sizeof(recordfile->filename) - 1);
			recordfile->size = pInfo->nFileSizeEx;

			recordfile->starttime = pInfo->stuStartTime;
			recordfile->endtime = pInfo->stuEndTime;
			recordfile->driveno = pInfo->nDriveNo;
			recordfile->startcluster = pInfo->nCluster;
			recordfile->nRecordFileType = 0;
			recordfile->ch = pInfo->nChannelID;

			m_QueryResultInfoVecotr.push_back(recordfile);
		}
		bRet = TRUE;
	}
	if (NULL != pMediaFileInfo)
	{
		delete []pMediaFileInfo;
		pMediaFileInfo = NULL;
	}
	return nRetCount;
}

int CQueryDownLoad::FindNextFaceDetectionPicture(int nSearchCount)
{
	int nRetCount = -1;
	BOOL bRet = FALSE;
	MEDIAFILE_FACE_DETECTION_INFO* pMediaFileInfo = NEW MEDIAFILE_FACE_DETECTION_INFO[nSearchCount];

	if (NULL == pMediaFileInfo)
	{
		return FALSE;
	}
	memset(pMediaFileInfo, 0, nSearchCount * sizeof(MEDIAFILE_FACE_DETECTION_INFO));

	for (int i = 0; i < nSearchCount; i++)
	{
		//set dwSize (Must do)
		pMediaFileInfo[i].dwSize = sizeof(MEDIAFILE_FACE_DETECTION_INFO);
	}

	nRetCount = CLIENT_FindNextFileEx(m_lFindHandle, nSearchCount, (void*)pMediaFileInfo, nSearchCount * sizeof(NET_OUT_MEDIA_QUERY_FILE), NULL,DEFAULT_WAIT_TIME);
	if (nRetCount < 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		bRet = FALSE;
	}

	else if (0 == nRetCount)
	{
		MessageBox(ConvertString("Query Number zero"), ConvertString("Prompt"));
		bRet = TRUE;
	}
	else
	{
		for (int i = 0; i < nRetCount; i++)
		{
			MEDIAFILE_FACE_DETECTION_INFO* pInfo = &pMediaFileInfo[i];

			STR_RECORDFILE_INFO *recordfile = NEW STR_RECORDFILE_INFO;
			memset(recordfile, 0, sizeof(STR_RECORDFILE_INFO));
			strncpy(recordfile->filename, pInfo->szFilePath, sizeof(recordfile->filename) - 1);
			if (pInfo->sizeEx > 0)
			{
				recordfile->size = pInfo->sizeEx;
			}
			else
			{
				recordfile->size = pInfo->size;
			}

			recordfile->starttime = pInfo->starttime;
			recordfile->endtime = pInfo->endtime;
			recordfile->driveno = pInfo->bDriveNo;
			recordfile->startcluster = pInfo->nCluster;
			recordfile->nRecordFileType = 4;		//picture type
			recordfile->bHint = pInfo->bHint;
			recordfile->ch = pInfo->ch;

			m_QueryResultInfoVecotr.push_back(recordfile);
		}
		bRet = TRUE;
	}

	if (NULL != pMediaFileInfo)
	{
		delete []pMediaFileInfo;
		pMediaFileInfo = NULL;
	}
	return nRetCount;
}

int CQueryDownLoad::FindNextTargetRecognitionPicure(int nSearchCount)
{
	int nRetCount = -1;
	BOOL bRet = FALSE;
	MEDIAFILE_FACERECOGNITION_INFO* pMediaFileInfo = NEW MEDIAFILE_FACERECOGNITION_INFO[nSearchCount];

	if (NULL == pMediaFileInfo)
	{
		return FALSE;
	}
	memset(pMediaFileInfo, 0, nSearchCount * sizeof(MEDIAFILE_FACERECOGNITION_INFO));

	for (int i = 0; i < nSearchCount; i++)
	{
		//Set  dwSize (Must do)
		pMediaFileInfo[i].dwSize = sizeof(MEDIAFILE_FACERECOGNITION_INFO);
		pMediaFileInfo[i].stGlobalScenePic.dwSize = sizeof(pMediaFileInfo[i].stGlobalScenePic);
		pMediaFileInfo[i].stObjectPic.dwSize = sizeof(pMediaFileInfo[i].stObjectPic);

		pMediaFileInfo[i].stuCandidatesPic[0].dwSize = sizeof(pMediaFileInfo[i].stuCandidatesPic[0]);
	}

	nRetCount = CLIENT_FindNextFileEx(m_lFindHandle, nSearchCount, (void*)pMediaFileInfo, nSearchCount * sizeof(MEDIAFILE_FACERECOGNITION_INFO), NULL,DEFAULT_WAIT_TIME);
	if (nRetCount < 0)
	{
		MessageBox(ConvertString("Query fail"), ConvertString("Prompt"));
		bRet = FALSE;
	}

	else if (0 == nRetCount)
	{
		MessageBox(ConvertString("Query Number zero"), ConvertString("Prompt"));
		bRet = TRUE;
	}
	else
	{
		for (int i = 0; i < nRetCount; i++)
		{
			MEDIAFILE_FACERECOGNITION_INFO* pInfo = &pMediaFileInfo[i];

			STR_RECORDFILE_INFO *recordfile = NEW STR_RECORDFILE_INFO;
			memset(recordfile, 0, sizeof(STR_RECORDFILE_INFO));
			strncpy(recordfile->filename, pInfo->stObjectPic.szFilePath, sizeof(recordfile->filename) - 1);
			recordfile->starttime = pInfo->stTime;
			recordfile->endtime = pInfo->stTime;
			recordfile->ch = pInfo->nChannelId;
			recordfile->emBeard = pInfo->stuCandidatesEx[0].stPersonInfo.emBeard;
			recordfile->emEmotion = pInfo->stuCandidatesEx[0].stPersonInfo.emEmotion;
			recordfile->emSex = (EM_DEV_EVENT_FACEDETECT_SEX_TYPE)pInfo->stuCandidatesEx[0].stPersonInfo.bySex;

			if (pInfo->stuCandidatesEx[0].stPersonInfo.byGlasses == 1)
			{
				recordfile->emGlasses = EM_FACEDETECT_WITHOUT_GLASSES;
			}
			else if (pInfo->stuCandidatesEx[0].stPersonInfo.byGlasses == 2)
			{
				recordfile->emGlasses = EM_FACEDETECT_WITH_GLASSES;
			}
			else
			{
				recordfile->emGlasses = EM_FACEDETECT_GLASSES_UNKNOWN;
			}
			
			recordfile->emMask = pInfo->stuCandidatesEx[0].stPersonInfo.emMask;
	
			m_QueryResultInfoVecotr.push_back(recordfile);
		}
		bRet = TRUE;
	}

	if (NULL != pMediaFileInfo)
	{
		delete []pMediaFileInfo;
		pMediaFileInfo = NULL;
	}
	return nRetCount;
}

void CQueryDownLoad::UpdateLcQueryList(int nInfoNum)
{
	CString strIndex = "";
	CString strStartTime = "";
	CString strEndTime = "";
	CString strChannel = "";
	CString strInfo = "";
	m_lcQueryResult.DeleteAllItems();

	for (int n = 0; n < nInfoNum; n++)
	{
		strIndex.Format("%d", m_nPage * QUERY_FILE_NUM + n + 1);
		STR_RECORDFILE_INFO *pInfo = m_QueryResultInfoVecotr[m_nPage * QUERY_FILE_NUM + n];
		strChannel.Format("%d", pInfo->ch + 1);
		strStartTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->starttime.dwYear,
			pInfo->starttime.dwMonth,
			pInfo->starttime.dwDay,
			pInfo->starttime.dwHour,
			pInfo->starttime.dwMinute,
			pInfo->starttime.dwSecond);

		strEndTime.Format("%d-%02d-%02d %02d:%02d:%02d",
			pInfo->endtime.dwYear,
			pInfo->endtime.dwMonth,
			pInfo->endtime.dwDay,
			pInfo->endtime.dwHour,
			pInfo->endtime.dwMinute,
			pInfo->endtime.dwSecond);
		m_lcQueryResult.InsertItem(n, strIndex);
		m_lcQueryResult.SetItemText(n, 1, strChannel);
		m_lcQueryResult.SetItemText(n, 2, strStartTime);
		m_lcQueryResult.SetItemText(n, 3, strEndTime);
	}
}

void CQueryDownLoad::CloseFindHandle()
{
	if (0 == m_lFindHandle)
	{
		return;
	}
	BOOL bRet = CLIENT_FindCloseEx(m_lFindHandle);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Stop Qeury fail"), ConvertString("Prompt"));
	}
	m_lFindHandle = 0;
}

void CQueryDownLoad::ClearQueryResultInfoVector()
{
	std::vector<STR_RECORDFILE_INFO*>::iterator it = m_QueryResultInfoVecotr.begin();
	while(it != m_QueryResultInfoVecotr.end())
	{
		STR_RECORDFILE_INFO *pstGroupInfo = *it;
		if (pstGroupInfo)
		{
			it = m_QueryResultInfoVecotr.erase(it);
			delete pstGroupInfo;
			pstGroupInfo = NULL;
		}
	}
	m_QueryResultInfoVecotr.clear();
}

void CQueryDownLoad::OnBnClickedButtonQueryPrepage()
{
	m_nPage--;
	UpdateLcQueryList(QUERY_FILE_NUM);
	if (m_nPage <= 0)
	{
		GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(FALSE);
	}
	GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(TRUE);
}

void CQueryDownLoad::OnBnClickedButtonQueryNextpage()
{
	m_nPage++;
	int nSize = m_QueryResultInfoVecotr.size();
	if ( (m_nPage * QUERY_FILE_NUM + 1) == nSize )
	{
		int nCount = DoFindNextPageEx(QUERY_FILE_NUM);
		if (nCount == QUERY_FILE_NUM )
		{
			GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
			UpdateLcQueryList(nCount + 1);
		}
	}
	else if((m_nPage+1) * QUERY_FILE_NUM > nSize)
	{
		int nUpdateCount = nSize - m_nPage * QUERY_FILE_NUM;
		UpdateLcQueryList(nUpdateCount);
		GetDlgItem(IDC_BUTTON_QUERY_NEXTPAGE)->EnableWindow(FALSE);
	}
	else if ((m_nPage+1) * QUERY_FILE_NUM < nSize)
	{
		UpdateLcQueryList(QUERY_FILE_NUM);
	}
	GetDlgItem(IDC_BUTTON_QUERY_PREPAGE)->EnableWindow(TRUE);
}

void CQueryDownLoad::OnBnClickedButtonQueryDownload()
{
	CloseDownLoadHandle();
	CListCtrl *pQueryResult = (CListCtrl*)GetDlgItem(IDC_LIST_QUERY_RESULT);
	int nSel = pQueryResult->GetNextItem(-1, LVNI_SELECTED);
	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a download file"), ConvertString("Prompt"));
		return;
	}

	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(FALSE);

	switch(m_emQuryType)
	{
	case QUERY_PICFD:
		{
			DownLoadFaceDetectionPicture(nSel);
			break;
		}
	case QUERY_PICFR:
		{
			DownLoadTargetRecognitionPicure(nSel);
			break;
		}
	case QUERY_VIDEO:
		{
			DownLoadFaceVideo(nSel);
			break;
		}
	default:
		break;
	}
}

BOOL CQueryDownLoad::DownLoadFaceDetectionPicture( int nSel)
{
	BOOL bRet = FALSE;

	STR_RECORDFILE_INFO* pInfo = m_QueryResultInfoVecotr[m_nPage * QUERY_FILE_NUM + nSel];

	char szFileName[256] = {0};
	_snprintf(szFileName, sizeof(szFileName)-1,"\\DownLoad\\FaceDetection\\%d%02lu%02lu%02lu%02lu%02lu%02lu.jpg",
		pInfo->ch,
		pInfo->starttime.dwYear,
		pInfo->starttime.dwMonth,
		pInfo->starttime.dwDay,
		pInfo->starttime.dwHour,
		pInfo->starttime.dwMinute,
		pInfo->starttime.dwSecond
		);

	char *pFileName = NULL;
	CString filepath = "";
	if (m_pSoftPath)
	{
		filepath.Format("%s", m_pSoftPath);
	}
	CString filename(szFileName);
	CString strFile = filepath + filename;
	pFileName = strFile.GetBuffer(200);

	DH_IN_DOWNLOAD_REMOTE_FILE stuRemoteFileParm;
	memset(&stuRemoteFileParm, 0, sizeof(DH_IN_DOWNLOAD_REMOTE_FILE));
	stuRemoteFileParm.dwSize = sizeof(DH_IN_DOWNLOAD_REMOTE_FILE);
	stuRemoteFileParm.pszFileName = pInfo->filename ;
	stuRemoteFileParm.pszFileDst = pFileName;
	DH_OUT_DOWNLOAD_REMOTE_FILE fileinfo = {sizeof(DH_OUT_DOWNLOAD_REMOTE_FILE)};

	BOOL BRet = CLIENT_DownloadRemoteFile(m_lLogin, &stuRemoteFileParm, &fileinfo);
	if (BRet == FALSE)
	{
		MessageBox(ConvertString("DownLoad file fail"), ConvertString("Prompt"));
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
		MessageBox(ConvertString("Download file success!"), ConvertString("Prompt"));
	}

	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
	return bRet;
}

BOOL CQueryDownLoad::DownLoadTargetRecognitionPicure( int nSel)
{
	BOOL bRet = TRUE;
	STR_RECORDFILE_INFO* pInfo = m_QueryResultInfoVecotr[m_nPage * QUERY_FILE_NUM + nSel];
	char szFileName[128] = {0};
	_snprintf(szFileName, sizeof(szFileName)-1,"\\DownLoad\\TargetRecognition\\%d%02lu%02lu%02lu%02lu%02lu%02lu.jpg",
		pInfo->ch,
		pInfo->starttime.dwYear,
		pInfo->starttime.dwMonth,
		pInfo->starttime.dwDay,
		pInfo->starttime.dwHour,
		pInfo->starttime.dwMinute,
		pInfo->starttime.dwSecond
		);

	char *pFileName = NULL;
	CString filepath = "";
	if (m_pSoftPath)
	{
		filepath.Format("%s", m_pSoftPath);
	}
	CString filename(szFileName);
	CString strFile = filepath + filename;
	pFileName = strFile.GetBuffer(200);

	DH_IN_DOWNLOAD_REMOTE_FILE stuRemoteFileParm;
	memset(&stuRemoteFileParm, 0, sizeof(DH_IN_DOWNLOAD_REMOTE_FILE));
	stuRemoteFileParm.dwSize = sizeof(DH_IN_DOWNLOAD_REMOTE_FILE);
	stuRemoteFileParm.pszFileName = pInfo->filename ;
	stuRemoteFileParm.pszFileDst = pFileName;
	DH_OUT_DOWNLOAD_REMOTE_FILE fileinfo = {sizeof(DH_OUT_DOWNLOAD_REMOTE_FILE)};

	BOOL BRet = CLIENT_DownloadRemoteFile(m_lLogin, &stuRemoteFileParm, &fileinfo);
	if (BRet == FALSE)
	{
		MessageBox(ConvertString("DownLoad file fail"), ConvertString("Prompt"));
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
		MessageBox(ConvertString("Download file success!"), ConvertString("Prompt"));
	}
	GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
	return bRet;
}

BOOL CQueryDownLoad::DownLoadFaceVideo(int nSel)
{
	BOOL bRet = FALSE;
	STR_RECORDFILE_INFO* pInfo = m_QueryResultInfoVecotr[m_nPage * QUERY_FILE_NUM + nSel];
	NET_RECORDFILE_INFO recordfile;
	memset(&recordfile, 0, sizeof(recordfile));
	strncpy(recordfile.filename, pInfo->filename, sizeof(recordfile.filename) - 1);
	recordfile.size = pInfo->size;

	recordfile.starttime = pInfo->starttime;
	recordfile.endtime = pInfo->endtime;
	recordfile.driveno = pInfo->driveno;
	recordfile.startcluster = pInfo->startcluster;
	recordfile.nRecordFileType = pInfo->nRecordFileType;
	recordfile.ch = pInfo->ch;

	char szFileName[128] = {0};
	_snprintf(szFileName, sizeof(szFileName)-1, "\\DownLoad\\FaceVideo\\%d%02lu%02lu%02lu%02lu%02lu%02lu.dav",
		pInfo->ch,
		pInfo->starttime.dwYear,
		pInfo->starttime.dwMonth,
		pInfo->starttime.dwDay,
		pInfo->starttime.dwHour,
		pInfo->starttime.dwMinute,
		pInfo->starttime.dwSecond
		);

	char *pFileName = NULL;
	CString filepath = "";
	if (m_pSoftPath)
	{
		filepath.Format("%s", m_pSoftPath);
	}
	CString filename(szFileName);
	CString strFile = filepath + filename;
	pFileName = strFile.GetBuffer(200);

	m_lDownLoadHandle = CLIENT_DownloadByRecordFileEx(m_lLogin, &recordfile, pFileName, DownLoadPosCallBack, 
		(LDWORD)this, NULL, NULL);

	if (m_lDownLoadHandle == 0)
	{
		MessageBox(ConvertString("DownLoad file fail"), ConvertString("Prompt"));
		bRet = FALSE;
		GetDlgItem(IDC_BUTTON_QUERY_DOWNLOAD)->EnableWindow(TRUE);
	}
	else
	{
		bRet = TRUE;

	}
	return bRet;
}

void CQueryDownLoad::OnLvnItemchangedListQueryResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	ClosePlayBackHandle();

	m_dwPlayBackCurValue = 0;
	m_dwPlayBackTotalSize = 0;

	NET_IN_PLAY_BACK_BY_TIME_INFO stNetin;
	NET_OUT_PLAY_BACK_BY_TIME_INFO stNetOut;
	memset(&stNetin, 0, sizeof(NET_IN_PLAY_BACK_BY_TIME_INFO));
	memset(&stNetOut, 0, sizeof(NET_OUT_PLAY_BACK_BY_TIME_INFO));

	int nChannelId = -1;

	int nSel = m_lcQueryResult.GetNextItem(-1, LVNI_SELECTED);

	if (nSel == CB_ERR)
	{
		MessageBox(ConvertString("Please choose a valid file!"), ConvertString("Prompt"));
		*pResult = 0;
		return;
	}
	switch (m_emQuryType)
	{
	case QUERY_PICFD:
		{
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_REALPLAY_PIC));
			MessageBox(ConvertString("Only Video Support PlayBack!"), ConvertString("Prompt"));
			break;
		}
	case QUERY_PICFR:
		{
			FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_REALPLAY_PIC));
			MessageBox(ConvertString("Only Video Support PlayBack!"), ConvertString("Prompt"));
			break;
		}
	case QUERY_VIDEO:
		{
			STR_RECORDFILE_INFO* pInfo = m_QueryResultInfoVecotr[m_nPage * QUERY_FILE_NUM + nSel];
			stNetin.stStartTime = pInfo->starttime;
			stNetin.stStopTime = pInfo->endtime;

			nChannelId = pInfo->ch;
			CWnd *pWnd = GetDlgItem(IDC_STATIC_REALPLAY_PIC);
			HWND hwnd = pWnd->GetSafeHwnd();
			stNetin.hWnd = hwnd;
			stNetin.nPlayDirection = 0;
			stNetin.cbDownLoadPos = DownLoadPosCallBack;
			stNetin.dwPosUser = (LDWORD)this;

			m_playBackHandle = CLIENT_PlayBackByTimeEx2(m_lLogin, nChannelId, &stNetin, &stNetOut);
			if (m_playBackHandle == 0)
			{
				CString str;
				str.Format("error :0x%ld", CLIENT_GetLastError());
				MessageBox(str);
				MessageBox(ConvertString("PlayBack fail"), ConvertString("Prompt"));
			}
			break;
		}
	}
	*pResult = 0;
}

void CQueryDownLoad::ClosePlayBackHandle()
{
	if (m_playBackHandle)
	{
		BOOL bRet = CLIENT_StopPlayBack(m_playBackHandle);
		if (bRet == FALSE)
		{
			MessageBox(ConvertString("Stop PlayBack fail"), ConvertString("Prompt"));
		}
		m_playBackHandle = 0;
	}
}

void CQueryDownLoad::CloseDownLoadHandle()
{
	if (m_lDownLoadHandle)
	{
		CLIENT_StopDownload(m_lDownLoadHandle);
	}
	m_lDownLoadHandle = 0;
}

void CQueryDownLoad::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == m_nTimer)
	{
		m_PlayBackProcess.SetRange32(0,(int)(m_dwPlayBackTotalSize));
		m_PlayBackProcess.SetPos((int)(m_dwPlayBackCurValue) + 1);

		m_ProcessDownLoad.SetRange32(0,(int)(m_dwDownLoadTotalSize/1024));
		m_ProcessDownLoad.SetPos((int)(m_dwDownLoadCurValue));
	}

	CDialog::OnTimer(nIDEvent);
}

BOOL CQueryDownLoad::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

//get time from ui
void CQueryDownLoad::GetTimeFromUI()
{
	CDateTimeCtrl *pCDataStart = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_START);
	CDateTimeCtrl *pCTimeStart = (CDateTimeCtrl*)GetDlgItem(IDC_TIME_START);
	CDateTimeCtrl *pCDataEnd = (CDateTimeCtrl*)GetDlgItem(IDC_DATE_END);
	CDateTimeCtrl *pCTimeEnd = (CDateTimeCtrl*)GetDlgItem(IDC_TIME_END);

	SYSTEMTIME DataStart = {0};
	SYSTEMTIME TimeStart = {0};
	SYSTEMTIME DataEnd = {0};
	SYSTEMTIME TimeEnd = {0};

	pCDataStart->GetTime(&DataStart);
	pCTimeStart->GetTime(&TimeStart);
	pCDataEnd->GetTime(&DataEnd);
	pCTimeEnd->GetTime(&TimeEnd);

	m_DateStart.dwYear = DataStart.wYear;
	m_DateStart.dwMonth = DataStart.wMonth;
	m_DateStart.dwDay = DataStart.wDay;
	m_DateStart.dwHour = TimeStart.wHour;
	m_DateStart.dwMinute = TimeStart.wMinute;
	m_DateStart.dwSecond = TimeStart.wSecond;

	m_DateEnd.dwYear = DataEnd.wYear;
	m_DateEnd.dwMonth = DataEnd.wMonth;
	m_DateEnd.dwDay = DataEnd.wDay;
	m_DateEnd.dwHour = TimeEnd.wHour;
	m_DateEnd.dwMinute = TimeEnd.wMinute;
	m_DateEnd.dwSecond = TimeEnd.wSecond;
}

BOOL CQueryDownLoad::GetAttributeFromUI()
{
	int nSel = m_comboGlasses.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_emGlasses = (EM_FACEDETECT_GLASSES_TYPE)m_comboGlasses.GetItemData(nSel);
	}
	
	nSel = m_comboBeard.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_emBeard = (EM_BEARD_STATE_TYPE)m_comboBeard.GetItemData(nSel);
	}

	nSel = m_comboMask.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_emMask = (EM_MASK_STATE_TYPE)m_comboMask.GetItemData(nSel);
	}

	nSel = m_comboSex.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_emSex = (EM_DEV_EVENT_FACEDETECT_SEX_TYPE)m_comboSex.GetItemData(nSel);
	}

	nSel = m_comboEmotion.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_emEmotion = (EM_DEV_EVENT_FACEDETECT_FEATURE_TYPE)m_comboEmotion.GetItemData(nSel);
	}

	m_nAge[0] = GetDlgItemInt(IDC_EDIT_AGE1);
	m_nAge[1] = GetDlgItemInt(IDC_EDIT_AGE2);

	if (m_nAge[0] > m_nAge[1] )
	{
		m_nAge[0] = 0;
		m_nAge[1] = 0;
		MessageBox(ConvertString("Input age error"), ConvertString("Prompt"));
		return FALSE;
	}

	m_nSimilaryRange[0] = GetDlgItemInt(IDC_EDIT_SimilaryRange1);
	m_nSimilaryRange[1] = GetDlgItemInt(IDC_EDIT_SimilaryRange2);
	if (m_nSimilaryRange[0] > m_nSimilaryRange[1] || m_nSimilaryRange[0] < 0 || m_nSimilaryRange[1] > 100)
	{
		m_nSimilaryRange[0] = 0;
		m_nSimilaryRange[1] = 0;
		MessageBox(ConvertString("Input SimilaryRange error"), ConvertString("Prompt"));
		return FALSE;
	}

	return TRUE;
}

LRESULT CQueryDownLoad::OnDoDownLoad(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
	{
		return 0;
	}

	strCallBackInfo *pInfo = (strCallBackInfo *)wParam;
	if (pInfo)
	{
		ReceivePlayPos(pInfo->lHandle, pInfo->dwTotalSize, pInfo->dwDownLoadSize);
		delete pInfo;
		pInfo = NULL;
	}
	return 0;
}

void CQueryDownLoad::OnBnClickedCheckAttributeEnable()
{
	int tcheck = m_AttributeEnable.GetCheck();
	if (tcheck == BST_CHECKED)
	{
		GetDlgItem(IDC_COMBO_FILE_TYPE)->EnableWindow(FALSE);

		GetDlgItem(IDC_EDIT_AGE1)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_AGE2)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_QUERY_BREAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_QUERY_MASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_QUERY_SEX)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_QUERY_GLASSES)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_QUERY_EMOTION)->EnableWindow(TRUE);

		int nSel = m_comboEventType.GetCurSel();
		if (nSel == 0)
		{
			GetDlgItem(IDC_EDIT_SimilaryRange1)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_SimilaryRange2)->EnableWindow(FALSE);
		}
		else if (nSel == 1)
		{
			GetDlgItem(IDC_EDIT_SimilaryRange1)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_SimilaryRange2)->EnableWindow(TRUE);
		}

		m_nAttributeFlag = 1;
	}
	else 
	{
		ClearAttribute();
		GetDlgItem(IDC_COMBO_FILE_TYPE)->EnableWindow(TRUE);

		GetDlgItem(IDC_EDIT_AGE1)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_AGE2)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_QUERY_BREAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_QUERY_MASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_QUERY_SEX)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_QUERY_GLASSES)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_QUERY_EMOTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SimilaryRange1)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SimilaryRange2)->EnableWindow(FALSE);

		m_nAttributeFlag = 0;
	}
}

void CQueryDownLoad::ClearAttribute()
{
	m_emSex = EM_DEV_EVENT_FACEDETECT_SEX_TYPE_UNKNOWN;
	m_emGlasses = EM_FACEDETECT_GLASSES_UNKNOWN;
	m_emMask = EM_MASK_STATE_UNKNOWN;
	m_emBeard = EM_BEARD_STATE_UNKNOWN;
	memset(&m_nAge, 0, sizeof(m_nAge) );
	memset(&m_nSimilaryRange, 0, sizeof(m_nSimilaryRange));

	SetDlgItemText(IDC_EDIT_AGE1, "");
	SetDlgItemText(IDC_EDIT_AGE2, "");

	SetDlgItemText(IDC_EDIT_SimilaryRange1, "");
	SetDlgItemText(IDC_EDIT_SimilaryRange2, "");

	m_comboGlasses.SetCurSel(0);
	m_comboMask.SetCurSel(0);
	m_comboSex.SetCurSel(0);
	m_comboBeard.SetCurSel(0);
	m_comboEmotion.SetCurSel(0);
}
void CQueryDownLoad::OnCbnSelchangeComboAlarmType()
{
	if (m_nAttributeFlag == 1)
	{
		int nSel = m_comboEventType.GetCurSel();
		if (nSel == 0)
		{
			GetDlgItem(IDC_EDIT_SimilaryRange1)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_SimilaryRange2)->EnableWindow(FALSE);
			memset(&m_nSimilaryRange, 0, sizeof(m_nSimilaryRange));
			SetDlgItemText(IDC_EDIT_SimilaryRange1, "");
			SetDlgItemText(IDC_EDIT_SimilaryRange2, "");
		}
		else if (nSel == 1)
		{
			GetDlgItem(IDC_EDIT_SimilaryRange1)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_SimilaryRange2)->EnableWindow(TRUE);
		}
	}
}
