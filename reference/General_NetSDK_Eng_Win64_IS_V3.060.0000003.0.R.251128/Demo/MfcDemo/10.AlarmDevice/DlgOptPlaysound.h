#if !defined(AFX_DLGOPTPLAYSOUND_H__647642DF_1863_4B58_AB2C_C12F0A9B21B7__INCLUDED_)
#define AFX_DLGOPTPLAYSOUND_H__647642DF_1863_4B58_AB2C_C12F0A9B21B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgOptPlaysound.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgOptPlaysound dialog

class CDlgOptPlaysound : public CDialog
{
// Construction
public:
	CDlgOptPlaysound(CWnd* pParent = NULL, LLONG hLoginID = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgOptPlaysound)
	enum { IDD = IDD_DLG_OPT_PLAYSOUND };
	CString	m_csFilePath;
	CComboBox	m_cmbAudioChannel;
	CSliderCtrl	m_slVolume;
	CString	m_edShowVolume;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgOptPlaysound)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgOptPlaysound)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnPlay();
	afx_msg void OnBtnStop();
	afx_msg void OnCustomdrawSliderVolume(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnSelchangeComboAudioChannel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void ShowInfo(int nCurSel = 0);
	LLONG					m_hLoginID;
	CFG_AUDIO_OUTPUT_VOLUME m_stuAudioOutput;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGOPTPLAYSOUND_H__647642DF_1863_4B58_AB2C_C12F0A9B21B7__INCLUDED_)
