   // DeviceWorkState.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "DeviceWorkState.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeviceWorkState dialog


CDeviceWorkState::CDeviceWorkState(CWnd* pParent /*=NULL*/)
	: CDialog(CDeviceWorkState::IDD, pParent)
{
	m_dev = 0;
	//{{AFX_DATA_INIT(CDeviceWorkState)
	//}}AFX_DATA_INIT
	memset(&m_almState, 0 , sizeof(NET_CLIENT_STATE));
	memset(m_shltAlarm, 0, 16);
	memset(m_recording, 0, 16);
	memset(&m_diskState, 0 , sizeof(DH_HARDDISK_STATE));
}


void CDeviceWorkState::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviceWorkState)
	DDX_Control(pDX, IDC_DISKSEL_COMBO, m_disksel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeviceWorkState, CDialog)
	//{{AFX_MSG_MAP(CDeviceWorkState)
	ON_CBN_SELCHANGE(IDC_DISKSEL_COMBO, OnSelchangeDiskselCombo)
	ON_CBN_SELCHANGE(IDC_CHANSEL_COMBO, OnSelchangeChanselCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeviceWorkState message handlers

void CDeviceWorkState::SetDevice(DeviceNode *dev)
{
	m_dev = dev;
}

BOOL CDeviceWorkState::OnInitDialog() 
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);
	
	CString nStr, subStr;
	
	if(!m_dev)
	{
		return TRUE;
	}

	int retlen = 0;
	//get common alarm state
	BOOL bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_COMM_ALARM,
										(char*)&m_almState, sizeof(NET_CLIENT_STATE), &retlen, 1000);
	if(!bRet || retlen != sizeof(NET_CLIENT_STATE))
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		return FALSE;
	}
	
	CString strDev;
	CString strTmp;
	CString strAlarm = ConvertString(MSG_CLIENTSTATE_ALARM);
	int i = 0;
    for (i=0; i<m_almState.alarminputcount; i++)
    {
        if (m_almState.alarm[i])
        {
			strTmp.Format(" %d", i+1);
			strAlarm += strTmp;
        }
    }

    CString strMD = ConvertString(MSG_CLIENTSTATE_MOTION);
    CString strVideoLost = ConvertString(MSG_CLIENTSTATE_VIDEOLOST);
    for (i=0; i<m_almState.channelcount; i++)
    {
        if (m_almState.motiondection[i])
        {
			strTmp.Format(" %d", i+1);
            strMD += strTmp;
        }
        if (m_almState.videolost[i])
        {
			strTmp.Format(" %d", i+1);
            strVideoLost += strTmp;
        }
    }


	//get shelter alarm state
	bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_SHELTER_ALARM,
		(char*)&m_shltAlarm, 16, &retlen, 1000);
	if(!bRet || retlen != 16)
	{
	}
	CString strShelter = ConvertString(MSG_WORKSTATE_SHELTER);
	for (i=0; i<16; i++)
    {
        if (m_shltAlarm[i])
        {
			strTmp.Format(" %d", i+1);
            strShelter += strTmp;
        }
    }

	//record state
	bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_RECORDING,
		(char*)&m_recording, 16, &retlen, 1000);
	if(!bRet || retlen != 16)
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		return FALSE;
	}
	CString strRecord = ConvertString(MSG_WORKSTATE_RECORDING);
	for (i=0; i<16; i++)
    {
        if (m_recording[i])
        {
			strTmp.Format(" %d", i+1);
            strRecord += strTmp;
        }
    }

	//×ÊÔ´×´Ì¬²éÑ¯
	DWORD dwResource[3] = {0};
	CString strResource = ConvertString("Resources: ");
	bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_RESOURCE, (char *)dwResource, 3*sizeof(DWORD), &retlen);
	if (!bRet || retlen != 3*sizeof(DWORD))
	{
		strTmp.Format(ConvertString("query error"));
		strResource += strTmp;
		((CNetSDKDemoDlg *)GetParent())->LastError();
	}
	else
	{
		strResource += ConvertString("System:");
		if (0 == dwResource[0])
		{
			strResource += ConvertString("natural");
		}
		else
		{
			strResource += ConvertString("CPU occupy too high");
		}
		strResource += ",Local show";
		if (0 == dwResource[1])
		{
			strResource += ConvertString("TV natural");
		}
		else
		{
			strResource += ConvertString("TV abnormality");
		}
		if (0 == dwResource[2])
		{
			strResource += ConvertString("VGA natural");
		}
		else
		{
			strResource += ConvertString("VGA abnormality");
		}
	}

	//»ñÈ¡ÂëÁ÷
	const int channum = 3;
	DWORD dwBitRate[channum] = {0};
	CString strBitRate = ConvertString("BitRate : ");
	bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_BITRATE, (char *)dwBitRate, channum*sizeof(DWORD), &retlen);
	if (!bRet || retlen != channum*sizeof(DWORD))
	{
		strBitRate += ConvertString("query error");
		((CNetSDKDemoDlg*)GetParent())->LastError();
	}
	else
	{
		for (int i = 0; i < channum; i++)
		{
			strTmp.Format("%d( %d ) ", i+1, dwBitRate[i]);
			strBitRate += strTmp;
		}
	}
	DWORD dwNetConn[11] = {0};
	CString strNetConn = ConvertString("NetConn :");
	bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_CONN, (char *)dwNetConn, 11*sizeof(DWORD), &retlen);
	if (!bRet || retlen < sizeof(DWORD) || retlen > 11*sizeof(DWORD))
	{
		strNetConn += ConvertString("query error");
		((CNetSDKDemoDlg*)GetParent())->LastError();
	}
	else
	{
		strTmp.Format("Num( %d ) ", dwNetConn[0]);
		strNetConn += strTmp;
		for (int i = 1; i <= dwNetConn[0]; i++)
		{
			strTmp.Format("Ip_%d( 0x%0x ) ", i, dwNetConn[i]);
			strNetConn += strTmp;
		}
	}
	bRet = CLIENT_QueryDevState(m_dev->LoginID, DH_DEVSTATE_DISK,
								(char*)&m_diskState, sizeof(DH_HARDDISK_STATE), &retlen, 1000);
	if(!bRet || retlen != sizeof(DH_HARDDISK_STATE))
	{
		((CNetSDKDemoDlg *)GetParent())->LastError();//Zhengdh 06.11.24
		return FALSE;
	}

	CString strDsk;
#ifdef LANG_ENG
	strDsk.Format(ConvertString("Disk num: %d"), m_diskState.dwDiskNum);
#else
	strDsk.Format("Ó²ÅÌÊýÁ¿: %d\n", m_diskState.dwDiskNum);
#endif
	strDev = strAlarm + "\n" + strShelter + "\n" + 
		strMD + "\n" + strVideoLost + "\n" + 
		strRecord + "\n" + strResource + "\n" + 
		strBitRate + "\n" + strNetConn + "\n" + strDsk + "\n";
	
	GetDlgItem(IDC_DEV_STATUS)->SetWindowText(strDev);

	m_disksel.Clear();
	CString strItem;
	for (int k=0; k<m_diskState.dwDiskNum; k++)
	{
		strItem.Format(ConvertString("Disk %d"), k);
		m_disksel.InsertString(k, strItem);
	}
		
	if (m_disksel.GetCount() > 0)
	{
		m_disksel.SetCurSel(0);
		OnSelchangeDiskselCombo();
	}

	return TRUE;
}

void CDeviceWorkState::OnSelchangeDiskselCombo() 
{
	int dskIndex = m_disksel.GetCurSel();
	CString strInfo = "";
#ifdef LANG_ENG
	strInfo.Format(ConvertString("Size: %d, Remain: %d"), 
				m_diskState.stDisks[dskIndex].dwVolume, 
				m_diskState.stDisks[dskIndex].dwFreeSpace);
	strInfo += "\n";
	CString strDskSt = "";
	switch(m_diskState.stDisks[dskIndex].dwStatus)
	{
	case 0:
		strDskSt.Format(ConvertString("Work status: Sleeping"));
		break;
	case 1:
		strDskSt.Format(ConvertString("Work status: Working"));
		break;
	case 2:
		strDskSt.Format(ConvertString("Work status: Error"));
	default:
		break;
	}
#else
	strInfo.Format("×ÜÈÝÁ¿: %d, Ê£ÓàÈÝÁ¿: %d\n", 
		m_diskState.stDisks[dskIndex].dwVolume, 
		m_diskState.stDisks[dskIndex].dwFreeSpace);
	CString strDskSt = "";
	switch(m_diskState.stDisks[dskIndex].dwStatus)
	{
	case 0:
		strDskSt.Format("Ó²ÅÌ×´Ì¬: ÐÝÃß\n");
		break;
	case 1:
		strDskSt.Format("Ó²ÅÌ×´Ì¬: ¹¤×÷\n");
		break;
	case 2:
		strDskSt.Format("Ó²ÅÌ×´Ì¬: ¹ÊÕÏ\n");
	default:
		break;
	}
#endif
	strInfo += strDskSt;
	GetDlgItem(IDC_DISK_STATUS)->SetWindowText(strInfo);
}

void CDeviceWorkState::OnSelchangeChanselCombo() 
{

}
