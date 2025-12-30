#if !defined(AFX_SPLICEWND_H__91F95A47_80F6_4BA8_AAC5_37B4DC6F1501__INCLUDED_)
#define AFX_SPLICEWND_H__91F95A47_80F6_4BA8_AAC5_37B4DC6F1501__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MonitorWall.h : header file
//
#include <vector>

#define WM_BLOCK_SEL_CHANGE		WM_USER + 0x7000

/////////////////////////////////////////////////////////////////////////////
// CMonitorWallWnd window

class CMonitorWallWnd : public CWnd
{
public:
	struct Block
	{
		POINT	stuLeftTop;			// 左上角位置
		SIZE	stuBlockSize;		// 纵横向的TV数
		SIZE	stuTVSize;			// 每个TV纵横向的网格数
		BOOL	bActive;			// 是否当前选中项
		BOOL	bNew;				// 是否新增的
		const CMonitorWallWnd* pWall;	// 所属电视墙
		char	szName[128];		// 融合屏名称
		std::vector<int> vecOutputs;// 输出通道
		char	szCompositeID[128];		// 融合屏ID

		Block(const CMonitorWallWnd* pInfo) : pWall(pInfo)
		{
			ASSERT(pInfo);
			stuLeftTop.x = stuLeftTop.y = 0;
			stuBlockSize.cx = stuBlockSize.cy = 1;
			stuTVSize.cx = stuTVSize.cy = 1;
			bActive = bNew = FALSE;
			*szName = 0;
			*szCompositeID = 0;
		}

		void GetRect(CRect& rect) const
		{
			rect.left = stuLeftTop.x;
			rect.top = stuLeftTop.y;
			rect.right = rect.left + stuBlockSize.cx * stuTVSize.cx * pWall->GetGridSize().cx;
			rect.bottom = rect.top + stuBlockSize.cy * stuTVSize.cx * pWall->GetGridSize().cy;
		}
	};

// Construction
public:
	CMonitorWallWnd();

	enum
	{
		SPLICE_ROW_NUM = 20,		// 网格行数
		SPLICE_COLUMN_NUM = 20,		// 网格列数
		GRID_LINE_WIDTH = 1,		// 网格线宽度
	};

// Attributes
public:
	// 添加区块
	BOOL AddBlock(Block& block);
	// 删除区块, -1表示当前活动的区块
	void RemoveBlock(UINT nIndex = -1);
	// 清空区块
	void ClearBlock();
	
	UINT GetBlockCount() const { return m_Blocks.size(); }
	Block* GetBlock(UINT nIndex);
	const Block* GetBlock(UINT nIndex) const;
	Block* GetActiveBlock();
	const Block* GetActiveBlock() const;
	int GetActiveBlockIndex() const;
	void SetActiveBlock(UINT nIndex);
	void SetActiveBlock(Block* pBlock);

	BOOL IsBlockIDEnable(const Block* pBlock);
	BOOL IsBlockIDConflict();

	CSize GetGridSize() const { return m_GridSize; }
	
	LPCTSTR GetName() const { return m_strName; }
	void SetName(LPCTSTR lpszName) { m_strName = lpszName; }
	
	void SetGridLayout(const SIZE& size) { m_GridLayout = size; }
	SIZE GetGridLayout() const { return m_GridLayout; }

	POINT GridToPixel(const POINT& ptGrid);
	POINT PixelToGrid(const POINT& ptPixel);
	RECT GridRect(const Block* pBlock);

protected:
	Block* BlockFromPoint(const CPoint& point);
	BOOL BlockAutoPosition(Block& block);
	BOOL IsBlockRectValid(const CRect& rcBlock);

protected:
	typedef std::vector<Block> BlockVector;
	BlockVector m_Blocks;

	CSize	m_GridSize;		// 单个网格大小
	BOOL	m_bBlockMoving;	// 拖动block
	CPoint  m_ptStart;		// 拖动起点
	CRect	m_rcMovingStart;// 拖动block的起始位置 

	CString m_strName;		// 电视墙名称
	SIZE	m_GridLayout;	// 网格行列数

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMonitorWallWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMonitorWallWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMonitorWallWnd)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

typedef CMonitorWallWnd::Block	WMBlock;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLICEWND_H__91F95A47_80F6_4BA8_AAC5_37B4DC6F1501__INCLUDED_)
