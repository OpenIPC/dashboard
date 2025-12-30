// DlgCfgGuard.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgCfgGuard.h"

#define MAX_SCENE_NUM 2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgGuard dialog


CDlgCfgGuard::CDlgCfgGuard(CWnd* pParent /* = NULL */, LLONG lLoginID /* = 0 */)
	: CDialog(CDlgCfgGuard::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCfgGuard)
	m_lLoginID = lLoginID;
	//}}AFX_DATA_INIT
	memset(&m_stuInfo, 0, sizeof(m_stuInfo));
	m_stuInfo.bSceneEnable = TRUE;
}


void CDlgCfgGuard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCfgGuard)
	DDX_Control(pDX, IDC_GUARD_CMB_SCENE, m_cmbScene);
	DDX_Control(pDX, IDC_GUARD_EDT_RETSCENECOUNT, m_edRetSceneCount);
	DDX_Control(pDX, IDC_GUARD_CMB_SCENENAME, m_cmbSceneName);
	DDX_Control(pDX, IDC_GUARD_CMB_CURRENTSCENE, m_cmbCurrentScene);
	DDX_Control(pDX, IDC_GUARD_CK_ALARMENABLE, m_ckAlarmEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCfgGuard, CDialog)
	//{{AFX_MSG_MAP(CDlgCfgGuard)
	ON_BN_CLICKED(IDC_GUARD_BTN_GET, OnGuardBtnGet)
	ON_BN_CLICKED(IDC_GUARD_BTN_SET, OnGuardBtnSet)
	ON_CBN_SELCHANGE(IDC_GUARD_CMB_SCENE, OnSelchangeGuardCmbScene)
	ON_CBN_DROPDOWN(IDC_GUARD_CMB_SCENE, OnDropdownGuardCmbScene)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgGuard message handlers
BOOL CDlgCfgGuard::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_CFG_GUARD);

	if (m_lLoginID == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_CFG_GUARD), ConvertString("Prompt"));
		return TRUE;
	}

	m_cmbCurrentScene.ResetContent();
	m_cmbCurrentScene.InsertString(-1, ConvertString("Unknown", DLG_CFG_GUARD));
	m_cmbCurrentScene.InsertString(-1, ConvertString("Outdoor", DLG_CFG_GUARD));
	m_cmbCurrentScene.InsertString(-1, ConvertString("AtHome", DLG_CFG_GUARD));
	m_cmbCurrentScene.SetCurSel(0);

	m_cmbSceneName.ResetContent();
	m_cmbSceneName.InsertString(-1, ConvertString("Unknown", DLG_CFG_GUARD));
	m_cmbSceneName.InsertString(-1, ConvertString("Outdoor", DLG_CFG_GUARD));
	m_cmbSceneName.InsertString(-1, ConvertString("AtHome", DLG_CFG_GUARD));
	m_cmbSceneName.SetCurSel(0);

	int i = 0;
	m_cmbScene.ResetContent();
	for (i = 0; i < MAX_SCENE_NUM; ++i)
	{
		CString csItem;
		csItem.Format("%s %02d", ConvertString("Scene", DLG_CFG_GUARD), i + 1);
		m_cmbScene.InsertString(-1, csItem);
	}

	m_cmbScene.SetCurSel(0);
	for (i = 0; i < MAX_SCENE_NUM; ++i)
	{
		m_stuInfo.stuScense[i].nAlarmInChannelsCount = MAX_CHANNEL_COUNT;
		m_stuInfo.stuScense[i].pnAlarmInChannels = new int[MAX_CHANNEL_COUNT];
		if (m_stuInfo.stuScense[i].pnAlarmInChannels == NULL)
		{
			MessageBox(CString("new error!"), ConvertString("Prompt"));
			return TRUE;
		}
		memset(m_stuInfo.stuScense[i].pnAlarmInChannels, -1, sizeof(int)*8);
	}
	m_ckAlarmEnable.EnableWindow(FALSE);
	m_cmbCurrentScene.EnableWindow(FALSE);
	OnGuardBtnGet();
	return TRUE; 
}

BOOL CDlgCfgGuard::GetCurrentGuardState()
{
	char szJsonBuf[1024] = {0};	

	int nErr = 0;
	BOOL bRet = CLIENT_GetNewDevConfig(m_lLoginID, CFG_CMD_COMMGLOBAL, -1,
		szJsonBuf, sizeof(szJsonBuf), &nErr, SDK_API_WAIT);
	if (!bRet)
	{
		CString csOut;
		csOut.Format("%s %08x.", ConvertString("Query CommGlobal failed:", DLG_CFG_GUARD), CLIENT_GetLastError());
		MessageBox(csOut, ConvertString("Prompt"));
		return FALSE;
	}

	DWORD dwRetLen = 0;
	bRet = CLIENT_ParseData(CFG_CMD_COMMGLOBAL, szJsonBuf, (void*)&m_stuInfo, sizeof(m_stuInfo), &dwRetLen);
	if (!bRet)
	{
		MessageBox(ConvertString("Parse CommGlobal failed.", DLG_CFG_GUARD), ConvertString("Prompt"));
		return FALSE;
	}
	return TRUE;
}


void CDlgCfgGuard::OnGuardBtnGet() 
{
	// TODO: Add your control notification handler code here
	if (GetCurrentGuardState())
	{
		StuToDlg();
	}
}

void CDlgCfgGuard::OnGuardBtnSet() 
{
	// TODO: Add your control notification handler code here
	DlgToStu(m_cmbScene.GetCurSel());

	char szJsonBuf[1024] = {0};
	int nerror = 0;
	int nrestart = 0;
	BOOL bRet = CLIENT_PacketData(CFG_CMD_COMMGLOBAL, &m_stuInfo, sizeof(m_stuInfo), szJsonBuf, sizeof(szJsonBuf));
	if (!bRet)
	{
		MessageBox(ConvertString(CString("packet CommGlobal error..."), DLG_CFG_GUARD), ConvertString("Prompt"));
		return;
	} 
	else
	{
		bRet = CLIENT_SetNewDevConfig(m_lLoginID, CFG_CMD_COMMGLOBAL, -1, szJsonBuf, 1024, &nerror, &nrestart, SDK_API_WAIT);
		if (!bRet)
		{
			CString csErr;
			csErr.Format("%s %08x", ConvertString("SetupConfig CommGlobal failed:", DLG_CFG_GUARD), CLIENT_GetLastError());
			MessageBox(csErr, ConvertString("Prompt"));
			return;
		}
		else
		{
			MessageBox(ConvertString(CString("SetConfig CommGlobal ok!"), DLG_CFG_GUARD), ConvertString("Prompt"));
		}
	}
	return;
}

void CDlgCfgGuard::StuToDlg()
{
    int nIndex = m_cmbScene.GetCurSel();
    if (nIndex < 0 || nIndex >= MAX_SCENE_COUNT)
    {
        return;
    }

	m_ckAlarmEnable.SetCheck(m_stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
	m_cmbCurrentScene.SetCurSel(m_stuInfo.emCurrentScene);

	m_cmbSceneName.SetCurSel(m_stuInfo.stuScense[nIndex].emName);
	char szRetChannelCount[10] = {0};
	_itoa(m_stuInfo.stuScense[nIndex].nRetAlarmInChannelsCount, szRetChannelCount, 10);
	m_edRetSceneCount.SetWindowText(szRetChannelCount);
	
	for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_GUARD_CK_CHANNEL_1 + i);
		if (pBtn)
		{
			pBtn->SetCheck(BST_UNCHECKED);
			for (int j = 0; j < __max(m_stuInfo.stuScense[nIndex].nRetAlarmInChannelsCount, m_stuInfo.stuScense[nIndex].nAlarmInChannelsCount); ++j)
			{
				if (i == m_stuInfo.stuScense[nIndex].pnAlarmInChannels[j])
				{
					pBtn->SetCheck(BST_CHECKED);
					break;
				}
				else
				{
					pBtn->SetCheck(BST_UNCHECKED);
				}
			}
		}
	}
}

void CDlgCfgGuard::DlgToStu(int nIndex)
{
    if (nIndex < 0 || nIndex >= MAX_SCENE_COUNT)
    {
        return;
    }

	m_stuInfo.nSceneCount = MAX_SCENE_NUM;
	int nChannelCount = 0;
			
	memset(m_stuInfo.stuScense[nIndex].pnAlarmInChannels, -1, sizeof(int)*MAX_CHANNEL_COUNT);
	for (int i = 0; i < MAX_CHANNEL_COUNT; i++)
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_GUARD_CK_CHANNEL_1 + i);
		if (pBtn)
		{
			if (BST_CHECKED == pBtn->GetCheck())
			{
				m_stuInfo.stuScense[nIndex].pnAlarmInChannels[nChannelCount] = i;
				++nChannelCount;
			}
		}
	}
	m_stuInfo.stuScense[nIndex].emName = (emCFG_SCENE_MODE)m_cmbSceneName.GetCurSel();
	m_stuInfo.stuScense[nIndex].nAlarmInChannelsCount = nChannelCount;
}

void CDlgCfgGuard::OnSelchangeGuardCmbScene() 
{
	// TODO: Add your control notification handler code here
	StuToDlg();
}

void CDlgCfgGuard::OnDropdownGuardCmbScene() 
{
	// TODO: Add your control notification handler code here
	int nIndex = m_cmbScene.GetCurSel();
	DlgToStu(nIndex);
}

void CDlgCfgGuard::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	for (int i = 0; i < MAX_SCENE_COUNT; i++)
	{
		if (m_stuInfo.stuScense[i].pnAlarmInChannels !=  NULL)
		{
			delete []m_stuInfo.stuScense[i].pnAlarmInChannels;
			m_stuInfo.stuScense[i].pnAlarmInChannels = NULL;
		}
	}
}
