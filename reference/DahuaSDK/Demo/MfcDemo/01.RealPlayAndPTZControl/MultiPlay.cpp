// MultiPlay.cpp : implementation file
//

#include "stdafx.h"
#include "RealPlayAndPTZControl.h"
#include "MultiPlay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiPlay dialog


CMultiPlay::CMultiPlay(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiPlay::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiPlay)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nMultiPlay = 0;
	m_nChannel = 0;
	m_nChannelCount = 0;
}


void CMultiPlay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiPlay)
	DDX_Control(pDX, IDC_COMBO_BEGINCHANNEL, m_comboChannel);
	DDX_Control(pDX, IDC_COMBO_MULTI, m_comboMulti);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiPlay, CDialog)
	//{{AFX_MSG_MAP(CMultiPlay)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiPlay message handlers

void CMultiPlay::OnOK() 
{
	// TODO: Add extra validation here
	int nIndex = m_comboMulti.GetCurSel();
	if(CB_ERR != nIndex)
	{
		m_nMultiPlay = m_comboMulti.GetItemData(nIndex);
	}
	nIndex = m_comboChannel.GetCurSel();
	if(CB_ERR != nIndex)
	{
		m_nChannel = m_comboChannel.GetItemData(nIndex);
	}
	CDialog::OnOK();
}

//Set dropdown menu 
void CMultiPlay::SetMultiPlayDlgInfo(int nChannelCount)
{
	m_nChannelCount = nChannelCount;
}

BOOL CMultiPlay::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	// TODO: Add extra initialization here
	//Channel dropdown menu 
	m_comboChannel.ResetContent();
	int i = 0;
	int nIndex = 0;
	for(i=0;i<m_nChannelCount;i++)
	{
		CString str;
		str.Format("%d",i);
		nIndex = m_comboChannel.AddString(str);
		m_comboChannel.SetItemData(nIndex,i);
	}
	if(0 < m_comboChannel.GetCount())
	{
		m_comboChannel.SetCurSel(0);
	}
	//Video amount dropdown menu 
	m_comboMulti.ResetContent();
	nIndex = m_comboMulti.AddString(ConvertString("Single"));
	m_comboMulti.SetItemData(nIndex,1);
	nIndex = m_comboMulti.AddString(ConvertString("Split-4"));
	m_comboMulti.SetItemData(nIndex,4);
	nIndex = m_comboMulti.AddString(ConvertString("Split-8"));
	m_comboMulti.SetItemData(nIndex,8);
	nIndex = m_comboMulti.AddString(ConvertString("Split-9"));
	m_comboMulti.SetItemData(nIndex,9);
	nIndex = m_comboMulti.AddString(ConvertString("Split-16"));
	m_comboMulti.SetItemData(nIndex,16);
	if(0 < m_comboMulti.GetCount())
	{
		m_comboMulti.SetCurSel(0);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}




















































































































































































































































































































































