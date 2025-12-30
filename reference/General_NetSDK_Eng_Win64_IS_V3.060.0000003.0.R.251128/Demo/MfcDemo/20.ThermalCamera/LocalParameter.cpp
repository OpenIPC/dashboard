// LocalParameter.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "LocalParameter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocalParameter dialog


CLocalParameter::CLocalParameter(CWnd* pParent /*=NULL*/,LLONG lLoginId)
	: CDialog(CLocalParameter::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLocalParameter)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_iLoginID = lLoginId;
}


void CLocalParameter::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocalParameter)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocalParameter, CDialog)
	//{{AFX_MSG_MAP(CLocalParameter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocalParameter message handlers

BOOL CLocalParameter::OnInitDialog() 
{
	CDialog::OnInitDialog();

   
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
