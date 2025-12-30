// SearchPicture.cpp : 实现文件
//

#include "stdafx.h"
#include "HumanTrait.h"
#include "SearchPictureDlg.h"
#include <cmath>

// CSearchPictureDlg 对话框


IMPLEMENT_DYNAMIC(CSearchPictureDlg, CDialog)

CSearchPictureDlg::CSearchPictureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSearchPictureDlg::IDD, pParent)
{
    m_nChannelNum = 0;
    m_nTotalPage = 0;
    m_nCurPage =0;
    m_nIndexOfTrafficPicture  = 0;
    m_nTotalCountOfTrafficPicture  = 0;

    m_lFindPictureHandle = NULL;
    m_lDownloadHandle = NULL;
    m_lLoginHandle = NULL;
}

CSearchPictureDlg::~CSearchPictureDlg()
{
}

void CSearchPictureDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PIC, m_ctrPicList);
    DDX_Control(pDX, IDC_CMB_COAT_TYPE, m_cmbCoatType);
    DDX_Control(pDX, IDC_CMB_COAT_COLOR, m_cmbCoatColor);
    DDX_Control(pDX, IDC_CMB_TROUSERS_TYPE, m_cmbTrousersType);
    DDX_Control(pDX, IDC_CMBTROUSERS_COLOR, m_cmbTrousersColor);
    DDX_Control(pDX, IDC_CMB_BAG, m_cmbBag);
    DDX_Control(pDX, IDC_CMB_HAT, m_cmbHat);
    DDX_Control(pDX, IDC_BTN_DOWNLOAD, m_btnDownload);
    DDX_Control(pDX, IDC_BTN_PREVIOUS_PAGE, m_btnPreviousPage);
    DDX_Control(pDX, IDC_BTN_NEXT_PAGE, m_btnNextPage);
    DDX_Control(pDX, IDC_CMB_CHANNEL, m_cmbChannel);
}


BEGIN_MESSAGE_MAP(CSearchPictureDlg, CDialog)
    ON_BN_CLICKED(IDC_BTN_SEARCH, &CSearchPictureDlg::OnBnClickedBtnSearch)
    ON_BN_CLICKED(IDC_BTN_PREVIOUS_PAGE, &CSearchPictureDlg::OnBnClickedBtnPreviousPage)
    ON_BN_CLICKED(IDC_BTN_NEXT_PAGE, &CSearchPictureDlg::OnBnClickedBtnNextPage)
    ON_BN_CLICKED(IDC_BTN_DOWNLOAD, &CSearchPictureDlg::OnBnClickedBtnDownload)
    ON_WM_DESTROY()
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_START_DATE, &CSearchPictureDlg::OnDtnDatetimechangeDatetimepickerStartDate)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER_END_DATE, &CSearchPictureDlg::OnDtnDatetimechangeDatetimepickerEndDate)
END_MESSAGE_MAP()


// CSearchPictureDlg 消息处理程序

BOOL CSearchPictureDlg::PreTranslateMessage(MSG* pMsg)
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


BOOL CSearchPictureDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_ctrPicList.SetExtendedStyle(m_ctrPicList.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_ctrPicList.InsertColumn(0, ConvertString("Index"), LVCFMT_LEFT, 80);
    m_ctrPicList.InsertColumn(1, ConvertString("Time"), LVCFMT_LEFT, 120);
    m_ctrPicList.InsertColumn(2, ConvertString("CoatType"), LVCFMT_LEFT, 100);
    m_ctrPicList.InsertColumn(3, ConvertString("CoatColor"), LVCFMT_LEFT, 100);
    m_ctrPicList.InsertColumn(4, ConvertString("TrousersType"), LVCFMT_LEFT, 100);
    m_ctrPicList.InsertColumn(5, ConvertString("TrousersColor"), LVCFMT_LEFT, 100);
    m_ctrPicList.InsertColumn(6, ConvertString("HasHat"), LVCFMT_LEFT, 100);
    m_ctrPicList.InsertColumn(7, ConvertString("HasBag"), LVCFMT_LEFT, 100);
    
    m_cmbCoatType.InsertString(0, ConvertString("All"));
    m_cmbCoatType.InsertString(1, ConvertString("LongSleeve"));
    m_cmbCoatType.InsertString(2, ConvertString("ShortSleeve"));
    m_cmbTrousersType.InsertString(0, ConvertString("All"));
    m_cmbTrousersType.InsertString(1, ConvertString("Trousers"));
    m_cmbTrousersType.InsertString(2, ConvertString("Shorts"));
    m_cmbTrousersType.InsertString(3, ConvertString("Skirt"));
    m_cmbBag.InsertString(0, ConvertString("All"));
    m_cmbBag.InsertString(1, ConvertString("NotHasBag"));
    m_cmbBag.InsertString(2, ConvertString("HasBag"));
    m_cmbHat.InsertString(0, ConvertString("All"));
    m_cmbHat.InsertString(1, ConvertString("NotHasHat"));
    m_cmbHat.InsertString(2, ConvertString("HasHat"));
    char* g_pColor[] = {"All", "White", "Orange", "Pink", "Black", "Red", "Yellow", "Gray", "Blue", "Green", "Purple", "Brown"};
    for (int i = 0; i < 12; i++)
    {
        m_cmbCoatColor.InsertString(i, ConvertString(g_pColor[i]));
        m_cmbTrousersColor.InsertString(i, ConvertString(g_pColor[i]));
    }
    if (m_nChannelNum>=0 )
    {
        for (int i = 0; i<m_nChannelNum;i++)
        {
            CString strChnnael;
            strChnnael.Format("%d", i+1);
            m_cmbChannel.InsertString(i, strChnnael);
        }
    }
    m_cmbCoatType.SetCurSel(0);
    m_cmbTrousersType.SetCurSel(0);
    m_cmbCoatColor.SetCurSel(0);
    m_cmbTrousersColor.SetCurSel(0);
    m_cmbBag.SetCurSel(0);
    m_cmbHat.SetCurSel(0);
    m_cmbChannel.SetCurSel(0);
    g_SetWndStaticText(this);
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CSearchPictureDlg::OnDestroy()
{
    CleanUpResultOfLastTimeQuery();
    CDialog::OnDestroy();
}


void CSearchPictureDlg::CleanUpResultOfLastTimeQuery()
{
    m_nTotalPage = 0;
    m_nCurPage = 0;
    m_nIndexOfTrafficPicture = 0;
    m_nTotalCountOfTrafficPicture = 0;

    m_ctrPicList.DeleteAllItems();
    m_btnNextPage.EnableWindow(FALSE);
    m_btnPreviousPage.EnableWindow(FALSE);
    SetDlgItemText(IDC_EDIT_RECORD_COUNT, "");
    SetDlgItemText(IDC_EDIT_RECORD_CURRENT_PAGE, "");

    for (unsigned int i =0; i < m_vecHumanTraitPicture.size(); i++)
    {
        MEDIAFILE_HUMAN_TRAIT_INFO* pInfo = m_vecHumanTraitPicture[i];
        if (NULL != pInfo)
        {
            delete pInfo;
            pInfo = NULL;
        }
    }
    m_vecHumanTraitPicture.clear();

    if (0 != m_lFindPictureHandle)
    {
        CLIENT_FindCloseEx(m_lFindPictureHandle);
        m_lFindPictureHandle = 0;
    }
}

void CSearchPictureDlg::OnBnClickedBtnSearch()
{
	 // Whether the begin time is less than end time
    bool bRet = IsTimeCorrent();
    if (!bRet)
    {
        MessageBox(ConvertString("The begin time is bigger than end time, please input again!"), ConvertString("Prompt"));
        return;
    }
	
    CleanUpResultOfLastTimeQuery();

    m_lFindPictureHandle = StartFindPictures();
    if (0 == m_lFindPictureHandle)
    {
        MessageBox(ConvertString("Start find picture failed!"), ConvertString("Prompt"));
        return;
    }
    
    bRet = CLIENT_GetTotalFileCount(m_lFindPictureHandle, &m_nTotalCountOfTrafficPicture, NULL, MAX_TIMEOUT);
    if (!bRet)
    {
        MessageBox(ConvertString("Get total count of picture failed!"), ConvertString("Prompt"));
        return;
    }
    
    m_nCurPage = 1;
    SetDlgItemInt(IDC_EDIT_RECORD_COUNT, m_nTotalCountOfTrafficPicture);
    SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);

    int nQueryCount = SINGLE_QUERY_COUNT;
    if (m_nTotalCountOfTrafficPicture <= SINGLE_QUERY_COUNT)
    {
        nQueryCount = m_nTotalCountOfTrafficPicture ;
        m_nTotalPage = 1;
    }
    else
    {
        m_btnNextPage.EnableWindow(TRUE);
        m_nTotalPage = (int)ceil(m_nTotalCountOfTrafficPicture / (float)nQueryCount);
    }

    if (!DoFindPictures(nQueryCount))
    {
        MessageBox(ConvertString("Find picture failed!"), ConvertString("Prompt"));
        m_btnNextPage.EnableWindow(FALSE);
        return;
    }

    m_btnDownload.EnableWindow(TRUE);
}

void CSearchPictureDlg::OnBnClickedBtnPreviousPage()
{
    if(1 >= m_nCurPage)
    {
        return;
    }

    m_ctrPicList.DeleteAllItems();
    for (int i = 0; i < SINGLE_QUERY_COUNT; i++)
    {
        ShowPictureInfo(m_vecHumanTraitPicture[(m_nCurPage-2)*SINGLE_QUERY_COUNT+i], (m_nCurPage-2)*SINGLE_QUERY_COUNT+i+1);
    }
    m_nCurPage--;
    SetDlgItemInt(IDC_EDIT_RECORD_CURRENT_PAGE, m_nCurPage);
    if (m_nCurPage <= 1)
    {
        m_btnPreviousPage.EnableWindow(FALSE);
    }
    m_btnNextPage.EnableWindow(TRUE);
}   

void CSearchPictureDlg::OnBnClickedBtnNextPage()
{
    if(0 >= m_nCurPage)
    {
        return;
    }
    m_ctrPicList.DeleteAllItems();
    if (m_vecHumanTraitPicture.size() > m_nCurPage*SINGLE_QUERY_COUNT)
    {
        int nRemainCount = m_vecHumanTraitPicture.size() - m_nCurPage*SINGLE_QUERY_COUNT;
        int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT? SINGLE_QUERY_COUNT:nRemainCount;
        for (int i = 0; i < nQueryCount; i++)
        {
            ShowPictureInfo(m_vecHumanTraitPicture[m_nCurPage*SINGLE_QUERY_COUNT+i], m_nCurPage*SINGLE_QUERY_COUNT+i+1);
        }
    }
    else
    {
        int nRemainCount = m_nTotalCountOfTrafficPicture - m_nCurPage*SINGLE_QUERY_COUNT;
        int nQueryCount = nRemainCount > SINGLE_QUERY_COUNT? SINGLE_QUERY_COUNT:nRemainCount;
        if (!DoFindPictures(nQueryCount))
        {
            MessageBox(ConvertString("Find picture failed!"), ConvertString("Prompt"));
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


void CSearchPictureDlg::OnBnClickedBtnDownload()
{
    int nSelect = m_ctrPicList.GetNextItem(-1,LVNI_SELECTED);
    if(nSelect < 0)
    {
        MessageBox(ConvertString(ConvertString("Please select a list!")),  ConvertString("Prompt"));
        return;
    }

    CString strFilePathName;
    CFileDialog dlg(FALSE,"*.jpg","download.jpg",OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
        "jpg(*.jpg)|*.jpg|All File(*.jpg)|*.*||",this);
    if(dlg.DoModal() == IDOK)
    {
        strFilePathName = dlg.GetPathName();
    }
    else
    {
        return;
    }
    int nIndex = (m_nCurPage-1)*SINGLE_QUERY_COUNT + nSelect;
    DH_IN_DOWNLOAD_REMOTE_FILE stuInDownloadFile  = {sizeof(DH_IN_DOWNLOAD_REMOTE_FILE  )};
    stuInDownloadFile.pszFileName = m_vecHumanTraitPicture[nIndex]->szHumanPath;
    stuInDownloadFile.pszFileDst = strFilePathName.GetBuffer();

    DH_OUT_DOWNLOAD_REMOTE_FILE stuOutDownloadFile = {sizeof(DH_OUT_DOWNLOAD_REMOTE_FILE )};
    BOOL bRet = CLIENT_DownloadRemoteFile(m_lLoginHandle, &stuInDownloadFile, &stuOutDownloadFile);
    if (bRet == FALSE)
    {
        MessageBox(ConvertString("Download picture failed!"), ConvertString("Prompt"));
        return;
    }
}




LLONG CSearchPictureDlg::StartFindPictures(  )
{
    int nChannel = m_cmbChannel.GetCurSel();
    int nCoatColor = m_cmbCoatColor.GetCurSel();
    int nTrousersColor = m_cmbTrousersColor.GetCurSel();
    int nCoatType = m_cmbCoatType.GetCurSel();
    int nTrousersType = m_cmbTrousersType.GetCurSel();
    int nHat = m_cmbHat.GetCurSel();
    int nBag = m_cmbBag.GetCurSel();
    
    MEDIAFILE_HUMAN_TRAIT_PARAM stHumanTraitInfo = {sizeof(MEDIAFILE_HUMAN_TRAIT_PARAM )};
    stHumanTraitInfo.nChannel = nChannel; 
    CDateTimeCtrl* pCtrBeginDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_DATE);
    CDateTimeCtrl* pCtrBeginTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_START_TIME);
    CDateTimeCtrl* pCtrEndtDate = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_DATE);
    CDateTimeCtrl* pCtrEndTime = (CDateTimeCtrl*)GetDlgItem(IDC_DATETIMEPICKER_END_TIME);
    GetTimeFromTimeCtr(stHumanTraitInfo.stStartTime, pCtrBeginDate, pCtrBeginTime);
    GetTimeFromTimeCtr(stHumanTraitInfo.stEndTime, pCtrEndtDate, pCtrEndTime);
    stHumanTraitInfo.nCoatColorNum = 1;
    stHumanTraitInfo.nCoatTypeNum = 1;
    stHumanTraitInfo.nTrousersColorNum = 1;
    stHumanTraitInfo.nTrousersTypeNum= 1;

    stHumanTraitInfo.emCoatColor[0] = (EM_CLOTHES_COLOR)nCoatColor;
    stHumanTraitInfo.emTrousersColor[0] = (EM_CLOTHES_COLOR)nTrousersColor;
    stHumanTraitInfo.emCoatType[0] = (EM_COAT_TYPE)nCoatType;
    stHumanTraitInfo.emTrousersType[0] = (EM_TROUSERS_TYPE)nTrousersType;
    stHumanTraitInfo.emHasHat = (EM_HAS_HAT)nHat;
    stHumanTraitInfo.emHasBag = (EM_HAS_BAG)nBag;

    return CLIENT_FindFileEx(m_lLoginHandle, DH_FILE_QUERY_HUMAN_TRAIT, &stHumanTraitInfo, NULL, MAX_TIMEOUT);
}

bool CSearchPictureDlg::DoFindPictures( int nQueryCount )
{
    MEDIAFILE_HUMAN_TRAIT_INFO* pHumanTraitInfo = new MEDIAFILE_HUMAN_TRAIT_INFO[nQueryCount];
    memset(pHumanTraitInfo, 0, sizeof(MEDIAFILE_HUMAN_TRAIT_INFO)* nQueryCount);
    for (int i= 0; i<nQueryCount; i++)
    {
        pHumanTraitInfo[i].dwSize = sizeof(MEDIAFILE_HUMAN_TRAIT_INFO);
    }

    int nRet = CLIENT_FindNextFileEx(m_lFindPictureHandle, nQueryCount, pHumanTraitInfo, sizeof(MEDIAFILE_HUMAN_TRAIT_INFO)* nQueryCount, NULL, MAX_TIMEOUT);
    if (0 >= nRet)
    {
        delete[] pHumanTraitInfo;
        return false;
    }

    for (int i = 0; i < nRet; i++)
    { 
        m_nIndexOfTrafficPicture++;
        ShowPictureInfo(pHumanTraitInfo+i, m_nIndexOfTrafficPicture);
        MEDIAFILE_HUMAN_TRAIT_INFO* pInfo = new MEDIAFILE_HUMAN_TRAIT_INFO;
        memcpy(pInfo, pHumanTraitInfo+i, sizeof(MEDIAFILE_HUMAN_TRAIT_INFO));        
        m_vecHumanTraitPicture.push_back(pInfo);
    }
    delete[] pHumanTraitInfo;
    return true;
}


void CSearchPictureDlg::ShowPictureInfo(MEDIAFILE_HUMAN_TRAIT_INFO* pRecordList, int nBeginIndex)
{
    CString strIndex;
    CString strTime;
   
    char* g_pColor[] = {"Unknown", "White", "Orange", "Pink", "Black", "Red", "Yellow", "Gray", "Blue", "Green", "Purple", "Brown"};
    char* g_pCoatType[] = {"Unknown", "LongSleeve", "ShortSleeve"};
    char* g_pTrousersType[] = {"Unknown", "LongPants", "ShortPants", "Skirt"};
    char* g_pHasBag[] = {"Unknown", "NotHas","Has"};
    char* g_pHasHat[] = {"Unknown", "NotHas","Has"};

    strIndex.Format("%d", nBeginIndex);
    strTime.Format("%04d-%02d-%02d %02d:%02d:%02d",
        pRecordList->stStartTime.dwYear, pRecordList->stStartTime.dwMonth, 
        pRecordList->stStartTime.dwDay, pRecordList->stStartTime.dwHour, 
        pRecordList->stStartTime.dwMinute, pRecordList->stStartTime.dwSecond);
    
    int nCount =  m_ctrPicList.GetItemCount();
    m_ctrPicList.InsertItem(LVIF_TEXT|LVIF_STATE,nCount,strIndex,0,LVIS_SELECTED,0,0);
    m_ctrPicList.SetItemText(nCount,0, strIndex);
    m_ctrPicList.SetItemText(nCount,1, strTime);
    m_ctrPicList.SetItemText(nCount,2, ConvertString(g_pCoatType[pRecordList->stuHumanTrait.emCoatType]));
    m_ctrPicList.SetItemText(nCount,3, ConvertString(g_pColor[pRecordList->stuHumanTrait.emCoatColor]));
    m_ctrPicList.SetItemText(nCount,4, ConvertString(g_pTrousersType[pRecordList->stuHumanTrait.emTrousersType]));
    m_ctrPicList.SetItemText(nCount,5, ConvertString(g_pColor[pRecordList->stuHumanTrait.emTrousersColor]));
    m_ctrPicList.SetItemText(nCount,6, ConvertString(g_pHasHat[pRecordList->stuHumanTrait.emHasHat]));
    m_ctrPicList.SetItemText(nCount,7, ConvertString(g_pHasBag[pRecordList->stuHumanTrait.emHasBag]));
}

void CSearchPictureDlg::GetTimeFromTimeCtr(NET_TIME& stTime, CDateTimeCtrl* pCtrDate, CDateTimeCtrl* pCtrTime)
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


void CSearchPictureDlg::Init(LLONG lLoginID, int nChannel)
{
    if (lLoginID!= 0)
    {
        m_lLoginHandle = lLoginID;
        m_nChannelNum = nChannel;
    }
}

bool CSearchPictureDlg::IsTimeCorrent()
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



void CSearchPictureDlg::OnDtnDatetimechangeDatetimepickerStartDate(NMHDR *pNMHDR, LRESULT *pResult)
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

void CSearchPictureDlg::OnDtnDatetimechangeDatetimepickerEndDate(NMHDR *pNMHDR, LRESULT *pResult)
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
