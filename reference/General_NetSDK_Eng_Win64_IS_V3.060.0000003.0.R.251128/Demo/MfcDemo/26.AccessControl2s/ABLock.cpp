// ABLock.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "ABLock.h"

IMPLEMENT_DYNAMIC(ABLock, CDialog)

ABLock::ABLock(CWnd* pParent /*=NULL*/)
	: CDialog(ABLock::IDD, pParent)
{
	memset(&m_stuInfo,0,sizeof(m_stuInfo));
}

ABLock::~ABLock()
{
}

void ABLock::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ABLOCK_CHE_ENABLE, m_cbeLockEnable);
	DDX_Control(pDX, IDC_ABLOCK_COM_GROUPNUM, m_comABLockGroupNO);
}


BEGIN_MESSAGE_MAP(ABLock, CDialog)
	ON_BN_CLICKED(IDC_ABLOCK_BUT_GET, &ABLock::OnBnClickedAblockButGet)
	ON_BN_CLICKED(IDC_ABLOCK_BUT_SET, &ABLock::OnBnClickedAblockButSet)
	ON_CBN_SELCHANGE(IDC_ABLOCK_COM_GROUPNUM, &ABLock::OnCbnSelchangeAblockComGroupnum)
	ON_CBN_SETFOCUS(IDC_ABLOCK_COM_GROUPNUM, &ABLock::OnCbnSetfocusAblockComGroupnum)
END_MESSAGE_MAP()

BOOL ABLock::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedAblockButGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_ABLOCK_BUT_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_ABLOCK_BUT_SET)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void ABLock::OnBnClickedAblockButGet()
{
	BOOL bret = Device::GetInstance().GetAccessGeneralInfo(m_stuInfo);
	if (bret)
	{
		SetABLockInfoToCtrl();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void ABLock::OnBnClickedAblockButSet()
{
	GetABLockInfoFromCtrl();
	BOOL bret = Device::GetInstance().SetAccessGeneralInfo(&m_stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void ABLock::SetABLockInfoToCtrl()
{
	m_comABLockGroupNO.SetCurSel(0);

	for (int i=0;i<CFG_MAX_ABLOCK_DOORS_NUM;i++)
	{
		Set_ABLOCK_EDIT_DOOR_text(i, "");
	}
	if (m_stuInfo.stuABLockInfo.bEnable)
	{
		m_cbeLockEnable.SetCheck(TRUE);
	}
	else
	{
		m_cbeLockEnable.SetCheck(FALSE);
	}

	for (int i=0;i<m_stuInfo.stuABLockInfo.stuDoors[0].nDoor;i++)
	{
		Set_ABLOCK_EDIT_DOOR_int(i, m_stuInfo.stuABLockInfo.stuDoors[0].anDoor[i]);
	}
}

void ABLock::GetABLockInfoFromCtrl()
{
	int nABLockGroupNO = m_comABLockGroupNO.GetCurSel();

	if (m_cbeLockEnable.GetCheck())
	{
		m_stuInfo.abABLockInfo = true;
		m_stuInfo.stuABLockInfo.bEnable = TRUE;
	}
	else
	{
		m_stuInfo.abABLockInfo = false;
		m_stuInfo.stuABLockInfo.bEnable = FALSE;
	}
	
	m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].nDoor = CFG_MAX_ABLOCK_DOORS_NUM;
	for (int i=0;i<CFG_MAX_ABLOCK_DOORS_NUM;i++)
	{
		CString str = Get_ABLOCK_EDIT_DOOR_text(i);
		if (str.IsEmpty())
		{
			m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].nDoor = i;
			break;
		}
		else
		{
			m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].anDoor[i] = Get_ABLOCK_EDIT_DOOR_int(i);
		}
	}

	m_stuInfo.stuABLockInfo.nDoors = CFG_MAX_ABLOCK_GROUP_NUM;
	for (int i=0;i<CFG_MAX_ABLOCK_GROUP_NUM;i++)
	{
		int ndoor = m_stuInfo.stuABLockInfo.stuDoors[i].nDoor;
		if (ndoor == 0)
		{
			m_stuInfo.stuABLockInfo.nDoors = i;
			break;
		}
	}
}	

void ABLock::InitDlg()
{
	int i = 0;
	m_comABLockGroupNO.ResetContent();
	for (i = 0; i < CFG_MAX_ABLOCK_GROUP_NUM; i++)
	{
		CString csInfo;
		csInfo.Format("%d", i);
		m_comABLockGroupNO.InsertString(-1, csInfo);
	}
	m_comABLockGroupNO.SetCurSel(0);

}
void ABLock::OnCbnSelchangeAblockComGroupnum()
{
	//显示新的值
	for (int i=0;i<CFG_MAX_ABLOCK_DOORS_NUM;i++)
	{
		Set_ABLOCK_EDIT_DOOR_text(i, "");
	}

	int nABLockGroupNO = m_comABLockGroupNO.GetCurSel();

	for (int i=0;i<m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].nDoor;i++)
	{
		Set_ABLOCK_EDIT_DOOR_int(i, m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].anDoor[i]);
	}

}

void ABLock::OnCbnSetfocusAblockComGroupnum()
{
	//保存下当前值
	int nABLockGroupNO = m_comABLockGroupNO.GetCurSel();

	m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].nDoor = CFG_MAX_ABLOCK_DOORS_NUM;
	for (int i=0;i<CFG_MAX_ABLOCK_DOORS_NUM;i++)
	{
		CString str = Get_ABLOCK_EDIT_DOOR_text(i);
		if (str.IsEmpty())
		{
			m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].nDoor = i;
			break;
		}
		else
		{
			m_stuInfo.stuABLockInfo.stuDoors[nABLockGroupNO].anDoor[i] = Get_ABLOCK_EDIT_DOOR_int(i);
		}
	}

	m_stuInfo.stuABLockInfo.nDoors = CFG_MAX_ABLOCK_GROUP_NUM;
	for (int i=0;i<CFG_MAX_ABLOCK_GROUP_NUM;i++)
	{
		int ndoor = m_stuInfo.stuABLockInfo.stuDoors[i].nDoor;
		if (ndoor == 0)
		{
			m_stuInfo.stuABLockInfo.nDoors = i;
			break;
		}
	}
}

CString ABLock::Get_ABLOCK_EDIT_DOOR_text(int nid)
{
	CString str = "";
	switch(nid)
	{
	case 0:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID1,str);
		break;
	case 1:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID2,str);
		break;
	case 2:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID3,str);
		break;
	case 3:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID4,str);
		break;
	case 4:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID5,str);
		break;
	case 5:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID6,str);
		break;
	case 6:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID7,str);
		break;
	case 7:
		GetDlgItemText(IDC_ABLOCK_EDIT_DOORID8,str);
		break;
	default:
		break;
	}
	return str;
}

void ABLock::Set_ABLOCK_EDIT_DOOR_text(int nid, CString str)
{
	switch(nid)
	{
	case 0:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID1, str);
		break;
	case 1:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID2,str);
		break;
	case 2:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID3,str);
		break;
	case 3:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID4,str);
		break;
	case 4:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID5,str);
		break;
	case 5:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID6,str);
		break;
	case 6:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID7,str);
		break;
	case 7:
		SetDlgItemText(IDC_ABLOCK_EDIT_DOORID8,str);
		break;
	default:
		break;
	}
}

int ABLock::Get_ABLOCK_EDIT_DOOR_int(int nid)
{
	int nd = 0;
	switch(nid)
	{
	case 0:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID1);
		break;
	case 1:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID2);
		break;
	case 2:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID3);
		break;
	case 3:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID4);
		break;
	case 4:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID5);
		break;
	case 5:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID6);
		break;
	case 6:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID7);
		break;
	case 7:
		nd = GetDlgItemInt(IDC_ABLOCK_EDIT_DOORID8);
		break;
	default:
		break;
	}
	return nd;
}

void ABLock::Set_ABLOCK_EDIT_DOOR_int(int nid, int nvalue)
{
	switch(nid)
	{
	case 0:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID1, nvalue);
		break;
	case 1:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID2, nvalue);
		break;
	case 2:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID3, nvalue);
		break;
	case 3:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID4, nvalue);
		break;
	case 4:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID5, nvalue);
		break;
	case 5:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID6, nvalue);
		break;
	case 6:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID7, nvalue);
		break;
	case 7:
		SetDlgItemInt(IDC_ABLOCK_EDIT_DOORID8, nvalue);
		break;
	default:
		break;
	}
}