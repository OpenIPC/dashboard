// UserInfoGetFingerPrint.cpp : 实现文件
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "UserInfoGetFingerPrint.h"
#include "AccessControl2sDlg.h"

// UserInfoGetFingerPrint 对话框

IMPLEMENT_DYNAMIC(UserInfoGetFingerPrint, CDialog)

UserInfoGetFingerPrint::UserInfoGetFingerPrint(CWnd* pParent /*=NULL*/)
	: CDialog(UserInfoGetFingerPrint::IDD, pParent)
{
	m_bDuress = FALSE;
	m_bCaptureSuc = FALSE;
	m_bStarted = FALSE;
	m_bFinished = FALSE;
	m_nFingerprintLen = 0;
	hParentWnd = NULL;
}

UserInfoGetFingerPrint::~UserInfoGetFingerPrint()
{
}

void UserInfoGetFingerPrint::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_DURESS, m_chkDuress);
}


BEGIN_MESSAGE_MAP(UserInfoGetFingerPrint, CDialog)
	ON_BN_CLICKED(IDC_BUT_GETFINGERPRINT, &UserInfoGetFingerPrint::OnBnClickedButGetfingerprint)
	ON_MESSAGE(WM_ACCESS_FINGERPRINTEX, &UserInfoGetFingerPrint::OnFingerPrintMessage)
	ON_BN_CLICKED(IDOK, &UserInfoGetFingerPrint::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL UserInfoGetFingerPrint::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	if (m_bDuress)
	{
		m_chkDuress.SetCheck(BST_CHECKED);
	}
	else
	{
		m_chkDuress.SetCheck(BST_UNCHECKED);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}
// UserInfoGetFingerPrint 消息处理程序

void UserInfoGetFingerPrint::OnBnClickedButGetfingerprint()
{
	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;

	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText("");
	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;
	::PostMessage(hParentWnd,WM_FINGERPRINT,1,0);

	BOOL bret = Device::GetInstance().StartGetFingerPrint();
	if (!bret)
	{
		MessageBox(ConvertString("Start capture fingerprint failed"), ConvertString("Prompt"));	
		return ;
	}

	SetTimer(1, 30*1000, NULL);
	m_bStarted = true;
	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Start Collection"));
	GetDlgItem(IDC_BUT_GETFINGERPRINT)->EnableWindow(FALSE);

}

LRESULT UserInfoGetFingerPrint::OnFingerPrintMessage(WPARAM wParam, LPARAM lParam)
{
	ALARM_CAPTURE_FINGER_PRINT_INFO *pstFingerPrint = (ALARM_CAPTURE_FINGER_PRINT_INFO *)wParam;
	if (NULL != pstFingerPrint)
	{
		//int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
		int nLen = pstFingerPrint->nPacketNum * pstFingerPrint->nPacketLen;
		if (nLen <= sizeof(m_byFingerprintData))
		{	
			KillTimer(1);
			::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
			m_bCaptureSuc = true;
			m_bFinished = true;
			memcpy(m_byFingerprintData, pstFingerPrint->szFingerPrintInfo, nLen);
			m_nFingerprintLen = nLen;
			GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection Completed"));
			GetDlgItem(IDC_BUT_GETFINGERPRINT)->EnableWindow(TRUE);

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
		//int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
		m_bCaptureSuc = true;
		m_bFinished = true;
		m_nFingerprintLen = 0;	
		memset(&m_byFingerprintData, 0, sizeof(m_byFingerprintData));
		GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection failed"));
		GetDlgItem(IDC_BUT_GETFINGERPRINT)->EnableWindow(TRUE);
	}

	return 0;
}

void UserInfoGetFingerPrint::OnDestroy()
{
	CDialog::OnDestroy();
	::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
	KillTimer(1);
}

void UserInfoGetFingerPrint::OnTimer(UINT_PTR nIDEvent)
{
	if (!m_bCaptureSuc)
	{
		//int nFingerPrintNo = m_cmbFingerPrintNO.GetCurSel();
		m_bFinished = true;
		m_nFingerprintLen = 0;	
		memset(&m_byFingerprintData, 0, sizeof(m_byFingerprintData));
		GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection failed"));
		GetDlgItem(IDC_BUT_GETFINGERPRINT)->EnableWindow(TRUE);
		KillTimer(1);
		::PostMessage(hParentWnd,WM_FINGERPRINT,0,0);
	}

	CDialog::OnTimer(nIDEvent);
}

void UserInfoGetFingerPrint::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_chkDuress.GetCheck())
	{
		m_bDuress = TRUE;
	}
	else
	{
		m_bDuress = FALSE;
	}

	OnOK();
}
