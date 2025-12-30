// SpliceWnd.cpp : implementation file
//

#include "stdafx.h"
#include "DemoMonitorWall.h"
#include "MonitorWallWnd.h"
#include "memdc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define SPLICE_COLOR_BACKGROUND			RGB(150, 175, 234)	// 背景色
#define SPLICE_COLOR_GRID_LINE			RGB(96, 96, 96)		// 分割线
#define SPLICE_BORDER_COLOR_ACTIVE		RGB(0, 255, 0)		// 活动block边框颜色
#define SPLICE_BORDER_COLOR_INACTIVE	RGB(255, 255, 255)	// 非活动block边框颜色
#define SPLICE_COLOR_BLOCK_NORMAL		RGB(5, 162, 240)	// 普通block颜色
#define SPLICE_COLOR_BLOCK_NEW			RGB(192, 64, 192)	// 新增block颜色

/////////////////////////////////////////////////////////////////////////////
// CMonitorWallWnd

CMonitorWallWnd::CMonitorWallWnd()
 : m_GridSize(1, 1)
 , m_ptStart(0, 0)
 , m_bBlockMoving(FALSE) 
 , m_strName("")
 , m_rcMovingStart(0,0,0,0)
{
	m_GridLayout.cx = 0;
	m_GridLayout.cy = 0;
}

CMonitorWallWnd::~CMonitorWallWnd()
{
}


BEGIN_MESSAGE_MAP(CMonitorWallWnd, CWnd)
	//{{AFX_MSG_MAP(CMonitorWallWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMonitorWallWnd message handlers

void CMonitorWallWnd::OnPaint() 
{
	CPaintDC dcPaint(this); // device context for painting
	
	CRect rcBlock;
	CRect rcClient;
	GetClientRect(rcClient);
	CMemDC dc(&dcPaint, rcClient);
	POINT offset = { -rcClient.left, -rcClient.top };
	rcClient.OffsetRect(offset);

	// 背景色
	dc->FillSolidRect(&rcClient, SPLICE_COLOR_BACKGROUND);

	// block颜色, 名称	
	int i = 0;
	CString strText;
	for (i = 0; i < m_Blocks.size(); ++i)
	{
		const Block& block = m_Blocks[i];
		strText = block.szCompositeID;
		block.GetRect(rcBlock);
		dc->FillSolidRect(&rcBlock, block.bNew ? SPLICE_COLOR_BLOCK_NEW : SPLICE_COLOR_BLOCK_NORMAL);
		dc->DrawText(strText, rcBlock, /*DT_WORDBREAK*/DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	// 分割线
	int x = 0;
	int y = 0;

	CPen pen(PS_SOLID, 1, SPLICE_COLOR_GRID_LINE);
	CPen* pOldPen = dc->SelectObject(&pen);

	for (i = 1; i < SPLICE_COLUMN_NUM; ++i)
	{
		x = i * m_GridSize.cx;
		dc->MoveTo(x, 0);
		dc->LineTo(x, rcClient.bottom);
	}
	for (i = 1; i < SPLICE_ROW_NUM; ++i)
	{
		y = i * m_GridSize.cy;
		dc->MoveTo(0, y);
		dc->LineTo(rcClient.right, y);
	}
	dc->SelectObject(pOldPen);
	pen.DeleteObject();

	// block边框, 先画普通的, 再画活动的	
	CPen penBorderInactive;
	penBorderInactive.CreatePen(PS_SOLID, 1, SPLICE_BORDER_COLOR_INACTIVE);
	pOldPen = dc->SelectObject(&penBorderInactive);
	BlockVector::const_iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{
		if (!it->bActive)
		{
			it->GetRect(rcBlock);
			dc->MoveTo(rcBlock.TopLeft());
			dc->LineTo(rcBlock.right, rcBlock.top);
			dc->LineTo(rcBlock.BottomRight());
			dc->LineTo(rcBlock.left, rcBlock.bottom);
			dc->LineTo(rcBlock.TopLeft());
		}
	}
	dc->SelectObject(pOldPen);
	penBorderInactive.DeleteObject();
	
	Block* pBlock = GetActiveBlock();
	if (pBlock)
	{	
		CPen penBorderActive;
		penBorderActive.CreatePen(PS_SOLID, 1, SPLICE_BORDER_COLOR_ACTIVE);
		pOldPen = dc->SelectObject(&penBorderActive);
		pBlock->GetRect(rcBlock);
		dc->MoveTo(rcBlock.TopLeft());
		dc->LineTo(rcBlock.right, rcBlock.top);
		dc->LineTo(rcBlock.BottomRight());
		dc->LineTo(rcBlock.left, rcBlock.bottom);
		dc->LineTo(rcBlock.TopLeft());		
		dc->SelectObject(pOldPen);
		penBorderActive.DeleteObject();
	}
}

void CMonitorWallWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	if (!IsIconic())
	{
		m_GridSize.cx = cx / SPLICE_COLUMN_NUM;
		m_GridSize.cy = cy / SPLICE_ROW_NUM;
	}		
}

BOOL CMonitorWallWnd::AddBlock( Block& block )
{
	CRect rcBlock, rcInter, rcTemp;
	block.GetRect(rcBlock);

	// 如果位置冲突, 重新确定位置
	BOOL bConflict = FALSE;
	BlockVector::iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{
		it->GetRect(rcTemp);
		if (rcInter.IntersectRect(rcBlock, rcTemp))
		{
			bConflict = TRUE;
			break;
		}
	}
	if (!bConflict || (bConflict && BlockAutoPosition(block)))
	{
		m_Blocks.push_back(block);
		SetActiveBlock(&m_Blocks.back());
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

WMBlock* CMonitorWallWnd::GetBlock( UINT nIndex )
{
	WMBlock* pBlock = NULL;
	if (nIndex < m_Blocks.size())
	{
		pBlock = &m_Blocks[nIndex];
	}
	return pBlock;
}

const WMBlock* CMonitorWallWnd::GetBlock( UINT nIndex ) const
{	
	const WMBlock* pBlock = NULL;
	if (nIndex < m_Blocks.size())
	{
		pBlock = &m_Blocks[nIndex];
	}
	return pBlock;
}

void CMonitorWallWnd::RemoveBlock( UINT nIndex /*= -1*/ )
{	
	ASSERT(nIndex == -1 || nIndex < m_Blocks.size());
	if (nIndex == -1)
	{
		BlockVector::iterator it = m_Blocks.begin();
		for (; it != m_Blocks.end(); ++it)
		{
			if (it->bActive)
			{
				m_Blocks.erase(it);

				if (m_Blocks.size() > 0)
				{
					SetActiveBlock(&m_Blocks.front());
				}
				else
				{
					Invalidate();
				}
				return;
			}
		}
	}
	else
	{
		BlockVector::iterator it = m_Blocks.begin();
		for (UINT i = 0; it != m_Blocks.end(); ++it, ++i)
		{
			if (i == nIndex)
			{
				m_Blocks.erase(it);
				Invalidate();
				return;
			}
		}
	}
}

void CMonitorWallWnd::ClearBlock()
{
	m_Blocks.clear();
	Invalidate();
}

WMBlock* CMonitorWallWnd::GetActiveBlock()
{
	WMBlock* pBlock = NULL;
	BlockVector::iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{
		if (it->bActive)
		{
			pBlock = &(*it);
			break;
		}
	}
	return pBlock;
}

const WMBlock* CMonitorWallWnd::GetActiveBlock() const
{
	WMBlock* pBlock = NULL;
	BlockVector::const_iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{
		if (it->bActive)
		{
			pBlock = const_cast<WMBlock*>(&(*it));
			break;
		}
	}
	return pBlock;
}

int CMonitorWallWnd::GetActiveBlockIndex() const
{
	int nIndex = -1;
	for (int i = 0; i < m_Blocks.size(); ++i)
	{
		if (m_Blocks[i].bActive)
		{
			nIndex = i;
			break;
		}
	}
	return nIndex;
}

void CMonitorWallWnd::SetActiveBlock( UINT nIndex )
{
	WMBlock* pBlock = GetBlock(nIndex);
	if (pBlock && !pBlock->bActive)
	{
		SetActiveBlock(pBlock);
	}
}

void CMonitorWallWnd::SetActiveBlock( WMBlock* pBlock )
{
	if (!pBlock->bActive)
	{
		WMBlock* pActiveBlock = GetActiveBlock();
		if (pActiveBlock)
		{
			pActiveBlock->bActive = FALSE;
		}
		pBlock->bActive = TRUE;
		Invalidate();
		GetParent()->SendMessage(WM_BLOCK_SEL_CHANGE, 0, GetActiveBlockIndex());
	}
}

void CMonitorWallWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	WMBlock* pBlock = BlockFromPoint(point);
	if (pBlock)
	{
		if (!pBlock->bActive)
		{
			SetActiveBlock(pBlock);
		}
		m_bBlockMoving = TRUE;
		m_ptStart = point;
		pBlock->GetRect(m_rcMovingStart);
		SetCapture();
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CMonitorWallWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bBlockMoving)
	{
		m_bBlockMoving = FALSE;
	}
	if (GetCapture() == this)
	{
		ReleaseCapture();
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CMonitorWallWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bBlockMoving)
	{
		WMBlock* pBlock = GetActiveBlock();
		if (pBlock)
		{
			// 目标位置
			POINT ptOffset = { point.x - m_ptStart.x, point.y - m_ptStart.y };
			CRect rcNew(m_rcMovingStart);
			rcNew.OffsetRect(ptOffset.x / m_GridSize.cx * m_GridSize.cx, ptOffset.y / m_GridSize.cy * m_GridSize.cy);
			if (rcNew.left < 0)
			{
				rcNew.OffsetRect(-rcNew.left, 0);
			}
			if (rcNew.top < 0)
			{
				rcNew.OffsetRect(0, -rcNew.top);
			}
			if (rcNew.right > m_GridSize.cx * SPLICE_COLUMN_NUM)
			{
				rcNew.OffsetRect(m_GridSize.cx * SPLICE_COLUMN_NUM - rcNew.right, 0);
			}
			if (rcNew.bottom > m_GridSize.cy * SPLICE_ROW_NUM)
			{
				rcNew.OffsetRect(0, m_GridSize.cy * SPLICE_ROW_NUM - rcNew.bottom);
			}

			// 判断是否可以移动在新位置
			BOOL bCanMove = TRUE;
			CRect rcInter, rcBlock;
			BlockVector::iterator it = m_Blocks.begin();
			for (; it != m_Blocks.end(); ++it)
			{				
				it->GetRect(rcBlock);
				if (&(*it) != pBlock && rcInter.IntersectRect(rcNew, rcBlock))
				{
					bCanMove = FALSE;
					break;
				}
			}
			if (bCanMove)
			{
				pBlock->stuLeftTop = rcNew.TopLeft();
				Invalidate();
			}
		}
	}
	
	CWnd::OnMouseMove(nFlags, point);
}

WMBlock* CMonitorWallWnd::BlockFromPoint( const CPoint& point )
{
	CRect rcBlock;
	POINT pt = { point.x, point.y };
	BlockVector::iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{
		it->GetRect(rcBlock);
		if (PtInRect(rcBlock, pt))
		{
			return &(*it);
		}
	}
	return NULL;
}

BOOL CMonitorWallWnd::BlockAutoPosition( Block& block )
{
	CRect rcBlock, rcTemp, rcInter;
	block.GetRect(rcBlock);

	for (int j = 0; j <= SPLICE_ROW_NUM - block.stuTVSize.cy * block.stuBlockSize.cy; ++j)
	{
		for (int i = 0; i <= SPLICE_COLUMN_NUM - block.stuTVSize.cx * block.stuBlockSize.cx; ++i)
		{
			POINT ptLeftTop = { m_GridSize.cx * i, m_GridSize.cy * j };
			POINT ptOffset = { ptLeftTop.x - rcBlock.left, ptLeftTop.y - rcBlock.top };
			CRect rcNew(rcBlock);
			rcNew.OffsetRect(ptOffset);
			
			if (IsBlockRectValid(rcNew))
			{
				block.stuLeftTop = rcNew.TopLeft();
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CMonitorWallWnd::IsBlockRectValid( const CRect& rcBlock )
{
	CRect rcTemp, rcInter;
	BlockVector::const_iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{
		it->GetRect(rcTemp);
		if (rcInter.IntersectRect(rcBlock, rcTemp))
		{		
			return FALSE;
		}
	}
	return TRUE;
}

POINT CMonitorWallWnd::GridToPixel( const POINT& ptGrid )
{
	POINT ptPixel = {0};
	ptPixel.x = ptGrid.x * m_GridSize.cx;
	ptPixel.y = ptGrid.y * m_GridSize.cy;
	return ptPixel;
}

POINT CMonitorWallWnd::PixelToGrid( const POINT& ptPixel )
{
	POINT ptGrid = {0};
	ptGrid.x = ptPixel.x / m_GridSize.cx;
	ptGrid.y = ptPixel.y / m_GridSize.cy;
	return ptGrid;
}

RECT CMonitorWallWnd::GridRect( const WMBlock* pBlock )
{
	RECT rcGrid = {0};
	POINT ptTopLeft = PixelToGrid(pBlock->stuLeftTop);
	rcGrid.left = ptTopLeft.x;
	rcGrid.top = ptTopLeft.y;
	rcGrid.right = rcGrid.left + pBlock->stuBlockSize.cx * pBlock->stuTVSize.cx - 1;
	rcGrid.bottom = rcGrid.top + pBlock->stuBlockSize.cy * pBlock->stuTVSize.cy - 1;
	return rcGrid;
}

BOOL CMonitorWallWnd::IsBlockIDEnable( const Block* pBlock )
{	
	BlockVector::const_iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{		
		if (&(*it) == pBlock)
			continue;
		if (strcmp(it->szCompositeID, pBlock->szCompositeID) == 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CMonitorWallWnd::IsBlockIDConflict()
{
	BlockVector::const_iterator it = m_Blocks.begin();
	for (; it != m_Blocks.end(); ++it)
	{		
		if (!IsBlockIDEnable(&(*it)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CMonitorWallWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnLButtonDblClk(nFlags, point);
}
