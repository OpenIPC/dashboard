#if !defined(AFX_CYCLECHILD_H__77E21C15_00CB_4085_A30E_DB641B277320__INCLUDED_)
#define AFX_CYCLECHILD_H__77E21C15_00CB_4085_A30E_DB641B277320__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CycleChild.h : header file
//

#include "netsdkdemodlg.h"

/////////////////////////////////////////////////////////////////////////////
// CCycleChild dialog

class CCycleChild : public CDialog
{
// Construction
public:
	CCycleChild(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCycleChild)
	enum { IDD = IDD_CYCLE_CHILD };
	CSpinButtonCtrl	m_spin;
	CListCtrl	m_childlist;
	UINT	m_cycleinterval;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCycleChild)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCycleChild)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
public:
	BOOL bHaveBadValue;

	void SetScreenNo(int scrNo);
	
	void RefreshList();
	
	void InitList(DWORD data);
	
	void InsertItem(int row, CString item);
	
	void SetItemText(int row, int column, CString text);
	
	void SetItemData(int row, DWORD data);
	
	int GetCurRow();
	
	int GetSelectionMark();
	
	DWORD GetItemData(int row);
	
	CString GetItemText(int row, int column);
	
	void DeleteItem(int index);
	
	void OnOK();
	
	void OnCancel();
	void DoCycle();

private:
	int m_screenNo;
	int m_curRow;
	
	SplitInfoNode m_SplitInfo;
	SplitCycleParam *m_cmParam;
	CNetSDKDemoDlg *m_pMain;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CYCLECHILD_H__77E21C15_00CB_4085_A30E_DB641B277320__INCLUDED_)
