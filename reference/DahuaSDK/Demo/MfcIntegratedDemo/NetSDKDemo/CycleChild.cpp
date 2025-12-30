  // CycleChild.cpp : implementation file

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "CycleChild.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCycleChild dialog
 
CCycleChild::CCycleChild(CWnd* pParent /*=NULL*/)
	: CDialog(CCycleChild::IDD, pParent)
{
	memset(&m_SplitInfo, 0, sizeof(SplitInfoNode));
	m_cmParam = NULL;
	bHaveBadValue = FALSE;
	//{{AFX_DATA_INIT(CCycleChild)
	m_cycleinterval = 0;
	//}}AFX_DATA_INIT

	m_screenNo = 0;

	m_curRow = 0;

	m_pMain = NULL;
}

void CCycleChild::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCycleChild)
	DDX_Control(pDX, IDC_SPIN1, m_spin);
	DDX_Control(pDX, IDC_CHANNELSEL_LIST, m_childlist);
	DDX_Text(pDX, IDC_EDIT1, m_cycleinterval);
	DDV_MinMaxUInt(pDX, m_cycleinterval, 5, 36000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCycleChild, CDialog)
	//{{AFX_MSG_MAP(CCycleChild)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCycleChild message handlers

void CCycleChild::SetScreenNo(int scrNo)
{
	m_screenNo = scrNo;
}

BOOL CCycleChild::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	m_curRow = 0;

	m_childlist.SetExtendedStyle(m_childlist.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	m_childlist.InsertColumn(0, ConvertString(NAME_DEVICE), LVCFMT_LEFT, 120, 0);
	m_childlist.InsertColumn(1, ConvertString(NAME_CHANNEL), LVCFMT_LEFT, 60, 1);

	m_spin.SetBuddy((CEdit *)GetDlgItem(IDC_EDIT1));
	m_spin.SetRange32( 5, 36000);
	m_spin.SetPos(50);

	((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1))->EnableWindow(FALSE);
	((CEdit *)GetDlgItem(IDC_EDIT1))->EnableWindow(FALSE);
	m_cycleinterval = 10;
	SetDlgItemInt(IDC_EDIT1, m_cycleinterval, FALSE);

	m_pMain = (CNetSDKDemoDlg *)AfxGetMainWnd();
	BOOL bGetInfo = m_pMain->GetSplitInfo_Main(m_screenNo, &m_SplitInfo);
	if (!bGetInfo)
	{
		AfxMessageBox(ConvertString("error while getting split information"));
		return TRUE;
	}
	RefreshList();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//刷新已有列表
void CCycleChild::RefreshList()
{
	if (m_SplitInfo.Type == SPLIT_TYPE_CYCLEMONITOR) 
	{
		//显示于列表中
		DeviceNode *nDev;
		CString strDev;
		CString strChl;

		SplitCycleParam *cm_Param = (SplitCycleParam *)m_SplitInfo.Param;
		if (!m_cmParam)
		{
			m_cmParam = new SplitCycleParam;
		}
		memcpy(m_cmParam, cm_Param, sizeof(SplitCycleParam));
		m_cmParam->pChannelList = new CPtrList(sizeof(CycleChannelInfo));
		
		POSITION nPos = cm_Param->pChannelList->GetHeadPosition();
		for (int y = 0; y < cm_Param->iChannelCount; y++ )
		{
			CycleChannelInfo *chInfo = (CycleChannelInfo *)cm_Param->pChannelList->GetNext(nPos);
			
			CycleChannelInfo *newNode = new CycleChannelInfo;  //for local variate: parameter
			memcpy(newNode, chInfo, sizeof(CycleChannelInfo));
			m_cmParam->pChannelList->AddTail(newNode);

			CycleChannelInfo *newItem = new CycleChannelInfo;  //for list item
			memcpy(newItem, newNode, sizeof(CycleChannelInfo));
			nDev = (DeviceNode *)newItem->dwDeviceID;  
			strDev.Format(" %s (%s):",nDev->Name, nDev->IP);
			strChl.Format("chl %02d", newItem->iChannel + 1);
			m_childlist.InsertItem(m_curRow, strDev);
			m_childlist.SetItemText(m_curRow, 1, strChl);
			m_childlist.SetItemData(m_curRow, (DWORD)newItem);
			m_curRow++;
		}
	
		//显示轮循时间间隔
		m_cycleinterval = cm_Param->iInterval;
		SetDlgItemInt(IDC_EDIT1, m_cycleinterval, FALSE);
		((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1))->EnableWindow(TRUE);
		((CEdit *)GetDlgItem(IDC_EDIT1))->EnableWindow(TRUE);
	}
}

void CCycleChild::InsertItem(int row, CString item)
{
	m_childlist.InsertItem(row, item);
	m_curRow++;
}

void CCycleChild::SetItemText(int row, int column, CString text)
{
	m_childlist.SetItemText(row, column, text);
}

void CCycleChild::InitList(DWORD data)
{
	m_SplitInfo.Type = SPLIT_TYPE_CYCLEMONITOR;
	if (!m_cmParam)
	{
		m_cmParam = new SplitCycleParam;
	}
	memset(m_cmParam, 0, sizeof(SplitCycleParam));
	
	m_cmParam->pChannelList = new CPtrList(sizeof(CycleChannelInfo));
	CycleChannelInfo *tempnode = new CycleChannelInfo;
	memcpy(tempnode, (CycleChannelInfo *)data, sizeof(CycleChannelInfo));
	
	m_cmParam->pChannelList->AddTail(tempnode);
	m_cmParam->iChannelCount++;
	//轮循间隔初始化为10秒
	m_cmParam->iInterval = 10;
}

void CCycleChild::SetItemData(int row, DWORD data)
{
	m_childlist.SetItemData(row, data);
	if (!m_cmParam)
	{
		//初始化该窗口的轮循列表
		InitList(data);
		((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1))->EnableWindow(TRUE);
		((CEdit *)GetDlgItem(IDC_EDIT1))->EnableWindow(TRUE);
		return;
	}
	else
	{
		//添加列表节点
		CycleChannelInfo *tempnode = new CycleChannelInfo;
		memcpy(tempnode, (CycleChannelInfo *)data, sizeof(CycleChannelInfo));

		m_cmParam->pChannelList->AddTail(tempnode);
		m_cmParam->iChannelCount++;
	}
}

int CCycleChild::GetCurRow()
{
	return m_curRow;
}

int CCycleChild::GetSelectionMark()
{
	return m_childlist.GetSelectionMark();
}

DWORD CCycleChild::GetItemData(int row)
{
	return m_childlist.GetItemData(row);
}

CString CCycleChild::GetItemText(int row, int column)
{
	return m_childlist.GetItemText(row, column);
}

void CCycleChild::DeleteItem(int index)
{
	m_childlist.DeleteItem(index);
	m_curRow--;	
	if (m_cmParam && m_cmParam->iChannelCount == 1)
	{
		//只剩一个节点的情况
		delete (CycleChannelInfo *)m_cmParam->pChannelList->GetTail();
		m_cmParam->pChannelList->RemoveAll();
		delete m_cmParam->pChannelList;
		delete m_cmParam;
		m_cmParam = NULL;
		
		((CSpinButtonCtrl *)GetDlgItem(IDC_SPIN1))->EnableWindow(FALSE);
		((CEdit *)GetDlgItem(IDC_EDIT1))->EnableWindow(FALSE);
	}
	else if (m_cmParam)
	{
		//删除之
		POSITION nPos = m_cmParam->pChannelList->FindIndex(index);
		delete (CycleChannelInfo *)m_cmParam->pChannelList->GetAt(nPos);
		m_cmParam->pChannelList->RemoveAt(nPos);
		m_cmParam->iChannelCount--;
	}
	
	m_childlist.SetFocus();
}

void CCycleChild::OnOK()
{
	bHaveBadValue = FALSE;
	BOOL bRet = UpdateData(TRUE);
	if (!bRet)
	{
		bHaveBadValue = TRUE;
	}
}

void CCycleChild::OnCancel()
{
	//释放内存
	for (int j = 0; j < m_curRow; j++)
	{
		delete (CycleChannelInfo *)m_childlist.GetItemData(j);
	}
	if (m_cmParam) 
	{ 
		int count = m_cmParam->pChannelList->GetCount();
		for (int i = 0; i < count; i++)
		{
			delete (CycleChannelInfo *)m_cmParam->pChannelList->GetTail();
			m_cmParam->pChannelList->RemoveTail();
		}
		delete m_cmParam->pChannelList;
		delete m_cmParam;
	}
}

void CCycleChild::DoCycle()
{
	//释放内存
	for (int j = 0; j < m_curRow; j++)
	{
		delete (CycleChannelInfo *)m_childlist.GetItemData(j);
	}

	//输出轮循信息
	if (m_cmParam) 
	{
		//读取用户设置的时间间隔		
		BOOL ret = FALSE;
		m_cmParam->iInterval = m_cycleinterval;
		ret = m_pMain->ProcessCloseScreen(m_screenNo);
		if (!ret)
		{
			MessageBox(ConvertString("check this out!"));
		}
		
		//将本地信息回写到全局变量里
		m_SplitInfo.Param = m_cmParam;
		ret = m_pMain->SetSplitInfo_Main(m_screenNo, &m_SplitInfo);
		if (!ret)
		{
			MessageBox(ConvertString("check that out!"));
		}
	}
	//原先在轮循，现在列表被清空的情况
	else if (m_SplitInfo.Type == SPLIT_TYPE_CYCLEMONITOR)
	{
		BOOL ret = m_pMain->ProcessCloseScreen(m_screenNo);
		if (!ret)
		{
			MessageBox(ConvertString("plz check this out!"));
		}
	}
}