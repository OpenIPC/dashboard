#pragma once
#include "afxwin.h"

class CPictureShow : public CStatic
{
	DECLARE_DYNAMIC(CPictureShow)

public:
	CPictureShow();
	virtual ~CPictureShow();

public:
	void SetImageFile(LPCSTR szImagePath);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();

public:
	IPicture *m_pPicture;
};
