// DlgUserInfo.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgUserInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUserInfo dialog


CDlgUserInfo::CDlgUserInfo(CWnd* pParent /* = NULL */, LLONG hLoginId /* = 0 */)
	: CDialog(CDlgUserInfo::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgUserInfo)
		// NOTE: the ClassWizard will add member initialization here
	m_hLoginId = hLoginId;
	m_bGetUserInfo = TRUE;
	//}}AFX_DATA_INIT
}


void CDlgUserInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUserInfo)
	DDX_Control(pDX, IDC_USERINFO_LIST_INFO, m_lsUserInfo);
	DDX_Control(pDX, IDC_USERINFO_CMB_TYPE, m_cmbType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUserInfo, CDialog)
	//{{AFX_MSG_MAP(CDlgUserInfo)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_USERINFO_BTN_GET, OnUserinfoGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Private method

void CDlgUserInfo::InitDlg(BOOL bGetUserInfo /* = TRUE */)
{
	int i = 0;

	m_lsUserInfo.DeleteAllItems();
	int nColumnCount = m_lsUserInfo.GetHeaderCtrl()->GetItemCount();
	for (i = 0; i < nColumnCount; i++)
	{
		m_lsUserInfo.DeleteColumn(0);
	}

	m_cmbType.ResetContent();
	for (i = 0; i < sizeof(stuDemoUserInfoType) / sizeof(stuDemoUserInfoType[0]); i++)
	{
		m_cmbType.InsertString(-1, ConvertString(stuDemoUserInfoType[i].pszName, DLG_USERINFO));
	}

	if (bGetUserInfo)
	{
		m_lsUserInfo.InsertColumn(0, ConvertString("Seq", DLG_USERINFO), LVCFMT_LEFT, 40);
		m_lsUserInfo.InsertColumn(1, ConvertString("Name", DLG_USERINFO), LVCFMT_LEFT, 100);
		m_lsUserInfo.InsertColumn(2, ConvertString("Modify time at last", DLG_USERINFO), LVCFMT_LEFT, 150);

		m_cmbType.SetCurSel((int)emUserInfoType_All);
	} 
	else
	{
		m_lsUserInfo.InsertColumn(0, ConvertString("Seq", DLG_USERINFO), LVCFMT_LEFT, 40);
		m_lsUserInfo.InsertColumn(1, ConvertString("Name", DLG_USERINFO), LVCFMT_LEFT, 50);
		m_lsUserInfo.InsertColumn(2, ConvertString("Type", DLG_USERINFO), LVCFMT_LEFT, 100);
		m_lsUserInfo.InsertColumn(3, ConvertString("Address", DLG_USERINFO), LVCFMT_LEFT, 100);
		m_lsUserInfo.InsertColumn(4, ConvertString("Login time", DLG_USERINFO), LVCFMT_LEFT, 150);
		m_lsUserInfo.InsertColumn(5, ConvertString("Group", DLG_USERINFO), LVCFMT_LEFT, 60);

		m_cmbType.SetCurSel((int)emUserInfoType_Active);
	}
}

void CDlgUserInfo::GetInfo(BOOL bGetUserInfo /* = TRUE */)
{	
	m_lsUserInfo.DeleteAllItems();
	
	if (m_bGetUserInfo)
	{
		// Get all users
		USER_MANAGE_INFO_NEW* pstuParam = new USER_MANAGE_INFO_NEW;
		if (!pstuParam)
		{
			return;
		}
		memset(pstuParam, 0, sizeof(USER_MANAGE_INFO_NEW));
		pstuParam->dwSize = sizeof(*pstuParam);
		{
			for (int x = 0; x < sizeof(pstuParam->groupListEx)/sizeof(pstuParam->groupListEx[0]); x++)
			{
                pstuParam->groupList[x].dwSize = sizeof(USER_GROUP_INFO_NEW);
				pstuParam->groupListEx[x].dwSize = sizeof(USER_GROUP_INFO_EX2);
			}
            for (int x =0; x< DH_NEW_MAX_RIGHT_NUM; x++)
            {
                pstuParam->rightList[x].dwSize = sizeof(OPR_RIGHT_NEW);
            }
            for (int x= 0; x< DH_MAX_USER_NUM; x++)
            {
                pstuParam->userList[x].dwSize = sizeof(USER_INFO_NEW);
            }
		}
		BOOL bRet = CLIENT_QueryUserInfoNew(m_hLoginId, pstuParam, NULL, SDK_API_WAIT);
		if (bRet)
		{
			int nCount = __min(pstuParam->dwUserNum, DH_MAX_USER_NUM);
			for (int i = 0; i < nCount; i++)
			{
				InsertUserInfo(pstuParam->userList[i]);
			}
		}
		else
		{
			CString csOut;
			csOut.Format("%s %08x", ConvertString("Query user info failed:", DLG_USERINFO), CLIENT_GetLastError());
			MessageBox(csOut, ConvertString("Prompt"));
		}

		if (pstuParam)
		{
			delete pstuParam;
			pstuParam = NULL;
		}
	} 
	else
	{
		char szJson[1024] = {0};
		CFG_ACTIVEALLUSER_INFO stuParam = {sizeof(stuParam)};
		int i = 0;
		for (i = 0; i < MAX_ACTIVEUSER_NUM; i++)
		{
			stuParam.stuActiveUserInfo[i].nStructSize = sizeof(CFG_ACTIVEUSER_INFO);
			stuParam.stuActiveUserInfo[i].stuLoginTime.nStructSize = sizeof(CFG_NET_TIME);
		}
		int nErr = 0;
		BOOL bRet = CLIENT_QueryNewSystemInfo(m_hLoginId, CFG_USERMANAGER_ACTIVEUSER, -1,
			szJson, sizeof(szJson), &nErr, SDK_API_WAIT);
		if (bRet)
		{
			int nRet = 0;
			bRet = CLIENT_ParseData(CFG_USERMANAGER_ACTIVEUSER, szJson, &stuParam, sizeof(stuParam), &nRet);
			if (bRet && nRet == sizeof(stuParam))
			{
				for (i = 0; i < stuParam.nCount; i++)
				{
					InsertActiveUserInfo(stuParam.stuActiveUserInfo[i]);
				}
			}
			else
			{
				CString csOut;
				csOut.Format("%s", ConvertString("Parse active user info failed!", DLG_USERINFO));
				MessageBox(csOut, ConvertString("Prompt"));
			}
		}
		else
		{
			CString csOut;
			csOut.Format("%s %08x", ConvertString("Query active user info failed:", DLG_USERINFO), CLIENT_GetLastError());
			MessageBox(csOut, ConvertString("Prompt"));
		}
	}
}

void CDlgUserInfo::InsertUserInfo(const USER_INFO_NEW& stuInfo)
{
	int nCount = m_lsUserInfo.GetItemCount();
	m_lsUserInfo.InsertItem(nCount, "");
	
	char szSeq[8] = {0};
	_itoa(nCount, szSeq, 10);
	m_lsUserInfo.SetItemText(nCount, 0, szSeq);
	
	m_lsUserInfo.SetItemText(nCount, 1, stuInfo.name);

	CString csTime;
	csTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
		stuInfo.stuTime.dwYear, stuInfo.stuTime.dwMonth, stuInfo.stuTime.dwDay,
		stuInfo.stuTime.dwHour, stuInfo.stuTime.dwMinute, stuInfo.stuTime.dwSecond);
	m_lsUserInfo.SetItemText(nCount, 2, csTime);
}

void CDlgUserInfo::InsertActiveUserInfo(const CFG_ACTIVEUSER_INFO& stuInfo)
{
	int nCount = m_lsUserInfo.GetItemCount();
	m_lsUserInfo.InsertItem(nCount, "");
	
	char szSeq[8] = {0};
	_itoa(nCount, szSeq, 10);
	m_lsUserInfo.SetItemText(nCount, 0, szSeq);
	
	m_lsUserInfo.SetItemText(nCount, 1, stuInfo.szUser);
	
	m_lsUserInfo.SetItemText(nCount, 2, stuInfo.szClientType);
	
	m_lsUserInfo.SetItemText(nCount, 3, stuInfo.szClientAddr);
	
	CString csTime;
	csTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
		stuInfo.stuLoginTime.dwYear, stuInfo.stuLoginTime.dwMonth, stuInfo.stuLoginTime.dwDay,
		stuInfo.stuLoginTime.dwHour, stuInfo.stuLoginTime.dwMinute, stuInfo.stuLoginTime.dwSecond);
	m_lsUserInfo.SetItemText(nCount, 4, csTime);

	m_lsUserInfo.SetItemText(nCount, 5, stuInfo.szGroupName);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgUserInfo message handlers

BOOL CDlgUserInfo::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_USERINFO);
	
	// TODO: Add extra initialization here
	InitDlg(m_bGetUserInfo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgUserInfo::OnUserinfoGet() 
{
	// TODO: Add your control notification handler code here
	m_bGetUserInfo = emUserInfoType_All == m_cmbType.GetCurSel() ? TRUE : FALSE;
	InitDlg(m_bGetUserInfo);

	GetInfo(m_bGetUserInfo);
}

