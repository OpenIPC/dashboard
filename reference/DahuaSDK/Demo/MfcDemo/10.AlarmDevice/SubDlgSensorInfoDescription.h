#if !defined(AFX_SUBDLGSENSORINFODESCRIPTION_H__14413C1C_AE81_4B23_BD40_C914548E010E__INCLUDED_)
#define AFX_SUBDLGSENSORINFODESCRIPTION_H__14413C1C_AE81_4B23_BD40_C914548E010E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SubDlgSensorInfoDescription.h : header file
//
#include <VECTOR>
/////////////////////////////////////////////////////////////////////////////
// CSubDlgSensorInfoDescription dialog

class CSubDlgSensorInfoDescription : public CDialog
{
// Construction
public:
	CSubDlgSensorInfoDescription(CWnd* pParent = NULL, int nMaxSensor = 72, int nColumn = 8);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSubDlgSensorInfoDescription)
	enum { IDD = IDD_SUBDLG_SENSORINFO_DESCRIPTION };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSubDlgSensorInfoDescription)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSubDlgSensorInfoDescription)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void				InitDlg();
	void				SetID(const std::vector<int>& vecID);
	const std::vector<int>&	GetID();
	BOOL				IsSet(int nChannel);

private:
	CButton*			m_pbtnCheck;
	int					m_nMaxSensor;
    int                 m_nColumn;
	
	typedef enum emBtnID
	{
		emBtnID_base = 0xff0,
	};

	std::vector<int>	m_vecChannel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBDLGSENSORINFODESCRIPTION_H__14413C1C_AE81_4B23_BD40_C914548E010E__INCLUDED_)
