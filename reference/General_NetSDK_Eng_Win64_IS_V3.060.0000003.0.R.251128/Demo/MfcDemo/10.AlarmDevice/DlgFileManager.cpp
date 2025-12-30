// DlgFileManager.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgFileManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFileManager dialog


void CALLBACK UploadCallback(AV_HANDLE hLoadHandle, AV_int64 nTotalSize, AV_int64 nUploadSize, void* pReserved, void* pUserParam)
{
	CDlgFileManager *dlg = (CDlgFileManager*)pUserParam;
	if (NULL == dlg)
	{
		return;
	}

	if ((0 != nTotalSize) && (nUploadSize > 0))
	{
		dlg->m_progressUpload.SetPos((int)(nUploadSize * 100 / nTotalSize));
	}

	if (dlg->m_progressUpload.GetPos() == 100)
	{
		dlg->GetDlgItem(IDC_BUTTON_FILE1)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_UPLOAD)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_STOP1)->EnableWindow(FALSE);
		
		dlg->GetDlgItem(IDC_BUTTON_FILE2)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_DOWNLOAD)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);
	}
}

void CALLBACK DownloadDataCallback(AV_HANDLE hLoadHandle, AV_BYTE *pBuf, AV_int32 nBufSize, void* pReserved, void* pUserParam)
{
	CDlgFileManager *dlg = (CDlgFileManager*)pUserParam;
	if (NULL == dlg)
	{
		return;
	}
	
	if (NULL == dlg->m_pFile)
	{
		return;
	}

	int nRet = fwrite(pBuf, 1, nBufSize, dlg->m_pFile);
}

void CALLBACK DownloadPoseCallback(AV_HANDLE hLoadHandle, AV_int64 nTotalSize, AV_int64 nUploadSize, void* pReserved, void* pUserParam)
{
	CDlgFileManager *dlg = (CDlgFileManager*)pUserParam;
	if (NULL == dlg)
	{
		return;
	}
	
	if ((0 != nTotalSize) && (nUploadSize > 0))
	{
		dlg->m_progressDownload.SetPos((int)(nUploadSize*100/nTotalSize));
	}
	
	if (dlg->m_progressDownload.GetPos() == 100)
	{
		//MessageBox(ConvertString("Download file successfully.", DLG_FILEMANAGER), ConvertString("Prompt"));
		dlg->GetDlgItem(IDC_BUTTON_FILE1)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_UPLOAD)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_STOP1)->EnableWindow(FALSE);
		
		dlg->GetDlgItem(IDC_BUTTON_FILE2)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_DOWNLOAD)->EnableWindow(TRUE);
		dlg->GetDlgItem(IDC_BUTTON_STOP2)->EnableWindow(FALSE);

		if (NULL != dlg->m_pFile)
		{
			fclose(dlg->m_pFile);
			dlg->m_pFile = NULL;
		}

	}
}

CDlgFileManager::CDlgFileManager(CWnd* pParent /*=NULL*/, LLONG hLoginID /*= NULL*/)
	: CDialog(CDlgFileManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFileManager)
	m_csFileNameDownload = _T("");
	m_hLogin = hLoginID;
	m_csFileNameUpload = _T("");

	m_CSFolderDownload = _T("");
	m_hUpload = 0;
	m_hDownload = 0;
	m_pFile = NULL;
	//}}AFX_DATA_INIT
}


void CDlgFileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFileManager)
	DDX_Control(pDX, IDC_PROGRESS_UPLOAD, m_progressUpload);
	DDX_Control(pDX, IDC_PROGRESS_DOWNLOAD, m_progressDownload);
	DDX_Text(pDX, IDC_EDIT_FILENAME_DOWN, m_csFileNameDownload);
	DDX_Text(pDX, IDC_EDIT_FILENAME_UP, m_csFileNameUpload);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgFileManager, CDialog)
	//{{AFX_MSG_MAP(CDlgFileManager)
	ON_BN_CLICKED(IDC_BUTTON_FILE1, OnBtnGetFileToUpLoad)
	ON_BN_CLICKED(IDC_BUTTON_FILE2, OnBtnSetFilePathToDownLoad)
	ON_BN_CLICKED(IDC_BUTTON_STOP1, OnBtnStopUpLoad)
	ON_BN_CLICKED(IDC_BUTTON_STOP2, OnBtnStopDownLoad)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, OnBtnStartUpLoad)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, OnBtnStartDownLoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFileManager message handlers
BOOL CDlgFileManager::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_FILEMANAGER);

	// TODO: Add extra initialization here
	((CButton*)GetDlgItem(IDC_BUTTON_FILE1))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_UPLOAD))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP1))->EnableWindow(FALSE);
	
	((CButton*)GetDlgItem(IDC_BUTTON_FILE2))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_DOWNLOAD))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP2))->EnableWindow(FALSE);
	
	m_hUpload = NULL;
	m_hDownload = NULL;
		
	m_progressUpload.SetRange(0, 100);
	m_progressDownload.SetRange(0, 100);

	SetDlgItemInt(IDC_FILEMANAGER_EDT_UPTIME, 5, FALSE);
	SetDlgItemInt(IDC_FILEMANAGER_EDT_DOWNTIME, 5, FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CDlgFileManager::OnBtnGetFileToUpLoad() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, "*.*", "", OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_NOCHANGEDIR,
		"All File(*.*)|*.*||",this);
	if(dlg.DoModal() == IDOK)
	{
		m_CSFileUpload = dlg.GetFileName();
		m_CSFolderUpload = dlg.GetPathName();
		SetDlgItemText(IDC_EDIT_UPLOAD, m_CSFolderUpload);
	}
}

void CDlgFileManager::OnBtnSetFilePathToDownLoad() 
{
	// TODO: Add your control notification handler code here
	LPMALLOC   pMalloc;  
	if   (::SHGetMalloc(&pMalloc)   ==   NOERROR)   
	{   
		BROWSEINFO bi={0};   
		TCHAR pszBuffer[MAX_PATH];   
		LPITEMIDLIST pidl;   
		bi.hwndOwner = this->m_hWnd;   
		bi.pidlRoot = NULL;   
		bi.pszDisplayName = pszBuffer;   
		bi.lpszTitle = "select folder";//_T(ConvertString("select folder"));   
		bi.ulFlags = 0;//BIF_RETURNFSANCESTORS|BIF_RETURNONLYFSDIRS;   
		bi.lpfn = NULL;   
		bi.lParam = 0;   
		bi.iImage = 0;   
		if((pidl=::SHBrowseForFolder(&bi)) != NULL)   
		{   
			if(::SHGetPathFromIDList(pidl, pszBuffer))   
			{
				m_CSFolderDownload.Empty();
				m_CSFolderDownload  =  pszBuffer;  
				SetDlgItemText(IDC_EDIT_DOWNLOAD, pszBuffer);
			}

			pMalloc->Free(pidl);   
		}   
		pMalloc->Release();   
	}
}

void CDlgFileManager::OnBtnStopUpLoad() 
{
	// TODO: Add your control notification handler code here
	m_progressUpload.SetPos(0);

	((CButton*)GetDlgItem(IDC_BUTTON_FILE1))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_UPLOAD))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP1))->EnableWindow(FALSE);
	
	((CButton*)GetDlgItem(IDC_BUTTON_FILE2))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_DOWNLOAD))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP2))->EnableWindow(FALSE);

	m_progressUpload.SetPos(0);
}

void CDlgFileManager::OnBtnStopDownLoad() 
{
	// TODO: Add your control notification handler code here
	m_progressDownload.SetPos(0);

	((CButton*)GetDlgItem(IDC_BUTTON_FILE1))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_UPLOAD))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP1))->EnableWindow(FALSE);
	
	((CButton*)GetDlgItem(IDC_BUTTON_FILE2))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_DOWNLOAD))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_STOP2))->EnableWindow(FALSE);

	m_progressDownload.SetPos(0);

	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

void CDlgFileManager::OnBtnStartUpLoad() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_csFileNameUpload.IsEmpty())
	{
		MessageBox(ConvertString("Remote file name is empty.", DLG_FILEMANAGER), ConvertString("Prompt"));
		return;
	}

	if (m_CSFolderUpload.IsEmpty())
	{
		MessageBox(ConvertString("Local file name is empty.", DLG_FILEMANAGER), ConvertString("Prompt"));
		return;
	}
	
	//pszFileDst 在代码里被当做文件名传入，协议"PATH"字段暂时没有使用，如果需要使用时，再做处理，保留意见
	DH_IN_UPLOAD_REMOTE_FILE stuInParam = { sizeof(DH_IN_UPLOAD_REMOTE_FILE), m_CSFolderUpload.GetBuffer(0), m_csFileNameUpload.GetBuffer(0) };
	DH_OUT_UPLOAD_REMOTE_FILE stuOutParam = {sizeof(DH_OUT_UPLOAD_REMOTE_FILE)};
	
	int nWaitTime = GetDlgItemInt(IDC_FILEMANAGER_EDT_UPTIME, NULL, TRUE);
	if (nWaitTime <= 0)
	{
		nWaitTime = 0;
	}
	BOOL bResult = CLIENT_UploadRemoteFile((LLONG)m_hLogin, &stuInParam, &stuOutParam, nWaitTime*1000);
	if(0 == bResult	)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("Upload file failed:", DLG_FILEMANAGER), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return;
	}
	else
	{
		MessageBox(ConvertString("Upload file succeed!", DLG_FILEMANAGER), ConvertString("Prompt"));
	}
}

void CDlgFileManager::OnBtnStartDownLoad() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_csFileNameDownload.IsEmpty())
	{
		MessageBox(ConvertString("File name is empty.", DLG_FILEMANAGER), ConvertString("Prompt"));
		return;
	}

	m_progressDownload.SetPos(0);

	CString csFilePath = m_CSFolderDownload + "\\" + m_csFileNameDownload;
	DH_IN_DOWNLOAD_REMOTE_FILE stuInParam = { sizeof(DH_IN_DOWNLOAD_REMOTE_FILE), m_csFileNameDownload.GetBuffer(0), csFilePath.GetBuffer(0) };
	DH_OUT_DOWNLOAD_REMOTE_FILE stuOutParam = {sizeof(DH_OUT_DOWNLOAD_REMOTE_FILE)};
	
	int nWaitTime = GetDlgItemInt(IDC_FILEMANAGER_EDT_DOWNTIME, NULL, TRUE);
	if (nWaitTime <= 0)
	{
		nWaitTime = 0;
	}
	BOOL bResult = CLIENT_DownloadRemoteFile((LLONG)m_hLogin, &stuInParam, &stuOutParam, nWaitTime*1000);
	
	if(0 == bResult	)
	{
		CString csOut;
		csOut.Format("%s %08x", ConvertString("download file failed:", DLG_FILEMANAGER), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return;
	}
	else
	{
		m_progressDownload.SetPos(100);
		MessageBox(ConvertString("download file succeed!", DLG_FILEMANAGER), ConvertString("Prompt"));
	}
}
