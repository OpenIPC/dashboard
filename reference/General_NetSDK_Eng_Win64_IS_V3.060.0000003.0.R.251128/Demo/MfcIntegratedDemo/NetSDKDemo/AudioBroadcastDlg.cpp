// AudioBroadcastDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "AudioBroadcastDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioBroadcastDlg dialog


CAudioBroadcastDlg::CAudioBroadcastDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioBroadcastDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioBroadcastDlg)
	m_bRecord = FALSE;
	//}}AFX_DATA_INIT
}


void CAudioBroadcastDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioBroadcastDlg)
	DDX_Control(pDX, IDC_LIST_BROADCASTDEV, m_broadcastDevListCtrl);
	DDX_Control(pDX, IDC_LIST_ALLDEV, m_allDevListCtrl);
	DDX_Check(pDX, IDC_CHECK_RECORDEN, m_bRecord);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioBroadcastDlg, CDialog)
	//{{AFX_MSG_MAP(CAudioBroadcastDlg)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnAdd)
	ON_BN_CLICKED(IDC_BUTTON_ADDALL, OnAddall)
	ON_BN_CLICKED(IDC_BUTTON_DEL, OnDel)
	ON_BN_CLICKED(IDC_BUTTON_DELALL, OnDelall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioBroadcastDlg message handlers

BOOL CAudioBroadcastDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);

	//初始化设备和通道列表
	m_allDevListCtrl.SetExtendedStyle(m_allDevListCtrl.GetExtendedStyle()|LVS_EX_FULLROWSELECT);
	m_allDevListCtrl.InsertColumn(0, ConvertString(NAME_UNBROADCASTDEV), LVCFMT_LEFT, 120, 0);
	m_broadcastDevListCtrl.InsertColumn(0, ConvertString(NAME_BROADCASTDEV), LVCFMT_LEFT, 120, 0);
	
	UpDataDevList();

	this->UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAudioBroadcastDlg::OnOK() 
{
	// TODO: Add extra validation here
	this->UpdateData();
	
	CDialog::OnOK();
}

void CAudioBroadcastDlg::OnCancel() 
{
	// TODO: Add extra cleanup here

	CDialog::OnCancel();
}

void CAudioBroadcastDlg::OnAdd() 
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_allDevListCtrl.GetFirstSelectedItemPosition();
	int iItemIndex = -1;
	int indexd = 0;
	while (pos)
	{
		iItemIndex = m_allDevListCtrl.GetNextSelectedItem(pos);
		MoveDevAll2Bc(iItemIndex - indexd);
		indexd++;
	}
	UpDataDevList();
}

void CAudioBroadcastDlg::OnAddall() 
{
	// TODO: Add your control notification handler code here
	int count = m_allDevListCtrl.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		MoveDevAll2Bc(0);
	}
	UpDataDevList();
}

void CAudioBroadcastDlg::OnDel() 
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_broadcastDevListCtrl.GetFirstSelectedItemPosition();
	int iItemIndex = -1;
	int indexd = 0;
	while (pos)
	{
		iItemIndex = m_broadcastDevListCtrl.GetNextSelectedItem(pos);
		MoveDevBc2All(iItemIndex - indexd);
		indexd++;
	}
	UpDataDevList();
}

void CAudioBroadcastDlg::OnDelall() 
{
	// TODO: Add your control notification handler code here
	int count = m_broadcastDevListCtrl.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		MoveDevBc2All(0);
	}
	UpDataDevList();
}

void CAudioBroadcastDlg::MoveDevAll2Bc(int index)
{
	int count = 0;
	list<DeviceNode *>::iterator it = m_AllDevList.begin();
	for (; it != m_AllDevList.end(); it++)
	{
		if (count < index)
		{
			count++;
			continue;
		}
		else if (count == index)
		{
			m_BroadcastDevList.push_back(*it);
			m_AllDevList.erase(it);
		}
		else
		{
			MessageBox(ConvertString("Something is wrong!"));
		}
		break;
	}
}

void CAudioBroadcastDlg::MoveDevBc2All(int index)
{
	int count = 0;
	list<DeviceNode *>::iterator it = m_BroadcastDevList.begin();
	for (; it != m_BroadcastDevList.end(); it++)
	{
		if (count < index)
		{
			count++;
			continue;
		}
		else if (count == index)
		{
			m_AllDevList.push_back(*it);
			m_BroadcastDevList.erase(it);
		}
		else
		{
			MessageBox(ConvertString("Something is wrong!"));
		}
		break;
	}
}
void CAudioBroadcastDlg::UpDataDevList()
{
	int iItemNum = 0;
	CString csName;
	m_allDevListCtrl.DeleteAllItems();
	m_broadcastDevListCtrl.DeleteAllItems();
	{
		list<DeviceNode *>::iterator it = m_AllDevList.begin();
		for (; it != m_AllDevList.end(); it++)
		{
			csName.Format("%s(%s)", (*it)->IP, (*it)->Name);
			m_allDevListCtrl.InsertItem(iItemNum, csName);
			iItemNum++;
		}
	}
	iItemNum = 0;
	{
		list<DeviceNode *>::iterator it = m_BroadcastDevList.begin();
		for (; it != m_BroadcastDevList.end(); it++)
		{
			csName.Format("%s(%s)", (*it)->IP, (*it)->Name);
			m_broadcastDevListCtrl.InsertItem(iItemNum, csName);
			iItemNum++;
		}
	}
}
