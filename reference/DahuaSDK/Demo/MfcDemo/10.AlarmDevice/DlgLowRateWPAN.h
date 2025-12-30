#if !defined(AFX_DLGLOWRATEWPAN_H__59A9FDE0_CA93_4E28_900F_AE5106398AC3__INCLUDED_)
#define AFX_DLGLOWRATEWPAN_H__59A9FDE0_CA93_4E28_900F_AE5106398AC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgLowRateWPAN.h : header file
//

#define WM_LOWRATEWPAN_INFO_NOTIFY  (WM_USER+456)

typedef struct
{
	NET_WIRELESS_DEVICE_TYPE	emDeviceType;
	char*						pszName;
}Demo_CodeID_Type;

const Demo_CodeID_Type stuType[] = 
{
	{NET_WIRELESS_DEVICE_TYPE_UNKNOWN, "Unknown"},
	{NET_WIRELESS_DEVICE_TYPE_KEYBOARD, "KeyBoard"},
	{NET_WIRELESS_DEVICE_TYPE_DEFENCE, "Defence"},
	{NET_WIRELESS_DEVICE_TYPE_REMOTECONTROL, "RemoteControl"},
	{NET_WIRELESS_DEVICE_TYPE_MAGNETOMER, "Magnetomer"},
};


const CString stuError[] = {"Right","AlarmExist","Other"};

/////////////////////////////////////////////////////////////////////////////
// CDlgLowRateWPAN dialog

class CDlgLowRateWPAN : public CDialog
{
// Construction
public:
	CDlgLowRateWPAN(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor
	void LowRateWPANDataCallBack(LLONG lLoginID, LLONG lAttachHandle, NET_CODEID_INFO* stuBuf, NET_CODEID_ERROR_TYPE emError);
// Dialog Data
	//{{AFX_DATA(CDlgLowRateWPAN)
	enum { IDD = IDD_DLG_LOWRATEWPAN };
	CListCtrl	m_listCodeIDList;
	CEdit	m_edtName;
	CComboBox	m_cmbType;
	CComboBox	m_cmbIndex;
	CButton	m_ckEnable;
	int		m_nID;
	int		m_nQueryNum; // [0, 100]
	int		m_nStartIndex; // [0, 200]
	int		m_nTotalCodeIDNum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLowRateWPAN)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgLowRateWPAN)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCmbLowRateWPanIndex();
	afx_msg void OnDestroy();
	afx_msg void OnLowRateWPanBtnQuery();
	afx_msg void OnLowRateWPanBtnModify();
	afx_msg void OnLowRateWPanBtnRemove();
	afx_msg void OnLowRateWPanBtnRemoveall();
	afx_msg void OnLowRateWPanBtnAttach();
	afx_msg void OnLowRateWPanBtnDetach();
    afx_msg LRESULT OnLowRateWPanNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLowRateWPanBtnTotal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void InitDlg();
    void ShowCodeID(const NET_CODEID_INFO& stuInfo);
    void InsertCodeID(const NET_CODEID_INFO& stuInfo);
    void ClearCodeID();

private:
	LLONG					m_lLoginId;     // µÇÂ¼¾ä±ú
	LLONG					m_lHandle;      // ¶©ÔÄ¾ä±ú
	int						m_nListIndex;   // ¶©ÔÄÐÅÏ¢´°¿ÚÐòºÅ
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLOWRATEWPAN_H__59A9FDE0_CA93_4E28_900F_AE5106398AC3__INCLUDED_)
