// BSWndContainer.cpp : implementation file
//

#include "stdafx.h"
#include "BSWndContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBSWndContainer

CBSWndContainer::CBSWndContainer()
{
	// init active page pointer
	m_pActivePage	= NULL;

	// init window state
	m_bFullScreen	= FALSE;	// Full screen sign 
	m_bMultiScreen	= TRUE;		// Multiple-window sign 
	m_bAutoAdjustPos= FALSE;	// Auto adjust sign 

	SetDrawActivePage(TRUE);	//	Enable frame

	m_nShowPortion=100;			//	Display proportion 
}

CBSWndContainer::~CBSWndContainer()
{
	// remove all pages
	while(!m_PageList.IsEmpty())
		m_PageList.RemoveHead();
}


BEGIN_MESSAGE_MAP(CBSWndContainer, CWnd)
	//{{AFX_MSG_MAP(CBSWndContainer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBSWndContainer member functions

///////////////////////////////////////////////////
// call this function to create container object.
// it is override from cwnd class
BOOL CBSWndContainer::Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext )
{
	dwStyle|=WS_EX_TOOLWINDOW;
	return CWnd::Create(lpszClassName,lpszWindowName,dwStyle,rect,pParentWnd,nID,pContext );
}


///////////////////////////////////////////////////
// call this function to add a page wnd to 
// container. if success retrun TRUE,else return 
// FALSE.
BOOL CBSWndContainer::AddPage(CWnd *pWnd, BOOL bRepaint)
{
	// check parameter
	if(	!pWnd || !IsWindow(pWnd->m_hWnd) )	return FALSE;

	// check list 
	POSITION pos=m_PageList.Find(pWnd);
	if(pos!=NULL) 
	{
		return TRUE;
	}

	// added page
	m_PageList.AddTail(pWnd);

	if( m_bDrawActive ) DrawActivePage(FALSE);

	// reset active page
	SetActivePage(pWnd, bRepaint);

	return TRUE;
}

///////////////////////////////////////////////////
// call this function to remove a page wnd from
// container. 
CWnd *CBSWndContainer::DelPage(CWnd *pWnd)
{
	// check list
	POSITION pos=m_PageList.Find(pWnd);
	if(pos==NULL)
	{
		return NULL;
	}
	if(pWnd==m_pActivePage)
		if(m_pActivePage==GetPrevPage(pWnd))//m_PageList.IsEmpty()?NULL:m_PageList.GetHead();
			m_pActivePage=NULL;
		else m_pActivePage=GetPrevPage(pWnd);
	
	m_PageList.RemoveAt(pos);

	if (pWnd)
	{
		pWnd->ShowWindow(SW_HIDE);
	}

//	Invalidate();

	return pWnd;
}

///////////////////////////////////////////////////
// call this function to remove active page from
// container.
CWnd *CBSWndContainer::DelPage()
{
	return DelPage(m_pActivePage);
}

///////////////////////////////////////////////////
// call this function to set a page to be active
// page.
void CBSWndContainer::SetActivePage(CWnd *pWnd, BOOL bRepaint)
{
	// check parameter
	if(	!pWnd || !IsWindow(pWnd->m_hWnd) )	return;

	// if pWnd is the Active Page, return 
	if( m_pActivePage==pWnd ) return;

	// check list
	POSITION pos=m_PageList.Find(pWnd);
	if(pos==NULL)
	{
		return;
	}

	if(bRepaint) UpdateWnd();

	if( m_bDrawActive ) DrawActivePage(FALSE);

	m_pActivePage=pWnd;

	if( m_bDrawActive ) DrawActivePage(TRUE);
}

///////////////////////////////////////////////////
// call this function to get the active page's
// pointer. if no active page,return NULL;
CWnd *CBSWndContainer::GetActivePage()
{
	return m_pActivePage;
}

CWnd *CBSWndContainer::GetTailPage()
{
	return m_PageList.GetTail();
}

///////////////////////////////////////////////////
// call this function to get the next page by
// the page that user defined. if the defined 
// page is not find in container, return NULL.
CWnd *CBSWndContainer::GetNextPage(CWnd *pWnd)
{
	// check parameter
	if(	!pWnd || !IsWindow(pWnd->m_hWnd) ) return NULL;

	// check list
	POSITION pos=m_PageList.Find(pWnd);
	if(pos==NULL)
	{
		return NULL;
	}
	
	//
	m_PageList.GetNext(pos);
	if(pos==NULL)
		return m_PageList.GetHead();
	else 
		return m_PageList.GetNext(pos);
}

///////////////////////////////////////////////////
// call this function to get the prev page by
// the page that user defined. if the defined
// page is not find in container,return NULL.
CWnd *CBSWndContainer::GetPrevPage(CWnd *pWnd)
{
	// check parameter
	if(	!pWnd || !IsWindow(pWnd->m_hWnd) ) return NULL;

	// check list
	POSITION pos=m_PageList.Find(pWnd);
	if(pos==NULL)
	{
		return NULL;
	}
	
	//
	m_PageList.GetPrev(pos);
	if(pos==NULL)
		return m_PageList.GetTail();
	else 
		return m_PageList.GetPrev(pos);
}

CWnd *CBSWndContainer::GetPage(int nIndex)
{
	CWnd *pRet = NULL;
	POSITION pos = m_PageList.FindIndex(nIndex);
	if(pos == NULL) return pRet;

	return m_PageList.GetAt(pos);
}

int CBSWndContainer::GetCount() const
{
	return m_PageList.GetCount();
}

///////////////////////////////////////////////////
// call this function to  page wnds,when
// the window is resized.
void CBSWndContainer::UpdateWnd()
{
	if(!IsWindowVisible()||IsIconic()) return;
/////////////////////
//Calculate display total zone 

	//To get current window device coordinates
	CRect rtContainer;
	GetClientRect(&rtContainer);
	GetShowRect(&rtContainer);
	rtContainer.DeflateRect(1,1);

	//Adjust Container position 
	if(m_bAutoAdjustPos)		
		AdjustRect(&rtContainer);

/////////////////////
//
	if(m_bMultiScreen)
	{ //Multiple-window status 
		CRect rt;
		int nCount=m_PageList.GetCount();
		int i=0;
		for(POSITION pos=m_PageList.GetHeadPosition();pos!=NULL;)
		{
			CWnd *p=m_PageList.GetNext(pos);

			rt=rtContainer;
			CalcPageRect(&rt,i,nCount);
			rt.DeflateRect(WINDOW_SPACE,WINDOW_SPACE,WINDOW_SPACE,WINDOW_SPACE); 
			p->MoveWindow(&rt);
			p->ShowWindow(SW_SHOW);
			i++;
		}
		if( m_bDrawActive && m_PageList.GetCount()>1 ) DrawActivePage(TRUE);
	}
	else
	{ //One-window status 
		for(POSITION pos=m_PageList.GetHeadPosition();pos!=NULL;)
		{
			CWnd *p=m_PageList.GetNext(pos);
			if(p==m_pActivePage)
				p->MoveWindow(&rtContainer);
			else 
			{
				if(m_bFullScreen)
					p->MoveWindow(0,0,1,1);
				else
					p->MoveWindow(rtContainer.right+1,rtContainer.bottom+1,1,1);
			}
		}
	}
}

///////////////////////////////////////////////////
// full screen
void CBSWndContainer::SetFullScreen(BOOL bFlag)
{
	if(bFlag==m_bFullScreen) return;

	if( bFlag )
	{//Full screen 
		//Get displayer resolution 
		int cx=GetSystemMetrics(SM_CXSCREEN);
		int cy=GetSystemMetrics(SM_CYSCREEN);

		//Save position information 
		GetWindowPlacement(&_temppl);
		//Modify style 
		ModifyStyle(WS_CHILD,WS_POPUP);
		//Modify main-window 
		_tempparent=SetParent(NULL);
		_tempparent->ShowWindow(SW_HIDE);
		//Move window 
		MoveWindow(0,0,cx,cy);
	//	SetWindowPos(&wndTopMost,0,0,cx,cy,NULL);
	}
	else
	{//Restore
		//Restore main window 
		_tempparent->ShowWindow(SW_SHOW);
		SetParent(_tempparent);
		//Restore style 
		ModifyStyle(WS_POPUP,WS_CHILD);
		//Restore position 
		SetWindowPlacement(&_temppl);
	}

	m_bFullScreen=bFlag;
	Invalidate();
}
BOOL CBSWndContainer::GetFullScreen()
{
	return m_bFullScreen;
}

///////////////////////////////////////////////////
// multiscreen
void CBSWndContainer::SetMultiScreen(BOOL bFlag)
{
	if(m_bMultiScreen==bFlag) return;
	m_bMultiScreen=bFlag;
	Invalidate();
}
BOOL CBSWndContainer::GetMultiScreen()
{
	return m_bMultiScreen;
}

//////////////////////////////////////////////////
// autoadjustpos
void CBSWndContainer::SetAutoAdjustPos(BOOL bFlag)
{
	if(m_bAutoAdjustPos==bFlag) return;
	m_bAutoAdjustPos=bFlag;
	Invalidate();
}
BOOL CBSWndContainer::GetAutoAdjustPos()
{
	return m_bAutoAdjustPos;
}

//////////////////////////////////////////////////
// draw active page
void CBSWndContainer::SetDrawActivePage(BOOL bFlag,COLORREF clrTopLeft,COLORREF clrBottomRight)
{
	if(m_bDrawActive==bFlag) return;
	if(bFlag)
	{
		m_clrTopLeft=clrTopLeft;
		m_clrBottomRight=clrBottomRight;
	}
	m_bDrawActive=bFlag;
	DrawActivePage(bFlag);
}
BOOL CBSWndContainer::GetDrawActivePage()
{
	return m_bDrawActive;
}

//////////////////////////////////////////////////
//	Display percentage 
//	40 <= nPortion <=100
void CBSWndContainer::SetShowPortion(int nPortion)
{
	if(m_nShowPortion==nPortion) return;
	if(m_nShowPortion<40) m_nShowPortion=40;
	if(m_nShowPortion>100) m_nShowPortion=100;
	m_nShowPortion=nPortion;
	Invalidate();
}
int  CBSWndContainer::GetShowPortion()
{
	return m_nShowPortion;
}

///////////////////////////////////////////////////
// clean the no useful page in the container,
// return the page count.
int CBSWndContainer::UpdateList()
{
	POSITION posPrev;
	for(POSITION pos=m_PageList.GetHeadPosition();pos!=NULL;)
	{
		posPrev=pos;
		CWnd *p=m_PageList.GetNext(pos);
		if(!IsWindow(p->m_hWnd))
			m_PageList.RemoveAt(posPrev);
	}
	return m_PageList.GetCount();
}

///////////////////////////////////////////////////
// get a rect by the index of a child
void CBSWndContainer::CalcPageRect(LPRECT lpRect,int nIndex,int nPageCount)
{
	if((nPageCount<=0)||(nIndex>=nPageCount))
	{
		lpRect->left=lpRect->right=lpRect->top=lpRect->bottom=0;
		return;
	}
//get row count
	int nRow=0;
	while((nRow)*(nRow)<nPageCount) nRow++;

//get singledlg width and height
	int nWidth=(lpRect->right-lpRect->left)/nRow;
	int nHeight=(lpRect->bottom-lpRect->top)/nRow;

//get top-left point
	CPoint pt;
	pt.x=lpRect->left+nWidth*(nIndex%nRow);
	pt.y=lpRect->top+nHeight*(nIndex/nRow);

//set rect return back
	lpRect->left=pt.x;
	lpRect->top=pt.y;
	lpRect->right=lpRect->left+nWidth;
	lpRect->bottom=lpRect->top+nHeight;
}

///////////////////////////////////////////////////
// adjust a rect by defined proportion 
void CBSWndContainer::AdjustRect(LPRECT lpRect)
{
	int nWidth=lpRect->right-lpRect->left;
	int nHeight=lpRect->bottom-lpRect->top;
	CPoint pt((lpRect->left+lpRect->right)/2,(lpRect->top+lpRect->bottom)/2);

	int nTemp=nWidth*8/11;
	if(nTemp>nHeight)
	{
		nWidth=nHeight*11/8;
	}
	else if(nTemp<nHeight)
	{
		nHeight=nTemp;
	}
	lpRect->left=pt.x-nWidth/2;
	lpRect->right=pt.x+nWidth/2;
	lpRect->top=pt.y-nHeight/2;
	lpRect->bottom=pt.y+nHeight/2;
}

///////////////////////////////////////////////////
//To get display zone in proportion 
void CBSWndContainer::GetShowRect(LPRECT lpRect)
{
	if(m_nShowPortion<40) m_nShowPortion=40;
	if(m_nShowPortion>100) m_nShowPortion=100;

	int nWidth	= lpRect->right-lpRect->left;
	int nHeight	= lpRect->bottom-lpRect->top;

	int nNewWidth	= (int)(nWidth*m_nShowPortion/100);
	int nNewHeight	= (int)(nHeight*m_nShowPortion/100);

	int ndx	= ( nWidth-nNewWidth )/2;
	int ndy = ( nHeight-nNewHeight )/2;

	lpRect->left	= lpRect->left	+ ndx;
	lpRect->top		= lpRect->top	+ ndy;
	lpRect->right	= lpRect->left	+ nNewWidth;
	lpRect->bottom	= lpRect->top	+ nNewHeight;	
}

///////////////////////////////////////////////////
// draw the frame of active page
void CBSWndContainer::DrawActivePage(BOOL bFlag)
{
	if( !m_bMultiScreen || 
		!m_pActivePage	|| 
		m_PageList.GetCount()<2 
		) return;

	CRect rt;
	m_pActivePage->GetWindowRect(&rt);
	ScreenToClient(&rt);
	rt.InflateRect(1,1);

	if(bFlag)
	{
		CDC *pDC=GetDC();
		if(!pDC) return;

//		pDC->Draw3dRect(&rt,m_clrTopLeft, m_clrBottomRight);	
		pDC->Draw3dRect(&rt,RGB(255,0,0), RGB(255,0,0));

		ReleaseDC(pDC);
	}
	else
		InvalidateRect(&rt);
}


