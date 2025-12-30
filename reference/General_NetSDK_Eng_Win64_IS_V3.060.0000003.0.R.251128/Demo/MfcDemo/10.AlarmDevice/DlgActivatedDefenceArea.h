#if !defined(AFX_DLGACTIVATEDDEFENCEAREA_H__9B9B66B9_322C_4FE6_84C4_D83A72103AC2__INCLUDED_)
#define AFX_DLGACTIVATEDDEFENCEAREA_H__9B9B66B9_322C_4FE6_84C4_D83A72103AC2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgActivatedDefenceArea.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgActivatedDefenceArea dialog

class CDlgActivatedDefenceArea : public CDialog
{
// Construction
public:
	CDlgActivatedDefenceArea(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgActivatedDefenceArea)
	enum { IDD = IDD_DLG_ACTIVATEDDEFENCEAREA };
	CListCtrl	m_strListExAlarmIn;
	CListCtrl	m_strListAlarmIn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgActivatedDefenceArea)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgActivatedDefenceArea)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnActivateddefenceareaBtnGet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	BOOL getInfoFromDevice();
	BOOL getInfo();
	void showInfo();
	void GetTimeStringByStruct(NET_TIME stuTime, char *pTime);

private:
	LLONG					 m_lLoginId;
	NET_ACTIVATEDDEFENCEAREA m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGACTIVATEDDEFENCEAREA_H__9B9B66B9_322C_4FE6_84C4_D83A72103AC2__INCLUDED_)
