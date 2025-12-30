// Heatmap.cpp : implementation file
//

#include "stdafx.h"
#include "ThermalCamera.h"
#include "Heatmap.h"
#include "ThermalCameraDlg.h"

#include "drcTable.h"
#if (defined(_WIN64) || defined(WIN64))
#pragma comment (lib, "./dhplay/win64/ImageAlg.lib")
#elif(defined(_WIN32) || defined(WIN32))
#pragma comment (lib, "./dhplay/win32/ImageAlg.lib")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHeatmap dialog

static NET_RADIOMETRY_DATA *g_pBuf = NULL;
CHeatmap::CHeatmap(CWnd* pParent /*=NULL*/,LLONG lLoginId,int nChannel)
	: CDialog(CHeatmap::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHeatmap)
	m_strStatus = _T("");
	m_nChannel = 0;
	m_nHeight = 0;
	m_nLength = 0;
	m_StrSensorType = _T("");
	m_StrTime = _T("");
	m_nWidth = 0;
	//}}AFX_DATA_INIT
    m_lLoginID = lLoginId;
    m_lAttachhandle = 0;
    m_nHeatChannel = nChannel;
	m_pBuf = NULL;

	if (NULL != g_pBuf)
	{
		if (NULL != g_pBuf->pbDataBuf)
		{
			delete [] g_pBuf->pbDataBuf;
			g_pBuf->pbDataBuf = NULL;
		}

		delete [] g_pBuf;
		g_pBuf = NULL;
	}
}


void CHeatmap::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHeatmap)
	DDX_Control(pDX, IDC_STOP, m_BtnStop);
	DDX_Control(pDX, IDC_STARTFETCH, m_BtnStart);
	DDX_Control(pDX, IDC_ATTACH, m_BtnAttach);
	DDX_Text(pDX, IDC_EDIT_STATUS, m_strStatus);
	DDX_Text(pDX, IDC_EDIT_CHANNEL, m_nChannel);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_nHeight);
	DDX_Text(pDX, IDC_EDIT_LENGTH, m_nLength);
	DDX_Text(pDX, IDC_EDIT_SENSORTYPE, m_StrSensorType);
	DDX_Text(pDX, IDC_EDIT_TIME, m_StrTime);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_nWidth);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BTNIMGANDTEMSAVE, m_BtnImgAndTemSave);
}


BEGIN_MESSAGE_MAP(CHeatmap, CDialog)
	//{{AFX_MSG_MAP(CHeatmap)
	ON_BN_CLICKED(IDC_STARTFETCH, OnStartfetch)
	ON_BN_CLICKED(IDC_ATTACH, OnAttach)
	ON_BN_CLICKED(IDC_STOP, OnStop)
    ON_MESSAGE(WM_HEATMAPINFO, OnShowInfo)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTNIMGANDTEMSAVE, &CHeatmap::OnBnClickedBtnimgandtemsave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHeatmap message handlers

LRESULT CHeatmap::OnShowInfo(WPARAM wParam, LPARAM lParam)
{
	if (NULL != g_pBuf)
	{
		if (NULL != g_pBuf->pbDataBuf)
		{
			delete [] g_pBuf->pbDataBuf;
			g_pBuf->pbDataBuf = NULL;
		}

		delete [] g_pBuf;
		g_pBuf = NULL;
	}

    m_pBuf = (NET_RADIOMETRY_DATA*)wParam;

	if (NULL == m_pBuf)
	{
		UpdateData(FALSE);
		return 0;
	}

    m_nHeight = m_pBuf->stMetaData.nHeight;
    m_nWidth = m_pBuf->stMetaData.nWidth;
    m_nChannel = m_pBuf->stMetaData.nChannel;
    m_nLength = m_pBuf->stMetaData.nLength;
    m_StrSensorType = m_pBuf->stMetaData.szSensorType;
    m_StrTime.Format("%2d-%d-%d-%d-%d-%d",m_pBuf->stMetaData.stTime.dwYear,m_pBuf->stMetaData.stTime.dwMonth,
        m_pBuf->stMetaData.stTime.dwDay, m_pBuf->stMetaData.stTime.dwHour,m_pBuf->stMetaData.stTime.dwMinute,
        m_pBuf->stMetaData.stTime.dwSecond);


	g_pBuf = new NET_RADIOMETRY_DATA;
	if (NULL != g_pBuf)
	{
		g_pBuf->dwBufSize = m_pBuf->dwBufSize;
		memcpy(&g_pBuf->stMetaData, &m_pBuf->stMetaData, sizeof(m_pBuf->stMetaData));

		g_pBuf->pbDataBuf = new BYTE[m_pBuf->dwBufSize];
		if (NULL != g_pBuf->pbDataBuf)
		{	
			memcpy(g_pBuf->pbDataBuf, m_pBuf->pbDataBuf, m_pBuf->dwBufSize);
		}
	}

	if (m_pBuf != NULL)
	{
		if (m_pBuf->pbDataBuf) 
		{
			delete[] m_pBuf->pbDataBuf;
			m_pBuf->pbDataBuf  = NULL;
		}
		delete m_pBuf;
		m_pBuf = NULL;
	}

    UpdateData(FALSE);
    return 0;
}
void CALLBACK  cbRadiometryAttachCB(LLONG  lAttachHandle,  NET_RADIOMETRY_DATA* pBuf, int nBufLen, LDWORD dwUser) 							
{
    //PRINT(pBuf->stMetaData.nLength);    
    
    CHeatmap *dlg = (CHeatmap *)dwUser;
    if ( dlg!=NULL)
    {

		NET_RADIOMETRY_DATA *pTmpBuf = new NET_RADIOMETRY_DATA;
		if (NULL == pTmpBuf)
		{
			return;
		}

		pTmpBuf->dwBufSize = pBuf->dwBufSize;
		memcpy(&pTmpBuf->stMetaData, &pBuf->stMetaData, sizeof(pBuf->stMetaData));

		pTmpBuf->pbDataBuf = new BYTE[pBuf->dwBufSize];
		if (NULL != pTmpBuf->pbDataBuf)
		{	
			memcpy(pTmpBuf->pbDataBuf, pBuf->pbDataBuf, pBuf->dwBufSize);
		}

        dlg->PostMessage(WM_HEATMAPINFO, (WPARAM)pTmpBuf, (LPARAM)0);
    }
   /* UpdateData(FALSE);*/
}

BOOL CHeatmap::OnInitDialog() 
{
	CDialog::OnInitDialog();
    g_SetWndStaticText(this);
	m_BtnStart.EnableWindow(FALSE);
    m_BtnStop.EnableWindow(FALSE);
	m_BtnImgAndTemSave.EnableWindow(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHeatmap::OnStartfetch() 
{
    NET_IN_RADIOMETRY_FETCH stInFetch = {sizeof(stInFetch), m_nHeatChannel};
    NET_OUT_RADIOMETRY_FETCH stOutFetch = {sizeof(stOutFetch)};
    BOOL bRet =  CLIENT_RadiometryFetch(m_lLoginID, &stInFetch, &stOutFetch, 1000);
    if (bRet)
    {
		m_BtnImgAndTemSave.EnableWindow(TRUE);
    }
	if (stOutFetch.nStatus ==0)
    {
        m_strStatus = ConvertString("Unknown");
    }
    else if (stOutFetch.nStatus == 1)
    {
        m_strStatus = ConvertString("free");
    }
    else if (stOutFetch.nStatus == 2)
    {
        m_strStatus = ConvertString("acquiring");
    }
 //   m_BtnStart.EnableWindow(FALSE);
    //PRINT(stOutFetch.nStatus);
    UpdateData(FALSE);
}

void CHeatmap::OnAttach() 
{
    NET_IN_RADIOMETRY_ATTACH stIn = {sizeof(stIn)};
    stIn.nChannel = m_nHeatChannel;
    stIn.dwUser = (LDWORD)this;
    stIn.cbNotify = cbRadiometryAttachCB;
    NET_OUT_RADIOMETRY_ATTACH stOut = {sizeof(stOut)};
    m_lAttachhandle = CLIENT_RadiometryAttach(m_lLoginID, &stIn, &stOut, 1000);
    m_BtnAttach.EnableWindow(FALSE);
    m_BtnStart.EnableWindow(TRUE);
    m_BtnStop.EnableWindow(TRUE);
}

void CHeatmap::OnStop() 
{
	CLIENT_RadiometryDetach(m_lAttachhandle);
    m_BtnAttach.EnableWindow(TRUE);
    m_BtnStart.EnableWindow(FALSE);
    m_BtnStop.EnableWindow(FALSE);
	m_BtnImgAndTemSave.EnableWindow(FALSE);
}

void CHeatmap::OnDestroy() 
{
	CDialog::OnDestroy();
    CLIENT_RadiometryDetach(m_lAttachhandle);
	
}

template<typename T>
BOOL writeFun(char *pFileName, T pBuf, int nLen)
{
	if (NULL == pFileName || NULL == pBuf || 0 == nLen)
	{
		return FALSE;
	}
	FILE *fp = fopen(pFileName, "wb");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(pBuf, nLen, 1, fp);
	fclose(fp);

	return TRUE;
}

BOOL writeFloatData(char *pFileName, char *pBuf, int nLen)
{
	if (NULL == pFileName || NULL == pBuf || 0 == nLen)
	{
		return FALSE;
	}
	FILE *fp = fopen(pFileName, "a++");
	if (NULL == fp)
	{
		return FALSE;
	}
	fwrite(pBuf, nLen, 1, fp);
	fclose(fp);

	return TRUE;
}

#define __max(a, b) (((a) > (b)) ? (a) : (b))
void SearchMax(float &nMax, float *YData, int yDataLen)
{
	if (NULL == YData)
	{
		return;
	}
	nMax = (float)YData[0];
	for (int i=1; i <yDataLen; i++)
	{
		nMax = __max(nMax, (float)YData[i]);
	}
}

#define  __min(a, b) (((a) < (b)) ? (a) : (b))
void SearchMin(float &nMin, float *YData, int yDataLen)
{
	if (NULL == YData)
	{
		return;
	}
	nMin = (float)YData[0];
	for (int i=1; i< yDataLen; i++)
	{
		nMin = __min(nMin, (float)YData[i]);
	}
}

#define PATH_TEM "./TemFile.txt"
void CHeatmap::OnBnClickedBtnimgandtemsave()
{
	// TODO: 在此添加控件通知处理程序代码
	//MessageBox("OnBtn_GrayscaleMapSave", ConvertString("Prompt"));
	int nRetMove = remove(PATH_TEM);
	if (0 != nRetMove)
	{
	}
	int nImgLen = m_nHeight*m_nWidth;
	unsigned short *pImg = new unsigned short[nImgLen];
	if (NULL != pImg)
	{
		memset(pImg, 0, sizeof(unsigned short)*nImgLen);
	}
	else
	{
		MessageBox(ConvertString("save GraySacleMap Malloc memory faild."), ConvertString("Prompt"));
		return;
	}

	float *pTemp = new float[nImgLen];
	if (NULL != pTemp)
	{
		memset(pTemp, 0, sizeof(unsigned short)*nImgLen);
	}
	else
	{
		MessageBox(ConvertString("save Temperature Malloc memory faild."), ConvertString("Prompt"));
		
		if (NULL != pImg)
		{
			delete [] pImg;
			pImg = NULL;
		}
		return;
	}

	// 转换后的Y数据
	unsigned char *pszYData = new unsigned char[nImgLen*3];
	if (NULL == pszYData)
	{
		if (NULL != pImg)
		{
			delete [] pImg;
			pImg = NULL;
		}

		if (NULL != pTemp)
		{
			delete [] pTemp;
			pTemp = NULL;
		}
		return;
	}
	else
	{
		memset(pszYData, 0, nImgLen*3);
	}

	unsigned short *plut = new unsigned short[1024];
	if (NULL == plut)
	{
		if (NULL != pImg)
		{
			delete [] pImg;
			pImg = NULL;
		}

		if (NULL != pTemp)
		{
			delete [] pTemp;
			pTemp = NULL;
		}

		if (NULL != pszYData)
		{
			delete [] pszYData;
			pszYData = NULL;
		}

		return;
	}
	memset(plut, 0, sizeof(unsigned short)*1024);
	BOOL bRetDataParse =  CLIENT_RadiometryDataParse(g_pBuf, pImg, pTemp);
	if (!bRetDataParse)
	{
		MessageBox(ConvertString("Radiometry Data Parse error."), ConvertString("Prompt"));
		
		if (NULL != pImg)
		{
			delete [] pImg;
			pImg = NULL;
		}

		if (NULL != pTemp)
		{
			delete [] pTemp;
			pTemp = NULL;
		}

		if (NULL != plut)
		{
			delete [] plut;
			plut = NULL;
		}

		if (NULL != pszYData)
		{
			delete [] pszYData;
			pszYData = NULL;
		}
		return;
	}
	else
	{
		if (0 != m_nHeight && 0 != m_nWidth)
		{
			float nMax = -1;
			SearchMax(nMax, pTemp, nImgLen);
			char szBufMax[32] = {0};
			_snprintf(szBufMax,sizeof(szBufMax), "%s:%.3lf", ConvertString("Max"), nMax);

			writeFloatData(PATH_TEM, szBufMax, strlen(szBufMax));
			writeFloatData(PATH_TEM, "\n", strlen("\n"));

			float nMin = -1;
			SearchMin(nMin, pTemp, nImgLen);
			char szBufMin[32] = {0};
			_snprintf(szBufMin,sizeof(szBufMin), "%s:%.3lf", ConvertString("Min"), nMin);
			writeFloatData(PATH_TEM, szBufMin, strlen(szBufMin));
			writeFloatData(PATH_TEM, "\n", strlen("\n"));

			drcTable(pImg, m_nWidth, m_nHeight, 0,pszYData, plut);
			writeFun( "./GraysaceMap.yuv", pszYData, m_nWidth*m_nHeight*2);
			for (int i=0; i<20; i++ )
			{
				char szBbuf[32] = {0};
				sprintf(szBbuf, "%.3lf", pTemp[i]);
				writeFloatData(PATH_TEM, szBbuf, strlen(szBbuf));
				writeFloatData(PATH_TEM, "\t", strlen("\t"));
			}
		
			MessageBox(ConvertString("Data Save success."), ConvertString("Prompt"));
		}
		else
		{
			MessageBox(ConvertString("Data Save faild."), ConvertString("Prompt"));
		}



	}

	if (NULL != pImg)
	{
		delete [] pImg;
		pImg = NULL;
	}

	if (NULL != pTemp)
	{
		delete [] pTemp;
		pTemp = NULL;
	}

	if (NULL != pszYData)
	{
		delete [] pszYData;
		pszYData = NULL;
	}

	if (NULL != plut)
	{
		delete [] plut;
		plut = NULL;
	}
}
