#if !defined(AFX_DHPTZMENU_H__DE4FF8BD_3232_4109_B899_952B1DC276B0__INCLUDED_)
#define AFX_DHPTZMENU_H__DE4FF8BD_3232_4109_B899_952B1DC276B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DHPtzMenu.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDHPtzMenu dialog

class CDHPtzMenu : public CDialog
{
// Construction
public:
	//Transmit main window parameter(log in handle and channel number) to the small window.
	void SetPtzParam(LLONG iHandle, int iChannel);
	CDHPtzMenu(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDHPtzMenu)
	enum { IDD = IDD_CDHPtzMenu };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDHPtzMenu)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDHPtzMenu)
	afx_msg void OnOprUp();
	afx_msg void OnOprDown();
	afx_msg void OnOprLeft();
	afx_msg void OnOprRight();
	afx_msg void OnOprOpenmenu();
	afx_msg void OnOprClosemenu();
	afx_msg void OnOprOk();
	afx_msg void OnOprCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void PtzMemuControl(DWORD dwCommand);
	int m_Channel;
	LLONG m_DeviceID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DHPTZMENU_H__DE4FF8BD_3232_4109_B899_952B1DC276B0__INCLUDED_)
