// AddCardDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GateDemo.h"
#include "AddCardDlg.h"

// CAddCardDlg 对话框

IMPLEMENT_DYNAMIC(CAddCardDlg, CDialog)

CAddCardDlg::CAddCardDlg(LLONG lLoginId, CWnd* pParent /*=NULL*/)
	: CDialog(CAddCardDlg::IDD, pParent)
{
	memset(&m_szFilePath, 0, sizeof(m_szFilePath));
	m_lLoginID = lLoginId;
	memset(&m_StuAddCardInfo, 0, sizeof(m_StuAddCardInfo));
}

CAddCardDlg::~CAddCardDlg()
{
}

void CAddCardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ADDPIC, m_addPic);
	DDX_Control(pDX, IDC_COMBO_CARDSTATUS, m_comboCardStatus);
	DDX_Control(pDX, IDC_COMBO_CARDTYPE, m_comboCardType);
	DDX_Control(pDX, IDC_CHECK_ISFIRST, m_btnIsFirst);
	DDX_Control(pDX, IDC_CHECK_ISENABLE, m_btnEnable);
}


BEGIN_MESSAGE_MAP(CAddCardDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_OPENPIC, &CAddCardDlg::OnBnClickedBtnAddOpenpic)
	ON_BN_CLICKED(IDOK, &CAddCardDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CAddCardDlg 消息处理程序

BOOL CAddCardDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CAddCardDlg::OnBnClickedBtnAddOpenpic()
{
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

//insert card 下发卡
BOOL CAddCardDlg::InsertCardInfo()
{
	BOOL bRet = FALSE;
	NET_CTRL_RECORDSET_INSERT_PARAM stuInParam ;
	memset(&stuInParam, 0, sizeof(NET_CTRL_RECORDSET_INSERT_PARAM));
	stuInParam.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_PARAM);
	stuInParam.stuCtrlRecordSetInfo.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_IN);
	stuInParam.stuCtrlRecordSetResult.dwSize = sizeof(NET_CTRL_RECORDSET_INSERT_OUT);
	stuInParam.stuCtrlRecordSetInfo.emType = NET_RECORD_ACCESSCTLCARD;

	NET_RECORDSET_ACCESS_CTL_CARD *pStrCardInfo = NEW NET_RECORDSET_ACCESS_CTL_CARD;
	if (pStrCardInfo == NULL)
	{
		MessageBox(ConvertString("NEW Memory fail"), "");
		return FALSE;
	}
	memset(pStrCardInfo, 0, sizeof(NET_RECORDSET_ACCESS_CTL_CARD));

	pStrCardInfo->dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	strncpy(pStrCardInfo->szCardNo, m_StuAddCardInfo.szCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(pStrCardInfo->szCardName, m_StuAddCardInfo.szCardName, DH_MAX_CARDNAME_LEN - 1);
	strncpy(pStrCardInfo->szUserID, m_StuAddCardInfo.szUserID, DH_MAX_USERID_LEN - 1);
	strncpy(pStrCardInfo->szPsw, m_StuAddCardInfo.szPsw, DH_MAX_CARDPWD_LEN - 1);
	pStrCardInfo->emStatus = m_StuAddCardInfo.emStatus;
	pStrCardInfo->emType = m_StuAddCardInfo.emType;
	pStrCardInfo->nUserTime = m_StuAddCardInfo.nUserTime;
	pStrCardInfo->bFirstEnter = m_StuAddCardInfo.bFirstEnter;
	pStrCardInfo->bIsValid = m_StuAddCardInfo.bIsValid;
	pStrCardInfo->stuValidStartTime = m_StuAddCardInfo.stuValidStartTime;
	pStrCardInfo->stuValidEndTime = m_StuAddCardInfo.stuValidEndTime;

	//DoorNum 固定为2  表示闸机的两扇门
	pStrCardInfo->nDoorNum = 2;
	pStrCardInfo->sznDoors[0] = 0;
	pStrCardInfo->sznDoors[1] = 1;
	//控制两扇门的开门有效时间  255表示全天
	pStrCardInfo->nTimeSectionNum = 2;
	pStrCardInfo->sznTimeSectionNo[0] = 255;
	pStrCardInfo->sznTimeSectionNo[1] = 255;

	stuInParam.stuCtrlRecordSetInfo.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	stuInParam.stuCtrlRecordSetInfo.pBuf = (void*)pStrCardInfo;

	bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_INSERT, &stuInParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Insert card fail"), ConvertString("Prompt"));
	}

	if (pStrCardInfo)
	{
		delete pStrCardInfo;
		pStrCardInfo = NULL;
	}

	return bRet;
}

// insert face
BOOL CAddCardDlg::InsertFaceInfo()
{
	BOOL bRet = FALSE;
	NET_IN_ADD_FACE_INFO stuInParam;
	memset(&stuInParam, 0, sizeof(NET_IN_ADD_FACE_INFO));
	stuInParam.dwSize = sizeof(NET_IN_ADD_FACE_INFO);
	strncpy(stuInParam.szUserID, m_StuAddCardInfo.szUserID, DH_MAX_USERID_LEN - 1);
	stuInParam.stuFaceInfo.nFacePhoto = 1;

	FILE *fPic = fopen(m_szFilePath, "rb");
	if (fPic == NULL)
	{
		return FALSE;
	}

	fseek(fPic, 0, SEEK_END);
	int nLength = ftell(fPic);
	if (nLength <= 0)
	{
		goto FREE_RETURN;
	}
	rewind(fPic);

	stuInParam.stuFaceInfo.nFacePhotoLen[0] = nLength;
	stuInParam.stuFaceInfo.pszFacePhoto[0] = new char[nLength];

	if (NULL == stuInParam.stuFaceInfo.pszFacePhoto[0])
	{
		MessageBox(ConvertString("NEW Memory fail"), "");
		goto FREE_RETURN;
	}

	memset(stuInParam.stuFaceInfo.pszFacePhoto[0], 0, nLength);
	int nReadLen = fread(stuInParam.stuFaceInfo.pszFacePhoto[0], 1, nLength, fPic);
	fclose(fPic);
	fPic = NULL;
	if (nReadLen <= 0)
	{
		goto FREE_RETURN;
	}

	NET_OUT_ADD_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(NET_OUT_ADD_FACE_INFO));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginID, EM_FACEINFO_OPREATE_ADD, (void*)&stuInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Add face fail"), ConvertString("Prompt"));
	}

FREE_RETURN:
	if (fPic)
	{
		fclose(fPic);
		fPic = NULL;
	}
	if (stuInParam.stuFaceInfo.pszFacePhoto[0])
	{
		delete[] stuInParam.stuFaceInfo.pszFacePhoto[0];
		stuInParam.stuFaceInfo.pszFacePhoto[0] = NULL;
	}

	return bRet;
}

BOOL CAddCardDlg::GetCardInfoFromUI()
{
	CString strCardNo;
	CString strUserId;
	CString strCardName;
	CString strCardPassword;

	GetDlgItemText(IDC_EDIT_ADD_CARDNO, strCardNo);
	GetDlgItemText(IDC_EDIT_ADD_USERID, strUserId);
	GetDlgItemText(IDC_EDIT_ADD_CARDNAME, strCardName);
	GetDlgItemText(IDC_EDIT_ADD_CARDPWD, strCardPassword);

	if(strCardNo == "" || strUserId == "")
	{
		MessageBox(ConvertString("CardNo or UserID can not empty"), ConvertString("Prompt"));
		return FALSE;
	}

	HWND hWnd = this->GetSafeHwnd();
	if (NULL == hWnd)
	{
		MessageBox(ConvertString("hWnd is NULL"), "");
		return FALSE;
	}

	//set the begin time
	SetDate(m_StuAddCardInfo.stuValidStartTime, IDC_BEGIN_DATE);
	SetTime(m_StuAddCardInfo.stuValidStartTime, IDC_BEGIN_TIME);

	//set the end time
	SetDate(m_StuAddCardInfo.stuValidEndTime, IDC_END_DATE);
	SetTime(m_StuAddCardInfo.stuValidEndTime, IDC_END_TIME);

	if (m_StuAddCardInfo.stuValidStartTime.dwYear < 2000 ||
		m_StuAddCardInfo.stuValidEndTime.dwYear > 2037)
	{
		MessageBox(ConvertString("Date input illegal valid time: 2000-01-01 ~ 2037-12-31"), ConvertString("Prompt"));
		return FALSE;
	}

	int nUserTime = GetDlgItemInt(IDC_EDIT_USERTIME);
	m_StuAddCardInfo.nUserTime = nUserTime;
	strncpy(m_StuAddCardInfo.szCardNo, strCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(m_StuAddCardInfo.szUserID, strUserId, DH_MAX_USERID_LEN - 1);
	strncpy(m_StuAddCardInfo.szCardName, strCardName, DH_MAX_CARDNAME_LEN - 1);
	strncpy(m_StuAddCardInfo.szPsw, strCardPassword, DH_MAX_CARDPWD_LEN - 1);

	int nCheck = m_btnEnable.GetCheck();
	if (nCheck == BST_CHECKED)
	{
		m_StuAddCardInfo.bIsValid = TRUE;
	}
	else
	{
		m_StuAddCardInfo.bIsValid = FALSE;
	}
	nCheck = m_btnIsFirst.GetCheck();
	if (nCheck == BST_CHECKED)
	{
		m_StuAddCardInfo.bFirstEnter = TRUE;
	}
	else
	{
		m_StuAddCardInfo.bFirstEnter = FALSE;
	}

	int nSel = m_comboCardStatus.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_StuAddCardInfo.emStatus = (NET_ACCESSCTLCARD_STATE)m_comboCardStatus.GetItemData(nSel);
	}
	nSel = m_comboCardType.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_StuAddCardInfo.emType = (NET_ACCESSCTLCARD_TYPE)m_comboCardType.GetItemData(nSel);
	}
	
	return TRUE;
}

void CAddCardDlg::OnBnClickedOk()
{
	memset(&m_StuAddCardInfo, 0, sizeof(tagCardInfo));
	BOOL bRet = GetCardInfoFromUI();
	if (FALSE == bRet)
	{
		return;
	}

	BOOL bFaceEmpty = IsFacePictureEmpty();
	if (TRUE == bFaceEmpty)
	{
		return;
	}

	//下发卡
	BOOL bAddCard = InsertCardInfo();
	if (FALSE == bAddCard)	//失败直接返回
	{
		return;
	}

	//下发人脸
	BOOL bAddFace = InsertFaceInfo();
	if (FALSE == bAddFace)	//失败直接返回
	{
		return;
	}

	MessageBox(ConvertString("Add successfully"), ConvertString("Prompt"));
	OnOK();
}

void CAddCardDlg::SetDate(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwYear = stuTime.wYear;
	stNetTime.dwMonth = stuTime.wMonth;
	stNetTime.dwDay = stuTime.wDay;
}

void CAddCardDlg::SetTime(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwHour = stuTime.wHour;
	stNetTime.dwMinute = stuTime.wMinute;
	stNetTime.dwSecond = stuTime.wSecond;
}

void CAddCardDlg::InitControl()
{
	m_comboCardStatus.ResetContent();
	m_comboCardStatus.AddString(ConvertString("Normal"));
	m_comboCardStatus.AddString(ConvertString("Lose"));
	m_comboCardStatus.AddString(ConvertString("Logoff"));
	m_comboCardStatus.AddString(ConvertString("Freeze"));
	m_comboCardStatus.AddString(ConvertString("Arrearage"));
	m_comboCardStatus.AddString(ConvertString("Overdue"));
	m_comboCardStatus.AddString(ConvertString("Prearrearage"));
	m_comboCardStatus.AddString(ConvertString("Unknown"));
	m_comboCardStatus.SetCurSel(0);

	m_comboCardStatus.SetItemData(0, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_NORMAL);
	m_comboCardStatus.SetItemData(1, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_LOSE);
	m_comboCardStatus.SetItemData(2, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_LOGOFF);
	m_comboCardStatus.SetItemData(3, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_FREEZE);
	m_comboCardStatus.SetItemData(4, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_ARREARAGE);
	m_comboCardStatus.SetItemData(5, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_OVERDUE);
	m_comboCardStatus.SetItemData(6, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_PREARREARAGE);
	m_comboCardStatus.SetItemData(7, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_UNKNOWN);

	m_comboCardType.ResetContent();
	m_comboCardType.AddString(ConvertString("General"));
	m_comboCardType.AddString(ConvertString("Vip"));
	m_comboCardType.AddString(ConvertString("Guest"));
	m_comboCardType.AddString(ConvertString("Patrol"));
	m_comboCardType.AddString(ConvertString("Blocklist"));
	m_comboCardType.AddString(ConvertString("Corce"));
	m_comboCardType.AddString(ConvertString("Polling"));
	m_comboCardType.AddString(ConvertString("Mothercard"));
	m_comboCardType.AddString(ConvertString("Unknown"));
	m_comboCardType.SetCurSel(0);

	m_comboCardType.SetItemData(0, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_GENERAL);
	m_comboCardType.SetItemData(1, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_VIP);
	m_comboCardType.SetItemData(2, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_GUEST);
	m_comboCardType.SetItemData(3, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_PATROL);
	m_comboCardType.SetItemData(4, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_BLACKLIST);
	m_comboCardType.SetItemData(5, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_CORCE);
	m_comboCardType.SetItemData(6, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_POLLING);
	m_comboCardType.SetItemData(7, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_MOTHERCARD);
	m_comboCardType.SetItemData(8, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_UNKNOWN);
}

BOOL CAddCardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	g_SetWndStaticText(this);
	InitControl();

	SetDlgItemText(IDC_EDIT_USERTIME, "0");

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CAddCardDlg::IsFacePictureEmpty()
{
	if (m_szFilePath[0] == '\0')
	{
		MessageBox(ConvertString("Face picture empty"), ConvertString("Prompt"));
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}