// DlgExAlarmHandler.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgExAlarmHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmHandler dialog


CDlgExAlarmHandler::CDlgExAlarmHandler(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgExAlarmHandler::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgExAlarmHandler)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ZeroMemory(m_dwExAlarmOutMask, sizeof(m_dwExAlarmOutMask));
}


void CDlgExAlarmHandler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgExAlarmHandler)
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgExAlarmHandler, CDialog)
	//{{AFX_MSG_MAP(CDlgExAlarmHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmHandler message handlers

BOOL CDlgExAlarmHandler::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
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
		cBtnExAlarmOut[i].Create(szIndex, WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|BS_DEFPUSHBUTTON, CRect(10+50*k, 30*j, 60+50*k, 30+30*j), this, 100+i);
		++k;
	}
	MoveWindow(0, 0, 1050, 130+30*j);
	CenterWindow();
	CRect cRectTemp;
	GetDlgItem(IDOK)->GetClientRect(cRectTemp);
	GetDlgItem(IDOK)->MoveWindow(350, 50+30*j, cRectTemp.Width(), cRectTemp.Height());
	GetDlgItem(IDCANCEL)->GetClientRect(cRectTemp);
	GetDlgItem(IDCANCEL)->MoveWindow(470, 50+30*j, cRectTemp.Width(), cRectTemp.Height());
	
	StuToDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgExAlarmHandler::OnOK() 
{
	// TODO: Add extra validation here
	DlgToStu();
	CDialog::OnOK();
}

void CDlgExAlarmHandler::StuToDlg()
{
	for (int i = 0; i < MAX_CHANNEL_COUNT*sizeof(DWORD)*8; i++)
	{
		int nRow = i / (sizeof(DWORD)*8);
		int nCol = i % (sizeof(DWORD)*8);
		if (m_dwExAlarmOutMask[nRow] & (0x1 << nCol))
		{
			cBtnExAlarmOut[i].SetCheck(BST_CHECKED);
		}
		else
		{
			cBtnExAlarmOut[i].SetCheck(BST_UNCHECKED);
		}
	}
}

void CDlgExAlarmHandler::DlgToStu()
{
	ZeroMemory(m_dwExAlarmOutMask, sizeof(m_dwExAlarmOutMask));
	for (int i = 0; i < MAX_CHANNEL_COUNT*sizeof(DWORD)*8; i++)
	{
		int nRow = i / (sizeof(DWORD)*8);
		int nCol = i % (sizeof(DWORD)*8);
		
		if (BST_CHECKED == cBtnExAlarmOut[i].GetCheck())
		{
			m_dwExAlarmOutMask[nRow] |= 0x1 << nCol;
		}	
	}
}