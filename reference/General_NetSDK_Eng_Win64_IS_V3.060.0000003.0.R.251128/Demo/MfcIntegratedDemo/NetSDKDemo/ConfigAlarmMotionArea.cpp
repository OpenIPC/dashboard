// ConfigAlarmMotionArea.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "ConfigAlarmMotionArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotionArea dialog


CConfigAlarmMotionArea::CConfigAlarmMotionArea(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigAlarmMotionArea::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigAlarmMotionArea)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bDrawing = FALSE;
	m_blckWid = 0;
	m_blckHght = 0;
	m_stpoint.x = -1;
	m_stpoint.y = -1;
	m_lastpoint.x = -1;
	m_lastpoint.y = -1;
}


void CConfigAlarmMotionArea::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigAlarmMotionArea)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigAlarmMotionArea, CDialog)
	//{{AFX_MSG_MAP(CConfigAlarmMotionArea)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_OK, OnOk)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotionArea message handlers

BOOL CConfigAlarmMotionArea::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	CRect m_rect;
	GetDlgItem(IDC_AREA_BOARD)->GetClientRect(&m_rect);
	GetDlgItem(IDC_AREA_BOARD)->ClientToScreen(&m_rect);
	ScreenToClient(&m_rect);
	int blckWid = (m_rect.Width()-DH_MOTION_COL+1)/DH_MOTION_COL;
	m_blckWid = blckWid;
	int blckHght = (m_rect.Height()-DH_MOTION_ROW+1)/DH_MOTION_ROW;
	m_blckHght = blckHght;
	for (int i = 0; i < DH_MOTION_ROW; i++)
	{
		for (int j = 0; j < DH_MOTION_COL; j++)
		{
			m_block[i][j].Create(
			NULL, 
			NULL, 
			WS_VISIBLE | WS_CHILD,
			CRect(m_rect.left+j*blckWid+j, m_rect.top+i*blckHght+i, m_rect.left+(j+1)*blckWid+j, m_rect.top+(i+1)*blckHght+i),
			this,
			1979,
			NULL);
			m_block[i][j].PostMessage(MSG_SHOW);
			m_block[i][j].SetPos(i,j);
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigAlarmMotionArea::SetArea(BYTE *area)
{
	if (!area)
	{
		return;
	}
	memcpy(m_myArea, area, DH_MOTION_COL*DH_MOTION_ROW);

	for (int i = 0; i < DH_MOTION_ROW; i++)
	{
		for (int j = 0; j < DH_MOTION_COL; j++)
		{
			m_block[i][j].SetStatus(m_myArea[i][j]);
		}
	}
}

void CConfigAlarmMotionArea::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CDialog::OnLButtonDown(nFlags, point);
}

void CConfigAlarmMotionArea::RecordPoint(BYTE x, BYTE y)
{
	m_stpoint.x = x;
	m_stpoint.y = y;
	m_lastpoint.x = x;
	m_lastpoint.y = y;
	m_bDrawing = TRUE;

	m_block[x][y].PostMessage(MSG_SELETED);
}

void CConfigAlarmMotionArea::ReleasePoint()
{
	m_bDrawing = FALSE;
}

void CConfigAlarmMotionArea::OnLButtonUp(UINT nFlags, CPoint point) 
{
	m_bDrawing = FALSE;

	CDialog::OnLButtonUp(nFlags, point);
}

BOOL IsBetween(int lft, int mid, int rht)
{
	if (lft >= rht)
	{
		return (mid >= rht && mid <= lft);
	}
	else
	{
		return (mid >= lft && mid <= rht);
	}
}

void CConfigAlarmMotionArea::MovePoint(DWORD dwParm, BYTE x, BYTE y)
{
	if (!m_bDrawing || !(dwParm&MK_LBUTTON) || (m_lastpoint.x == x && m_lastpoint.y == y))
	{
		return;
	}

	for (int i = 0; i < DH_MOTION_ROW; i++)
	{
		for (int j = 0; j < DH_MOTION_COL; j++)
		{
			if(((IsBetween(m_stpoint.x, i, x) && IsBetween(m_stpoint.y, j, y)) &&
				!(IsBetween(m_stpoint.x, i, m_lastpoint.x) && IsBetween(m_stpoint.y, j, m_lastpoint.y))) ||
				(IsBetween(m_stpoint.x, i, m_lastpoint.x) && IsBetween(m_stpoint.y, j, m_lastpoint.y)) &&
				!((IsBetween(m_stpoint.x, i, x) && IsBetween(m_stpoint.y, j, y))))
			{
				m_block[i][j].PostMessage(MSG_SELETED);
			}
		}
	}

	m_lastpoint.x = x;
	m_lastpoint.y = y;
}

HBRUSH CConfigAlarmMotionArea::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CConfigAlarmMotionArea::OnOk() 
{
	for (int i = 0; i < DH_MOTION_ROW; i++)
	{
		for (int j = 0; j < DH_MOTION_COL; j++)
		{
			m_myArea[i][j] = m_block[i][j].IsSeleted()?1:0;
		}
	}
	CDialog::OnOK();
}

void CConfigAlarmMotionArea::OnCancel() 
{
	CDialog::OnCancel();
}
