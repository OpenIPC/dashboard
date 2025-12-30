// MylistCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "searchdevice.h"
#include "MylistCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMylistCtrl

CMylistCtrl::CMylistCtrl()
{
}

CMylistCtrl::~CMylistCtrl()
{
}


BEGIN_MESSAGE_MAP(CMylistCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CMylistCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw) 
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMylistCtrl message handlers
void CMylistCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)    
{    
    // TODO: Add your control notification handler code here     
    *pResult = CDRF_DODEFAULT;    
    NMLVCUSTOMDRAW * lplvdr=(NMLVCUSTOMDRAW*)pNMHDR;    
    NMCUSTOMDRAW &nmcd = lplvdr->nmcd;  
	//Judge status
    switch(lplvdr->nmcd.dwDrawStage)     
    {    
    case CDDS_PREPAINT:    
        {    
            *pResult = CDRF_NOTIFYITEMDRAW;    
            break;    
        }
		
    case CDDS_ITEMPREPAINT:     
        {    
            TEXT_BK tb;  
            if(MapItemColor.Lookup(nmcd.dwItemSpec, tb))    
            {    
                lplvdr->clrText = tb.colText;     
                lplvdr->clrTextBk = tb.colTextBk;    
                *pResult = CDRF_DODEFAULT;    
            }    
        }    
        break;    
    }    
}    
  
void CMylistCtrl::ClearColor()  
{  
    MapItemColor.RemoveAll();  
}  
  
void CMylistCtrl::SetItemColor(DWORD iItem, COLORREF TextColor, COLORREF TextBkColor)  
{    
    TEXT_BK tb;  
    tb.colText = TextColor;  
    tb.colTextBk = TextBkColor;  
    
	//set a line color
    MapItemColor.SetAt(iItem, tb);     
    //re-dyeing
	this->RedrawItems(iItem, iItem);
           
    this->SetFocus();        
    UpdateWindow();    
}    
    
void CMylistCtrl::SetAllItemColor(DWORD iItem, COLORREF TextColor, COLORREF TextBkColor)    
{     
    TEXT_BK tb;  
    tb.colText = TextColor;  
    tb.colTextBk = TextBkColor;  
  
    if(iItem > 0)    
    {    
        for(DWORD numItem = 0; numItem < iItem ;numItem ++)    
        {       
            MapItemColor.SetAt(numItem, tb);    
            this->RedrawItems(numItem, numItem);    
        }    
    }    
      
    return;     
}   


