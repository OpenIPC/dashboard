// DlgVersion.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgVersion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgVersion dialog


CDlgVersion::CDlgVersion(CWnd* pParent /* = NULL */, LLONG hLoginId /* = 0 */)
	: CDialog(CDlgVersion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgVersion)
		// NOTE: the ClassWizard will add member initialization here
	m_hLoginId = hLoginId;
	//}}AFX_DATA_INIT
}


void CDlgVersion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgVersion)
	DDX_Control(pDX, IDC_STATIC_VERSION, m_staVersion);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgVersion, CDialog)
	//{{AFX_MSG_MAP(CDlgVersion)
	ON_BN_CLICKED(IDC_VERSION_UPDATE, OnVersionUpdate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgVersion message handlers

BOOL CDlgVersion::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_VERSION);
	
	// TODO: Add extra initialization here
	OnVersionUpdate();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgVersion::OnVersionUpdate() 
{
	// TODO: Add your control notification handler code here
	if (!m_hLoginId)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_VERSION), ConvertString("Prompt"));
		return;
	}

	int nRet = 0;
	DHDEV_VERSION_INFO stuInfo = {0};
	BOOL bRet = CLIENT_QueryDevState(m_hLoginId, DH_DEVSTATE_SOFTWARE, (char*)&stuInfo, sizeof(stuInfo), &nRet, SDK_API_WAIT);
	if (bRet)
	{
		CString csVer;
		csVer.Format("%s: %s\r\n\
%s: %s\r\n\
%s: %04d-%02d-%02d\r\n",
ConvertString("Serial", DLG_VERSION),
			stuInfo.szDevSerialNo,
			ConvertString("SoftwareVersion", DLG_VERSION),
			stuInfo.szSoftWareVersion,
			ConvertString("ReleaseTime", DLG_VERSION),
			((stuInfo.dwSoftwareBuildDate>>16) & 0xffff),
			((stuInfo.dwSoftwareBuildDate>>8) & 0xff),
			(stuInfo.dwSoftwareBuildDate & 0xff));
		m_staVersion.SetWindowText(csVer);
	} 
	else
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("get Version failed with code :", DLG_VERSION), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
	}
}
