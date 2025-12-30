// DlgLocalBypass.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgLocalBypass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLocalBypass dialog


CDlgLocalBypass::CDlgLocalBypass(CWnd* pParent /*=NULL*/, BOOL bNeedMode, int nWantToSetChannelCount)
	: CDialog(CDlgLocalBypass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLocalBypass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bNeedMode = bNeedMode;
	m_nWantToSetChannelCount = nWantToSetChannelCount;
	for (int i = 0; i < 16; ++i)
	{	
		m_pemLocalMode[i] = NET_BYPASS_MODE_UNKNOW;
	}
	memset(m_pnLocalChannel, 0, sizeof(m_pnLocalChannel));

	m_nLastCount = 0;
	m_nActualCount = 0;
}


void CDlgLocalBypass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLocalBypass)
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLocalBypass, CDialog)
	//{{AFX_MSG_MAP(CDlgLocalBypass)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLocalBypass message handlers
BOOL CDlgLocalBypass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_LOCAL_BYPASS);
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
void CDlgLocalBypass::CreateWithModeDialog()
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i < 16; ++i)
	{
		if (i%8 == 0)
		{
			k = 0;
			++j;
		}
		char szIndex[32] = {0};
		_itoa(i + 1, szIndex, 10);
		cBtnChannel[i].Create(szIndex, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j+30*j, 60+50*k, 30+30*j+30*j), this, 100+i);
		cBtnMode[i].Create("", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j+30*j+30, 60+50*k, 60+30*j+30*j), this, 300+i);

		++k;
	}
	MoveWindow(0, 0, 420, 200+30*j);
	CenterWindow();
	CRect cRectTemp;
	GetDlgItem(IDOK)->GetClientRect(cRectTemp);
	GetDlgItem(IDOK)->MoveWindow(100, 120+30*j, cRectTemp.Width(), cRectTemp.Height());
	GetDlgItem(IDCANCEL)->GetClientRect(cRectTemp);
	GetDlgItem(IDCANCEL)->MoveWindow(220, 120+30*j, cRectTemp.Width(), cRectTemp.Height());
}
	
void CDlgLocalBypass::CreateWithoutModeDialog()
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i < 16; ++i)
	{
		if (i%8 == 0)
		{
			k = 0;
			++j;
		}
		char szIndex[32] = {0};
		_itoa(i + 1, szIndex, 10);
		cBtnChannel[i].Create(szIndex, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j, 60+50*k, 30+30*j), this, 100+i);
		++k;
	}
	MoveWindow(0, 0, 420, 130+30*j);
	CenterWindow();
	CRect cRectTemp;
	GetDlgItem(IDOK)->GetClientRect(cRectTemp);
	GetDlgItem(IDOK)->MoveWindow(100, 50+30*j, cRectTemp.Width(), cRectTemp.Height());
	GetDlgItem(IDCANCEL)->GetClientRect(cRectTemp);
	GetDlgItem(IDCANCEL)->MoveWindow(220, 50+30*j, cRectTemp.Width(), cRectTemp.Height());
}

void CDlgLocalBypass::OnOK() 
{
	// TODO: Add extra validation here
	for (int i = 0; i < 16; ++i)
	{
		if (BST_CHECKED == cBtnChannel[i].GetCheck())
		{
			m_pnLocalChannel[m_nActualCount] = i;
			++m_nActualCount;
			if (m_nActualCount >= m_nWantToSetChannelCount)
			{
				break;
			}
		}
	}
	CDialog::OnOK();
}

void CDlgLocalBypass::OnDestroy() 
{
	CDialog::OnDestroy();
	
}

void CDlgLocalBypass::StuToDlg()
{
	int nTempChannel = 0;
	for (int i = 0; i < m_nLastCount; ++i)
	{
		nTempChannel = m_pnLocalChannel[i];
		cBtnChannel[nTempChannel].SetCheck(BST_CHECKED);
		if (m_pemLocalMode[i] == NET_BYPASS_MODE_BYPASS)
		{		
			cBtnMode[nTempChannel].SetCheck(BST_CHECKED);
		}
	}
}