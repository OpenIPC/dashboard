// FirstEnter.cpp 
//

#include "stdafx.h"
#include "AccessControl2s.h"
#include "FirstEnter.h"

IMPLEMENT_DYNAMIC(FirstEnter, CDialog)

FirstEnter::FirstEnter(CWnd* pParent /*=NULL*/)
	: CDialog(FirstEnter::IDD, pParent)
{
	memset(&m_stuInfo,0,sizeof(m_stuInfo));
}

FirstEnter::~FirstEnter()
{
}

void FirstEnter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FIRSTENTER_CMB_STATUS, m_cbFirstEnterStatus);
	DDX_Control(pDX, IDC_FIRSTENTER_CHK_ENABLE, m_chkFirstEnterEnable);
	DDX_Control(pDX, IDC_FIRSTENTER_CMB_CHANNEL, m_cmbChannel);
}


BEGIN_MESSAGE_MAP(FirstEnter, CDialog)
	ON_BN_CLICKED(IDC_FIRSTENTER_BTN_SET, &FirstEnter::OnBnClickedFirstenterBtnSet)
	ON_BN_CLICKED(IDC_FIRSTENTER_BTN_GET, &FirstEnter::OnBnClickedFirstenterBtnGet)
END_MESSAGE_MAP()


BOOL FirstEnter::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	InitDlg();
	if (Device::GetInstance().GetLoginState())
	{
		OnBnClickedFirstenterBtnGet();
	}
	else
	{
		MessageBox(ConvertString("Not login yet!"), ConvertString("Prompt"));
		GetDlgItem(IDC_FIRSTENTER_BTN_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_FIRSTENTER_BTN_GET)->EnableWindow(FALSE);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void FirstEnter::InitDlg()
{
	int i = 0;
	// channel
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

	m_cbFirstEnterStatus.ResetContent();
	static const char* szFirstEnterStat[] = 
	{	"Unknown", 
		"KeepOpen", 
		"Normal"
	};
	for (i=0; i<3; i++)
	{
		m_cbFirstEnterStatus.InsertString(-1, ConvertString(szFirstEnterStat[i]));
	}
	m_cbFirstEnterStatus.SetCurSel((int)m_stuInfo.stuFirstEnterInfo.emStatus);

	SetDlgItemInt(IDC_FIRSTENTER_EDT_TIMERINDEX, 0);
}

void FirstEnter::OnBnClickedFirstenterBtnSet()
{
	int nChn = m_cmbChannel.GetCurSel();
	if (-1 == nChn)
	{
		return;
	}
	GetFirstEnterInfoFromCtrl();
	BOOL bret = Device::GetInstance().SetAccessEventCfg(nChn, &m_stuInfo);
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Set config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void FirstEnter::OnBnClickedFirstenterBtnGet()
{
	int nChn = m_cmbChannel.GetCurSel();
	if (-1 == nChn)
	{
		return;
	}
	BOOL bret = Device::GetInstance().GetAccessEventCfg(nChn, m_stuInfo);
	if (bret)
	{
		SetFirstEnterInfoToCtrl();
	}
	else
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Get config failed:"), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
}

void FirstEnter::SetFirstEnterInfoToCtrl()
{
	//first enter enable
	if (m_stuInfo.abFirstEnterEnable)
	{
		if (m_stuInfo.stuFirstEnterInfo.bEnable)
		{
			m_chkFirstEnterEnable.SetCheck(BST_CHECKED);
		}
		else
		{
			m_chkFirstEnterEnable.SetCheck(BST_UNCHECKED);
		}
	}
	else
	{
		m_chkFirstEnterEnable.SetCheck(BST_UNCHECKED);
	}
	// first enter time index
	SetDlgItemInt(IDC_FIRSTENTER_EDT_TIMERINDEX, m_stuInfo.stuFirstEnterInfo.nTimeIndex);

	// first enter status
	m_cbFirstEnterStatus.SetCurSel((int)m_stuInfo.stuFirstEnterInfo.emStatus);
}

void FirstEnter::GetFirstEnterInfoFromCtrl()
{
	//打开此使能，netsdk才会下发首卡相关配置
	m_stuInfo.abFirstEnterEnable = true;
	if (m_chkFirstEnterEnable.GetCheck())
	{
		m_stuInfo.stuFirstEnterInfo.bEnable = TRUE;
	}
	else
	{
		m_stuInfo.stuFirstEnterInfo.bEnable = FALSE;
	}

	// first enter status
	m_stuInfo.stuFirstEnterInfo.emStatus = static_cast<CFG_ACCESS_FIRSTENTER_STATUS>(m_cbFirstEnterStatus.GetCurSel());

	// first enter time index
	m_stuInfo.stuFirstEnterInfo.nTimeIndex = GetDlgItemInt(IDC_FIRSTENTER_EDT_TIMERINDEX);
}