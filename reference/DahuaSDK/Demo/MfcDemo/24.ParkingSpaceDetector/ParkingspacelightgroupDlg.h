#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// ParkingspacelightgroupDlg 对话框

class CController;
class ParkingspacelightgroupDlg : public CDialog
{
	DECLARE_DYNAMIC(ParkingspacelightgroupDlg)

public:
	ParkingspacelightgroupDlg(CController* pCtl, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~ParkingspacelightgroupDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_PARKINGSPACELIGHTGROUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonGet();
	afx_msg void OnBnClickedButtonSet();
	CListCtrl m_listLightGroup;
	afx_msg void OnNMDblclkListLightgroupList(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_edit;
	afx_msg void OnEnKillfocusEdit2();

private:
	void	InitControl();		//初始化界面
	void	getConfig();		//获取配置
	void	setConfig();		//下发配置

	void	getDataFromControl();	//从控件读取数据
	void	showDataToControl();	//将数据显示到控件上

private:
	int										m_row;
	int										m_column;
	CController*							m_pCtl;

	int										m_nCfgNum;
	int										m_nLanesNum;
	CFG_PARKING_SPACE_LIGHT_GROUP_INFO_ALL	m_stuInfo;

};
