// DlgExtendedBypass.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgExtendedBypass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExtendedBypass dialog

CDlgExtendedBypass::CDlgExtendedBypass(CWnd* pParent, BOOL bNeedMode, int nWantToSetChannelCount)
	: CDialog(CDlgExtendedBypass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgExtendedBypass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bNeedMode = bNeedMode;
 	m_nWantToSetChannelCount = nWantToSetChannelCount;
	memset(m_nExtendedChannel, 0, sizeof(m_nExtendedChannel));
	for (int i = 0; i < MAX_CHANNEL_COUNT*sizeof(DWORD)*8; ++i)
	{	
		m_emExtendedMode[i] = NET_BYPASS_MODE_UNKNOW;
	}

	m_nLastCount = 0;
	m_nActualCount = 0;
}


void CDlgExtendedBypass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgExtendedBypass)
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgExtendedBypass, CDialog)
	//{{AFX_MSG_MAP(CDlgExtendedBypass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExtendedBypass message handlers
BOOL CDlgExtendedBypass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_EXTENDED_BYPASS);
	// TODO: Add extra initialization here
	if (m_bNeedMode)
	{
		CreateWithModeDialog();
		StuToDlg();
	} 
	else
	{
		CreateWithoutModeDialog();
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExtendedBypass::OnOK() 
{
	// TODO: Add extra validation here
	DlgToStu();
	CDialog::OnOK();
}

void CDlgExtendedBypass::StuToDlg()
{
	int nTempChannel = 0;
	for (int i = 0; i < m_nLastCount; ++i)
	{
		nTempChannel = m_nExtendedChannel[i];
		if (m_emExtendedMode[i] == NET_BYPASS_MODE_BYPASS)
		{
			cBtnExtendedMode[nTempChannel].SetCheck(BST_CHECKED);
		}
	}
}

void CDlgExtendedBypass::DlgToStu()
{
	for (int i = 0; i < MAX_CHANNEL_COUNT*sizeof(DWORD)*8; i++)
	{		
		if (BST_CHECKED == cBtnExtendedChannel[i].GetCheck())
		{
			m_nExtendedChannel[m_nActualCount] = i;
			++m_nActualCount;
			if (m_nActualCount >= m_nWantToSetChannelCount)
			{
				break;
			}
		}	
	}
}
void CDlgExtendedBypass::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CDlgExtendedBypass::CreateWithModeDialog()
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i < MAX_CHANNEL_COUNT*sizeof(DWORD)*8; ++i)
	{
		if (i%20 == 0)
		{
			k = 0;
			++j;
		}
		char szIndex[32] = {0};
		_itoa(i + 1, szIndex, 10);
		cBtnExtendedChannel[i].Create(szIndex, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j+30*j, 60+50*k, 30+30*j+30*j), this, 100+i);
		cBtnExtendedMode[i].Create("", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j+30*j+30, 60+50*k, 60+30*j+30*j), this, 600+i);
		++k;
	}
	MoveWindow(0, 0, 1050, 550+30*j);
	CenterWindow();
	CRect cRectTemp;
	GetDlgItem(IDOK)->GetClientRect(cRectTemp);
	GetDlgItem(IDOK)->MoveWindow(350, 470+30*j, cRectTemp.Width(), cRectTemp.Height());
	GetDlgItem(IDCANCEL)->GetClientRect(cRectTemp);
	GetDlgItem(IDCANCEL)->MoveWindow(470, 470+30*j, cRectTemp.Width(), cRectTemp.Height());
}

void CDlgExtendedBypass::CreateWithoutModeDialog()
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i < MAX_CHANNEL_COUNT*sizeof(DWORD)*8; ++i)
	{
		if (i%20 == 0)
		{
			k = 0;
			++j;
		}
		char szIndex[32] = {0};
		_itoa(i + 1, szIndex, 10);
		cBtnExtendedChannel[i].Create(szIndex, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j, 60+50*k, 30+30*j), this, 100+i);
		++k;
	}
	MoveWindow(0, 0, 1050, 130+30*j);
	CenterWindow();
	CRect cRectTemp;
	GetDlgItem(IDOK)->GetClientRect(cRectTemp);
	GetDlgItem(IDOK)->MoveWindow(350, 50+30*j, cRectTemp.Width(), cRectTemp.Height());
	GetDlgItem(IDCANCEL)->GetClientRect(cRectTemp);
	GetDlgItem(IDCANCEL)->MoveWindow(470, 50+30*j, cRectTemp.Width(), cRectTemp.Height());
}
