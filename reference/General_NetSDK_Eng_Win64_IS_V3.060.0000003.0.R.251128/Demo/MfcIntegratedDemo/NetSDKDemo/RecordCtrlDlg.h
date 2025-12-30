#if !defined(AFX_RECORDCTRLDLG_H__3E4662F5_955B_474B_804A_825B94C7A51E__INCLUDED_)
#define AFX_RECORDCTRLDLG_H__3E4662F5_955B_474B_804A_825B94C7A51E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RecordCtrlDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecordCtrlDlg dialog
#define  MAX_RECORD_CHANNEL 16
class CRecordCtrlDlg : public CDialog
{
	LLONG m_DeviceId;
	int m_ChannelNum;
	char m_State[MAX_RECORD_CHANNEL];
// Construction
public:
	CRecordCtrlDlg(CWnd* pParent = NULL);   // standard constructor
	void SetDeviceId(LLONG nDeviceId);
// Dialog Data
	//{{AFX_DATA(CRecordCtrlDlg)
	enum { IDD = IDD_RECORDSTATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordCtrlDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecordCtrlDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECORDCTRLDLG_H__3E4662F5_955B_474B_804A_825B94C7A51E__INCLUDED_)
