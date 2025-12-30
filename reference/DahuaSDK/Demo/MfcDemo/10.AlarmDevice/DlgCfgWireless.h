#if !defined(AFX_DLGCFGWIRELESS_H__42773A73_800D_4806_BF32_BBF1EDCBC9C3__INCLUDED_)
#define AFX_DLGCFGWIRELESS_H__42773A73_800D_4806_BF32_BBF1EDCBC9C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCfgWireless.h : header file
//

// APN
typedef struct
{
	EM_CFG_APN	emAPN;
	char*		pszName;
}Demo_APN;

const Demo_APN stuDemoAPN[] = 
{
	{EM_CFG_APN_CTNET, "CTNET"},
};

// FluxTactic
typedef struct
{
	EM_CFG_DAY3GFLUXTACTIC	emTactic;
	char*					pszName;
}Demo_FluxTactic;

const Demo_FluxTactic stuDemoFluxTactic[] = 
{
	{EM_CFG_DAY3GFLUXTACTIC_BYFLUX, "ByFlux"},
	{EM_CFG_DAY3GFLUXTACTIC_BYTIME, "ByTime"},
};

// FluxAction
typedef struct
{
	EM_CFG_DAY3GFLUXACTION	emAction;
	char*					pszName;
}Demo_FluxAction;

const Demo_FluxAction stuDemoFluxAction[] = 
{
	{EM_CFG_DAY3GFLUXACTION_NOTHING, "Nothing"},
	{EM_CFG_DAY3GFLUXACTION_3GNETDOWN, "3GNetDown "},
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCfgWireless dialog

class CDlgCfgWireless : public CDialog
{
	// Construction
public:
	CDlgCfgWireless(CWnd* pParent = NULL, LLONG lLoginID = 0);   // standard constructor
	
	
	// Dialog Data
	//{{AFX_DATA(CDlgCfgWireless)
	enum { IDD = IDD_DLG_CFG_WIRELESS };
	CButton	m_chkEnable;
	CComboBox	m_cmbFluxTactic;
	CComboBox	m_cmbFluxAction;
	CComboBox	m_cmbAPN;
	//}}AFX_DATA
	
	
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCfgWireless)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	
	// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CDlgCfgWireless)
	virtual BOOL OnInitDialog();
	afx_msg void OnSet();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
		
private:
	void InitDlg();
	void StuToDlg();
	void DlgToStu();	
	BOOL GetCurrentStrategy();
	
private:
	LLONG				m_lLoginId;
	CFG_WIRELESS_INFO	m_stuInfo;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCFGWIRELESS_H__42773A73_800D_4806_BF32_BBF1EDCBC9C3__INCLUDED_)
