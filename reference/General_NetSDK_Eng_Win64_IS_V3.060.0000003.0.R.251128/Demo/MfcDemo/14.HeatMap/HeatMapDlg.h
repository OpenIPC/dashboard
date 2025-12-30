// HeatMapDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "PictureCtrl.h"


// CHeatMapDlg dialog
class CHeatMapDlg : public CDialog
{
// Construction
public:
	CHeatMapDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HEATMAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnLoginAndLogout();
    afx_msg void OnBnClickedBtnStartPlayAndStopPlay();
	afx_msg void OnBnClickedBtnQueryHeatMap();
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

private:
    // Self-define message function -- device disconnect function WM_DEVICE_DISCONNECT
    LRESULT OnDeviceDisConnect(WPARAM wParam, LPARAM lParam);

    // Init NetSDK
    void InitNetSdk();

    // Init dialog Control 
    void InitDialogContorl();

    // Get the time from time control
    void GetTimeFromTimeCtr(NET_TIME_EX& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime);

    // Whether the begin time is less than end time
    bool IsTimeCorrent();
private:
    // Login handle
    LLONG   m_lLoginHandle;

    // Real play handle
    LLONG   m_lRealPlayHandle;

    // Picture control, show HeatMap picture
    PictureCtrl m_ctrPicture;
};
