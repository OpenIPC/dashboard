/*********************************************************************************
 *	类名 :	CBSWndContainer
 *
 *	功能 :	在同一个窗口同时播放多路视频时,完成多窗口界面的一些控制控制。
 *			
 *			[单/多窗口切换],[全屏],[边框],[窗口按比例缩放],[自动调整宽高比例]
 *		
 *	作者 :	OLinS
 *
 *	时间 :	2003.1.15
 *												
 *********************************************************************************
 */


#if !defined(AFX_BSWNDCONTAINER_H__73CB8E46_8ED9_4C36_BA91_29D5F5BB05DE__INCLUDED_)
#define AFX_BSWNDCONTAINER_H__73CB8E46_8ED9_4C36_BA91_29D5F5BB05DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BSWndContainer.h : header file
//
//视频窗口之间的间隔
#define WINDOW_SPACE  1

/////////////////////////////////////////////////////////////////////////////
// CBSWndContainer window
#include <afxtempl.h>

class CBSWndContainer : public CWnd
{
// Construction
public:
	CBSWndContainer();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBSWndContainer)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBSWndContainer();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBSWndContainer)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////////////////////////////////
// public interface member
public:

	///////////////////////////////////////////////////
	// 创建对象(Container)
	BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	///////////////////////////////////////////////////
	// 添加窗口
	BOOL AddPage(CWnd *pWnd, BOOL bRepaint = TRUE);

	///////////////////////////////////////////////////
	// 删除窗口(只是从链表中删除,实际对象的必须外部删除) 
	CWnd *DelPage(CWnd *pWnd);

	///////////////////////////////////////////////////
	// 删除激活窗口
	CWnd *DelPage();

	///////////////////////////////////////////////////
	// 设置激活窗口
	void SetActivePage(CWnd *pWnd, BOOL bRepaint = TRUE);

	///////////////////////////////////////////////////
	// 得到激活窗口
	CWnd *GetActivePage();

	///////////////////////////////////////////////////
	// 得到后一个窗口
	CWnd *GetNextPage(CWnd *pWnd);

	///////////////////////////////////////////////////
	// 得到前一个窗口
	CWnd *GetPrevPage(CWnd *pWnd);

	///////////////////////////////////////////////////
	//	得到指定序号的窗口
	CWnd *GetPage(int nIndex);

	///////////////////////////////////////////////////
	//	得到窗口总数
	int GetCount() const;

	///////////////////////////////////////////////////
	//	得到最后窗口
	CWnd *GetTailPage();

	///////////////////////////////////////////////////
	// 更新窗口
	virtual void UpdateWnd();

	///////////////////////////////////////////////////
	//	全屏显示
	void SetFullScreen(BOOL bFlag);
	BOOL GetFullScreen();

	///////////////////////////////////////////////////
	//	多屏显示
	void SetMultiScreen(BOOL bFlag);
	BOOL GetMultiScreen();

	//////////////////////////////////////////////////
	//	自动调节尺寸
	void SetAutoAdjustPos(BOOL bFlag);
	BOOL GetAutoAdjustPos();

	//////////////////////////////////////////////////
	//	激活窗口边框
	void SetDrawActivePage(BOOL bFlag,COLORREF clrTopLeft=RGB(255, 0, 0), COLORREF clrBottomRight=RGB(255, 0, 0));
	BOOL GetDrawActivePage();

	//////////////////////////////////////////////////
	//	显示百分比
	//	40 <= nPortion <=100
	void SetShowPortion(int nPortion=100);
	int  GetShowPortion();

//////////////////////////////////////////////////////////////////////////////////////////////////////
// protected member for derived class
protected:
	
	///////////////////////////////////////////////////
	// 之窗口链表
	CList<CWnd *,CWnd *> m_PageList;

	///////////////////////////////////////////////////
	// 激活窗口指针
	CWnd *m_pActivePage;

	///////////////////////////////////////////////////
	// 全屏标记
	BOOL m_bFullScreen;

	///////////////////////////////////////////////////
	// 多屏标记
	BOOL m_bMultiScreen;	

	///////////////////////////////////////////////////
	// 自动调整标记
	BOOL m_bAutoAdjustPos;

	///////////////////////////////////////////////////
	// 边框标记
	BOOL m_bDrawActive;

	///////////////////////////////////////////////////
	// 显示区域百分比(40-100)
	int	m_nShowPortion; 

	WINDOWPLACEMENT m_OldWndPlacement; //用来保存原窗口位置 
	CRect m_FullScreenRect; //表示全屏显示时的窗口位置
////////////////////////////////////////////////////////////////////////////////////////////////////////
// private member for inter user
private:

	///////////////////////////////////////////////////
	//	更新链表,删除其中的无效节点,返回节点个数
	int UpdateList();

	///////////////////////////////////////////////////
	//	根据子窗口的序号 和 显示范围 ,得到窗口的位置
	virtual void CalcPageRect(LPRECT lpRect,int nIndex,int nPageCount);

	///////////////////////////////////////////////////
	//	根据预定义的比例得到显示区域(11/8)
	void AdjustRect(LPRECT lpRect);

	///////////////////////////////////////////////////
	//	按比例得到显示区域
	void GetShowRect(LPRECT lpRect);

	///////////////////////////////////////////////////
	//	画激活窗口边框
	void DrawActivePage(BOOL bFlag);

	///////////////////////////////////////////////////
	//	窗口边框颜色
	COLORREF m_clrTopLeft;
	COLORREF m_clrBottomRight;

	///////////////////////////////////////////////////
	//	全屏时保存原窗口信息,用来恢复窗口
	WINDOWPLACEMENT _temppl;		//window's placement
	CWnd *			_tempparent;	//window's parent
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BSWNDCONTAINER_H__73CB8E46_8ED9_4C36_BA91_29D5F5BB05DE__INCLUDED_)
