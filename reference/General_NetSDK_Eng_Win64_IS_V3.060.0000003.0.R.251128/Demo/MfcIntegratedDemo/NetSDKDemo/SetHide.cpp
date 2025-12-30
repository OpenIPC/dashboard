// SetHide.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "SetHide.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetHide dialog

CSetHide::CSetHide(CWnd* pParent /*=NULL*/)
	: CDialog(CSetHide::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetHide)
	m_bSetHide = FALSE;
	m_bShowHide = FALSE;
	//}}AFX_DATA_INIT
}


void CSetHide::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetHide)
	DDX_Control(pDX, IDC_DETECTPIC, m_DetectPic);
	DDX_Check(pDX, IDC_SETHIDE_CHECK, m_bSetHide);
	DDX_Check(pDX, IDC_SHOWHIDE_CHECK, m_bShowHide);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSetHide, CDialog)
	//{{AFX_MSG_MAP(CSetHide)
	ON_BN_CLICKED(IDC_SHOWHIDE_CHECK, OnShowhideCheck)
	ON_BN_CLICKED(IDC_SETHIDE_CHECK, OnSethideCheck)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BTNOK, OnSetHideOK)
	ON_BN_CLICKED(IDC_BTNQUIT, OnQuit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetHide message handlers
RECT g_ShowRect1[MAX_SHELTERNUM];
RECT g_MouseRect1[MAX_SHELTERNUM];
int		m_idetectnum1 = 4;

CSetHide::SetShelter(int nIndex, NET_DEV_SHELTER *shelter)
{
	memcpy(&m_struShelter[nIndex], shelter, sizeof(NET_DEV_SHELTER));
}

CSetHide::GetShelter(int nIndex, NET_DEV_SHELTER *shelter)
{
	memcpy(shelter, &m_struShelter[nIndex],sizeof(NET_DEV_SHELTER));
}

BOOL CSetHide::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	int i;
	m_DetectPic.MoveWindow(45, 10, 352, 288);
	for(i=0; i<MAX_SHELTERNUM; i++)
	{
		g_MouseRect1[i].left = m_struShelter[i].wHideAreaTopLeftX/2;
		g_MouseRect1[i].top = m_struShelter[i].wHideAreaTopLeftY/2;
		g_MouseRect1[i].bottom = (m_struShelter[i].wHideAreaTopLeftY + m_struShelter[i].wHideAreaHeight)/2;
		g_MouseRect1[i].right = (m_struShelter[i].wHideAreaTopLeftX + m_struShelter[i].wHideAreaWidth)/2;
	}
/*	
	NET_DVR_CLIENTINFO playstru;
	playstru.hPlayWnd = m_DetectPic.GetSafeHwnd();
	playstru.lChannel = m_iChannel;
	playstru.lLinkMode = 0;
	playstru.sMultiCastIP = "";	 

	m_iPlayhandle = NET_DVR_RealPlay(m_lServerID, &playstru);	

	if(m_iPlayhandle == -1)
	{
		CString sTemp;
		sTemp.Format("ÍøÂçÁ¬½ÓÊ§°Ü!");
		AfxMessageBox(sTemp);
	}	
	*/	
	//m_bDrawdetect = FALSE;
	GetDlgItem(IDC_SHOWHIDE_CHECK)->EnableWindow(TRUE);
	GetDlgItem(IDC_SETHIDE_CHECK)->EnableWindow(TRUE);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSetHide::OnShowhideCheck() 
{
//	if (m_iPlayhandle < 0)
//		return;
	if (!UpdateData(TRUE))
	{
		return;
	}
	if (m_bSetHide)
	{
		m_bSetHide = FALSE;
		m_bDrawdetect = FALSE;
		UpdateData(FALSE);
		GetDlgItem(IDC_BTNOK)->EnableWindow(FALSE);
	}
	if (m_bShowHide)
	{
	//	NET_DVR_RigisterDrawFun(m_iPlayhandle,NULL, 0);
	//	Sleep(200);
	//	NET_DVR_RigisterDrawFun(m_iPlayhandle,DrawFunShow1, 0);
	}
	else
	{
	//	NET_DVR_RigisterDrawFun(m_iPlayhandle,NULL, 0);
	}
}

void CSetHide::OnSethideCheck() 
{
//	if (m_iPlayhandle < 0)
//		return;
	if (!UpdateData(TRUE))
	{
		return;
	}
	if (m_bShowHide)
	{
		m_bShowHide = FALSE;
		UpdateData(FALSE);
	}
	if (m_bSetHide)
	{
	//	NET_DVR_RigisterDrawFun(m_iPlayhandle,NULL,0);
		m_bDrawdetect = TRUE;
		m_idetectnum1 = 0;
		for(int i=0; i<MAX_SHELTERNUM; i++)
		{
			g_MouseRect1[i].left = 0;
			g_MouseRect1[i].top = 0;
			g_MouseRect1[i].bottom = 0;
			g_MouseRect1[i].right = 0;
		}
		GetDlgItem(IDC_BTNOK)->EnableWindow(TRUE);
	}
	else
	{
		m_bDrawdetect = FALSE;
	//	NET_DVR_RigisterDrawFun(m_iPlayhandle,NULL,0);
	//	GetDlgItem(IDC_BTNOK)->EnableWindow(FALSE);
	}
}

void CSetHide::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bDrawdetect)
	{
		if (MK_CONTROL&nFlags)
		{
		//	NET_DVR_RigisterDrawFun(m_iPlayhandle, DrawFun0, 0);
			if (m_idetectnum1 >=MAX_SHELTERNUM)
				m_idetectnum1 = 0;
			point.x -= 45;
			point.y -= 10;
			if (point.x < 0)
				point.x = 0;
			g_MouseRect1[m_idetectnum1].left=point.x/16*16;
			if (point.y < 0)
				point.y = 0;
			g_MouseRect1[m_idetectnum1].top=point.y/16*16;
			g_MouseRect1[m_idetectnum1].right=g_MouseRect1[m_idetectnum1].left;
			g_MouseRect1[m_idetectnum1].bottom=g_MouseRect1[m_idetectnum1].top;
			POINT tPoint=point;
			g_ShowRect1[m_idetectnum1].left=tPoint.x/16*16;
			g_ShowRect1[m_idetectnum1].top=tPoint.y/16*16;
			g_ShowRect1[m_idetectnum1].right=tPoint.x/16*16+1;
			g_ShowRect1[m_idetectnum1].bottom=tPoint.y/16*16+1;
			m_idetectnum1 ++;
		}
	}		
	CDialog::OnLButtonDown(nFlags, point);
}

void CSetHide::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bDrawdetect)
	{
		if (MK_CONTROL&nFlags&&MK_LBUTTON&nFlags)
		{
			point.x -= 45;
			point.y -= 10;
			if (point.x > 352)
				point.x = 352;
			g_MouseRect1[m_idetectnum1-1].right=point.x/16*16;
			if (point.y > 288)
				point.y = 288;
			g_MouseRect1[m_idetectnum1-1].bottom=point.y/16*16;
			POINT tPoint=point;
			g_ShowRect1[m_idetectnum1-1].right=tPoint.x/16*16;
			g_ShowRect1[m_idetectnum1-1].bottom=tPoint.y/16*16;
		}
	}	
	CDialog::OnMouseMove(nFlags, point);
}

void CSetHide::OnSetHideOK() 
{
	int k=0;
	for(k=0; k<MAX_SHELTERNUM; k++)
	{
		m_struShelter[k].wHideAreaTopLeftX = 0;
		m_struShelter[k].wHideAreaTopLeftY = 0;
		m_struShelter[k].wHideAreaWidth = 0;
		m_struShelter[k].wHideAreaHeight = 0;
	}
	for (k=0; k<m_idetectnum1; k++)
	{
		if (g_MouseRect1[k].top <= g_MouseRect1[k].bottom)
		{
			if(g_MouseRect1[k].left <= g_MouseRect1[k].right)
			{
				m_struShelter[k].wHideAreaTopLeftX = g_MouseRect1[k].left*2;
				m_struShelter[k].wHideAreaTopLeftY = g_MouseRect1[k].top*2;
				m_struShelter[k].wHideAreaWidth = (g_MouseRect1[k].right-g_MouseRect1[k].left)*2;
				m_struShelter[k].wHideAreaHeight = (g_MouseRect1[k].bottom-g_MouseRect1[k].top)*2;		
			}
			else
			{
				m_struShelter[k].wHideAreaTopLeftX = g_MouseRect1[k].right*2;
				m_struShelter[k].wHideAreaTopLeftY = g_MouseRect1[k].top*2;
				m_struShelter[k].wHideAreaWidth = (g_MouseRect1[k].left-g_MouseRect1[k].right)*2;
				m_struShelter[k].wHideAreaHeight = (g_MouseRect1[k].bottom-g_MouseRect1[k].top)*2;						
			}
		}
		else
		{
			if(g_MouseRect1[k].left <= g_MouseRect1[k].right)
			{
				m_struShelter[k].wHideAreaTopLeftX = g_MouseRect1[k].left*2;
				m_struShelter[k].wHideAreaTopLeftY = g_MouseRect1[k].bottom*2;
				m_struShelter[k].wHideAreaWidth = (g_MouseRect1[k].right-g_MouseRect1[k].left)*2;
				m_struShelter[k].wHideAreaHeight = (g_MouseRect1[k].top-g_MouseRect1[k].bottom)*2;
			}
			else
			{
				m_struShelter[k].wHideAreaTopLeftX = g_MouseRect1[k].right*2;
				m_struShelter[k].wHideAreaTopLeftY = g_MouseRect1[k].bottom*2;
				m_struShelter[k].wHideAreaWidth = (g_MouseRect1[k].left-g_MouseRect1[k].right)*2;
				m_struShelter[k].wHideAreaHeight = (g_MouseRect1[k].top-g_MouseRect1[k].bottom)*2;
			}
		}
	}
}

void CSetHide::OnQuit() 
{
/*
	if (m_iPlayhandle >= 0)
	{
		NET_DVR_RigisterDrawFun(m_iPlayhandle,NULL, 0);
		NET_DVR_StopRealPlay(m_iPlayhandle);
		m_iPlayhandle = -1;
	}*/

	CDialog::OnOK();
}
