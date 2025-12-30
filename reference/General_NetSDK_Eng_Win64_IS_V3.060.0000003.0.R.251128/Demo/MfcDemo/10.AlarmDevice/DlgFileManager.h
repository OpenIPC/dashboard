#if !defined(AFX_DLGFILEMANAGER_H__479A79E3_EA14_4164_A2A4_582D0B3977A3__INCLUDED_)
#define AFX_DLGFILEMANAGER_H__479A79E3_EA14_4164_A2A4_582D0B3977A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFileManager.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgFileManager dialog

class CDlgFileManager : public CDialog
{
// Construction
public:
	CDlgFileManager(CWnd* pParent = NULL, LLONG hLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgFileManager)
	enum { IDD = IDD_DLG_FILEMANAGER };
	CProgressCtrl	m_progressUpload;
	CProgressCtrl	m_progressDownload;
	CString	m_csFileNameDownload;
	CString	m_csFileNameUpload;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFileManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgFileManager)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnGetFileToUpLoad();
	afx_msg void OnBtnSetFilePathToDownLoad();
	afx_msg void OnBtnStopUpLoad();
	afx_msg void OnBtnStopDownLoad();
	afx_msg void OnBtnStartUpLoad();
	afx_msg void OnBtnStartDownLoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	LLONG		m_hLogin;
	CString		m_CSFileUpload;
	CString		m_CSFolderUpload;
	CString		m_CSFolderDownload;
	AV_HANDLE	m_hUpload;
	AV_HANDLE	m_hDownload;
	FILE		*m_pFile;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFILEMANAGER_H__479A79E3_EA14_4164_A2A4_582D0B3977A3__INCLUDED_)
