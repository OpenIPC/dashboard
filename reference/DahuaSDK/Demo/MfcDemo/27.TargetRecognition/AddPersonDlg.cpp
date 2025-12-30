// AddPersonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "AddPersonDlg.h"
#include <atlconv.h>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddPersonDlg dialog


CAddPersonDlg::CAddPersonDlg(const LLONG lLoginHandle, const char *pszGroupID, const char *pszGroupName, const int nOpreateType, CANDIDATE_INFOEX *pstCandidateInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CAddPersonDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddPersonDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(m_szGroupId, 0, sizeof(m_szGroupId));
	strncpy(m_szGroupId, pszGroupID, sizeof(m_szGroupId)-1);
	memset(m_szGroupName, 0, sizeof(m_szGroupName));
	strncpy(m_szGroupName, pszGroupName, sizeof(m_szGroupName)-1);
	memset(m_szFilePath, 0, sizeof(m_szFilePath));
	m_lLoginID = lLoginHandle;
	m_nOpreateType = nOpreateType;
	memset(&m_stCandiDateInfo, 0, sizeof(m_stCandiDateInfo));
	if (NULL != pstCandidateInfo)
	{
		memcpy(&m_stCandiDateInfo, pstCandidateInfo, sizeof(m_stCandiDateInfo));
	}
	memset(&m_stAddPersonInfo, 0, sizeof(m_stAddPersonInfo));
}

CAddPersonDlg::~CAddPersonDlg()
{

}

BOOL CAddPersonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if (1 == m_nOpreateType) // add
	{
		SetWindowText(ConvertString("Add PersonInfo"));
	}
	else if (2 == m_nOpreateType) // edit
	{
		SetWindowText(ConvertString("Edit PersonInfo"));

		char szSoftPath[512] = {0};
		int filelen = GetModuleFileName(NULL, szSoftPath, 512);
		int k = filelen;
		while (szSoftPath[k] != '\\')
		{
			k--;
		}
		szSoftPath[k+1] = '\0';
		CString filepath(szSoftPath);
		CString filename("Face\\RemoteFace\\SepectPerson.jpg");
		CString strFile = filepath + filename;
		char *pFileDst = strFile.GetBuffer(256);
		m_pPersonAddPic.SetImageFile(pFileDst);

		SYSTEMTIME daTime = {0};
		CDateTimeCtrl* pBirth = (CDateTimeCtrl*)GetDlgItem(IDC_AddPersonBirthDay);
		//invalid data
		if (m_stCandiDateInfo.stPersonInfo.wYear ==0 | 
			m_stCandiDateInfo.stPersonInfo.byMonth ==0 |
			m_stCandiDateInfo.stPersonInfo.byDay == 0)
		{
			daTime.wYear = 1900;
			daTime.wMonth = 01;
			daTime.wDay = 01;
		}
		else
		{
			daTime.wYear = m_stCandiDateInfo.stPersonInfo.wYear;
			daTime.wMonth = m_stCandiDateInfo.stPersonInfo.byMonth;
			daTime.wDay = m_stCandiDateInfo.stPersonInfo.byDay;
		}
		pBirth->SetTime(&daTime);
	}
	
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

	SetDlgItemText(IDC_EDIT_ADD_NAME, m_stCandiDateInfo.stPersonInfo.szPersonName);
	SetDlgItemText(IDC_EDIT_ADD_CARDID, m_stCandiDateInfo.stPersonInfo.szID);

	SetDlgItemText(IDC_AddPersonGroupID, m_szGroupId);
	SetDlgItemText(IDC_AddPersonGroupName, m_szGroupName);
	
	m_cbSexType.SetCurSel(m_stCandiDateInfo.stPersonInfo.bySex);
	m_cbCardType.SetCurSel(m_stCandiDateInfo.stPersonInfo.byIDType);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAddPersonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddPersonDlg)
	DDX_Control(pDX, IDC_COMBO_ADD_SEX, m_cbSexType);
	DDX_Control(pDX, IDC_COMBO_ADD_ACRDTYPE, m_cbCardType);
	DDX_Control(pDX, IDC_AddPersonBirthDay, m_BirthDay);
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_PERSON_PIC, m_pPersonAddPic);
}


BEGIN_MESSAGE_MAP(CAddPersonDlg, CDialog)
	//{{AFX_MSG_MAP(CAddPersonDlg)
	ON_BN_CLICKED(IDC_OPEN_PIC, OnOpenPic)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CAddPersonDlg::OnBnClickedOk)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddPersonDlg message handlers

void CAddPersonDlg::OnOpenPic() 
{
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

		m_pPersonAddPic.SetImageFile(PicPath);

	}
}

void CAddPersonDlg::OnBnClickedOk()
{
	NET_IN_OPERATE_FACERECONGNITIONDB stuInParam = {sizeof(stuInParam)};
	NET_OUT_OPERATE_FACERECONGNITIONDB stuOutParam = {sizeof(stuOutParam)};

	stuInParam.bUsePersonInfoEx = TRUE;

	if (m_nOpreateType == FACE_PERSON_ADD)
	{
		stuInParam.emOperateType = NET_FACERECONGNITIONDB_ADD;
	}
	else if (m_nOpreateType == FACE_PERSON_EDIT)
	{
		stuInParam.emOperateType = NET_FACERECONGNITIONDB_MODIFY;
		if (strlen(m_stCandiDateInfo.stPersonInfo.szUID))
		{
			strncpy(stuInParam.stPersonInfoEx.szUID, m_stCandiDateInfo.stPersonInfo.szUID, sizeof(stuInParam.stPersonInfoEx.szUID)-1);
		}
	}

	CString strName;
	GetDlgItemText(IDC_EDIT_ADD_NAME, strName);
	char *pstrName = (LPSTR)(LPCSTR)strName;
	strncpy(stuInParam.stPersonInfoEx.szPersonName, pstrName, sizeof(stuInParam.stPersonInfoEx.szPersonName)-1);
	stuInParam.stPersonInfoEx.bySex = m_cbSexType.GetCurSel();
	stuInParam.stPersonInfoEx.byIDType = m_cbCardType.GetCurSel();
	CString strCardID;
	GetDlgItemText(IDC_EDIT_ADD_CARDID, strCardID);
	char *pstrCardID = (LPSTR)(LPCSTR)strCardID;
	strncpy(stuInParam.stPersonInfoEx.szID, pstrCardID, sizeof(stuInParam.stPersonInfoEx.szID)-1);

	CDateTimeCtrl *pCDataStart = (CDateTimeCtrl*)GetDlgItem(IDC_AddPersonBirthDay);
	SYSTEMTIME pstTime = {0};
	
	DWORD dwResult = m_BirthDay.GetTime(&pstTime);
	if (dwResult == GDT_VALID)
	{
		stuInParam.stPersonInfoEx.wYear = pstTime.wYear;
		stuInParam.stPersonInfoEx.byMonth = pstTime.wMonth;
		stuInParam.stPersonInfoEx.byDay = pstTime.wDay;
	}

	FILE *fPic = NULL;
	BOOL bRet = FALSE;

	strncpy(stuInParam.stPersonInfoEx.szGroupName, m_szGroupName, sizeof(stuInParam.stPersonInfoEx.szGroupName)-1);
	strncpy(stuInParam.stPersonInfoEx.szGroupID, m_szGroupId, sizeof(stuInParam.stPersonInfoEx.szGroupID)-1);

	if (strlen(m_szFilePath) > 0)
	{
		fPic = fopen(m_szFilePath, "rb+");
		if (fPic)
		{
			fseek(fPic, 0, SEEK_END);
			int nLength = ftell(fPic);
			if (nLength <= 0)
			{
				goto FREE_RETURN;
			}
			rewind(fPic);

			stuInParam.nBufferLen = nLength;
			stuInParam.pBuffer = new char[stuInParam.nBufferLen];
			if (NULL == stuInParam.pBuffer)
			{
				MessageBox(ConvertString("Memory error"), "");
				goto FREE_RETURN;
			}
			memset(stuInParam.pBuffer, 0, stuInParam.nBufferLen);
			int nReadLen = fread(stuInParam.pBuffer, 1, nLength, fPic);
			if (nReadLen != nLength)
			{
				goto FREE_RETURN;
			}

			stuInParam.stPersonInfoEx.wFacePicNum = 1;
			stuInParam.stPersonInfoEx.szFacePicInfo[0].dwOffSet = 0;
			stuInParam.stPersonInfoEx.szFacePicInfo[0].dwFileLenth = nLength;
		}
	}

	bRet = CLIENT_OperateFaceRecognitionDB(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		if (FACE_PERSON_ADD == m_nOpreateType)
		{
			MessageBox(ConvertString("Failed to add this person info!"), "");
		}
		else if (FACE_PERSON_EDIT == m_nOpreateType)
		{
			MessageBox(ConvertString("Failed to modify this person info!"), "");
		}
		goto FREE_RETURN;
	}
	else
	{
		if (FACE_PERSON_ADD == m_nOpreateType)
		{
			MessageBox(ConvertString("Success to add this person info!"), "");
		}
		else if (FACE_PERSON_EDIT == m_nOpreateType)
		{
			MessageBox(ConvertString("Success to modify this person info!"), "");
		}
	}

	memcpy(&m_stAddPersonInfo, &stuInParam.stPersonInfoEx, sizeof(m_stAddPersonInfo));
	if (FACE_PERSON_ADD == m_nOpreateType)
	{
		strncpy(m_stAddPersonInfo.szUID, stuOutParam.szUID, sizeof(m_stAddPersonInfo.szUID)-1);
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
}

BOOL CAddPersonDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
