#if !defined(AFX_DLGANALOGALARMCHANNELS_H__A135FDF5_1F99_4BCC_80F1_8B9C32C1974D__INCLUDED_)
#define AFX_DLGANALOGALARMCHANNELS_H__A135FDF5_1F99_4BCC_80F1_8B9C32C1974D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgAnalogAlarmChannels.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgAnalogAlarmChannels dialog

class CDlgAnalogAlarmChannels : public CDialog
{
// Construction
public:
	CDlgAnalogAlarmChannels(CWnd* pParent = NULL, LLONG lLoginId = 0);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgAnalogAlarmChannels)
	enum { IDD = IDD_DLG_ANALOGALARMCHANNELS };
	CListCtrl	m_listAnalogChannelMap;
	int		m_edtMaxCount;
	int		m_edtRetCount;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgAnalogAlarmChannels)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgAnalogAlarmChannels)
	virtual BOOL OnInitDialog();
	afx_msg void OnAnalogalarmchannelsBtnGet();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	BOOL		ApplyMemory();
	void		ShowInfo();
	BOOL		getInfoFromDevice();
	LLONG						m_lLoginId;
	NET_ANALOGALARM_CHANNELS	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGANALOGALARMCHANNELS_H__A135FDF5_1F99_4BCC_80F1_8B9C32C1974D__INCLUDED_)
