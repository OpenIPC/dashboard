// TargetRecognitionDlg.h : header file
//

#if !defined(AFX_TARGETRECOGNITIONDLG_H__D5AA7D69_AAA4_49AC_90C2_45E13CEA71CB__INCLUDED_)
#define AFX_TARGETRECOGNITIONDLG_H__D5AA7D69_AAA4_49AC_90C2_45E13CEA71CB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdAfx.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CTargetRecognitionDlg dialog

class CTargetRecognitionDlg : public CDialog
{
// Construction
public:
	CTargetRecognitionDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CTargetRecognitionDlg();

// Dialog Data
	//{{AFX_DATA(CTargetRecognitionDlg)
	enum { IDD = IDD_TARGETRECOGNITION_DIALOG };
	CComboBox	m_comboChannel;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTargetRecognitionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	LLONG	m_lLoginID;
	int		m_nChnCnt;
	LLONG	m_lRealPicHandle;
	LLONG	m_lRealHandle;
	char	*m_pszSoftPath;
	DWORD	m_nFaceDetectGroupId;


protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTargetRecognitionDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLogin();
	afx_msg void OnButtonAttachOrDeteach();
	afx_msg void OnButtonDeteach();
	afx_msg void OnButtonFaceDb();
	afx_msg void OnBnClickedButtonRealplay();
	afx_msg void OnBnClickedButtonQueryDownload();
	afx_msg void OnBnClickedButtonSearchbypic();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void DoFaceData(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize);

	afx_msg LRESULT DoDeviceAttach(WPARAM wParam, LPARAM lParam);
	
private:
	void InitSdk();
	void ShowLoginErrorReason(int nError);
	void OnButtonLoginOut();
	void OnButtonLoginIn();
	
	void ShowFaceDataInfo(NET_FACE_DATA *pstDataInfo);
	void ShowFaceDataInfo(DEV_EVENT_FACEDETECT_INFO *pstDataInfo);
	void ClearShowEventInfo();
	void ShowGlobalPicture(DEV_EVENT_FACERECOGNITION_INFO *pstInfo, BYTE *pBuffer);
	void ShowPersonPicture(DEV_EVENT_FACERECOGNITION_INFO *pstInfo, BYTE *pBuffer);
	void ShowCandidatePicture(DEV_EVENT_FACERECOGNITION_INFO *pstInfo, BYTE *pBuffer);
	void ShowFaceDetectPicture(DEV_EVENT_FACEDETECT_INFO *pstInfo, BYTE *pBuffer, DWORD dwBufSize);
	void FillCWndWithDefaultColor(CWnd* cwnd);
	//void PaintImage(int nId, IPicture* m_pPicture);
	void ClearCandidateShowInfo();

	LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);

public:
	CPictureShow m_pBicPic;
	CPictureShow m_pCandiPic;
	CPictureShow m_pFacePic;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TARGETRECOGNITIONDLG_H__D5AA7D69_AAA4_49AC_90C2_45E13CEA71CB__INCLUDED_)
