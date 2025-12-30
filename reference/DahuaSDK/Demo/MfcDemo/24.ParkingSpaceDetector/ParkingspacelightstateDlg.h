#pragma once
#include "afxwin.h"


// ParkingspacelightstateDlg 对话框
class CController;
class ParkingspacelightstateDlg : public CDialog
{
	DECLARE_DYNAMIC(ParkingspacelightstateDlg)

public:
	ParkingspacelightstateDlg(CController* pCtl, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ParkingspacelightstateDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_PARKINGSPACELIGHTSTATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSetstate();
	afx_msg void OnBnClickedButtonGetstate();
	CComboBox m_combo_spacefree;
	CComboBox m_combo_spacefull;
	CComboBox m_combo_netport1abort;
	CComboBox m_combo_netport2abort;

private:
	void		InitControl();			// Init dialog
	void		getConfig();			// get config
	void		setConfig();			// set config

	void		showDataToControl();	
	void		getDataFromControl();	

	void	setComboBoxInfoFromLightInfo(NET_PARKINGSPACELIGHT_INFO lightInfo, CComboBox* comBox, int nCheckID);
	void	getLightInfoFromComBox(NET_PARKINGSPACELIGHT_INFO &lightInfo, CComboBox *comBox, int nCheckID);

private:
	CController*	m_pCtl;
	NET_PARKINGSPACELIGHT_STATE_INFO	m_stuParkLightState;
};
