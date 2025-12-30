// DlgSetBypass.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgSetBypass.h"
#include "DlgLocalBypass.h"
#include "DlgExtendedBypass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetBypass dialog


CDlgSetBypass::CDlgSetBypass(CWnd* pParent /* = NULL */, LLONG lLoginId /* = 0 */)
	: CDialog(CDlgSetBypass::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSetBypass)
	m_edExtendedCount = 0;
	m_edLocalCount = 16;
	m_edExtendedCountForGet = 0;
	m_edLocalCountForGet = 16;
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuSetBypass, 0, sizeof(NET_CTRL_SET_BYPASS));
	m_stuSetBypass.dwSize = sizeof(NET_CTRL_SET_BYPASS);
	memset(&m_stuGetBypass, 0, sizeof(NET_DEVSTATE_GET_BYPASS));
	m_stuGetBypass.dwSize = sizeof(NET_DEVSTATE_GET_BYPASS);
	m_bNeedMode = FALSE;
	m_nLastLocalCount = 0;
	m_nLastExtendedCount = 0;
}


void CDlgSetBypass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetBypass)
	DDX_Control(pDX, IDC_SETBYPASS_EDIT_PASSWORD, m_edPassword);
	DDX_Control(pDX, IDC_SETBYPASS_CHECK_MODE, m_ckMode);
	DDX_Text(pDX, IDC_SETBYPASS_EDIT_EXTENDEDCOUNT, m_edExtendedCount);
	DDV_MinMaxUInt(pDX, m_edExtendedCount, 0, 256);
	DDX_Text(pDX, IDC_SETBYPASS_EDIT_LOCALCOUNT, m_edLocalCount);
	DDV_MinMaxUInt(pDX, m_edLocalCount, 0, 16);
	DDX_Text(pDX, IDC_SETBYPASS_EDIT_EXTENDEDCOUNT_FORGET, m_edExtendedCountForGet);
	DDV_MinMaxUInt(pDX, m_edExtendedCountForGet, 0, 256);
	DDX_Text(pDX, IDC_SETBYPASS_EDIT_LOCALCOUNT_FORGET, m_edLocalCountForGet);
	DDV_MinMaxUInt(pDX, m_edLocalCountForGet, 0, 16);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSetBypass, CDialog)
	//{{AFX_MSG_MAP(CDlgSetBypass)
	ON_BN_CLICKED(IDC_SETBYPASS_BUTTON_LOCAL, OnSetbypassButtonLocal)
	ON_BN_CLICKED(IDC_SETBYPASS_BUTTON_EXTENDED, OnSetbypassButtonExtended)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SETBYPASS_BUTTON_SET, OnSetbypassButtonSet)
	ON_BN_CLICKED(IDC_SETBYPASS_BUTTON_EXTENDED_FORGET, OnSetbypassButtonExtendedForget)
	ON_BN_CLICKED(IDC_SETBYPASS_BUTTON_GET, OnSetbypassButtonGet)
	ON_BN_CLICKED(IDC_SETBYPASS_BUTTON_LOCAL_FORGET, OnSetbypassButtonLocalForget)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetBypass message handlers

BOOL CDlgSetBypass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_BYPASS);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetBypass::OnSetbypassButtonLocal() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_edLocalCount <= 0)
	{
		MessageBox(ConvertString("please input LocalCount first!",DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	if (m_edLocalCount > 16)
	{
		return;
	}
	if (m_stuSetBypass.pnLocal != NULL)
	{
		delete[] m_stuSetBypass.pnLocal;
		m_stuSetBypass.pnLocal = NULL;
	}
	m_stuSetBypass.pnLocal = new int[m_edLocalCount];
	if (m_stuSetBypass.pnLocal == NULL)
	{
		MessageBox(ConvertString("new extended channel error!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	memset(m_stuSetBypass.pnLocal, 0, m_edLocalCount * sizeof(int));

	CDlgLocalBypass CDlg(this, FALSE, m_edLocalCount);

	if (IDOK == CDlg.DoModal())
	{
		CDlg.GetLocalChannel(m_stuSetBypass.pnLocal);
		m_stuSetBypass.nLocalCount = CDlg.GetActualCount();
	}
}

void CDlgSetBypass::OnSetbypassButtonExtended() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_edExtendedCount <= 0)
	{
		MessageBox(ConvertString("please input extendedCount first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	if (m_edExtendedCount > 256)
	{
		return;
	}
	if (m_stuSetBypass.pnExtended != NULL)
	{
		delete[] m_stuSetBypass.pnExtended;
		m_stuSetBypass.pnExtended = NULL;
	}
	m_stuSetBypass.pnExtended = new int[m_edExtendedCount];
	if (m_stuSetBypass.pnExtended == NULL)
	{
		MessageBox(ConvertString("new extended channel error!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	memset(m_stuSetBypass.pnExtended, 0, m_edExtendedCount * sizeof(int));

	CDlgExtendedBypass CDlg(this, FALSE, m_edExtendedCount);

	if (IDOK == CDlg.DoModal())
	{
		CDlg.GetExtendedChannnel(m_stuSetBypass.pnExtended);
		m_stuSetBypass.nExtendedCount = CDlg.GetActualCount();
	}
}

void CDlgSetBypass::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_stuSetBypass.pnLocal != NULL)
	{
		delete[] m_stuSetBypass.pnLocal;
		m_stuSetBypass.pnLocal = NULL;
	}

	if (m_stuSetBypass.pnExtended != NULL)
	{
		delete[] m_stuSetBypass.pnExtended;
		m_stuSetBypass.pnExtended = NULL;
	}

	if (m_stuGetBypass.pnLocal != NULL)
	{
		delete[] m_stuGetBypass.pnLocal;
		m_stuGetBypass.pnLocal = NULL;
	}
	
	if (m_stuGetBypass.pnExtended != NULL)
	{
		delete[] m_stuGetBypass.pnExtended;
		m_stuGetBypass.pnExtended = NULL;
	}
}

void CDlgSetBypass::OnSetbypassButtonSet() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_lLoginId <= 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}

	if (m_edLocalCount <= 0 && m_edExtendedCount <= 0)
	{
		MessageBox(ConvertString("please input Count first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}


	CString csPsw;
	GetDlgItemText(IDC_SETBYPASS_EDIT_PASSWORD, csPsw);
	m_stuSetBypass.szDevPwd = csPsw.GetBuffer(0);
	m_stuSetBypass.emMode = m_ckMode.GetCheck() ? NET_BYPASS_MODE_BYPASS : NET_BYPASS_MODE_NORMAL;
	BOOL bRet = CLIENT_ControlDevice(m_lLoginId, DH_CTRL_SET_BYPASS, &m_stuSetBypass, SDK_API_WAIT);
	if (bRet)
	{
		MessageBox(ConvertString("Set bypass ok!", DLG_BYPASS), ConvertString("Prompt"));
	} 
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("set bypass failed:", DLG_BYPASS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
}

void CDlgSetBypass::OnSetbypassButtonExtendedForget() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_edExtendedCountForGet <= 0)
	{
		MessageBox(ConvertString("please input extendedCount first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}

	if (m_edExtendedCountForGet > 256)
	{
		return;
	}


	int nTempChannel[256] = {0};
	if (m_stuGetBypass.pnExtended != NULL)
	{
		for (unsigned int i = 0; i < __min((unsigned)m_nLastExtendedCount, m_edExtendedCountForGet); ++i)
		{
			nTempChannel[i] = m_stuGetBypass.pnExtended[i];
		}
		delete[] m_stuGetBypass.pnExtended;
		m_stuGetBypass.pnExtended = NULL;
	}
	NET_BYPASS_MODE emTempMode[256] = {NET_BYPASS_MODE_UNKNOW};
	if (m_stuGetBypass.pemExtended != NULL)
	{
		for (unsigned int i = 0; i < __min((unsigned)m_nLastExtendedCount, m_edExtendedCountForGet); ++i)
		{
			emTempMode[i] = m_stuGetBypass.pemExtended[i];
		}
		delete[] m_stuGetBypass.pemExtended;
		m_stuGetBypass.pemExtended = NULL;
	}
	m_stuGetBypass.pnExtended = new int[m_edExtendedCountForGet];
	m_stuGetBypass.pemExtended = new NET_BYPASS_MODE[m_edExtendedCountForGet];
	if (m_stuGetBypass.pnExtended == NULL || m_stuGetBypass.pemExtended == NULL)
	{
		MessageBox(ConvertString("new extended channel error!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	memset(m_stuGetBypass.pnExtended, 0, m_edExtendedCountForGet * sizeof(int));
	for (unsigned int i = 0; i < m_edExtendedCountForGet; ++i)
	{	
		m_stuGetBypass.pemExtended[i] = NET_BYPASS_MODE_UNKNOW;
	}

	CDlgExtendedBypass CDlg(this, TRUE, m_edExtendedCountForGet);
	CDlg.SetLastCount(m_nLastExtendedCount);
	CDlg.SetExtendedChannel(nTempChannel);

	if (m_bNeedMode)
	{
		CDlg.SetExtendedMode(emTempMode);
	}

	if (IDOK == CDlg.DoModal())
	{
		CDlg.GetExtendedChannnel(m_stuGetBypass.pnExtended);
		m_stuGetBypass.nExtendedCount = CDlg.GetActualCount();
		m_nLastExtendedCount = __min(m_edExtendedCountForGet, m_stuGetBypass.nExtendedCount);
	}
	else
	{
		m_nLastExtendedCount = 0;
	}
}

void CDlgSetBypass::OnSetbypassButtonLocalForget() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if (m_edLocalCountForGet <= 0)
	{
		MessageBox(ConvertString("please input localCount first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}

	if (m_edLocalCountForGet > 16)
	{
		return;
	}

	int nTempLocalChannel[16] = {0};
	if (m_stuGetBypass.pnLocal != NULL)
	{
		for (unsigned int i = 0; i < __min((unsigned)m_nLastLocalCount, m_edLocalCountForGet); ++i)
		{
			nTempLocalChannel[i] = m_stuGetBypass.pnLocal[i];
		}
		delete[] m_stuGetBypass.pnLocal;
		m_stuGetBypass.pnLocal = NULL;
	}
	NET_BYPASS_MODE emTempLocalMode[16] = {NET_BYPASS_MODE_UNKNOW};
	if (m_stuGetBypass.pemLocal != NULL)
	{
		for (unsigned int i = 0; i < __min((unsigned)m_nLastLocalCount, m_edLocalCountForGet); ++i)
		{
			emTempLocalMode[i] = m_stuGetBypass.pemLocal[i];
		}

		delete[] m_stuGetBypass.pemLocal;
		m_stuGetBypass.pemLocal = NULL;
	}

	m_stuGetBypass.pnLocal = new int[m_edLocalCountForGet];
	m_stuGetBypass.pemLocal = new NET_BYPASS_MODE[m_edLocalCountForGet];
	if (m_stuGetBypass.pnLocal == NULL || m_stuGetBypass.pemLocal == NULL)
	{
		MessageBox(ConvertString("new Local channel error!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	memset(m_stuGetBypass.pnLocal, 0, m_edLocalCountForGet * sizeof(int));
	for (unsigned int i = 0; i < m_edLocalCountForGet; ++i)
	{	
		m_stuGetBypass.pemLocal[i] = NET_BYPASS_MODE_UNKNOW;
	}

	CDlgLocalBypass CDlg(this, TRUE, m_edLocalCountForGet);
	CDlg.SetLastCount(m_nLastLocalCount);
	CDlg.SetLocalChannel(nTempLocalChannel);

	if (m_bNeedMode)
	{
		CDlg.SetLocalMode(emTempLocalMode);
	}

	if (IDOK == CDlg.DoModal())
	{
		CDlg.GetLocalChannel(m_stuGetBypass.pnLocal);
		m_stuGetBypass.nLocalCount = CDlg.GetActualCount();
		m_nLastLocalCount = __min(m_edLocalCountForGet, m_stuGetBypass.nLocalCount);
	}
	else
	{
		m_nLastLocalCount = 0;
	}
}

void CDlgSetBypass::OnSetbypassButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId <= 0)
	{
		MessageBox(ConvertString("Please login first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}
	if (m_edExtendedCountForGet <= 0 && m_edLocalCountForGet <= 0)
	{
		MessageBox(ConvertString("please input extendedCount first!", DLG_BYPASS), ConvertString("Prompt"));
		return;
	}

	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_GET_BYPASS, 
		(char*)&m_stuGetBypass, sizeof(NET_DEVSTATE_GET_BYPASS), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("QueryChannelMode failed:", DLG_BYPASS), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
		return;
	} 
	else
	{
		m_bNeedMode = TRUE;
		MessageBox(ConvertString("QueryChannelMode bypass ok!", DLG_BYPASS), ConvertString("Prompt"));	
	}
}

