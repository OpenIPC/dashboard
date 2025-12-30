#if !defined(AFX_DLGCFGSERIAL_H__FF31020F_BECE_4E97_B47B_81D1C13BBD61__INCLUDED_)
#define AFX_DLGCFGSERIAL_H__FF31020F_BECE_4E97_B47B_81D1C13BBD61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgSerial.h : header file
//

// protocol
enum em_CommProtocol
{
	em_CommProtocol_Unknown,
	em_CommProtocol_DHAlarm,
	em_CommProtocol_Bosch,
};

typedef struct
{
	em_CommProtocol emCommProtocol;
	char*			pszName;
}Demo_CommProtocol;

const Demo_CommProtocol DemoCommProtocol[] = 
{
	{em_CommProtocol_Unknown, "Unknown"},
	{em_CommProtocol_DHAlarm, "DH-ALARM"}
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgSerial dialog

class CDlgCfgSerial : public CDialog
{
// Construction
public:
	CDlgCfgSerial(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor
	void GetSerialPara();

// Dialog Data
	//{{AFX_DATA(CDlgCfgSerial)
	enum { IDD = IDD_DLG_CFG_SERIAL };
	CComboBox	m_cbProtocol;
	CComboBox	m_cbChannel;
	CComboBox	m_cbStopBit;
	CComboBox	m_cbVerify;
	CComboBox	m_cbDataBit;
	CComboBox	m_cbBaudRate;
	CString	m_csChannel;
	CString	m_csProtocol;
	//}}AFX_DATA
	
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgSerial)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CDlgCfgSerial)
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCfgSerialCmbChn();
	afx_msg void OnSelendcancelSerialCmbProtocol();
	afx_msg void OnSelendcancelComboDatabit();
	afx_msg void OnSelendcancelComboStopbit();
	afx_msg void OnSelendcancelComboVerify();
	afx_msg void OnSelendcancelComboBaudrate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();
	BOOL GetCommConfig();
	BOOL SetCommConfig();
	
private:
	LLONG			m_lLoginId;
	CFG_COMMGROUP_INFO	m_stuComm;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGSERIAL_H__FF31020F_BECE_4E97_B47B_81D1C13BBD61__INCLUDED_)
