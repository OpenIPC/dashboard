#include "stdafx.h"
#include "PlayBackTimeView.h"

#include <cmath>
#include <GdiPlus.h>

#pragma comment(lib,"gdiplus")

namespace TimeContorlConstant
{
	const int barHeight = 10;
	const float barBorderThickness = 1;

	const int leftPadding = 10;
	const int topPadding = 22;
	const int rightPadding = 10;
	const int bottomPadding = 18;

	const int barLineHeight = 5;
	const int hourCount = 24;
	const int fiveMinute = 5;
	const int minuteCount = 12;

	const int oneHourSecondsNumber = 3600;
	const int triangleX = 6;
	const int triangleY = 8;

	const float fontSize = 10.0f; // time font
	const WCHAR fontName[] = L"Arial";
	const int fontMarginLeftA = 5;
	const int fontMarginLeftB = 8;
	const int fontMarginLeftC = 10;
	const int fontMarginTop = 15;

	const int oneMinuteSecondsNumber = 60;
	const int lastMinuteOrSecond = 59;
	const int triangleMarinBarBottom = 5;

	const int lableHeight = 18;
	const int lableWidth = 80;

	const int lableMarginLeft = 20;
	const int lableMarginTop = 5;

	const int timeStrHeight = 16;
	const int timeStrWidth = 54;
	const int timeStrMarginLeft = 5;
	const int timeStrMarginTop = 3;

};
using namespace TimeContorlConstant;
using namespace std;
using namespace Gdiplus;

// CPlayBackTimeView

IMPLEMENT_DYNCREATE(CPlayBackTimeView, CView)

CPlayBackTimeView::CPlayBackTimeView(NotifyMovePosition* pNotify):
m_notifyMovePosion(pNotify)
{
	m_pDrawControl = DrawControl::getInstance(this);
	m_isInTopTracker = false;
	m_isInBottomTracker = false;
	m_isMouseMove = false;
    m_isLBtnDown = false;

	m_isInit = false;
	m_isMove = false;

	m_pShowPlayBackTime = NULL;
}

CPlayBackTimeView::~CPlayBackTimeView()
{
}

BEGIN_MESSAGE_MAP(CPlayBackTimeView, CView)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CPlayBackTimeView 绘图

void CPlayBackTimeView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: 在此添加绘制代码


// 	CRect rc;
// 	GetClientRect(&rc);
// 	pDC->FillSolidRect(rc,RGB(0,0,0));
	
	m_pDrawControl->draw();
//	m_pDrawControl->drawCurrentTime(m_currentPlayBackTime,m_pDrawControl->m_currentTopTrickerPoint.x,false);
	
	if (m_isInit)
	{
		DrawRecordScope();

	}

	//	DrawRecordScope(g);
	//	m_isInit = false;
	
}


// CPlayBackTimeView 诊断

#ifdef _DEBUG
void CPlayBackTimeView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CPlayBackTimeView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


int CPlayBackTimeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_pDrawControl->prepareDrawData();
	
	RECT rect1 ;
	GetClientRect(&rect1);

#if 0
	RECT rect2 = {(rect1.right-rect1.left-lableWidth)/2,
		lableMarginTop ,
		(rect1.right-rect1.left-lableWidth)/2+lableWidth,
		lableMarginTop + lableHeight};
	m_playBackTimeControl.Create(_T("00:00:00"),WS_CHILD| WS_VISIBLE | WS_TABSTOP |SS_CENTER ,
		rect2, this);

	m_playBackTimeControl.SetTransparent(true);
	m_playBackTimeControl.SetTextColor(RGB(0,255,0));
	m_playBackTimeControl.SetBackColor(RGB(0,0,0));
	m_playBackTimeControl.SetFontSize();
#endif
	
	return 0;
}

void CPlayBackTimeView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_isLBtnDown )
    {
        m_isMouseMove = true;
        TrickerMove(nFlags,point);	
    }
	
	CView::OnMouseMove(nFlags, point);
}

void CPlayBackTimeView::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_isLBtnDown = true;	
	if (m_pDrawControl->m_currentTopTrickerRect.PtInRect(point))
	{
		m_isInTopTracker = true;
	}

	else if (m_pDrawControl->m_currentBottomTrickerRect.PtInRect(point))
	{
		m_isInBottomTracker = true;
	}

	CView::OnLButtonDown(nFlags, point);
}

void CPlayBackTimeView::OnLButtonUp(UINT nFlags, CPoint point)
{
     m_isLBtnDown = false;
	 m_isMouseMove = false;
     if (m_pDrawControl->m_topActiveValidRect.PtInRect(point))
	{
		m_isInTopTracker = true;
	}
	else if (m_pDrawControl->m_bottomActiveValidRect.PtInRect(point))
	{
		m_isInBottomTracker = true;
	}
	
	TrickerMove(nFlags,point);


	
	m_isInTopTracker = false;
	m_isInBottomTracker = false;

	CView::OnLButtonUp(nFlags, point);
}


bool CPlayBackTimeView::InitTimeBar(const COleDateTime& startTime,const VideoTimeList& validTimeList)
{
	UninitTimeBar();
	if (validTimeList.empty())
	{
		return false;
	}
	m_startTime = startTime;
	m_validTimeList = validTimeList;
	m_currentPlayBackTime = m_validTimeList.begin()->startTime;

	// calculate the pixel
	double onePixelsSecondsOnTop = fiveMinute * oneMinuteSecondsNumber / (double)m_pDrawControl->GetFiveMinuteWidth(); // 小时进度条一个像素点代表多少秒
	double onePixelsSecondsOnBottom = oneHourSecondsNumber / (double)m_pDrawControl->GetHourWidth(); // 天进度条一个像素点代表多少秒
	
	
	COleDateTime startTimeValid = m_startTime;
	COleDateTime endTimeValid = COleDateTime(m_startTime.GetYear(),m_startTime.GetMonth(),m_startTime.GetDay(),23,59,59);

	videoTimelist_iter it = m_validTimeList.begin(); 
	for (; it != m_validTimeList.end(); ++it)
	{
		
		if (it->startTime < startTimeValid)
		{
			VideoTime temp = *it;
			temp.startTime = startTimeValid;
			StoreDrawRecordPoint(temp,onePixelsSecondsOnTop,onePixelsSecondsOnBottom);
			continue;
		}

		if (it->endTime > endTimeValid)
		{
			VideoTime temp = *it;
			temp.endTime = endTimeValid;

			StoreDrawRecordPoint(temp,onePixelsSecondsOnTop,onePixelsSecondsOnBottom);
			continue;
		}

		// 一小时内的录像
		if(it->startTime.GetHour() == it->endTime.GetHour())
		{
			StoreDrawRecordPoint(*it, onePixelsSecondsOnTop, onePixelsSecondsOnBottom);
			continue;
		}
				
		//超过一小时的录像 
		for(int k = it->startTime.GetHour(); k <= it->endTime.GetHour(); k++)
		{
			VideoTime vt ;
			if(k == it->startTime.GetHour())
			{
				vt.startTime = it->startTime;
				vt.endTime = COleDateTime(vt.startTime.GetYear(), vt.startTime.GetMonth(), vt.startTime.GetDay(),
					vt.startTime.GetHour(), lastMinuteOrSecond, lastMinuteOrSecond);

			}
			else
			{
				if (k == it->endTime.GetHour())
				{
					vt.startTime = COleDateTime(it->startTime.GetYear(), it->startTime.GetMonth(), it->startTime.GetDay(), k, 0, 0);
					vt.endTime = it->endTime;
				}
				else
				{
					vt.startTime = COleDateTime(vt.startTime.GetYear(), vt.startTime.GetMonth(), vt.startTime.GetDay(), k, 0, 0);
					vt.endTime = COleDateTime(vt.startTime.GetYear(), vt.startTime.GetMonth(), vt.startTime.GetDay(), k, lastMinuteOrSecond, lastMinuteOrSecond);
				}
			}

			StoreDrawRecordPoint(vt, onePixelsSecondsOnTop, onePixelsSecondsOnBottom);
		}
	}

	// 合并录像文件像素点
	MergeRectList();
	// 绘图
	DrawRecordScope();

	m_isInit = true;
	
	m_pDrawControl->drawCurrentTricker(
		m_pDrawControl->m_oldTopTrickerPoint,
		m_pDrawControl->m_currentTopTrickerRect,
		m_pDrawControl->m_currentTopTrickerPoint
		);
	m_pDrawControl->drawCurrentTricker(
		m_pDrawControl->m_oldBottomTrickerPoint,
		m_pDrawControl->m_currentBottomTrickerRect,
		m_pDrawControl->m_currentBottomTrickerPoint
		);
	
	COleDateTime time = ChangePointToTime();
	m_currentPlayBackTime = time;

    m_pShowPlayBackTime->SetWindowText("00:00::00");
	//UpdateTime();
//	GetNowPlayBackTime();
	return true;
}

bool CPlayBackTimeView::UninitTimeBar()
{
	m_topRectList.clear();
	m_bottomRectList.clear();
	m_topRectMergeList.clear();
	m_bottomRectMergeList.clear();

	m_validTimeList.clear();
	m_pDrawControl->resetControl();

	m_isInit = false;

	COleDateTime time = ChangePointToTime();
	m_currentPlayBackTime = time;
	UpdateTime();

//	m_currentPlayBackTime = m_validTimeList.begin()->startTime;
//	GetNowPlayBackTime();
//	m_pDrawControl->drawCurrentTime(m_currentPlayBackTime,m_pDrawControl->m_currentTopTrickerPoint.x,false);
	return true;
}

void CPlayBackTimeView::SeekTrickerByTime(const COleDateTime& time)
{
	if(m_isMouseMove)
	{
		return;
	}
	if(time.GetDay() < m_startTime.GetDay())
	{
		return;
	}
	
	if (time == m_currentPlayBackTime)
	{
		return;
	}

	int nCurrHour = m_currentPlayBackTime.GetHour();
	int nNewHour = time.GetHour();
	m_currentPlayBackTime = time + COleDateTimeSpan(1);
	int hour = m_currentPlayBackTime.GetHour();
	if (nCurrHour != nNewHour)
	{
		m_pDrawControl->fillTopBar();
		videoRectList_iter it1 = m_topRectMergeList.begin();
		for (; it1 != m_topRectMergeList.end(); ++it1)
		{
			if (hour == it1->hour)
				m_pDrawControl->drawMinuteRect(it1->startPoint,it1->endPoint);

		}
	}
			
	m_currentPlayBackTime = time + COleDateTimeSpan(1);

	CString strCurrentTime;
	strCurrentTime.Format("%02d:%02d:%02d", time.GetHour(), time.GetMinute(), time.GetSecond());

	if (NULL == m_pShowPlayBackTime)
	{
		return;
	}
	m_pShowPlayBackTime->SetWindowText(strCurrentTime);

	CPoint topP;
	CPoint bottomP;
	double onePixelsSecondsOnTop = fiveMinute * oneMinuteSecondsNumber /  m_pDrawControl->GetFiveMinuteWidth();
	double onePixelsSecondsOnBottom = oneHourSecondsNumber / m_pDrawControl->GetHourWidth();
	double topx = (time.GetMinute() * oneMinuteSecondsNumber + time.GetSecond()) / onePixelsSecondsOnTop;
	double bottomSecondx = (time.GetMinute() * oneMinuteSecondsNumber + time.GetSecond()) / onePixelsSecondsOnBottom;
	topP.x = (int)topx + leftPadding;
	topP.y = m_pDrawControl->m_oldTopTrickerPoint.y;
// 	if(topP != m_pDrawControl->m_oldTopTrickerPoint &&
// 		(topP.x >= m_pDrawControl->m_oldTopTrickerPoint.x || topP.x == leftPadding))
	{
		if (topP.x == m_pDrawControl->m_currentTopTrickerPoint.x)
		{
			return ;
		}
//		m_pDrawControl->drawCurrentTime(m_currentPlayBackTime,m_pDrawControl->m_currentTopTrickerPoint.x,true);
		m_pDrawControl->drawCurrentTricker(
			m_pDrawControl->m_currentTopTrickerPoint,
			m_pDrawControl->m_currentTopTrickerRect,
			topP);
		
//		m_pDrawControl->drawCurrentTime(m_currentPlayBackTime,m_pDrawControl->m_currentTopTrickerPoint.x,false);


		m_pDrawControl->m_oldTopTrickerPoint = topP;
	}

	bottomP.x = leftPadding + (int)(time.GetHour() * m_pDrawControl->GetHourWidth() + bottomSecondx);
	bottomP.y = m_pDrawControl->m_oldBottomTrickerPoint.y;            
// 	if(bottomP != m_pDrawControl->m_oldBottomTrickerPoint && 
// 		bottomP.x >= m_pDrawControl->m_oldBottomTrickerPoint.x)
	{
		if (bottomP.x == m_pDrawControl->m_currentBottomTrickerPoint.x)
		{
			return;
		}
		m_pDrawControl->drawCurrentTricker(
			m_pDrawControl->m_currentBottomTrickerPoint,
			m_pDrawControl->m_currentBottomTrickerRect,
			bottomP);
		m_pDrawControl->m_oldBottomTrickerPoint = bottomP;
	}


}

void  CPlayBackTimeView::StoreDrawRecordPoint(const VideoTime& vt, double onePixelsSecondsOnTop,
											 double onePixelsSecondsOnBottom )
{
	COleDateTimeSpan timeInterval = vt.endTime - vt.startTime;
	double totalSeconds = timeInterval.GetTotalSeconds();

	double onBottomWidth = totalSeconds / onePixelsSecondsOnBottom; // (天进度)当前录像文件转换成对应的像素长度
	if (onBottomWidth < (int)barBorderThickness) 
	{
		onBottomWidth = (int)barBorderThickness;
	}
	double onTopWidth = totalSeconds / onePixelsSecondsOnTop; // (小时进度条)当前录像文件转换成对应的像素长度
	if (onTopWidth < (int)barBorderThickness)
	{
		onTopWidth = (int)barBorderThickness;
	}

	VideoRect topRect ;
	topRect.hour = vt.startTime.GetHour();
	if (vt.startTime.GetHour() == 0 || (vt.startTime.GetMinute() == 0
		&& vt.startTime.GetSecond() < onePixelsSecondsOnTop))
	{
		topRect.startPoint = vt.startTime.GetMinute() / fiveMinute * m_pDrawControl->GetFiveMinuteWidth() + 
			(int)((vt.startTime.GetMinute() % fiveMinute * oneMinuteSecondsNumber + 
			vt.startTime.GetSecond()) / onePixelsSecondsOnTop) +
			leftPadding + (int)barBorderThickness;

		topRect.endPoint = topRect.startPoint + (int)onTopWidth - (int)barBorderThickness;

	}
	else
	{
		topRect.startPoint = vt.startTime.GetMinute() / fiveMinute * m_pDrawControl->GetFiveMinuteWidth() +
			(int)((vt.startTime.GetMinute() % fiveMinute * oneMinuteSecondsNumber + 
			vt.startTime.GetSecond()) / onePixelsSecondsOnTop) + leftPadding;


		topRect.endPoint = topRect.startPoint + (int)onTopWidth;
	}
	m_topRectList.push_back(topRect);

	VideoRect bottomRect;
	bottomRect.hour = vt.startTime.GetHour();
	if (vt.startTime.GetHour() == 0)
	{
		bottomRect.startPoint = vt.startTime.GetHour() * m_pDrawControl->GetHourWidth() + leftPadding + 
			(int)(vt.startTime.GetMinute() * oneMinuteSecondsNumber / onePixelsSecondsOnBottom) +
			(int)barBorderThickness;

		bottomRect.endPoint = bottomRect.startPoint + (int)onBottomWidth - (int)barBorderThickness;
	}
	else
	{
		bottomRect.startPoint = vt.startTime.GetHour() * m_pDrawControl->GetHourWidth() + leftPadding +
			(int)(vt.startTime.GetMinute() * oneMinuteSecondsNumber / onePixelsSecondsOnBottom);

		bottomRect.endPoint = bottomRect.startPoint + (int)onBottomWidth;
	}
	m_bottomRectList.push_back(bottomRect);
}

// 合并 录像 节点信息
void CPlayBackTimeView::MergeRectList()
{

	if (m_topRectList.empty() || m_bottomRectList.empty())
	{
		return;
	}
	
	videoRectList_iter topit1 = m_topRectList.begin();
	videoRectList_iter topit2 = ++topit1;
	--topit1;
	VideoRect rectPointTemp = *topit1;

	m_pDrawControl->m_oldTopTrickerPoint = m_pDrawControl->m_currentTopTrickerPoint;
	m_pDrawControl->m_currentTopTrickerPoint.x = topit1->startPoint;


	for(; topit1 != m_topRectList.end() && topit2 != m_topRectList.end(); 
		++topit1,++topit2)
	{
		// 如果相邻的节点不在同一小时内，直接push到 mergelist中
		if(topit1->hour != topit2->hour)
		{
			m_topRectMergeList.push_back(rectPointTemp);
			rectPointTemp = *topit2;
		}
		else 
		{
			// 如果相邻的节点，首尾相连，合并节点信息 
			// topit1，topit2 步进1步
			if (topit1->endPoint == topit2->startPoint ||
				topit1->endPoint == topit2->startPoint-1
				)
			{
				rectPointTemp.endPoint = topit2->endPoint;
			}
			// 如果 相邻的节点，首尾不相连，直接push rectPointTemp到mergelist

			else 
			{
				m_topRectMergeList.push_back(rectPointTemp);
				rectPointTemp = *topit2;
			}
		}

	}
	m_topRectMergeList.push_back(rectPointTemp);


	videoRectList_iter bottomit1 = m_bottomRectList.begin();
	videoRectList_iter bottomit2 = ++bottomit1;
	--bottomit1;
	VideoRect rectPointTemp2 = *bottomit1;
	m_pDrawControl->m_oldBottomTrickerPoint = m_pDrawControl->m_currentBottomTrickerPoint;
	m_pDrawControl->m_currentBottomTrickerPoint.x = bottomit1->startPoint;

	for(; bottomit1 != m_bottomRectList.end() && bottomit2 != m_bottomRectList.end();
		++bottomit1,++bottomit2)
	{
		// 如果相邻的节点，首尾相连，合并节点信息 
		// topit1，topit2 步进1步
		if (bottomit1->endPoint == bottomit2->startPoint ||
			bottomit1->endPoint == bottomit2->startPoint-1
			)
		{
			rectPointTemp2.endPoint = bottomit2->endPoint;
		}
		// 如果 相邻的节点，首尾不相连，直接push rectPointTemp到mergelist
		else 
		{
			m_bottomRectMergeList.push_back(rectPointTemp2);
			rectPointTemp2 = *bottomit2;
		}
	}
	m_bottomRectMergeList.push_back(rectPointTemp2);

}

bool CPlayBackTimeView::IsCurrentTimeExist(const COleDateTime& time)
{
	bool result = false;
	videoTimelist_iter it = m_validTimeList.begin();
	for(; it != m_validTimeList.end(); ++it)
	{
		if (time >= it->startTime && time <= it->endTime)
		{
			result = true;
			break;
		}
	}

	return result;
}

CPlayBackTimeView::DrawControl::DrawControl(CView* pView):
m_parentView(pView)
{
	// top bar X coordinate
	m_topBarX = leftPadding;
	// top bar Y coordinate
	m_topBarY = 0;
	// bottom bar X coordinate
	m_bottomBarX = leftPadding;
	// bottom bar Y coordinate
	m_bottomBarY = 0;

	// hour Interval
	m_hourInterval = 0;
	// minute Interval
	m_fiveMinuteInterval = 0;

	m_barWidth = 0;
	m_timeStrPointX = 0;
	m_timeStrPointY = 0;

	GdiplusStartupInput gdiPlusInput;
	GdiplusStartup(&m_gdiplusToken,&gdiPlusInput,NULL);

}

CPlayBackTimeView::DrawControl::~DrawControl()
{
	GdiplusShutdown(m_gdiplusToken);
}
// only called by CPlayBackTimeView at onCreate
void CPlayBackTimeView::DrawControl::prepareDrawData()
{
	RECT rect;
	m_parentView->GetClientRect(&rect);

	LONG nWidth =rect.right - rect.left;
	LONG nHeight = rect.bottom - rect.top ;

	// top bar X coordinate
	m_topBarX = leftPadding;
	// top bar Y coordinate
	m_topBarY = topPadding+barLineHeight;

	// bottom bar X coordinate
	m_bottomBarX = leftPadding;
	// bottom bar Y coordinate
	m_bottomBarY = nHeight - barHeight - bottomPadding;


	if ((nWidth - leftPadding - rightPadding) % hourCount == 0)
	{
		m_barWidth = nWidth - leftPadding - rightPadding;
		m_hourInterval = m_barWidth / hourCount;
	}
	else
	{
		m_hourInterval = (nWidth - leftPadding - rightPadding) / hourCount;
		m_barWidth = (int)m_hourInterval * hourCount;
	}

	m_fiveMinuteInterval = m_barWidth / minuteCount;

	m_currentTopTrickerPoint.SetPoint(m_topBarX,m_topBarY + barHeight+triangleMarinBarBottom);
	m_currentBottomTrickerPoint.SetPoint(m_bottomBarX, m_bottomBarY+barHeight+triangleMarinBarBottom);

	m_currentTopTrickerRect.SetRect(m_topBarX - triangleX,
		m_topBarY + barHeight+triangleMarinBarBottom, 
		m_topBarX + triangleX,
		m_topBarY + barHeight+triangleMarinBarBottom+ triangleY);

	m_currentBottomTrickerRect.SetRect(m_bottomBarX - triangleX,
		m_bottomBarY+barHeight+triangleMarinBarBottom,
		m_bottomBarX + triangleX,
		m_bottomBarY + barHeight + triangleMarinBarBottom + triangleY);

	m_topActiveValidRect.SetRect(m_topBarX,
		m_topBarY+barHeight,
		m_topBarX+m_barWidth,
		m_topBarY+barHeight + triangleY+triangleMarinBarBottom);
	m_bottomActiveValidRect.SetRect(m_bottomBarX,
		m_bottomBarY+barHeight,
		m_bottomBarX+m_barWidth,
		m_bottomBarY+barHeight+triangleY+triangleMarinBarBottom);


	m_timeStrPointX = m_topBarX + triangleX + timeStrMarginLeft;
	m_timeStrPointY = m_topBarY - timeStrMarginTop;
	
}

void CPlayBackTimeView::DrawControl::draw()
{
	drawTimeBar();
	drawTimeLine();
	drawTracker();
}

void CPlayBackTimeView::DrawControl::drawTimeBar()
{
	Pen whitePen(Color::White);
	Graphics gp(m_parentView->GetSafeHwnd());
	
	// draw top bar
	gp.DrawRectangle(&whitePen,m_topBarX,m_topBarY,m_hourInterval*hourCount,barHeight);

	// draw bottom bar
	gp.DrawRectangle(&whitePen,m_bottomBarX,m_bottomBarY,m_hourInterval*hourCount,barHeight);		
	
}


void CPlayBackTimeView::DrawControl::drawTimeLine()
{
	Pen whitePen(Color::White);
	Graphics gp(m_parentView->GetSafeHwnd());

	// draw top bar line:minute
	for (int i = 0; i <= minuteCount; ++i)
	{
		gp.DrawLine(&whitePen, 
			float(m_topBarX+i*m_fiveMinuteInterval),
			float(m_topBarY-barLineHeight),
			float(m_topBarX+i*m_fiveMinuteInterval),
			float(m_topBarY));
	}

	// draw bottom bar line:hour
	for (int i = 0; i <= hourCount; ++i)
	{
		gp.DrawLine(&whitePen,
			float(m_bottomBarX+i*m_hourInterval), 
			float(m_bottomBarY-barLineHeight),
			float(m_bottomBarX+i*m_hourInterval),
			float(m_bottomBarY));
	}

	// draw minute string
	for (int i = 0; i <= minuteCount; ++i)
	{
		//gp.DrawString()
		SolidBrush  brush(Color::White);
		FontFamily  fontFamily(fontName);
		Font        font(&fontFamily, fontSize);

		// minute string point
		float pointX =static_cast<float> (i < 2 ? 
			(m_topBarX - fontMarginLeftA + i*m_fiveMinuteInterval):
		(m_topBarX - fontMarginLeftC + i*m_fiveMinuteInterval));
		float pointY = static_cast<float>(m_topBarY-barLineHeight - fontMarginTop);


		PointF      pointF(pointX, pointY);
		wchar_t minuteStr[10];
		swprintf_s(minuteStr,L"%d",i*fiveMinute);
		gp.DrawString(minuteStr, -1, &font, pointF, &brush);

	}
	// draw hour string
	for (int i = 0; i <= hourCount; ++i)
	{
		SolidBrush  brush(Color::White);
		FontFamily  fontFamily(fontName);
		Font        font(&fontFamily, fontSize);

		// minute string point
		float pointX =static_cast<float> (i < 10 ? 
			(m_bottomBarX - fontMarginLeftA + i*m_hourInterval):
		(m_bottomBarX - fontMarginLeftB + i*m_hourInterval));
		float pointY = static_cast<float>(m_bottomBarY-barLineHeight - fontMarginTop);


		PointF      pointF(pointX, pointY);
		wchar_t hourStr[10];
		swprintf_s(hourStr,L"%d",i);
		gp.DrawString(hourStr, -1, &font, pointF, &brush);
	}
}

void CPlayBackTimeView::DrawControl::drawCurrentTime(COleDateTime& time,float x,bool isErase)
{
	Graphics gp(m_parentView->GetSafeHwnd());
	Font myFont(L"Cambria", 9);
	StringFormat format;
	format.SetAlignment(StringAlignmentFar);

	
	wchar_t timeStr[12];
	memset(timeStr,0,sizeof(timeStr));
	swprintf_s(timeStr,L"%02d:%02d:%02d",time.GetHour(),time.GetMinute(),time.GetSecond());

	if (isErase)
	{
		RectF layoutRect(m_timeStrPointX, m_timeStrPointY, (float)timeStrWidth, (float)timeStrHeight);
		SolidBrush  brush(Color::Black);
		gp.DrawString(
			timeStr,
			-1,
			&myFont,
			layoutRect,
			&format,
			&brush);
#if 1		
		RECT rect = {m_timeStrPointX,m_timeStrPointY,m_timeStrPointX + timeStrWidth, m_timeStrPointY+timeStrHeight};
		Pen BlackPen(Color::Black);
		CRect rect1(m_timeStrPointX,m_timeStrPointY,m_timeStrPointX + timeStrWidth, m_timeStrPointY+timeStrHeight);
		gp.DrawRectangle(&BlackPen,m_timeStrPointX,m_timeStrPointY,timeStrWidth,timeStrHeight);
		m_parentView->InvalidateRect(&rect1);
#endif	

	//	gp.Clear(Color::Black);
	
		

	}
	else
	{
		
		float pointX = x + triangleX + timeStrMarginLeft;
		float pointY = m_currentTopTrickerPoint.y-timeStrMarginTop;
		if (x >= m_fiveMinuteInterval * 11 -10)
		{
			pointX = x-triangleX-timeStrWidth-timeStrMarginLeft;
		}

		

		PointF      pointF(pointX, pointY);

		RectF layoutRect(pointX, pointY, (float)timeStrWidth, (float)timeStrHeight);
		SolidBrush  brush(Color::GreenYellow);
		gp.DrawString(
			timeStr,
			-1,
			&myFont,
			layoutRect,
			&format,
			&brush);
		
		m_timeStrPointX = pointX;
		m_timeStrPointY = pointY;
		//gp.DrawString(timeStr,-1,&font, pointF,&brush);
	}



}

void CPlayBackTimeView::DrawControl::drawTracker()
{
	Pen whitePen(Color::White);
	SolidBrush whiteBrush(Color::White);
	Graphics gp(m_parentView->GetSafeHwnd());

 	Point topBarP1(m_currentTopTrickerPoint.x, m_currentTopTrickerPoint.y);
 	Point topBarP2(m_currentTopTrickerPoint.x - triangleX, m_currentTopTrickerPoint.y+ triangleY);
 	Point topBarP3(m_currentTopTrickerPoint.x + triangleX, m_currentTopTrickerPoint.y+ triangleY);
 
 	Point bottomBarP1(m_currentBottomTrickerPoint.x, m_currentBottomTrickerPoint.y);
 	Point bottomBarP2(m_currentBottomTrickerPoint.x - triangleX, m_currentBottomTrickerPoint.y + triangleY);
 	Point bottomBarP3(m_currentBottomTrickerPoint.x + triangleX, m_currentBottomTrickerPoint.y + triangleY);

	Point topBarPoints[3] = {topBarP1,topBarP2,topBarP3};
	Point bottomBarPoints[3] = {bottomBarP1,bottomBarP2,bottomBarP3};

	gp.DrawPolygon(&whitePen,topBarPoints,3);
	gp.FillPolygon(&whiteBrush,topBarPoints,3);

	gp.DrawPolygon(&whitePen,bottomBarPoints,3);
	gp.FillPolygon(&whiteBrush,bottomBarPoints,3);

}

void CPlayBackTimeView::DrawRecordScope()
{
#if 1	
	videoRectList_iter it1 = m_topRectMergeList.begin(); 
	int hour = m_currentPlayBackTime.GetHour();
	
	for (; it1 != m_topRectMergeList.end(); ++it1)
	{
		if (hour == it1->hour)
			m_pDrawControl->drawMinuteRect(it1->startPoint,it1->endPoint);
	
	}

	videoRectList_iter it2 = m_bottomRectMergeList.begin();

	
	for (; it2 != m_bottomRectMergeList.end(); ++it2)
	{
		m_pDrawControl->drawHourRect(it2->startPoint,it2->endPoint);
		
	}
	



#endif

#if 0
	videoRectList_iter it1 = m_topRectList.begin(); 
	for (; it1 != m_topRectList.end(); ++it1)
	{
		m_pDrawControl->drawMinuteRect(it1->startPoint,it1->endPoint);

	}

	videoRectList_iter it2 = m_bottomRectList.begin();
	for (; it2 != m_bottomRectList.end(); ++it2)
	{
		m_pDrawControl->drawHourRect(it2->startPoint,it2->endPoint);

	}
#endif
}

void CPlayBackTimeView::UpdateTime()
{
//	m_currentPlayBackTime = time;
	if (NULL == m_pShowPlayBackTime)
	{
		return ;
	}
	m_pShowPlayBackTime->SetWindowText(GetNowPlayBackTime());
}

void CPlayBackTimeView::TrickerMove(UINT nFlags, CPoint point)
{

	m_pDrawControl->m_oldTopTrickerPoint = m_pDrawControl->m_currentTopTrickerPoint;
	m_pDrawControl->m_oldBottomTrickerPoint = m_pDrawControl->m_currentBottomTrickerPoint;
	// set top bar tricker;
	if (m_isInTopTracker)
	{
		bool isDraw = true;
		m_pDrawControl->drawTopTricker(point, isDraw);
	
		if (isDraw)
		{
			UpdateTime();
		}	
	}

	else if (m_isInBottomTracker)
	{
		bool isDraw = true;
		int currentHours = m_pDrawControl->drawBottomTricker(point, isDraw);
		if (isDraw)
		{
			UpdateTime();
		}	

		if (currentHours < 0) 
			return;
		Graphics g(GetSafeHwnd());
		if ((int)currentHours != hourCount)
		{
			m_pDrawControl->fillTopBar();
			videoRectList_iter it = m_topRectMergeList.begin();
			for (; it != m_topRectMergeList.end(); ++it)
			{
				if (it->hour == currentHours)
				{
					m_pDrawControl->drawMinuteRect(it->startPoint,it->endPoint);
				}
			}
		}

	}


	if( (m_isInTopTracker||m_isInBottomTracker) && m_isInit )
	{
		COleDateTime time = ChangePointToTime();
		if (IsCurrentTimeExist(time))
		{
			m_currentPlayBackTime = time;
			if(m_notifyMovePosion != NULL && m_isLBtnDown == false)
            {
                m_notifyMovePosion->invoke(&time);
            }
		}
		else
		{
			m_pDrawControl->drawCurrentTricker(
				m_pDrawControl->m_currentTopTrickerPoint,
				m_pDrawControl->m_currentTopTrickerRect,
				m_pDrawControl->m_oldTopTrickerPoint);

			m_pDrawControl->drawCurrentTricker(
				m_pDrawControl->m_currentBottomTrickerPoint,
				m_pDrawControl->m_currentBottomTrickerRect,
				m_pDrawControl->m_oldBottomTrickerPoint);

			m_pDrawControl->fillTopBar();
			videoRectList_iter it1 = m_topRectMergeList.begin(); 
			int hour = m_currentPlayBackTime.GetHour();

			for (; it1 != m_topRectMergeList.end(); ++it1)
			{
				if (hour == it1->hour)
					m_pDrawControl->drawMinuteRect(it1->startPoint,it1->endPoint);

			}
			{
	//			m_pDrawControl->drawCurrentTime(m_currentPlayBackTime,m_pDrawControl->m_oldTopTrickerPoint.x,true);
	//			GetNowPlayBackTime();
	//			m_pDrawControl->drawCurrentTime(m_currentPlayBackTime,m_pDrawControl->m_currentTopTrickerPoint.x,false);

				COleDateTime time = ChangePointToTime();
				m_currentPlayBackTime = time;
				UpdateTime();
			}
			

		}
	}
}

void CPlayBackTimeView::DrawControl::drawMinuteRect(int beginX, int endX)
{
	Graphics gp(m_parentView->GetSafeHwnd());
	SolidBrush greenBrush(Color::Green);
	gp.FillRectangle(&greenBrush,beginX,m_topBarY + (int)barBorderThickness,endX-beginX,(int)(barHeight-2*barBorderThickness));
}

void CPlayBackTimeView::DrawControl::drawHourRect(int beginX, int endX)
{
	Graphics gp(m_parentView->GetSafeHwnd());
	SolidBrush greenBrush(Color::Green);
	gp.FillRectangle(&greenBrush,beginX, m_bottomBarY + (int)barBorderThickness,endX-beginX,(int)(barHeight-2*barBorderThickness));
}

void CPlayBackTimeView::DrawControl::drawTopTricker( CPoint const& point, bool& isDraw)
{
	if (point.x >= m_topBarX && point.x <= m_topBarX + m_barWidth)
	{
		
		double onePixelsSecondsBottom = oneHourSecondsNumber / m_hourInterval;
		double onePixelsSecondsTop = oneHourSecondsNumber/ m_barWidth;


		int topPointX = m_currentTopTrickerPoint.x;
		int  bottomPointX = 0;
		int currentWidth = point.x - m_topBarX;
		double currentSecondsTop = currentWidth * oneHourSecondsNumber / m_barWidth;
	

	
		drawCurrentTricker(m_currentTopTrickerPoint,m_currentTopTrickerRect, point);


		if (topPointX < m_currentTopTrickerPoint.x)//move to right
		{
			if (fmod((m_currentBottomTrickerPoint.x - m_bottomBarX), m_hourInterval) == 0.0 && 
				currentSecondsTop + onePixelsSecondsBottom >= oneHourSecondsNumber)
			{
				return;
			}
 			bottomPointX = m_currentBottomTrickerPoint.x + 
 				(currentSecondsTop - 
 				fmod((m_currentBottomTrickerPoint.x - m_bottomBarX),m_hourInterval) * onePixelsSecondsBottom) / onePixelsSecondsBottom;
		}
		else //move to left
		{
			if (((m_currentBottomTrickerPoint.x - m_bottomBarX) == 0) || 
				((fmod((m_currentBottomTrickerPoint.x - m_bottomBarX) , m_hourInterval) == 0.0) &&
				currentSecondsTop < onePixelsSecondsBottom))
			{
				return;
			}
			int hourSeconds = 0;
			if (fmod((m_currentBottomTrickerPoint.x - m_bottomBarX) , m_hourInterval) == 0.0)
			{
				hourSeconds = oneHourSecondsNumber;
			}
			else
			{
				int hour = (m_currentBottomTrickerPoint.x - m_bottomBarX) / (int)m_hourInterval;
				int pixelsCount = (int)((m_currentBottomTrickerPoint.x - m_bottomBarX) - hour * m_hourInterval);
				hourSeconds = (int)(pixelsCount * onePixelsSecondsBottom);
			}
			bottomPointX = (int)(m_currentBottomTrickerPoint.x - (hourSeconds - currentSecondsTop) / onePixelsSecondsBottom);
		}

		if (bottomPointX - m_bottomBarX > m_barWidth)
		CPoint point(bottomPointX,m_currentBottomTrickerPoint.y);
		drawCurrentTricker(m_currentBottomTrickerPoint, m_currentBottomTrickerRect,point);
	}
	else
	{
		isDraw = false;
	}
}

int CPlayBackTimeView::DrawControl::drawBottomTricker( CPoint const& point, bool& isDraw)
{
	if (point.x >= m_bottomBarX && point.x <= m_bottomBarX + m_barWidth)
	{
		int bottomPointX = m_currentBottomTrickerPoint.x;

		drawCurrentTricker(m_currentBottomTrickerPoint,m_currentBottomTrickerRect, point);
		
	
		//sync topbar
		int currentWidth = point.x - m_bottomBarX;
		double currentHours = currentWidth * hourCount / m_barWidth;
		double currentSeconds = fmod(currentWidth , m_hourInterval) * oneHourSecondsNumber / m_hourInterval;
		int topX = 0;
		if (bottomPointX <= m_currentBottomTrickerPoint.x)//move right
		{
			int toptargetX = (int)(currentSeconds * m_barWidth / oneHourSecondsNumber);
			if ((m_currentBottomTrickerPoint.x - m_bottomBarX) / m_hourInterval == hourCount)
			{
				topX = m_barWidth + m_topBarX;
			}
			else
			{
				topX = toptargetX + m_topBarX;
			}
		}
		else // move left
		{
			topX = (int)(currentSeconds * m_barWidth / oneHourSecondsNumber + m_topBarX);
		}

		CPoint point(topX,m_currentTopTrickerPoint.y);
		drawCurrentTricker(m_currentTopTrickerPoint, m_currentTopTrickerRect,point);

		return (int)currentHours;
	}
	else
	{
		isDraw = false;
	}
	return -1;
}


void CPlayBackTimeView::DrawControl::drawCurrentTricker(CPoint& currentPoint, CRect& currentRect,CPoint const& point)
{
	HWND hwd = m_parentView->GetSafeHwnd();
	Graphics g(hwd);


	Pen whitePen(Color::White);
	SolidBrush whiteBrush(Color::White);

	Pen blackPen(Color::Black);
	SolidBrush blackBrush(Color::Black);

	Point BarP11(currentPoint.x, currentPoint.y);
	Point BarP12(currentPoint.x - triangleX, currentPoint.y + triangleY);
	Point BarP13(currentPoint.x + triangleX, currentPoint.y + triangleY);

	


	Point BarPoints[3] = {BarP11,BarP12,BarP13};

	g.DrawPolygon(&blackPen,BarPoints,3);
	g.FillPolygon(&blackBrush,BarPoints,3);

	currentPoint.x = point.x;

	Point BarP21(currentPoint.x, currentPoint.y);
	Point BarP22(currentPoint.x - triangleX, currentPoint.y + triangleY);
	Point BarP23(currentPoint.x + triangleX, currentPoint.y + triangleY);
	Point BarPoints2[3] = {BarP21,BarP22,BarP23};
	g.DrawPolygon(&whitePen,BarPoints2,3);
	g.FillPolygon(&whiteBrush,BarPoints2,3);

	currentRect.SetRect(BarP22.X, BarP21.Y,BarP23.X,BarP23.Y);
}


COleDateTime CPlayBackTimeView::ChangePointToTime()
{
	double currentWidth = m_pDrawControl->m_currentTopTrickerPoint.x - leftPadding;
	double currentSeconds = currentWidth * oneHourSecondsNumber /m_pDrawControl->GetBarWidth();
	double onePixelsSeconds = oneHourSecondsNumber /m_pDrawControl->GetBarWidth(); 

	int currentMinute = currentSeconds / oneMinuteSecondsNumber;
	int currentSecond = fmod(currentSeconds , oneMinuteSecondsNumber);
	int currentHour = (m_pDrawControl->m_currentBottomTrickerPoint.x - leftPadding) / m_pDrawControl->GetHourWidth();

	if (currentMinute == oneMinuteSecondsNumber)
	{
		currentMinute = lastMinuteOrSecond;
		currentSecond = lastMinuteOrSecond;
	}
	if (fmod((m_pDrawControl->m_currentBottomTrickerPoint.x - leftPadding), m_pDrawControl->GetHourWidth()) == 0.000000f 
		&& m_pDrawControl->m_currentTopTrickerPoint.x + onePixelsSeconds > leftPadding + m_pDrawControl->GetBarWidth())
	{
// 		double temp = fmod((m_pDrawControl->m_currentBottomTrickerPoint.x - leftPadding), m_pDrawControl->GetHourWidth());
// 		double temp1 = m_pDrawControl->m_currentTopTrickerPoint.x + onePixelsSeconds;
// 		double temp2 = leftPadding + m_pDrawControl->GetBarWidth();
		
		currentHour  = (currentHour == 0) ?   currentHour: currentHour - 1;
	}

	if (m_isInit)
	{
// 
// 		double temp1 = m_pDrawControl->m_currentTopTrickerPoint.x + onePixelsSeconds ;
// 		double temp2 = leftPadding + m_pDrawControl->GetBarWidth();
// 		double temp = fmod((m_pDrawControl->m_currentBottomTrickerPoint.x - leftPadding), m_pDrawControl->GetHourWidth());

	
		return COleDateTime(m_startTime.GetYear(), m_startTime.GetMonth(), m_startTime.GetDay(),
			currentHour, currentMinute, currentSecond);
	}
	else 
	{
		return COleDateTime(1900, 1, 1,
			currentHour, currentMinute, currentSecond);
	}
	
}

CString CPlayBackTimeView::GetNowPlayBackTime()
{
	COleDateTime time = ChangePointToTime();
	//m_currentPlayBackTime = time;
	CString temp;
	temp.Format("%02d:%02d:%02d",time.GetHour(),time.GetMinute(),time.GetSecond());
	return temp;
}

void CPlayBackTimeView::DrawControl::drawStartPlayBackTime()
{

}

void CPlayBackTimeView::DrawControl::fillTopBar()
{
	HWND hwd = m_parentView->GetSafeHwnd();
	Graphics g(hwd);
	g.SetSmoothingMode(SmoothingModeHighQuality);
	
	SolidBrush  blackBrush(Color::Black);

	g.FillRectangle(&blackBrush, 
		m_topBarX+(int)barBorderThickness,
		m_topBarY + (int)barBorderThickness,
		m_barWidth-(int)(2*barBorderThickness),
		barHeight-(int)(2*barBorderThickness));

}

void CPlayBackTimeView::DrawControl::fillBottomBar()
{
	HWND hwd = m_parentView->GetSafeHwnd();
	Graphics g(hwd);
	g.SetSmoothingMode(SmoothingModeHighQuality);

	SolidBrush  blackBrush(Color::Black);

	g.FillRectangle(&blackBrush, 
		m_bottomBarX+(int)barBorderThickness,
		m_bottomBarY + (int)barBorderThickness,
		m_barWidth-(int)(2*barBorderThickness),
		barHeight-(int)(2*barBorderThickness));
}

void CPlayBackTimeView::DrawControl::resetControl()
{

	fillTopBar();
	fillBottomBar();
	CPoint topPoint(m_topBarX, m_topBarY);
	CPoint bottomPoint(m_bottomBarX, m_bottomBarY);
	drawCurrentTricker(m_currentTopTrickerPoint,m_currentTopTrickerRect,topPoint);
	drawCurrentTricker(m_currentBottomTrickerPoint,m_currentBottomTrickerRect, bottomPoint);
}


CPlayBackTimeView::DrawControl * CPlayBackTimeView::DrawControl::getInstance(CView* pView)
{
	static DrawControl instance(pView);
	return &instance;
}


// CColorStatic

IMPLEMENT_DYNAMIC(CColorStatic, CStatic)

CColorStatic::CColorStatic()
{
	m_bTran = true;
	m_crText = Color::White;
	m_crBackColor = Color::Black;
	memset(&m_lf, 0, sizeof(m_lf));

}

CColorStatic::~CColorStatic()
{
}


BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
//	ON_WM_CTLCOLOR()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



// CColorStatic 消息处理程序

void CColorStatic::SetTextColor(COLORREF crText)
{
	m_crText = crText;
}

void CColorStatic::SetFontSize(int nSize)
{
	
	m_font.DeleteObject();
	m_font.CreatePointFont(nSize*10,"Arial");
	
	RedrawWindow();
}

void CColorStatic::SetBackColor(COLORREF crBackColor)
{
	m_crBackColor = crBackColor;
	m_brush.Detach();
	m_brush.CreateSolidBrush(m_crBackColor);
	RedrawWindow();
}

void CColorStatic::SetTransparent(bool bTran)
{
	m_bTran = bTran;
	RedrawWindow();
}

HBRUSH CColorStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	if (CTLCOLOR_STATIC == nCtlColor)
	{
		pDC->SelectObject(&m_font);
		pDC->SetTextColor(m_crText);
		pDC->SetBkColor(m_crBackColor);

		if (m_bTran == true)
			pDC->SetBkMode(TRANSPARENT);
		
	}
	return m_brush;
}

BOOL CPlayBackTimeView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
// 	CRect rc;
// 	GetClientRect(&rc);
// 	CBrush brush;
// 	brush.CreateSolidBrush(RGB(0,0,0));
// 	pDC->FillRect(&rc, &brush);


	int nWidth;
	int nHeight;

// 	CPlayBackTimeView* pDoc = GetDocument();
// 	ASSERT_VALID(pDoc);
	CRect rect;
	GetWindowRect(&rect);
	nWidth = rect.Width();
	nHeight = rect.Height();

	CDC MemDC;
	CBitmap MemBitmap;

	MemDC.CreateCompatibleDC(NULL);
	MemBitmap.CreateCompatibleBitmap(pDC, nWidth, nHeight);

	CBitmap *pOldBit = MemDC.SelectObject(&MemBitmap);
	MemDC.FillSolidRect(0,0,nWidth,nHeight,RGB(0,0,0));

	pDC->BitBlt(0,0,nWidth,nHeight,&MemDC,0,0,SRCCOPY);
	
	MemBitmap.DeleteObject();
	MemDC.DeleteDC();

	return TRUE;
//	return CView::OnEraseBkgnd(pDC);
}
