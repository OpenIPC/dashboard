// RealAttch.cpp : 实现文件
//

#include "stdafx.h"
#include "PassengerFlow.h"
#include "RealAttch.h"


#define WM_USER_VIDEOSUMMARY_COME (WM_USER + 211)

// CRealAttch 对话框

IMPLEMENT_DYNAMIC(CRealAttch, CDialog)

CRealAttch::CRealAttch(CWnd* pParent /*=NULL*/)
	: CDialog(CRealAttch::IDD, pParent)
{
	m_nChannel = 0;
	m_lLoginId = 0;
	m_lAttachID = 0;
}

CRealAttch::~CRealAttch()
{
	if (m_lAttachID)
	{
		CLIENT_DetachVideoStatSummary(m_lAttachID);
		m_lAttachID = 0;
	}
}

void CRealAttch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ALARM_INFO, m_lctPeopleCounting);

}


BEGIN_MESSAGE_MAP(CRealAttch, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_REALATTACH, &CRealAttch::OnBnClickedButtonRealattach)
	ON_BN_CLICKED(IDC_BUTTON_REALDETACH, &CRealAttch::OnBnClickedButtonRealdetach)

	ON_MESSAGE(WM_USER_VIDEOSUMMARY_COME, &CRealAttch::OnVideoSummaryCome)
END_MESSAGE_MAP()


// CRealAttch 消息处理程序

void CRealAttch::InitCountingLct()
{
	m_lctPeopleCounting.SetExtendedStyle(m_lctPeopleCounting.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_lctPeopleCounting.SetExtendedStyle(m_lctPeopleCounting.GetExtendedStyle()|LVS_EX_GRIDLINES); 

	CRect rect;
	int width ;
	m_lctPeopleCounting.GetClientRect(&rect);
	width = rect.Width();

	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;

	CString strToConvert = ConvertString(_T("Channel"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 - 20;
	lvc.iSubItem = 0;
	m_lctPeopleCounting.InsertColumn(0, &lvc);

	strToConvert = ConvertString(_T("Total(Enters)"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 10;
	lvc.iSubItem = 1;
	m_lctPeopleCounting.InsertColumn(1, &lvc);

	strToConvert = ConvertString(_T("Today(Enters)"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 10 ;
	lvc.iSubItem = 2;
	m_lctPeopleCounting.InsertColumn(2, &lvc);

	strToConvert = ConvertString(_T("Total(Exits)"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 8;
	lvc.iSubItem = 3;
	m_lctPeopleCounting.InsertColumn(3, &lvc);

	strToConvert = ConvertString(_T("Today(Exits)"));
	lvc.pszText = (LPTSTR)(LPCTSTR)strToConvert;
	lvc.cx = width/5 + 8 ;
	lvc.iSubItem = 4;
	m_lctPeopleCounting.InsertColumn(4, &lvc);
}

BOOL CRealAttch::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	InitCountingLct();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void	CRealAttch::InitRealAttchDlg(int nChannelNum, LLONG lLogind)
{
	m_nChannel = 0;
	m_lLoginId = lLogind;

	for (int j = 0; j < nChannelNum; ++ j)
	{
		LV_ITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.pszText = _T("");
		lvi.iImage = 0;
		lvi.iItem = j;

		CString strChannel;
		strChannel.Format(_T("%d"),j+1);
		m_lctPeopleCounting.InsertItem(&lvi);
		m_lctPeopleCounting.SetItemText(j,0,strChannel); 
	}
	GetDlgItem(IDC_BUTTON_REALATTACH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_REALDETACH)->EnableWindow(FALSE);
}
void CRealAttch::OnBnClickedButtonRealattach()
{
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(_T("Please login device first!")), ConvertString(_T("Prompt")));
		return;
	}

	int tmp = m_nChannel;

	NET_IN_ATTACH_VIDEOSTAT_SUM inVideoSummary;
	inVideoSummary.dwSize = sizeof(inVideoSummary);
	inVideoSummary.dwUser = (LDWORD)this;
	inVideoSummary.nChannel = tmp;
	inVideoSummary.cbVideoStatSum = CRealAttch::VideoStatSumCallBackFunc;

	NET_OUT_ATTACH_VIDEOSTAT_SUM outVideoSummary = {sizeof(outVideoSummary)};
	LLONG lRet = CLIENT_AttachVideoStatSummary(m_lLoginId, &inVideoSummary, &outVideoSummary,3000);
	if (lRet == 0)
	{
		CString tmp;	
		tmp.Format(ConvertString(_T("subscribe to People Counting failed!")));
		MessageBox(tmp, ConvertString(_T("Prompt")));
		return;
	}
	GetDlgItem(IDC_BUTTON_REALATTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_REALDETACH)->EnableWindow(TRUE);
	m_lAttachID = lRet;
}

void CRealAttch::OnBnClickedButtonRealdetach()
{
	if (m_lAttachID)
	{
		CLIENT_DetachVideoStatSummary(m_lAttachID);
	}
	m_lAttachID = 0;

	GetDlgItem(IDC_BUTTON_REALATTACH)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_REALDETACH)->EnableWindow(FALSE);
}

void CRealAttch::SetChannelNum(int nChannel)
{
	m_nChannel = nChannel;
}

void CALLBACK CRealAttch::VideoStatSumCallBackFunc(LLONG lAttachHandle, NET_VIDEOSTAT_SUMMARY* pBuf,
														  DWORD dwBufLen, LDWORD dwUser)
{
	CRealAttch* pInstance = (CRealAttch*)dwUser;

	NET_VIDEOSTAT_SUMMARY* pSummaryInfo = new NET_VIDEOSTAT_SUMMARY;
	memcpy(pSummaryInfo, pBuf, dwBufLen);

	HWND hwnd = pInstance->GetSafeHwnd();

	::PostMessage(hwnd, WM_USER_VIDEOSUMMARY_COME, (WPARAM)(pSummaryInfo), 0);
}

LRESULT CRealAttch::OnVideoSummaryCome(WPARAM wParam, LPARAM lParam)
{
	NET_VIDEOSTAT_SUMMARY* pVideoSummaryInfo = (NET_VIDEOSTAT_SUMMARY* )wParam;

	this->VideoStatSummary(pVideoSummaryInfo);

	delete pVideoSummaryInfo;

	return 0;
}

void CRealAttch::VideoStatSummary(NET_VIDEOSTAT_SUMMARY* pSummaryInfo)
{
	CString strChannel;
	CString strInTotal;
	CString strOutTotal;
	CString strInToday;
	CString strOutToday;

	strChannel.Format(_T("%d"), pSummaryInfo->nChannelID + 1);
	strInTotal.Format(_T("%d"),pSummaryInfo->stuEnteredSubtotal.nTotal);
	strOutTotal.Format(_T("%d"), pSummaryInfo->stuExitedSubtotal.nTotal);


	strInToday.Format(_T("%d"), 
		pSummaryInfo->stuEnteredSubtotal.nToday);

	strOutToday.Format(_T("%d"), 
		pSummaryInfo->stuExitedSubtotal.nToday
		);

	int i = pSummaryInfo->nChannelID;

	m_lctPeopleCounting.SetItemText(i, 0, strChannel);
	m_lctPeopleCounting.SetItemText(i, 1, strInTotal);
	m_lctPeopleCounting.SetItemText(i, 2, strInToday);
	m_lctPeopleCounting.SetItemText(i, 3, strOutTotal);
	m_lctPeopleCounting.SetItemText(i, 4, strOutToday);
}

void CRealAttch::ExitDlg()
{
	OnBnClickedButtonRealdetach();
	m_lctPeopleCounting.DeleteAllItems();
	GetDlgItem(IDC_BUTTON_REALATTACH)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_REALDETACH)->EnableWindow(FALSE);
}
BOOL CRealAttch::PreTranslateMessage(MSG* pMsg)
{
	// Escape key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
