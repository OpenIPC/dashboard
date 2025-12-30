// AudioBoardcastDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "AudioBoardcastDlg.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioBoardcastDlg dialog


CAudioBoardcastDlg::CAudioBoardcastDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioBoardcastDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioBoardcastDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAudioBoardcastDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioBoardcastDlg)
	DDX_Control(pDX, IDC_LIST_DEVICE, m_lbDevice);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioBoardcastDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioBoardcastDlg)
	ON_BN_CLICKED(IDC_BTN_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, OnBtnDel)
	ON_BN_CLICKED(IDC_BTN_START, OnBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioBoardcastDlg message handlers

BOOL CAudioBoardcastDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this, DLG_TALK_BOARDCAST);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioBoardcastDlg::OnBtnAdd() 
{
	CLoginDlg dlg;
	if (dlg.DoModal() == IDOK)
	{
		BOOL bRet = FALSE;
		LLONG lLoginID = dlg.GetLoginHandle();
		
		NET_TALK_EX stuTalkInfo = { sizeof(NET_TALK_EX) };
		strncpy(stuTalkInfo.szMulticastAddr, "234.5.6.7", sizeof(stuTalkInfo.szMulticastAddr) - 1);
		stuTalkInfo.wMulticastLocalPort = 56789;
		stuTalkInfo.wMulticastRemotePort = 54321;
		CLIENT_SetDeviceMode(lLoginID, DH_TALK_MODE3, &stuTalkInfo);

		DHDEV_TALKDECODE_INFO stuAudioFmt;
		memset(&stuAudioFmt, 0, sizeof(stuAudioFmt));
		stuAudioFmt.encodeType = DH_TALK_PCM;
		stuAudioFmt.dwSampleRate = 16000;
		stuAudioFmt.nAudioBit = 16;
		stuAudioFmt.nPacketPeriod = 0;	
		CLIENT_SetDeviceMode(lLoginID, DH_TALK_ENCODE_TYPE, &stuAudioFmt);

		if (CLIENT_AudioBroadcastAddDev(lLoginID))
		{
			if (CLIENT_RecordStartEx(lLoginID))
			{
				bRet = TRUE;
				int nIndex = m_lbDevice.AddString(dlg.GetDeviceAddr());
				m_lbDevice.SetItemDataPtr(nIndex, (void*)lLoginID);
				m_lbDevice.SetCurSel(nIndex);
			}
			else
			{
				CLIENT_AudioBroadcastDelDev(lLoginID);
			}
		}
		
		if (!bRet)
		{
			CLIENT_Logout(lLoginID);
			MessageBox(ConvertString("Add broadcast device error.", DLG_TALK_BOARDCAST), ConvertString("Prompt"));
		}
	}
}

void CAudioBoardcastDlg::OnBtnDel() 
{
	int nIndex = m_lbDevice.GetCurSel();
	if (nIndex >= 0)
	{
		LLONG lLoginID = (LLONG)m_lbDevice.GetItemDataPtr(nIndex);

		CLIENT_RecordStopEx(lLoginID);
		CLIENT_AudioBroadcastDelDev(lLoginID);
		CLIENT_Logout(lLoginID);

		m_lbDevice.DeleteString(nIndex);
		int nCount = m_lbDevice.GetCount();
		if (nIndex >= nCount)
		{
			nIndex = nCount - 1;
		}
		m_lbDevice.SetCurSel(nIndex);
	}
}

void CAudioBoardcastDlg::OnBtnStart() 
{

}

void CAudioBoardcastDlg::OnBtnStop() 
{
	// TODO: Add your control notification handler code here
	
}

void CAudioBoardcastDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	int nCount = m_lbDevice.GetCount();
	for (int i = nCount - 1; i >= 0; --i)
	{
		LLONG lLoginID = (LLONG)m_lbDevice.GetItemDataPtr(i);
		CLIENT_AudioBroadcastDelDev(lLoginID);
		CLIENT_Logout(lLoginID);
	}	
}
