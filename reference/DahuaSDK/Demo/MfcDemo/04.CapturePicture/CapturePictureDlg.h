// CapturePictureDlg.h : header file
//

#if !defined(AFX_CAPTUREPICTUREDLG_H__D9C45EF9_395C_4B97_BC84_822B259025D9__INCLUDED_)
#define AFX_CAPTUREPICTUREDLG_H__D9C45EF9_395C_4B97_BC84_822B259025D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCapturePictureDlg dialog
#include "../../../Include/Common/dhnetsdk.h"

#if defined(_WIN64)
	#pragma comment(lib, "../../../Lib/Win64/dhnetsdk.lib")
#else
	#pragma comment(lib, "../../../Lib/Win32/dhnetsdk.lib")
#endif


class CCapturePictureDlg : public CDialog
{
// Construction
public:
	void OnOnePicture(LLONG ILoginID, BYTE *pBuf, UINT RevLen, UINT EncodeType, UINT CmdSerial);
	CCapturePictureDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCapturePictureDlg)
	enum { IDD = IDD_CAPTUREPICTURE_DIALOG };
	CStatic	m_showbmp;
	CComboBox	m_ctlChannel;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCapturePictureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation

protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCapturePictureDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLogin();
	afx_msg void OnButtonLogout();	
	afx_msg void OnClose();
	afx_msg void OnSelchangeComboChannel();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int		m_nChannelCount;
	LLONG	m_LoginID;
	void	ShowLoginErrorReason(int nError);
	void	InitNetSDK();
	HBITMAP LoadImageFile(LPCSTR szImagePath, int &nWidth, int &nHeight);
	HBITMAP ExtractBitmap(IPicture *pPicture, int &nWidth, int &nHeight);
	void	ShowSnapImage(LPCSTR szImagePath);
	void	FillCWndWithDefaultColor(CWnd* cwnd);
private:
	DH_SNAP_ATTR_EN			m_stuSnapAttr;		
	DHDEV_SYSTEM_ATTR_CFG   m_stuDevAttrInfo;
	BOOL                    m_bDataError[32];
	IPicture*				m_pPicture;
public:
	afx_msg void OnBnClickedButtonStart();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAPTUREPICTUREDLG_H__D9C45EF9_395C_4B97_BC84_822B259025D9__INCLUDED_)
