#if !defined(_LATTICESCREENDLG2_)
#define _LATTICESCREENDLG2_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxcmn.h"
#include <string>

// LatticeScreenDlg2 对话框
enum LATTICE_SCREEN_ACT_TYPE
{
	ACT_UNKNOWN = -1,
	ACT_ADD,
	ACT_MODIFY,
};

//emTextType
const char* const szScreenShowTextType[] = {"Ordinary", "Local time", "Qr code", "RESOURCE"};
const char* const szBroadCastTextType[] = {"Ordinary", "Plate number", "Time", "Number string"};
//emTextColor
const char* const szTextColor[] = {"Green", "Red", "Yellow", "White"};
//emTextRollMode
const char* const szTextRollMode[] = {"No scrolling", "Scroll left and right", "Page up and down"};

int String2Index(CString cstrText, const char* const szText[], int count);

class LatticeScreenDlg2 : public CDialog
{
	DECLARE_DYNAMIC(LatticeScreenDlg2)

public:
	LatticeScreenDlg2(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~LatticeScreenDlg2();

// 对话框数据
	enum { IDD = IDD_DIALOG_LATTICESCREEN2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl m_ListScreenShow;
	CListCtrl m_ListBroadCast;
	int m_nSeletedRowS;
	int m_nSeletedRowB;

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonScreenshowAdd();
	afx_msg void OnBnClickedButtonIssue();
	afx_msg void OnBnClickedButtonScreenshowModify();
	afx_msg void OnNMClickListScreenshow(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonScreenshowDelete();
	afx_msg void OnBnClickedButtonBroadcastAdd();

	afx_msg void OnNMClickListBroadcast(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonBroadcastModify();
	afx_msg void OnBnClickedButtonBroadcastDelete();
};

#endif //_LATTICESCREENDLG2_
