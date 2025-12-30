// LatticeScreenDlg2.cpp : 实现文件
//

#include "StdAfx.h"
#include "EntranceAccessDevices.h"
#include "LatticeScreenDlg2.h"
#include "ScreenShowDlg.h"
#include "BroadCastDlg.h"
#include <vector>

// LatticeScreenDlg2 对话框

IMPLEMENT_DYNAMIC(LatticeScreenDlg2, CDialog)

LatticeScreenDlg2::LatticeScreenDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(LatticeScreenDlg2::IDD, pParent)
{
	m_nSeletedRowS = -1;
	m_nSeletedRowB = -1;
}

LatticeScreenDlg2::~LatticeScreenDlg2()
{
}

void LatticeScreenDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SCREENSHOW, m_ListScreenShow);
	DDX_Control(pDX, IDC_LIST_BROADCAST, m_ListBroadCast);
}


BEGIN_MESSAGE_MAP(LatticeScreenDlg2, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SCREENSHOW_ADD, &LatticeScreenDlg2::OnBnClickedButtonScreenshowAdd)
	ON_BN_CLICKED(IDC_BUTTON_ISSUE, &LatticeScreenDlg2::OnBnClickedButtonIssue)
	ON_BN_CLICKED(IDC_BUTTON_SCREENSHOW_MODIFY, &LatticeScreenDlg2::OnBnClickedButtonScreenshowModify)
	ON_NOTIFY(NM_CLICK, IDC_LIST_SCREENSHOW, &LatticeScreenDlg2::OnNMClickListScreenshow)
	ON_BN_CLICKED(IDC_BUTTON_SCREENSHOW_DELETE, &LatticeScreenDlg2::OnBnClickedButtonScreenshowDelete)
	ON_BN_CLICKED(IDC_BUTTON_BROADCAST_ADD, &LatticeScreenDlg2::OnBnClickedButtonBroadcastAdd)
	ON_NOTIFY(NM_CLICK, IDC_LIST_BROADCAST, &LatticeScreenDlg2::OnNMClickListBroadcast)
	ON_BN_CLICKED(IDC_BUTTON_BROADCAST_MODIFY, &LatticeScreenDlg2::OnBnClickedButtonBroadcastModify)
	ON_BN_CLICKED(IDC_BUTTON_BROADCAST_DELETE, &LatticeScreenDlg2::OnBnClickedButtonBroadcastDelete)
END_MESSAGE_MAP()


// LatticeScreenDlg2 消息处理程序
#ifndef _endof
#define _endof(array) (array + _countof(array))
#endif

#ifndef _rangeof
#define _rangeof(array) array, _endof(array)
#endif

BOOL LatticeScreenDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	// TODO: 在此添加额外的初始化代码	
	CRect rectScreenShow;
	CRect rectBroadCast;

	// 获取编程语言列表视图控件的位置和大小   
	m_ListScreenShow.GetClientRect(&rectScreenShow);
	m_ListBroadCast.GetClientRect(&rectBroadCast);

	// 为列表视图控件添加全行选中和栅格风格   
	m_ListScreenShow.SetExtendedStyle(m_ListScreenShow.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ListBroadCast.SetExtendedStyle(m_ListBroadCast.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 为列表视图控件添加列   
	int columnScreenShow = 7; //总列数
	m_ListScreenShow.InsertColumn(0, ConvertString(_T("Index")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 0);
	m_ListScreenShow.InsertColumn(1, ConvertString(_T("ScreenNo")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 1);
	m_ListScreenShow.InsertColumn(2, ConvertString(_T("TextType")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 2);
	m_ListScreenShow.InsertColumn(3, ConvertString(_T("TextColor")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 3);
	m_ListScreenShow.InsertColumn(4, ConvertString(_T("TextRollMode")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 4);
	m_ListScreenShow.InsertColumn(5, ConvertString(_T("RollSpeed")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 5);
	m_ListScreenShow.InsertColumn(6, ConvertString(_T("Text")), LVCFMT_CENTER, rectScreenShow.Width() / columnScreenShow, 6);

	int columnBroadCast = 3; //总列数
	m_ListBroadCast.InsertColumn(0, ConvertString(_T("Index")), LVCFMT_CENTER, rectBroadCast.Width() / columnBroadCast, 0);
	m_ListBroadCast.InsertColumn(1, ConvertString(_T("TextType")), LVCFMT_CENTER, rectBroadCast.Width() / columnBroadCast, 1);
	m_ListBroadCast.InsertColumn(2, ConvertString(_T("Voice text")), LVCFMT_CENTER, rectBroadCast.Width() / columnBroadCast, 2);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void LatticeScreenDlg2::OnBnClickedButtonScreenshowAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_ListScreenShow.GetItemCount() > 15)
	{
		MessageBox(ConvertString("Maximum ScreenShowInfo limit (16) exceeded!"), ConvertString("Prompt"));
	}
	else
	{
		ScreenShowDlg dlg(this,ACT_ADD);
		dlg.DoModal();
	}
}

int String2Index(CString cstrText, const char* const szText[], int count)
{
	std::vector< CString> szTextChn;
	for (int i = 0; i < count; i++) //vs2005不支持批量初始化
	{ 
		szTextChn.push_back(ConvertString(szText[i]));
	}
	for (size_t i = 0; i < szTextChn.size(); i++)
	{
		if (cstrText == szTextChn[i])
		{
			return (int)i;
		}
	}

	return -1;
}

void LatticeScreenDlg2::OnBnClickedButtonIssue()
{
	// TODO: 在此添加控件通知处理程序代码
	NET_IN_SET_PARK_CONTROL_INFO stuInSetParkControlInfo = {sizeof(stuInSetParkControlInfo)};
	NET_OUT_SET_PARK_CONTROL_INFO stuOutSetParkControlInfo = {sizeof(stuOutSetParkControlInfo)};

	UINT nScreenShowCount = m_ListScreenShow.GetItemCount();
	stuInSetParkControlInfo.nScreenShowInfoNum = __min(nScreenShowCount, _countof(stuInSetParkControlInfo.stuScreenShowInfo));
	for (UINT i = 0; i < nScreenShowCount; i++)
	{
		stuInSetParkControlInfo.stuScreenShowInfo[i].nScreenNo = _ttoi(m_ListScreenShow.GetItemText(i, 1));
		stuInSetParkControlInfo.stuScreenShowInfo[i].emTextType = (EM_SCREEN_TEXT_TYPE)String2Index(m_ListScreenShow.GetItemText(i, 2), szScreenShowTextType, _countof(szScreenShowTextType));
		stuInSetParkControlInfo.stuScreenShowInfo[i].emTextColor = (EM_SCREEN_TEXT_COLOR)String2Index(m_ListScreenShow.GetItemText(i, 3), szTextColor, _countof(szTextColor));
		stuInSetParkControlInfo.stuScreenShowInfo[i].emTextRollMode = (EM_SCREEN_TEXT_ROLL_MODE)String2Index(m_ListScreenShow.GetItemText(i, 4), szTextRollMode, _countof(szTextRollMode));
		stuInSetParkControlInfo.stuScreenShowInfo[i].nRollSpeed = _ttoi(m_ListScreenShow.GetItemText(i, 5));
		memcpy(stuInSetParkControlInfo.stuScreenShowInfo[i].szText, m_ListScreenShow.GetItemText(i, 6), m_ListScreenShow.GetItemText(i, 6).GetLength());
	}

	UINT nBroadCastCount = m_ListBroadCast.GetItemCount();
	stuInSetParkControlInfo.nBroadcastInfoNum = __min(nBroadCastCount,_countof(stuInSetParkControlInfo.stuScreenShowInfo));
	for (UINT i = 0; i < nBroadCastCount; i++)
	{
		stuInSetParkControlInfo.stuBroadcastInfo[i].emTextType = (EM_BROADCAST_TEXT_TYPE)String2Index(m_ListBroadCast.GetItemText(i, 1), szBroadCastTextType, _countof(szBroadCastTextType));
		memcpy(stuInSetParkControlInfo.stuBroadcastInfo[i].szText, m_ListBroadCast.GetItemText(i, 2), m_ListBroadCast.GetItemText(i, 2).GetLength());
	}

	BOOL bret = Device::GetInstance().SetParkControlInfo(&stuInSetParkControlInfo, &stuOutSetParkControlInfo);
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Issuing is completed!"), ConvertString("Prompt"));
	}

}

void LatticeScreenDlg2::OnNMClickListScreenshow(NMHDR *pNMHDR, LRESULT *pResult)
{
	m_nSeletedRowS = ((NMLISTVIEW*)pNMHDR)->iItem;

	*pResult = 0;
}

void LatticeScreenDlg2::OnBnClickedButtonScreenshowModify()
{
	// TODO: 在此添加控件通知处理程序代码
	if(1 > m_ListScreenShow.GetSelectedCount())
	{
		MessageBox(ConvertString("Please select an item!"), ConvertString("Prompt"));
		return;
	}
	else
	{
		ScreenShowDlg dlg(this, ACT_MODIFY, m_nSeletedRowS);
		dlg.DoModal();
	}
}

void LatticeScreenDlg2::OnBnClickedButtonScreenshowDelete()
{
	// TODO: 在此添加控件通知处理程序代码
	int nItemCount = m_ListScreenShow.GetItemCount();
	if (1 > m_ListScreenShow.GetSelectedCount())
		return;
	for (int i = nItemCount - 1; i >= 0; i--)
	{
		if (m_ListScreenShow.GetItemState(i, LVIS_SELECTED) != 0)
		{
			m_ListScreenShow.DeleteItem(i);
		}
	}
	
	//更新序号
	for (int i = 0;i < nItemCount; i++)
	{				  
		CString Indextmp;
		Indextmp.Format(_T("%d"), i);  //Use Unicode Character Set
		m_ListScreenShow.SetItemText(i, 0, _T(Indextmp));
	}

}

void LatticeScreenDlg2::OnBnClickedButtonBroadcastAdd()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_ListBroadCast.GetItemCount() > 15)
	{
		MessageBox(ConvertString("Maximum BroadCastInfo limit (16) exceeded!"), ConvertString("Prompt"));
	}
	else
	{
		BroadCastDlg dlg(this,ACT_ADD);
		dlg.DoModal();
	}
}

void LatticeScreenDlg2::OnNMClickListBroadcast(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	m_nSeletedRowB = ((NMLISTVIEW*)pNMHDR)->iItem;

	*pResult = 0;
}

void LatticeScreenDlg2::OnBnClickedButtonBroadcastModify()
{
	// TODO: 在此添加控件通知处理程序代码
	if(1 > m_ListBroadCast.GetSelectedCount())
	{
		MessageBox(ConvertString("Please select an item!"), ConvertString("Prompt"));
		return;
	}
	else
	{
		BroadCastDlg dlg(this, ACT_MODIFY, m_nSeletedRowB);
		dlg.DoModal();
	}
}

void LatticeScreenDlg2::OnBnClickedButtonBroadcastDelete()
{
	// TODO: 在此添加控件通知处理程序代码
	int nItemCount = m_ListBroadCast.GetItemCount();
	if (1 > m_ListBroadCast.GetSelectedCount())
		return;
	for (int i = nItemCount - 1; i >= 0; i--)
	{
		if (m_ListBroadCast.GetItemState(i, LVIS_SELECTED) != 0)
		{
			m_ListBroadCast.DeleteItem(i);
		}
	}

	//更新序号
	for (int i = 0;i < nItemCount; i++)
	{				  
		CString Indextmp;
		Indextmp.Format(_T("%d"), i);  //Use Unicode Character Set
		m_ListBroadCast.SetItemText(i, 0, _T(Indextmp));
	}
}
