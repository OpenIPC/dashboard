#if !defined(AFX_TABALARMDLG_H__091EEEF3_EF29_4370_BB87_DA2AF0711203__INCLUDED_)
#define AFX_TABALARMDLG_H__091EEEF3_EF29_4370_BB87_DA2AF0711203__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabAlarmDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabAlarmDlg dialog

class CTabAlarmDlg : public CDialog
{
// Construction
public:
	//Display HDD malfunction error  
	void SetDiskErrorAlarmInfo(DWORD dwDiskError);
	//HDD insufficient capacity alarm 
	void SetDiskFullAlarmInfo(BOOL bDiskFull);
	//Display audio alarm 
	void SetSoundAlarmInfo(WORD wSoundAlarm);
	//Camera masking alarm 
	void SetShelterAlarmInfo(WORD wVideoShelter);
	//Display general alarm 
	void SetCommonAlarmInfo(WORD wInputAlarm,WORD wVideoLost,WORD wMotionAlarm,int nChannelCount,int nInputAlarmCount);
	void CleanAllInfo(void);
	CTabAlarmDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTabAlarmDlg)
	enum { IDD = IDD_DIALOG_NEW_ALARM1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabAlarmDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTabAlarmDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void SetCheckBox(int nChannelCount,int nInputAlarmCount);
	void CleanColum(UINT uID, int nLen);
	int m_nInputAlarmCount;
	int m_nChannelCount;
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABALARMDLG_H__091EEEF3_EF29_4370_BB87_DA2AF0711203__INCLUDED_)
