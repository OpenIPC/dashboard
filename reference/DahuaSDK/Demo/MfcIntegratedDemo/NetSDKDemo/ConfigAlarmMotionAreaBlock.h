#if !defined(AFX_CONFIGALARMMOTIONAREABLOCK_H__0B2DC7D3_494D_49F0_B752_9542B28C0472__INCLUDED_)
#define AFX_CONFIGALARMMOTIONAREABLOCK_H__0B2DC7D3_494D_49F0_B752_9542B28C0472__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigAlarmMotionAreaBlock.h : header file


//MESSAGE FOR CHANGE COLOR

#define		MSG_SELETED					WM_USER + 2007
#define		MSG_SHOW					WM_USER + 2008

/////////////////////////////////////////////////////////////////////////////
// CConfigAlarmMotionAreaBlock dialog

class CConfigAlarmMotionAreaBlock : public CWnd
{
// Construction
public:
	CConfigAlarmMotionAreaBlock();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigAlarmMotionAreaBlock)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigAlarmMotionAreaBlock)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL
	afx_msg LRESULT OnSeleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShow(WPARAM wParam, LPARAM lParam);
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigAlarmMotionAreaBlock)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL IsSeleted(){return m_bSelected;}
	void SetPos(BYTE x, BYTE y);
	void SetStatus(BOOL selected){m_bSelected = selected;}
	
private:
	CBrush m_whiteBrush;
	CBrush m_blueBrush;
	CBrush *m_brush;
	BOOL   m_bSelected;
	BYTE   m_byX;
	BYTE   m_byY;

	CRect m_rect;
	CDC *m_thisDC;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGALARMMOTIONAREABLOCK_H__0B2DC7D3_494D_49F0_B752_9542B28C0472__INCLUDED_)
