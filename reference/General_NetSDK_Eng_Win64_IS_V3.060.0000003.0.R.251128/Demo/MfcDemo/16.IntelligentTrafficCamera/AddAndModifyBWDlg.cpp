// AddAndModifyBWDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelligentTrafficCamera.h"
#include "AddAndModifyBWDlg.h"

// CAddAndModifyBWDlg Dialog

IMPLEMENT_DYNAMIC(CAddAndModifyBWDlg, CDialog)

CAddAndModifyBWDlg::CAddAndModifyBWDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAddAndModifyBWDlg::IDD, pParent)
{
	m_lLoginHandle = 0;
	memset(&m_stTrafficListInfo, 0, sizeof(NET_TRAFFIC_LIST_RECORD));
	m_stTrafficListInfo.dwSize = sizeof(NET_TRAFFIC_LIST_RECORD);
	m_emTrafficOperatorType = EM_ADD_BACKLIST;

}

CAddAndModifyBWDlg::~CAddAndModifyBWDlg()
{
}

void CAddAndModifyBWDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_PLATETYPE, m_cbPlateType);
	DDX_Control(pDX, IDC_COMBO_PLATECOLOR, m_cbPlateColor);
	DDX_Control(pDX, IDC_COMBO_VEHICLETYPE, m_cbVehicleType);
	DDX_Control(pDX, IDC_COMBO_VEHICLECOLOR, m_cbVehicleColor);
	DDX_Control(pDX, IDC_COMBO_CONTROLTYPE, m_cbControlType);
}


BEGIN_MESSAGE_MAP(CAddAndModifyBWDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OK, &CAddAndModifyBWDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCLE, &CAddAndModifyBWDlg::OnBnClickedBtnCancle)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_START_DATE, &CAddAndModifyBWDlg::OnDtnDatetimechangeDatetimepickerStartDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_END_DATE, &CAddAndModifyBWDlg::OnDtnDatetimechangeDatetimepickerEndDate)
END_MESSAGE_MAP()


// CAddAndModifyBWDlg message handlers

BOOL CAddAndModifyBWDlg::PreTranslateMessage(MSG* pMsg)
{
	// Enter key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}

	// Escape key
	if(pMsg->message == WM_KEYDOWN &&
		pMsg->wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CAddAndModifyBWDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	SetDlgItemText(IDC_EDIT_PLATE_NUMBER,m_stTrafficListInfo.szPlateNumber);
	SetDlgItemText(IDC_EDIT_OWNER,m_stTrafficListInfo.szMasterOfCar);


	for (int i = 0; i < sizeof(nPlateTypeList)/sizeof(int); i++)
	{	
		m_cbPlateType.InsertString(i, ConvertString(strPlateTypeList[i]));
	}
	m_cbPlateType.SetCurSel(0);

	for (int i = 0; i < sizeof(nPlateColorTypeList)/sizeof(int); i++)
	{	
		m_cbPlateColor.InsertString(i, ConvertString(strPlateColorTypeList[i]));
	}
	m_cbPlateColor.SetCurSel(0);

	for (int i = 0; i < sizeof(nVehtcleTypeList)/sizeof(int); i++)
	{	
		m_cbVehicleType.InsertString(i, ConvertString(strVehtcleTypeList[i]));
	}
	m_cbVehicleType.SetCurSel(0);

	for (int i = 0; i < sizeof(nVehtcleColorTypeList)/sizeof(int); i++)
	{	
		m_cbVehicleColor.InsertString(i, ConvertString(strVehtcleColorTypeList[i]));
	}
	m_cbVehicleColor.SetCurSel(0);

	for (int i = 0; i < sizeof(nCarControlTypeList)/sizeof(int); i++)
	{	
		m_cbControlType.InsertString(i, ConvertString(strCarControlTypeList[i]));
	}
	m_cbControlType.SetCurSel(0);

	CString strWndindowName;
	switch (m_emTrafficOperatorType)
	{
	case EM_ADD_BACKLIST:
		strWndindowName = ConvertString("Add Block List");
		GetDlgItem(IDC_STATIC_CT)->ShowWindow(TRUE);
		GetDlgItem(IDC_COMBO_CONTROLTYPE)->ShowWindow(TRUE);
		break;
	case EM_ADD_ALLOWLIST:
		strWndindowName = ConvertString("Add Allow List");
		GetDlgItem(IDC_STATIC_CT)->ShowWindow(FALSE);
		GetDlgItem(IDC_COMBO_CONTROLTYPE)->ShowWindow(FALSE);
		break;
	case EM_MODIFY_BACKLIST:
		ShowTime();
		m_cbPlateType.SetCurSel(m_stTrafficListInfo.emPlateType);
		m_cbPlateColor.SetCurSel(m_stTrafficListInfo.emPlateColor);
		m_cbVehicleType.SetCurSel(m_stTrafficListInfo.emVehicleType);
		m_cbVehicleColor.SetCurSel(m_stTrafficListInfo.emVehicleColor);
		m_cbControlType.SetCurSel(m_stTrafficListInfo.emControlType);
		strWndindowName = ConvertString("Modify Block List");
		GetDlgItem(IDC_STATIC_CT)->ShowWindow(TRUE);
		GetDlgItem(IDC_COMBO_CONTROLTYPE)->ShowWindow(TRUE);
		break;
	case EM_MODIFY_ALLOWLIST:
		ShowTime();
		m_cbPlateType.SetCurSel(m_stTrafficListInfo.emPlateType);
		m_cbPlateColor.SetCurSel(m_stTrafficListInfo.emPlateColor);
		m_cbVehicleType.SetCurSel(m_stTrafficListInfo.emVehicleType);
		m_cbVehicleColor.SetCurSel(m_stTrafficListInfo.emVehicleColor);
		strWndindowName = ConvertString("Modify Allow List");
		GetDlgItem(IDC_STATIC_CT)->ShowWindow(FALSE);
		GetDlgItem(IDC_COMBO_CONTROLTYPE)->ShowWindow(FALSE);
		break;
	default:
		strWndindowName = ConvertString("Unknown");
		break;
	}
	SetWindowText(strWndindowName);

	return TRUE;  // return TRUE unless you set the focus to a control
}


void CAddAndModifyBWDlg::SetLoginHandle(const LLONG lLoginHandle)
{
	if (0 != lLoginHandle)
	{
		m_lLoginHandle = lLoginHandle;
	}
}


void CAddAndModifyBWDlg::OnBnClickedBtnOk()
{
	if (0 == m_lLoginHandle)
	{
		return;
	}
	bool bRet = IsTimeCorrent();
	if (!bRet)
	{
		MessageBox(ConvertString("The begin time is bigger than end time, please input again!"), ConvertString("Prompt"));
		return;
	}

	CString strPlateNumber;
	CString strOwner;

	GetDlgItemText(IDC_EDIT_PLATE_NUMBER, strPlateNumber);
	GetDlgItemText(IDC_EDIT_OWNER, strOwner);

	NET_IN_OPERATE_TRAFFIC_LIST_RECORD stInParam = { sizeof(NET_IN_OPERATE_TRAFFIC_LIST_RECORD) };
	NET_OUT_OPERATE_TRAFFIC_LIST_RECORD stOutParam = { sizeof(NET_OUT_OPERATE_TRAFFIC_LIST_RECORD) };
	CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE);
	CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME);
	CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE);
	CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);

	if (EM_ADD_BACKLIST == m_emTrafficOperatorType || EM_ADD_ALLOWLIST == m_emTrafficOperatorType)
	{
		stInParam.emOperateType = NET_TRAFFIC_LIST_INSERT;
		if (EM_ADD_BACKLIST == m_emTrafficOperatorType )
		{
			stInParam.emRecordType = NET_RECORD_TRAFFICBLACKLIST;
		}
		else
		{
			stInParam.emRecordType = NET_RECORD_TRAFFICREDLIST;
		}

		NET_TRAFFIC_LIST_RECORD stTrafficListRecord = { sizeof(NET_TRAFFIC_LIST_RECORD) };
		GetTimeFromTimeCtr(stTrafficListRecord.stBeginTime, pCtrBeginDate, pCtrBeginTime);
		GetTimeFromTimeCtr(stTrafficListRecord.stCancelTime, pCtrEndtDate, pCtrEndTime);
		strncpy(stTrafficListRecord.szPlateNumber, strPlateNumber.GetBuffer(), DH_MAX_PLATE_NUMBER_LEN-1);
		strncpy(stTrafficListRecord.szMasterOfCar, strOwner.GetBuffer(), DH_MAX_NAME_LEN-1); 
		stTrafficListRecord.emPlateType = (EM_NET_PLATE_TYPE)m_cbPlateType.GetCurSel();
		stTrafficListRecord.emPlateColor = (EM_NET_PLATE_COLOR_TYPE)m_cbPlateColor.GetCurSel();
		stTrafficListRecord.emVehicleType = (EM_NET_VEHICLE_TYPE)m_cbVehicleType.GetCurSel();
		stTrafficListRecord.emVehicleColor = (EM_NET_VEHICLE_COLOR_TYPE)m_cbVehicleColor.GetCurSel();
		if (EM_ADD_BACKLIST == m_emTrafficOperatorType )
		{
			stTrafficListRecord.emControlType = (EM_NET_TRAFFIC_CAR_CONTROL_TYPE)m_cbControlType.GetCurSel();
		}

		NET_INSERT_RECORD_INFO stInsertInfo = { sizeof( NET_INSERT_RECORD_INFO) };
		stInsertInfo.pRecordInfo = &stTrafficListRecord;
		stInParam.pstOpreateInfo = &stInsertInfo;
		BOOL bRet = CLIENT_OperateTrafficList(m_lLoginHandle, &stInParam, &stOutParam, MAX_TIMEOUT);
		if (!bRet)
		{
			MessageBox(ConvertString("Operate Traffic list failed!"), ConvertString("Prompt"));
			return;
		}
	}
	else
	{
		stInParam.emOperateType = NET_TRAFFIC_LIST_UPDATE;
		if (EM_MODIFY_BACKLIST == m_emTrafficOperatorType)
		{
			stInParam.emRecordType = NET_RECORD_TRAFFICBLACKLIST;
		}
		else
		{
			stInParam.emRecordType = NET_RECORD_TRAFFICREDLIST;
		}
		NET_TRAFFIC_LIST_RECORD stTrafficListRecord = { sizeof(NET_TRAFFIC_LIST_RECORD) };
		GetTimeFromTimeCtr(stTrafficListRecord.stBeginTime, pCtrBeginDate, pCtrBeginTime);
		GetTimeFromTimeCtr(stTrafficListRecord.stCancelTime, pCtrEndtDate, pCtrEndTime);
		strncpy(stTrafficListRecord.szPlateNumber, strPlateNumber.GetBuffer(), DH_MAX_PLATE_NUMBER_LEN-1);
		strncpy(stTrafficListRecord.szMasterOfCar, strOwner.GetBuffer(), DH_MAX_NAME_LEN-1);
		stTrafficListRecord.emPlateType = (EM_NET_PLATE_TYPE)m_cbPlateType.GetCurSel();
		stTrafficListRecord.emPlateColor = (EM_NET_PLATE_COLOR_TYPE)m_cbPlateColor.GetCurSel();
		stTrafficListRecord.emVehicleType = (EM_NET_VEHICLE_TYPE)m_cbVehicleType.GetCurSel();
		stTrafficListRecord.emVehicleColor = (EM_NET_VEHICLE_COLOR_TYPE)m_cbVehicleColor.GetCurSel();
		if (EM_MODIFY_BACKLIST == m_emTrafficOperatorType )
		{
			stTrafficListRecord.emControlType = (EM_NET_TRAFFIC_CAR_CONTROL_TYPE)m_cbControlType.GetCurSel();
		}

		stTrafficListRecord.nRecordNo = m_stTrafficListInfo.nRecordNo;  //¼ÇÂ¼¼¯±àºÅ

		NET_UPDATE_RECORD_INFO stModifyRecord = {sizeof(NET_UPDATE_RECORD_INFO)};
		stModifyRecord.pRecordInfo = &stTrafficListRecord;
		stInParam.pstOpreateInfo = &stModifyRecord;
		BOOL bRet = CLIENT_OperateTrafficList(m_lLoginHandle, &stInParam, &stOutParam, MAX_TIMEOUT);
		if (!bRet)
		{
			MessageBox(ConvertString("Operate Traffic list failed!"), ConvertString("Prompt"));
			return;
		}
	}

	OnOK();
}

void CAddAndModifyBWDlg::OnBnClickedBtnCancle()
{
	OnCancel();
}


void CAddAndModifyBWDlg::GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime)
{
	if (NULL == pCtrDate || NULL == pCtrTime)
	{
		return;
	}

	COleDateTime tmDate;
	COleDateTime tmTime;
	pCtrDate->GetTime(tmDate);
	pCtrTime->GetTime(tmTime);

	stTime.dwYear = tmDate.GetYear();
	stTime.dwMonth = tmDate.GetMonth();
	stTime.dwDay = tmDate.GetDay();
	stTime.dwHour = tmTime.GetHour();
	stTime.dwMinute = tmTime.GetMinute();
	stTime.dwSecond = tmTime.GetSecond();
}


void CAddAndModifyBWDlg::SetTrafficListInfo(const NET_TRAFFIC_LIST_RECORD* pTrafficListInfo)
{
	if (NULL != pTrafficListInfo)
	{
		memcpy(&m_stTrafficListInfo, pTrafficListInfo, sizeof(NET_TRAFFIC_LIST_RECORD));
	}
}


void CAddAndModifyBWDlg::SetTrafficOperatorType(EM_TRAFFIC_LIST_OPERATOR_TYPE emTrafficOperatorType)
{
	m_emTrafficOperatorType = emTrafficOperatorType;
}

void CAddAndModifyBWDlg::ShowTime()
{
	COleDateTime tmDate;
	tmDate.SetDate(m_stTrafficListInfo.stBeginTime.dwYear, m_stTrafficListInfo.stBeginTime.dwMonth, m_stTrafficListInfo.stBeginTime.dwDay);
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE))->SetTime(tmDate);

	tmDate.SetDate(m_stTrafficListInfo.stCancelTime.dwYear, m_stTrafficListInfo.stCancelTime.dwMonth, m_stTrafficListInfo.stCancelTime.dwDay);
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE))->SetTime(tmDate);

	tmDate.SetTime(m_stTrafficListInfo.stBeginTime.dwHour, m_stTrafficListInfo.stBeginTime.dwMinute, m_stTrafficListInfo.stBeginTime.dwSecond);
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME))->SetTime(tmDate);

	tmDate.SetTime(m_stTrafficListInfo.stCancelTime.dwHour, m_stTrafficListInfo.stCancelTime.dwMinute, m_stTrafficListInfo.stCancelTime.dwSecond);
	((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME))->SetTime(tmDate);
}

bool CAddAndModifyBWDlg::IsTimeCorrent()
{
	CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE);
	CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME);
	CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE);
	CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);

	COleDateTime beginDate;
	COleDateTime beginTime;
	COleDateTime endDate;
	COleDateTime endTime;
	pCtrBeginDate->GetTime(beginDate);
	pCtrBeginTime->GetTime(beginTime);
	pCtrEndtDate->GetTime(endDate);
	pCtrEndTime->GetTime(endTime);

	if (endDate >= beginDate)
	{
		if (endDate == beginDate)
		{
			if (endTime < beginTime)
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}


void CAddAndModifyBWDlg::OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);

	if (2000 > pDTChange->st.wYear || 2038 < pDTChange->st.wYear)
	{
		MessageBox(ConvertString("The time range is from 2000 to 2038.Please input again!"), ConvertString("Prompt"));
		COleDateTime tmDate  = COleDateTime::GetCurrentTime();
		((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE))->SetTime(tmDate);

	}   
	*pResult = 0;
}

void CAddAndModifyBWDlg::OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	if (2000 > pDTChange->st.wYear || 2038 < pDTChange->st.wYear)
	{
		MessageBox(ConvertString("The time range is from 2000 to 2038.Please input again!"), ConvertString("Prompt"));
		COleDateTime tmDate  = COleDateTime::GetCurrentTime();
		((CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE))->SetTime(tmDate);

	}   
	*pResult = 0;
}
