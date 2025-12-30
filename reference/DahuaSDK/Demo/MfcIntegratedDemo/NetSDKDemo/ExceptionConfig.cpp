// ExceptionConfig.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ExceptionConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExceptionConfig dialog


CExceptionConfig::CExceptionConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CExceptionConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExceptionConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExceptionConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExceptionConfig)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExceptionConfig, CDialog)
	//{{AFX_MSG_MAP(CExceptionConfig)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExceptionConfig message handlers
