#pragma once
#include <atlimage.h>
#include "PictureWnd.h"
#include "Lock.h"
using namespace ATL;

// CDialogPicEvent 对话框
#define WM_PICEVENTALARM_INFO (WM_USER + 700)

const COLORREF RESERVED_COLOR_EVENT = RGB(255, 255, 255);

enum Color {
    PIC_UNKNOW,    // 默认为0
    GREEN,  // 默认为1
    BLUE    // 默认为2
};


class CDialogPicEvent : public CDialog
{
	DECLARE_DYNAMIC(CDialogPicEvent)

public:
	CDialogPicEvent(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogPicEvent();

	//自定义对外接口
	void setChnCount(int count);

// 对话框数据
	enum { IDD = IDD_DIALOG_PICEVENT };
	CListCtrl		m_listPicEventAlarm;
	CComboBox		m_picChannel;

	int m_chnCount;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	afx_msg LRESULT OnPicEventAlarmInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()

private:	
	void			InitCtrls();
	BOOL			StartListenEventAlarm();
	BOOL			StopListenEventAlarm();

	// 存放图片数据到固定文件与绘制接口
	BOOL			addPicToFile(EM_ACCESS_CTL_IMAGE_TYPE type, BYTE* content, int dataSize);
	BOOL			drawPic(EM_ACCESS_CTL_IMAGE_TYPE type);

	// 存放图片数据到变化文件与绘制接口
	BOOL			addPicToFilePath(const char* filePath, BYTE* data, int dataSize);
	BOOL			drawPicFile(EM_ACCESS_CTL_IMAGE_TYPE type, const char* filePath);

	void			InitPicControl();
	void			FillCWndWithDefaultColor(CWnd* cwnd);

	void			clearWindows();

	BOOL			m_bListenEvent;

	CPictureWnd		m_picGlobalWnd;
	CPictureWnd		m_picFaceWnd;
	CPictureWnd		m_picCadidateWnd;
	CRITICAL_SECTION		m_csLock; 

public:
	afx_msg void OnBnClickedButtonStartPic();
	afx_msg void OnBnClickedButtonStopPic();
};
