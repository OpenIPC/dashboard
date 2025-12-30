// GetFingerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VTODemo.h"
#include "GetFingerDlg.h"


BOOL CALLBACK funcMessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf, DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, BOOL bAlarmAckFlag, LONG nEventID, LDWORD dwUser)
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

		CGetFingerDlg *pThis = (CGetFingerDlg *)dwUser;
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

// CGetFingerDlg 对话框

IMPLEMENT_DYNAMIC(CGetFingerDlg, CDialog)

CGetFingerDlg::CGetFingerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetFingerDlg::IDD, pParent)
{
	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;
	m_bListened = false;

	CLIENT_SetDVRMessCallBackEx1(funcMessCallBack, (DWORD)this);

	//m_bListened = m_pManager->StartListenEx();
	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;

	m_lLoginHandle = 0;
}

CGetFingerDlg::~CGetFingerDlg()
{
}

void CGetFingerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGetFingerDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_LOGOUT_FEINGER, &CGetFingerDlg::OnBnClickedBtnLogoutFeinger)
	ON_BN_CLICKED(IDC_BTN_LOGIN_FEINGER, &CGetFingerDlg::OnBnClickedBtnLoginFeinger)
	ON_BN_CLICKED(IDC_BTN_COLLECTION, &CGetFingerDlg::OnBnClickedBtnCollection)

	ON_MESSAGE(WM_ACCESS_FINGERPRINT, &CGetFingerDlg::OnFingerPrintMessage)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_BTN_COLLECTIONDONE, &CGetFingerDlg::OnBnClickedBtnCollectiondone)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CGetFingerDlg 消息处理程序



BOOL CGetFingerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	g_SetWndStaticText(this);

	GetDlgItem(IDC_BTN_COLLECTION)->EnableWindow(FALSE);
	GetDlgItem(ID_BTN_COLLECTIONDONE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_LOGOUT_FEINGER)->EnableWindow(FALSE);

	SetDlgItemText(IDC_IPADDRESS_GETFINGER, _T("172.23.29.60"));
	SetDlgItemText(IDC_EDIT_PORT_GETFINGER, _T("37777"));
	SetDlgItemText(IDC_EDIT_USERNAME_FINGER, _T("admin"));
	SetDlgItemText(IDC_EDIT_PWD_GETFINGER, _T("admin123"));



	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CGetFingerDlg::OnBnClickedBtnLogoutFeinger()
{
	// TODO: 在此添加控件通知处理程序代码

	if (m_bListened && m_lLoginHandle)
	{
		CLIENT_StopListen(m_lLoginHandle);
		m_bListened = false;
		KillTimer(1);
	}

	if(0 != m_lLoginHandle)
	{
		CLIENT_Logout(m_lLoginHandle);
		m_lLoginHandle = 0;
	}

	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;
	m_bListened = false;

	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;

	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString(""));
	GetDlgItem(IDC_BTN_COLLECTION)->EnableWindow(FALSE);
	GetDlgItem(ID_BTN_COLLECTIONDONE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_LOGOUT_FEINGER)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_LOGIN_FEINGER)->EnableWindow(TRUE);
}

void CGetFingerDlg::OnBnClickedBtnLoginFeinger()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: 在此添加控件通知处理程序代码
	CString strIp;
	CString strUser;
	CString strPwd;
	int nPort;

	nPort = GetDlgItemInt(IDC_EDIT_PORT_GETFINGER);
	GetDlgItemText(IDC_IPADDRESS_GETFINGER, strIp);
	GetDlgItemText(IDC_EDIT_USERNAME_FINGER, strUser);
	GetDlgItemText(IDC_EDIT_PWD_GETFINGER, strPwd);

	char *pcIP = (LPSTR)(LPCSTR)strIp;
	char *pcUser = (LPSTR)(LPCSTR)strUser;
	char *pcPwssd = (LPSTR)(LPCSTR)strPwd;

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, pcIP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, pcPwssd, sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, pcUser, sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = nPort;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);
	LLONG lRet = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam);

	if(lRet != 0)
	{
		m_lLoginHandle = lRet;

		GetDlgItem(IDC_BTN_COLLECTION)->EnableWindow(TRUE);
		GetDlgItem(ID_BTN_COLLECTIONDONE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGIN_FEINGER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOGOUT_FEINGER)->EnableWindow(TRUE);
	}
	else
	{
		if(1 == stOutparam.nError)			MessageBox(ConvertString("Invalid password!"), ConvertString("Prompt"));
		else if(2 == stOutparam.nError)	MessageBox(ConvertString("Invalid account!"),ConvertString("Prompt"));
		else if(3 == stOutparam.nError)	MessageBox(ConvertString("Timeout!"), ConvertString("Prompt"));
		else if(4 == stOutparam.nError)	MessageBox(ConvertString("The user has logged in!"),ConvertString("Prompt"));
		else if(5 == stOutparam.nError)	MessageBox(ConvertString("The user has been locked!"), ConvertString("Prompt"));
		else if(6 == stOutparam.nError)	MessageBox(ConvertString("The user has listed into illegal!"), ConvertString("Prompt"));
		else if(7 == stOutparam.nError)	MessageBox(ConvertString("The system is busy!"),ConvertString( "Prompt"));
		else if(9 == stOutparam.nError)	MessageBox(ConvertString("You Can't find the network server!"), ConvertString("Prompt"));
		else	MessageBox(ConvertString("Login failed!"), ConvertString("Prompt"));
	}
}

void CGetFingerDlg::OnBnClickedBtnCollection()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bCaptureSuc = false;
	m_bStarted = false;
	m_bFinished = false;
	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText("");
	memset(m_byFingerprintData, 0, sizeof(m_byFingerprintData));
	m_nFingerprintLen = 0;
	if (!m_bListened)
	{
		m_bListened = CLIENT_StartListenEx(m_lLoginHandle) == TRUE ? true : false;

		if (false == m_bListened)
		{
			MessageBox(ConvertString("listen failed"), ConvertString("Prompt"));
			return;
		}
	}

	NET_CTRL_CAPTURE_FINGER_PRINT stuCapParam = {sizeof(stuCapParam)};
	stuCapParam.nChannelID = 0;
	strncpy(stuCapParam.szReaderID, "1", sizeof(stuCapParam.szReaderID)-1);
	bool bRet = CLIENT_ControlDevice(m_lLoginHandle, DH_CTRL_CAPTURE_FINGER_PRINT, &stuCapParam, 5000) == TRUE ? true : false;

	if (false == bRet)
	{
		MessageBox(ConvertString("start capture fingerprint failed"), ConvertString("Prompt"));	
		return ;
	}

	SetTimer(1, 30*1000, NULL);
	m_bStarted = true;
	GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Start Collection"));
	GetDlgItem(IDC_BTN_COLLECTION)->EnableWindow(FALSE);
}

LRESULT CGetFingerDlg::OnFingerPrintMessage(WPARAM wParam, LPARAM lParam)
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
				CLIENT_StopListen(m_lLoginHandle);
				m_bListened = false;
			}
			m_bCaptureSuc = true;
			m_bFinished = true;
			memcpy(m_byFingerprintData, pstFingerPrint->szFingerPrintInfo, nLen);
			m_nFingerprintLen = nLen;
			GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection Completed"));
			GetDlgItem(IDC_BTN_COLLECTION)->EnableWindow(TRUE);
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

void CGetFingerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (!m_bCaptureSuc)
	{
		m_bFinished = true;
		m_nFingerprintLen = 0;	
		memset(&m_byFingerprintData, 0, sizeof(m_byFingerprintData));
		GetDlgItem(IDC_STATIC_COLLECTIONSTATE)->SetWindowText(ConvertString("Collection failed"));
		GetDlgItem(IDC_BTN_COLLECTION)->EnableWindow(TRUE);
		KillTimer(1);
		if (m_bListened)
		{
			CLIENT_StopListen(m_lLoginHandle);
			m_bListened = false;
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void CGetFingerDlg::OnBnClickedBtnCollectiondone()
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

void CGetFingerDlg::OnDestroy()
{
	CDialog::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
	if (m_bListened && m_lLoginHandle)
	{
		CLIENT_StopListen(m_lLoginHandle);
		m_bListened = false;
		KillTimer(1);
	}
	if(0 != m_lLoginHandle)
	{
		CLIENT_Logout(m_lLoginHandle);
		m_lLoginHandle = 0;
	}
}

bool CGetFingerDlg::GetFingerPrintData(char *pFingerBuf, int nBufLen)
{
	if (false == m_bCaptureSuc || pFingerBuf == NULL || nBufLen < m_nFingerprintLen || m_nFingerprintLen == 0)
	{
		return false;
	}

	memcpy(pFingerBuf, m_byFingerprintData, m_nFingerprintLen);
	return true;
}


int CGetFingerDlg::GetFingerPrintLen()
{
	return m_nFingerprintLen;
}


BOOL CGetFingerDlg::PreTranslateMessage(MSG* pMsg)
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
