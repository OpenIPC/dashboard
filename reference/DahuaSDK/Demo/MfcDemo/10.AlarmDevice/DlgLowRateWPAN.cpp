// DlgLowRateWPAN.cpp : implementation file
//

#include "stdafx.h"
#include "AlarmDevice.h"
#include "DlgLowRateWPAN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLowRateWPAN dialog


CDlgLowRateWPAN::CDlgLowRateWPAN(CWnd* pParent /*=NULL*/, LLONG lLoginId)
	: CDialog(CDlgLowRateWPAN::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLowRateWPAN)
	m_nID = 0;
	m_nQueryNum = 0;
	m_nStartIndex = 0;
	m_nTotalCodeIDNum = 0;
	//}}AFX_DATA_INIT
	m_lLoginId = lLoginId;
	m_lHandle = 0;
	m_nListIndex = 0;
}


void CDlgLowRateWPAN::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLowRateWPAN)
	DDX_Control(pDX, IDC_LOWRATEWPAN_LIST_CODEIDLIST, m_listCodeIDList);
	DDX_Control(pDX, IDC_LOWRATEWPAN_EDT_NAME, m_edtName);
	DDX_Control(pDX, IDC_LOWRATEWPAN_CMB_TYPE, m_cmbType);
	DDX_Control(pDX, IDC_LOWRATEWPAN_CMB_INDEX, m_cmbIndex);
	DDX_Control(pDX, IDC_LOWRATEWPAN_CK_ENABLE, m_ckEnable);
	DDX_Text(pDX, IDC_LOWRATEWPAN_EDT_ID, m_nID);
	DDX_Text(pDX, IDC_LOWRATEWPAN_EDT_QUERYNUM, m_nQueryNum);
//	DDV_MinMaxInt(pDX, m_nQueryNum, 0, 100);
	DDX_Text(pDX, IDC_LOWRATEWPAN_EDT_STARTINDEX, m_nStartIndex);
//	DDV_MinMaxInt(pDX, m_nStartIndex, 0, 200);
	DDX_Text(pDX, IDC_LOWRATEWPAN_EDT_TOTALCODEIDNUM, m_nTotalCodeIDNum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLowRateWPAN, CDialog)
	//{{AFX_MSG_MAP(CDlgLowRateWPAN)
	ON_CBN_SELCHANGE(IDC_LOWRATEWPAN_CMB_INDEX, OnSelchangeCmbLowRateWPanIndex)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_QUERY, OnLowRateWPanBtnQuery)
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_MODIFY, OnLowRateWPanBtnModify)
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_REMOVE, OnLowRateWPanBtnRemove)
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_REMOVEALL, OnLowRateWPanBtnRemoveall)
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_ATTACH, OnLowRateWPanBtnAttach)
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_DETACH, OnLowRateWPanBtnDetach)
    ON_MESSAGE(WM_LOWRATEWPAN_INFO_NOTIFY, OnLowRateWPanNotify)
	ON_BN_CLICKED(IDC_LOWRATEWPAN_BTN_TOTAL, OnLowRateWPanBtnTotal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLowRateWPAN private method

void CALLBACK OnLowRateWPanDataCallBack(LLONG lLoginID, LLONG lAttachHandle, NET_CODEID_INFO* stuBuf, NET_CODEID_ERROR_TYPE emError, LDWORD dwUser)
{
    ((CDlgLowRateWPAN*)dwUser)->LowRateWPANDataCallBack(lLoginID, lAttachHandle, stuBuf, emError);
}

void CDlgLowRateWPAN::LowRateWPANDataCallBack(LLONG lLoginID, LLONG lAttachHandle, NET_CODEID_INFO* stuBuf, NET_CODEID_ERROR_TYPE emError)
{
    if (m_lLoginId == lLoginID && m_lHandle == lAttachHandle)
    {
        NET_CODEID_INFO* pInfo = new NET_CODEID_INFO;
        if (pInfo != NULL)
        {
            memset(pInfo, 0, sizeof(*pInfo));
            memcpy(pInfo, stuBuf, sizeof(NET_CODEID_INFO));
            PostMessage(WM_LOWRATEWPAN_INFO_NOTIFY, (WPARAM)pInfo, (LPARAM)emError);
        }
    }
}

LRESULT CDlgLowRateWPAN::OnLowRateWPanNotify(WPARAM wParam, LPARAM lParam)
{
    NET_CODEID_INFO* pInfo = (NET_CODEID_INFO*)wParam;
    NET_CODEID_ERROR_TYPE emError = (NET_CODEID_ERROR_TYPE)lParam;
    if (NULL == pInfo)
    {
        return (LRESULT)-1;
    }

    char szIndex[32] = {0};
    _itoa(m_nListIndex, szIndex, 10);
    m_listCodeIDList.InsertItem(m_nListIndex, NULL);
    m_listCodeIDList.SetItemText(m_nListIndex, 0, szIndex);
    CString csBufInfo;
    csBufInfo.Format("WirelessId:%03d, Type:%s, szName:%s, bEnable:%d", pInfo->nWirelessId, stuType[(int)pInfo->emType].pszName, pInfo->szName, pInfo->bEnable);
    m_listCodeIDList.SetItemText(m_nListIndex, 1, csBufInfo);
    m_listCodeIDList.SetItemText(m_nListIndex, 2, stuError[(int)emError]);
    ++m_nListIndex;

    delete pInfo;
    pInfo = NULL;
    return (LRESULT)0;
}

void CDlgLowRateWPAN::InitDlg()
{
	// Device Type
    m_cmbType.ResetContent();
    for (int i = 0; i < sizeof(stuType) / sizeof(stuType[0]); i++)
    {
        m_cmbType.InsertString(-1, ConvertString(stuType[i].pszName, DLG_LOWRATEWPAN));
    }
    m_cmbType.SetCurSel(0);

    // List 
    m_listCodeIDList.SetExtendedStyle(m_listCodeIDList.GetExtendedStyle()| LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );
    m_listCodeIDList.InsertColumn(0, ConvertString("Index", DLG_LOWRATEWPAN), LVCFMT_LEFT, 60, -1);
    m_listCodeIDList.InsertColumn(1, ConvertString("BufInfo", DLG_LOWRATEWPAN), LVCFMT_LEFT, 200, -1);
    m_listCodeIDList.InsertColumn(2, ConvertString("Error", DLG_LOWRATEWPAN), LVCFMT_LEFT, 50, -1);

    // query num
    SetDlgItemInt(IDC_LOWRATEWPAN_EDT_QUERYNUM, 8, FALSE);

    // detach button
	GetDlgItem(IDC_BTN_LOWRATEWPAN_DETACH)->EnableWindow(FALSE);
}

void CDlgLowRateWPAN::ShowCodeID(const NET_CODEID_INFO& stuInfo)
{
    // id
    SetDlgItemInt(IDC_LOWRATEWPAN_EDT_ID, stuInfo.nWirelessId);
    
    // name
    SetDlgItemText(IDC_LOWRATEWPAN_EDT_NAME, stuInfo.szName);
    
    // type
    m_cmbType.SetCurSel((int)stuInfo.emType);
    
    // enable
    m_ckEnable.SetCheck(stuInfo.bEnable ? BST_CHECKED : BST_UNCHECKED);
}

void CDlgLowRateWPAN::ClearCodeID()
{
    for (int i = 0; i < m_cmbIndex.GetCount(); i++)
    {
       NET_CODEID_INFO* pInfo = (NET_CODEID_INFO*)m_cmbIndex.GetItemDataPtr(i);
       if (pInfo != NULL)
       {
           delete pInfo;
           pInfo = NULL;
       }
    }
    m_cmbIndex.ResetContent();
}

void CDlgLowRateWPAN::InsertCodeID(const NET_CODEID_INFO& stuInfo)
{
    NET_CODEID_INFO* pInfo = new NET_CODEID_INFO;
    if (pInfo != NULL)
    {
        memset(pInfo, 0, sizeof(*pInfo));
        pInfo->dwSize = sizeof(*pInfo);
        memcpy(pInfo, &stuInfo, sizeof(NET_CODEID_INFO));

        int nCount = m_cmbIndex.GetCount();
        CString csIndex;
        csIndex.Format("%d", nCount + 1);
        
        int nIndex = m_cmbIndex.InsertString(-1, csIndex);
        m_cmbIndex.SetItemDataPtr(nIndex, (void*)pInfo);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgLowRateWPAN message handlers

BOOL CDlgLowRateWPAN::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this, DLG_LOWRATEWPAN);
	// TODO: Add extra initialization here
    InitDlg();

	//UpdateData(FALSE);

	// 获取信息
	OnLowRateWPanBtnQuery();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgLowRateWPAN::OnLowRateWPanBtnModify() 
{
	// TODO: Add your control notification handler code here
	//UpdateData(TRUE);
	NET_CTRL_LOWRATEWPAN_MODIFY stuParam = {sizeof(NET_CTRL_LOWRATEWPAN_MODIFY)};
	stuParam.stuCodeIDInfo.dwSize = sizeof(NET_CODEID_INFO);

	stuParam.stuCodeIDInfo.nWirelessId = GetDlgItemInt(IDC_LOWRATEWPAN_EDT_ID, NULL, FALSE);
    GetDlgItemText(IDC_LOWRATEWPAN_EDT_NAME, stuParam.stuCodeIDInfo.szName, DH_USER_NAME_LENGTH - 1);
	stuParam.stuCodeIDInfo.emType = (NET_WIRELESS_DEVICE_TYPE)m_cmbType.GetCurSel();
    stuParam.stuCodeIDInfo.bEnable = m_ckEnable.GetCheck() == BST_CHECKED ? TRUE : FALSE;

	BOOL bRet = CLIENT_ControlDevice((LLONG)m_lLoginId, DH_CTRL_LOWRATEWPAN_MODIFY, &stuParam, SDK_API_WAIT);
	CString csOut;
	if (bRet)
	{
		csOut.Format("%s %d %s", ConvertString("Num", DLG_LOWRATEWPAN),
			m_cmbIndex.GetCurSel() + 1, ConvertString("Modify CodeID Info success!", DLG_LOWRATEWPAN));
	} 
	else
	{
		csOut.Format("%s %d %s %08x", ConvertString("Num", DLG_LOWRATEWPAN),
			m_cmbIndex.GetCurSel() + 1, ConvertString("Modify CodeID Info error:", DLG_LOWRATEWPAN), CLIENT_GetLastError());
	}

	MessageBox(csOut, ConvertString("Prompt"));

}

void CDlgLowRateWPAN::OnLowRateWPanBtnRemove() 
{
	// TODO: Add your control notification handler code here
	//UpdateData(TRUE);
	NET_CTRL_LOWRATEWPAN_REMOVE stuParam = {sizeof(NET_CTRL_LOWRATEWPAN_REMOVE)};
	stuParam.nWirelessId = GetDlgItemInt(IDC_LOWRATEWPAN_EDT_ID, NULL, FALSE);
	
	BOOL bRet = CLIENT_ControlDevice((LLONG)m_lLoginId, DH_CTRL_LOWRATEWPAN_REMOVE, &stuParam, SDK_API_WAIT);
	CString csOut;
	if (bRet)
	{
		csOut.Format("%s", ConvertString("Remove CodeID Info success!", DLG_LOWRATEWPAN));
	} 
	else
	{
		csOut.Format("%s %08x", ConvertString("Remove CodeID Info error:", DLG_LOWRATEWPAN), CLIENT_GetLastError());
	}
	
	MessageBox(csOut, ConvertString("Prompt"));

	// 刷新显示
	OnLowRateWPanBtnQuery();
}

void CDlgLowRateWPAN::OnLowRateWPanBtnRemoveall() 
{/*
	// TODO: Add your control notification handler code here
	NET_CTRL_LOWRATEWPAN_REMOVEALL stuParam = {sizeof(NET_CTRL_LOWRATEWPAN_REMOVEALL)};
		
	BOOL bRet = CLIENT_ControlDevice((LLONG)m_lLoginId, DH_CTRL_LOWRATEWPAN_REMOVEALL, &stuParam, SDK_API_WAIT);
	CString csOut;
	if (bRet)
	{
		csOut.Format("%s", ConvertString("RemoveAll success!", DLG_LOWRATEWPAN));
	} 
	else
	{
		csOut.Format("%s %08x", ConvertString("RemoveAll error:", DLG_LOWRATEWPAN), CLIENT_GetLastError());
	}
	
	MessageBox(csOut, ConvertString("Prompt"));

	// 再查询一遍
	OnLowratewpanBtnQuery();
    */
}

void CDlgLowRateWPAN::OnLowRateWPanBtnQuery() 
{
	// TODO: Add your control notification handler code here
    UINT nQueryNum = GetDlgItemInt(IDC_LOWRATEWPAN_EDT_QUERYNUM, NULL, FALSE);
    UINT nStartIndex = GetDlgItemInt(IDC_LOWRATEWPAN_EDT_STARTINDEX, NULL, FALSE);
	NET_CODEID_INFO* pstuCodeIDInfo = new NET_CODEID_INFO[nQueryNum];
	if (pstuCodeIDInfo == NULL)
	{
		MessageBox(ConvertString("Apply memory error!", DLG_LOWRATEWPAN), ConvertString("Prompt"));
		return;
	}
	
    int i = 0;
	for (i = 0; i < nQueryNum; ++i)
	{
		memset(&pstuCodeIDInfo[i], 0, sizeof(NET_CODEID_INFO));
		pstuCodeIDInfo[i].dwSize = sizeof(NET_CODEID_INFO);
	}
	
    NET_GET_CODEID_LIST stuCodeIDList = {sizeof(stuCodeIDList)};
    stuCodeIDList.pstuCodeIDInfo = pstuCodeIDInfo;
	stuCodeIDList.nStartIndex = nStartIndex;
	stuCodeIDList.nQueryNum = nQueryNum;
	
	int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_GET_CODEID_LIST, 
		(char*)&stuCodeIDList, sizeof(NET_GET_CODEID_LIST), &nRetLen, SDK_API_WAIT);
	if (!bRet)
	{
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query CodeID List failed:", DLG_LOWRATEWPAN), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));

        delete []pstuCodeIDInfo;
        pstuCodeIDInfo = NULL;
		return;
	} 

    // update list info
    ClearCodeID();
    for (i = 0; i < __min(stuCodeIDList.nRetCodeIDNum, nQueryNum); i++)
    {
        InsertCodeID(stuCodeIDList.pstuCodeIDInfo[i]);
    }
	m_cmbIndex.SetCurSel(0);
    ShowCodeID(stuCodeIDList.pstuCodeIDInfo[0]);

    delete []pstuCodeIDInfo;
    pstuCodeIDInfo = NULL;
}

void CDlgLowRateWPAN::OnLowRateWPanBtnAttach() 
{
	// TODO: Add your control notification handler code here
	NET_IN_ATTACH_LOWRATEWPAN stuInParam = {sizeof(NET_IN_ATTACH_LOWRATEWPAN)};
    stuInParam.cbAttachLowRateWPANCB = OnLowRateWPanDataCallBack;
	stuInParam.dwUser = (LDWORD)this;
	NET_OUT_ATTACH_LOWRATEWPAN stuOutParam = {sizeof(NET_OUT_ATTACH_LOWRATEWPAN)};

	m_lHandle = CLIENT_AttachLowRateWPAN(m_lLoginId, &stuInParam, &stuOutParam, SDK_API_WAIT);
	if (m_lHandle == 0)
	{
		CString csInfo;
		csInfo.Format("%s:0x%08x", ConvertString("Attach LowRateWPAN data failed", DLG_LOWRATEWPAN), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	else
	{
		GetDlgItem(IDC_BTN_LOWRATEWPAN_ATTACH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LOWRATEWPAN_DETACH)->EnableWindow();
	}
}

void CDlgLowRateWPAN::OnLowRateWPanBtnDetach() 
{
	// TODO: Add your control notification handler code here
	if (m_lHandle == 0)
	{
		MessageBox(ConvertString("Please get attach handle first!", DLG_LOWRATEWPAN), ConvertString("Prompt"));
		return;
	}
	CLIENT_DetachLowRateWPAN(m_lHandle);
	GetDlgItem(IDC_BTN_LOWRATEWPAN_ATTACH)->EnableWindow();
	GetDlgItem(IDC_BTN_LOWRATEWPAN_DETACH)->EnableWindow(FALSE);
}

void CDlgLowRateWPAN::OnSelchangeCmbLowRateWPanIndex() 
{
	// TODO: Add your control notification handler code here
    int nSel = m_cmbIndex.GetCurSel();
    if (-1 == nSel)
    {
        return;
    }

    NET_CODEID_INFO* pInfo = (NET_CODEID_INFO*)m_cmbIndex.GetItemDataPtr(nSel);
    if (pInfo != NULL)
    {
        ShowCodeID(*pInfo);
    }
	//StuToDlg(m_cmbIndex.GetCurSel());
}

void CDlgLowRateWPAN::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
    if (m_lHandle != 0)
    {
        CLIENT_DetachLowRateWPAN(m_lHandle);
    }
}

void CDlgLowRateWPAN::OnLowRateWPanBtnTotal() 
{
	// TODO: Add your control notification handler code here
    NET_GET_CODEID_COUNT stuCount = {sizeof(stuCount)};

    int nRetLen = 0;
	BOOL bRet = CLIENT_QueryDevState(m_lLoginId, DH_DEVSTATE_GET_CODEID_COUNT, (char*)&stuCount, sizeof(stuCount), &nRetLen, SDK_API_WAIT);
    if (bRet)
    {
        SetDlgItemInt(IDC_LOWRATEWPAN_EDT_TOTALCODEIDNUM, stuCount.nCodeIDCount, FALSE);
    } 
    else
    {
		CString csErr;
		csErr.Format("%s %08x", ConvertString("Query CodeID Count failed:", DLG_LOWRATEWPAN), CLIENT_GetLastError());
		MessageBox(csErr, ConvertString("Prompt"));
        SetDlgItemInt(IDC_LOWRATEWPAN_EDT_TOTALCODEIDNUM, 0, FALSE);
    }
}
