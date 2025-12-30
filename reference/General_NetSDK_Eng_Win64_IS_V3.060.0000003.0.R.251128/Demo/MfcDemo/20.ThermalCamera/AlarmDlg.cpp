// AlarmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "AlarmDlg.h"
#include "ThermalCameraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define WM_FIREWARNING_FLOW         (WM_USER + 20)   
#define WM_FIREWARNING_PIC			(WM_USER + 21) 
/////////////////////////////////////////////////////////////////////////////
// CAlarmDlg dialog

int CALLBACK cbFireWarningCallBack(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
	if (EVENT_IVS_FIREWARNING != dwAlarmType || dwUser == NULL)
	{
		return -1;
	}

  	DEV_EVENT_FIREWARNING_INFO* pInfo =  new DEV_EVENT_FIREWARNING_INFO;
  	memcpy(pInfo, (DEV_EVENT_FIREWARNING_INFO*)pAlarmInfo, sizeof(DEV_EVENT_FIREWARNING_INFO));
 	PostMessage(((CAlarmDlg *)dwUser)->GetSafeHwnd(), WM_FIREWARNING_FLOW, (WPARAM)pInfo, (LPARAM)lAnalyzerHandle);
 
 	BYTE *pBufferPic = new BYTE[dwBufSize];
 	memcpy(pBufferPic, pBuffer, dwBufSize);
 	PostMessage(((CAlarmDlg *)dwUser)->GetSafeHwnd(), WM_FIREWARNING_PIC, (WPARAM)pBufferPic, (LPARAM)dwBufSize);

	return 0;
}

CAlarmDlg::CAlarmDlg(CWnd* pParent /*=NULL*/,LLONG iLoginId, int nChannelCount)
	: CDialog(CAlarmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAlarmDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_iLoginID = iLoginId;
	m_nAlarmIndex = -1;
	m_nFirewarningIndex = -1;
	m_lRealLoadHandle = 0;
	m_nChannelCount = nChannelCount;
	m_pPicture = NULL;
}


void CAlarmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlarmDlg)
	DDX_Control(pDX, IDC_LIST_ALARM, m_List);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_LIST_FIREWARNING, m_firewarning);
	DDX_Control(pDX, IDC_COMBO_CHN, m_combo_chn);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_Pic);
}


BEGIN_MESSAGE_MAP(CAlarmDlg, CDialog)
	//{{AFX_MSG_MAP(CAlarmDlg)
	ON_BN_CLICKED(IDC_STARTLISTEN, OnStartlisten)
    ON_MESSAGE(WM_ALARM_INFO, OnAlarmInfo)
	ON_MESSAGE(WM_FIREWARNING_FLOW, &CAlarmDlg::OnFirewarningFlowDate)
	ON_MESSAGE(WM_FIREWARNING_PIC, &CAlarmDlg::OnFirewarningPic)
	ON_BN_CLICKED(IDC_STOPLISTEN, OnStoplisten)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_STARTLISTEN2, &CAlarmDlg::OnBnClickedStartlisten2)
	ON_BN_CLICKED(IDC_STOPLISTEN2, &CAlarmDlg::OnBnClickedStoplisten2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlarmDlg message handlers

BOOL CAlarmDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_STATIC_MESSAGE)->SetWindowText("");
	m_pPicture = NULL;

	for (int i = 0 ;i<m_nChannelCount;i++)
	{
		CString str;
		str.Format("%d",i);
		m_combo_chn.AddString(str);
	}
	m_combo_chn.SetCurSel(0);

	g_SetWndStaticText(this);
	m_List.SetExtendedStyle(m_List.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_List.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,60);
	m_List.InsertColumn(1,ConvertString("Alarm type"),LVCFMT_LEFT,90);
	m_List.InsertColumn(2,ConvertString("Channel") , LVCFMT_LEFT, 70);
	m_List.InsertColumn(3,ConvertString("Info"), LVCFMT_LEFT, 800);     //将原来的400改成了500，就能在demo里面完整地显示信息了

	m_firewarning.SetExtendedStyle(m_firewarning.GetExtendedStyle() | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_firewarning.InsertColumn(0,ConvertString("Index"),LVCFMT_LEFT,60);
	m_firewarning.InsertColumn(1,ConvertString("Alarm type"),LVCFMT_LEFT,90);
	m_firewarning.InsertColumn(2,ConvertString("Channel") , LVCFMT_LEFT, 70);
	m_firewarning.InsertColumn(3,ConvertString("Info"), LVCFMT_LEFT, 500);     //将原来的400改成了500，就能在demo里面完整地显示信息了
	//UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// 每个报警事件的信息，作为投递消息的载体，用户管理内存
typedef struct tagAlarmInfo
{
    long	lCommand;
    char*	pBuf;
    DWORD	dwBufLen;
    
    tagAlarmInfo()
    {
        lCommand = 0;
        pBuf = NULL;
        dwBufLen = 0;
    }
    
    ~tagAlarmInfo()
    {
        if (pBuf)
        {
            delete []pBuf;
			pBuf = NULL;
        }
    }
}AlarmInfo;

BOOL CALLBACK MessCallBack(LONG lCommand, LLONG lLoginID, char *pBuf,
                           DWORD dwBufLen, char *pchDVRIP, LONG nDVRPort, LDWORD dwUser)
{
    if(!dwUser) 
    {
        return FALSE;
    }

     CAlarmDlg* dlg = (CAlarmDlg*)dwUser;
     BOOL bRet = FALSE;
     if (dlg != NULL && dlg->GetSafeHwnd())
     {
         AlarmInfo* pInfo = new AlarmInfo;
         if (!pInfo)
         {
             return FALSE;
         }
         pInfo->lCommand = lCommand;
         pInfo->pBuf = new char[dwBufLen];
         if (!pInfo->pBuf)
         {
             delete pInfo;
 			pInfo = NULL;
             return FALSE;
         }
         memcpy(pInfo->pBuf, pBuf, dwBufLen);
         pInfo->dwBufLen = dwBufLen;
         dlg->PostMessage(WM_ALARM_INFO, (WPARAM)pInfo, (LPARAM)0);
     }
    
    return bRet;	
}
void CAlarmDlg::OnStartlisten() 
{
    if (!m_iLoginID)
    {
        MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
        return;
    }

// 	BOOL bret = UpdateData();
// 	if (!bret)
// 	{
// 		return;
// 	}
	m_nAlarmIndex = 0;
    // 设置回调接口
    CLIENT_SetDVRMessCallBack(MessCallBack, (LDWORD)this);
    
    BOOL bRet = CLIENT_StartListenEx(m_iLoginID);
    if (!bRet)
    {
        MessageBox(ConvertString("Subscribe failed"), ConvertString("Prompt"));
        return;
    }
    
    GetDlgItem(IDC_STARTLISTEN)->EnableWindow(FALSE);
    GetDlgItem(IDC_STOPLISTEN)->EnableWindow(TRUE);
	return;
}


LRESULT CAlarmDlg::OnAlarmInfo(WPARAM wParam, LPARAM lParam)
{
    AlarmInfo* pAlarmInfo = (AlarmInfo*)wParam;
    if (!pAlarmInfo || !pAlarmInfo->pBuf || pAlarmInfo->dwBufLen <= 0)
    {
		if (NULL != pAlarmInfo)
		{
			delete pAlarmInfo;
			pAlarmInfo = NULL;
		}
        return -1;
    }
    m_List.SetRedraw(FALSE);
    SYSTEMTIME st;
    GetLocalTime(&st);

    if (pAlarmInfo->lCommand == DH_ALARM_HEATIMG_TEMPER)
    {
        ALARM_HEATIMG_TEMPER_INFO *AlarmHeatimgInfo = (ALARM_HEATIMG_TEMPER_INFO*)pAlarmInfo->pBuf;
		if (m_List.GetItemCount() > MAX_MSG_SHOW)
		{
			m_List.DeleteItem(MAX_MSG_SHOW);
		}

		CString strIndex;
		strIndex.Format("%d",m_nAlarmIndex + 1);
        //m_List.InsertItem(0, NULL);
		LV_ITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.pszText = _T("");
		lvi.iImage = 0;
		lvi.iItem = 0;

		m_List.InsertItem(&lvi);
        
	    m_List.SetItemText(0, 0, strIndex);
        m_List.SetItemText(0, 1, ConvertString("Thermal temperature abnormal event alarm"));
        CString str;
        str.Format("%04d-%02d-%02d %02d:%02d:%02d",st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        str.Format("channel:%d",AlarmHeatimgInfo->nChannel);
        m_List.SetItemText(0, 2, str);
        str.Format("x:%d y:%d",AlarmHeatimgInfo->stCoordinate.nx,AlarmHeatimgInfo->stCoordinate.ny);
        CString str1;
        str1 = AlarmHeatimgInfo->szName;
        str = ConvertString("name:") + str1+" ";
        str1.Format("AlarmId:%d",AlarmHeatimgInfo->nAlarmId);
        str = str + str1+" ";
        str1 =ConvertString("Result:");
        if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_UNKNOWN)
        {
            str1 += ConvertString("unknown");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_VAL)
        {
            str1+= ConvertString("concrete value");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_MAX)
        {
            str1 += ConvertString("max");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_MIN)
        {
            str1 += ConvertString("min");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_AVR)
        {
            str1 +=ConvertString("average");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_STD)
        {
            str1 +=ConvertString("standard");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_MID)
        {
            str1 +=ConvertString("middle");
        }
        else if (AlarmHeatimgInfo->nAlarmId == NET_RADIOMETRY_RESULT_ISO)
        {
            str1 +=ConvertString("ISO");
        }
        str +=str1+" " ;
        str1 = ConvertString("condition:");
        if (AlarmHeatimgInfo->nAlarmContion == NET_RADIOMETRY_ALARMCONTION_UNKNOWN)
        {
            str1 +=ConvertString("unknown");
        }
        else if (AlarmHeatimgInfo->nAlarmContion == NET_RADIOMETRY_ALARMCONTION_BELOW)
        {
            str1 += ConvertString("under");
        }
        else if (AlarmHeatimgInfo->nAlarmContion == NET_RADIOMETRY_ALARMCONTION_MATCH)
        {
            str1 += ConvertString("equal");
        }
        else if (AlarmHeatimgInfo->nAlarmContion == NET_RADIOMETRY_ALARMCONTION_ABOVE)
        {
            str1 += ConvertString("above");
        }
        str += str1+" ";
        str1.Format("value:%f",AlarmHeatimgInfo->fTemperatureValue);   
        str += str1+" ";
        str1 = ConvertString("unit:");
        if (AlarmHeatimgInfo->nTemperatureUnit == TEMPERATURE_UNIT_UNKNOWN)
        {
             str1+=ConvertString("unknown");
        }
        else if (AlarmHeatimgInfo->nTemperatureUnit == TEMPERATURE_UNIT_CENTIGRADE)
        {
            str1 +=ConvertString("Celsius");
        }
        else if (AlarmHeatimgInfo->nTemperatureUnit == TEMPERATURE_UNIT_FAHRENHEIT)
        {
            str1 +=ConvertString("Fahrenheit");
        }
        str += str1 + " ";
        str1.Format("Presentid:%d",AlarmHeatimgInfo->nPresetID);   
        str +=str1;
        m_List.SetItemText(0,3,str);
        m_nAlarmIndex++;
    }
	else if (pAlarmInfo->lCommand == DH_ALARM_FIREWARNING_INFO)
	{
		// 热成像火情信息上报事件
		ALARM_FIREWARNING_INFO_DETAIL *AlarmFirewarningInfo = (ALARM_FIREWARNING_INFO_DETAIL*)pAlarmInfo->pBuf;
		if (m_List.GetItemCount() > MAX_MSG_SHOW)
		{
			m_List.DeleteItem(MAX_MSG_SHOW);
		}

		CString strIndex;
		strIndex.Format("%d",m_nAlarmIndex + 1);
		LV_ITEM lvi;
		lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		lvi.iSubItem = 0;
		lvi.pszText = _T("");
		lvi.iImage = 0;
		lvi.iItem = 0;

		m_List.InsertItem(&lvi);
		//m_List.InsertItem(0, NULL);

		m_List.SetItemText(0, 0, strIndex);
		m_List.SetItemText(0, 1, ConvertString("Alarm Firewarning info"));
		CString str;
		CString str1;
		CString strTime;
		strTime.Format("%04d-%02d-%02d %02d:%02d:%02d",st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		str = ConvertString("channel");
		str1.Format(":%d",AlarmFirewarningInfo->nChannel);
		str = str + str1+" ";
		m_List.SetItemText(0, 2, str);
		str = str + strTime+" ";
		str1.Format("WarningInfoCount:%d",AlarmFirewarningInfo->nWarningInfoCount);
		str = str + str1+" ";

		if (AlarmFirewarningInfo->nWarningInfoCount > 0)
		{
			str1 = ConvertString("PresetId");//预置点编号
			str = str + str1+" ";
			str1.Format(": %d",AlarmFirewarningInfo->stuFireWarningInfo[0].nPresetId);
			str = str + str1+" ";
			if (AlarmFirewarningInfo->stuFireWarningInfo[0].nTemperatureUnit == NET_TEMPERATURE_UNIT_CENTIGRADE)//摄氏度
			{
				str1 = ConvertString("centigrade");
				str = str + str1+" ";
			}
			else if (AlarmFirewarningInfo->stuFireWarningInfo[0].nTemperatureUnit == NET_TEMPERATURE_UNIT_FAHRENHEIT)//华氏度
			{
				str1 = ConvertString("fahrenheit");
				str = str + str1+" ";
			}
			str = str + ConvertString("Temperature");
			str1.Format(": %f",AlarmFirewarningInfo->stuFireWarningInfo[0].fTemperature);//最高点温度值
			str = str + str1+" ";
			str = str + ConvertString("Distance");
			str1.Format(": %d",AlarmFirewarningInfo->stuFireWarningInfo[0].nDistance);//着火点距离
			str = str + str1+" ";

			double dGPSLongitude = UINTToLONLAT(AlarmFirewarningInfo->stuFireWarningInfo[0].stuGpsPoint.dwLongitude, 1);
			double dGPSLatitude = UINTToLONLAT(AlarmFirewarningInfo->stuFireWarningInfo[0].stuGpsPoint.dwLatidude, 2);

			str = str + ConvertString("Longitude");
			str1.Format(": %f",dGPSLongitude);//经度
			str = str + str1+" ";

			str = str + ConvertString("Latitude");
			str1.Format(": %f",dGPSLatitude);//纬度
			str = str + str1+" ";

			str = str + ConvertString("altitude");
			str1.Format(": %f",AlarmFirewarningInfo->stuFireWarningInfo[0].fAltitude);//高度
			str = str + str1+" ";

			str = str + ConvertString("FSID");
			str1.Format(": %d",AlarmFirewarningInfo->stuFireWarningInfo[0].nFSID);
			str = str + str1+" ";
		}
		m_List.SetItemText(0,3,str);
		m_nAlarmIndex++;
	}
	m_List.SetRedraw(TRUE);
    //UpdateData(FALSE);
	delete pAlarmInfo;
	pAlarmInfo = NULL;
    return 0;
}

double CAlarmDlg::UINTToLONLAT(UINT unParam, int nType)
{
	double dtemp = 0;
	if (nType == 1)//unint -> 经度
	{
		dtemp = ((unParam - 0) - 180.0 * 1000000.0)/1000000.0;
	}
	else if(nType == 2)//unint -> 纬度
	{
		//纬度
		dtemp = ((unParam - 0) - 90.0 * 1000000.0)/1000000.0;
	}
	return dtemp;
}

void CAlarmDlg::OnStoplisten() 
{
    if(m_iLoginID == 0)
    {
        MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
        return;
    }
   BOOL bret = CLIENT_StopListen(m_iLoginID);
	if (!bret)
	{
		MessageBox(ConvertString("Stop Subscribe failed"), ConvertString("Prompt"));
		return;
	}

    GetDlgItem(IDC_STARTLISTEN)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOPLISTEN)->EnableWindow(FALSE);
}

void CAlarmDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	if(m_pPicture != NULL)
	{
		m_pPicture->Release();
	}	
	
	CLIENT_StopListen(m_iLoginID);
	CLIENT_StopLoadPic(m_lRealLoadHandle);
	m_lRealLoadHandle = 0;
}

LRESULT CAlarmDlg::OnFirewarningPic(WPARAM wParam, LPARAM lParam)
{
	if (0 == wParam || lParam == 0)
	{
		return 0;
	}

	BYTE *pBufferPic = (BYTE*)wParam;
 	DWORD dwBufSize = (DWORD)lParam;
 	m_Pic.SetImageDate(pBufferPic,dwBufSize);
	//定义变量存储图片信息

/*
	RECT rect;
	GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(&rect);

	CDC* pDC = NULL;
	//pDC = this->GetWindowDC();
	pDC=GetDlgItem(IDC_STATIC_PICTURE)->GetDC();
	if (NULL == pDC)
	{
		return -1;
	}

	if (m_pPicture != NULL)
	{
		m_pPicture->Release();
		m_pPicture = NULL;
	}

	HGLOBAL hMem  = GlobalAlloc(GMEM_MOVEABLE, dwBufSize);
	if (hMem == NULL) 
	{
		return -1;
	}

	BYTE *pTmpBuf = (BYTE *)GlobalLock(hMem);
	memcpy(pTmpBuf, pBufferPic, dwBufSize);
	GlobalUnlock(hMem);


	IStream* pStream = NULL;
	CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (pStream == NULL)
	{
		ReleaseDC(pDC);
		GlobalFree(hMem);
		return -1;
	}

	// 加载图片
	HRESULT hr = S_FALSE;
	hr = OleLoadPictureEx(pStream, dwBufSize, TRUE, IID_IPicture, LP_DEFAULT, LP_DEFAULT, LP_DEFAULT,(LPVOID*)&m_pPicture);
	if (NULL == m_pPicture)
	{
		ReleaseDC(pDC);
		pStream->Release();
		GlobalFree(hMem);
		return -1;
	}

	//显示全图
	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	m_pPicture->get_Width(&hmWidth); 
	m_pPicture->get_Height(&hmHeight); 

	hr = m_pPicture->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	ReleaseDC(pDC);

	pStream->Release();
	GlobalFree(hMem);
*/

	//保存图片
// 	SYSTEMTIME st;
// 	GetLocalTime(&st);
// 
// 	GUID guid = {0};
// 	HRESULT hr =  CoCreateGuid(&guid);
// 	CString strTime;
// 	strTime.Format("%04d%02d%02d%02d%02d%02d%03d",st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
// 	std::string strGUID(strTime);
// 	strGUID.append(GuidToString(guid));	
// 
// 	std::string strCfgFolder = GetDataFolder();
// 	std::string strJpgFile = strCfgFolder + strGUID + std::string(".jpg");	
// 	::CreateDirectory(strCfgFolder.c_str(), NULL);	
// 
// 	FILE* fp = fopen(strJpgFile.c_str(), "wb");
// 	if (fp == NULL)
// 	{
// 		OutputDebugString("Save picture failed!");	
// 		GetDlgItem(IDC_STATIC_MESSAGE)->SetWindowText(ConvertString("Save picture failed!"));
// 		delete[] pBufferPic;
// 		pBufferPic = NULL;
// 		return -1;
// 	}
// 	else
// 	{
// 		GetDlgItem(IDC_STATIC_MESSAGE)->SetWindowText("");
// 	}
// 	
// 
// 	fwrite(pBufferPic, lParam, 1, fp);
// 	fclose(fp);
// 
// 	m_Pic.SetImageFile(strJpgFile.c_str());	

	delete[] pBufferPic;
	pBufferPic = NULL;
	return 0;
}
LRESULT CAlarmDlg::OnFirewarningFlowDate(WPARAM wParam, LPARAM lParam)
{
	if (0 == wParam /*|| lParam == 0*/)
	{
		return 0;
	}
	m_firewarning.SetRedraw(FALSE);
	if (m_firewarning.GetItemCount() > MAX_MSG_SHOW)
	{
		m_firewarning.DeleteItem(MAX_MSG_SHOW);
	}

	SYSTEMTIME st;
	GetLocalTime(&st);

	DEV_EVENT_FIREWARNING_INFO* pInfo = (DEV_EVENT_FIREWARNING_INFO*)wParam;

	CString strIndex;
	strIndex.Format("%d",m_nFirewarningIndex + 1);
	LV_ITEM lvi;
	lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
	lvi.iSubItem = 0;
	lvi.pszText = _T("");
	lvi.iImage = 0;
	lvi.iItem = 0;

	m_firewarning.InsertItem(&lvi);
	//m_firewarning.InsertItem(0, NULL);

	m_firewarning.SetItemText(0, 0, strIndex);
	m_firewarning.SetItemText(0, 1, ConvertString("Firewarning"));
	CString str;
	CString str1;
	CString strTime;
	strTime.Format("%04d-%02d-%02d %02d:%02d:%02d",pInfo->stuFileInfo.stuFileTime.dwYear, pInfo->stuFileInfo.stuFileTime.dwMonth, pInfo->stuFileInfo.stuFileTime.dwDay, pInfo->stuFileInfo.stuFileTime.dwHour, pInfo->stuFileInfo.stuFileTime.dwMinute, pInfo->stuFileInfo.stuFileTime.dwSecond);

	str = ConvertString("channel");
	str1.Format(":%d",pInfo->nChannelID);
	str = str + str1+" ";

	m_firewarning.SetItemText(0/*m_nFirewarningIndex*/, 2, str);
	str = str + strTime + " ";
	str = str + ConvertString("FSID");
	str1.Format(":%d",pInfo->nFSID);
	str = str + str1+" ";
	if (pInfo->nAction == 1)
	{
		str = str + ConvertString("Start") + " ";
	} 
	else
	{
		str = str + ConvertString("Stop") + " ";
	}
	
	switch(pInfo->emPicType)
	{
	case EM_PIC_UNKNOWN:// 未知
		str = str + ConvertString("PicType:unknown") + " ";
		break;
	case EM_PIC_NONE:// 无
		str = str + ConvertString("PicType:none") + " ";
		break;
	case EM_PIC_OVERVIEW:// 全景图
		str = str + ConvertString("PicType:overview") + " ";
		break;
	case EM_PIC_DETAIL:// 细节图
		str = str + ConvertString("PicType:detail") + " ";
		break;
	default:
		str = str + ConvertString("PicType:unknown") + " ";
		break;
	}
	m_firewarning.SetItemText(0,3,str);
	m_nFirewarningIndex++;
	m_firewarning.SetRedraw(TRUE);

	delete pInfo;
	pInfo = NULL;
	return 0;
}

std::string CAlarmDlg::GuidToString(const GUID &guid)
{
	char buf[64] = {0};
	_snprintf(
		buf,
		sizeof(buf),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return std::string(buf);
}

std::string CAlarmDlg::GetDataFolder()
{
	TCHAR szFilePath[MAX_PATH+1]={0};
	GetModuleFileName(NULL,szFilePath,MAX_PATH);
	(_tcsrchr(szFilePath,_T('\\')))[1]=0;
	CString str_path = szFilePath;
		
	std::string strCfgFilePath(szFilePath);
	std::string strCfgDefaultFolder;
	//GetModulePath("dhnetsdk.dll", strCfgFilePath);
	strCfgDefaultFolder = strCfgFilePath;
	strCfgFilePath.append("Config.ini");
	strCfgDefaultFolder.append("PCStore\\");
	char szFolder[1024] = {0};
	::GetPrivateProfileString("CFG", "DATA_PATH", strCfgDefaultFolder.c_str(), szFolder, 1024, strCfgFilePath.c_str());
	return std::string(szFolder);
}

void CAlarmDlg::OnBnClickedStartlisten2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_iLoginID)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		return;
	}
// 	BOOL bret = UpdateData();
// 	if (!bret)
// 	{
// 		return;
// 	}
	int nChannelID = m_combo_chn.GetCurSel();
	m_nFirewarningIndex = 0;

	//MessageBox("Subscribe ok.", ConvertString("Prompt"));
	//#define EVENT_IVS_FIREWARNING					0x00000245		// 火警事件(对应 DEV_EVENT_FIREWARNING_INFO)
	m_lRealLoadHandle = CLIENT_RealLoadPictureEx(m_iLoginID, nChannelID, EVENT_IVS_ALL, TRUE, cbFireWarningCallBack, (LDWORD)this, NULL);
	if (m_lRealLoadHandle == 0)
	{		
		MessageBox(ConvertString("Subscribe intelligent failed!"), ConvertString("Prompt"));
		return;
	}
	GetDlgItem(IDC_STARTLISTEN2)->EnableWindow(FALSE);
	GetDlgItem(IDC_STOPLISTEN2)->EnableWindow(TRUE);
}

void CAlarmDlg::OnBnClickedStoplisten2()
{
	// TODO: 在此添加控件通知处理程序代码

	if(m_iLoginID == 0)
	{
		MessageBox(ConvertString("Please login device first"), ConvertString("Prompt"));
		return;
	}
	BOOL bret = CLIENT_StopLoadPic(m_lRealLoadHandle);
	if (!bret)
	{
		MessageBox(ConvertString("Stop Subscribe failed"), ConvertString("Prompt"));
		return;
	}
	m_lRealLoadHandle = 0;


	//MessageBox(ConvertString("Stop Subscribe ok"), ConvertString("Prompt"));

	GetDlgItem(IDC_STARTLISTEN2)->EnableWindow(TRUE);
	GetDlgItem(IDC_STOPLISTEN2)->EnableWindow(FALSE);
}
