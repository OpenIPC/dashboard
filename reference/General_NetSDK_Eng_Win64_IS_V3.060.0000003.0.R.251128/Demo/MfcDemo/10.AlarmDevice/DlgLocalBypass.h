#if !defined(AFX_DLGLOCALBYPASS_H__B1819F7B_B5DA_4153_9F33_F3055A8A205A__INCLUDED_)
#define AFX_DLGLOCALBYPASS_H__B1819F7B_B5DA_4153_9F33_F3055A8A205A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgLocalBypass.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgLocalBypass dialog

class CDlgLocalBypass : public CDialog
{
// Construction
public:
	CDlgLocalBypass(CWnd* pParent = NULL, BOOL bNeedMode = FALSE, int nWantToSetChannelCount = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgLocalBypass)
	enum { IDD = IDD_DLG_LOCAL_BYPASS };
	CButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgLocalBypass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgLocalBypass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void StuToDlg();
	void CreateWithModeDialog();
	void CreateWithoutModeDialog();
	
	void SetLocalChannel(int* pnLocalChannel)
	{
		for (int i = 0; i < m_nLastCount; ++i)
		{
			m_pnLocalChannel[i] = pnLocalChannel[i];
		}
	}

	void SetLocalMode(NET_BYPASS_MODE* pemLocalMode)
	{
		for (int i = 0; i < m_nLastCount; ++i)
		{
			m_pemLocalMode[i] = pemLocalMode[i];
		}
	}

	void GetLocalChannel(int* pnLocalChannel)
	{
		for (int i = 0; i < m_nWantToSetChannelCount; ++i)
		{
			pnLocalChannel[i] = m_pnLocalChannel[i];
		}
	}
	
	void SetLastCount(int nLastCount)
	{
		m_nLastCount = nLastCount;
	}

	int GetActualCount(){return m_nActualCount;}

private:
	CButton cBtnChannel[16];
	CButton cBtnMode[16];
	BOOL	m_bNeedMode;
	int		m_nWantToSetChannelCount;
	int  	m_pnLocalChannel[16];
	NET_BYPASS_MODE m_pemLocalMode[16];
	int		m_nLastCount;
	int		m_nActualCount;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGLOCALBYPASS_H__B1819F7B_B5DA_4153_9F33_F3055A8A205A__INCLUDED_)
