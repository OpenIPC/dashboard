// AddGroupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TargetRecognition.h"
#include "AddGroupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddGroupDlg dialog


CAddGroupDlg::CAddGroupDlg(const LLONG lLoginHandle, const int nOpreateType, const NET_FACERECONGNITION_GROUP_INFO *pstGroupInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CAddGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddGroupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_lLoginID = lLoginHandle;
	//memset(m_szGroupId, 0, sizeof(m_szGroupId));
	m_nOpreateType = nOpreateType;
	memset(&m_stuGroupInfo, 0, sizeof(m_stuGroupInfo));
	m_stuGroupInfo.dwSize = sizeof(m_stuGroupInfo);
	if (pstGroupInfo)
	{
		strncpy(m_szGroupName, pstGroupInfo->szGroupName, sizeof(m_szGroupName) - 1);
		memcpy(&m_stuGroupInfo, pstGroupInfo, sizeof(m_stuGroupInfo));
	}
}

BOOL CAddGroupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	if (1 == m_nOpreateType) // add
	{
		SetWindowText(ConvertString("Add Face Group"));
	}
	else if (2 == m_nOpreateType) // edit
	{
		SetWindowText(ConvertString("Modify Face Group"));
	}
	SetDlgItemText(IDC_EDIT_ADD_GROUP_NAME,m_stuGroupInfo.szGroupName);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAddGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddGroupDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddGroupDlg, CDialog)
	//{{AFX_MSG_MAP(CAddGroupDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CAddGroupDlg::OnBnClickedOk)
END_MESSAGE_MAP()

void CAddGroupDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddGroupDlg::OnBnClickedOk()
{
	NET_IN_OPERATE_FACERECONGNITION_GROUP stuInParam = {sizeof(stuInParam)};
	NET_OUT_OPERATE_FACERECONGNITION_GROUP stuOutParam = {sizeof(stuOutParam)};

	NET_ADD_FACERECONGNITION_GROUP_INFO stuAddGroupInfo = {sizeof(stuAddGroupInfo)};
	NET_MODIFY_FACERECONGNITION_GROUP_INFO stuEditGroupInfo = {sizeof(stuEditGroupInfo)};

	CString strGroupName;
	GetDlgItemText(IDC_EDIT_ADD_GROUP_NAME, strGroupName);
	char *pcGroupName = (LPSTR)(LPCSTR)strGroupName;

	if (m_nOpreateType == 1) // add a group
	{
		stuInParam.emOperateType = NET_FACERECONGNITION_GROUP_ADD;
		stuAddGroupInfo.stuGroupInfo.dwSize = sizeof(stuAddGroupInfo.stuGroupInfo);
		stuAddGroupInfo.stuGroupInfo.emFaceDBType = NET_FACE_DB_TYPE_BLACKLIST;
		strncpy(stuAddGroupInfo.stuGroupInfo.szGroupName, pcGroupName, sizeof(stuAddGroupInfo.stuGroupInfo.szGroupName)-1);
		stuInParam.pOPerateInfo = &stuAddGroupInfo;
	}
	else if (m_nOpreateType == 2) // modify a group
	{
		stuInParam.emOperateType = NET_FACERECONGNITION_GROUP_MODIFY;
		stuEditGroupInfo.stuGroupInfo.dwSize = sizeof(stuEditGroupInfo.stuGroupInfo);
		stuEditGroupInfo.stuGroupInfo.emFaceDBType = NET_FACE_DB_TYPE_BLACKLIST;
		strncpy(stuEditGroupInfo.stuGroupInfo.szGroupName, pcGroupName, sizeof(stuEditGroupInfo.stuGroupInfo.szGroupName)-1);
		strncpy(stuEditGroupInfo.stuGroupInfo.szGroupId, m_stuGroupInfo.szGroupId, sizeof(stuEditGroupInfo.stuGroupInfo.szGroupId)-1);
		stuInParam.pOPerateInfo = &stuEditGroupInfo;
	}

	BOOL bRet = CLIENT_OperateFaceRecognitionGroup(m_lLoginID, &stuInParam, &stuOutParam, DEFAULT_WAIT_TIME);
	if (!bRet)
	{
		if (1 == m_nOpreateType)
		{
			MessageBox(ConvertString("Failed to add group!"), "");
		}
		else if (2 == m_nOpreateType)
		{
			MessageBox(ConvertString("Failed to modify this group!"), "");
		}
		return;
	}

	strncpy(m_szGroupName, stuEditGroupInfo.stuGroupInfo.szGroupName, sizeof(m_szGroupName)-1);
	CDialog::OnOK();
}

BOOL CAddGroupDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
