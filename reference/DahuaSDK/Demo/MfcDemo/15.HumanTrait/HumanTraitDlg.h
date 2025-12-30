// HumanTraitDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "PictureCtrl.h"

// CHumanTraitDlg dialog
class CHumanTraitDlg : public CDialog
{
// Construction
public:
	CHumanTraitDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HUMANTRAIT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedBtnLoginAndLogout();
    afx_msg void OnBnClickedBtnSubscirbe();
    afx_msg void OnBnClickedBtnStartPlayAndStopPlay();

    LRESULT CHumanTraitDlg::OnDeviceDisConnect(WPARAM wParam, LPARAM lParam);
    LRESULT CHumanTraitDlg::OnDeviceReconnect(WPARAM wParam, LPARAM lParam);
    LRESULT CHumanTraitDlg::OnIntelligentEvent(WPARAM wParam, LPARAM lParam);
    CButton m_btnPlay;
    CComboBox m_cmbChannel;
    CButton m_btnSubscibe;
    CButton m_btnSearch;

    LLONG   m_lLoginHandle;
    LLONG   m_lRealPlayHandle;
    LLONG   m_lRealLoadHandle;
    int     m_nChannelNum;
    afx_msg void OnDestroy();
    PictureCtrl m_ctrHumanTratPicture;
    PictureCtrl m_ctrFacePicture;
    afx_msg void OnBnClickedBtnSearchPicture();
};
