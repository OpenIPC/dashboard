#include "afxwin.h"
#if !defined(_OPENDOORGROUP_)
#define _OPENDOORGROUP_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class OpenDoorGroup : public CDialog
{
	DECLARE_DYNAMIC(OpenDoorGroup)

public:
	OpenDoorGroup(CWnd* pParent = NULL);
	virtual ~OpenDoorGroup();
	enum { IDD = IDD_DIALOG_OPENDOORGROUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOpendoorgroupButSet();
	afx_msg void OnBnClickedOpendoorgroupButGet();
	DECLARE_MESSAGE_MAP()
private:
	void InitDlg();
	void SetOpenDoorGroupInfoToCtrl();
	void GetOpenDoorGroupInfoFromCtrl();
	CFG_OPEN_DOOR_GROUP_INFO stuInfo;
public:
	CComboBox m_comUserNO1;
	CComboBox m_comUserNO2;
	CComboBox m_comUserNO3;
	CComboBox m_comUserNO4;
	CComboBox m_comOpenDoorMethod1;
	CComboBox m_comOpenDoorMethod2;
	CComboBox m_comOpenDoorMethod3;
	CComboBox m_comOpenDoorMethod4;
	afx_msg void OnCbnSelchangeOpendoorgroupCom0Userno1();
	afx_msg void OnCbnSelchangeOpendoorgroupCom0Userno2();
	afx_msg void OnCbnSelchangeOpendoorgroupCom0Userno3();
	afx_msg void OnCbnSelchangeOpendoorgroupCom0Userno4();
	CComboBox m_cmbChannel;
};
#endif