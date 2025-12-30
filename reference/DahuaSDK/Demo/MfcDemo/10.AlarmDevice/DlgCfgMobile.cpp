// DlgCfgMobile.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgMobile.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMobile dialog


CDlgCfgMobile::CDlgCfgMobile(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgCfgMobile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgMobile)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgMobile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgMobile)
	DDX_Control(pDX, IDC_MOBILE_CHK_ENABLE, m_chkEnable);
	DDX_Control(pDX, IDC_MOBILE_LIST_RECEIVENUMBER, m_lsNumberList);
	DDX_Control(pDX, IDC_MOBILE_CMB_TYPE, m_cmbType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgMobile, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgMobile)
	ON_BN_CLICKED(IDC_MOBILE_BTN_ADDNUMBER, OnMobileBtnAddnumber)
	ON_BN_CLICKED(IDC_MOBILE_BTN_DELNUMBER, OnMobileBtnDelnumber)
	ON_BN_CLICKED(IDC_MOBILE_BTN_GET, OnMobileBtnGet)
	ON_BN_CLICKED(IDC_MOBILE_BTN_SET, OnMobileBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Private methods

void CDlgCfgMobile::InitDlg(BOOL bShow /* = TRUE */)
{
	m_chkEnable.SetCheck(BST_UNCHECKED);

	for (int i = 0; i < sizeof(stuDemoMsgType) / sizeof(stuDemoMsgType[0]); i++)
	{
		m_cmbType.InsertString(-1, ConvertString(stuDemoMsgType[i].pszName, DLG_CFG_MOBILE));
	}

	SetDlgItemText(IDC_MOBILE_EDT_TITLE, m_stuInfo.stuEventMsgSending.szTitle);

	m_lsNumberList.SetExtendedStyle(m_lsNumberList.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lsNumberList.DeleteAllItems();
	m_lsNumberList.InsertColumn(0, ConvertString("Seq", DLG_CFG_MOBILE), LVCFMT_LEFT, 60);
	m_lsNumberList.InsertColumn(1, ConvertString("Number", DLG_CFG_MOBILE), LVCFMT_LEFT, 160);
}

BOOL CDlgCfgMobile::SetConfigToDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_MOBILE, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet Mobile error..."), DLG_CFG_MOBILE), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif	
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_MOBILE,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRetSetupCfg)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig Mobile failed:", DLG_CFG_MOBILE), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig Mobile ok!"), DLG_CFG_MOBILE), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgMobile::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_MOBILE,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_MOBILE_INFO stuInfo = {0};
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_MOBILE, szJsonBuf, (void*)&stuInfo, sizeof(stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse Mobile error..."), DLG_CFG_MOBILE), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_MOBILE_INFO));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig Mobile error:", DLG_CFG_MOBILE), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgMobile::StuToDlg()
{
	// enable
	m_chkEnable.SetCheck(m_stuInfo.stuEventMsgSending.bEnable ? BST_CHECKED : BST_UNCHECKED);

	// type
	m_cmbType.SetCurSel((int)m_stuInfo.stuEventMsgSending.emMsgType);

	// title
	SetDlgItemText(IDC_MOBILE_EDT_TITLE, m_stuInfo.stuEventMsgSending.szTitle);

	// number list
	m_lsNumberList.DeleteAllItems();
	int nCount = __min(m_stuInfo.stuEventMsgSending.dwReceiverCount, MAX_RECEIVER_NUM);
	for (int i = 0; i < nCount; i++)
	{
		m_lsNumberList.InsertItem(i, "");

		char szSeq[8] = {0};
		_itoa(i, szSeq, 10);
		m_lsNumberList.SetItemText(i, 0, szSeq);
		m_lsNumberList.SetItemText(i, 1, m_stuInfo.stuEventMsgSending.szReceiverNumbersList[i]);
	}
}

void CDlgCfgMobile::DlgToStu()
{
	// enable
	m_stuInfo.stuEventMsgSending.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;
	
	// type
	m_stuInfo.stuEventMsgSending.emMsgType = m_cmbType.GetCurSel() == -1 ? EM_MSG_UNKNOWN : (EM_MSG_TYPE)m_cmbType.GetCurSel();

	// title
	CString strTitle;
	GetDlgItemText(IDC_MOBILE_EDT_TITLE, strTitle);
	memset(m_stuInfo.stuEventMsgSending.szTitle, 0, MAX_NAME_LEN);
	if (strTitle.GetLength())
	{
		strncpy(m_stuInfo.stuEventMsgSending.szTitle, strTitle, __min(strTitle.GetLength(), MAX_NAME_LEN));
	}

	// number list
	int nCount = m_lsNumberList.GetItemCount();
	nCount = __min(nCount, MAX_RECEIVER_NUM);
	m_stuInfo.stuEventMsgSending.dwReceiverCount = nCount;
	memset(m_stuInfo.stuEventMsgSending.szReceiverNumbersList, 0, sizeof(m_stuInfo.stuEventMsgSending.szReceiverNumbersList));
	for (int i = 0; i < nCount; i++)
	{
		CString strNumber = m_lsNumberList.GetItemText(i, 1);
		strncpy(m_stuInfo.stuEventMsgSending.szReceiverNumbersList[i], 
			strNumber, 
			MAX_MSG_NUMBER_LEN - 1);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgMobile message handlers

BOOL CDlgCfgMobile::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_MOBILE);
	
	// TODO: Add extra initialization here	
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_CFG_MOBILE), ConvertString("Prompt"));
		//EndDialog(0);
	}
	
	InitDlg();
	OnMobileBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgMobile::OnMobileBtnAddnumber() 
{
	// TODO: Add your control notification handler code here
	CString csNumber;
	GetDlgItemText(IDC_MOBILE_EDT_NUMBER, csNumber);
	if (!csNumber.GetLength())
	{
		return;
	}
	
	int nCount = m_lsNumberList.GetItemCount();
	m_lsNumberList.InsertItem(nCount, _T(""));
	
	char szSeq[10] = {0};
	_itoa(nCount + 1, szSeq, 10);
	m_lsNumberList.SetItemText(nCount, 0, szSeq);
	m_lsNumberList.SetItemText(nCount, 1, csNumber);	
}

void CDlgCfgMobile::OnMobileBtnDelnumber() 
{
	// TODO: Add your control notification handler code here
	POSITION pItem = m_lsNumberList.GetFirstSelectedItemPosition();
	if (pItem)
	{
		int nSel = m_lsNumberList.GetNextSelectedItem(pItem);
		CString csSelNum = m_lsNumberList.GetItemText(nSel, 1);

		{
			typedef std::list<CString> lsItem;
			lsItem lsNumberItem;
			int nCount = m_lsNumberList.GetItemCount();
			for (int i = 0; i < nCount; i++)
			{
				CString csItem = m_lsNumberList.GetItemText(i, 1);
				lsNumberItem.push_back(csItem);
			}

			lsItem::iterator it = lsNumberItem.begin();
			while (it != lsNumberItem.end())
			{
				CString csNum = *it;
				if (csNum == csSelNum)
				{
					lsNumberItem.erase(it++);
					break;
				}

				it++;
			}

			m_lsNumberList.DeleteAllItems();
			it = lsNumberItem.begin();

			for (int j = 0; j < lsNumberItem.size(); j++)
			{
				CString csItem = *it++;

				m_lsNumberList.InsertItem(j, "");

				char szSeq[8] = {0};
				_itoa(j + 1, szSeq, 10);
				m_lsNumberList.SetItemText(j, 0, szSeq);
				m_lsNumberList.SetItemText(j, 1, csItem);
			}
		}
	}
}

void CDlgCfgMobile::OnMobileBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
}

void CDlgCfgMobile::OnMobileBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfigToDevice();
}
