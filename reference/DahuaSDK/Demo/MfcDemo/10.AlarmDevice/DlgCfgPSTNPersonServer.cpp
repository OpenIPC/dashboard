// DlgCfgPSTNPersonServer.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgPSTNPersonServer.h"


// CDlgCfgPSTNPersonServer 对话框

IMPLEMENT_DYNAMIC(CDlgCfgPSTNPersonServer, CDialog)

CDlgCfgPSTNPersonServer::CDlgCfgPSTNPersonServer(CWnd* pParent /*=NULL*/, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgCfgPSTNPersonServer::IDD, pParent)
{
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));

}

CDlgCfgPSTNPersonServer::~CDlgCfgPSTNPersonServer()
{
}

void CDlgCfgPSTNPersonServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CFGPSTNPERSONSERVER_CHK_ENABLE, m_chkEnable);
	DDX_Control(pDX, IDC_LIST_PTSNPERSONSERVER, m_lsPersonServerList);
}


BEGIN_MESSAGE_MAP(CDlgCfgPSTNPersonServer, CDialog)
	ON_BN_CLICKED(IDC_BTN_PSTNPERSONSERVER_GET, OnBnClickedBtnPstnpersonserverGet)
	ON_BN_CLICKED(IDC_BTN_PSTNPERSONSERVER_SET, OnBnClickedBtnPstnpersonserverSet)
	ON_BN_CLICKED(IDC_BTN_PSTNPERSONSERVER_ADDNUMBER, OnBnClickedBtnPstnpersonserverAddnumber)
	ON_BN_CLICKED(IDC_BTN_PSTNPERSONSERVER_DELNUMBER, OnBnClickedBtnPstnpersonserverDelnumber)
END_MESSAGE_MAP()


// CDlgCfgPSTNPersonServer 消息处理程序

void CDlgCfgPSTNPersonServer::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}

BOOL CDlgCfgPSTNPersonServer::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_PSTNPERSONSERVER);

	// TODO: Add extra initialization here	
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_CFG_PSTNPERSONSERVER), ConvertString("Prompt"));
		//EndDialog(0);
	}

	InitDlg();
	OnBnClickedBtnPstnpersonserverGet();

	return TRUE;  // return TRUE unless you set the focus to a control

}

void CDlgCfgPSTNPersonServer::InitDlg( BOOL bShow /*= TRUE*/ )
{
	m_chkEnable.SetCheck(BST_UNCHECKED);

	m_lsPersonServerList.SetExtendedStyle(m_lsPersonServerList.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lsPersonServerList.DeleteAllItems();
	m_lsPersonServerList.InsertColumn(0, ConvertString("Seq", DLG_CFG_PSTNPERSONSERVER), LVCFMT_LEFT, 60);
	m_lsPersonServerList.InsertColumn(1, ConvertString("Name", DLG_CFG_PSTNPERSONSERVER), LVCFMT_LEFT, 160);
	m_lsPersonServerList.InsertColumn(2, ConvertString("Number", DLG_CFG_PSTNPERSONSERVER), LVCFMT_LEFT, 160);

}


BOOL CDlgCfgPSTNPersonServer::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_PSTN_PERSON_SERVER,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_PSTN_PERSON_SERVER_INFO_ALL stuInfo = {0};
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_PSTN_PERSON_SERVER, szJsonBuf, (void*)&stuInfo, sizeof(stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse PSTNPersonServer error..."), DLG_CFG_PSTNPERSONSERVER), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_PSTN_PERSON_SERVER_INFO_ALL));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig PSTNPersonServer error:", DLG_CFG_PSTNPERSONSERVER), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}

BOOL CDlgCfgPSTNPersonServer::SetConfigToDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_PSTN_PERSON_SERVER, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet PSTNPersonServer error..."), DLG_CFG_PSTNPERSONSERVER), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif	
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_PSTN_PERSON_SERVER,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRetSetupCfg)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig PSTNPersonServer failed:", DLG_CFG_PSTNPERSONSERVER), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig PSTNPersonServer ok!"), DLG_CFG_PSTNPERSONSERVER), ConvertString("Prompt"));
		}
	}
	return TRUE;
}

void CDlgCfgPSTNPersonServer::StuToDlg()
{
	// enable
	m_chkEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);

	// number list
	m_lsPersonServerList.DeleteAllItems();
	int nCount = __min(m_stuInfo.nServerCount, MAX_RECEIVER_NUM);
	for (int i = 0; i < nCount; i++)
	{
		m_lsPersonServerList.InsertItem(i, "");

		char szSeq[8] = {0};
		_itoa(i, szSeq, 10);
		m_lsPersonServerList.SetItemText(i, 0, szSeq);
		m_lsPersonServerList.SetItemText(i, 1, m_stuInfo.stuPSTNPersonServer[i].szName);
		m_lsPersonServerList.SetItemText(i, 2, m_stuInfo.stuPSTNPersonServer[i].szNumber);
	}
}

void CDlgCfgPSTNPersonServer::DlgToStu()
{
	// enable
	m_stuInfo.bEnable = m_chkEnable.GetCheck() ? TRUE : FALSE;

	// number list
	int nCount = m_lsPersonServerList.GetItemCount();
	nCount = __min(nCount, _countof(m_stuInfo.stuPSTNPersonServer));
	m_stuInfo.nServerCount = nCount;
	memset(m_stuInfo.stuPSTNPersonServer, 0, sizeof(m_stuInfo.stuPSTNPersonServer));
	for (int i = 0; i < nCount; i++)
	{
		CString strName = m_lsPersonServerList.GetItemText(i, 1);
		strncpy(m_stuInfo.stuPSTNPersonServer[i].szName, 
			strName, 
			sizeof(m_stuInfo.stuPSTNPersonServer[i].szName) - 1);

		CString strNumber = m_lsPersonServerList.GetItemText(i, 2);
		strncpy(m_stuInfo.stuPSTNPersonServer[i].szNumber, 
			strNumber, 
			sizeof(m_stuInfo.stuPSTNPersonServer[i].szNumber) - 1);
	}
}
void CDlgCfgPSTNPersonServer::OnBnClickedBtnPstnpersonserverGet()
{
	// TODO: 在此添加控件通知处理程序代码
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
}

void CDlgCfgPSTNPersonServer::OnBnClickedBtnPstnpersonserverSet()
{
	// TODO: 在此添加控件通知处理程序代码
	DlgToStu();
	SetConfigToDevice();
}

void CDlgCfgPSTNPersonServer::OnBnClickedBtnPstnpersonserverAddnumber()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csNumber;
	GetDlgItemText(IDC_EDT_PSTNPERSONSERVER_NUMBER, csNumber);
	if (!csNumber.GetLength())
	{
		return;
	}

	CString csName;
	GetDlgItemText(IDC_EDT_PSTNPERSONSERVER_NAME, csName);

	int nCount = m_lsPersonServerList.GetItemCount();
	m_lsPersonServerList.InsertItem(nCount, _T(""));

	char szSeq[10] = {0};
	_itoa(nCount + 1, szSeq, 10);
	m_lsPersonServerList.SetItemText(nCount, 0, szSeq);
	m_lsPersonServerList.SetItemText(nCount, 1, csName);	
	m_lsPersonServerList.SetItemText(nCount, 2, csNumber);	
}

void CDlgCfgPSTNPersonServer::OnBnClickedBtnPstnpersonserverDelnumber()
{
	// TODO: 在此添加控件通知处理程序代码
	POSITION pItem = m_lsPersonServerList.GetFirstSelectedItemPosition();
	if (pItem)
	{
		int nSel = m_lsPersonServerList.GetNextSelectedItem(pItem);

		{
			typedef std::list<CString> lsItem;
			lsItem lsNameItem;
			lsItem lsNumberItem;
			int nCount = m_lsPersonServerList.GetItemCount();
			for (int i = 0; i < nCount; i++)
			{
				CString csNameItem = m_lsPersonServerList.GetItemText(i, 1);
				lsNameItem.push_back(csNameItem);
				CString csNumberItem = m_lsPersonServerList.GetItemText(i, 2);
				lsNumberItem.push_back(csNumberItem);
			}

			int nIndex = 0;
			lsItem::iterator it = lsNumberItem.begin();
			lsItem::iterator itName = lsNameItem.begin();
			while (it != lsNumberItem.end())
			{
				if (nSel == nIndex++)
				{
					lsNameItem.erase(itName);
					lsNumberItem.erase(it);
					break;
				}
				itName++;
				it++;
			}

			m_lsPersonServerList.DeleteAllItems();
			it = lsNumberItem.begin();
			itName = lsNameItem.begin();

			for (int j = 0; j < lsNumberItem.size(); j++)
			{
				CString csName = *itName++;
				CString csNumber = *it++;

				m_lsPersonServerList.InsertItem(j, "");

				char szSeq[8] = {0};
				_itoa(j + 1, szSeq, 10);
				m_lsPersonServerList.SetItemText(j, 0, szSeq);
				m_lsPersonServerList.SetItemText(j, 1, csName);
				m_lsPersonServerList.SetItemText(j, 2, csNumber);
			}
		}
	}
}
