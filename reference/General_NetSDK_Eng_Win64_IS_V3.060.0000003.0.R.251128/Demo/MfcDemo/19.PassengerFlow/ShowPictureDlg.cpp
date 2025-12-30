// ShowPictureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PassengerFlow.h"
#include "ShowPictureDlg.h"


// CShowPictureDlg 对话框

IMPLEMENT_DYNAMIC(CShowPictureDlg, CDialog)

CShowPictureDlg::CShowPictureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShowPictureDlg::IDD, pParent)
{

}

CShowPictureDlg::~CShowPictureDlg()
{
}

void CShowPictureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_showPicture);
}


BEGIN_MESSAGE_MAP(CShowPictureDlg, CDialog)
	ON_STN_CLICKED(IDC_STATIC_PICTURE, &CShowPictureDlg::OnStnClickedStaticShowpic)
END_MESSAGE_MAP()


// CShowPictureDlg 消息处理程序

void CShowPictureDlg::OnStnClickedStaticShowpic()
{
	// TODO: 在此添加控件通知处理程序代码
	ShowWindow(FALSE);
}


BOOL CShowPictureDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	g_SetWndStaticText(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

BOOL CShowPictureDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CShowPictureDlg::ShowPicture(BYTE* data, DWORD size)
{
	m_showPicture.SetImageDate(data, size);
}