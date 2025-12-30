// SetPTZZoom.cpp : 实现文件
//

#include "stdafx.h"
#include "TestOSD.h"
#include "SetPTZZoom.h"
#include "TestOSDDlg.h"

extern LLONG g_lLogin;
extern unsigned int g_iChannelIndex;
// CSetPTZZoom 对话框

IMPLEMENT_DYNAMIC(CSetPTZZoom, CDialog)

CSetPTZZoom::CSetPTZZoom(CWnd* pParent /*=NULL*/)
	: CDialog(CSetPTZZoom::IDD, pParent)
	, m_nLeft(0)
	, m_nTop(0)
	, m_nTime(0)
	, m_bMainBlend(FALSE)
	, m_bPreviewBlend(FALSE)
{
	memset(&m_stuPTZZoom,0,sizeof(m_stuPTZZoom));
	m_stuPTZZoom.dwSize = sizeof(m_stuPTZZoom);
}

CSetPTZZoom::~CSetPTZZoom()
{
}

void CSetPTZZoom::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LEFT, m_nLeft);
//	DDV_MinMaxInt(pDX, m_nLeft, 0, 8191);
	DDX_Text(pDX, IDC_EDIT_TOP, m_nTop);
//	DDV_MinMaxInt(pDX, m_nTop, 0, 8191);
	DDX_Text(pDX, IDC_EDIT_TIME, m_nTime);
//	DDV_MinMaxInt(pDX, m_nTime, 0, INT_MAX);
	DDX_Check(pDX, IDC_CHECK_MAINBLEND, m_bMainBlend);
	DDX_Check(pDX, IDC_CHECK_PREVIEWBLEND, m_bPreviewBlend);
}


BEGIN_MESSAGE_MAP(CSetPTZZoom, CDialog)
	ON_BN_CLICKED(IDC_GET, OnBnClickedGet)
	ON_BN_CLICKED(IDC_SET, OnBnClickedSet)
END_MESSAGE_MAP()


// CSetPTZZoom 消息处理程序

BOOL CSetPTZZoom::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	OnBnClickedGet();

	return TRUE;
}

void CSetPTZZoom::OnBnClickedGet()
{

	BOOL bRet = CLIENT_GetConfig(g_lLogin,NET_EM_CFG_OSD_PTZZOOM,g_iChannelIndex,&m_stuPTZZoom,sizeof(m_stuPTZZoom),5000);
	if (!bRet)
	{
		MessageBox(ConvertString("get OSD zoom overlay config failed!"),ConvertString("Prompt"));
	}
	BYTE byR = 0;
	BYTE byG = 0;
	BYTE byB = 0;
	BYTE byA = 0;
	byR = m_stuPTZZoom.stuFrontColor.nRed;
	byG = m_stuPTZZoom.stuFrontColor.nGreen;
	byB = m_stuPTZZoom.stuFrontColor.nBlue;
	byA = m_stuPTZZoom.stuFrontColor.nAlpha;
	((CIPAddressCtrl*)(GetDlgItem(IDC_IPADDRESS_FRONT)))->SetAddress(byR,byG,byB,byA);
	byR = m_stuPTZZoom.stuBackColor.nRed;
	byG = m_stuPTZZoom.stuBackColor.nGreen;
	byB = m_stuPTZZoom.stuBackColor.nBlue;
	byA = m_stuPTZZoom.stuBackColor.nAlpha;
	((CIPAddressCtrl*)(GetDlgItem(IDC_IPADDRESS_BACK)))->SetAddress(byR,byG,byB,byA);
	m_bMainBlend = m_stuPTZZoom.bMainBlend;
	m_bPreviewBlend = m_stuPTZZoom.bPreviewBlend;
	m_nTime = m_stuPTZZoom.nDisplayTime;
	m_nLeft = m_stuPTZZoom.stuRect.nLeft;
	m_nTop = m_stuPTZZoom.stuRect.nTop;
	UpdateData(FALSE);

}

void CSetPTZZoom::OnBnClickedSet()
{
	UpdateData();
	BYTE byR = 0;
	BYTE byG = 0;
	BYTE byB = 0;
	BYTE byA = 0;
	((CIPAddressCtrl*)(GetDlgItem(IDC_IPADDRESS_FRONT)))->GetAddress(byR,byG,byB,byA);
	m_stuPTZZoom.stuFrontColor.nRed = byR;
	m_stuPTZZoom.stuFrontColor.nGreen = byG;
	m_stuPTZZoom.stuFrontColor.nBlue = byB;
	m_stuPTZZoom.stuFrontColor.nAlpha = byA;

	((CIPAddressCtrl*)(GetDlgItem(IDC_IPADDRESS_BACK)))->GetAddress(byR,byG,byB,byA);
	m_stuPTZZoom.stuBackColor.nRed = byR;
	m_stuPTZZoom.stuBackColor.nGreen = byG;
	m_stuPTZZoom.stuBackColor.nBlue = byB;
	m_stuPTZZoom.stuBackColor.nAlpha = byA;

	m_stuPTZZoom.bMainBlend = m_bMainBlend;
	m_stuPTZZoom.bPreviewBlend = m_bPreviewBlend;
	m_stuPTZZoom.nDisplayTime = m_nTime;
	m_stuPTZZoom.stuRect.nLeft = m_nLeft;
	m_stuPTZZoom.stuRect.nRight = m_nLeft;
	m_stuPTZZoom.stuRect.nTop = m_nTop;
	m_stuPTZZoom.stuRect.nBottom = m_nTop;
	BOOL bRet = CLIENT_SetConfig(g_lLogin,NET_EM_CFG_OSD_PTZZOOM,g_iChannelIndex,&m_stuPTZZoom,sizeof(m_stuPTZZoom),5000);
	if (!bRet)
	{
		MessageBox(ConvertString("get OSD zoom overlay config failed!"),ConvertString("Prompt"));
	}

}
