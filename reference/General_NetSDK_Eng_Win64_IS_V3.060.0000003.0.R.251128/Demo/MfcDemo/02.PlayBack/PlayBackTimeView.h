#pragma once

#include <list>
class CColorStatic : public CStatic
{
	DECLARE_DYNAMIC(CColorStatic)

public:
	CColorStatic();
	virtual ~CColorStatic();

	void SetTextColor(COLORREF crText);
	void SetFontSize(int nSize=9.5f);
	void SetBackColor(COLORREF crBackColor);
	void SetTransparent(bool bTran);

protected:
	DECLARE_MESSAGE_MAP()

	COLORREF m_crText;
	COLORREF m_crBackColor;
	CBrush	 m_brush;
	LOGFONT	 m_lf;
	CFont	 m_font;

	bool	 m_bTran;
public:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	
};

// CPlayBackTimeView

class CPlayBackTimeView : public CView
{
	DECLARE_DYNCREATE(CPlayBackTimeView)

public:
	struct VideoTime
	{
		COleDateTime startTime;
		COleDateTime endTime;
	};

	struct VideoRect
	{
		int hour;
		int startPoint;
		int endPoint;
	};
	typedef std::list<VideoTime> VideoTimeList;
	typedef VideoTimeList::iterator videoTimelist_iter;

	typedef std::list<VideoRect> VideoRectList;
	typedef VideoRectList::iterator videoRectList_iter;

	class NotifyMovePosition
	{
	public:
		virtual void invoke(const COleDateTime* pTime) = 0 ;
	};

	CPlayBackTimeView(NotifyMovePosition* pNotify = NULL);           // 动态创建所使用的受保护的构造函数
	virtual ~CPlayBackTimeView();



public:
	virtual void OnDraw(CDC* pDC);      // 重写以绘制该视图
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
	
private:


	NotifyMovePosition* m_notifyMovePosion;



	class DrawControl;
	DrawControl* m_pDrawControl;
	
	
	VideoRectList m_topRectList;
	VideoRectList m_bottomRectList;
	VideoRectList m_topRectMergeList;
	VideoRectList m_bottomRectMergeList;

	VideoTimeList m_validTimeList;
	
	COleDateTime	m_startTime; 
	COleDateTime	m_currentPlayBackTime;

	
	bool m_isInTopTracker ;
	bool m_isInBottomTracker ;
	bool m_isMouseMove;
    bool m_isLBtnDown;

	bool m_isInit;
	bool m_isMove;
	
	

public:

	// Init Time Bar 
	bool InitTimeBar(const COleDateTime& startTime, const VideoTimeList& validTimeList);

	// clear the time bar 
	bool UninitTimeBar();

	// seek time bar tracker position
	void SeekTrickerByTime(const COleDateTime& time);

	COleDateTime GetCurrentTime() {return m_currentPlayBackTime;}

	void SetShowTimeCtl(CStatic* pStatic) {m_pShowPlayBackTime = pStatic;}
	

private:
	void StoreDrawRecordPoint(const VideoTime& vt, double onePixelsSecondsOnTop, double onePixelsSecondsOnBottom );
	// draw time scope of record file 
	void DrawRecordScope();

	// Merge Rect list
	void MergeRectList();

	void UpdateTime();

	void TrickerMove(UINT nFlags, CPoint point);

	bool IsCurrentTimeExist(const COleDateTime& time);

	CString GetNowPlayBackTime();

	COleDateTime ChangePointToTime();

	CStatic* m_pShowPlayBackTime;
	

private:
	
	class DrawControl{
	public:
		static DrawControl* getInstance(CView* pView);
	private:
		CView *m_parentView;
		DrawControl(CView* pView);
		~DrawControl();
		void operator = (const DrawControl&);

	public:
		void prepareDrawData();
		void draw();
		void resetControl();
		void drawMinuteRect(int beginX, int endX);
		void drawHourRect(int beginX, int endX);
		void drawTopTricker( CPoint const& point,bool& isDraw);
		int	drawBottomTricker( CPoint const& point,bool& isDraw);
		void drawCurrentTricker(CPoint& currentPoint, CRect& currentRect,CPoint const& point);
		void fillTopBar();
		void fillBottomBar();
		
		
		double GetBarWidth() { return m_barWidth;}
		double GetFiveMinuteWidth() { return m_fiveMinuteInterval;}
		double GetHourWidth() {return m_hourInterval;}
		

		void drawCurrentTime(COleDateTime& time,float pointX,bool isErase);
	private:
		void drawTimeBar( );
		void drawTracker( );
		void drawTimeLine();

		void drawStartPlayBackTime();
		
		

	public:
		CRect m_currentTopTrickerRect;
		CRect m_currentBottomTrickerRect;
		CPoint m_currentTopTrickerPoint; 
		CPoint m_currentBottomTrickerPoint;

	

		CPoint m_oldTopTrickerPoint;
		CPoint m_oldBottomTrickerPoint;

		CRect m_topActiveValidRect;
		CRect m_bottomActiveValidRect;

	private:
		
		// top bar x coordinate
		int m_topBarX ;
		// top bar Y coordinate
		int m_topBarY;

		// bottom bar X coordinate
		int m_bottomBarX;
		// bottom bar Y coordinate
		int m_bottomBarY;

		// hour Interval
		double m_hourInterval;
		// minute Interval
		double m_fiveMinuteInterval;

		// bar width
		double m_barWidth;



		ULONG_PTR m_gdiplusToken;

	public:
		double m_timeStrPointX;
		double m_timeStrPointY;

	};

public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};





