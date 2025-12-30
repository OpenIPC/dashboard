// SubDlgSensorInfoDescription.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "SubDlgSensorInfoDescription.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubDlgSensorInfoDescription dialog


CSubDlgSensorInfoDescription::CSubDlgSensorInfoDescription(CWnd* pParent /* = NULL */, int nMaxSensor /* = 72 */, int nColumn /* = 8 */)
	: CDialog(CSubDlgSensorInfoDescription::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSubDlgSensorInfoDescription)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	nMaxSensor <= 0 ? (m_nMaxSensor = 72) : (m_nMaxSensor = nMaxSensor);
    nColumn <= 0 ? (m_nColumn = 8) : (m_nColumn = nColumn);
	m_pbtnCheck = NULL;
}


void CSubDlgSensorInfoDescription::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSubDlgSensorInfoDescription)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSubDlgSensorInfoDescription, CDialog)
	//{{AFX_MSG_MAP(CSubDlgSensorInfoDescription)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSubDlgSensorInfoDescription private method

void CSubDlgSensorInfoDescription::InitDlg()
{
	int nCfgCount = m_nMaxSensor;
	int nColumnMaxCount = m_nColumn;
	int nRowMaxCount = (nCfgCount%nColumnMaxCount == 0 ? nCfgCount/nColumnMaxCount : (nCfgCount/nColumnMaxCount + 1));
	int nBtnWidth = 150;	// 100 * 1.5 
	int nBtnHeight = 20;	// 10 * 1.5
	int nInterval = 6;
	
	{		
		CRect rcBtn;
		GetDlgItem(IDOK)->GetClientRect(&rcBtn);
		
		int nWndWidth = nBtnWidth * nColumnMaxCount + nInterval * (nColumnMaxCount + 1);
		int nWndHeight = nBtnHeight * nRowMaxCount + nInterval * (nRowMaxCount + 1)
			+ (rcBtn.Height()*2 + nInterval * 4);
		MoveWindow(0, 0, nWndWidth, nWndHeight);
		
		GetDlgItem(IDOK)->MoveWindow(nWndWidth / 2 - rcBtn.Width() - nInterval, nWndHeight - rcBtn.Height()*2 - nInterval * 2, 
			rcBtn.Width(), rcBtn.Height());
		GetDlgItem(IDCANCEL)->MoveWindow(nWndWidth / 2 + nInterval, nWndHeight - rcBtn.Height()*2 - nInterval * 2,
			rcBtn.Width(), rcBtn.Height());
		
		CenterWindow();
	}
	
	m_pbtnCheck = new CButton[nCfgCount];
	assert(m_pbtnCheck);
	
	for (int i = 0; i < nCfgCount; i++)
	{
		m_pbtnCheck[i].Create("", WS_CHILD | WS_VISIBLE | /*BS_CHECKBOX |*/ BS_AUTOCHECKBOX, 
			CRect(0, 0, 0, 0), this, emBtnID_base + i);

		CString csInfo;
		csInfo.Format("%s%d", ConvertString("Channel"), i + 1);
		m_pbtnCheck[i].SetWindowText(csInfo);

		if (IsSet(i))
		{
			m_pbtnCheck[i].SetCheck(BST_CHECKED);
		}
		
		int nSpaceX = i - i/nColumnMaxCount * nColumnMaxCount;
		int nSpaceY = (i - i%nColumnMaxCount) / nColumnMaxCount;//i - i/nRowMaxCount * nRowMaxCount;
		
		m_pbtnCheck[i].MoveWindow(nInterval * (1 + nSpaceX) + nBtnWidth * nSpaceX, 
			nInterval * (1 + nSpaceY) + nBtnHeight * nSpaceY, 
			nBtnWidth, nBtnHeight, TRUE);
	}
}

void CSubDlgSensorInfoDescription::SetID(const std::vector<int>& vecID)
{	
	m_vecChannel = vecID;
}

const std::vector<int>& CSubDlgSensorInfoDescription::GetID()
{
	return m_vecChannel;
}

BOOL CSubDlgSensorInfoDescription::IsSet(int nChannel)
{
	std::vector<int>::iterator it = std::find(m_vecChannel.begin(), m_vecChannel.end(), nChannel);
	if (it != m_vecChannel.end())
	{
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CSubDlgSensorInfoDescription message handlers

BOOL CSubDlgSensorInfoDescription::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	InitDlg();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSubDlgSensorInfoDescription::OnOK() 
{
	// TODO: Add extra validation here
	m_vecChannel.clear();
	for (int i = 0; i < m_nMaxSensor; i++)
	{
		if (m_pbtnCheck[i].GetCheck())
		{
			m_vecChannel.push_back(i);
		}
	}
	CDialog::OnOK();
}

void CSubDlgSensorInfoDescription::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_pbtnCheck)
	{
		for (int i = 0; i < m_nMaxSensor; i++)
		{
			if (m_pbtnCheck[i].GetSafeHwnd())
			{
				m_pbtnCheck[i].DestroyWindow();
			}
		}
		delete []m_pbtnCheck;
		m_pbtnCheck = NULL;
	}
}
