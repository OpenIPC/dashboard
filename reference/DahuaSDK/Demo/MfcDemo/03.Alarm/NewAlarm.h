#if !defined(AFX_NEWALARM_H__FEE5DB07_D167_48FA_A5D1_68B33681F1CF__INCLUDED_)
#define AFX_NEWALARM_H__FEE5DB07_D167_48FA_A5D1_68B33681F1CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewAlarm.h : header file
//
#include "alarmInfo.h"
#include "DlgChn.h"
/////////////////////////////////////////////////////////////////////////////
// CNewAlarm dialog

class CNewAlarm : public CDialog
{
// Construction
public:
	CNewAlarm(CWnd* pParent = NULL);   // standard constructor
	void SetAlarmInfo(BYTE *dwAlarm, CDlgChn* dlgAlarm, int nAlarmCount);
	void SetSoundAlarmInfo(DH_NEW_SOUND_ALARM_STATE &stuSoundAlarm,DWORD dwID);
	void SetCheckBox(int nChannelCount, int nInputAlarmCount);
	void SetAlarmDecoderAlarm(ALARM_DECODER_ALARM & stuAlarmDecoderAlarm);
	void SetAlarmAlarmArmDisarmState(ALARM_ARM_DISARM_STATE_INFO & stuAlarmArmDisarmstate);
// Dialog Data
	//{{AFX_DATA(CNewAlarm)
	enum { IDD = IDD_DIALOG_NEW_ALARM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewAlarm)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void CleanColum(UINT uID, int nLen);
public:
	CDlgChn m_dlgShelter;
	CDlgChn m_dlgDiskError;
	CDlgChn m_dlgDiskFull;
//	CDlgChn m_dlgCoder;
	CDlgChn m_dlgDynamic;
	CDlgChn m_dlgVideoLost;
//	CDlgChn m_dlgStatic;
	CDlgChn m_dlgOutput;
//	CDlgChn m_dlgUrgency;
//	CDlgChn m_dlgWireless;

	void CleanAllInfo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWALARM_H__FEE5DB07_D167_48FA_A5D1_68B33681F1CF__INCLUDED_)
