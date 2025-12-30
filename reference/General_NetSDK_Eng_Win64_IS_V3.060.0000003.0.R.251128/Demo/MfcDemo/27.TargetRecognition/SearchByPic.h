#if !defined(AFX_SEARCHBYPIC_H__3562A868_C9D3_4A9D_8BD6_104CD36264EF__INCLUDED_)
#define AFX_SEARCHBYPIC_H__3562A868_C9D3_4A9D_8BD6_104CD36264EF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SearchByPic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSearchByPic dialog
#include "FaceMutex.h"
#include "afxwin.h"

typedef struct tagNET_THREAD_PARAM
{
	LLONG	lFindHandle;
	LLONG	lAttachHandle;
	LDWORD  dwUser;
} NET_THREAD_PARAM;

class CSearchByPic : public CDialog
{
	// Construction
public:
	CSearchByPic(const LLONG lLoginHandle, const int m_nChnCnt, char *pSoftPath = NULL, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchByPic();
	// Dialog Data
	//{{AFX_DATA(CSearchByPic)
	enum { IDD = IDD_SEARCH_BY_PIC_DIALOG };
	CComboBox	m_comboChannel;
	//	CComboBox	m_comDbType;
	CProgressCtrl	m_ctlSearchPro;
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	void DoFindTargetRecognitionFunc();
	void DoUpdateCount();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchByPic)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSearchByPic)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenPic();
	afx_msg void OnSearchPic();
	afx_msg void OnBnClickedButtonSearchpicStop();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ClearShowPersonInfoSearchByPic();
	void AddCandidateInfoToList(CANDIDATE_INFOEX * pstAddInfo);
	void ClearCandidateInfo();
	void ShowPersonInfoSearchByPic(FACERECOGNITION_PERSON_INFOEX *pstPersonInfo, int index, int nbySimilarity);
	LRESULT OnDeviceAttch(WPARAM wParam, LPARAM lParam);
	void	OnTimer(UINT_PTR nIDEvent);

	void	StopAttchHandle();
	void	StopFindHandle();

public:
	int			m_nProgress;
	int			m_nTotalCount;

private:
	char	m_szFilePath[FACE_MAX_PATH_LEN];
	LLONG	m_lLoginId;
	std::list<CANDIDATE_INFOEX*>	 m_lstCandidateInfo; // the list of candidate info
	char		m_szGroupId[GROUP_ID_NUM];	
	int			m_nChannelCnt;
	DWORD		m_dwThreadID;
	LLONG		m_lFindHandle;
	LLONG		m_lAttachHandle;
	char 		*m_pSoftPath;
	CFaceMutex	m_Mutex;		
	UINT		m_nTimer;
	HANDLE		m_lEvent;
public:
	CPictureShow m_pPicShow;
	CPictureShow m_pResultPic2;
	CPictureShow m_pResultPic1;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHBYPIC_H__3562A868_C9D3_4A9D_8BD6_104CD36264EF__INCLUDED_)
