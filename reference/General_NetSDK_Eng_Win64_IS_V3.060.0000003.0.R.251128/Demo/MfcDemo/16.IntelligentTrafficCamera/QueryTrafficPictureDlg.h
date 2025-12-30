#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CQueryPictureAndRecordDlg Dialog

class CQueryTrafficPictureDlg : public CDialog
{
	DECLARE_DYNAMIC(CQueryTrafficPictureDlg)

public:
	CQueryTrafficPictureDlg(CWnd* pParent = NULL,int nChannel = 0,LLONG lLoginID = 0);   // standard constructor
	virtual ~CQueryTrafficPictureDlg();

	// Dialog Data
	enum { IDD = IDD_QUERY_TRAFFIC_PICTURE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnQuery();
	afx_msg void OnBnClickedBtnPtrviousPage();
	afx_msg void OnBnClickedBtnNextPage();
	afx_msg void OnBnClickedBtnDownload();
	afx_msg void OnDestroy();
	afx_msg void OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

private:
	LLONG StartFindPictures();
	int GetTotalCountOfPicture();
	bool DoFindPicutres( int nQueryCount );
	void GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime);
	void ShowTrafficPictureInfo(MEDIAFILE_TRAFFICCAR_INFO_EX* pRecordList, int nBeginIndex);
	void CleanUpResultOfLastTimeQuery();
	LRESULT OnDownloadPicturePos(WPARAM wParam, LPARAM lParam);
	bool IsTimeCorrent();

private:
	CButton m_btnQuery;
	CButton m_btnDownload;
	CButton m_btnPreviousPage;
	CButton m_btnNextPage;
	CListCtrl m_ctrPicture;
	CComboBox m_cmbEventType;
	CComboBox m_cmbChannel;

	LLONG	m_lLoginHandle;
	LLONG	m_lFindPictureHandle;
	LLONG	m_lDownloadHandle;
	int		m_nChannel;

	int m_nTotalPage;
	int m_nCurPage;
	int m_nIndexOfTrafficPicture;
	int m_nTotalCountOfTrafficPicture;
	std::vector<MEDIAFILE_TRAFFICCAR_INFO_EX*> m_vecTrafficPicture;

public:
	void CleanUp();
};
