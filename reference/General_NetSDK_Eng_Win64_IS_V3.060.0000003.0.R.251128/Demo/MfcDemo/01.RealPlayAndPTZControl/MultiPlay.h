#if !defined(AFX_MULTIPLAY_H__667E5476_E09C_4F4D_8C71_C21425CE3E45__INCLUDED_)
#define AFX_MULTIPLAY_H__667E5476_E09C_4F4D_8C71_C21425CE3E45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiPlay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiPlay dialog

class CMultiPlay : public CDialog
{
// Construction
public:
	//Initial channel 
	int m_nChannel;
	//Window amount
	int m_nMultiPlay;
	//Call external to initialize dialogue box 
	void SetMultiPlayDlgInfo(int nChannelCount);

	CMultiPlay(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMultiPlay)
	enum { IDD = IDD_DIALOG_MULTIPLAY };
	CComboBox	m_comboChannel;
	CComboBox	m_comboMulti;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiPlay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultiPlay)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_nChannelCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIPLAY_H__667E5476_E09C_4F4D_8C71_C21425CE3E45__INCLUDED_)
