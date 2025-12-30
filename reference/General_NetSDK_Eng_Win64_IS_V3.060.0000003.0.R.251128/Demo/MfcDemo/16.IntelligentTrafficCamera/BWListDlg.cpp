// BWListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelligentTrafficCamera.h"
#include "BWListDlg.h"
#include "AddAndModifyBWDlg.h"
#include <cmath>


// CBWListDlg Dialog

IMPLEMENT_DYNAMIC(CBWListDlg, CDialog)

CBWListDlg::CBWListDlg(CWnd* pParent ,int nChannel,LLONG lLoginID )
: CDialog(CBWListDlg::IDD, pParent)
{

	m_nCurPage = 0;
	m_nTotalPage = 0;
	m_nIndexOfTrafficList = 0;
	m_nTotalCountOfTrafficList = 0;

	//m_lLoginHandle = 0;
	m_lLoginHandle = lLoginID;
	m_nChannel = nChannel;
	m_LFindeHandle = 0;
}

CBWListDlg::~CBWListDlg()
{
	CleanUp();
}

void CBWListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTTN_QUERY, m_btnQuery);
	DDX_Control(pDX, IDC_BTN_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_MODIFY, m_btnModify);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_COMBO_CHANNEL, m_cmbChannel);
	DDX_Control(pDX, IDC_LIST_BW, m_ctrTrafficList);
	DDX_Control(pDX, IDC_BTN_PREVIOUS_PAGE, m_btnPreviousPage);
	DDX_Control(pDX, IDC_BTN_NEXT_PAGE, m_btnNextPage);
	DDX_Control(pDX, IDC_COMBO_CHANNEL2, m_cmbType);
}


BEGIN_MESSAGE_MAP(CBWListDlg, CDialog)
	ON_BN_CLICKED(IDC_BTTN_QUERY, &CBWListDlg::OnBnClickedBtnQuery)
	ON_BN_CLICKED(IDC_BTN_MODIFY, &CBWListDlg::OnBnClickedBtnModify)
	ON_BN_CLICKED(IDC_BTN_ADD, &CBWListDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DELETE, &CBWListDlg::OnBnClickedBtnDelete)
	ON_BN_CLICKED(IDC_BTN_PREVIOUS_PAGE, &CBWListDlg::OnBnClickedBtnPreviousPage)
	ON_BN_CLICKED(IDC_BTN_NEXT_PAGE, &CBWListDlg::OnBnClickedBtnNextPage)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNEL2, &CBWListDlg::OnCbnSelchangeComboChannelTrafficList)
END_MESSAGE_MAP()


// CBWListDlg message handlers

BOOL CBWListDlg::PreTranslateMessage(MSG* pMsg)
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


BOOL CBWListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);

	for (unsigned int i = 0 ;i < m_nChannel; i++)
	{
		CString csChannel;
		csChannel.Format("%d", i+1);
		m_cmbChannel.InsertString(i, csChannel);
	}
	m_cmbChannel.SetCurSel(0);

	m_cmbType.InsertString(0, ConvertString("Allow List"));
	m_cmbType.InsertString(1, ConvertString("Block List"));
	m_cmbType.SetCurSel(0);

	m_btnQuery.EnableWindow(TRUE);
	m_btnAdd.EnableWindow(TRUE);
	m_btnModify.EnableWindow(TRUE);
	m_btnDelete.EnableWindow(TRUE);

	m_btnPreviousPage.EnableWindow(FALSE);
	m_btnNextPage.EnableWindow(FALSE);

	m_ctrTrafficList.SetExtendedStyle(m_ctrTrafficList.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctrTrafficList.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 60);
	m_ctrTrafficList.InsertColumn(1, ConvertString("Plate Number"), LVCFMT_LEFT, 120);
	m_ctrTrafficList.InsertColumn(2, ConvertString("Owner of Car"), LVCFMT_LEFT, 120);
	m_ctrTrafficList.InsertColumn(3, ConvertString("Start Time"), LVCFMT_LEFT, 130);
	m_ctrTrafficList.InsertColumn(4, ConvertString("End Time"), LVCFMT_LEFT, 130);
	m_ctrTrafficList.InsertColumn(5, ConvertString("PlateType"), LVCFMT_LEFT, 130);
	m_ctrTrafficList.InsertColumn(6, ConvertString("PlateColor"), LVCFMT_LEFT, 130);
	m_ctrTrafficList.InsertColumn(7, ConvertString("VehicleType"), LVCFMT_LEFT, 130);
	m_ctrTrafficList.InsertColumn(8, ConvertString("VehicleColor"), LVCFMT_LEFT, 130);
	m_ctrTrafficList.InsertColumn(9, ConvertString("ControlType"), LVCFMT_LEFT, 130);
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CBWListDlg::CleanUp()
{
	if (0 != m_LFindeHandle)
	{
		// stop querying
		CLIENT_FindRecordClose(m_LFindeHandle);
	}

	//m_btnQuery.EnableWindow(FALSE);
	//m_btnAdd.EnableWindow(FALSE);
	//m_btnModify.EnableWindow(FALSE);
	//m_btnDelete.EnableWindow(FALSE);
	//m_btnPreviousPage.EnableWindow(FALSE);
	//m_btnNextPage.EnableWindow(FALSE);

	//m_cmbType.ResetContent();
	//m_cmbChannel.ResetContent();
	//m_cmbChannel.ResetContent();
	//m_ctrTrafficList.DeleteAllItems();

	//SetDlgItemText(IDC_EDIT_RECORD_COUNT, "");
	//SetDlgItemText(IDC_EDIT_RECORD_CURRENT_PAGE, "");

	// Delete vector 
	for (unsigned int i =0; i < m_vecTrafficListInfo.size(); i++)
	{
		NET_TRAFFIC_LIST_RECORD* pTraffic = m_vecTrafficListInfo[i];
		if (pTraffic)
		{
			delete pTraffic;
			pTraffic = NULL;
		}
	}
	m_vecTrafficListInfo.clear();
}

void CBWListDlg::OnBnClickedBtnAdd()
{
	CAddAndModifyBWDlg dlg;
	int nType = m_cmbType.GetCurSel();

	if (0 == nType)  // Add whilte list
	{
		dlg.SetTrafficOperatorType(EM_ADD_ALLOWLIST);
	}
	else   // Add block list
	{
		dlg.SetTrafficOperatorType(EM_ADD_BACKLIST);
	}
	dlg.SetLoginHandle(m_lLoginHandle);

	if (IDOK == dlg.DoModal())
	{
		OnBnClickedBtnQuery();
	}
}

void CBWListDlg::OnBnClickedBtnModify()
{
	int nSelect = m_ctrTrafficList.GetNextItem(-1,LVNI_SELECTED);
	if (-1 == nSelect)
	{
		MessageBox(ConvertString("Please select a traffic list!"), ConvertString("Prompt"));
		return;
	}
	int nCurrentIndex = (m_nCurPage-1)*SINGLE_QUERY_COUNT+nSelect;

	CAddAndModifyBWDlg dlg;
	int nType = m_cmbType.GetCurSel();
	if (0 == nType)  // Modify Allow list
	{
		dlg.SetTrafficOperatorType(EM_MODIFY_ALLOWLIST);
	}
	else  // Modify block list
	{
		dlg.SetTrafficOperatorType(EM_MODIFY_BACKLIST);
	}
	dlg.SetLoginHandle(m_lLoginHandle);
	dlg.SetTrafficListInfo(m_vecTrafficListInfo[nCurrentIndex]);

	if ( IDOK == dlg.DoModal())
	{
		OnBnClickedBtnQuery();
	}
}


void CBWListDlg::OnBnClickedBtnDelete()
{
	int nSelect = m_ctrTrafficList.GetNextItem(-1,LVNI_SELECTED);
	if (-1 == nSelect)
	{
		MessageBox(ConvertString("Please select a traffic list!"), ConvertString("Prompt"));
		return;
	}

	int nCurrentIndex = (m_nCurPage-1)*SINGLE_QUERY_COUNT+nSelect;

	NET_IN_OPERATE_TRAFFIC_LIST_RECORD stInParam = { sizeof(NET_IN_OPERATE_TRAFFIC_LIST_RECORD) };
	NET_OUT_OPERATE_TRAFFIC_LIST_RECORD stOutParam = { sizeof(NET_OUT_OPERATE_TRAFFIC_LIST_RECORD) };
	stInParam.emOperateType = NET_TRAFFIC_LIST_REMOVE;
	if (0 == m_cmbType.GetCurSel())
	{
		stInParam.emRecordType = NET_RECORD_TRAFFICREDLIST;
	}
	else
	{
		stInParam.emRecordType = NET_RECORD_TRAFFICBLACKLIST;
	}
	NET_REMOVE_RECORD_INFO stRemoveRecord = { sizeof(NET_REMOVE_RECORD_INFO) };
	stRemoveRecord.nRecordNo = m_vecTrafficListInfo[nCurrentIndex]->nRecordNo;
	stInParam.pstOpreateInfo = &stRemoveRecord;

	BOOL bRet = CLIENT_OperateTrafficList(m_lLoginHandle, &stInParam, &stOutParam, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Delete Traffic list failed!"), ConvertString("Prompt"));
		return;
	}
	OnBnClickedBtnQuery();
}

void CBWListDlg::OnBnClickedBtnQuery()
{  
	CleanUpResultOfLastTimeQuery();

	// Start query records
	m_LFindeHandle = StartFindTrafficList();
	if (0 ==  m_LFindeHandle)
	{
		return;
	}

	// Get the total count
	m_nTotalCountOfTrafficList = GetTotalCountOfTrafficList(m_LFindeHandle);
	if (m_nTotalCountOfTrafficList <= 0)
	{
		return;
	}

	m_nCurPage = 1;
	SetDlgItemInt(IDC_EDIT_RECORD_COUNT, m_nTotalCountOfTrafficList);
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);

	int nQueryCount = SINGLE_QUERY_COUNT;
	if (m_nTotalCountOfTrafficList <= SINGLE_QUERY_COUNT)
	{
		nQueryCount = m_nTotalCountOfTrafficList ;
		m_nTotalPage = 1;
	}
	else
	{
		m_btnNextPage.EnableWindow(TRUE);
		m_nTotalPage = (int)ceil(m_nTotalCountOfTrafficList / (float)nQueryCount);
	}

	// Do find N number of traffic list
	if (!DoFindTrafficList(nQueryCount))
	{
		m_btnNextPage.EnableWindow(FALSE);
		return;
	}
}

void CBWListDlg::CleanUpResultOfLastTimeQuery()
{
	m_ctrTrafficList.DeleteAllItems();
	m_nCurPage = 0;
	m_nTotalPage = 0;
	m_nIndexOfTrafficList = 0;
	m_nTotalCountOfTrafficList = 0;
	m_btnPreviousPage.EnableWindow(FALSE);
	m_btnNextPage.EnableWindow(FALSE);

	SetDlgItemText(IDC_EDIT_RECORD_COUNT, "");
	SetDlgItemText(IDC_EDIT_RECORD_CURRENT_PAGE, "");

	for (unsigned int i =0; i < m_vecTrafficListInfo.size(); i++)
	{
		NET_TRAFFIC_LIST_RECORD* pTraffic = m_vecTrafficListInfo[i];
		if (pTraffic)
		{
			delete pTraffic;
			pTraffic = NULL;
		}
	}
	m_vecTrafficListInfo.clear();

	if (0 != m_LFindeHandle)
	{
		// stop querying
		CLIENT_FindRecordClose(m_LFindeHandle);
		m_LFindeHandle = 0;
	}
}

LLONG CBWListDlg::StartFindTrafficList()
{
	NET_IN_FIND_RECORD_PARAM stuFindInParam = {sizeof(NET_IN_FIND_RECORD_PARAM)};
	NET_OUT_FIND_RECORD_PARAM stuFindOutParam = {sizeof(NET_OUT_FIND_RECORD_PARAM)};
	if (0 == m_cmbType.GetCurSel())
	{
		stuFindInParam.emType = NET_RECORD_TRAFFICREDLIST;
	}
	else
	{
		stuFindInParam.emType = NET_RECORD_TRAFFICBLACKLIST;
	}
	FIND_RECORD_TRAFFICREDLIST_CONDITION stuRedListCondition = {sizeof(FIND_RECORD_TRAFFICREDLIST_CONDITION)};
	CString strPlateNumber;
	GetDlgItemText(IDC_EDIT_PLATE_NUMBER, strPlateNumber);
	memcpy(stuRedListCondition.szPlateNumberVague, strPlateNumber.GetBuffer(), DH_MAX_PLATE_NUMBER_LEN);
	stuFindInParam.pQueryCondition = &stuRedListCondition;

	BOOL bRet = CLIENT_FindRecord(m_lLoginHandle, &stuFindInParam, &stuFindOutParam, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Start query record failed!"), ConvertString("Prompt"));
		return 0;
	}
	return stuFindOutParam.lFindeHandle;
}

int CBWListDlg::GetTotalCountOfTrafficList(LLONG lFindeHandle)
{
	NET_IN_QUEYT_RECORD_COUNT_PARAM inQueryCountParam = { sizeof(NET_IN_QUEYT_RECORD_COUNT_PARAM)};
	NET_OUT_QUEYT_RECORD_COUNT_PARAM outQueryCountParam  = { sizeof(NET_OUT_QUEYT_RECORD_COUNT_PARAM) };
	inQueryCountParam.lFindeHandle =  lFindeHandle;

	BOOL bRet = CLIENT_QueryRecordCount(&inQueryCountParam, &outQueryCountParam , MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Query record count failed!"), ConvertString("Prompt"));
		return -1;
	}
	if (outQueryCountParam.nRecordCount == 0)
	{
		MessageBox(ConvertString("Query zero!"), ConvertString("Prompt"));
		return -1;
	}
	return outQueryCountParam.nRecordCount;
}

bool CBWListDlg::DoFindTrafficList( int nQueryCount )
{
	NET_TRAFFIC_LIST_RECORD* pRecordList = new NET_TRAFFIC_LIST_RECORD[nQueryCount];
	memset(pRecordList, 0, sizeof(NET_TRAFFIC_LIST_RECORD) * nQueryCount);
	for (int unIndex = 0; unIndex < nQueryCount; ++unIndex)
	{
		pRecordList[unIndex].dwSize = sizeof(NET_TRAFFIC_LIST_RECORD);
		for (unsigned int unIndex2 = 0; unIndex2 < DH_MAX_AUTHORITY_LIST_NUM; ++unIndex2)
		{
			pRecordList[unIndex].stAuthrityTypes[unIndex2].dwSize = sizeof(NET_AUTHORITY_TYPE);
		}
	}

	NET_IN_FIND_NEXT_RECORD_PARAM stuFindNextInParam =  {sizeof(NET_IN_FIND_NEXT_RECORD_PARAM)};
	stuFindNextInParam.lFindeHandle = m_LFindeHandle;
	stuFindNextInParam.nFileCount = nQueryCount;

	NET_OUT_FIND_NEXT_RECORD_PARAM stuFindNextOutParam = {sizeof(NET_OUT_FIND_NEXT_RECORD_PARAM)};
	stuFindNextOutParam.pRecordList = pRecordList;
	stuFindNextOutParam.nMaxRecordNum = nQueryCount;
	BOOL bRet = CLIENT_FindNextRecord(&stuFindNextInParam, &stuFindNextOutParam, MAX_TIMEOUT);
	if (!bRet)
	{
		MessageBox(ConvertString("Query record count failed!"), ConvertString("Prompt"));
		delete[] pRecordList;
		return false;
	}
	if (0 == stuFindNextOutParam.nRetRecordNum)
	{
		MessageBox(ConvertString("Query record over!"), ConvertString("Prompt"));
		delete[] pRecordList;
		return false;
	}

	for (int i = 0; i < stuFindNextOutParam.nRetRecordNum; i++)
	{ 
		m_nIndexOfTrafficList++;
		ShowTrafficList(pRecordList+i, m_nIndexOfTrafficList);

		NET_TRAFFIC_LIST_RECORD* pTrafficListInfo = new NET_TRAFFIC_LIST_RECORD;
		memcpy(pTrafficListInfo, pRecordList+i, sizeof(NET_TRAFFIC_LIST_RECORD));        
		m_vecTrafficListInfo.push_back(pTrafficListInfo);
	}
	delete[] pRecordList;
	return true;
}

void CBWListDlg::ShowTrafficList(NET_TRAFFIC_LIST_RECORD* pRecordList, int nCurrentIndex)
{
	if (NULL == pRecordList)
	{
		return;
	}
	CString strStartTime;
	CString strEndTime;
	CString strIndex;
	strIndex.Format("%d", nCurrentIndex);
	strStartTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
		pRecordList->stBeginTime.dwYear, pRecordList->stBeginTime.dwMonth, 
		pRecordList->stBeginTime.dwDay, pRecordList->stBeginTime.dwHour, 
		pRecordList->stBeginTime.dwMinute, pRecordList->stBeginTime.dwSecond);
	strEndTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
		pRecordList->stCancelTime.dwYear, pRecordList->stCancelTime.dwMonth, 
		pRecordList->stCancelTime.dwDay, pRecordList->stCancelTime.dwHour, 
		pRecordList->stCancelTime.dwMinute, pRecordList->stCancelTime.dwSecond);

	int nCount =  m_ctrTrafficList.GetItemCount();
	m_ctrTrafficList.InsertItem(LVIF_TEXT|LVIF_STATE,nCount,strIndex,0,LVIS_SELECTED,0,0);
	m_ctrTrafficList.SetItemText(nCount,0, strIndex);
	m_ctrTrafficList.SetItemText(nCount,1, pRecordList->szPlateNumber);
	m_ctrTrafficList.SetItemText(nCount,2, pRecordList->szMasterOfCar);
	m_ctrTrafficList.SetItemText(nCount,3, strStartTime);
	m_ctrTrafficList.SetItemText(nCount,4, strEndTime);
	m_ctrTrafficList.SetItemText(nCount,5, ConvertString(strPlateTypeList[pRecordList->emPlateType]));
	m_ctrTrafficList.SetItemText(nCount,6, ConvertString(strPlateColorTypeList[pRecordList->emPlateColor]));
	m_ctrTrafficList.SetItemText(nCount,7, ConvertString(strVehtcleTypeList[pRecordList->emVehicleType]));
	m_ctrTrafficList.SetItemText(nCount,8, ConvertString(strVehtcleColorTypeList[pRecordList->emVehicleColor]));
	if (1 == m_cmbType.GetCurSel())
	{
		m_ctrTrafficList.SetItemText(nCount,9, ConvertString(strCarControlTypeList[pRecordList->emControlType]));
	}
}

void CBWListDlg::OnBnClickedBtnPreviousPage()
{
	if(1 >= m_nCurPage )
	{
		return;
	}
	m_ctrTrafficList.DeleteAllItems();
	for (int i = 0; i < SINGLE_QUERY_COUNT; i++)
	{
		ShowTrafficList(m_vecTrafficListInfo[(m_nCurPage-2)*SINGLE_QUERY_COUNT+i], (m_nCurPage-2)*SINGLE_QUERY_COUNT+i+1);
	}
	m_nCurPage--;
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);
	if (m_nCurPage <= 1)
	{
		m_btnPreviousPage.EnableWindow(FALSE);
	}
	m_btnNextPage.EnableWindow(TRUE);
}

void CBWListDlg::OnBnClickedBtnNextPage()
{
	if(0 >= m_nCurPage || m_nCurPage >= m_nTotalPage)
	{
		return;
	}
	m_ctrTrafficList.DeleteAllItems();
	if (m_vecTrafficListInfo.size() > m_nCurPage*SINGLE_QUERY_COUNT)
	{
		int nRemainCount = m_vecTrafficListInfo.size() - m_nCurPage*SINGLE_QUERY_COUNT;
		int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT?SINGLE_QUERY_COUNT:nRemainCount;
		for (int i = 0; i < nQueryCount; i++)
		{
			ShowTrafficList(m_vecTrafficListInfo[m_nCurPage*SINGLE_QUERY_COUNT+i], m_nCurPage*SINGLE_QUERY_COUNT+i+1);
		}
	}
	else
	{
		int nRemainCount = m_nTotalCountOfTrafficList - m_nCurPage*SINGLE_QUERY_COUNT;
		int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT?SINGLE_QUERY_COUNT:nRemainCount;
		if (!DoFindTrafficList(nQueryCount))
		{
			return;
		}
	} 
	m_nCurPage++;
	SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);
	if (m_nCurPage >= m_nTotalPage)
	{
		m_btnNextPage.EnableWindow(FALSE);
	}

	m_btnPreviousPage.EnableWindow(TRUE);
}


void CBWListDlg::OnCbnSelchangeComboChannelTrafficList()
{
	CleanUpResultOfLastTimeQuery();
}


