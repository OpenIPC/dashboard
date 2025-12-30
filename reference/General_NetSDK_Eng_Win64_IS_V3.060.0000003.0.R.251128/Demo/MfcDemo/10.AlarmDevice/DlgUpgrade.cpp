// DlgUpgrade.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgUpgrade.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUpgrade dialog


CDlgUpgrade::CDlgUpgrade(CWnd* pParent /* = NULL */, LLONG hLoginId /* = NULL */)
	: CDialog(CDlgUpgrade::IDD, pParent),
	m_hLoginId(hLoginId),
	m_hUpgradeId(NULL)
{
	//{{AFX_DATA_INIT(CDlgUpgrade)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgUpgrade::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUpgrade)
	DDX_Control(pDX, IDC_UPGRADE_LIST_INFO, m_lsInfo);
	DDX_Control(pDX, IDC_UPGRADE_PROGRESS, m_ctrlProgress);
	DDX_Control(pDX, IDC_UPGRADE_STATIC_COUNT, m_staticProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUpgrade, CDialog)
	//{{AFX_MSG_MAP(CDlgUpgrade)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_UPGRADE_BTN_PATH, OnUpgradeBtnPath)
	ON_BN_CLICKED(IDC_UPGRADE_BTN_UPGRADE, OnUpgradeBtnUpgrade)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgUpgrade private handlers

void CDlgUpgrade::StopUpgrade(LLONG lUpgradId /* = NULL */)
{
	if (lUpgradId)
	{
		CLIENT_StopUpgrade(lUpgradId);
		lUpgradId = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlgUpgrade message handlers

BOOL CDlgUpgrade::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_UPGRADE);
	
	// TODO: Add extra initialization here
	if (!m_hLoginId)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_UPGRADE), ConvertString("Prompt"));
		return TRUE;
	}
	
	m_ctrlProgress.SetRange(0, 100);
	m_ctrlProgress.SetPos(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgUpgrade::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	StopUpgrade(m_hUpgradeId);	
}

void CDlgUpgrade::OnUpgradeBtnPath() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_NOCHANGEDIR, _T("All Types (*.*)|*.*||"), this);
	
	if (dlg.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_UPGRADE_EDT_PATH, dlg.GetPathName());
	}	
}

void CDlgUpgrade::OnUpgradeBtnUpgrade() 
{
	// TODO: Add your control notification handler code here
	{
		// make sure the previous upgrading is closed before another upgrading operation
		StopUpgrade(m_hUpgradeId);
	}

	CString strFilePath;
	GetDlgItemText(IDC_UPGRADE_EDT_PATH, strFilePath);
	if (strFilePath.IsEmpty())
	{
		MessageBox(ConvertString("please choose a upgrade packet file.", DLG_UPGRADE), ConvertString("Prompt"));
		return;
	}
	
	m_hUpgradeId = CLIENT_StartUpgradeEx(m_hLoginId, (EM_UPGRADE_TYPE)0, strFilePath.GetBuffer(0), UpgradeCallBack, (LLONG)this);

	if (m_hUpgradeId)
	{
		m_ctrlProgress.SetPos(0);
		int nRet = CLIENT_SendUpgrade(m_hUpgradeId);
	}
	else
	{
		m_ctrlProgress.SetPos(0);
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Upgrade failed with code", DLG_UPGRADE), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
}

void WINAPI UpgradeCallBack(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize, LDWORD dwUser)
{
	((CDlgUpgrade*)dwUser)->UpgradeState(lLoginID, lUpgradechannel, nTotalSize, nSendSize);
}

void CDlgUpgrade::UpgradeState(LLONG lLoginID, LLONG lUpgradechannel, int nTotalSize, int nSendSize)
{
	if (lLoginID && lLoginID == m_hLoginId && lUpgradechannel /*&& lUpgradechannel == m_hUpgradeId*/)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		int nCount = m_lsInfo.GetCount();
		CString csInfo;

		CString csProgress = ConvertString("upgrade progress", DLG_UPGRADE);
		CString csSucceed = ConvertString("upgrade succeed", DLG_UPGRADE);
		CString csFailed = ConvertString("upgrade failed", DLG_UPGRADE);

		if (nTotalSize == 0)
		{
			if (nSendSize == -1)
			{
				csInfo.Format("%02d:%02d:%02d.%03d %s: %s", 
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, csProgress, csSucceed);
				m_lsInfo.InsertString(nCount, csInfo);
				m_lsInfo.SetCurSel(nCount);
			} 
			else if (nSendSize == -2)
			{
				csInfo.Format("%02d:%02d:%02d.%03d %s: %s", 
					st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, csProgress, csFailed);
				m_lsInfo.InsertString(nCount, csInfo);
				m_lsInfo.SetCurSel(nCount);
			}
			return;
		}
		else if (nTotalSize > 0)
		{
			int nProgress = 100 * nSendSize / nTotalSize;
			if (nProgress <= 0)
			{
				nProgress = 0;
			}
			if (nProgress > 99)
			{
				nProgress = 100;
			}
			m_ctrlProgress.SetPos(nProgress);
			
			csInfo.Format("%02d:%02d:%02d.%03d %s: %d %d/%d", 
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, csProgress,
				nProgress, nSendSize, nTotalSize);
			m_lsInfo.InsertString(nCount, csInfo);
			m_lsInfo.SetCurSel(nCount);
		}
	}
}
