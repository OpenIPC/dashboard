// Card.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "AccessControl1sDlg.h"
#include "Card.h"
#include "DlgCfgDoorOpenTimeSection.h"
#include "DlgSensorInfoDescription.h"

IMPLEMENT_DYNAMIC(CCard, CDialog)

CCard::CCard(CWnd* pParent /*=NULL*/)
	: CDialog(CCard::IDD, pParent)
{
	hParentWnd = pParent->GetSafeHwnd();
	vecDoor.clear();
	vecTimeSection.clear();

	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;
	for (int i=0;i<2;i++)
	{
		memset(m_byFingerprintData[i], 0, sizeof(m_byFingerprintData[i]));
		m_nFingerprintLen[i] = 0;
	}
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);
}

CCard::~CCard()
{
}

void CCard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COM_CARD_EXECUTETYPE, m_cmbCtlType);
	DDX_Control(pDX, IDC_RECSET_CARD_DTP_CT_DATE, m_dtpCreateTimeDate);
	DDX_Control(pDX, IDC_RECSET_CARD_DTP_CT_TIME, m_dtpCreateTimeTime);
	DDX_Control(pDX, IDC_RECSET_CARD_DTP_VDSTART, m_dtpVDStart);
	DDX_Control(pDX, IDC_RECSET_CARD_DTP_VTSTART, m_dtpVTStart);
	DDX_Control(pDX, IDC_RECSET_CARD_DTP_VDEND, m_dtpVDEnd);
	DDX_Control(pDX, IDC_RECSET_CARD_DTP_VTEND, m_dtpVTEnd);
	DDX_Control(pDX, IDC_RECSET_CARD_CHK_FIRSTENTER, m_chkFirstEnter);
	DDX_Control(pDX, IDC_RECSET_CARD_CMB_CARDTYPE, m_cmbCardType);
	DDX_Control(pDX, IDC_RECSET_CARD_CMB_CARDSTATUS, m_cmbCardStatus);
	DDX_Control(pDX, IDC_RECSET_CARD_CMB_FINGERPRINTNO, m_cmbFingerPrintNO);
}

BEGIN_MESSAGE_MAP(CCard, CDialog)
	ON_BN_CLICKED(IDC_BUT_CARD_EXECUTE, &CCard::OnBnClickedButCardExecute)
	ON_BN_CLICKED(IDC_RECSET_CARD_BTN_DOORS, &CCard::OnBnClickedRecsetCardBtnDoors)
	ON_BN_CLICKED(IDC_RECSET_CARD_BTN_TM, &CCard::OnBnClickedRecsetCardBtnTm)
	ON_CBN_SELCHANGE(IDC_COM_CARD_EXECUTETYPE, &CCard::OnCbnSelchangeComCardExecutetype)
	ON_BN_CLICKED(IDC_RECSET_FINGERPRINT_BTN_GET, &CCard::OnBnClickedRecsetFingerprintBtnGet)
	ON_MESSAGE(WM_ACCESS_FINGERPRINT, &CCard::OnFingerPrintMessage)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUT_CARD_GET, &CCard::OnBnClickedButCardGet)
END_MESSAGE_MAP()

BOOL CCard::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);

	unsigned int i = 0;
	InitDlg();
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(FALSE);
	}
	OnCbnSelchangeComCardExecutetype();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCard::OnBnClickedButCardExecute()
{
	int nCtlType = m_cmbCtlType.GetCurSel();
	if (-1 == nCtlType)
	{
		return;
	}

	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);

	if (Em_Operate_Type_Insert == nCtlType + 1)
	{
		CardInsert(FALSE);
	}
	else if (Em_Operate_Type_Get == nCtlType + 1)
	{
		CardGet();
	}
	else if (Em_Operate_Type_Update == nCtlType + 1)
	{
		CardUpdate(FALSE);
		int nCtlType = m_cmbCtlType.GetCurSel();
		OnChangeUIState(nCtlType);
	}
	else if (Em_Operate_Type_Remove == nCtlType + 1)
	{
		CardRemove();
	}
	else if (Em_Operate_Type_Clear == nCtlType + 1)
	{
		CardClear();
	}
	else if (Em_Operate_Type_InsertEX == nCtlType + 1)
	{
		CardInsert(TRUE);
	}
	else if (Em_Operate_Type_UpdateEX == nCtlType + 1)
	{
		CardUpdate(TRUE);
		int nCtlType = m_cmbCtlType.GetCurSel();
		OnChangeUIState(nCtlType);
	}
}

void CCard::InitDlg()
{
	int i = 0;

	// Card Status
	m_cmbCardStatus.ResetContent();
	for (i = 0; i < sizeof(stuDemoCardStatus)/sizeof(stuDemoCardStatus[0]); i++)
	{
		m_cmbCardStatus.InsertString(-1, ConvertString(stuDemoCardStatus[i].szName));
	}

	// Card Type
	m_cmbCardType.ResetContent();
	for (i = 0; i < sizeof(stuDemoCardType)/sizeof(stuDemoCardType[0]); i++)
	{
		m_cmbCardType.InsertString(-1, ConvertString(stuDemoCardType[i].szName));
	}

	m_cmbCtlType.ResetContent();
	for ( i = 0; i < sizeof(stuDemoCardSetCtlType)/sizeof(stuDemoCardSetCtlType[0]); i++)
	{
		m_cmbCtlType.InsertString(-1, ConvertString(stuDemoCardSetCtlType[i].szName));
	}
	m_cmbCtlType.SetCurSel(0);

	m_cmbFingerPrintNO.ResetContent();
	m_cmbFingerPrintNO.InsertString(-1, ConvertString("FingerPrint1"));
	m_cmbFingerPrintNO.InsertString(-1, ConvertString("FingerPrint2"));
	m_cmbFingerPrintNO.SetCurSel(0);
}

void CCard::StuToDlg()
{
	// RecNo
	SetDlgItemInt(IDC_RECSET_CARD_EDT_RECNO, m_stuInfo.nRecNo);

	// create time
	SYSTEMTIME st = {0};
	st.wYear	= (WORD)m_stuInfo.stuCreateTime.dwYear;
	st.wMonth	= (WORD)m_stuInfo.stuCreateTime.dwMonth;
	st.wDay		= (WORD)m_stuInfo.stuCreateTime.dwDay;
	m_dtpCreateTimeDate.SetTime(&st);
	st.wHour	= (WORD)m_stuInfo.stuCreateTime.dwHour;
	st.wMinute	= (WORD)m_stuInfo.stuCreateTime.dwMinute;
	st.wSecond	= (WORD)m_stuInfo.stuCreateTime.dwSecond;
	m_dtpCreateTimeTime.SetTime(&st);

	// card no
	SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO, m_stuInfo.szCardNo);

	// user id
	SetDlgItemText(IDC_RECSET_CARD_EDT_USERID, m_stuInfo.szUserID);

	// card status
	m_cmbCardStatus.SetCurSel(CardStatusToInt(m_stuInfo.emStatus));

	// card type
	m_cmbCardType.SetCurSel(CardTypeToInt(m_stuInfo.emType));

	// password
	SetDlgItemText(IDC_RECSET_CARD_EDT_PWD, m_stuInfo.szPsw);

	// use time
	SetDlgItemInt(IDC_RECSET_CARD_EDT_USETIME, m_stuInfo.nUserTime, TRUE);

	// valid time start
	st.wYear	= (WORD)m_stuInfo.stuValidStartTime.dwYear;
	st.wMonth	= (WORD)m_stuInfo.stuValidStartTime.dwMonth;
	st.wDay		= (WORD)m_stuInfo.stuValidStartTime.dwDay;
	m_dtpVDStart.SetTime(&st);
	st.wHour    = (WORD)m_stuInfo.stuValidStartTime.dwHour;
	st.wMinute  = (WORD)m_stuInfo.stuValidStartTime.dwMinute;
	st.wSecond  = (WORD)m_stuInfo.stuValidStartTime.dwSecond;
	m_dtpVTStart.SetTime(&st);

	// valid time end
	st.wYear	= (WORD)m_stuInfo.stuValidEndTime.dwYear;
	st.wMonth	= (WORD)m_stuInfo.stuValidEndTime.dwMonth;
	st.wDay		= (WORD)m_stuInfo.stuValidEndTime.dwDay;
	m_dtpVDEnd.SetTime(&st);
	st.wHour    = (WORD)m_stuInfo.stuValidEndTime.dwHour;
	st.wMinute  = (WORD)m_stuInfo.stuValidEndTime.dwMinute;
	st.wSecond  = (WORD)m_stuInfo.stuValidEndTime.dwSecond;
	m_dtpVTEnd.SetTime(&st);

	// first enter
	if (m_stuInfo.bFirstEnter)
	{
		m_chkFirstEnter.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkFirstEnter.SetCheck(BST_UNCHECKED);
	}

	//door
	vecDoor.clear();
	for (int i = 0; i < m_stuInfo.nDoorNum; i++)
	{
		vecDoor.push_back(m_stuInfo.sznDoors[i]);
	}
	//TimeSection
	vecTimeSection.clear();
	for (int i = 0; i < m_stuInfo.nTimeSectionNum; i++)
	{
		vecTimeSection.push_back(m_stuInfo.sznTimeSectionNo[i]);
	}

	//Fingerprint
	for (int i=0;i<2;i++)
	{
		m_nFingerprintLen[i] = 0;	
		memset(&m_byFingerprintData[i], 0, sizeof(m_byFingerprintData[i]));
	}
	for (int i=0;i<m_stuInfo.stuFingerPrintInfoEx.nCount;i++)
	{
		m_nFingerprintLen[i] = m_stuInfo.stuFingerPrintInfoEx.nLength;
		memcpy(m_byFingerprintData[i], m_stuInfo.stuFingerPrintInfoEx.pPacketData + m_stuInfo.stuFingerPrintInfoEx.nLength * i, m_stuInfo.stuFingerPrintInfoEx.nLength);
	}

}

void CCard::DlgToStu()
{
	// RecNo
	m_stuInfo.nRecNo = GetDlgItemInt(IDC_RECSET_CARD_EDT_RECNO);

	// create time
	SYSTEMTIME st = {0};
	m_dtpCreateTimeDate.GetTime(&st);
	m_stuInfo.stuCreateTime.dwYear	= st.wYear;
	m_stuInfo.stuCreateTime.dwMonth = st.wMonth;
	m_stuInfo.stuCreateTime.dwDay	= st.wDay;
	m_dtpCreateTimeTime.GetTime(&st);
	m_stuInfo.stuCreateTime.dwHour	= st.wHour;
	m_stuInfo.stuCreateTime.dwMinute = st.wMinute;
	m_stuInfo.stuCreateTime.dwSecond = st.wSecond;

	// card no
	GetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO, m_stuInfo.szCardNo, sizeof(m_stuInfo.szCardNo));

	// user id
	GetDlgItemText(IDC_RECSET_CARD_EDT_USERID, m_stuInfo.szUserID, sizeof(m_stuInfo.szUserID));

	// card status
	m_stuInfo.emStatus = IntToCardStatus(m_cmbCardStatus.GetCurSel());

	// card type
	m_stuInfo.emType = IntToCardType(m_cmbCardType.GetCurSel());

	// password
	GetDlgItemText(IDC_RECSET_CARD_EDT_PWD, m_stuInfo.szPsw, sizeof(m_stuInfo.szPsw));

	// use time
	m_stuInfo.nUserTime = GetDlgItemInt(IDC_RECSET_CARD_EDT_USETIME, NULL, TRUE);

	// valid time start
	m_dtpVDStart.GetTime(&st);
	m_stuInfo.stuValidStartTime.dwYear = st.wYear;
	m_stuInfo.stuValidStartTime.dwMonth = st.wMonth;
	m_stuInfo.stuValidStartTime.dwDay = st.wDay;
	m_dtpVTStart.GetTime(&st);
	m_stuInfo.stuValidStartTime.dwHour = st.wHour;
	m_stuInfo.stuValidStartTime.dwMinute = st.wMinute;
	m_stuInfo.stuValidStartTime.dwSecond = st.wSecond;

	// valid time end
	m_dtpVDEnd.GetTime(&st);
	m_stuInfo.stuValidEndTime.dwYear = st.wYear;
	m_stuInfo.stuValidEndTime.dwMonth = st.wMonth;
	m_stuInfo.stuValidEndTime.dwDay = st.wDay;
	m_dtpVTEnd.GetTime(&st);
	m_stuInfo.stuValidEndTime.dwHour = st.wHour;
	m_stuInfo.stuValidEndTime.dwMinute = st.wMinute;
	m_stuInfo.stuValidEndTime.dwSecond = st.wSecond;

	// first enter
	if (m_chkFirstEnter.GetCheck())
	{
		m_stuInfo.bFirstEnter = TRUE;
	}
	else
	{
		m_stuInfo.bFirstEnter = FALSE;
	}

	//Door
	std::vector<int>::iterator itDoor = vecDoor.begin();
	for (int i = 0; i < __min(vecDoor.size(), DH_MAX_DOOR_NUM) && itDoor != vecDoor.end(); i++, itDoor++)
	{
		m_stuInfo.sznDoors[i] = *itDoor;
	}
	m_stuInfo.nDoorNum = __min(vecDoor.size(), DH_MAX_DOOR_NUM);

	//TimeSection
	std::vector<int>::iterator itTimerSection = vecTimeSection.begin();
	for (int i = 0; i < __min(vecTimeSection.size(), DH_MAX_TIMESECTION_NUM) && itTimerSection != vecTimeSection.end(); i++, itTimerSection++)
	{
		m_stuInfo.sznTimeSectionNo[i] = *itTimerSection;
	}
	m_stuInfo.nTimeSectionNum = __min(vecTimeSection.size(), DH_MAX_TIMESECTION_NUM);

	int nCtlType = m_cmbCtlType.GetCurSel();
	if (Em_Operate_Type_InsertEX == (nCtlType + 1) || Em_Operate_Type_UpdateEX == (nCtlType + 1))
	{
		int nLenAll = 0;
		int nFingerPrintNum = 0;
		for (int i=0;i<2;i++)
		{
			if (m_nFingerprintLen[i] > 0)
			{
				nFingerPrintNum++;
				nLenAll = nLenAll + m_nFingerprintLen[i];
			}
		}
		m_stuInfo.bEnableExtended = true;
		m_stuInfo.stuFingerPrintInfoEx.nCount = nFingerPrintNum;
		if (nFingerPrintNum == 0)
		{
			m_stuInfo.stuFingerPrintInfoEx.nLength = 0;
		} 
		else
		{
			m_stuInfo.stuFingerPrintInfoEx.nLength = nLenAll/nFingerPrintNum /*m_nFingerprintLen[0]*/;
		}
		m_stuInfo.stuFingerPrintInfoEx.nPacketLen = nLenAll;
		m_stuInfo.stuFingerPrintInfoEx.pPacketData = new char[nLenAll];
		memset(m_stuInfo.stuFingerPrintInfoEx.pPacketData, 0, nLenAll);
		memcpy(m_stuInfo.stuFingerPrintInfoEx.pPacketData, m_byFingerprintData[0], m_nFingerprintLen[0]);
		memcpy(m_stuInfo.stuFingerPrintInfoEx.pPacketData + m_nFingerprintLen[0], m_byFingerprintData[1], m_nFingerprintLen[1]);
	}
	for (int i=0;i<2;i++)
	{
		m_nFingerprintLen[i] = 0;	
		memset(&m_byFingerprintData[i], 0, sizeof(m_byFingerprintData[i]));
	}
}


int CCard::CardStatusToInt(NET_ACCESSCTLCARD_STATE emStatus)
{
	int n = -1;
	if (NET_ACCESSCTLCARD_STATE_UNKNOWN == emStatus)
	{
		n = 0;
	}
	else if (NET_ACCESSCTLCARD_STATE_NORMAL == emStatus)
	{
		n = 1;
	}
	else if (NET_ACCESSCTLCARD_STATE_LOSE == emStatus)
	{
		n = 2;
	}
	else if (NET_ACCESSCTLCARD_STATE_LOGOFF == emStatus)
	{
		n = 3;
	}
	else if (NET_ACCESSCTLCARD_STATE_FREEZE == emStatus)
	{
		n = 4;
	}
	return n;
}

int CCard::CardTypeToInt(NET_ACCESSCTLCARD_TYPE emType)
{
	int n = -1;
	if (NET_ACCESSCTLCARD_TYPE_UNKNOWN == emType)
	{
		n = 0;
	}
	else if (NET_ACCESSCTLCARD_TYPE_GENERAL == emType)
	{
		n = 1;
	}
	else if (NET_ACCESSCTLCARD_TYPE_VIP == emType)
	{
		n = 2;
	}
	else if (NET_ACCESSCTLCARD_TYPE_GUEST == emType)
	{
		n = 3;
	}
	else if (NET_ACCESSCTLCARD_TYPE_PATROL == emType)
	{
		n = 4;
	}
	else if (NET_ACCESSCTLCARD_TYPE_BLACKLIST == emType)
	{
		n = 5;
	}
	else if (NET_ACCESSCTLCARD_TYPE_CORCE == emType)
	{
		n = 6;
	}
	else if (NET_ACCESSCTLCARD_TYPE_MOTHERCARD == emType)
	{
		n = 7;
	}
	return n;
}

NET_ACCESSCTLCARD_STATE CCard::IntToCardStatus(int n)
{
	NET_ACCESSCTLCARD_STATE emStatua = NET_ACCESSCTLCARD_STATE_UNKNOWN;
	if (0 == n)
	{
	}
	else if (1 == n)
	{
		emStatua = NET_ACCESSCTLCARD_STATE_NORMAL;
	}
	else if (2 == n)
	{
		emStatua = NET_ACCESSCTLCARD_STATE_LOSE;
	}
	else if (3 == n)
	{
		emStatua = NET_ACCESSCTLCARD_STATE_LOGOFF;
	}
	else if (4 == n)
	{
		emStatua = NET_ACCESSCTLCARD_STATE_FREEZE;
	}
	return emStatua;
}

NET_ACCESSCTLCARD_TYPE CCard::IntToCardType(int n)
{
	NET_ACCESSCTLCARD_TYPE emType = NET_ACCESSCTLCARD_TYPE_UNKNOWN;
	if (0 == n)
	{
	}
	else if (1 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_GENERAL;
	}
	else if (2 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_VIP;
	}
	else if (3 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_GUEST;
	}
	else if (4 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_PATROL;
	}
	else if (5 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_BLACKLIST;
	}
	else if (6 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_CORCE;
	}
	else if (7 == n)
	{
		emType = NET_ACCESSCTLCARD_TYPE_MOTHERCARD;
	}
	return emType;
}

void CCard::CardInsert(BOOL bEx)
{
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);
	DlgToStu();
	int nRetNo = 0;
	BOOL bret = Device::GetInstance().InsertCardRecorde(&m_stuInfo, bEx, nRetNo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s RetNo=%d", ConvertString("Execute successfully."), nRetNo);
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	if (bEx)
	{
		if(m_stuInfo.stuFingerPrintInfoEx.pPacketData)
		{
			delete []m_stuInfo.stuFingerPrintInfoEx.pPacketData;
			m_stuInfo.stuFingerPrintInfoEx.pPacketData = NULL;
		}
	}
}

void CCard::CardGet()
{
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);
	DlgToStu();
	m_stuInfo.bEnableExtended = TRUE;
	m_stuInfo.stuFingerPrintInfoEx.pPacketData = new char[820*2];
	m_stuInfo.stuFingerPrintInfoEx.nPacketLen = 820 * 2;
	
	BOOL bret = Device::GetInstance().GetCardRecorde(m_stuInfo);
	if (bret)
	{
		StuToDlg();
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	if(m_stuInfo.stuFingerPrintInfoEx.pPacketData)
	{
		delete []m_stuInfo.stuFingerPrintInfoEx.pPacketData;
		m_stuInfo.stuFingerPrintInfoEx.pPacketData = NULL;
	}
}

void CCard::CardUpdate(BOOL bEx)
{
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);
	DlgToStu();
	int nRetNo = 0;
	BOOL bret = Device::GetInstance().UpdateCardRecorde(&m_stuInfo, bEx, nRetNo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s RetNo=%d", ConvertString("Execute successfully."), nRetNo);
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	if (bEx)
	{
		if(m_stuInfo.stuFingerPrintInfoEx.pPacketData)
		{
			delete []m_stuInfo.stuFingerPrintInfoEx.pPacketData;
			m_stuInfo.stuFingerPrintInfoEx.pPacketData = NULL;
		}
	}
}

void CCard::CardRemove()
{
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);
	DlgToStu();
	BOOL bret = Device::GetInstance().RemoveCardRecorde(&m_stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void CCard::CardClear()
{
	BOOL bret = Device::GetInstance().ClearCardRecorde();
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void CCard::OnBnClickedRecsetCardBtnDoors()
{
	std::vector<int> vecChn;
	int i = 0;
	for (; i < m_stuInfo.nDoorNum; i++)
	{
		vecChn.push_back(m_stuInfo.sznDoors[i]);
	}
	int m_emOperateType = m_cmbCtlType.GetCurSel() + 1;
	int m_nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(m_nAccessGroup);
	DlgSensorInfoDescription dlg(this, m_nAccessGroup);
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		if (Em_Operate_Type_Insert == m_emOperateType
			|| Em_Operate_Type_Update == m_emOperateType
			|| Em_Operate_Type_InsertEX == m_emOperateType
			|| Em_Operate_Type_UpdateEX == m_emOperateType)
		{
			vecChn.clear();
			vecChn = dlg.GetID();
			vecDoor.clear();
			vecDoor = dlg.GetID();
		}
	}
}

void CCard::OnBnClickedRecsetCardBtnTm()
{
	std::vector<int> vecChn;
	int i = 0;
	for (; i < m_stuInfo.nTimeSectionNum; i++)
	{
		vecChn.push_back(m_stuInfo.sznTimeSectionNo[i]);
	}
	int m_emOperateType = m_cmbCtlType.GetCurSel() + 1;
	DlgSensorInfoDescription dlg(this, 128);
	dlg.SetID(vecChn);
	if (IDOK == dlg.DoModal())
	{
		if (Em_Operate_Type_Insert == m_emOperateType
			|| Em_Operate_Type_Update == m_emOperateType
			|| Em_Operate_Type_InsertEX == m_emOperateType
			|| Em_Operate_Type_UpdateEX == m_emOperateType)
		{
			vecChn.clear();
			vecChn = dlg.GetID();
			vecTimeSection.clear();
			vecTimeSection = dlg.GetID();
		}
	}
}

void CCard::OnCbnSelchangeComCardExecutetype()
{
	int nCtlType = m_cmbCtlType.GetCurSel();
	OnChangeUIState(nCtlType);
}

void CCard::OnChangeUIState(int nState)
{
	GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_DTP_CT_DATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECSET_CARD_DTP_CT_TIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_RECSET_CARD_EDT_CARDNO)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_EDT_USERID)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_CMB_CARDSTATUS)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_CMB_CARDTYPE)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_EDT_PWD)->EnableWindow(TRUE);

	GetDlgItem(IDC_RECSET_CARD_BTN_DOORS)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_BTN_TM)->EnableWindow(TRUE);

	GetDlgItem(IDC_RECSET_CARD_EDT_USETIME)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_DTP_VDSTART)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_DTP_VTSTART)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_DTP_VDEND)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_DTP_VTEND)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_CHK_FIRSTENTER)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(TRUE);
	GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(TRUE);
	
	GetDlgItem(IDC_BUT_CARD_GET)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUT_CARD_GET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(TRUE);
	//int nCtlType = m_cmbCtlType.GetCurSel();
	switch(nState)
	{
	case 0://Insert
		{
			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);

			GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->SetWindowText(ConvertString("Insert"));

			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USERID,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_PWD,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USETIME,"");
			m_chkFirstEnter.SetCheck(BST_UNCHECKED);
			m_cmbCardStatus.SetCurSel(-1);
			m_cmbCardType.SetCurSel(-1);
		}
		break;
	case 5://InsertEX
		{
			GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->SetWindowText(ConvertString("Insert"));

			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USERID,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_PWD,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USETIME,"");
			m_chkFirstEnter.SetCheck(BST_UNCHECKED);
			m_cmbCardStatus.SetCurSel(-1);
			m_cmbCardType.SetCurSel(-1);
		}
		break;
	case 2://Update  Step1
	case 6://UpdateEX Step1
		{
			GetDlgItem(IDC_BUT_CARD_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(FALSE);
		/*	GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");*/
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->SetWindowText(ConvertString("Update"));

			GetDlgItem(IDC_RECSET_CARD_EDT_CARDNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_USERID)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_CARDSTATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_CARDTYPE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_PWD)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_BTN_DOORS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_BTN_TM)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_USETIME)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VDSTART)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VTSTART)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VDEND)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VTEND)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CHK_FIRSTENTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);

			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USERID,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_PWD,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USETIME,"");
			m_chkFirstEnter.SetCheck(BST_UNCHECKED);
			m_cmbCardStatus.SetCurSel(-1);
			m_cmbCardType.SetCurSel(-1);
		}
		break;
	case 1://Get
		{
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->SetWindowText(ConvertString("Get"));

			GetDlgItem(IDC_RECSET_CARD_EDT_CARDNO)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECSET_CARD_EDT_USERID)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECSET_CARD_CMB_CARDSTATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_CARDTYPE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_PWD)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_USETIME)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VDSTART)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VTSTART)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VDEND)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VTEND)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CHK_FIRSTENTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);

			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USERID,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_PWD,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USETIME,"");
			m_chkFirstEnter.SetCheck(BST_UNCHECKED);
			m_cmbCardStatus.SetCurSel(-1);
			m_cmbCardType.SetCurSel(-1);
		}
		break;
	case 3://Remove
		{
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->SetWindowText(ConvertString("Remove"));
 			GetDlgItem(IDC_RECSET_CARD_EDT_CARDNO)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_EDT_USERID)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_CMB_CARDSTATUS)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_CMB_CARDTYPE)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_EDT_PWD)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_BTN_DOORS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_BTN_TM)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_EDT_USETIME)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_DTP_VDSTART)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_DTP_VTSTART)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_DTP_VDEND)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_DTP_VTEND)->EnableWindow(FALSE);
 			GetDlgItem(IDC_RECSET_CARD_CHK_FIRSTENTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);

			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USERID,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_PWD,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USETIME,"");
			m_chkFirstEnter.SetCheck(BST_UNCHECKED);
			m_cmbCardStatus.SetCurSel(-1);
			m_cmbCardType.SetCurSel(-1);
		}
		break;
	case 4://Clear
		{
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->SetWindowText(ConvertString("Clear"));

			GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_CARDNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_USERID)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_CARDSTATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_CARDTYPE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_PWD)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_BTN_DOORS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_BTN_TM)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_EDT_USETIME)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VDSTART)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VTSTART)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VDEND)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_DTP_VTEND)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CHK_FIRSTENTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);
			SetDlgItemText(IDC_RECSET_CARD_EDT_RECNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_CARDNO,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USERID,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_PWD,"");
			SetDlgItemText(IDC_RECSET_CARD_EDT_USETIME,"");
			m_chkFirstEnter.SetCheck(BST_UNCHECKED);
			m_cmbCardStatus.SetCurSel(-1);
			m_cmbCardType.SetCurSel(-1);
		}
		break;
	case 7://Update  Step2
		{
			GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(FALSE);

			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);

			GetDlgItem(IDC_BUT_CARD_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_CARD_GET)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(TRUE);
		}
		break;
	case 8://UpdateEX Step2
		{
			GetDlgItem(IDC_RECSET_CARD_EDT_RECNO)->EnableWindow(FALSE);

			GetDlgItem(IDC_RECSET_CARD_CMB_FINGERPRINTNO)->EnableWindow(TRUE);
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(TRUE);

			GetDlgItem(IDC_BUT_CARD_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_CARD_GET)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(TRUE);
		}
		break;
	default:
		break;
	}
}

void CCard::OnBnClickedRecsetFingerprintBtnGet()
{
	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;
	int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText("");

	::PostMessage(hParentWnd,WM_FINGERPRINT,1,0);
	
	BOOL bret = Device::GetInstance().StartGetFingerPrint();
	if (!bret)
	{
		MessageBox(ConvertString("start capture fingerprint failed"), ConvertString("Prompt"));	
		return ;
	}

	SetTimer(1, 30*1000, NULL);
	m_bStarted = true;
	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Start Collection"));
	GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(FALSE);
}

LRESULT CCard::OnFingerPrintMessage(WPARAM wParam, LPARAM lParam)
{
	ALARM_CAPTURE_FINGER_PRINT_INFO *pstFingerPrint = (ALARM_CAPTURE_FINGER_PRINT_INFO *)wParam;
	if (NULL != pstFingerPrint)
	{
		int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
		int nLen = pstFingerPrint->nPacketNum * pstFingerPrint->nPacketLen;
		if (nLen <= sizeof(m_byFingerprintData[nFingerPrintNo]))
		{	
			KillTimer(1);
			::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
			m_bCaptureSuc = true;
			m_bFinished = true;
			memset(m_byFingerprintData[nFingerPrintNo], 0, sizeof(m_byFingerprintData[nFingerPrintNo]));
			m_nFingerprintLen[nFingerPrintNo] = 0;
			memcpy(m_byFingerprintData[nFingerPrintNo], pstFingerPrint->szFingerPrintInfo, nLen);
			m_nFingerprintLen[nFingerPrintNo] = nLen;
			GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection Completed"));
			GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(TRUE);
		}
		if (NULL != pstFingerPrint->szFingerPrintInfo)
		{
			delete []pstFingerPrint->szFingerPrintInfo;
			pstFingerPrint->szFingerPrintInfo = NULL;
		}
		delete pstFingerPrint;
		pstFingerPrint = NULL;
	}
	else
	{
		KillTimer(1);
		::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
		int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
		m_bCaptureSuc = true;
		m_bFinished = true;
		GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection failed"));
		GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(TRUE);
	}

	return 0;
}

void CCard::OnDestroy()
{
	CDialog::OnDestroy();
	::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
	KillTimer(1);
}

void CCard::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_bCaptureSuc)
	{
		int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
		m_bFinished = true;
		m_nFingerprintLen[nFingerPrintNo] = 0;	
		memset(&m_byFingerprintData[nFingerPrintNo], 0, sizeof(m_byFingerprintData[nFingerPrintNo]));
		GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection failed"));
		GetDlgItem(IDC_RECSET_FINGERPRINT_BTN_GET)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUT_CARD_EXECUTE)->EnableWindow(TRUE);
		KillTimer(1);
		::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
	}

	CDialog::OnTimer(nIDEvent);
}

void CCard::OnBnClickedButCardGet()
{
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.dwSize = sizeof(m_stuInfo);
	DlgToStu();
	m_stuInfo.bEnableExtended = TRUE;
	m_stuInfo.stuFingerPrintInfoEx.pPacketData = new char[820*2];
	m_stuInfo.stuFingerPrintInfoEx.nPacketLen = 820 * 2;
	
	BOOL bret = Device::GetInstance().GetCardRecorde(m_stuInfo);
	if (bret)
	{
		StuToDlg();

		if (m_cmbCtlType.GetCurSel() == 2)
		{
			OnChangeUIState(7);
		} 
		else 
		{
			OnChangeUIState(8);
		}
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	if(m_stuInfo.stuFingerPrintInfoEx.pPacketData)
	{
		delete []m_stuInfo.stuFingerPrintInfoEx.pPacketData;
		m_stuInfo.stuFingerPrintInfoEx.pPacketData = NULL;
	}
}
