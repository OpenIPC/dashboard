#if !defined(AFX_CHANGEPASSWORD_H__8C398F68_BFE8_4621_9DD0_D7158E3B5860__INCLUDED_)
#define AFX_CHANGEPASSWORD_H__8C398F68_BFE8_4621_9DD0_D7158E3B5860__INCLUDED_

#include "ShowPicture.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangePassword.h : header file
//

#include "../../../Include/Common/dhnetsdk.h"

#define MAX_QRCODE_LEN	1024
#define MAX_FILE_LEN	260

/////////////////////////////////////////////////////////////////////////////
// CChangePassword dialog

#define OUT_FILE_PIXEL_PRESCALER	4											// Prescaler (number of pixels in bmp file for each QRCode pixel, on each dimension)
#define PIXEL_COLOR_R				0											// Color of bmp pixels
#define PIXEL_COLOR_G				0
#define PIXEL_COLOR_B				0

class CResetPassword : public CDialog
{
// Construction
public:
	CResetPassword(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangePassword)
	enum { IDD = IDD_RESTPWD_DIALOG };

	CString	m_strSecuityCode;
	CString	m_strNewPwd;
	CString	m_strConfirmPwd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangePassword)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangePassword)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	int ShowQrCode();

public:
	char m_cQrCode[MAX_QRCODE_LEN];
    bool MyQRGenerator(char *szSourceSring);
	CShowPicture  *m_showPicture;
	char m_fileName[MAX_FILE_LEN];
	afx_msg void OnBnClickedOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void SetResetPwdParam(char* szMac, unsigned int nMacLen, char* szUserName, unsigned int nNameLen, BYTE byInitStaus);

private:
	int ResetPwd();

private:
	char m_szMac[DH_MACADDR_LEN];
	char m_szUserName[DH_USER_NAME_LENGTH_EX];

	// Init status
	BYTE m_byInitStaus;
	// Notify the device whether overseas or not
	bool m_bOverseas;
public:
	void SetQRCodeScanDescription(bool bOverseas);
private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEPASSWORD_H__8C398F68_BFE8_4621_9DD0_D7158E3B5860__INCLUDED_)
