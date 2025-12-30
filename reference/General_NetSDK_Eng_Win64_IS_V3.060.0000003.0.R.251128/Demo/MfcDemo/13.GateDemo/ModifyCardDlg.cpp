// ModifyCardDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GateDemo.h"
#include "ModifyCardDlg.h"


// CModifyCardDlg 对话框

IMPLEMENT_DYNAMIC(CModifyCardDlg, CDialog)

CModifyCardDlg::CModifyCardDlg(LLONG lLoginId, const tagCardInfo& CardInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CModifyCardDlg::IDD, pParent)
{
	memset(&m_szFilePath, 0, sizeof(m_szFilePath));
	memset(&m_CardInfo, 0, sizeof(m_CardInfo));
	m_lLoginID = lLoginId;
	m_CardInfo = CardInfo;
}

CModifyCardDlg::~CModifyCardDlg()
{
}

void CModifyCardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MODIFYPIC, m_modifyPic);
	DDX_Control(pDX, IDC_COMBO_MODIFY_CARDSTATUS, m_cbxCardStatus);
	DDX_Control(pDX, IDC_COMBO_MODIFY_CARDTYPE, m_cbxCardType);
	DDX_Control(pDX, IDC_CHECK_ISENABLE_MODIFY, m_btnEnable);
	DDX_Control(pDX, IDC_CHECK_ISFIRST_MODIFY, m_btnIsFirst);
}


BEGIN_MESSAGE_MAP(CModifyCardDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN__MODIFY_OPENPIC, &CModifyCardDlg::OnBnClickedBtn)
	ON_BN_CLICKED(IDOK, &CModifyCardDlg::OnBnClickedOk)
END_MESSAGE_MAP()


BOOL CModifyCardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	InitControl();
	SetCardInfoToUI();

	return TRUE;  // return TRUE unless you set the focus to a control
}

// CModifyCardDlg 消息处理程序

BOOL CModifyCardDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CModifyCardDlg::OnBnClickedBtn()
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

		m_modifyPic.SetImageFile(PicPath);

		PicPath.ReleaseBuffer();
	}
}

void CModifyCardDlg::OnBnClickedOk()
{
	BOOL bRet = GetCardInfoFromUI();
	if (bRet == FALSE)
	{
		return;
	}

	//下发修改人脸信息
	BOOL bModifyFace = ModifyFaceInfo();
	if (FALSE == bModifyFace)
	{
		return;
	}

	//下发修改卡信息
	BOOL bModifyCard = ModifyCardInfo();
	if (FALSE == bModifyCard)
	{
		return;
	}

	MessageBox(ConvertString("Modify successfully"), ConvertString("Prompt"));
	OnOK();
}

void CModifyCardDlg::InitControl()
{
	m_cbxCardStatus.ResetContent();
	m_cbxCardStatus.AddString(ConvertString("Normal"));
	m_cbxCardStatus.AddString(ConvertString("Lose"));
	m_cbxCardStatus.AddString(ConvertString("Logoff"));
	m_cbxCardStatus.AddString(ConvertString("Freeze"));
	m_cbxCardStatus.AddString(ConvertString("Arrearage"));
	m_cbxCardStatus.AddString(ConvertString("Overdue"));
	m_cbxCardStatus.AddString(ConvertString("Prearrearage"));
	m_cbxCardStatus.AddString(ConvertString("Unknown"));
	m_cbxCardStatus.SetCurSel(0);

	m_cbxCardStatus.SetItemData(0, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_NORMAL);
	m_cbxCardStatus.SetItemData(1, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_LOSE);
	m_cbxCardStatus.SetItemData(2, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_LOGOFF);
	m_cbxCardStatus.SetItemData(3, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_FREEZE);
	m_cbxCardStatus.SetItemData(4, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_ARREARAGE);
	m_cbxCardStatus.SetItemData(5, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_OVERDUE);
	m_cbxCardStatus.SetItemData(6, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_PREARREARAGE);
	m_cbxCardStatus.SetItemData(7, (DWORD_PTR)NET_ACCESSCTLCARD_STATE_UNKNOWN);

	m_cbxCardType.ResetContent();
	m_cbxCardType.AddString(ConvertString("General"));
	m_cbxCardType.AddString(ConvertString("Vip"));
	m_cbxCardType.AddString(ConvertString("Guest"));
	m_cbxCardType.AddString(ConvertString("Patrol"));
	m_cbxCardType.AddString(ConvertString("Blocklist"));
	m_cbxCardType.AddString(ConvertString("Corce"));
	m_cbxCardType.AddString(ConvertString("Polling"));
	m_cbxCardType.AddString(ConvertString("Mothercard"));
	m_cbxCardType.AddString(ConvertString("Unknown"));
	m_cbxCardType.SetCurSel(0);

	m_cbxCardType.SetItemData(0, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_GENERAL);
	m_cbxCardType.SetItemData(1, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_VIP);
	m_cbxCardType.SetItemData(2, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_GUEST);
	m_cbxCardType.SetItemData(3, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_PATROL);
	m_cbxCardType.SetItemData(4, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_BLACKLIST);
	m_cbxCardType.SetItemData(5, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_CORCE);
	m_cbxCardType.SetItemData(6, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_POLLING);
	m_cbxCardType.SetItemData(7, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_MOTHERCARD);
	m_cbxCardType.SetItemData(8, (DWORD_PTR)NET_ACCESSCTLCARD_TYPE_UNKNOWN);
}

void CModifyCardDlg::SetData(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwYear = stuTime.wYear;
	stNetTime.dwMonth = stuTime.wMonth;
	stNetTime.dwDay = stuTime.wDay;
}

void CModifyCardDlg::SetTime(NET_TIME &stNetTime, int nID)
{
	CDateTimeCtrl *pInfo = (CDateTimeCtrl*)GetDlgItem(nID);
	SYSTEMTIME stuTime = {0};
	pInfo->GetTime(&stuTime);
	stNetTime.dwHour = stuTime.wHour;
	stNetTime.dwMinute = stuTime.wMinute;
	stNetTime.dwSecond = stuTime.wSecond;
}

BOOL CModifyCardDlg::GetCardInfoFromUI()
{
	CString strCardName;
	CString strCardPassword;
	CString strCardNo;
	CString strUserId;

	GetDlgItemText(IDC_EDIT_MODIFY_CARDNAME, strCardName);
	GetDlgItemText(IDC_EDIT_MODIFY_CARDPWD, strCardPassword);

	GetDlgItemText(IDC_EDIT_MODIFY_CARDNO, strCardNo);
	GetDlgItemText(IDC_EDIT_MODIFY_USERID, strUserId);

	//set the begin time
	SetData(m_CardInfo.stuValidStartTime, IDC_BEGIN_DATE_MODIFY);
	SetTime(m_CardInfo.stuValidStartTime, IDC_BEGIN_TIME_MODIFY);

	//set the end time
	SetData(m_CardInfo.stuValidEndTime, IDC_END_DATE_MODIFY);
	SetTime(m_CardInfo.stuValidEndTime, IDC_END_TIME_MODIFY);

	if (m_CardInfo.stuValidStartTime.dwYear < 2000 ||
		m_CardInfo.stuValidEndTime.dwYear > 2037)
	{
		MessageBox(ConvertString("Date input illegal valid time: 2000-01-01 ~ 2037-12-31"), ConvertString("Prompt"));
		return FALSE;
	}

	int nUserTime = GetDlgItemInt(IDC_EDIT_MODIFY_USERTIME);
	m_CardInfo.nUserTime = nUserTime;

	strncpy(m_CardInfo.szCardNo, strCardNo, sizeof(m_CardInfo.szCardNo) - 1);
	strncpy(m_CardInfo.szUserID, strUserId, sizeof(m_CardInfo.szUserID) - 1);
	strncpy(m_CardInfo.szCardName, strCardName, sizeof(m_CardInfo.szCardName) - 1);
	strncpy(m_CardInfo.szPsw, strCardPassword, sizeof(m_CardInfo.szPsw) - 1);

	int nCheck = m_btnEnable.GetCheck();
	if (nCheck == BST_CHECKED)
	{
		m_CardInfo.bIsValid = TRUE;
	}
	else
	{
		m_CardInfo.bIsValid = FALSE;
	}
	nCheck = m_btnIsFirst.GetCheck();
	if (nCheck == BST_CHECKED)
	{
		m_CardInfo.bFirstEnter = TRUE;
	}
	else
	{
		m_CardInfo.bFirstEnter = FALSE;
	}

	int nSel = m_cbxCardStatus.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_CardInfo.emStatus = (NET_ACCESSCTLCARD_STATE)m_cbxCardStatus.GetItemData(nSel);
	}
	nSel = m_cbxCardType.GetCurSel();
	if (nSel != CB_ERR)
	{
		m_CardInfo.emType = (NET_ACCESSCTLCARD_TYPE)m_cbxCardType.GetItemData(nSel);
	}

	return TRUE;
}

BOOL CModifyCardDlg::ModifyCardInfo()
{
	NET_CTRL_RECORDSET_PARAM stuInParam ;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	stuInParam.emType = NET_RECORD_ACCESSCTLCARD;

	NET_RECORDSET_ACCESS_CTL_CARD *pStrCardInfo = NEW NET_RECORDSET_ACCESS_CTL_CARD;
	if (pStrCardInfo == NULL)
	{
		MessageBox(ConvertString("NEW Memory fail"), "");
		return FALSE;
	}
	memset(pStrCardInfo, 0, sizeof(NET_RECORDSET_ACCESS_CTL_CARD));

	pStrCardInfo->dwSize = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	strncpy(pStrCardInfo->szCardNo, m_CardInfo.szCardNo, DH_MAX_CARDNO_LEN - 1);
	strncpy(pStrCardInfo->szCardName, m_CardInfo.szCardName, DH_MAX_CARDNAME_LEN - 1);
	strncpy(pStrCardInfo->szUserID, m_CardInfo.szUserID, DH_MAX_USERID_LEN - 1);
	strncpy(pStrCardInfo->szPsw, m_CardInfo.szPsw, DH_MAX_CARDPWD_LEN - 1);
	pStrCardInfo->emStatus = m_CardInfo.emStatus;
	pStrCardInfo->emType = m_CardInfo.emType;
	pStrCardInfo->nUserTime = m_CardInfo.nUserTime;
	pStrCardInfo->bFirstEnter = m_CardInfo.bFirstEnter;
	pStrCardInfo->bIsValid = m_CardInfo.bIsValid;
	pStrCardInfo->stuValidStartTime = m_CardInfo.stuValidStartTime;
	pStrCardInfo->stuValidEndTime = m_CardInfo.stuValidEndTime;
	pStrCardInfo->nRecNo = m_CardInfo.nRecNo;

	
	//DoorNum 固定为2  表示闸机的两扇门
	pStrCardInfo->nDoorNum = 2;
	pStrCardInfo->sznDoors[0] = 0;
	pStrCardInfo->sznDoors[1] = 1;
	//控制两扇门的开门有效时间  255表示全天
	pStrCardInfo->nTimeSectionNum = 2;
	pStrCardInfo->sznTimeSectionNo[0] = 255;
	pStrCardInfo->sznTimeSectionNo[1] = 255;

	stuInParam.nBufLen = sizeof(NET_RECORDSET_ACCESS_CTL_CARD);
	stuInParam.pBuf = (void*)pStrCardInfo;

	BOOL bRet = CLIENT_ControlDevice(m_lLoginID, DH_CTRL_RECORDSET_UPDATEEX, &stuInParam, DEFAULT_WAIT_TIME);
	if (FALSE == bRet)
	{
		MessageBox(ConvertString("Modify cardinfo fail"), ConvertString("Prompt"));
	}

	if (pStrCardInfo)
	{
		delete pStrCardInfo;
		pStrCardInfo = NULL;
	}

	return bRet;
}

BOOL CModifyCardDlg::ModifyFaceInfo()
{
	if (m_szFilePath[0] == '\0')
	{
		return TRUE;
	}

	BOOL bRet = FALSE;
	NET_IN_UPDATE_FACE_INFO stuInParam;
	memset(&stuInParam, 0, sizeof(stuInParam));
	stuInParam.dwSize = sizeof(stuInParam);
	strncpy(stuInParam.szUserID, m_CardInfo.szUserID, sizeof(m_CardInfo.szUserID) - 1);
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

	NET_OUT_UPDATE_FACE_INFO stuOutParam;
	memset(&stuOutParam, 0, sizeof(stuOutParam));
	stuOutParam.dwSize = sizeof(stuOutParam);

	bRet = CLIENT_FaceInfoOpreate(m_lLoginID, EM_FACEINFO_OPREATE_UPDATE, (void*)&stuInParam, (void*)&stuOutParam, DEFAULT_WAIT_TIME);
	if (bRet == FALSE)
	{
		MessageBox(ConvertString("Modify face fail"), ConvertString("Prompt"));
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

void CModifyCardDlg::SetCardInfoToUI()
{
	CString strCardNo;
	CString strUserID;
	CString strCardName;
	CString strCardPassword;

	strCardNo.Format("%s", m_CardInfo.szCardNo);
	strUserID.Format("%s", m_CardInfo.szUserID);
	strCardName.Format("%s", m_CardInfo.szCardName);
	strCardPassword.Format("%s", m_CardInfo.szPsw);

	SetDlgItemText(IDC_EDIT_MODIFY_CARDNO, strCardNo);
	SetDlgItemText(IDC_EDIT_MODIFY_USERID, strUserID);
	SetDlgItemText(IDC_EDIT_MODIFY_CARDNAME, strCardName);
	SetDlgItemText(IDC_EDIT_MODIFY_CARDPWD, strCardPassword);
	SetDlgItemInt(IDC_EDIT_MODIFY_USERTIME, m_CardInfo.nUserTime);

	int nSel = 0;
	switch (m_CardInfo.emStatus)
	{
	case NET_ACCESSCTLCARD_STATE_NORMAL:
		{
			nSel = 0;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_LOSE:
		{
			nSel = 1;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_LOGOFF:
		{
			nSel = 2;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_FREEZE:
		{
			nSel = 3;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_ARREARAGE:
		{
			nSel = 4;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_OVERDUE:
		{
			nSel = 5;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_PREARREARAGE:
		{
			nSel = 6;
			break;
		}
	case NET_ACCESSCTLCARD_STATE_UNKNOWN:
		{
			nSel = 7;
			break;
		}

	default:
		break;
	}
	m_cbxCardStatus.SetCurSel(nSel);

	int nTypeSel = 0;
	switch (m_CardInfo.emType)
	{
	case NET_ACCESSCTLCARD_TYPE_GENERAL:
		{
			nTypeSel = 0;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_VIP:
		{
			nTypeSel = 1;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_GUEST:
		{
			nTypeSel = 2;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_PATROL:
		{
			nTypeSel = 3;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_BLACKLIST:
		{
			nTypeSel = 4;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_CORCE:
		{
			nTypeSel = 5;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_POLLING:
		{
			nTypeSel = 6;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_MOTHERCARD:
		{
			nTypeSel = 7;
			break;
		}
	case NET_ACCESSCTLCARD_TYPE_UNKNOWN:
		{
			nTypeSel = 7;
			break;
		}
	default:
		break;
	}
	m_cbxCardType.SetCurSel(nTypeSel);

	if (m_CardInfo.bFirstEnter)
	{
		m_btnIsFirst.SetCheck(1);
	}
	else
	{
		m_btnIsFirst.SetCheck(0);
	}

	if (m_CardInfo.bIsValid)
	{
		m_btnEnable.SetCheck(1);
	}
	else
	{
		m_btnEnable.SetCheck(0);
	}

	CDateTimeCtrl *pDateBegin = (CDateTimeCtrl*)GetDlgItem(IDC_BEGIN_DATE_MODIFY);
	SYSTEMTIME dateBegin;
	memset(&dateBegin, 0, sizeof(dateBegin));
	dateBegin.wYear = m_CardInfo.stuValidStartTime.dwYear;
	dateBegin.wMonth = m_CardInfo.stuValidStartTime.dwMonth;
	dateBegin.wDay = m_CardInfo.stuValidStartTime.dwDay;

	pDateBegin->SetTime(&dateBegin);

	CDateTimeCtrl *pTimeBegin = (CDateTimeCtrl*)GetDlgItem(IDC_BEGIN_TIME_MODIFY);
	SYSTEMTIME timeBegin;
	memset(&timeBegin, 0, sizeof(timeBegin));
	timeBegin.wYear = m_CardInfo.stuValidStartTime.dwYear;
	timeBegin.wMonth = m_CardInfo.stuValidStartTime.dwMonth;
	timeBegin.wDay = m_CardInfo.stuValidStartTime.dwDay;
	timeBegin.wHour = m_CardInfo.stuValidStartTime.dwHour;
	timeBegin.wMinute = m_CardInfo.stuValidStartTime.dwMinute;
	timeBegin.wSecond = m_CardInfo.stuValidStartTime.dwSecond;

	pTimeBegin->SetTime(&timeBegin);

	CDateTimeCtrl *pDateEnd = (CDateTimeCtrl*)GetDlgItem(IDC_END_DATE_MODIFY);
	SYSTEMTIME dateEnd;
	memset(&dateEnd, 0, sizeof(dateEnd));
	dateEnd.wYear = m_CardInfo.stuValidEndTime.dwYear;
	dateEnd.wMonth = m_CardInfo.stuValidEndTime.dwMonth;
	dateEnd.wDay = m_CardInfo.stuValidEndTime.dwDay;

	pDateEnd->SetTime(&dateEnd);

	CDateTimeCtrl *pTimeEnd = (CDateTimeCtrl*)GetDlgItem(IDC_END_TIME_MODIFY);
	SYSTEMTIME timeEnd;
	memset(&timeEnd, 0, sizeof(timeEnd));
	timeEnd.wYear = m_CardInfo.stuValidEndTime.dwYear;
	timeEnd.wMonth = m_CardInfo.stuValidEndTime.dwMonth;
	timeEnd.wDay = m_CardInfo.stuValidEndTime.dwDay;
	timeEnd.wHour = m_CardInfo.stuValidEndTime.dwHour;
	timeEnd.wMinute = m_CardInfo.stuValidEndTime.dwMinute;
	timeEnd.wSecond = m_CardInfo.stuValidEndTime.dwSecond;

	pTimeEnd->SetTime(&timeEnd);
}
