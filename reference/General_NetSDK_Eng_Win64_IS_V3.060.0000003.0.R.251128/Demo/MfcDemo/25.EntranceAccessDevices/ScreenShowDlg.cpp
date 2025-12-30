// ScreenShowDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "ScreenShowDlg.h"

// ScreenShowDlg 对话框

IMPLEMENT_DYNAMIC(ScreenShowDlg, CDialog)

ScreenShowDlg::ScreenShowDlg(CWnd* pParent /*=NULL*/,
							 LATTICE_SCREEN_ACT_TYPE LatticeScreenActTmp,
							 int SeletedRowtmp)
	: CDialog(ScreenShowDlg::IDD, pParent)
{
	m_pParent = pParent;
	m_emScreenShowAct = LatticeScreenActTmp;
	m_nSeletedRow = SeletedRowtmp;
}

ScreenShowDlg::~ScreenShowDlg()
{
}

void ScreenShowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SCREENSHOW_SCREENNO, m_edScreenNo);
	DDX_Control(pDX, IDC_COMBO_SCREENSHOW_TEXTTYPE, m_cmbTextType);
	DDX_Control(pDX, IDC_COMBO_SCREENSHOW_TEXTCOLOR, m_cmbTextColor);
	DDX_Control(pDX, IDC_COMBO_SCREENSHOW_TEXTROLLMODE, m_cmbTextRollMode);
	DDX_Control(pDX, IDC_COMBO_SCREENSHOW_ROLLSPEED, m_cmbRollSpeed);
	DDX_Control(pDX, IDC_EDIT_SCREENSHOW_TEXT, m_edText);
}


BEGIN_MESSAGE_MAP(ScreenShowDlg, CDialog)
	ON_BN_CLICKED(IDOK, &ScreenShowDlg::OnBnClickedOk)
END_MESSAGE_MAP()



// ScreenShowDlg 消息处理程序

BOOL ScreenShowDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// TODO:  在此添加额外的初始化
	m_edScreenNo.SetWindowTextA(_T("0"));

	m_cmbTextType.AddString(ConvertString("Ordinary"));
	m_cmbTextType.AddString(ConvertString("Local time"));
	m_cmbTextType.AddString(ConvertString("Qr code"));
	m_cmbTextType.AddString(ConvertString("RESOURCE"));
	m_cmbTextType.SetCurSel(0);

	m_cmbTextColor.AddString(ConvertString("Green"));
	m_cmbTextColor.AddString(ConvertString("Red"));
	m_cmbTextColor.AddString(ConvertString("Yellow"));
	m_cmbTextColor.AddString(ConvertString("White"));
	m_cmbTextColor.SetCurSel(0);

	m_cmbTextRollMode.AddString(ConvertString("No scrolling"));
	m_cmbTextRollMode.AddString(ConvertString("Scroll left and right"));
	m_cmbTextRollMode.AddString(ConvertString("Page up and down"));
	m_cmbTextRollMode.SetCurSel(0);

	m_cmbRollSpeed.AddString(ConvertString("1"));
	m_cmbRollSpeed.AddString(ConvertString("2"));
	m_cmbRollSpeed.AddString(ConvertString("3"));
	m_cmbRollSpeed.AddString(ConvertString("4"));
	m_cmbRollSpeed.AddString(ConvertString("5"));
	m_cmbRollSpeed.SetCurSel(0);

	CListCtrl* pBoxList = (CListCtrl*)m_pParent->GetDlgItem(IDC_LIST_SCREENSHOW);
	switch (m_emScreenShowAct)
	{
	case ACT_ADD:
		break;
	case ACT_MODIFY:
		{
			m_edScreenNo.SetWindowText(_T(pBoxList->GetItemText(m_nSeletedRow, 1)));
			m_cmbTextType.SetCurSel(String2Index(pBoxList->GetItemText(m_nSeletedRow, 2),szScreenShowTextType, _countof(szScreenShowTextType)));
			m_cmbTextColor.SetCurSel(String2Index(pBoxList->GetItemText(m_nSeletedRow, 3),szTextColor, _countof(szTextColor)));
			m_cmbTextRollMode.SetCurSel(String2Index(pBoxList->GetItemText(m_nSeletedRow, 4),szTextRollMode, _countof(szTextRollMode)));
			m_cmbRollSpeed.SetCurSel(_ttoi(pBoxList->GetItemText(m_nSeletedRow, 5)) - 1);
			m_edText.SetWindowText(_T(pBoxList->GetItemText(m_nSeletedRow, 6)));
		}
		break;
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void ScreenShowDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();

	CListCtrl* pBoxList = (CListCtrl*)m_pParent->GetDlgItem(IDC_LIST_SCREENSHOW);
	CString csTmp;
	int SetRow = -1;
	switch (m_emScreenShowAct)
	{
	case ACT_ADD:
		  {
			  SetRow = pBoxList->GetItemCount();
			  CString cstrIndex;
			  cstrIndex.Format(_T("%d"), pBoxList->GetItemCount());  //Use Unicode Character Set
			  pBoxList->InsertItem(SetRow, cstrIndex);
		  }
		break;
	case ACT_MODIFY:
		  {
			  SetRow = m_nSeletedRow;
		  }
		  break;
	default:
		break;
	}

	//更新父窗口List
	m_edScreenNo.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 1, _T(csTmp));
	m_cmbTextType.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 2, _T(csTmp));
	m_cmbTextColor.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 3, _T(csTmp));
	m_cmbTextRollMode.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 4, _T(csTmp));
	m_cmbRollSpeed.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 5, _T(csTmp));
	m_edText.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 6, _T(csTmp));
}
