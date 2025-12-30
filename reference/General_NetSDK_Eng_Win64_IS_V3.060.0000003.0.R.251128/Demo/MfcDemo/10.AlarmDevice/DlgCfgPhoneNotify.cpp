// DlgCfgPhoneNotify.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgPhoneNotify.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPhoneNotify dialog


CDlgCfgPhoneNotify::CDlgCfgPhoneNotify(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgCfgPhoneNotify::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgPhoneNotify)
		// NOTE: the ClassWizard will add member initialization here
	m_lLoginId = lLoginId;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
}


void CDlgCfgPhoneNotify::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgPhoneNotify)
	DDX_Control(pDX, IDC_PHONENOTIFY_LIST_RECEIVENUMBER, m_lsNumberList);
	DDX_Control(pDX, IDC_PHONENOTIFY_CHK_ENABLE, m_chkEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgPhoneNotify, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgPhoneNotify)
	ON_BN_CLICKED(IDC_PHONENOTIFY_BTN_ADDNUMBER, OnPhonenotifyBtnAddnumber)
	ON_BN_CLICKED(IDC_PHONENOTIFY_BTN_DELNUMBER, OnPhonenotifyBtnDelnumber)
	ON_BN_CLICKED(IDC_PHONENOTIFY_BTN_GET, OnPhonenotifyBtnGet)
	ON_BN_CLICKED(IDC_PHONENOTIFY_BTN_SET, OnPhonenotifyBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Private methods

void CDlgCfgPhoneNotify::InitDlg(BOOL bShow /* = TRUE */)
{
	m_chkEnable.SetCheck(BST_UNCHECKED);

	m_lsNumberList.SetExtendedStyle(m_lsNumberList.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lsNumberList.DeleteAllItems();
	m_lsNumberList.InsertColumn(0, ConvertString("Seq", DLG_CFG_PHONENOTIFY), LVCFMT_LEFT, 60);
	m_lsNumberList.InsertColumn(1, ConvertString("Number", DLG_CFG_PHONENOTIFY), LVCFMT_LEFT, 160);
}

BOOL CDlgCfgPhoneNotify::SetConfigToDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_PHONEEVENTNOTIFY, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet PhoneEventNotify error..."), DLG_CFG_PHONENOTIFY), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif	
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_PHONEEVENTNOTIFY,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRetSetupCfg)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig PhoneEventNotify failed:", DLG_CFG_PHONENOTIFY), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig PhoenEventNotify ok!"), DLG_CFG_PHONENOTIFY), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

BOOL CDlgCfgPhoneNotify::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0, nRestart = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_PHONEEVENTNOTIFY,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_PHONEEVENTNOTIFY_INFO stuInfo = {0};
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_PHONEEVENTNOTIFY, szJsonBuf, (void*)&stuInfo, sizeof(stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse PhoneEventNotify error..."), DLG_CFG_PHONENOTIFY), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_PHONEEVENTNOTIFY_INFO));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x",ConvertString("GetDevConfig PhoneEventNotify error:", DLG_CFG_PHONENOTIFY), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

void CDlgCfgPhoneNotify::StuToDlg()
{
	// enable
	m_chkEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);

	// number list
	m_lsNumberList.DeleteAllItems();
	int nCount = __min(m_stuInfo.dwContactCount, MAX_CONTACT_NUM);
	for (int i = 0; i < nCount; i++)
	{
		m_lsNumberList.InsertItem(i, "");

		char szSeq[8] = {0};
		_itoa(i, szSeq, 10);
		m_lsNumberList.SetItemText(i, 0, szSeq);
		m_lsNumberList.SetItemText(i, 1, m_stuInfo.szContactNumbersList[i]);
	}
}

void CDlgCfgPhoneNotify::DlgToStu()
{
	// enable
	m_stuInfo.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;
	
	// number list
	int nCount = m_lsNumberList.GetItemCount();
	m_stuInfo.dwContactCount = nCount;
	memset(m_stuInfo.szContactNumbersList, 0, sizeof(m_stuInfo.szContactNumbersList));
	for (int i = 0; i < nCount; i++)
	{
		CString strNumber = m_lsNumberList.GetItemText(i, 1);
		strncpy(m_stuInfo.szContactNumbersList[i], strNumber,
			    _countof(m_stuInfo.szContactNumbersList[i])-1);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgPhoneNotify message handlers

BOOL CDlgCfgPhoneNotify::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_PHONENOTIFY);
	
	// TODO: Add extra initialization here	
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString("We haven't login yet!", DLG_CFG_PHONENOTIFY), ConvertString("Prompt"));
		//EndDialog(0);
	}
	
	InitDlg();
	OnPhonenotifyBtnGet();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCfgPhoneNotify::OnPhonenotifyBtnAddnumber() 
{
	// TODO: Add your control notification handler code here	
	CString csNumber;
	GetDlgItemText(IDC_PHONENOTIFY_EDT_NUMBER, csNumber);
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

void CDlgCfgPhoneNotify::OnPhonenotifyBtnDelnumber() 
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

void CDlgCfgPhoneNotify::OnPhonenotifyBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}	
}

void CDlgCfgPhoneNotify::OnPhonenotifyBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu();
	SetConfigToDevice();
}
