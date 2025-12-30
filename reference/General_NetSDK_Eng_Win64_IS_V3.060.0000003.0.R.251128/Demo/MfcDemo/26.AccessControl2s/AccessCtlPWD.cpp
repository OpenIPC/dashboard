// AccessCtlPWD.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "AccessCtlPWD.h"
#include "DlgSensorInfoDescription.h"

IMPLEMENT_DYNAMIC(AccessCtlPWD, CDialog)

AccessCtlPWD::AccessCtlPWD(CWnd* pParent /*=NULL*/)
	: CDialog(AccessCtlPWD::IDD, pParent)
{
	vecDoor.clear();
	memset(&m_stuRecordAccessPWD, 0, sizeof(m_stuRecordAccessPWD));
}

AccessCtlPWD::~AccessCtlPWD()
{
}

void AccessCtlPWD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COM_ACCESSPWD_EXECUTETYPE, m_cmbCtlType);
}


BEGIN_MESSAGE_MAP(AccessCtlPWD, CDialog)
	ON_BN_CLICKED(IDC_BUT_ACCESSPWD_EXECUTE, &AccessCtlPWD::OnBnClickedButAccesspwdExecute)
	ON_BN_CLICKED(IDC_INFO_PASSWORD_BTN_DOORS, &AccessCtlPWD::OnBnClickedInfoPasswordBtnDoors)
	ON_CBN_SELCHANGE(IDC_COM_ACCESSPWD_EXECUTETYPE, &AccessCtlPWD::OnCbnSelchangeComAccesspwdExecutetype)
	ON_BN_CLICKED(IDC_BUT_ACCESSPWD_GET, &AccessCtlPWD::OnBnClickedButAccesspwdGet)
END_MESSAGE_MAP()


BOOL AccessCtlPWD::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	memset(&m_stuRecordAccessPWD, 0, sizeof(m_stuRecordAccessPWD));

	unsigned int i = 0;

	m_cmbCtlType.ResetContent();
	for (int n = 0; n < sizeof(stuDemoRecordSetCtlType)/sizeof(stuDemoRecordSetCtlType[0]); n++)
	{
		m_cmbCtlType.InsertString(-1, ConvertString(stuDemoRecordSetCtlType[n].szName));
	}
	m_cmbCtlType.SetCurSel(0);

	if (!Device::GetInstance().GetLoginState())
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->EnableWindow(FALSE);
	}
	OnCbnSelchangeComAccesspwdExecutetype();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void AccessCtlPWD::OnBnClickedButAccesspwdExecute()
{
	int nCtlType = m_cmbCtlType.GetCurSel();
	if (-1 == nCtlType)
	{
		return;
	}

	memset(&m_stuRecordAccessPWD, 0, sizeof(m_stuRecordAccessPWD));
	m_stuRecordAccessPWD.dwSize = sizeof(m_stuRecordAccessPWD);

	if (Em_Operate_Type_Insert == nCtlType + 1)
	{
		AccessPWDInsert();
	}
	else if (Em_Operate_Type_Get == nCtlType + 1)
	{
		AccessPWDGet();
	}
	else if (Em_Operate_Type_Update == nCtlType + 1)
	{
		AccessPWDUpdate();
		OnChangeUIState(2);
	}
	else if (Em_Operate_Type_Remove == nCtlType + 1)
	{
		AccessPWDRemove();
	}
	else if (Em_Operate_Type_Clear == nCtlType + 1)
	{
		AccessPWDClear();
	}
}

void AccessCtlPWD::AccessPWDInsert()
{
	GetAccessPWDInfoFromCtrl();
	int nRecNo;
	BOOL bret = Device::GetInstance().InsertPWDRecorde(&m_stuRecordAccessPWD,nRecNo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s:%d", ConvertString("Insert Password ok with RecNo"), nRecNo);
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void AccessCtlPWD::AccessPWDGet()
{
	GetAccessPWDInfoFromCtrl();
	// user id
	SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, "");
	// pwd of opening door
	SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, "");

	BOOL bret = Device::GetInstance().GetPWDRecorde(m_stuRecordAccessPWD);
	if (bret)
	{
		SetAccessPWDInfoToCtrl();
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void AccessCtlPWD::AccessPWDUpdate()
{
	GetAccessPWDInfoFromCtrl();
	BOOL bret = Device::GetInstance().UpdatePWDRecorde(&m_stuRecordAccessPWD);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void AccessCtlPWD::AccessPWDRemove()
{
	GetAccessPWDInfoFromCtrl();
	BOOL bret = Device::GetInstance().RemovePWDRecorde(&m_stuRecordAccessPWD);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void AccessCtlPWD::AccessPWDClear()
{
	BOOL bret = Device::GetInstance().ClearPWDRecorde();
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
	}
}

void AccessCtlPWD::GetAccessPWDInfoFromCtrl()
{
	// RecNo
	m_stuRecordAccessPWD.nRecNo = GetDlgItemInt(IDC_INFO_PASSWORD_EDIT_RECNO, NULL, TRUE);

	// user id
	//GetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, m_stuRecordAccessPWD.szUserID, sizeof(m_stuRecordAccessPWD.szUserID));

	// pwd of opening door
	GetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, m_stuRecordAccessPWD.szDoorOpenPwd, sizeof(m_stuRecordAccessPWD.szDoorOpenPwd));

	//door
	std::vector<int>::iterator it = vecDoor.begin();
	for (int i = 0; i < __min(vecDoor.size(), DH_MAX_DOOR_NUM) && it != vecDoor.end(); i++, it++)
	{
		m_stuRecordAccessPWD.sznDoors[i] = *it;
	}
	m_stuRecordAccessPWD.nDoorNum = __min(vecDoor.size(), DH_MAX_DOOR_NUM);
}

void AccessCtlPWD::SetAccessPWDInfoToCtrl()
{
	// RecNo
	SetDlgItemInt(IDC_INFO_PASSWORD_EDIT_RECNO, m_stuRecordAccessPWD.nRecNo);

	// user id
	SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, m_stuRecordAccessPWD.szUserID);

	// pwd of opening door
	SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, m_stuRecordAccessPWD.szDoorOpenPwd);

}

void AccessCtlPWD::OnBnClickedInfoPasswordBtnDoors()
{
	std::vector<int> vecChn;
	int i = 0;
	for (; i < __min(m_stuRecordAccessPWD.nDoorNum, DH_MAX_DOOR_NUM); i++)
	{
		vecChn.push_back(m_stuRecordAccessPWD.sznDoors[i]);
	}
	int m_emOperateType = m_cmbCtlType.GetCurSel() + 1;
	int m_nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(m_nAccessGroup);
	if (bret && m_nAccessGroup > 0)
	{
		DlgSensorInfoDescription dlg(this, m_nAccessGroup);
		dlg.SetID(vecChn);
		if (IDOK == dlg.DoModal())
		{
			if (Em_Operate_Type_Insert == m_emOperateType
				|| Em_Operate_Type_Update == m_emOperateType)
			{
				vecChn.clear();
				vecChn = dlg.GetID();
				vecDoor.clear();
				vecDoor = dlg.GetID();
			}
		}
	}
}

void AccessCtlPWD::OnCbnSelchangeComAccesspwdExecutetype()
{
	int nCtlType = m_cmbCtlType.GetCurSel();
	OnChangeUIState(nCtlType);
}

void AccessCtlPWD::OnChangeUIState(int nState)
{
	GetDlgItem(IDC_INFO_PASSWORD_EDIT_RECNO)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_PASSWORD_EDIT_USERID)->EnableWindow(TRUE);
	GetDlgItem(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD)->EnableWindow(TRUE);

	GetDlgItem(IDC_BUT_ACCESSPWD_GET)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUT_ACCESSPWD_GET)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->EnableWindow(TRUE);
	//int nCtlType = m_cmbCtlType.GetCurSel();
	switch(nState)
	{
	case 0://insert
		{
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_RECNO)->EnableWindow(FALSE);
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_RECNO,"");
			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->SetWindowText(ConvertString("Insert"));
		}
		break;
	case 1://get
		{
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_USERID)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD)->EnableWindow(TRUE);
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, "");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, "");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_RECNO,"");
			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->SetWindowText(ConvertString("Get"));
		}
		break;
	case 2://Update step1
		{
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_USERID)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD)->EnableWindow(TRUE);
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, "");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, "");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_RECNO,"");

			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->SetWindowText(ConvertString("Update"));
			GetDlgItem(IDC_BUT_ACCESSPWD_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_ACCESSPWD_GET)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->EnableWindow(FALSE);
		}
		break;
	case 3://remove
		{
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_USERID)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD)->EnableWindow(TRUE);
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, "");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, "");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_RECNO,"");

			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->SetWindowText(ConvertString("Remove"));
		}
		break;
	case 4://clear
		{
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_RECNO)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_USERID)->EnableWindow(FALSE);
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD)->EnableWindow(TRUE);
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_RECNO,"");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID,"");
			SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD,"");
			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->SetWindowText(ConvertString("Clear"));
		}
		break;
	case 5://Update step2
		{
			GetDlgItem(IDC_BUT_ACCESSPWD_GET)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUT_ACCESSPWD_GET)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUT_ACCESSPWD_EXECUTE)->EnableWindow(TRUE);
			GetDlgItem(IDC_INFO_PASSWORD_EDIT_RECNO)->EnableWindow(FALSE);
		}
		break;
	default:
		break;
	}
}

void AccessCtlPWD::OnBnClickedButAccesspwdGet()
{
	GetAccessPWDInfoFromCtrl();
	// user id
	SetDlgItemText(IDC_INFO_PASSWORD_EDIT_USERID, "");
	// pwd of opening door
	SetDlgItemText(IDC_INFO_PASSWORD_EDIT_DOOROPENPWD, "");

	BOOL bret = Device::GetInstance().GetPWDRecorde(m_stuRecordAccessPWD);
	if (bret)
	{
		SetAccessPWDInfoToCtrl();
		//MessageBox(ConvertString("Execute successfully."), ConvertString("Prompt"));
		OnChangeUIState(5);
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Execute failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}
