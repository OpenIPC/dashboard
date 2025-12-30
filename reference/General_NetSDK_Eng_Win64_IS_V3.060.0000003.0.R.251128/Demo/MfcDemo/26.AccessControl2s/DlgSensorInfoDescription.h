#if !defined(_DLGSENSORINFODESCRIPTION_)
#define _DLGSENSORINFODESCRIPTION_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <VECTOR>
class DlgSensorInfoDescription : public CDialog
{
	DECLARE_DYNAMIC(DlgSensorInfoDescription)

public:
	DlgSensorInfoDescription(CWnd* pParent = NULL, int nMaxSensor = 72);
	virtual ~DlgSensorInfoDescription();

	enum { IDD = IDD_SENSORINFO_DESCRIPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
public:
	void				InitDlg();
	void				SetID(const std::vector<int>& vecID);
	const std::vector<int>&	GetID();
	BOOL				IsSet(int nChannel);
private:
	CButton*			m_pbtnCheck;
	int					m_nMaxSensor;
	typedef enum emBtnID
	{
		emBtnID_base = 0xff0,
	};
	std::vector<int>	m_vecChannel;
};
#endif