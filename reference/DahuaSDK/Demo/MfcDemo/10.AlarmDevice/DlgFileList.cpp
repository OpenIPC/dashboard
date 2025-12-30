// DlgFileList.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgFileList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_FILE_COUNT	128
/////////////////////////////////////////////////////////////////////////////
// CDlgFileList dialog


CDlgFileList::CDlgFileList(CWnd* pParent /* = NULL */, LLONG hLoginId /* = NULL */)
	: CDialog(CDlgFileList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFileList)
	m_hLoginId = hLoginId;
	m_uiRetCount = 0;
	m_uiMaxCount = 0;
	//}}AFX_DATA_INIT
}


void CDlgFileList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFileList)
	DDX_Control(pDX, IDC_FILENAME_LIST_FILENAME, m_lsFileList);
	DDX_Text(pDX, IDC_FILENAME_EDT_RETNUMBER, m_uiRetCount);
	DDX_Text(pDX, IDC_FILENAME_EDT_MAXNUMBER, m_uiMaxCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFileList, CDialog)
	//{{AFX_MSG_MAP(CDlgFileList)
	ON_BN_CLICKED(IDC_FILENAME_BTN_GET, OnFilenameBtnGet)
	ON_BN_CLICKED(IDC_FILENAME_BTN_REMOVE, OnFilenameBtnRemove)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Private method

void CDlgFileList::InitDlg()
{
	SetDlgItemInt(IDC_FILENAME_EDT_MAXNUMBER, 16, FALSE);
	SetDlgItemInt(IDC_FILENAME_EDT_RETNUMBER, 0, FALSE);
	
	m_lsFileList.SetExtendedStyle(m_lsFileList.GetExtendedStyle() 
		| LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
	m_lsFileList.DeleteAllItems();
	m_lsFileList.InsertColumn(0, ConvertString("Seq", DLG_FILENAME), LVCFMT_LEFT, 50);
	m_lsFileList.InsertColumn(1, ConvertString("FileName", DLG_FILENAME), LVCFMT_LEFT, 180);

	CString csPath = ConvertString(CString("DefaultPath"), DLG_FILENAME);
	if (csPath == "DefaultPath")
	{
		SetDlgItemText(IDC_FILENAME_EDT_PATH, "/audio");
	} 
	else
	{
		SetDlgItemText(IDC_FILENAME_EDT_PATH, csPath);
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDlgFileList message handlers

BOOL CDlgFileList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_FILENAME);
	
	// TODO: Add extra initialization here
	InitDlg();
	OnFilenameBtnGet();

// 	char szSeq[8] = {0};
// 	itoa(1, szSeq, 10);
// 	m_lsFileList.InsertItem(0, "", 0);
// 	m_lsFileList.SetItemText(0, 0, szSeq);
// 	m_lsFileList.SetItemText(0, 1, "abc");
// 
// 	itoa(2, szSeq, 10);
// 	m_lsFileList.InsertItem(1, "", 0);
// 	m_lsFileList.SetItemText(1, 0, szSeq);
// 	m_lsFileList.SetItemText(1, 1, "123");
// 	
// 	itoa(3, szSeq, 10);
// 	m_lsFileList.InsertItem(2, "", 0);
// 	m_lsFileList.SetItemText(2, 0, szSeq);
// 	m_lsFileList.SetItemText(2, 1, "456");
// 
// 	char* szTemp1 = "abc";
// 	char* szTemp2 = "123";
// 	char* szTemp3 = "456";
// 	m_listInfo.push_back(szTemp1);
// 	m_listInfo.push_back(szTemp2);
// 	m_listInfo.push_back(szTemp3);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgFileList::OnFilenameBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (0 == m_hLoginId)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_FILENAME), ConvertString("Prompt"));
		return;
	}

	m_lsFileList.DeleteAllItems();

	DH_IN_LIST_REMOTE_FILE stuIn = {sizeof(stuIn)};
	
	CString csPath;
	GetDlgItemText(IDC_FILENAME_EDT_PATH, csPath);
	stuIn.bFileNameOnly = TRUE;
	stuIn.pszPath = csPath.GetBuffer(0);

	DH_OUT_LIST_REMOTE_FILE stuOut = {sizeof(stuOut)};
	
	int nCount = GetDlgItemInt(IDC_FILENAME_EDT_MAXNUMBER, NULL, FALSE);
	if (nCount <= 0)
	{
		nCount = 16;
		SetDlgItemInt(IDC_FILENAME_EDT_MAXNUMBER, 16, FALSE);
	}
	DH_REMOTE_FILE_INFO* pFile = new DH_REMOTE_FILE_INFO[nCount];
	if (!pFile)
	{
		return;
	}
	memset(pFile, 0, sizeof(DH_REMOTE_FILE_INFO) * nCount);

	for (int i = 0; i < nCount; i++)
	{
		pFile[i].dwSize = sizeof(DH_REMOTE_FILE_INFO);
	}
	stuOut.nMaxFileCount = nCount;
	stuOut.pstuFiles = pFile;

	BOOL bRet = CLIENT_ListRemoteFile(m_hLoginId, &stuIn, &stuOut, SDK_API_WAIT);
	SetDlgItemInt(IDC_FILENAME_EDT_RETNUMBER, stuOut.nRetFileCount, FALSE);
	if (bRet)
	{
		for (int j = 0; j < stuOut.nRetFileCount; j++)
		{
			m_lsFileList.InsertItem(j, "", 0);
			
			char szSeq[8] = {0};
			_itoa(j + 1, szSeq, 10);
			m_lsFileList.SetItemText(j, 0, szSeq);
			m_lsFileList.SetItemText(j, 1, pFile[j].szPath);
			m_listInfo.push_back(pFile[j].szPath);
		}
	} 
	else
	{
		if (NET_INSUFFICIENT_BUFFER == CLIENT_GetLastError())
		{
			SetDlgItemInt(IDC_FILENAME_EDT_RETNUMBER, stuOut.nRetFileCount, TRUE);
		}
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Get file_list err:", DLG_FILENAME), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
	}
	
	if (pFile)
	{
		delete []pFile;
		pFile = NULL;
	}
}

void CDlgFileList::OnFilenameBtnRemove() 
{
	// TODO: Add your control notification handler code here
	if (0 == m_hLoginId)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_FILENAME), ConvertString("Prompt"));
		return;
	}

	int nDeleteCount = 0;
	char *pszPath[MAX_FILE_COUNT] = {0};
	char szPath[MAX_FILE_COUNT][MAX_PATH] = {0};
	for (int i = 0; i < m_lsFileList.GetItemCount() && nDeleteCount < MAX_FILE_COUNT; ++i)
	{
		if (m_lsFileList.GetCheck(i))
		{
			m_lsFileList.GetItemText(i, 1, szPath[nDeleteCount], MAX_PATH);
			pszPath[nDeleteCount] = szPath[nDeleteCount];
			++nDeleteCount;
//			std::list<char*>::iterator it = m_listInfo.begin();

// 			for (; it != m_listInfo.end(); ++it)
// 			{
// 				if (strcmp(*it, szPath[nDeleteCount]) == 0)
// 				{
// 					m_listInfo.erase(it++);
// 					break;
// 				}
// 			}
		}
	}
	
	DH_IN_REMOVE_REMOTE_FILES pInParam = {sizeof(DH_IN_REMOVE_REMOTE_FILES), (const char**)pszPath, nDeleteCount};
	DH_OUT_REMOVE_REMOTE_FILES pOutParam = {sizeof(DH_OUT_REMOVE_REMOTE_FILES)};

	BOOL bRet = CLIENT_RemoveRemoteFiles(m_hLoginId, &pInParam, &pOutParam, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("RemoveFiles failed:", DLG_FILENAME), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	}
	else
	{
		Sleep(500);
		OnFilenameBtnGet();
	}
}

