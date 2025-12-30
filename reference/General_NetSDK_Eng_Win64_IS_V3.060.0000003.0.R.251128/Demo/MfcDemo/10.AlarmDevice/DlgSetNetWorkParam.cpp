// DlgSetNetWorkParam.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgSetNetWorkParam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSetNetWorkParam dialog


CDlgSetNetWorkParam::CDlgSetNetWorkParam(CWnd* pParent /*=NULL*/, LLONG lLoginId /*=0*/)
	: CDialog(CDlgSetNetWorkParam::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSetNetWorkParam)
	m_edConnectBufSize = 0;
	m_edConnectTime = 0;
	m_edConnectTryNum = 0;
	m_edGetDevInfoTime = 0;
	m_edPicBufSize = 0;
	m_edPlaybackBufSize = 0;
	m_edSearchRecordTime = 0;
	m_edSubConnectSpaceTime = 0;
	m_edSubDisconnectTime = 0;
	m_edWaittime = 0;
	m_edGetConnInfoTime = 0;
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	memset(&m_stuInfo, 0, sizeof(NET_PARAM));
}

void CDlgSetNetWorkParam::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSetNetWorkParam)
	DDX_Control(pDX, IDC_SETNETWORKPARAM_CMB_NETTYPE, m_cmbNetType);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_CONNECTBUFSIZE, m_edConnectBufSize);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_CONNECTTIME, m_edConnectTime);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_CONNECTTRYNUM, m_edConnectTryNum);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_GETDEVINFOTIME, m_edGetDevInfoTime);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_PICBUFSIZE, m_edPicBufSize);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_PLAYBACKBUFSIZE, m_edPlaybackBufSize);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_SEARCHRECORDTIME, m_edSearchRecordTime);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_SUBCONNECTSPACETIME, m_edSubConnectSpaceTime);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_SUBDISCONNETTIME, m_edSubDisconnectTime);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_WAITTIME, m_edWaittime);
	DDX_Text(pDX, IDC_SETNETWORKPARAM_EDT_GETCONNINFOTIME, m_edGetConnInfoTime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSetNetWorkParam, CDialog)
	//{{AFX_MSG_MAP(CDlgSetNetWorkParam)
	ON_BN_CLICKED(IDC_SETNETWORKPARAM_BTN_SET, OnSetnetworkparamBtnSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSetNetWorkParam message handlers

BOOL CDlgSetNetWorkParam::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_SETNETWORKPARAM);
	
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_SETNETWORKPARAM), ConvertString("Prompt"));
		return TRUE;
	}
	
	m_cmbNetType.ResetContent();
	m_cmbNetType.InsertString(-1, ConvertString("LAN", DLG_SETNETWORKPARAM));
	m_cmbNetType.InsertString(-1, ConvertString("WAN", DLG_SETNETWORKPARAM));

	Init();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetNetWorkParam::OnSetnetworkparamBtnSet() 
{
	// TODO: Add your control notification handler code here
	if (m_lLoginId == 0)
	{
		MessageBox(ConvertString(CString("we haven't login a device yet!"), DLG_SETNETWORKPARAM), ConvertString("Prompt"));
		return ;
	}
	DlgToStu();
	CLIENT_SetNetworkParam(&m_stuInfo);
	MessageBox(ConvertString(CString("set network param ok!"), DLG_SETNETWORKPARAM), ConvertString("Prompt"));
}

void CDlgSetNetWorkParam::Init()
{
	m_edConnectBufSize = 250;
	m_edConnectTime = 1500;
	m_edConnectTryNum = 1;
	m_edGetDevInfoTime = 1000;
	m_edPicBufSize = 2;
	m_edPlaybackBufSize = 4;
	m_edSearchRecordTime = 3000;
	m_edSubConnectSpaceTime = 10;
	m_edSubDisconnectTime = 60000;
	m_edWaittime = 5000;
	m_edGetConnInfoTime = 1000;
	m_cmbNetType.SetCurSel(0);
	UpdateData(FALSE);
}

void CDlgSetNetWorkParam::DlgToStu()
{
	UpdateData(TRUE);
	m_stuInfo.nWaittime = m_edWaittime;
	m_stuInfo.nConnectTime = m_edConnectTime;
	m_stuInfo.nConnectTryNum = m_edConnectTryNum;
	m_stuInfo.nSubConnectSpaceTime = m_edSubConnectSpaceTime;
	m_stuInfo.nGetDevInfoTime = m_edGetDevInfoTime;
	m_stuInfo.nConnectBufSize = m_edConnectBufSize * 1024;
	m_stuInfo.nGetConnInfoTime = m_edGetConnInfoTime;
	m_stuInfo.nSearchRecordTime = m_edSearchRecordTime;
	m_stuInfo.nsubDisconnetTime = m_edSubDisconnectTime;
	m_stuInfo.byNetType = m_cmbNetType.GetCurSel();
	m_stuInfo.byPlaybackBufSize = m_edPlaybackBufSize;
	m_stuInfo.nPicBufSize = m_edPicBufSize * 1024 * 1024;
}
