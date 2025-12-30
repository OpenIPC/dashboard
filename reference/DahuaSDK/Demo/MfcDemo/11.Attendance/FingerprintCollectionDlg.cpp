// FingerprintCollectionDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Attendance.h"
#include "FingerprintCollectionDlg.h"
#include "Util.h"

// CFingerprintCollectionDlg 对话框

IMPLEMENT_DYNAMIC(CFingerprintCollectionDlg, CDialog)

LRESULT CFingerprintCollectionDlg::OnFingerPrintMessage(WPARAM wParam, LPARAM lParam)
{
	ALARM_CAPTURE_FINGER_PRINT_INFO *pstFingerPrint = (ALARM_CAPTURE_FINGER_PRINT_INFO *)wParam;
	if (NULL != pstFingerPrint)
	{
		int nLen = pstFingerPrint->nPacketNum * pstFingerPrint->nPacketLen;
		if (nLen <= sizeof(m_byFingerprintData))
		{	
			KillTimer(1);
			if (m_bListened)
			{
				m_pManager->StopListen();
				m_bListened = false;
			}
			m_bCaptureSuc = true;
			m_bFinished = true;
			memcpy(m_byFingerprintData, pstFingerPrint->szFingerPrintInfo, nLen);
			m_nFingerprintLen = nLen;
			GetDlgItem(IDC_STATIC_COLLECTION_STATE)->SetWindowText(ConvertString("Collection Completed"));
			GetDlgItem(IDC_BUTTON_START_COLLECTION)->EnableWindow(TRUE);
		}
		if (NULL != pstFingerPrint->szFingerPrintInfo)
		{
			delete []pstFingerPrint->szFingerPrintInfo;
			pstFingerPrint->szFingerPrintInfo = NULL;
		}
		delete pstFingerPrint;
		pstFingerPrint = NULL;
	}

	return 0;
}


BOOL CALLBACK funcMessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
	if (DH_ALARM_FINGER_PRINT == lCommand)
	{
		ALARM_CAPTURE_FINGER_PRINT_INFO *pstuFingerInfo = (ALARM_CAPTURE_FINGER_PRINT_INFO *)pBuf;
		if (pstuFingerInfo->nPacketLen <= 0 || pstuFingerInfo->nPacketNum <= 0 || 
			pstuFingerInfo->szFingerPrintInfo == NULL)
		{
			return FALSE;
		}

		ALARM_CAPTURE_FINGER_PRINT_INFO *pstuFingerMsg = new ALARM_CAPTURE_FINGER_PRINT_INFO;
		if (NULL == pstuFingerMsg)
		{
			return FALSE;
		}
		memcpy(pstuFingerMsg, pstuFingerInfo, sizeof(*pstuFingerMsg));
		pstuFingerMsg->szFingerPrintInfo  = new char[pstuFingerInfo->nPacketLen * pstuFingerInfo->nPacketNum];
		if (NULL == pstuFingerMsg->szFingerPrintInfo)
		{
			delete pstuFingerMsg;
			pstuFingerMsg = NULL;
			return FALSE;
		}
		memcpy(pstuFingerMsg->szFingerPrintInfo, pstuFingerInfo->szFingerPrintInfo, pstuFingerInfo->nPacketLen * pstuFingerInfo->nPacketNum);

		CFingerprintCollectionDlg *pThis = (CFingerprintCollectionDlg *)dwUser;
		HWND hWnd = pThis->GetSafeHwnd();
		if (NULL == hWnd)
		{
			delete []pstuFingerMsg->szFingerPrintInfo;
			pstuFingerMsg->szFingerPrintInfo = NULL;
			delete pstuFingerMsg;
			pstuFingerMsg = NULL;
			return FALSE;
		}
		PostMessage(hWnd, WM_ACCESS_FINGERPRINT, (WPARAM)pstuFingerMsg, NULL);
	}

	return TRUE;
}

CFingerprintCollectionDlg::CFingerprintCollectionDlg(CAttendanceManager *pManager, CWnd* pParent /*=NULL*/)
	: CDialog(CFingerprintCollectionDlg::IDD, pParent)
{
	m_pManager = pManager;
	m_bListened = false;
	m_bCaptureSuc = false;
	m_pManager->SetDVRMessCallBack(funcMessCallBack, (DWORD)this);

	//m_bListened = m_pManager->StartListenEx();
	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;
	m_bStarted = false;
	m_bFinished = false;
}

CFingerprintCollectionDlg::~CFingerprintCollectionDlg()
{
	
}

void CFingerprintCollectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFingerprintCollectionDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CFingerprintCollectionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_START_COLLECTION, &CFingerprintCollectionDlg::OnBnClickedButtonStartCollection)
	ON_MESSAGE(WM_ACCESS_FINGERPRINT, &CFingerprintCollectionDlg::OnFingerPrintMessage)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CFingerprintCollectionDlg 消息处理程序

BOOL CFingerprintCollectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFingerprintCollectionDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (0 == m_nFingerprintLen)
	{
		if (!m_bStarted)
		{
			MessageBox(ConvertString("Did not start collecting"), ConvertString("Prompt"));
			return;
		}
		else if (!m_bFinished)
		{
			MessageBox(ConvertString("In the collection"), ConvertString("Prompt"));
			return;	
		}
		
		MessageBox(ConvertString("No fingerprint data,because collection failed"), ConvertString("Prompt"));
		return ;
	}
	OnOK();
}

void CFingerprintCollectionDlg::OnBnClickedButtonStartCollection()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;
	GetDlgItem(IDC_STATIC_COLLECTION_STATE)->SetWindowText("");
	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;
	if (!m_bListened)
	{
		m_bListened = m_pManager->StartListenEx();
		if (false == m_bListened)
		{
			MessageBox(ConvertString("listen failed"), ConvertString("Prompt"));
			return;
		}
	}

	bool bRet = m_pManager->CaptureFingerprint();
	if (false == bRet)
	{
		MessageBox(ConvertString("start capture fingerprint failed"), ConvertString("Prompt"));	
		return ;
	}

	SetTimer(1, 30*1000, NULL);
	m_bStarted = true;
	GetDlgItem(IDC_STATIC_COLLECTION_STATE)->SetWindowText(ConvertString("Start Collection"));
	GetDlgItem(IDC_BUTTON_START_COLLECTION)->EnableWindow(FALSE);
}

bool CFingerprintCollectionDlg::GetFingerPrintData(char *pFingerBuf, int nBufLen)
{
	if (false == m_bCaptureSuc || pFingerBuf == NULL || nBufLen < m_nFingerprintLen || m_nFingerprintLen == 0)
	{
		return false;
	}

	memcpy(pFingerBuf, m_byFingerprintData, m_nFingerprintLen);
	return true;
}

void CFingerprintCollectionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_bCaptureSuc)
	{
		m_bFinished = true;
		m_nFingerprintLen = 0;	
		memset(&m_byFingerprintData, 0, sizeof(m_byFingerprintData));
		GetDlgItem(IDC_STATIC_COLLECTION_STATE)->SetWindowText(ConvertString("Collection failed"));
		GetDlgItem(IDC_BUTTON_START_COLLECTION)->EnableWindow(TRUE);
		KillTimer(1);
		if (m_bListened)
		{
			m_pManager->StopListen();
			m_bListened = false;
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void CFingerprintCollectionDlg::OnDestroy()
{
	if (m_bListened)
	{
		m_pManager->StopListen();
		m_bListened = false;
		KillTimer(1);
	}

	CDialog::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
}

int CFingerprintCollectionDlg::GetFingerPrintLen()
{
	return m_nFingerprintLen;
}

BOOL CFingerprintCollectionDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_KEYDOWN == pMsg->message)
	{
		switch (pMsg->wParam)
		{
		case VK_RETURN:
		case VK_ESCAPE:
			return true;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
