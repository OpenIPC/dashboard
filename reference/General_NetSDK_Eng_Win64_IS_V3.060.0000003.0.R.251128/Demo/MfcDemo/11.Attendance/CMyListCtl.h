#ifndef __CMYLISTCTL_H__
#define __CMYLISTCTL_H__
#pragma once
#include "afxcmn.h"

class CMyListCtl:public CListCtrl
{
public:
	CMyListCtl();
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	DECLARE_MESSAGE_MAP()
//	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
};
#endif