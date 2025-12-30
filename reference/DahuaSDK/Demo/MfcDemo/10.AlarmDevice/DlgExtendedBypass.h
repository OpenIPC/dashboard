#if !defined(AFX_DLGEXTENDEDBYPASS_H__14C7937C_3EEA_413A_9C1F_605FF6CDD6EB__INCLUDED_)
#define AFX_DLGEXTENDEDBYPASS_H__14C7937C_3EEA_413A_9C1F_605FF6CDD6EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgExtendedBypass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgExtendedBypass dialog

class CDlgExtendedBypass : public CDialog
{
// Construction
public:
	CDlgExtendedBypass(CWnd* pParent = NULL, BOOL bNeedMode = FALSE, int nWantToSetChannelCount = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgExtendedBypass)
	enum { IDD = IDD_DLG_EXTENDED_BYPASS };
	CButton	m_btnOk;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgExtendedBypass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgExtendedBypass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void StuToDlg();
	void DlgToStu();
	void CreateWithModeDialog();
	void CreateWithoutModeDialog();

	void SetExtendedChannel(int* pnExtendedChannel)
	{
		for (int i = 0; i < m_nLastCount; ++i)
		{
			m_nExtendedChannel[i] = pnExtendedChannel[i];
		}
	}	

	void SetExtendedMode(NET_BYPASS_MODE* pemExtendedMode)
	{
		for (int i = 0; i < m_nLastCount; ++i)
		{
			m_emExtendedMode[i] = pemExtendedMode[i];
		}
	}

	void GetExtendedChannnel(int* pnExtendedChannel)
	{
		for (int i = 0; i < m_nWantToSetChannelCount; ++i)
		{
			pnExtendedChannel[i] = m_nExtendedChannel[i];
		}
	}
	
	void SetLastCount(int nLastCount)
	{
		m_nLastCount = nLastCount;
	}

	int GetActualCount(){return m_nActualCount;}
private:
	CButton cBtnExtendedChannel[MAX_CHANNEL_COUNT*sizeof(DWORD)*8];
	CButton cBtnExtendedMode[MAX_CHANNEL_COUNT*sizeof(DWORD)*8];
	int		m_nExtendedChannel[MAX_CHANNEL_COUNT*sizeof(DWORD)*8];
	NET_BYPASS_MODE m_emExtendedMode[MAX_CHANNEL_COUNT*sizeof(DWORD)*8];
	BOOL	m_bNeedMode;
	int		m_nWantToSetChannelCount;
	int		m_nLastCount;
	int		m_nActualCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEXTENDEDBYPASS_H__14C7937C_3EEA_413A_9C1F_605FF6CDD6EB__INCLUDED_)
