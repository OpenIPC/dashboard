#if !defined(AFX_DLGEXALARMHANDLER_H__65B40604_3063_481E_9A17_513E1561698F__INCLUDED_)
#define AFX_DLGEXALARMHANDLER_H__65B40604_3063_481E_9A17_513E1561698F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgExAlarmHandler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgExAlarmHandler dialog

class CDlgExAlarmHandler : public CDialog
{
// Construction
public:
	CDlgExAlarmHandler(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgExAlarmHandler)
	enum { IDD = IDD_DLG_EXALARM_HANDLER };
	CButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgExAlarmHandler)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgExAlarmHandler)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void StuToDlg();
	void DlgToStu();
	void SetAlarmOutMask(DWORD dwExAlarmOutMask[MAX_CHANNEL_COUNT])
	{
		for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			m_dwExAlarmOutMask[i] = dwExAlarmOutMask[i];
		}
	}
	
	void GetAlarmOutMask(DWORD dwExAlarmOutMask[])
	{
		for (int i = 0; i < MAX_CHANNEL_COUNT; ++i)
		{
			dwExAlarmOutMask[i] = m_dwExAlarmOutMask[i];
		}
	}
	
private:
	CButton cBtnExAlarmOut[MAX_CHANNEL_COUNT*sizeof(DWORD)*8];
	DWORD m_dwExAlarmOutMask[MAX_CHANNEL_COUNT];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEXALARMHANDLER_H__65B40604_3063_481E_9A17_513E1561698F__INCLUDED_)
