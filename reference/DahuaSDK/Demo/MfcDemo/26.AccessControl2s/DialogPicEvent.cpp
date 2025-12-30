// DialogPicEvent.cpp : 实现文件
//
#include <gdiplus.h>
#include <windows.h>
#include "stdafx.h"
#include "AccessControl2s.h"
#include "DialogPicEvent.h"
using namespace Gdiplus;


// CDialogPicEvent 对话框

IMPLEMENT_DYNAMIC(CDialogPicEvent, CDialog)

CDialogPicEvent::CDialogPicEvent(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPicEvent::IDD, pParent)
{
	m_bListenEvent = FALSE;
	m_chnCount = 1;
	InitializeCriticalSection(&m_csLock);
}

CDialogPicEvent::~CDialogPicEvent()
{
	DeleteCriticalSection(&m_csLock);
}

void CDialogPicEvent::setChnCount(int count)
{
	m_chnCount = count;
}

void CDialogPicEvent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_PICEVENT_CHN, m_picChannel);
	DDX_Control(pDX, IDC_LIST_PIC_EVENT_INFO, m_listPicEventAlarm);
}


BEGIN_MESSAGE_MAP(CDialogPicEvent, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_START_PIC, &CDialogPicEvent::OnBnClickedButtonStartPic)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PIC, &CDialogPicEvent::OnBnClickedButtonStopPic)
	// 添加消息
	ON_MESSAGE(WM_PICEVENTALARM_INFO, OnPicEventAlarmInfo)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDialogPicEvent 消息处理程序
BOOL CDialogPicEvent::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);

	CString title;
	title.Format("%s", ConvertString("OpenDoorEvent"));
	this->SetWindowText(title);

	InitCtrls();

	InitPicControl();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDialogPicEvent::InitCtrls()
{
	for (int i = 0; i < m_chnCount; i++)
	{
		char szContent[8] = {0};
		_snprintf(szContent, sizeof(szContent), "%d", i + 1);
		m_picChannel.AddString(szContent);
		m_picChannel.SetItemData(i, (DWORD)i);
	}
	m_picChannel.SetCurSel(0);


	m_listPicEventAlarm.SetExtendedStyle(m_listPicEventAlarm.GetExtendedStyle()|LVS_EX_FULLROWSELECT);  
	m_listPicEventAlarm.SetExtendedStyle(m_listPicEventAlarm.GetExtendedStyle()|LVS_EX_GRIDLINES);

	std::vector<ColDes> vecTitles;
	vecTitles.push_back(ColDesObj("UserID",100));
	vecTitles.push_back(ColDesObj("CardNo",100));
	vecTitles.push_back(ColDesObj("Time",100));
	vecTitles.push_back(ColDesObj("EventInfo",200));

	int nColCount = vecTitles.size();
	LV_COLUMN lvc;
	lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM;
	lvc.fmt=LVCFMT_LEFT;
	for(int j = 0; j < nColCount; j++) 
	{
		lvc.pszText = (char*)(vecTitles[j].strColTitle.c_str());
		lvc.cx = vecTitles[j].nColWidth;
		lvc.iSubItem = j;
		m_listPicEventAlarm.InsertColumn(j, &lvc);
	}

}


// 提取的事件信息字段
struct PicEventInfo
{
	DWORD	dwEventType;
	BYTE*	pBuffer;
	DWORD	dwBufSize;

	// 缓存事件回调信息
	int							nChannelID;
	char						szUserID[DH_COMMON_STRING_64];
	char						szCardNo[DH_MAX_CARDNO_LEN];
	char						szUTCTime[128];
	char						szEventTimeID[128];
	NET_ACCESS_DOOROPEN_METHOD  emOpenMethod;
	BOOL						bStatus;
	DEV_ACCESS_CTL_IMAGE_INFO	stuImageInfo[6];
	int							nImageInfoCount;

	PicEventInfo()
	{
		dwEventType = 0;
		pBuffer = NULL;
		dwBufSize = 0;

		nChannelID = 0;
		memset(szUserID, 0, sizeof(szUserID));
		memset(szCardNo, 0, sizeof(szCardNo));
		memset(szUTCTime, 0, sizeof(szUTCTime));
		memset(szEventTimeID, 0, sizeof(szEventTimeID));
		emOpenMethod = NET_ACCESS_DOOROPEN_METHOD_UNKNOWN;
		bStatus = FALSE;
	}

	~PicEventInfo()
	{
		if (NULL != pBuffer)
		{
			delete []pBuffer;
			pBuffer = NULL;
		}
	}
};

// 将原事件的信息深拷贝到StuEventInfo结构体中
BOOL copyEventInfo(DWORD nEventType, void* pEventInfo, PicEventInfo* pStuEventInfo)
{
	if (NULL == pEventInfo || NULL == pStuEventInfo)
	{
		return FALSE;
	}

	switch(nEventType)
	{		
		case EVENT_IVS_ACCESS_CTL:
		{
				DEV_EVENT_ACCESS_CTL_INFO *pInfo = (DEV_EVENT_ACCESS_CTL_INFO*)pEventInfo;
				if (NULL != pInfo)
				{
					strncpy(pStuEventInfo->szUserID, pInfo->szUserID, DH_COMMON_STRING_64);

					strncpy(pStuEventInfo->szCardNo, pInfo->szCardNo, DH_MAX_CARDNO_LEN);

					_snprintf(pStuEventInfo->szUTCTime, 128,
								"%04d-%02d-%02d %02d:%02d:%02d",
								pInfo->UTC.dwYear, pInfo->UTC.dwMonth, pInfo->UTC.dwDay, pInfo->UTC.dwHour, pInfo->UTC.dwMinute, pInfo->UTC.dwSecond);

					_snprintf(pStuEventInfo->szEventTimeID, 128,
								"%04d_%02d_%02d_%02d_%02d_%02d",
								pInfo->UTC.dwYear, pInfo->UTC.dwMonth, pInfo->UTC.dwDay, pInfo->UTC.dwHour, pInfo->UTC.dwMinute, pInfo->UTC.dwSecond);

					pStuEventInfo->nChannelID = pInfo->nChannelID;

					pStuEventInfo->emOpenMethod = pInfo->emOpenMethod;

					pStuEventInfo->bStatus = pInfo->bStatus;

					pStuEventInfo->nImageInfoCount = pInfo->nImageInfoCount;
					for (int i = 0; i < pStuEventInfo->nImageInfoCount; i++)
					{
						pStuEventInfo->stuImageInfo[i].emType = pInfo->stuImageInfo[i].emType;
						pStuEventInfo->stuImageInfo[i].nLength = pInfo->stuImageInfo[i].nLength;
						pStuEventInfo->stuImageInfo[i].nOffSet = pInfo->stuImageInfo[i].nOffSet;
					}
			}
		}
		break;

		default:
		return FALSE;
	}

	return TRUE;
}

int CALLBACK AnalyzerDataCallBack(LLONG lAnalyzerHandle, DWORD dwAlarmType, void* pAlarmInfo, BYTE *pBuffer, DWORD dwBufSize, LDWORD dwUser, int nSequence, void *reserved)
{
	if(!dwUser || NULL==pBuffer || dwBufSize <= 0)
	{
		return FALSE;
	}

	CDialogPicEvent* dlg = (CDialogPicEvent*)dwUser;
	if (dlg != NULL && dlg->GetSafeHwnd())
	{
		if (EVENT_IVS_ACCESS_CTL == dwAlarmType)
		{
			PicEventInfo* pInfo = NEW PicEventInfo;
			if (!pInfo)
			{
				return FALSE;
			}
			memset(pInfo, 0, sizeof(PicEventInfo));
			pInfo->dwEventType = dwAlarmType;
			//pInfo->pAlarmInfo = pAlarmInfo;
			pInfo->pBuffer = NEW BYTE[dwBufSize];
			if (!pInfo->pBuffer)
			{
				delete pInfo;
				pInfo = NULL;
				return 0;
			}
			memcpy(pInfo->pBuffer, pBuffer, dwBufSize);
			pInfo->dwBufSize = dwBufSize;

			// 提取深拷贝事件信息
			if (!copyEventInfo(dwAlarmType, pAlarmInfo, pInfo))
			{
				delete pInfo;
				pInfo = NULL;
				return 0;
			}

			if (dlg->PostMessage(WM_PICEVENTALARM_INFO, (WPARAM)pInfo, (LPARAM)0)<0)
			{
				if (NULL != pInfo)
				{
					delete pInfo;
					pInfo = NULL;
					return 0;
				}
			}

		}
	}

	return 0;
}

LRESULT CDialogPicEvent::OnPicEventAlarmInfo(WPARAM wParam, LPARAM lParam)
{
	CLock lock(m_csLock);

	if (!m_bListenEvent)
	{
		return -1;
	}

	PicEventInfo* pEventInfo = (PicEventInfo*)wParam;
	if (NULL == pEventInfo || NULL == pEventInfo->pBuffer || pEventInfo->dwBufSize <= 0)
	{
		if (NULL != pEventInfo)
		{
			if (NULL != pEventInfo->pBuffer)
			{
				delete []pEventInfo->pBuffer;
				pEventInfo->pBuffer = NULL;
			}
			delete pEventInfo;
			pEventInfo = NULL;
		}
		return -1;
	}

	clearWindows();

	m_listPicEventAlarm.SetRedraw(FALSE);


	switch(pEventInfo->dwEventType)
	{
		case EVENT_IVS_ACCESS_CTL:
		{
			if (m_listPicEventAlarm.GetItemCount() > 10)
			{
				m_listPicEventAlarm.DeleteItem(10);
			}
			LV_ITEM lvi;
			lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iSubItem = 0;
			lvi.pszText = _T("");
			lvi.iImage = 0;
			lvi.iItem = 0;
			m_listPicEventAlarm.InsertItem(&lvi);

			CString strUserID;
			strUserID.Format("%s", pEventInfo->szUserID);
			m_listPicEventAlarm.SetItemText(0, 0, strUserID);

			CString strCardNo;
			strCardNo.Format("%s", pEventInfo->szCardNo);
			m_listPicEventAlarm.SetItemText(0, 1, strCardNo);

			CString strUTC;
			strUTC.Format("%s", pEventInfo->szUTCTime);
			m_listPicEventAlarm.SetItemText(0, 2, strUTC);

			CString strInfo;
			std::string strMethod; 
			switch (pEventInfo->emOpenMethod)
			{
				case NET_ACCESS_DOOROPEN_METHOD_CARD:
					strMethod = "Card(卡)";
					break;
				case NET_ACCESS_DOOROPEN_METHOD_FACE_RECOGNITION:
					strMethod = "TargetRecognition(目标识别)";
					break;
				case NET_ACCESS_DOOROPEN_METHOD_FINGERPRINT:
					strMethod = "Fingerprint(指纹)";
					break;
				case NET_ACCESS_DOOROPEN_METHOD_REMOTE:
					strMethod = "Remote(远程)";
					break;
				default:
					strMethod = "Unknown(未知)";
					break;
			}
			std::string strStatus;
			if (pEventInfo->bStatus)
			{
				strStatus = "True(成功)";
			}
			else
			{
				strStatus = "False(失败)";
			}
			strInfo.Format("Channel:%d,Method:%s,Status:%s", pEventInfo->nChannelID, strMethod.c_str(), strStatus.c_str());
			m_listPicEventAlarm.SetItemText(0, 3, strInfo);

			// 准备图片缓存路径
			char szPicFilePath[1024*3]={0};
			char szPicFileName[256]={0};
			char currentFold[2048]={0};
			if (!GetCurrentDirectoryA(sizeof(currentFold), currentFold))
			{
				CString csInfo;
				csInfo.Format("%s", ConvertString("Failed to obtain the file directory."));
				MessageBox(csInfo, ConvertString("Prompt"));
				break;
			}

			// 显示图片
			for (int i = 0; i < pEventInfo->nImageInfoCount; i++)
			{
				// 全景图
				if (pEventInfo->stuImageInfo[i].emType == EM_ACCESS_CTL_IMAGE_SCENE && pEventInfo->stuImageInfo[i].nLength>0)
				{
					BYTE* data = new BYTE[pEventInfo->stuImageInfo[i].nLength];
					if (data)
					{
						memset(szPicFileName, 0, sizeof(szPicFileName));
						sprintf(szPicFileName, "Global_%s.jpg", pEventInfo->szEventTimeID);
						memset(szPicFilePath, 0, sizeof(szPicFilePath));
						sprintf(szPicFilePath, "%s\\%s", currentFold, szPicFileName);

						memcpy(data, pEventInfo->pBuffer+pEventInfo->stuImageInfo[i].nOffSet, pEventInfo->stuImageInfo[i].nLength);
						if (addPicToFilePath(szPicFilePath, data, pEventInfo->stuImageInfo[i].nLength))
						{
							drawPicFile(EM_ACCESS_CTL_IMAGE_SCENE, szPicFilePath);
						}

						delete []data;
						data = NULL;
					}
				}

				// 人脸抠图
				if (pEventInfo->stuImageInfo[i].emType == EM_ACCESS_CTL_IMAGE_FACE&& pEventInfo->stuImageInfo[i].nLength>0)
				{
					BYTE* data = new BYTE[pEventInfo->stuImageInfo[i].nLength];
					if (data)
					{
						memset(szPicFileName, 0, sizeof(szPicFileName));
						sprintf(szPicFileName, "Face_%s.jpg", pEventInfo->szEventTimeID);
						memset(szPicFilePath, 0, sizeof(szPicFilePath));
						sprintf(szPicFilePath, "%s\\%s", currentFold, szPicFileName);

						memcpy(data, pEventInfo->pBuffer+pEventInfo->stuImageInfo[i].nOffSet, pEventInfo->stuImageInfo[i].nLength);
						if (addPicToFilePath(szPicFilePath, data, pEventInfo->stuImageInfo[i].nLength))
						{
							drawPicFile(EM_ACCESS_CTL_IMAGE_FACE, szPicFilePath);
						}
						delete []data;
						data = NULL;
					}
				}

				// 人脸数据库底图
				if (pEventInfo->stuImageInfo[i].emType == EM_ACCESS_CTL_IMAGE_LOCAL && pEventInfo->stuImageInfo[i].nLength>0)
				{
					BYTE* data = new BYTE[pEventInfo->stuImageInfo[i].nLength];
					if (data)
					{
						memset(szPicFileName, 0, sizeof(szPicFileName));
						sprintf(szPicFileName, "Local_%s.jpg", pEventInfo->szEventTimeID);
						memset(szPicFilePath, 0, sizeof(szPicFilePath));
						sprintf(szPicFilePath, "%s\\%s", currentFold, szPicFileName);

						memcpy(data, pEventInfo->pBuffer+pEventInfo->stuImageInfo[i].nOffSet, pEventInfo->stuImageInfo[i].nLength);
						if (addPicToFilePath(szPicFilePath, data, pEventInfo->stuImageInfo[i].nLength))
						{
							drawPicFile(EM_ACCESS_CTL_IMAGE_LOCAL, szPicFilePath);
						}

						delete []data;
						data = NULL;
					}
				}
			}
		}
		break;
	}

	m_listPicEventAlarm.SetRedraw(TRUE);


	if (NULL != pEventInfo)
	{
		if (NULL != pEventInfo->pBuffer )
		{
			delete []pEventInfo->pBuffer;
			pEventInfo->pBuffer = NULL;
		}
		delete pEventInfo;
		pEventInfo = NULL;
	}

	return 0;
}

void CDialogPicEvent::OnBnClickedButtonStartPic()
{
	BOOL bret = StartListenEventAlarm();
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Failed to subscribe."), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return;
	}
	else
	{
		m_bListenEvent = TRUE;
	}

	GetDlgItem(IDC_BUTTON_START_PIC)->EnableWindow(FALSE);

}

void CDialogPicEvent::OnBnClickedButtonStopPic()
{
	CLock lock(m_csLock);

	BOOL bret = StopListenEventAlarm();
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Failed to unsubscribe."), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
		return;
	}

	m_bListenEvent = FALSE;
	GetDlgItem(IDC_BUTTON_START_PIC)->EnableWindow(TRUE);
}

BOOL CDialogPicEvent::StartListenEventAlarm()
{
	if (!m_bListenEvent)
	{
		int nChannel = m_picChannel.GetCurSel();
		BOOL bret =  Device::GetInstance().StartListenPicEvent(nChannel, EVENT_IVS_ACCESS_CTL, AnalyzerDataCallBack,(LDWORD)this);
		return bret;
	}
	return TRUE;
}

BOOL CDialogPicEvent::StopListenEventAlarm()
{

	if (!Device::GetInstance().StopListenPicEvent())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CDialogPicEvent::addPicToFile(EM_ACCESS_CTL_IMAGE_TYPE type, BYTE* data, int dataSize)
{
	std::string Path;
	char currentPath[1024]={0};
	if (!GetCurrentDirectoryA(sizeof(currentPath), currentPath))
	{
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to obtain the file directory."));
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}

	char filePath[2048]={0};
	std::string fileName;
	switch (type)
	{
	case EM_ACCESS_CTL_IMAGE_SCENE:
		fileName = "Global.jpg";
		break;
	case EM_ACCESS_CTL_IMAGE_FACE:
		fileName = "Face.jpg";
		break;
	case EM_ACCESS_CTL_IMAGE_LOCAL:
		fileName = "Candidate.jpg";
		break;
	default:
		break;
	}
	sprintf(filePath, "%s\\%s", currentPath, fileName.c_str());

	// 打开文件以写入二进制数据
	FILE *file = fopen(filePath, "wb");
	if (file == NULL) {
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to open file."));
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}

	// 写入数据到文件
	size_t bytesWritten = fwrite(data, 1, dataSize, file);
	if (bytesWritten < dataSize) {
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to write data to file."));
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	
	// 关闭文件
	fclose(file);
	file = NULL;
	return TRUE;
}

void CDialogPicEvent::InitPicControl()
{
	m_picGlobalWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1981);
	CRect screenRectGlobal;
	GetDlgItem(IDC_STATIC_PIC_GLOBAL)->GetWindowRect(&screenRectGlobal);
	ScreenToClient(&screenRectGlobal);
	m_picGlobalWnd.MoveWindow(screenRectGlobal);
	m_picGlobalWnd.ShowWindow(SW_SHOW);


	m_picFaceWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1981);
	CRect screenRectFace;
	GetDlgItem(IDC_STATIC_FACE)->GetWindowRect(&screenRectFace);
	ScreenToClient(&screenRectFace);
	m_picFaceWnd.MoveWindow(screenRectFace);
	m_picFaceWnd.ShowWindow(SW_SHOW);


	m_picCadidateWnd.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, CRect(0,0,0,0), this, 1981);
	CRect screenRectCadidate;
	GetDlgItem(IDC_STATIC_CANDIDATE)->GetWindowRect(&screenRectCadidate);
	ScreenToClient(&screenRectCadidate);
	m_picCadidateWnd.MoveWindow(screenRectCadidate);
	m_picCadidateWnd.ShowWindow(SW_SHOW);

	//m_picGlobal = GetDlgItem(IDC_STATIC_PIC_GLOBAL);
	//m_picFace = GetDlgItem(IDC_STATIC_FACE);
	//m_picCadidate = GetDlgItem(IDC_STATIC_CANDIDATE);
}


BOOL CDialogPicEvent::drawPic(EM_ACCESS_CTL_IMAGE_TYPE type)
{
	BOOL bRet = FALSE;

	std::string Path;
	char currentPath[1024]={0};
	if (!GetCurrentDirectoryA(sizeof(currentPath), currentPath))
	{
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to obtain the file directory."));
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}
	char filePath[2048]={0};
	std::string fileName;

	CWnd* pImageWindow = NULL;
	if (EM_ACCESS_CTL_IMAGE_SCENE == type)
	{
		fileName = "Global.jpg";
		sprintf(filePath, "%s\\%s", currentPath, fileName.c_str());
		pImageWindow = GetDlgItem(IDC_STATIC_PIC_GLOBAL);
	}
	else if(EM_ACCESS_CTL_IMAGE_FACE == type)
	{
		fileName = "Face.jpg";
		sprintf(filePath, "%s\\%s", currentPath, fileName.c_str());
		pImageWindow = GetDlgItem(IDC_STATIC_FACE);
	}
	else if(EM_ACCESS_CTL_IMAGE_LOCAL == type)
	{
		fileName = "Candidate.jpg";
		sprintf(filePath, "%s\\%s", currentPath, fileName.c_str());
		pImageWindow = GetDlgItem(IDC_STATIC_CANDIDATE);
	}
	else
	{
		return bRet;
	}

	if (NULL == pImageWindow)
	{
		return bRet;
	}

	CDC* pDC = pImageWindow->GetWindowDC();
	if (NULL == pDC)
	{
		return bRet;
	}

	USES_CONVERSION;
	IPicture* pPic = NULL;
	HRESULT hr = S_FALSE;
	hr = OleLoadPicturePath(T2OLE(filePath), NULL, 0, RESERVED_COLOR_EVENT, IID_IPicture, (LPVOID*)&pPic);
	if (FAILED(hr))
	{
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to load the picture file."));
		MessageBox(csInfo, ConvertString("Prompt"));
	}

	if (pPic == NULL)
	{
	    goto e_clearup;
	}

	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	pPic->get_Width(&hmWidth); 
	pPic->get_Height(&hmHeight); 
	RECT rect;
	pImageWindow->GetWindowRect(&rect);
	hr = pPic->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	if (FAILED(hr))
	{
		FillCWndWithDefaultColor(pImageWindow);
	}

	bRet = TRUE;


e_clearup:
	if (pPic != NULL)
	{
		pPic->Release();
		pPic = NULL;
	}
	if (pImageWindow != NULL && pDC != NULL)
	{
		pImageWindow->ReleaseDC(pDC);
		pImageWindow = NULL;
	}

	return bRet;
}

void CDialogPicEvent::FillCWndWithDefaultColor(CWnd* cwnd) 
{
	if (NULL == cwnd)
	{
		return;
	}

	CDC* cdc = cwnd->GetDC();
	if (NULL == cdc)
	{
		return;
	}

	RECT rect;
	cwnd->GetClientRect(&rect);
	CBrush brush(PICTURE_BACK_COLOR);
	cdc->FillRect(&rect, &brush);
	cwnd->ReleaseDC(cdc);
}

void CDialogPicEvent::OnClose()
{
	CLock lock(m_csLock);

	BOOL bret = StopListenEventAlarm();
	if (!bret)
	{
		CString csInfo;
		csInfo.Format("%s0x%08x", ConvertString("Failed to unsubscribe."), CLIENT_GetLastError());
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	m_bListenEvent = FALSE;
	CDialog::OnClose();
}

void CDialogPicEvent::clearWindows()
{
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_PIC_GLOBAL));
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_FACE));
	FillCWndWithDefaultColor(GetDlgItem(IDC_STATIC_CANDIDATE));
}

BOOL CDialogPicEvent::addPicToFilePath(const char* filePath, BYTE* data, int dataSize)
{
	// 打开文件以写入二进制数据
	FILE *file = fopen(filePath, "wb");
	if (file == NULL) {
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to open file."));
		MessageBox(csInfo, ConvertString("Prompt"));
		return FALSE;
	}

	// 写入数据到文件
	size_t bytesWritten = fwrite(data, 1, dataSize, file);
	if (bytesWritten < dataSize) {
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to write data to file."));
		MessageBox(csInfo, ConvertString("Prompt"));
	}
	
	// 关闭文件
	fclose(file);
	file = NULL;
	return TRUE;
}

BOOL CDialogPicEvent::drawPicFile(EM_ACCESS_CTL_IMAGE_TYPE type, const char* filePath)
{
	BOOL bRet = FALSE;

	CWnd* pImageWindow = NULL;
	if (EM_ACCESS_CTL_IMAGE_SCENE == type)
	{
		pImageWindow = GetDlgItem(IDC_STATIC_PIC_GLOBAL);
	}
	else if(EM_ACCESS_CTL_IMAGE_FACE == type)
	{
		pImageWindow = GetDlgItem(IDC_STATIC_FACE);
	}
	else if(EM_ACCESS_CTL_IMAGE_LOCAL == type)
	{
		pImageWindow = GetDlgItem(IDC_STATIC_CANDIDATE);
	}
	else
	{
		return bRet;
	}

	if (NULL == pImageWindow)
	{
		return bRet;
	}

	CDC* pDC = pImageWindow->GetWindowDC();
	if (NULL == pDC)
	{
		return bRet;
	}

	USES_CONVERSION;
	IPicture* pPic = NULL;
	HRESULT hr = S_FALSE;
	hr = OleLoadPicturePath(T2OLE(filePath), NULL, 0, RESERVED_COLOR_EVENT, IID_IPicture, (LPVOID*)&pPic);
	if (FAILED(hr))
	{
		CString csInfo;
		csInfo.Format("%s", ConvertString("Failed to load the picture file."));
		MessageBox(csInfo, ConvertString("Prompt"));
	}

	if (pPic == NULL)
	{
	    goto e_clearup;
	}

	OLE_XSIZE_HIMETRIC hmWidth; 
	OLE_YSIZE_HIMETRIC hmHeight; 
	pPic->get_Width(&hmWidth); 
	pPic->get_Height(&hmHeight); 
	RECT rect;
	pImageWindow->GetWindowRect(&rect);
	hr = pPic->Render(*pDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0, hmHeight, hmWidth, -hmHeight, NULL);
	if (FAILED(hr))
	{
		FillCWndWithDefaultColor(pImageWindow);
	}

	bRet = TRUE;


e_clearup:
	if (pPic != NULL)
	{
		pPic->Release();
		pPic = NULL;
	}
	if (pImageWindow != NULL && pDC != NULL)
	{
		pImageWindow->ReleaseDC(pDC);
		pImageWindow = NULL;
	}

	// 清理文件
	DeleteFileA(filePath);
	return bRet;
}
