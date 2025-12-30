// FingerprintByUserIDDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Attendance.h"
#include "FingerprintByUserIDDlg.h"
#include "Util.h"
#include "FingerprintCollectionDlg.h"
#include "Base64.h"
// CFingerprintByUserIDDlg 对话框

IMPLEMENT_DYNAMIC(CFingerprintByUserIDDlg, CDialog)

CFingerprintByUserIDDlg::CFingerprintByUserIDDlg(CAttendanceManager *pManager, CString szUserID, CWnd* pParent /*=NULL*/)
	: CDialog(CFingerprintByUserIDDlg::IDD, pParent)
	, m_strUserID(_T(""))
{
	m_pManager = pManager;
	m_strUserID = szUserID;
}

CFingerprintByUserIDDlg::~CFingerprintByUserIDDlg()
{
}

void CFingerprintByUserIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lstFingerprint);
	DDX_Text(pDX, IDC_EDIT_FINGERPRINT_USERID, m_strUserID);
}


BEGIN_MESSAGE_MAP(CFingerprintByUserIDDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_BYUSERID, &CFingerprintByUserIDDlg::OnBnClickedButtonSearchByuserid)
	ON_BN_CLICKED(IDC_BUTTON_ADD_FINGER_BY_USERID, &CFingerprintByUserIDDlg::OnBnClickedButtonAddFingerByUserid)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_FINGER_BY_USERID, &CFingerprintByUserIDDlg::OnBnClickedButtonDeleteFingerByUserid)
//	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST1, &CFingerprintByUserIDDlg::OnLvnColumnclickList1)
//ON_WM_PAINT()
END_MESSAGE_MAP()


// CFingerprintByUserIDDlg 消息处理程序

BOOL CFingerprintByUserIDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	// TODO:  在此添加额外的初始化
	m_lstFingerprint.SetExtendedStyle(m_lstFingerprint.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_lstFingerprint.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 50, -1);
	m_lstFingerprint.InsertColumn(1, ConvertString("UserID"), LVCFMT_LEFT, 200, -1);
	m_lstFingerprint.InsertColumn(2, ConvertString("FingerprintID"), LVCFMT_LEFT, 100, -1);
	m_lstFingerprint.InsertColumn(3, ConvertString("FingerprintData"), LVCFMT_LEFT, 10000, -1);

	m_lstFingerprint.GetHeaderCtrl()->EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFingerprintByUserIDDlg::OnBnClickedButtonSearchByuserid()
{
	// TODO: 在此添加控件通知处理程序代码
	m_lstFingerprint.DeleteAllItems();

	NET_IN_FINGERPRINT_GETBYUSER stuInParam = {sizeof(stuInParam)};
	char *pcUserId = (LPSTR)(LPCSTR)m_strUserID;
	strncpy(stuInParam.szUserID, pcUserId, sizeof(stuInParam.szUserID)-1);

	NET_OUT_FINGERPRINT_GETBYUSER stuOutParam = {sizeof(stuOutParam)};
	stuOutParam.nMaxFingerDataLength = 100*1024;
	stuOutParam.pbyFingerData = (BYTE *)new char[stuOutParam.nMaxFingerDataLength];
	bool bRet = m_pManager->GetFingerByUserID(&stuInParam, &stuOutParam);
	if (!bRet)
	{
		MessageBox(ConvertString("No fingerprint data"), ConvertString("Prompt"));
		delete []stuOutParam.pbyFingerData;
		stuOutParam.pbyFingerData = NULL;
		return ;
	}
	else
	{
		if (stuOutParam.nRetFingerPrintCount == 0)
		{
			MessageBox(ConvertString("No fingerprint data"), ConvertString("Prompt"));
			delete []stuOutParam.pbyFingerData;
			stuOutParam.pbyFingerData = NULL;
			return ;
		}

		if (stuOutParam.nRetFingerPrintCount * stuOutParam.nSinglePacketLength > stuOutParam.nMaxFingerDataLength ||
			stuOutParam.nRetFingerDataLength > stuOutParam.nMaxFingerDataLength)
		{
			MessageBox(ConvertString("return data failed"), ConvertString("Prompt"));
			delete []stuOutParam.pbyFingerData;
			stuOutParam.pbyFingerData = NULL;
			return ;
		}

		for (int i = 0; i < stuOutParam.nRetFingerPrintCount; i++)
		{	
			int nIndex = m_lstFingerprint.InsertItem(i, NULL);
			CString strIndex;
			strIndex.Format("%d", i);
			m_lstFingerprint.SetItemText(nIndex, 0, strIndex);
			m_lstFingerprint.SetItemText(nIndex, 1, pcUserId);
			CString strFingerprintID;
			strFingerprintID.Format("%d", stuOutParam.nFingerPrintIDs[i]);
			m_lstFingerprint.SetItemText(nIndex, 2, strFingerprintID);

			CString strFinger = "";
			base64Encode((const char *)(stuOutParam.pbyFingerData + i * stuOutParam.nSinglePacketLength), stuOutParam.nSinglePacketLength, strFinger);
			m_lstFingerprint.SetItemText(nIndex, 3, strFinger);
			
		}
		delete []stuOutParam.pbyFingerData;
		stuOutParam.pbyFingerData = NULL;
	}
}

void CFingerprintByUserIDDlg::OnBnClickedButtonAddFingerByUserid()
{
	// TODO: 在此添加控件通知处理程序代码
	CFingerprintCollectionDlg *pdlg = new CFingerprintCollectionDlg(m_pManager);
	int nResponse = pdlg->DoModal();
	if (nResponse == IDOK)
	{
		int nFingerLen = pdlg->GetFingerPrintLen();
		if (nFingerLen <= 0)
		{
			MessageBox(ConvertString("no fingerprint data failed"), ConvertString("Prompt"));
			return;
		}

		char *pFingerprintData = new char[nFingerLen];
		if (NULL != pFingerprintData)
		{
			pdlg->GetFingerPrintData(pFingerprintData, nFingerLen);
			NET_IN_FINGERPRINT_INSERT_BY_USERID stuInInsert = {sizeof(stuInInsert)};
			char *pcUserID = (LPSTR)(LPCSTR)m_strUserID;
			strncpy(stuInInsert.szUserID, pcUserID, sizeof(stuInInsert.szUserID)-1);
			stuInInsert.nSinglePacketLen = nFingerLen;
			stuInInsert.nPacketCount = 1;
			stuInInsert.szFingerPrintInfo = pFingerprintData;
			NET_OUT_FINGERPRINT_INSERT_BY_USERID stuOutInsert = {sizeof(stuOutInsert)};
			bool bRet = m_pManager->InsertFingerByUserID(&stuInInsert, &stuOutInsert);
			if (bRet)
			{
				if (stuOutInsert.nFailedCode != 0)
				{
					if (stuOutInsert.nFailedCode == 2)
					{
						MessageBox(ConvertString("Add FingerPrint Failed, limits on the fingerprint capabilities of this user"), ConvertString("Prompt"));
					}
					else 
					{
						MessageBox(ConvertString("Add FingerPrint Failed, other error"), ConvertString("Prompt"));
					}
				}
				else
				{
					MessageBox(ConvertString("Add FingerPrint Successfully"), ConvertString("Prompt"));
				}
			}
			else
			{
				MessageBox(ConvertString("Add FingerPrint Failed"), ConvertString("Prompt"));	
			}
			delete []pFingerprintData;
			pFingerprintData = NULL;
		}
	}
	delete pdlg;
	pdlg = NULL;
}

void CFingerprintByUserIDDlg::OnBnClickedButtonDeleteFingerByUserid()
{
	// TODO: 在此添加控件通知处理程序代码
	NET_CTRL_IN_FINGERPRINT_REMOVE_BY_USERID stuInRemove = {sizeof(stuInRemove)};
	NET_CTRL_OUT_FINGERPRINT_REMOVE_BY_USERID stuOutRemove = {sizeof(stuOutRemove)};
	char *pcUserID = (LPSTR)(LPCSTR)m_strUserID;
	strncpy(stuInRemove.szUserID, pcUserID, sizeof(stuInRemove.szUserID)-1);
	bool bRet = m_pManager->RemoveFingerByUserID(&stuInRemove, &stuOutRemove);
	if (bRet)
	{
		MessageBox(ConvertString("Delete Fingerprint Successfully"), ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Delete Fingerprint Failed"), ConvertString("Prompt"));
	}
}

BOOL CFingerprintByUserIDDlg::PreTranslateMessage(MSG* pMsg)
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

//void CFingerprintByUserIDDlg::OnLvnColumnclickList1(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//	// TODO: 在此添加控件通知处理程序代码
//	*pResult = 0;
//
//	Invalidate(TRUE);
//	UpdateWindow();
//}

//void CFingerprintByUserIDDlg::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//	// TODO: 在此处添加消息处理程序代码
//	// 不为绘图消息调用 CDialog::OnPaint()
//}
