// LatticeScreenDlg.cpp :
//

#include "stdafx.h"
#include "EntranceAccessDevices.h"
#include "LatticeScreenDlg.h"

IMPLEMENT_DYNAMIC(LatticeScreenDlg, CDialog)

LatticeScreenDlg::LatticeScreenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LatticeScreenDlg::IDD, pParent)
{

}

LatticeScreenDlg::~LatticeScreenDlg()
{

}

void LatticeScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_USERTYPE, m_combo_usertype);
	DDX_Control(pDX, IDC_COMBO_STATUS, m_combo_status);
}


BEGIN_MESSAGE_MAP(LatticeScreenDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SET, &LatticeScreenDlg::OnBnClickedButtonSet)
	ON_CBN_SELCHANGE(IDC_COMBO_STATUS, &LatticeScreenDlg::OnCbnSelchangeComboStatus)
END_MESSAGE_MAP()

BOOL LatticeScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	m_combo_usertype.AddString(ConvertString("monthlyCardUser"));
	m_combo_usertype.AddString(ConvertString("yearlyCardUser"));
	m_combo_usertype.AddString(ConvertString("longTimeUser"));
	m_combo_usertype.AddString(ConvertString("casualUser"));
	m_combo_usertype.SetCurSel(0);

	m_combo_status.AddString(ConvertString("CarPass"));
	m_combo_status.AddString(ConvertString("CarFree"));
	m_combo_status.SetCurSel(0);

	GetDlgItem(IDC_EDIT_PLATENUMBER)->SetWindowText("");
	GetDlgItem(IDC_EDIT_OWNEROFCAR)->SetWindowText("SSS");
	GetDlgItem(IDC_EDIT_REMAINDAY)->SetWindowText("123");
	GetDlgItem(IDC_EDIT_PARKTIME)->SetWindowText("SSS");
	GetDlgItem(IDC_EDIT_PARKCHARGE)->SetWindowText("SSS");
	GetDlgItem(IDC_EDIT_SUBUSERTYPE)->SetWindowText("SSS");
	GetDlgItem(IDC_EDIT_REMARKS)->SetWindowText("SSS");
	GetDlgItem(IDC_EDIT_REMAINSPACE)->SetWindowText("SSS");
	GetDlgItem(IDC_EDIT_CUSTOM)->SetWindowText("SSS");

	((CButton*)GetDlgItem(IDC_RADIO_PASS))->SetCheck(1);
	OnCbnSelchangeComboStatus();

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void LatticeScreenDlg::OnBnClickedButtonSet()
{
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);

	NET_CTRL_SET_PARK_INFO stuCtrlSetParkInfo = {sizeof(stuCtrlSetParkInfo)};
	GetDlgItemText(IDC_EDIT_PLATENUMBER,stuCtrlSetParkInfo.szPlateNumber,MAX_PLATE_NUMBER_LEN);
	stuCtrlSetParkInfo.nParkTime = GetDlgItemInt(IDC_EDIT_PARKTIME);
	GetDlgItemText(IDC_EDIT_OWNEROFCAR,stuCtrlSetParkInfo.szMasterofCar,MAX_MASTER_OF_CAR_LEN);
	int nUserType = ((CComboBox*)GetDlgItem(IDC_COMBO_USERTYPE))->GetCurSel();
	switch(nUserType)
	{
	case 0:
		memcpy(stuCtrlSetParkInfo.szUserType,"monthlyCardUser",15);
		break;
	case 1:
		memcpy(stuCtrlSetParkInfo.szUserType,"yearlyCardUser",14);
		break;
	case 2:
		memcpy(stuCtrlSetParkInfo.szUserType,"longTimeUser",12);
		break;
	case 3:
		memcpy(stuCtrlSetParkInfo.szUserType,"casualUser",10);
		break;
	default:
		break;
	}
	stuCtrlSetParkInfo.nRemainDay = GetDlgItemInt(IDC_EDIT_REMAINDAY);
	GetDlgItemText(IDC_EDIT_PARKCHARGE,stuCtrlSetParkInfo.szParkCharge,MAX_PARK_CHARGE_LEN);
	stuCtrlSetParkInfo.nRemainSpace = GetDlgItemInt(IDC_EDIT_REMAINSPACE);
	stuCtrlSetParkInfo.nPassEnable = ((CButton*)GetDlgItem(IDC_RADIO_PASS))->GetCheck();
	
	CTime time;
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_IN1))->GetTime(time);
	stuCtrlSetParkInfo.stuInTime.dwYear = time.GetYear();
	stuCtrlSetParkInfo.stuInTime.dwMonth = time.GetMonth();
	stuCtrlSetParkInfo.stuInTime.dwDay = time.GetDay();
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_IN2))->GetTime(time);
	stuCtrlSetParkInfo.stuInTime.dwHour = time.GetHour();
	stuCtrlSetParkInfo.stuInTime.dwMinute = time.GetMinute();
	stuCtrlSetParkInfo.stuInTime.dwSecond = time.GetSecond();
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_OUT1))->GetTime(time);
	stuCtrlSetParkInfo.stuOutTime.dwYear = time.GetYear();
	stuCtrlSetParkInfo.stuOutTime.dwMonth = time.GetMonth();
	stuCtrlSetParkInfo.stuOutTime.dwDay = time.GetDay();
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_OUT2))->GetTime(time);
	stuCtrlSetParkInfo.stuOutTime.dwHour = time.GetHour();
	stuCtrlSetParkInfo.stuOutTime.dwMinute = time.GetMinute();
	stuCtrlSetParkInfo.stuOutTime.dwSecond = time.GetSecond();
	int nStatus = ((CComboBox*)GetDlgItem(IDC_COMBO_STATUS))->GetCurSel();
	stuCtrlSetParkInfo.emCarStatus = (EM_CARPASS_STATUS)(nStatus+1);
	GetDlgItemText(IDC_EDIT_CUSTOM,stuCtrlSetParkInfo.szCustom,MAX_CUSTOM_LEN);
	GetDlgItemText(IDC_EDIT_SUBUSERTYPE,stuCtrlSetParkInfo.szSubUserType,MAX_SUB_USER_TYPE_LEN);
	GetDlgItemText(IDC_EDIT_REMARKS,stuCtrlSetParkInfo.szRemarks,MAX_REMARKS_LEN);

	BOOL bret = Device::GetInstance().SetLatticeScreen(DH_CTRL_SET_PARK_INFO,&stuCtrlSetParkInfo);
	if (!bret)
	{
		MessageBox(ConvertString(ShowErr(Device::GetInstance().GetDeviceErr())),ConvertString("Prompt"));
	}

	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);
}

void LatticeScreenDlg::OnCbnSelchangeComboStatus()
{
	GetDlgItem(IDC_EDIT_PLATENUMBER)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_OWNEROFCAR)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO_USERTYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_REMAINDAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATETIMEPICKER_IN1)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATETIMEPICKER_IN2)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATETIMEPICKER_OUT1)->EnableWindow(FALSE);
	GetDlgItem(IDC_DATETIMEPICKER_OUT2)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_PARKTIME)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_PARKCHARGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_NOPASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIO_PASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_SUBUSERTYPE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_REMARKS)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_REMAINSPACE)->EnableWindow(FALSE);
	int nStatus = ((CComboBox*)GetDlgItem(IDC_COMBO_STATUS))->GetCurSel();
	if (nStatus == 0)
	{
		GetDlgItem(IDC_EDIT_PLATENUMBER)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_OWNEROFCAR)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_USERTYPE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_REMAINDAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_DATETIMEPICKER_IN1)->EnableWindow(TRUE);
		GetDlgItem(IDC_DATETIMEPICKER_IN2)->EnableWindow(TRUE);
		GetDlgItem(IDC_DATETIMEPICKER_OUT1)->EnableWindow(TRUE);
		GetDlgItem(IDC_DATETIMEPICKER_OUT2)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PARKTIME)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PARKCHARGE)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_NOPASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_PASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SUBUSERTYPE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_REMARKS)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_CUSTOM)->EnableWindow(TRUE);
	} 
	else
	{
		GetDlgItem(IDC_EDIT_REMAINSPACE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_CUSTOM)->EnableWindow(TRUE);	
	}
}
