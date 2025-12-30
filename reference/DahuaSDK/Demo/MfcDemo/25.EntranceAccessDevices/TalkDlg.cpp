// TalkDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "EntranceAccessDevicesDlg.h"
#include "TalkDlg.h"

IMPLEMENT_DYNAMIC(TalkDlg, CDialog)

TalkDlg::TalkDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TalkDlg::IDD, pParent)
{
	hParentWnd = pParent->GetSafeHwnd();
	m_bRecordStatus = FALSE;
	m_hTalkHandle = 0;
}

TalkDlg::~TalkDlg()
{

}

void TalkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TalkDlg, CDialog)

	ON_BN_CLICKED(IDC_BUTTON_BEGINTALK, &TalkDlg::OnBnClickedButtonBegintalk)
	ON_BN_CLICKED(IDC_BUTTON_STOPTALK, &TalkDlg::OnBnClickedButtonStoptalk)



	ON_MESSAGE(WM_EVENTALARM_INVITE, InviteEvent)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_CALLEVENT, &TalkDlg::OnBnClickedCheckCallevent)
END_MESSAGE_MAP()

BOOL TalkDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	if (Device::GetInstance().GetLoginState())
	{
		GetDlgItem(IDC_BUTTON_BEGINTALK)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOPTALK)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CALLBACK AudioDataCallBack(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag, LDWORD dwUser)
{
	if(dwUser == 0)
	{
		return;
	}
	TalkDlg *dlg = (TalkDlg *)dwUser;
	dlg->AudioData(lTalkHandle, pDataBuf,dwBufSize,byAudioFlag);
}

void TalkDlg::AudioData(LLONG lTalkHandle, char *pDataBuf, DWORD dwBufSize, BYTE byAudioFlag)
{
	Device::GetInstance().AudioData(lTalkHandle,pDataBuf, dwBufSize, byAudioFlag);
}

void TalkDlg::OnBnClickedButtonBegintalk()
{
	//	First set audio encode format in audio talk 
	BOOL bret = Device::GetInstance().StartTalk(AudioDataCallBack, (LDWORD)this);
	if (!bret)
	{
		GetDlgItem(IDC_BUTTON_BEGINTALK)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOPTALK)->EnableWindow(FALSE);
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
	else
	{
		GetDlgItem(IDC_BUTTON_BEGINTALK)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_STOPTALK)->EnableWindow(TRUE);
	}
}

void TalkDlg::OnBnClickedButtonStoptalk()
{
	BOOL bret = Device::GetInstance().StopTalk();
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}
	else
	{
		GetDlgItem(IDC_BUTTON_BEGINTALK)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_STOPTALK)->EnableWindow(FALSE);
	}
}



void TalkDlg::OnDestroy()
{
	CDialog::OnDestroy();

	::PostMessage(hParentWnd,WM_LISTENINVITE,0,0);

	Device::GetInstance().StopTalk();
}

LRESULT TalkDlg::InviteEvent(WPARAM wParam, LPARAM lParam)
{
	int bret = MessageBox(ConvertString("answer the call?"),ConvertString("Prompt"),MB_OKCANCEL);
	if (bret == 1)
	{
		if (m_hTalkHandle == 0 && !m_bRecordStatus)
		{
			OnBnClickedButtonBegintalk();
		}
	}
	return 0;
}
void TalkDlg::OnBnClickedCheckCallevent()
{
	int nCheck = ((CButton*)GetDlgItem(IDC_CHECK_CALLEVENT))->GetCheck();
	::PostMessage(hParentWnd,WM_LISTENINVITE,nCheck,0);
}
