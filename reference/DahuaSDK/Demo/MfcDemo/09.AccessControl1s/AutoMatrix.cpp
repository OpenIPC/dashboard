// AutoMatrix.cpp 
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "AutoMatrix.h"

IMPLEMENT_DYNAMIC(AutoMatrix, CDialog)

AutoMatrix::AutoMatrix(CWnd* pParent /*=NULL*/)
	: CDialog(AutoMatrix::IDD, pParent)
{

}

AutoMatrix::~AutoMatrix()
{

}

void AutoMatrix::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_AUTOREBOOTDAY, m_comAutoRebootDay);
	DDX_Control(pDX, IDC_COMBO_AUTOREBOOTTIME, m_comAutoRebootTime);
	DDX_Control(pDX, IDC_COMBO_AUTODELETEFILESTIME, m_comAutoDeleteFileTime);
}


BEGIN_MESSAGE_MAP(AutoMatrix, CDialog)
	ON_BN_CLICKED(IDC_AUTOMATRIX_BTN_GET, &AutoMatrix::OnBnClickedAutomatrixBtnGet)
	ON_BN_CLICKED(IDC_AUTOMATRIX_BTN_SET, &AutoMatrix::OnBnClickedAutomatrixBtnSet)
END_MESSAGE_MAP()


BOOL AutoMatrix::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_AUTOMATRIX_BTN_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUTOMATRIX_BTN_SET)->EnableWindow(FALSE);
	}
	else
	{
		OnBnClickedAutomatrixBtnGet();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void AutoMatrix::InitDlg()
{
	int i = 0;
	// channel
	m_comAutoRebootDay.ResetContent();
	m_comAutoRebootDay.InsertString(-1, ConvertString("Never"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EveryDay"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EverySunday"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EveryMonday"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EveryTuesday"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EveryWednesday"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EveryThursday"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EveryFriday"));
	m_comAutoRebootDay.InsertString(-1, ConvertString("EverySaturday"));
	m_comAutoRebootDay.SetCurSel(0);

	m_comAutoRebootTime.ResetContent();
	for (i = 0; i < 24; i++)
	{
		CString csInfo;
		csInfo.Format("%d:00",i);
		m_comAutoRebootTime.InsertString(-1, csInfo);
	}
	m_comAutoRebootTime.SetCurSel(0);

	m_comAutoDeleteFileTime.ResetContent();
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("Never"));
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("24H"));
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("48H"));
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("72H"));
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("96H"));
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("One Week"));
	m_comAutoDeleteFileTime.InsertString(-1, ConvertString("One Month"));
	m_comAutoDeleteFileTime.SetCurSel(0);

}

void AutoMatrix::OnBnClickedAutomatrixBtnGet()
{
	DHDEV_AUTOMT_CFG stuAutoMT = {sizeof(stuAutoMT)};
	BOOL bret = Device::GetInstance().GetAutoMatrix(stuAutoMT);
	if (bret)
	{
		m_comAutoRebootDay.SetCurSel(stuAutoMT.byAutoRebootDay);
		m_comAutoRebootTime.SetCurSel(stuAutoMT.byAutoRebootTime);
		m_comAutoDeleteFileTime.SetCurSel(stuAutoMT.byAutoDeleteFilesTime);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void AutoMatrix::OnBnClickedAutomatrixBtnSet()
{
	DHDEV_AUTOMT_CFG stuAutoMT = {sizeof(stuAutoMT)};
	stuAutoMT.byAutoRebootDay = m_comAutoRebootDay.GetCurSel();
	stuAutoMT.byAutoRebootTime = m_comAutoRebootTime.GetCurSel();
	stuAutoMT.byAutoDeleteFilesTime = m_comAutoDeleteFileTime.GetCurSel();
	BOOL bret = Device::GetInstance().SetAutoMatrix(&stuAutoMT);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}
