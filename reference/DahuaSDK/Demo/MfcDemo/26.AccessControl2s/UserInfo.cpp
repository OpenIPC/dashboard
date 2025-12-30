// UserInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "UserInfo.h"
#include "UserInfoGetFingerPrint.h"
#include "UserInfoGetCard.h"
#include "DlgSensorInfoDescription.h"

#define WM_UI_USER_LIST_LOAD		(WM_USER + 400)
#define WM_UI_USER_CTRL_ENABLE		(WM_USER + 401)

DWORD WINAPI DoFindCardInfo(LPVOID lpParam)
{
	if (lpParam == 0)
	{
		return 0;
	}

	UserInfo *dlg = (UserInfo*)lpParam;
	if (dlg)
	{
		dlg->DoFindNextCardRecord();
	}
	return 1;
}

IMPLEMENT_DYNAMIC(UserInfo, CDialog)

UserInfo::UserInfo(CWnd* pParent /*=NULL*/)
	: CDialog(UserInfo::IDD, pParent)
{
	m_nDlgMode = 0;

	memset(&m_stuUserInfo,0,sizeof(m_stuUserInfo));
	m_stuUserInfo.nTimeSectionNum = 1;
	m_stuUserInfo.nTimeSectionNo[0] = 255;
	m_stuUserInfo.nSpecialDaysScheduleNum = 1;
	m_stuUserInfo.nSpecialDaysSchedule[0] = 255;
	m_stuUserInfo.emAuthority = NET_ATTENDANCE_AUTHORITY_CUSTOMER;
	m_stuUserInfo.emUserType = NET_ENUM_USER_TYPE_NORMAL;

	SYSTEMTIME time;
	GetSystemTime(&time);
	m_stuUserInfo.stuValidBeginTime.dwYear = time.wYear;
	m_stuUserInfo.stuValidBeginTime.dwMonth = time.wMonth;
	m_stuUserInfo.stuValidBeginTime.dwDay = time.wDay;
	m_stuUserInfo.stuValidBeginTime.dwHour = time.wHour;
	m_stuUserInfo.stuValidBeginTime.dwMinute = time.wMinute;
	m_stuUserInfo.stuValidBeginTime.dwSecond = time.wSecond;

	m_stuUserInfo.stuValidEndTime.dwYear = time.wYear + 10;
	m_stuUserInfo.stuValidEndTime.dwMonth = time.wMonth;
	m_stuUserInfo.stuValidEndTime.dwDay = time.wDay;
	m_stuUserInfo.stuValidEndTime.dwHour = time.wHour;
	m_stuUserInfo.stuValidEndTime.dwMinute = time.wMinute;
	m_stuUserInfo.stuValidEndTime.dwSecond = time.wSecond;

	vecDoor.clear();

	memset(&m_szFilePath, 0, sizeof(m_szFilePath));

	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;

	//m_nFingerPrintCount = 0;

	memset(&m_stuFingerprint,0,sizeof(m_stuFingerprint));
	m_stuFingerprint.dwSize = sizeof(m_stuFingerprint);
	m_stuFingerprint.nMaxFingerDataLength = 18*1024;
	m_stuFingerprint.pbyFingerData = new BYTE[18*1024];
	memset(m_stuFingerprint.pbyFingerData,0,18*1024);

	m_nFingerprintIndex = -1;

	for (int i=0;i<5;i++)
	{
		memset(&m_pCardInfo[i],0,sizeof(m_pCardInfo[i]));
	}
	m_nCardNum = 0;

	m_bIsDoFindNextCard = FALSE;
	m_ThreadHandle = 0;
	m_dwThreadID = 0;

	m_nCardIndex = -1;
	hParentWnd = NULL;
	pUserInfoGetFingerPrintDlg = NULL;
}

UserInfo::~UserInfo()
{
	if (m_stuFingerprint.pbyFingerData != NULL)
	{
		delete[] m_stuFingerprint.pbyFingerData;
		m_stuFingerprint.pbyFingerData = NULL;
	}

	m_bIsDoFindNextCard = FALSE;
	if (m_ThreadHandle)
	{
		int dwRet = WaitForSingleObject(m_ThreadHandle, INFINITE);
		if (dwRet == WAIT_OBJECT_0)
		{
			CloseHandle(m_ThreadHandle);
			m_ThreadHandle = 0;
		}
	}
}

void UserInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCTRL_FINGERPRINT, m_listFingerPrint);
	DDX_Control(pDX, IDC_LISTCTRL_CARD, m_listCard);
	DDX_Control(pDX, IDC_CMB_USERINFO_AUTHORITY, m_cmbAuthority);
	DDX_Control(pDX, IDC_CMB_USERINFO_USERTYPE, m_cmbUserType);
	DDX_Control(pDX, IDC_DTP_USERINFO_VDSTART, m_dtpVDStart);
	DDX_Control(pDX, IDC_DTP_USERINFO_VTSTART, m_dtpVTStart);
	DDX_Control(pDX, IDC_DTP_USERINFO_VDEND, m_dtpVDEnd);
	DDX_Control(pDX, IDC_DTP_USERINFO_VTEND, m_dtpVTEnd);
	DDX_Control(pDX, IDC_STATIC_MODIFYPIC, m_modifyPic);
	DDX_Control(pDX, IDC_RECSET_CARD_CHK_FIRSTENTER, m_chkFirstEnter);
}

BEGIN_MESSAGE_MAP(UserInfo, CDialog)
	ON_BN_CLICKED(IDOK, &UserInfo::OnBnClickedOk)
	ON_MESSAGE(WM_UI_USER_LIST_LOAD, &UserInfo::UserListLoad)
	ON_MESSAGE(WM_UI_USER_CTRL_ENABLE, &UserInfo::UserCtrlEnable)
	ON_BN_CLICKED(IDC_BUT_FACE_OPENPIC, &UserInfo::OnBnClickedButFaceOpenpic)
	ON_BN_CLICKED(IDC_BUT_FACE_GET, &UserInfo::OnBnClickedButFaceGet)
	ON_BN_CLICKED(IDC_BUT_FACE_SET, &UserInfo::OnBnClickedButFaceSet)
	ON_BN_CLICKED(IDC_BUT_FACE_MODIFY, &UserInfo::OnBnClickedButFaceModify)
	ON_BN_CLICKED(IDC_BUT_FACE_DELETE, &UserInfo::OnBnClickedButFaceDelete)
	ON_BN_CLICKED(IDC_BUT_FINGERPRINT_GET, &UserInfo::OnBnClickedButFingerprintGet)
	ON_BN_CLICKED(IDC_BUT_FINGERPRINT_ADD, &UserInfo::OnBnClickedButFingerprintAdd)
	ON_BN_CLICKED(IDC_BUT_FINGERPRINT_MODIFY, &UserInfo::OnBnClickedButFingerprintModify)
	ON_BN_CLICKED(IDC_BUT_FINGERPRINT_DELETE, &UserInfo::OnBnClickedButFingerprintDelete)
	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_FINGERPRINT, &UserInfo::OnNMClickListctrlFingerprint)
	ON_BN_CLICKED(IDC_BUT_CARD_GET, &UserInfo::OnBnClickedButCardGet)
	ON_BN_CLICKED(IDC_BUT_CARD_ADD, &UserInfo::OnBnClickedButCardAdd)
	ON_BN_CLICKED(IDC_BUT_CARD_MODIFY, &UserInfo::OnBnClickedButCardModify)
	ON_BN_CLICKED(IDC_BUT_CARD_DELETE, &UserInfo::OnBnClickedButCardDelete)
	ON_NOTIFY(NM_CLICK, IDC_LISTCTRL_CARD, &UserInfo::OnNMClickListctrlCard)
	ON_BN_CLICKED(IDC_INFO_USERINFO_BTN_DOORS, &UserInfo::OnBnClickedInfoUserinfoBtnDoors)
END_MESSAGE_MAP()

BOOL UserInfo::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	StuToDlg();

	if (Device::GetInstance().GetLoginState())
	{
		if (m_nDlgMode == 2)
		{
			OnBnClickedButFaceGet();
			OnBnClickedButFingerprintGet();
			OnBnClickedButCardGet();
		}
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDOK)->EnableWindow(FALSE);
		GetDlgItem(IDCANCEL)->EnableWindow(FALSE);

		GetDlgItem(IDC_BUT_FACE_OPENPIC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_DELETE)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void UserInfo::InitDlg()
{
	//listFingerPrint
	m_listFingerPrint.SetExtendedStyle(m_listFingerPrint.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listFingerPrint.SetExtendedStyle(m_listFingerPrint.GetExtendedStyle()|LVS_EX_GRIDLINES); 	

	std::vector<ColDes> vecTitlesFingerPrint;
	vecTitlesFingerPrint.push_back(ColDesObj("Index", 40));
	vecTitlesFingerPrint.push_back(ColDesObj("Type", 180));

	int nColCountFingerPrint = vecTitlesFingerPrint.size();
	LV_COLUMN lvcFingerPrint;
	lvcFingerPrint.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvcFingerPrint.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCountFingerPrint; j++) 
	{
		lvcFingerPrint.pszText = (char*)(vecTitlesFingerPrint[j].strColTitle.c_str());
		lvcFingerPrint.cx = vecTitlesFingerPrint[j].nColWidth;
		lvcFingerPrint.iSubItem = j;
		m_listFingerPrint.InsertColumn(j, &lvcFingerPrint);
	}

	//listCard
 	m_listCard.SetExtendedStyle(m_listCard.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
 	m_listCard.SetExtendedStyle(m_listCard.GetExtendedStyle()|LVS_EX_GRIDLINES); 	
 
 	std::vector<ColDes> vecTitlesCard;
 	vecTitlesCard.push_back(ColDesObj("Index", 40));
 	vecTitlesCard.push_back(ColDesObj("Type", 80));
 	vecTitlesCard.push_back(ColDesObj("Card", 80));
 
 	int nColCountCard = vecTitlesCard.size();
 	LV_COLUMN lvcCard;
 	lvcCard.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
 	lvcCard.fmt=LVCFMT_LEFT;
 	for(int j = 0; j < nColCountCard; j++) 
 	{
 		lvcCard.pszText = (char*)(vecTitlesCard[j].strColTitle.c_str());
 		lvcCard.cx = vecTitlesCard[j].nColWidth;
 		lvcCard.iSubItem = j;
 		m_listCard.InsertColumn(j, &lvcCard);
 	}

	// Authority
	m_cmbAuthority.ResetContent();
	for (int i = 0; i < sizeof(stuDemoAuthority)/sizeof(stuDemoAuthority[0]); i++)
	{
		m_cmbAuthority.InsertString(-1, ConvertString(stuDemoAuthority[i].szName));
	}
	m_cmbAuthority.SetCurSel(0);

	// UserType
	m_cmbUserType.ResetContent();
	for (int i = 0; i < sizeof(stuDemoUserType)/sizeof(stuDemoUserType[0]); i++)
	{
		m_cmbUserType.InsertString(-1, ConvertString(stuDemoUserType[i].szName));
	}
	m_cmbUserType.SetCurSel(0);

	if (m_nDlgMode == 1) //add user stye
	{
		GetDlgItem(IDC_BUT_FACE_OPENPIC)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FACE_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_FINGERPRINT_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_MODIFY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUT_CARD_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->SetWindowText(ConvertString("Add"));
	}
	else if(m_nDlgMode == 2) //modify user stye
	{
		GetDlgItem(IDC_EDIT_USERINFO_USERID)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->SetWindowText(ConvertString("Modify"));
	}		
}

void UserInfo::StuToDlg()
{
	//szUserID
	SetDlgItemText(IDC_EDIT_USERINFO_USERID, m_stuUserInfo.szUserID);

	//szName
	SetDlgItemText(IDC_EDIT_USERINFO_NAME, m_stuUserInfo.szName);

	//szPsw
	SetDlgItemText(IDC_EDIT_USERINFO_PWD, m_stuUserInfo.szPsw);

	//nTimeSectionNo
	if (m_stuUserInfo.nTimeSectionNum >= 1)
	{
		SetDlgItemInt(IDC_EDIT_USERINFO_TIMESECTIONNO,m_stuUserInfo.nTimeSectionNo[0],1);
	}

	//nSpecialDaysSchedule
	if (m_stuUserInfo.nSpecialDaysScheduleNum >= 1)
	{
		SetDlgItemInt(IDC_EDIT_USERINFO_SPECIALDAYS,m_stuUserInfo.nSpecialDaysSchedule[0],1);
	}

	//nUserTime
	SetDlgItemInt(IDC_EDIT_USERINFO_USERTIME,m_stuUserInfo.nUserTime,1);

	//emAuthority
	m_cmbAuthority.SetCurSel(AuthorityToInt(m_stuUserInfo.emAuthority));

	//emUserType
	m_cmbUserType.SetCurSel(UserTypeToInt(m_stuUserInfo.emUserType));

	// first enter
	if (m_stuUserInfo.bFirstEnter)
	{
		m_chkFirstEnter.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkFirstEnter.SetCheck(BST_UNCHECKED);
	}

	// valid time start
	SYSTEMTIME st = {0};
	st.wYear	= (WORD)m_stuUserInfo.stuValidBeginTime.dwYear;
	st.wMonth	= (WORD)m_stuUserInfo.stuValidBeginTime.dwMonth;
	st.wDay		= (WORD)m_stuUserInfo.stuValidBeginTime.dwDay;
	m_dtpVDStart.SetTime(&st);
	st.wHour    = (WORD)m_stuUserInfo.stuValidBeginTime.dwHour;
	st.wMinute  = (WORD)m_stuUserInfo.stuValidBeginTime.dwMinute;
	st.wSecond  = (WORD)m_stuUserInfo.stuValidBeginTime.dwSecond;
	m_dtpVTStart.SetTime(&st);

	// valid time end
	st.wYear	= (WORD)m_stuUserInfo.stuValidEndTime.dwYear;
	st.wMonth	= (WORD)m_stuUserInfo.stuValidEndTime.dwMonth;
	st.wDay		= (WORD)m_stuUserInfo.stuValidEndTime.dwDay;
	m_dtpVDEnd.SetTime(&st);
	st.wHour    = (WORD)m_stuUserInfo.stuValidEndTime.dwHour;
	st.wMinute  = (WORD)m_stuUserInfo.stuValidEndTime.dwMinute;
	st.wSecond  = (WORD)m_stuUserInfo.stuValidEndTime.dwSecond;
	m_dtpVTEnd.SetTime(&st);
}

void UserInfo::DlgToStu()
{
	//szUserID
	GetDlgItemText(IDC_EDIT_USERINFO_USERID, m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));

	//szName
	GetDlgItemText(IDC_EDIT_USERINFO_NAME, m_stuUserInfo.szName, sizeof(m_stuUserInfo.szName));

	//szPsw
	GetDlgItemText(IDC_EDIT_USERINFO_PWD, m_stuUserInfo.szPsw, sizeof(m_stuUserInfo.szPsw));

	//door
	std::vector<int>::iterator it = vecDoor.begin();
	for (int i = 0; i < __min(vecDoor.size(), DH_MAX_DOOR_NUM) && it != vecDoor.end(); i++, it++)
	{
		m_stuUserInfo.nDoors[i] = *it;
	}
	m_stuUserInfo.nDoorNum = __min(vecDoor.size(), DH_MAX_DOOR_NUM);

	//nTimeSectionNo
	m_stuUserInfo.nTimeSectionNum = 1;
	m_stuUserInfo.nTimeSectionNo[0] = GetDlgItemInt(IDC_EDIT_USERINFO_TIMESECTIONNO);

	//nSpecialDaysSchedule
	m_stuUserInfo.nSpecialDaysScheduleNum = 1;
	m_stuUserInfo.nSpecialDaysSchedule[0] = GetDlgItemInt(IDC_EDIT_USERINFO_SPECIALDAYS);

	//nUserTime
	m_stuUserInfo.nUserTime = GetDlgItemInt(IDC_EDIT_USERINFO_USERTIME);

	//emAuthority
	m_stuUserInfo.emAuthority = IntToAuthority(m_cmbAuthority.GetCurSel());

	//emUserType
	m_stuUserInfo.emUserType = IntToUserType(m_cmbUserType.GetCurSel());

	// first enter
	if (m_chkFirstEnter.GetCheck())
	{
		m_stuUserInfo.bFirstEnter = TRUE;
		m_stuUserInfo.nFirstEnterDoorsNum = 1;
		m_stuUserInfo.nFirstEnterDoors[0] = -1;
	}
	else
	{
		m_stuUserInfo.bFirstEnter = FALSE;
		m_stuUserInfo.nFirstEnterDoorsNum = 0;
	}

	// valid time start
	SYSTEMTIME st = {0};
	m_dtpVDStart.GetTime(&st);
	m_stuUserInfo.stuValidBeginTime.dwYear = st.wYear;
	m_stuUserInfo.stuValidBeginTime.dwMonth = st.wMonth;
	m_stuUserInfo.stuValidBeginTime.dwDay = st.wDay;
	m_dtpVTStart.GetTime(&st);
	m_stuUserInfo.stuValidBeginTime.dwHour = st.wHour;
	m_stuUserInfo.stuValidBeginTime.dwMinute = st.wMinute;
	m_stuUserInfo.stuValidBeginTime.dwSecond = st.wSecond;

	// valid time end
	m_dtpVDEnd.GetTime(&st);
	m_stuUserInfo.stuValidEndTime.dwYear = st.wYear;
	m_stuUserInfo.stuValidEndTime.dwMonth = st.wMonth;
	m_stuUserInfo.stuValidEndTime.dwDay = st.wDay;
	m_dtpVTEnd.GetTime(&st);
	m_stuUserInfo.stuValidEndTime.dwHour = st.wHour;
	m_stuUserInfo.stuValidEndTime.dwMinute = st.wMinute;
	m_stuUserInfo.stuValidEndTime.dwSecond = st.wSecond;
}

void UserInfo::OnBnClickedOk()
{
	DlgToStu();
	if (strcmp(m_stuUserInfo.szUserID,"") == 0)
	{
		MessageBox(ConvertString("UserID can not be empty"), ConvertString("Prompt"));
		return;
	}
	OnOK();
}

int UserInfo::AuthorityToInt(NET_ATTENDANCE_AUTHORITY emStatus)
{
	int n = -1;
	if (NET_ATTENDANCE_AUTHORITY_UNKNOWN == emStatus)
	{
		n = 0;
	}
	else if (NET_ATTENDANCE_AUTHORITY_CUSTOMER == emStatus)
	{
		n = 1;
	}
	else if (NET_ATTENDANCE_AUTHORITY_ADMINISTRATORS == emStatus)
	{
		n = 2;
	}
	return n;
}

NET_ATTENDANCE_AUTHORITY UserInfo::IntToAuthority(int n)
{
	NET_ATTENDANCE_AUTHORITY emStatua = NET_ATTENDANCE_AUTHORITY_UNKNOWN;
	if (1 == n)
	{
		emStatua = NET_ATTENDANCE_AUTHORITY_CUSTOMER;
	}
	else if (2 == n)
	{
		emStatua = NET_ATTENDANCE_AUTHORITY_ADMINISTRATORS;
	}
	return emStatua;
}

int UserInfo::UserTypeToInt(NET_ENUM_USER_TYPE emStatus)
{
	int n = -1;
	if (NET_ENUM_USER_TYPE_UNKNOWN == emStatus)
	{
		n = 0;
	}
	else if (NET_ENUM_USER_TYPE_NORMAL == emStatus)
	{
		n = 1;
	}
	else if (NET_ENUM_USER_TYPE_BLACKLIST == emStatus)
	{
		n = 2;
	}
	else if (NET_ENUM_USER_TYPE_GUEST == emStatus)
	{
		n = 3;
	}
	else if (NET_ENUM_USER_TYPE_PATROL == emStatus)
	{
		n = 4;
	}
	else if (NET_ENUM_USER_TYPE_VIP == emStatus)
	{
		n = 5;
	}
	else if (NET_ENUM_USER_TYPE_HANDICAP == emStatus)
	{
		n = 6;
	}
	else if (NET_ENUM_USER_TYPE_CUSTOM1 == emStatus)
	{
		n = 7;
	}
	else if (NET_ENUM_USER_TYPE_CUSTOM2 == emStatus)
	{
		n = 8;
	}
	return n;
}

NET_ENUM_USER_TYPE UserInfo::IntToUserType(int n)
{
	NET_ENUM_USER_TYPE emStatua = NET_ENUM_USER_TYPE_UNKNOWN;
	if (1 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_NORMAL;
	}
	else if (2 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_BLACKLIST;
	}
	else if (3 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_GUEST;
	}
	else if (4 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_PATROL;
	}
	else if (5 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_VIP;
	}
	else if (6 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_HANDICAP;
	}
	else if (7 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_CUSTOM1;
	}
	else if (8 == n)
	{
		emStatua = NET_ENUM_USER_TYPE_CUSTOM2;
	}
	return emStatua;
}

void UserInfo::FaceUIState(BOOL b)
{
	GetDlgItem(IDC_BUT_FACE_OPENPIC)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FACE_GET)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FACE_SET)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FACE_MODIFY)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FACE_DELETE)->EnableWindow(b);
}

void UserInfo::OnBnClickedButFaceOpenpic()
{
	FaceUIState(FALSE);
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
	FaceUIState(TRUE);
}

void UserInfo::OnBnClickedButFaceGet()
{
	FaceUIState(FALSE);
	NET_IN_ACCESS_FACE_SERVICE_GET stuFaceGetIn = {sizeof(stuFaceGetIn)};
	stuFaceGetIn.nUserNum = 1;
	memcpy(&stuFaceGetIn.szUserID[0], &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));

	NET_OUT_ACCESS_FACE_SERVICE_GET stuFaceGetOut = {sizeof(stuFaceGetOut)};
	stuFaceGetOut.nMaxRetNum = 1;
	NET_ACCESS_FACE_INFO stuFaceInfo;
	memset(&stuFaceInfo,0,sizeof(stuFaceInfo));
	for (int i=0;i<5;i++)
	{
		stuFaceInfo.nInFacePhotoLen[i] = 100*1024;
		stuFaceInfo.pFacePhoto[i] = new char[100*1024];
		memset(stuFaceInfo.pFacePhoto[i],0,100*1024);
	}
	stuFaceGetOut.pFaceInfo = &stuFaceInfo;

	NET_EM_FAILCODE stuFailCodeR = NET_EM_FAILCODE_NOERROR;
	stuFaceGetOut.pFailCode = &stuFailCodeR;
	BOOL bret = Device::GetInstance().FaceGet(&stuFaceGetIn, stuFaceGetOut);
	if (bret)
	{
		m_modifyPic.SetImageDate((BYTE*)stuFaceInfo.pFacePhoto[0], stuFaceInfo.nOutFacePhotoLen[0]);
	}
	else
	{
		
		if(stuFailCodeR != NET_EM_FAILCODE_UNKNOWN && stuFailCodeR != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCodeR).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
		//MessageBox(ConvertString("Get face fail"), ConvertString("Prompt"));
	}
	
	for (int i=0;i<5;i++)
	{
		delete[] stuFaceInfo.pFacePhoto[i]; 
		stuFaceInfo.pFacePhoto[i] = NULL;
	}
	FaceUIState(TRUE);
}

void UserInfo::OnBnClickedButFaceSet()
{
	
	if (m_szFilePath[0] == '\0')
	{
		return;
	}
	FaceUIState(FALSE);
	NET_IN_ACCESS_FACE_SERVICE_INSERT  stuFaceInsertIn = {sizeof(stuFaceInsertIn)};
	stuFaceInsertIn.nFaceInfoNum = 1;
	NET_ACCESS_FACE_INFO stuFaceInfo;
	memset(&stuFaceInfo,0,sizeof(stuFaceInfo));
	memcpy(&stuFaceInfo.szUserID, &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
	stuFaceInfo.nFacePhoto = 1;

	FILE *fPic = fopen(m_szFilePath, "rb");
	if (fPic == NULL)
	{
		FaceUIState(TRUE);
		MessageBox(ConvertString("Open picture fail"), ConvertString("Prompt"));
		return;
	}

	fseek(fPic, 0, SEEK_END);
	int nLength = ftell(fPic);
	if (nLength <= 0)
	{
		goto FREE_RETURN;
	}
	rewind(fPic);

	stuFaceInfo.nInFacePhotoLen[0]= nLength;
	stuFaceInfo.nOutFacePhotoLen[0] = nLength;
	stuFaceInfo.pFacePhoto[0] = new char[nLength];
	memset(stuFaceInfo.pFacePhoto[0], 0, nLength);
	int nReadLen = fread(stuFaceInfo.pFacePhoto[0], 1, nLength, fPic);
	fclose(fPic);
	fPic = NULL;
	if (nReadLen <= 0)
	{
		goto FREE_RETURN;
	}
	stuFaceInsertIn.pFaceInfo = &stuFaceInfo;

	NET_OUT_ACCESS_FACE_SERVICE_INSERT  stuFaceInsertOut = {sizeof(stuFaceInsertOut)};
	stuFaceInsertOut.nMaxRetNum = 1;
	NET_EM_FAILCODE stuFailCodeR = NET_EM_FAILCODE_NOERROR;
	stuFaceInsertOut.pFailCode = &stuFailCodeR;
	BOOL bRet = Device::GetInstance().FaceInsert(1,&stuFaceInfo,stuFailCodeR);
	if (bRet == FALSE)
	{
		if(stuFailCodeR != NET_EM_FAILCODE_UNKNOWN && stuFailCodeR != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCodeR).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	FaceUIState(TRUE);
FREE_RETURN:
	if (fPic)
	{
		fclose(fPic);
		fPic = NULL;
	}
	if (stuFaceInfo.pFacePhoto[0])
	{
		delete[] stuFaceInfo.pFacePhoto[0];
		stuFaceInfo.pFacePhoto[0] = NULL;
	}
	FaceUIState(TRUE);
}

void UserInfo::OnBnClickedButFaceModify()
{
	if (m_szFilePath[0] == '\0')
	{
		return;
	}
	FaceUIState(FALSE);
	NET_IN_ACCESS_FACE_SERVICE_INSERT  stuFaceInsertIn = {sizeof(stuFaceInsertIn)};
	stuFaceInsertIn.nFaceInfoNum = 1;
	NET_ACCESS_FACE_INFO stuFaceInfo;
	memset(&stuFaceInfo,0,sizeof(stuFaceInfo));
	memcpy(&stuFaceInfo.szUserID, &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
	stuFaceInfo.nFacePhoto = 1;

	FILE *fPic = fopen(m_szFilePath, "rb");
	if (fPic == NULL)
	{
		FaceUIState(TRUE);
		MessageBox(ConvertString("Open picture fail"), ConvertString("Prompt"));
		return;
	}

	fseek(fPic, 0, SEEK_END);
	int nLength = ftell(fPic);
	if (nLength <= 0)
	{
		goto FREE_RETURN;
	}
	rewind(fPic);

	stuFaceInfo.nInFacePhotoLen[0]= nLength;
	stuFaceInfo.nOutFacePhotoLen[0] = nLength;
	stuFaceInfo.pFacePhoto[0] = new char[nLength];
	memset(stuFaceInfo.pFacePhoto[0], 0, nLength);
	int nReadLen = fread(stuFaceInfo.pFacePhoto[0], 1, nLength, fPic);
	fclose(fPic);
	fPic = NULL;
	if (nReadLen <= 0)
	{
		goto FREE_RETURN;
	}
	stuFaceInsertIn.pFaceInfo = &stuFaceInfo;

	NET_OUT_ACCESS_FACE_SERVICE_INSERT  stuFaceInsertOut = {sizeof(stuFaceInsertOut)};
	stuFaceInsertOut.nMaxRetNum = 1;
	NET_EM_FAILCODE stuFailCodeR = NET_EM_FAILCODE_NOERROR;
	stuFaceInsertOut.pFailCode = &stuFailCodeR;
	BOOL bRet = Device::GetInstance().FaceUpdate(1,&stuFaceInfo,stuFailCodeR);
	if (bRet == FALSE)
	{
		if(stuFailCodeR != NET_EM_FAILCODE_UNKNOWN && stuFailCodeR != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCodeR).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	FaceUIState(TRUE);
FREE_RETURN:
	if (fPic)
	{
		fclose(fPic);
		fPic = NULL;
	}
	if (stuFaceInfo.pFacePhoto[0])
	{
		delete[] stuFaceInfo.pFacePhoto[0];
		stuFaceInfo.pFacePhoto[0] = NULL;
	}
	FaceUIState(TRUE);
}

void UserInfo::OnBnClickedButFaceDelete()
{
	FaceUIState(FALSE);
	m_modifyPic.SetImageDate(NULL,0);
	GetDlgItem(IDC_STATIC_MODIFYPIC)->Invalidate();

	//Remove
	NET_IN_ACCESS_FACE_SERVICE_REMOVE stuFaceRIn = {sizeof(stuFaceRIn)};
	stuFaceRIn.nUserNum = 1;
	memcpy(&stuFaceRIn.szUserID[0], &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
	NET_OUT_ACCESS_FACE_SERVICE_REMOVE stuFaceROut = {sizeof(stuFaceROut)};
	stuFaceROut.nMaxRetNum = 1;
	NET_EM_FAILCODE stuFailCodeR = NET_EM_FAILCODE_NOERROR;
	stuFaceROut.pFailCode = &stuFailCodeR;
	BOOL bret = Device::GetInstance().FaceRemove(&stuFaceRIn, stuFaceROut);
	if (bret == FALSE)
	{
		if(stuFailCodeR != NET_EM_FAILCODE_UNKNOWN && stuFailCodeR != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCodeR).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	FaceUIState(TRUE);
}

void UserInfo::FingerPrintUIState(BOOL b)
{
	GetDlgItem(IDC_BUT_FINGERPRINT_GET)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FINGERPRINT_ADD)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FINGERPRINT_MODIFY)->EnableWindow(b);
	GetDlgItem(IDC_BUT_FINGERPRINT_DELETE)->EnableWindow(b);
}

void UserInfo::OnBnClickedButFingerprintGet()
{
	FingerPrintUIState(FALSE);
	m_listFingerPrint.DeleteAllItems();
	m_stuFingerprint.nRetFingerPrintCount = 0;

	NET_IN_ACCESS_FINGERPRINT_SERVICE_GET stuFingerPrintGetIn = {sizeof(stuFingerPrintGetIn)};
	memcpy(&stuFingerPrintGetIn.szUserID[0], &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));

	BOOL bret = Device::GetInstance().FingerprintGet(&stuFingerPrintGetIn,m_stuFingerprint);
	if (bret)
	{
		m_listFingerPrint.SetRedraw(FALSE);
		
		for (int i=0;i<m_stuFingerprint.nRetFingerPrintCount;i++)
		{
			CString strIndex;
			strIndex.Format("%d",i + 1);
			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = i;
			m_listFingerPrint.InsertItem(&lvi);
			m_listFingerPrint.SetItemText(i, 0, strIndex);
			CString str;
			if (m_stuFingerprint.nDuressIndex == (i+1))
			{
				str.Format("%s",ConvertString("DuressFinger"));
			}
			else
			{
				str.Format("%s",ConvertString("Normal"));
			}
			m_listFingerPrint.SetItemText(i, 1, str);
			
		}
		m_listFingerPrint.SetRedraw(TRUE);
	}
	else
	{
		
		//MessageBox(ConvertString("Get fingerprint fail"), ConvertString("Prompt"));
	}
	FingerPrintUIState(TRUE);
}

void UserInfo::OnBnClickedButFingerprintAdd()
{
	if (m_stuFingerprint.nRetFingerPrintCount >= 3)
	{
		MessageBox(ConvertString("The fingerprints are full"), ConvertString("Prompt"));
		return;
	}
	FingerPrintUIState(FALSE);
	pUserInfoGetFingerPrintDlg = new UserInfoGetFingerPrint(this);
	if (NULL != pUserInfoGetFingerPrintDlg)
	{
		pUserInfoGetFingerPrintDlg->SetDlgHWND(hParentWnd);
		if (IDOK == pUserInfoGetFingerPrintDlg->DoModal())
		{
			m_nFingerprintLen = pUserInfoGetFingerPrintDlg->GetFingerprintData(m_byFingerprintData);
			BOOL bDuress = pUserInfoGetFingerPrintDlg->GetDuressState();
			if (m_nFingerprintLen > 0)
			{
				NET_ACCESS_FINGERPRINT_INFO stuFingerPrintInfo;
				memset(&stuFingerPrintInfo,0,sizeof(stuFingerPrintInfo));
				memcpy(&stuFingerPrintInfo.szUserID, &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
				stuFingerPrintInfo.nPacketLen = m_nFingerprintLen;
				stuFingerPrintInfo.nPacketNum = 1;
				stuFingerPrintInfo.szFingerPrintInfo = new char[m_nFingerprintLen];
				memset(stuFingerPrintInfo.szFingerPrintInfo, 0, m_nFingerprintLen);
				memcpy(stuFingerPrintInfo.szFingerPrintInfo, m_byFingerprintData, m_nFingerprintLen);
				if (bDuress)
				{
					stuFingerPrintInfo.nDuressIndex = 1;
				}
				//stuFingerPrintInfo.nDuressIndex
				NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
				BOOL bret = Device::GetInstance().FingerprintInsert(1,&stuFingerPrintInfo,stuFailCode);
				if (!bret)
				{
					if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
					{
						CString csInfo;
						csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
						MessageBox(csInfo, ConvertString("Prompt"));
					}
				}
				else
				{
					OnBnClickedButFingerprintGet();
				}
				if (stuFingerPrintInfo.szFingerPrintInfo != NULL)
				{
					delete[] stuFingerPrintInfo.szFingerPrintInfo;
					stuFingerPrintInfo.szFingerPrintInfo = NULL;
				}
			}
		}

		delete pUserInfoGetFingerPrintDlg;
		pUserInfoGetFingerPrintDlg = NULL;
	}
	FingerPrintUIState(TRUE);
}

void UserInfo::OnBnClickedButFingerprintModify()
{
	FingerPrintUIState(FALSE);
	pUserInfoGetFingerPrintDlg = new UserInfoGetFingerPrint(this);
	if (NULL != pUserInfoGetFingerPrintDlg)
	{
		pUserInfoGetFingerPrintDlg->SetDlgHWND(hParentWnd);
		if (m_stuFingerprint.nDuressIndex == (m_nFingerprintIndex + 1))
		{
			pUserInfoGetFingerPrintDlg->SetDuressState(TRUE);
		}
		if (IDOK == pUserInfoGetFingerPrintDlg->DoModal())
		{
			m_nFingerprintLen = pUserInfoGetFingerPrintDlg->GetFingerprintData(m_byFingerprintData);
			BOOL bDuress = pUserInfoGetFingerPrintDlg->GetDuressState();
			if (m_nFingerprintLen > 0)
			{
				NET_ACCESS_FINGERPRINT_INFO stuFingerPrintInfo;
				memset(&stuFingerPrintInfo,0,sizeof(stuFingerPrintInfo));
				memcpy(&stuFingerPrintInfo.szUserID, &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
				if (m_nFingerprintLen != m_stuFingerprint.nSinglePacketLength)
				{
					MessageBox(ConvertString("FingerprintLen error"), ConvertString("Prompt"));
					FingerPrintUIState(TRUE);
					return;
				}
				stuFingerPrintInfo.nPacketLen = m_nFingerprintLen;
				stuFingerPrintInfo.nPacketNum = m_stuFingerprint.nRetFingerPrintCount;
				stuFingerPrintInfo.szFingerPrintInfo = new char[m_nFingerprintLen * m_stuFingerprint.nRetFingerPrintCount];
				memset(stuFingerPrintInfo.szFingerPrintInfo, 0, m_nFingerprintLen * m_stuFingerprint.nRetFingerPrintCount);
				memcpy(stuFingerPrintInfo.szFingerPrintInfo, m_stuFingerprint.pbyFingerData, m_stuFingerprint.nRetFingerDataLength);
				memcpy(stuFingerPrintInfo.szFingerPrintInfo + m_nFingerprintIndex * m_nFingerprintLen, m_byFingerprintData, m_nFingerprintLen);
				if (bDuress)
				{
					stuFingerPrintInfo.nDuressIndex = m_nFingerprintIndex + 1;
				}
				//stuFingerPrintInfo.nDuressIndex
				NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
				BOOL bret = Device::GetInstance().FingerprintUpdate(1,&stuFingerPrintInfo,stuFailCode);
				if (!bret)
				{
					if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
					{
						CString csInfo;
						csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
						MessageBox(csInfo, ConvertString("Prompt"));
					}
				}
				else
				{
					OnBnClickedButFingerprintGet();
				}
				if (stuFingerPrintInfo.szFingerPrintInfo != NULL)
				{
					delete[] stuFingerPrintInfo.szFingerPrintInfo;
					stuFingerPrintInfo.szFingerPrintInfo = NULL;
				}
			}
		}

		delete pUserInfoGetFingerPrintDlg;
		pUserInfoGetFingerPrintDlg = NULL;
	}
	FingerPrintUIState(TRUE);
}

void UserInfo::OnBnClickedButFingerprintDelete()
{
	FingerPrintUIState(FALSE);
	NET_IN_ACCESS_FINGERPRINT_SERVICE_REMOVE stuFingerPrintRemoveIn = {sizeof(stuFingerPrintRemoveIn)};
	stuFingerPrintRemoveIn.nUserNum = 1;
	memcpy(&stuFingerPrintRemoveIn.szUserID[0], &m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
	NET_OUT_ACCESS_FINGERPRINT_SERVICE_REMOVE stuFingerPrintRemoveOut = {sizeof(stuFingerPrintRemoveOut)};
	stuFingerPrintRemoveOut.nMaxRetNum = 1;
	NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
	stuFingerPrintRemoveOut.pFailCode = &stuFailCode;

	BOOL bret = Device::GetInstance().FingerprintRemove(&stuFingerPrintRemoveIn,stuFingerPrintRemoveOut);
	if (!bret)
	{
		if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
	}
	else
	{
		OnBnClickedButFingerprintGet();
	}
	FingerPrintUIState(TRUE);
}

void UserInfo::OnNMClickListctrlFingerprint(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pNMHDR)
	{
		HD_NOTIFY *phdn = (HD_NOTIFY*)pNMHDR;
		m_nFingerprintIndex = phdn->iItem;
	}
	*pResult = 0;
}

void UserInfo::OnBnClickedButCardGet()
{
	CardUIState(FALSE);
	m_ThreadHandle = CreateThread(NULL, 0, DoFindCardInfo, this, 0, (LPDWORD)&m_dwThreadID);
}

void UserInfo::DoFindNextCardRecord()
{
	BOOL bCardFind = Device::GetInstance().CardStartFind();
	if (bCardFind)
	{
		m_CardInfoVector.clear();
		//
		int nStartNo = 0;
		m_bIsDoFindNextCard = TRUE;
		while (m_bIsDoFindNextCard)
		{
			NET_ACCESS_CARD_INFO* pCardInfo = new NET_ACCESS_CARD_INFO[10];
			if (pCardInfo)
			{
				int nRecordNum = 0;
				m_bIsDoFindNextCard = Device::GetInstance().CardFindNext(nStartNo,10,pCardInfo,nRecordNum);

				for (int i=0;i<nRecordNum;i++)
				{
					if (strcmp(m_stuUserInfo.szUserID,pCardInfo[i].szUserID) == 0)
					{
						NET_ACCESS_CARD_INFO stuCardInfo;
						memset(&stuCardInfo,0,sizeof(NET_ACCESS_CARD_INFO));
						memcpy(&stuCardInfo,&pCardInfo[i],sizeof(NET_ACCESS_CARD_INFO));
						m_CardInfoVector.push_back(stuCardInfo);
					}
				}
				nStartNo += nRecordNum;

				delete []pCardInfo;
				pCardInfo = NULL;
			}
			else
			{
				m_bIsDoFindNextCard = FALSE;
			}
		}

		bCardFind = Device::GetInstance().UserStopFind();
		//CardListReload();
		::PostMessage(this->GetSafeHwnd(), WM_UI_USER_CTRL_ENABLE, NULL, NULL);
	}
	CloseHandle(m_ThreadHandle);
	m_ThreadHandle = 0;
	//CardUIState(TRUE);
	::PostMessage(this->GetSafeHwnd(), WM_UI_USER_LIST_LOAD, NULL, NULL);
}

void UserInfo::CardListReload()
{
	if (m_listCard)
	{
		m_listCard.SetRedraw(FALSE);
		m_listCard.DeleteAllItems();
		for (int i = 0; i < m_CardInfoVector.size(); i++)
		{
			NET_ACCESS_CARD_INFO stuCardInfo;
			memset(&stuCardInfo,0,sizeof(NET_ACCESS_CARD_INFO));
			memcpy(&stuCardInfo,&m_CardInfoVector[i],sizeof(NET_ACCESS_CARD_INFO));

			CString strIndex;
			strIndex.Format("%d",i + 1);
			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = i;
			m_listCard.InsertItem(&lvi);
			m_listCard.SetItemText(i, 0, strIndex);
			std::string dstr = CardType2Str(stuCardInfo.emType);
			m_listCard.SetItemText(i, 1, dstr.c_str());
			CString str;
			str.Format("%s",stuCardInfo.szCardNo);
			m_listCard.SetItemText(i, 2, str);
		}
		m_listCard.SetRedraw(TRUE);
	}
}

void UserInfo::CardUIState(BOOL b)
{
	if (CWnd::GetSafeHwnd() )
	{
		GetDlgItem(IDC_BUT_CARD_GET)->EnableWindow(b);
		GetDlgItem(IDC_BUT_CARD_ADD)->EnableWindow(b);
		GetDlgItem(IDC_BUT_CARD_MODIFY)->EnableWindow(b);
		GetDlgItem(IDC_BUT_CARD_DELETE)->EnableWindow(b);
	}
}

void UserInfo::OnBnClickedButCardAdd()
{
	if (m_CardInfoVector.size() >= 5)
	{
		MessageBox(ConvertString("The cards are full"), ConvertString("Prompt"));
		return;
	}
	CardUIState(FALSE);
	UserInfoGetCard dlg(this);
	dlg.SetDlgMode(1);
	if (IDOK == dlg.DoModal())
	{
		const NET_ACCESS_CARD_INFO* pstuCardInfo = dlg.GetCardInfo();
		if (pstuCardInfo != NULL)
		{
			NET_ACCESS_CARD_INFO stuCardInfo;
			memset(&stuCardInfo,0,sizeof(stuCardInfo));
			memcpy(&stuCardInfo, pstuCardInfo, sizeof(stuCardInfo));
			memcpy(stuCardInfo.szUserID,m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
			NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
			BOOL bret = Device::GetInstance().CardInsert(1,&stuCardInfo,stuFailCode);
			if (!bret)
			{
				if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
				{
					CString csInfo;
					csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
					MessageBox(csInfo, ConvertString("Prompt"));
				}
				CardUIState(TRUE);
			}
			else
			{
				OnBnClickedButCardGet();
			}
		}
		else
		{
			CardUIState(TRUE);
		}
	}
	else
	{
		CardUIState(TRUE);
	}
}

void UserInfo::OnBnClickedButCardModify()
{
	if (m_nCardIndex < 0 || m_nCardIndex >= m_CardInfoVector.size())
	{
		return;
	}
	CardUIState(FALSE);
	UserInfoGetCard dlg(this);
	dlg.SetDlgMode(2);
	dlg.SetCardInfo(&m_CardInfoVector[m_nCardIndex]);
	if (IDOK == dlg.DoModal())
	{
		const NET_ACCESS_CARD_INFO* pstuCardInfo = dlg.GetCardInfo();
		if (pstuCardInfo != NULL)
		{
			NET_ACCESS_CARD_INFO stuCardInfo;
			memset(&stuCardInfo,0,sizeof(stuCardInfo));
			memcpy(&stuCardInfo, pstuCardInfo, sizeof(stuCardInfo));
			memcpy(stuCardInfo.szUserID,m_stuUserInfo.szUserID, sizeof(m_stuUserInfo.szUserID));
			NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
			BOOL bret = Device::GetInstance().CardUpdate(1,&stuCardInfo,stuFailCode);
			if (!bret)
			{
				if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
				{
					CString csInfo;
					csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
					MessageBox(csInfo, ConvertString("Prompt"));
				}
				CardUIState(TRUE);
			}
			else
			{
				//刷新列表
				OnBnClickedButCardGet();
			}
		}
		else
		{
			CardUIState(TRUE);
		}
	}
	else
	{
		CardUIState(TRUE);
	}
}

void UserInfo::OnBnClickedButCardDelete()
{
	if (m_nCardIndex < 0 || m_nCardIndex >= m_CardInfoVector.size())
	{
		return;
	}
	CardUIState(FALSE);
	NET_IN_ACCESS_CARD_SERVICE_REMOVE stuCardRemoveIn = {sizeof(stuCardRemoveIn)};
	stuCardRemoveIn.nCardNum = 1;
	NET_ACCESS_CARD_INFO stuCardInfo = m_CardInfoVector[m_nCardIndex];
	memcpy(&stuCardRemoveIn.szCardNo[0], stuCardInfo.szCardNo, sizeof(stuCardInfo.szCardNo));
	NET_OUT_ACCESS_CARD_SERVICE_REMOVE stuCardRemoveOut = {sizeof(stuCardRemoveOut)};
	stuCardRemoveOut.nMaxRetNum = 1;
	NET_EM_FAILCODE stuFailCode = NET_EM_FAILCODE_NOERROR;
	stuCardRemoveOut.pFailCode = &stuFailCode;
	BOOL bret = Device::GetInstance().CardRemove(&stuCardRemoveIn, stuCardRemoveOut);
	if (!bret)
	{
		if(stuFailCode != NET_EM_FAILCODE_UNKNOWN && stuFailCode != NET_EM_FAILCODE_NOERROR)
		{
			CString csInfo;
			csInfo.Format("%s", FailCodeType2Str(stuFailCode).c_str());	
			MessageBox(csInfo, ConvertString("Prompt"));
		}
		CardUIState(TRUE);
	}
	else
	{
		OnBnClickedButCardGet();
	}
}

void UserInfo::OnNMClickListctrlCard(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (pNMHDR)
	{
		HD_NOTIFY *phdn = (HD_NOTIFY*)pNMHDR;
		m_nCardIndex = phdn->iItem;
	}
	*pResult = 0;
}

void UserInfo::OnBnClickedInfoUserinfoBtnDoors()
{
	std::vector<int> vecChn;
	int i = 0;
	for (; i < __min(m_stuUserInfo.nDoorNum, DH_MAX_DOOR_NUM); i++)
	{
		vecChn.push_back(m_stuUserInfo.nDoors[i]);
	}
	int m_nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(m_nAccessGroup);
	if (bret && m_nAccessGroup > 0)
	{
		DlgSensorInfoDescription dlg(this, m_nAccessGroup);
		dlg.SetID(vecChn);
		if (IDOK == dlg.DoModal())
		{
			if (m_nDlgMode == 1 || m_nDlgMode == 2)
			{
				vecChn.clear();
				vecChn = dlg.GetID();
				vecDoor.clear();
				vecDoor = dlg.GetID();
			}
		}
	}
}

LRESULT UserInfo::UserListLoad(WPARAM wParam, LPARAM lParam)
{
	CardListReload();
	return 0;
}
LRESULT UserInfo::UserCtrlEnable(WPARAM wParam, LPARAM lParam)
{
	CardUIState(TRUE);
	return 0;
}