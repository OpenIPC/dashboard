#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CSearchPictureDlg 对话框

class CSearchPictureDlg : public CDialog
{
	DECLARE_DYNAMIC(CSearchPictureDlg)

public:
	CSearchPictureDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSearchPictureDlg();

// 对话框数据
	enum { IDD = IDD_SEARCH_PICTURE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedBtnSearch();
    afx_msg void OnBnClickedBtnPreviousPage();
    afx_msg void OnBnClickedBtnNextPage();
    afx_msg void OnBnClickedBtnDownload();
    afx_msg void OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
private:
    CListCtrl m_ctrPicList;
    CComboBox m_cmbChannel;
    CComboBox m_cmbCoatType;
    CComboBox m_cmbCoatColor;
    CComboBox m_cmbTrousersType;
    CComboBox m_cmbTrousersColor;
    CComboBox m_cmbBag;
    CComboBox m_cmbHat;
    CButton m_btnDownload;
    CButton m_btnPreviousPage;
    CButton m_btnNextPage;
 
    int m_nTotalPage;
    int m_nCurPage;
    int m_nIndexOfTrafficPicture;
    int m_nTotalCountOfTrafficPicture;
    std::vector<MEDIAFILE_HUMAN_TRAIT_INFO*> m_vecHumanTraitPicture;

    LLONG m_lFindPictureHandle;
    LLONG m_lDownloadHandle;
    LLONG m_lLoginHandle;
    int m_nChannelNum;

    void CleanUpResultOfLastTimeQuery();
    LLONG StartFindPictures(  );
    bool DoFindPictures( int nQueryCount );
    void ShowPictureInfo(MEDIAFILE_HUMAN_TRAIT_INFO* pRecordList, int nBeginIndex);
    void GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime);
	bool IsTimeCorrent();

public:
    void Init(LLONG lLoginID, int nChannel);
};
