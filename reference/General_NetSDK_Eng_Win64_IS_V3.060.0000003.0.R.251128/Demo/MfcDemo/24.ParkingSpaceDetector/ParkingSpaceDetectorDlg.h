// IntelligentTrafficDlg.h : header file
//

#if !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
#define AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PictureWnd.h"
#include "afxwin.h"
#include "PictureCtrl.h"
#include "Utils/Lock.h"
#include "Observer.h"

#include "resource.h"

#define WM_USER_EVENT_COME (WM_USER + 1)

struct AlarmData;
class CAlaram;

/////////////////////////////////////////////////////////////////////////////
// CParkingSpaceDetectorDlg dialog

class CController;

class CParkingSpaceDetectorDlg : public CDialog, public CObserver
{
	typedef enum UI_STATE
	{
		UI_STATE_SUBSCRIBE_NOW,
		UI_STATE_SUBSCRIBE_STOP
	};
// Construction
public:
	CParkingSpaceDetectorDlg(CWnd* pParent = NULL);	// standard constructor
	~CParkingSpaceDetectorDlg();	

// Dialog Data
	//{{AFX_DATA(CParkingSpaceDetectorDlg)
	enum { IDD = IDD_REALLOADPICTURE_DIALOG };
	CListCtrl		m_lcRealEvent;	
	CComboBox		m_cbChannel;
	CPictureCtrl	m_ctrlPlateImage;
	CMenu			m_Menu;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParkingSpaceDetectorDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CParkingSpaceDetectorDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonLogin();
	afx_msg void OnButtonLogout();
	afx_msg void OnButtonStartPreview();
	afx_msg void OnButtonStopPreview();
	afx_msg void OnButtonStart();
	afx_msg void OnButtonStop();
	afx_msg void OnNMClickListctrlRealEvent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();		
	afx_msg void OnKeydownListctrlRealEvent(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnClose();
	afx_msg LRESULT OnDisConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEventCome(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//static int  __stdcall RealLoadPicCallback (LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *userdata);
public:
	afx_msg void OnSetParkingspacelightgroup();
	afx_msg void OnSetParkingspacelightstate();
	afx_msg void OnBnClickedButtonClear();

	void Update(int nType, void* pMsg);
	
private:	
	void InitMenu();
	void InitPictureWnd();
	void InitLoadPicturePath();
	void InitCtrl();

	void ClearCount();
	void UpdateChannelCtrl(BOOL bLogin);
	
	void ClearInfoInEventCtrl();
	void SetPictureStorePath();	
	
	// Another way to display license plate image. If DisplayLicensePlateImage fail, try this method to display license plate image
	void FillCWndWithDefaultColor(CWnd* cwnd);

	BOOL StorePicture(const char* szGUID, const char* szCfgFilePath, const char* szJpgFilePath,CAlaram* pAlarm);
	BOOL DisplayImage(const char* szGUID, const char* szJpgFilePath);	
	BOOL UpdateListCtrlInfo(const char* szGUID, StuEventInfo& stuEventInfo);

	BOOL DisplayEventImage(const char* szJPGPath);

	void ChangeUI(int nState);	
	void DisplaySelectedPicInfo(int nItemIndex, CListCtrl* listCtrl);	

private:
	CController*			m_pCtl;
	CPictureWnd				m_cPictureWnd;
	unsigned int			m_nEventCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REALLOADPICTUREDLG_H__CB9F65CD_15BE_4771_8D68_C69302319271__INCLUDED_)
