// lientState.cpp : implementation file
//

#include "StdAfx.h"
#include "netsdkdemo.h"
#include "clientState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientState dialog

CClientState::CClientState(CWnd* pParent /*=NULL*/)
	: CDialog(CClientState::IDD, pParent)
{
	m_dev = 0;
	memset(&m_myState, 0, sizeof(NET_CLIENT_STATE));
	//{{AFX_DATA_INIT(CClientState)
	m_isNoMoreShow = FALSE;
	//}}AFX_DATA_INIT
}

CClientState::~CClientState()
{
}

void CClientState::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClientState)
	DDX_Check(pDX, IDC_NOSHOW, m_isNoMoreShow);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClientState, CDialog)
	//{{AFX_MSG_MAP(CClientState)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ClientState message handlers
void CClientState::UpdateState()
{
	if (!m_dev)
	{
		return;
	}
	CDevMgr::GetDevMgr().GetAlarmInfo(m_dev->LoginID, &m_myState);
	CString str;
	str.Format("(%s [%s])",m_dev->Name, m_dev->IP);
	CString strTmp;
	CString strDiskError = ConvertString(MSG_CLIENTSTATE_DISKERR);
	int i = 0;
    for (i=0; i<32; i++)
    {  
		if (m_myState.dError & (0x01<<i))
        {
			strTmp.Format(" %d", i+1);
			strDiskError += strTmp;
        }
    }

	CString strDiskFull = ConvertString(MSG_CLIENTSTATE_DISKFULL);	
	if (m_myState.dFull)
	{
		strDiskFull += ConvertString("Full");
	}
	else
	{
		strDiskFull += ConvertString("Not");
	}
	
	CString strShelter = ConvertString(MSG_CLIENTSTATE_SHELTER);
	for (i=0; i<16; i++)
    {
        if (m_myState.shelter[i])
        {
			strTmp.Format(" %d", i+1);
            strShelter += strTmp;
        }
    }

    CString strAlarm = ConvertString(MSG_CLIENTSTATE_ALARM);
	int nNum = min(16, m_dev->Info.nAlarmInPortNum);
    for (i=0; i<nNum; i++)
    {
     
		if (m_myState.cState.alarm[i])
        {
			strTmp.Format(" %d", i+1);
			strAlarm += strTmp;
        }
    }

    CString strMD = ConvertString(MSG_CLIENTSTATE_MOTION);
    CString strVideoLost = ConvertString(MSG_CLIENTSTATE_VIDEOLOST);
	CString strSD = ConvertString(MSG_CLIENTSTATE_SOUND);
	nNum = min(16, m_dev->Info.nChanNum);
    for (i=0; i<nNum; i++)
    {
        if (m_myState.cState.motiondection[i])
        {
			strTmp.Format(" %d", i+1);
            strMD += strTmp;
        }
        if (m_myState.cState.videolost[i])
        {
			strTmp.Format(" %d", i+1);
            strVideoLost += strTmp;
        }
        if (m_myState.soundalarm[i])
        {
			strTmp.Format(" %d", i+1);
            strSD += strTmp;
        }
    }

	CString strSerial = ConvertString(MSG_CLIENTSTATE_SERIAL);
	strTmp.Format("%s", m_dev->Info.sSerialNumber);
	strSerial += strTmp;

	CString strAlmDec = ConvertString(MSG_CLIENTSTATE_ALMDEC);
	strAlmDec += ": \n";
	for (i = 0; i < 16; i++)
	{
		for(int j = 0; j < 8; j++)
		{
			if (m_myState.almDecoder[i] & (1<<j))
			{
				strTmp.Format("%d", 1);
			}
			else
			{
				strTmp.Format("%d", 0);
			}
			strAlmDec += strTmp;
		}
		strAlmDec += " ";
	}

	str += ConvertString(MSG_CLIENTSTATE_CLIENTSTATE);
    str = str +"\n" +"\n"+
		strDiskError + "\n" +"\n"+
		strDiskFull + "\n" +"\n"+
		strShelter +"\n" + "\n"+
        strAlarm + "\n" +"\n"+
        strMD + "\n" +"\n"+
        strVideoLost + "\n" +"\n"+
		strSD + "\n" +"\n"+
		strAlmDec + "\n" + "\n" +
		strSerial;

    GetDlgItem(IDC_CLIENT_STATE)->SetWindowText(str);
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CClientState message handlers

BOOL CClientState::OnInitDialog() 
{
	CDialog::OnInitDialog();

	g_SetWndStaticText(this);
	return TRUE;  
}

void CClientState::StartRefresh()
{
	SetTimer((LLONG)this,200,0);
}

void CClientState::StopRefresh()
{
	KillTimer((LLONG)this);
}

void CClientState::OnTimer(UINT_PTR nIDEvent) 
{
	UpdateState();
	CDialog::OnTimer(nIDEvent);
}

void CClientState::SetDevice(DeviceNode *dev)
{
	m_dev = dev;
}

DeviceNode* CClientState::GetDevice()
{
	return m_dev;
}

void CClientState::OnClose() 
{
	StopRefresh();
	CDialog::OnClose();
}

void CClientState::OnOK() 
{
	StopRefresh();
	CDialog::OnOK();
}

BOOL CClientState::PreTranslateMessage(MSG* pMsg) 
{
	if (WM_KEYDOWN == pMsg->message && (VK_ESCAPE == pMsg->wParam))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
