// DlgCfgArmLink.cpp : 实现文件
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgArmLink.h"


// DlgCfgArmLink 对话框

IMPLEMENT_DYNAMIC(DlgCfgArmLink, CDialog)

DlgCfgArmLink::DlgCfgArmLink(CWnd* pParent /*=NULL*/, LLONG lLoginId /* = 0*/)
	: CDialog(DlgCfgArmLink::IDD, pParent)
{
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));

}

DlgCfgArmLink::~DlgCfgArmLink()
{
}

void DlgCfgArmLink::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHK_CFGARMLINK_MMSENABLE, m_chkMMSEnable);
	DDX_Control(pDX, IDC_CHK_CFGARMLINK_PERSONALARMENABLE, m_chkPersonAlarmEnable);
	DDX_Control(pDX, IDC_CHK_CFGARMLINK_NEEDREPORT, m_chkNeedReportEnable);
	DDX_Control(pDX, IDC_LIST_CFGARMLINK, m_lsNumberList);
}


BEGIN_MESSAGE_MAP(DlgCfgArmLink, CDialog)
	ON_BN_CLICKED(IDC_BTN_CFGARMLINK_ADDINDEX, OnBnClickedBtnCfgarmlinkAddindex)
	ON_BN_CLICKED(IDC_BTN_CFGARMLINK_DELINDEX, OnBnClickedBtnCfgarmlinkDelindex)
	ON_BN_CLICKED(IDC_BTN_CFGARMLINK_GET, OnBnClickedBtnCfgarmlinkGet)
	ON_BN_CLICKED(IDC_BTN_CFGARMLINK_SET, OnBnClickedBtnCfgarmlinkSet)
END_MESSAGE_MAP()

BOOL DlgCfgArmLink::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_ARMLINK);

	// TODO: Add extra initialization here	
	if (0 == m_lLoginId)
	{
		MessageBox(ConvertString(CString("We haven't login yet!"), DLG_CFG_ARMLINK), ConvertString("Prompt"));
		//EndDialog(0);
	}

	InitDlg();
	OnBnClickedBtnCfgarmlinkGet();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE

}

// DlgCfgArmLink 消息处理程序
void DlgCfgArmLink::OnBnClickedBtnCfgarmlinkGet()
{
	// TODO: 在此添加控件通知处理程序代码
	if (GetConfigFromDevice())
	{
		StuToDlg();
	}
}

void DlgCfgArmLink::OnBnClickedBtnCfgarmlinkSet()
{
	// TODO: 在此添加控件通知处理程序代码
	DlgToStu();
	SetConfigToDevice();
}

void DlgCfgArmLink::InitDlg( BOOL bShow /*= TRUE*/ )
{
	m_chkMMSEnable.SetCheck(BST_UNCHECKED);
	m_chkPersonAlarmEnable.SetCheck(BST_UNCHECKED);
	m_chkNeedReportEnable.SetCheck(BST_UNCHECKED);

	m_lsNumberList.SetExtendedStyle(m_lsNumberList.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lsNumberList.DeleteAllItems();
	m_lsNumberList.InsertColumn(0, ConvertString("Seq", DLG_CFG_ARMLINK), LVCFMT_LEFT, 60);
	m_lsNumberList.InsertColumn(1, ConvertString("Alarm Server Center Index", DLG_CFG_ARMLINK), LVCFMT_LEFT, 160);

}

void DlgCfgArmLink::OnBnClickedBtnCfgarmlinkAddindex()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csNumber;
	GetDlgItemText(IDC_EDT_CFGARMLINK_ALARMSERVERCENTER_INDEX, csNumber);
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

void DlgCfgArmLink::OnBnClickedBtnCfgarmlinkDelindex()
{
	// TODO: 在此添加控件通知处理程序代码
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


BOOL DlgCfgArmLink::SetConfigToDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	BOOL bRet = CLIENT_PacketData(CFG_CMD_ARM_LINK, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet ArmLink error..."), DLG_CFG_ARMLINK), ConvertString("Prompt"));
		return FALSE;
	} 
	else
	{
#if 0
		WriteLog(szJsonBuf);
#endif	
		int nErr = 0, nRestart = 0;
		BOOL bRetSetupCfg = CLIENT_SetNewDevConfig(m_lLoginId, CFG_CMD_ARM_LINK,
			-1, szJsonBuf, strlen(szJsonBuf), &nErr, &nRestart, SDK_API_WAIT);
		if (!bRetSetupCfg)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig ArmLink failed:", DLG_CFG_ARMLINK), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig ArmLink ok!"), DLG_CFG_ARMLINK), ConvertString("Prompt"));
		}
	}
	return TRUE;

}

BOOL DlgCfgArmLink::GetConfigFromDevice()
{
	char szJsonBuf[1024 * 10] = {0};
	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginId, CFG_CMD_ARM_LINK,
		-1, szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (bRet)
	{
		CFG_ARMLINK_INFO stuInfo = {0};
		DWORD dwRetLen = 0;
		bRet = CLIENT_ParseData(CFG_CMD_ARM_LINK, szJsonBuf, (void*)&stuInfo, sizeof(stuInfo), &dwRetLen);
		if (!bRet)
		{
			MessageBox(ConvertString(CString("parse ArmLink error..."), DLG_CFG_ARMLINK), ConvertString("Prompt"));
			return FALSE;
		}
		else
		{
			memcpy(&m_stuInfo, &stuInfo, sizeof(CFG_ARMLINK_INFO));
		}
	}
	else
	{			
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryConfig ArmLink error:", DLG_CFG_ARMLINK), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;

}

void DlgCfgArmLink::DlgToStu()
{
	// enable
	m_stuInfo.bMMSEnable = m_chkMMSEnable.GetCheck() ? TRUE : FALSE;
	m_stuInfo.bPersonAlarmEnable = m_chkPersonAlarmEnable.GetCheck() ? TRUE : FALSE;
	m_stuInfo.stuPSTNAlarmServer.bNeedReport = m_chkNeedReportEnable.GetCheck() ? TRUE : FALSE;

	// number list
	int nCount = m_lsNumberList.GetItemCount();
	nCount = __min(nCount, _countof(m_stuInfo.stuPSTNAlarmServer.byDestination));
	m_stuInfo.stuPSTNAlarmServer.nServerCount = nCount;
	memset(m_stuInfo.stuPSTNAlarmServer.byDestination, 0, sizeof(m_stuInfo.stuPSTNAlarmServer.byDestination));
	for (int i = 0; i < nCount; i++)
	{
		CString strNumber = m_lsNumberList.GetItemText(i, 1);
		m_stuInfo.stuPSTNAlarmServer.byDestination[i] = (BYTE)_ttoi(strNumber);
	}

}

void DlgCfgArmLink::StuToDlg()
{
	// enable
	m_chkMMSEnable.SetCheck(m_stuInfo.bMMSEnable ? BST_CHECKED : BST_UNCHECKED);
	m_chkPersonAlarmEnable.SetCheck(m_stuInfo.bPersonAlarmEnable ? BST_CHECKED : BST_UNCHECKED);
	m_chkNeedReportEnable.SetCheck(m_stuInfo.stuPSTNAlarmServer.bNeedReport ? BST_CHECKED : BST_UNCHECKED);

	// number list
	m_lsNumberList.DeleteAllItems();
	int nCount = __min(m_stuInfo.stuPSTNAlarmServer.nServerCount, MAX_RECEIVER_NUM);
	for (int i = 0; i < nCount; i++)
	{
		m_lsNumberList.InsertItem(i, "");

		char szSeq[8] = {0};
		_itoa(i, szSeq, 10);
		m_lsNumberList.SetItemText(i, 0, szSeq);
		char szIndex[8] = {0};
		_itoa((int)m_stuInfo.stuPSTNAlarmServer.byDestination[i],szIndex,10);
		m_lsNumberList.SetItemText(i, 1, szIndex);
	}

}
