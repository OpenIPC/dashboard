// NameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveLoginDemo.h"
#include "NameDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNameDlg dialog


CNameDlg::CNameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNameDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNameDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNameDlg, CDialog)
	//{{AFX_MSG_MAP(CNameDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CNameDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNameDlg message handlers

BOOL CNameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this,DLG_NAME);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CNameDlg::OnBnClickedOk()
{
	UpdateData();
	m_strName.Replace(_T(" "),_T(""));
	if(m_strName.IsEmpty())
	{
		MessageBox(ConvertString("Name is Empty!",DLG_NAME), ConvertString("Prompt"));
		return;
	}
	UpdateData(FALSE);
	OnOK();
}
