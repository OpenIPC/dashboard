// SelectPannel.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "SelectPannel.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectPannel dialog


CSelectPannel::CSelectPannel(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectPannel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectPannel)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectPannel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectPannel)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectPannel, CDialog)
	//{{AFX_MSG_MAP(CSelectPannel)
	ON_BN_CLICKED(IDC_BTN_SEL_DEVLIST, OnBtnSelDevlist)
	ON_BN_CLICKED(IDC_BTN_SEL_PTZ, OnBtnSelPtz)
	ON_BN_CLICKED(IDC_BTN_SEL_COLOR, OnBtnSelColor)
	ON_BN_CLICKED(IDC_BTN_SEL_PLAYCTRL, OnBtnSelPlayctrl)
	ON_BN_CLICKED(IDC_BTN_SEL_NORMAL, OnBtnSelNormal)
	ON_BN_CLICKED(IDC_BTN_SEL_ADVANCE, OnBtnSelAdvance)
	ON_BN_CLICKED(IDC_BTN_SEL_DATA, OnBtnSelData)
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectPannel message handlers

void CSelectPannel::OnBtnSelDevlist() 
{
	((CNetSDKDemoDlg *)GetParent())->SeleteDevListPannel();
}

void CSelectPannel::OnBtnSelPtz() 
{
	((CNetSDKDemoDlg *)GetParent())->SeletePTZPannel();
}

void CSelectPannel::OnBtnSelColor() 
{
	((CNetSDKDemoDlg *)GetParent())->SeleteColorPannel();
}

void CSelectPannel::OnBtnSelPlayctrl() 
{
	((CNetSDKDemoDlg *)GetParent())->SeletePlayCtrlPannel();
}

void CSelectPannel::OnBtnSelNormal() 
{
	((CNetSDKDemoDlg *)GetParent())->SeleteNormalPannel();
}

void CSelectPannel::OnBtnSelAdvance() 
{
	((CNetSDKDemoDlg *)GetParent())->SeleteAdvancePannel();
}

void CSelectPannel::OnBtnSelData() 
{
	
}

BOOL CSelectPannel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	g_SetWndStaticText(this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSelectPannel::PreTranslateMessage(MSG* pMsg) 
{
	if (WM_KEYDOWN == pMsg->message && 
		(VK_ESCAPE == pMsg->wParam || VK_RETURN == pMsg->wParam))
	{
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CSelectPannel::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC dc;
	BOOL b = dc.Attach(lpDrawItemStruct->hDC);

	if (b)
	{
		switch(nIDCtl)
		{
		case IDC_BTN_SEL_NORMAL:
			{	
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				CBrush brush;
				brush.CreateSolidBrush(RGB(200,200,255));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(10,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_NORMAL), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}

		case IDC_BTN_SEL_ADVANCE:
			{
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				
				CBrush brush;
				brush.CreateSolidBrush(RGB(170,170,255));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(10,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_ADVANCE), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}
		case IDC_BTN_SEL_PTZ:
			{	
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				CBrush brush;
				brush.CreateSolidBrush(RGB(140,140, 255));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(12,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_PTZ), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}
		case IDC_BTN_SEL_DEVLIST:
			{
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				
				CBrush brush;
				brush.CreateSolidBrush(RGB(255,255,255));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(3,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_DEVLIST), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}
		case IDC_BTN_SEL_COLOR:
			{	
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				CBrush brush;
				brush.CreateSolidBrush(RGB(220,220,220));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(5,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_COLOR), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}
		case IDC_BTN_SEL_DATA:
			{
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				
				CBrush brush;
				brush.CreateSolidBrush(RGB(195,195,195));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(5,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_CBDATA), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}
		case IDC_BTN_SEL_PLAYCTRL:
			{
				CRect rect;
				GetDlgItem(nIDCtl)->GetClientRect(&rect);
				
				CBrush brush;
				brush.CreateSolidBrush(RGB(170,170,170));
				dc.FillRect(&rect, &brush);
				
				rect.DeflateRect(1,1);
				dc.DrawFocusRect(&rect);
				
				rect.DeflateRect(5,3);
				int mode = dc.SetBkMode(TRANSPARENT);
				dc.DrawText(ConvertString(NAME_BTN_PBCTRL), &rect, 0);
				dc.SetBkMode(mode);
				break;
			}
		default:
			break;	
		}
	}
	
	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}
