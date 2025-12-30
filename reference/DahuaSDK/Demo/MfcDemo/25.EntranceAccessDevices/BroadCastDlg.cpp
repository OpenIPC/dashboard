// BroadCastDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "BroadCastDlg.h"

// BroadCastDlg 对话框

IMPLEMENT_DYNAMIC(BroadCastDlg, CDialog)

BroadCastDlg::BroadCastDlg(CWnd* pParent /*=NULL*/,
						   LATTICE_SCREEN_ACT_TYPE LatticeScreenActTmp,
						   int SeletedRowtmp)
	: CDialog(BroadCastDlg::IDD, pParent)
{
	m_pParent = pParent;
	m_emBroadCastAct = LatticeScreenActTmp;
	m_nSeletedRow = SeletedRowtmp;
}

BroadCastDlg::~BroadCastDlg()
{
}

void BroadCastDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BROADCAST_TEXTTYPE, m_cmbTextType);
	DDX_Control(pDX, IDC_EDIT_BROADCAST_TEXT, m_edText);
}


BEGIN_MESSAGE_MAP(BroadCastDlg, CDialog)
	ON_BN_CLICKED(IDOK, &BroadCastDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// BroadCastDlg 消息处理程序

BOOL BroadCastDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// TODO:  在此添加额外的初始化
	m_cmbTextType.AddString(ConvertString("Ordinary"));
	m_cmbTextType.AddString(ConvertString("Plate number"));
	m_cmbTextType.AddString(ConvertString("Time"));
	m_cmbTextType.AddString(ConvertString("Number string"));
	m_cmbTextType.SetCurSel(0);

	CListCtrl* pBoxList = (CListCtrl*)m_pParent->GetDlgItem(IDC_LIST_BROADCAST);
	switch (m_emBroadCastAct)
	{
	case ACT_ADD:
		break;
	case ACT_MODIFY:
		{
			m_cmbTextType.SetCurSel(String2Index(pBoxList->GetItemText(m_nSeletedRow, 1), szBroadCastTextType, _countof(szBroadCastTextType)));
			m_edText.SetWindowText(_T(pBoxList->GetItemText(m_nSeletedRow, 2)));
		}
		break;
	default:
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void BroadCastDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();

	CListCtrl* pBoxList = (CListCtrl*)m_pParent->GetDlgItem(IDC_LIST_BROADCAST);
	CString csTmp;
	int SetRow = -1;
	switch (m_emBroadCastAct)
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
	m_cmbTextType.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 1, _T(csTmp));
	m_edText.GetWindowText(csTmp);
	pBoxList->SetItemText(SetRow, 2, _T(csTmp));
}
