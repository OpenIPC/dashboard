// OpenDoorGroup.cpp
//

#include "stdafx.h"
#include "AccessControl1s.h"
#include "OpenDoorGroup.h"

IMPLEMENT_DYNAMIC(OpenDoorGroup, CDialog)

OpenDoorGroup::OpenDoorGroup(CWnd* pParent /*=NULL*/)
	: CDialog(OpenDoorGroup::IDD, pParent)
{
	memset(&stuInfo,0,sizeof(stuInfo));
}

OpenDoorGroup::~OpenDoorGroup()
{
}

void OpenDoorGroup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_USERNO1, m_comUserNO1);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_USERNO2, m_comUserNO2);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_USERNO3, m_comUserNO3);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_USERNO4, m_comUserNO4);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_OPENDOORMETHOD1, m_comOpenDoorMethod1);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_OPENDOORMETHOD2, m_comOpenDoorMethod2);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_OPENDOORMETHOD3, m_comOpenDoorMethod3);
	DDX_Control(pDX, IDC_OPENDOORGROUP_COM_0_OPENDOORMETHOD4, m_comOpenDoorMethod4);
	DDX_Control(pDX, IDC_OPENDOORGROUP_CMB_CHANNEL, m_cmbChannel);
}


BEGIN_MESSAGE_MAP(OpenDoorGroup, CDialog)
	ON_BN_CLICKED(IDC_OPENDOORGROUP_BUT_SET, &OpenDoorGroup::OnBnClickedOpendoorgroupButSet)
	ON_BN_CLICKED(IDC_OPENDOORGROUP_BUT_GET, &OpenDoorGroup::OnBnClickedOpendoorgroupButGet)
	ON_CBN_SELCHANGE(IDC_OPENDOORGROUP_COM_0_USERNO1, &OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno1)
	ON_CBN_SELCHANGE(IDC_OPENDOORGROUP_COM_0_USERNO2, &OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno2)
	ON_CBN_SELCHANGE(IDC_OPENDOORGROUP_COM_0_USERNO3, &OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno3)
	ON_CBN_SELCHANGE(IDC_OPENDOORGROUP_COM_0_USERNO4, &OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno4)
END_MESSAGE_MAP()


BOOL OpenDoorGroup::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedOpendoorgroupButGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_OPENDOORGROUP_BUT_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPENDOORGROUP_BUT_GET)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void OpenDoorGroup::InitDlg()
{
	unsigned int i = 0;

	m_comOpenDoorMethod1.ResetContent();
	for (int n = 0; n < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]) - 1; n++)
	{
		m_comOpenDoorMethod1.InsertString(-1, ConvertString(stuOpenDoorGroupSetCtlType[n].szName));
	}
	m_comOpenDoorMethod1.SetCurSel(0);

	m_comOpenDoorMethod2.ResetContent();
	for (int n = 0; n < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]) - 1; n++)
	{
		m_comOpenDoorMethod2.InsertString(-1, ConvertString(stuOpenDoorGroupSetCtlType[n].szName));
	}
	m_comOpenDoorMethod2.SetCurSel(0);

	m_comOpenDoorMethod3.ResetContent();
	for (int n = 0; n < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]) - 1; n++)
	{
		m_comOpenDoorMethod3.InsertString(-1, ConvertString(stuOpenDoorGroupSetCtlType[n].szName));
	}
	m_comOpenDoorMethod3.SetCurSel(0);

	m_comOpenDoorMethod4.ResetContent();
	for (int n = 0; n < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]) - 1; n++)
	{
		m_comOpenDoorMethod4.InsertString(-1, ConvertString(stuOpenDoorGroupSetCtlType[n].szName));
	}
	m_comOpenDoorMethod4.SetCurSel(0);

	CString csInfo;
	m_comUserNO1.ResetContent();
	for (int i = 0; i < 50; i++)
	{
		csInfo.Format("%d", i);
		m_comUserNO1.InsertString(i, csInfo);
	}
	m_comUserNO1.SetCurSel(0);

	m_comUserNO2.ResetContent();
	for (int i = 0; i < 50; i++)
	{
		csInfo.Format("%d", i);
		m_comUserNO2.InsertString(i, csInfo);
	}
	m_comUserNO2.SetCurSel(0);

	m_comUserNO3.ResetContent();
	for (int i = 0; i < 50; i++)
	{
		csInfo.Format("%d", i);
		m_comUserNO3.InsertString(i, csInfo);
	}
	m_comUserNO3.SetCurSel(0);

	m_comUserNO4.ResetContent();
	for (int i = 0; i < 50; i++)
	{
		csInfo.Format("%d", i);
		m_comUserNO4.InsertString(i, csInfo);
	}
	m_comUserNO4.SetCurSel(0);

	m_cmbChannel.ResetContent();
	int nChn = 4;
	int nAccessGroup = 0;
	BOOL bret = Device::GetInstance().GetAccessCount(nAccessGroup);
	if (bret && nAccessGroup > 0)
	{
		nChn = nAccessGroup;
	}
	for (i = 0; i < nChn; i++)
	{
		char szContent[8] = {0};
		_snprintf(szContent, sizeof(szContent), "%d", i + 1);
		m_cmbChannel.AddString(szContent);
		m_cmbChannel.SetItemData(i, (DWORD)i);
	}
	m_cmbChannel.SetCurSel(0);	
}

void OpenDoorGroup::OnBnClickedOpendoorgroupButSet()
{
	GetOpenDoorGroupInfoFromCtrl();
	int nUserCountAll = stuInfo.stuGroupInfo[0].nUserCount + stuInfo.stuGroupInfo[1].nUserCount + stuInfo.stuGroupInfo[2].nUserCount + stuInfo.stuGroupInfo[3].nUserCount;
	if (nUserCountAll > 5)
	{
		CString csInfo;
		csInfo.Format("%s", ConvertString("The effective number cannot be greater than 5"));
		MessageBox(csInfo, ConvertString("Prompt"));
		return;
	}

	BOOL bret = Device::GetInstance().SetOpenDoorGroup(m_cmbChannel.GetCurSel(), &stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void OpenDoorGroup::OnBnClickedOpendoorgroupButGet()
{
	BOOL bret = Device::GetInstance().GetOpenDoorGroup(m_cmbChannel.GetCurSel(), stuInfo);
	if (bret)
	{
		SetOpenDoorGroupInfoToCtrl();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void OpenDoorGroup::SetOpenDoorGroupInfoToCtrl()
{
	if(stuInfo.nGroup >= 1)
	{
		SetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM1, stuInfo.stuGroupInfo[0].nUserCount, TRUE);
		if (stuInfo.stuGroupInfo[0].nGroupNum > 0)
		{
			m_comUserNO1.SetCurSel(0);
			SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID1,stuInfo.stuGroupInfo[0].stuGroupDetail[0].szUserID);
			//m_comOpenDoorMethod1.SetCurSel(stuInfo.stuGroupInfo[0].stuGroupDetail[0].emMethod);

			/////
			BOOL bOpenDoorMethod1 = FALSE;
			for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
			{
				if (stuInfo.stuGroupInfo[0].stuGroupDetail[0].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
				{
					bOpenDoorMethod1 = TRUE;
					m_comOpenDoorMethod1.SetCurSel(i);
					break;
				}
			}
			if (!bOpenDoorMethod1)
			{
				m_comOpenDoorMethod1.SetCurSel(-1);
			}
			/////////

		}
	}

	if(stuInfo.nGroup >= 2)
	{
		SetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM2, stuInfo.stuGroupInfo[1].nUserCount, TRUE);
		if (stuInfo.stuGroupInfo[1].nGroupNum > 0)
		{
			m_comUserNO2.SetCurSel(0);
			SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID2,stuInfo.stuGroupInfo[1].stuGroupDetail[0].szUserID);
			//m_comOpenDoorMethod2.SetCurSel(stuInfo.stuGroupInfo[1].stuGroupDetail[0].emMethod);

			/////
			BOOL bOpenDoorMethod2 = FALSE;
			for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
			{
				if (stuInfo.stuGroupInfo[1].stuGroupDetail[0].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
				{
					bOpenDoorMethod2 = TRUE;
					m_comOpenDoorMethod2.SetCurSel(i);
					break;
				}
			}
			if (!bOpenDoorMethod2)
			{
				m_comOpenDoorMethod2.SetCurSel(-1);
			}
			/////////
		}
	}

	if(stuInfo.nGroup >= 3)
	{
		SetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM3, stuInfo.stuGroupInfo[2].nUserCount, TRUE);
		if (stuInfo.stuGroupInfo[2].nGroupNum > 0)
		{
			m_comUserNO3.SetCurSel(0);
			SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID3,stuInfo.stuGroupInfo[2].stuGroupDetail[0].szUserID);
			//m_comOpenDoorMethod3.SetCurSel(stuInfo.stuGroupInfo[2].stuGroupDetail[0].emMethod);

			/////
			BOOL bOpenDoorMethod3 = FALSE;
			for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
			{
				if (stuInfo.stuGroupInfo[2].stuGroupDetail[0].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
				{
					bOpenDoorMethod3 = TRUE;
					m_comOpenDoorMethod3.SetCurSel(i);
					break;
				}
			}
			if (!bOpenDoorMethod3)
			{
				m_comOpenDoorMethod3.SetCurSel(-1);
			}
			/////////
		}
	}

	if(stuInfo.nGroup >= 4)
	{
		SetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM4, stuInfo.stuGroupInfo[3].nUserCount, TRUE);
		if (stuInfo.stuGroupInfo[3].nGroupNum > 0)
		{
			m_comUserNO4.SetCurSel(0);
			SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID4,stuInfo.stuGroupInfo[3].stuGroupDetail[0].szUserID);
			//m_comOpenDoorMethod4.SetCurSel(stuInfo.stuGroupInfo[3].stuGroupDetail[0].emMethod);

			/////
			BOOL bOpenDoorMethod4 = FALSE;
			for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
			{
				if (stuInfo.stuGroupInfo[3].stuGroupDetail[0].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
				{
					bOpenDoorMethod4 = TRUE;
					m_comOpenDoorMethod4.SetCurSel(i);
					break;
				}
			}
			if (!bOpenDoorMethod4)
			{
				m_comOpenDoorMethod4.SetCurSel(-1);
			}
			/////////
		}
	}
}

void OpenDoorGroup::GetOpenDoorGroupInfoFromCtrl()
{
	stuInfo.stuGroupInfo[0].nUserCount = GetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM1, NULL, 1);
	int nUserNO1 = m_comUserNO1.GetCurSel();
	GetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID1,stuInfo.stuGroupInfo[0].stuGroupDetail[nUserNO1].szUserID,CFG_MAX_USER_ID_LEN);

	int nOpenDoorMethod1Index = m_comOpenDoorMethod1.GetCurSel();
	if (nOpenDoorMethod1Index >= 0 && nOpenDoorMethod1Index < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]))
	{
		stuInfo.stuGroupInfo[0].stuGroupDetail[nUserNO1].emMethod = (EM_CFG_OPEN_DOOR_GROUP_METHOD)stuOpenDoorGroupSetCtlType[nOpenDoorMethod1Index].emType;
	}
	else
	{
		stuInfo.stuGroupInfo[0].stuGroupDetail[nUserNO1].emMethod = EM_CFG_OPEN_DOOR_GROUP_METHOD_UNKNOWN;
	}
	
	for (int i=0;i<64;i++)
	{
		CString str = stuInfo.stuGroupInfo[0].stuGroupDetail[i].szUserID;
		if (str.IsEmpty())
		{
			stuInfo.stuGroupInfo[0].nGroupNum = i;
			break;
		}
	}

	stuInfo.stuGroupInfo[1].nUserCount = GetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM2, NULL, 1);
	int nUserNO2 = m_comUserNO2.GetCurSel();
	GetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID2,stuInfo.stuGroupInfo[1].stuGroupDetail[nUserNO2].szUserID,CFG_MAX_USER_ID_LEN);

	int nOpenDoorMethod2Index = m_comOpenDoorMethod2.GetCurSel();
	if (nOpenDoorMethod2Index >= 0 && nOpenDoorMethod2Index < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]))
	{
		stuInfo.stuGroupInfo[1].stuGroupDetail[nUserNO2].emMethod = (EM_CFG_OPEN_DOOR_GROUP_METHOD)stuOpenDoorGroupSetCtlType[nOpenDoorMethod2Index].emType;
	}
	else
	{
		stuInfo.stuGroupInfo[1].stuGroupDetail[nUserNO2].emMethod = EM_CFG_OPEN_DOOR_GROUP_METHOD_UNKNOWN;
	}

	for (int i=0;i<64;i++)
	{
		CString str = stuInfo.stuGroupInfo[1].stuGroupDetail[i].szUserID;
		if (str.IsEmpty())
		{
			stuInfo.stuGroupInfo[1].nGroupNum = i;
			break;
		}
	}

	stuInfo.stuGroupInfo[2].nUserCount = GetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM3, NULL, 1);
	int nUserNO3 = m_comUserNO3.GetCurSel();
	GetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID3,stuInfo.stuGroupInfo[2].stuGroupDetail[nUserNO3].szUserID,CFG_MAX_USER_ID_LEN);

	int nOpenDoorMethod3Index = m_comOpenDoorMethod3.GetCurSel();
	if (nOpenDoorMethod3Index >= 0 && nOpenDoorMethod3Index < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]))
	{
		stuInfo.stuGroupInfo[2].stuGroupDetail[nUserNO3].emMethod = (EM_CFG_OPEN_DOOR_GROUP_METHOD)stuOpenDoorGroupSetCtlType[nOpenDoorMethod3Index].emType;
	}
	else
	{
		stuInfo.stuGroupInfo[2].stuGroupDetail[nUserNO3].emMethod = EM_CFG_OPEN_DOOR_GROUP_METHOD_UNKNOWN;
	}

	for (int i=0;i<64;i++)
	{
		CString str = stuInfo.stuGroupInfo[2].stuGroupDetail[i].szUserID;
		if (str.IsEmpty())
		{
			stuInfo.stuGroupInfo[2].nGroupNum = i;
			break;
		}
	}

	stuInfo.stuGroupInfo[3].nUserCount = GetDlgItemInt(IDC_OPENDOORGROUP_EDIT_USEROPENNUM4, NULL, 1);
	int nUserNO4 = m_comUserNO4.GetCurSel();
	GetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID4,stuInfo.stuGroupInfo[3].stuGroupDetail[nUserNO4].szUserID,CFG_MAX_USER_ID_LEN);

	int nOpenDoorMethod4Index = m_comOpenDoorMethod4.GetCurSel();
	if (nOpenDoorMethod4Index >= 0 && nOpenDoorMethod4Index < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]))
	{
		stuInfo.stuGroupInfo[3].stuGroupDetail[nUserNO4].emMethod = (EM_CFG_OPEN_DOOR_GROUP_METHOD)stuOpenDoorGroupSetCtlType[nOpenDoorMethod4Index].emType;
	}
	else
	{
		stuInfo.stuGroupInfo[3].stuGroupDetail[nUserNO4].emMethod = EM_CFG_OPEN_DOOR_GROUP_METHOD_UNKNOWN;
	}

	for (int i=0;i<64;i++)
	{
		CString str = stuInfo.stuGroupInfo[3].stuGroupDetail[i].szUserID;
		if (str.IsEmpty())
		{
			stuInfo.stuGroupInfo[3].nGroupNum = i;
			break;
		}
	}
	stuInfo.nGroup = 4;
	for (int i=0;i<4;i++)
	{
		if (stuInfo.stuGroupInfo[i].nGroupNum == 0)
		{
			stuInfo.nGroup = i;
			break;
		}
	}
}

void OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno1()
{
	int nUserNO = m_comUserNO1.GetCurSel();
	SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID1, stuInfo.stuGroupInfo[0].stuGroupDetail[nUserNO].szUserID);

	BOOL bOpenDoorMethod1 = FALSE;
	for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
	{
		if (stuInfo.stuGroupInfo[0].stuGroupDetail[nUserNO].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
		{
			bOpenDoorMethod1 = TRUE;
			m_comOpenDoorMethod1.SetCurSel(i);
			break;
		}
	}
	if (!bOpenDoorMethod1)
	{
		m_comOpenDoorMethod1.SetCurSel(-1);
	}
}

void OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno2()
{
	int nUserNO = m_comUserNO2.GetCurSel();
	SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID2, stuInfo.stuGroupInfo[1].stuGroupDetail[nUserNO].szUserID);

	BOOL bOpenDoorMethod2 = FALSE;
	for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
	{
		if (stuInfo.stuGroupInfo[1].stuGroupDetail[nUserNO].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
		{
			bOpenDoorMethod2 = TRUE;
			m_comOpenDoorMethod2.SetCurSel(i);
			break;
		}
	}
	if (!bOpenDoorMethod2)
	{
		m_comOpenDoorMethod2.SetCurSel(-1);
	}

}

void OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno3()
{
	int nUserNO = m_comUserNO3.GetCurSel();
	SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID3, stuInfo.stuGroupInfo[2].stuGroupDetail[nUserNO].szUserID);

	BOOL bOpenDoorMethod3 = FALSE;
	for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
	{
		if (stuInfo.stuGroupInfo[2].stuGroupDetail[nUserNO].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
		{
			bOpenDoorMethod3 = TRUE;
			m_comOpenDoorMethod3.SetCurSel(i);
			break;
		}
	}
	if (!bOpenDoorMethod3)
	{
		m_comOpenDoorMethod3.SetCurSel(-1);
	}
}

void OpenDoorGroup::OnCbnSelchangeOpendoorgroupCom0Userno4()
{
	int nUserNO = m_comUserNO4.GetCurSel();
	SetDlgItemText(IDC_OPENDOORGROUP_EDIT_0_USERID4, stuInfo.stuGroupInfo[3].stuGroupDetail[nUserNO].szUserID);

	BOOL bOpenDoorMethod4 = FALSE;
	for (int i = 0; i < sizeof(stuOpenDoorGroupSetCtlType)/sizeof(stuOpenDoorGroupSetCtlType[0]); i++)
	{
		if (stuInfo.stuGroupInfo[3].stuGroupDetail[nUserNO].emMethod == stuOpenDoorGroupSetCtlType[i].emType)
		{
			bOpenDoorMethod4 = TRUE;
			m_comOpenDoorMethod4.SetCurSel(i);
			break;
		}
	}
	if (!bOpenDoorMethod4)
	{
		m_comOpenDoorMethod4.SetCurSel(-1);
	}
}
