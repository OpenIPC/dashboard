// AudioSource.cpp : implementation file
//

#include "stdafx.h"
#include "imagetest.h"
#include "AudioSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioSource dialog


CAudioSource::CAudioSource(CWnd* pParent /*=NULL*/,int nChannel,LLONG lLoginID)
	: CDialog(CAudioSource::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAudioSource)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_nChannel = nChannel;
    m_LoginID = lLoginID;
    memset(&m_stuAudioSource, 0, sizeof(m_stuAudioSource));
    m_stuAudioSource.dwSize = sizeof(m_stuAudioSource);
}


void CAudioSource::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAudioSource)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAudioSource, CDialog)
	//{{AFX_MSG_MAP(CAudioSource)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_BN_CLICKED(IDC_GET, OnGet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioSource message handlers

BOOL CAudioSource::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this);
	
	if (GetVideoIn())
	{
        StuToDlg();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
BOOL CAudioSource::GetVideoIn()
{
    BOOL bRet = CLIENT_GetConfig(m_LoginID,NET_EM_CFG_AUDIOIN_SOURCE,m_nChannel,&m_stuAudioSource,sizeof(m_stuAudioSource),TIMEOUT);
    if (!bRet)
    {
        MessageBox(ConvertString("Get audio input type failed!"), ConvertString("Prompt"));
    }
    return bRet;
}
BOOL CAudioSource::SetVideoIn()
{
    BOOL bRet = CLIENT_SetConfig(m_LoginID,NET_EM_CFG_AUDIOIN_SOURCE,m_nChannel,&m_stuAudioSource,sizeof(m_stuAudioSource),TIMEOUT);
    if (!bRet)
    {
        MessageBox(ConvertString("Set audio input type failed!"), ConvertString("Prompt"));
    }
    return bRet;
}
void CAudioSource::StuToDlg()
{
    int nNum = 0;
    BOOL bCheck[16]; 
	int i = 0;
    for (i = 0;i <16 ;i++)
    {
        bCheck[i] = FALSE;
    }
    for(int j =0 ;j<m_stuAudioSource.nRetAudioInSource;j++)
    {
        bCheck[m_stuAudioSource.emAudioInSource[j]-1] = TRUE;
    }
    for (i= 0; i<16; i++)
    {
        ((CButton*)GetDlgItem(IDC_RADIO_COAXIAL+ i))->SetCheck(bCheck[i]);
       
    }
    UpdateData(FALSE);
}
void CAudioSource::DlgToStu()
{
    UpdateData();
    int nCount = 0;
    for (int i= 0; i<16 ;i++)
    {
        if (((CButton*)GetDlgItem(IDC_RADIO_COAXIAL+i))->GetCheck())
        {
            m_stuAudioSource.emAudioInSource[nCount] = NET_EM_AUDIOIN_SOURCE_TYPE(i+1);
            nCount++;
        }
    }
    m_stuAudioSource.nMaxAudioInSource = nCount;
}

void CAudioSource::OnSet() 
{
	DlgToStu();
    SetVideoIn();
}

void CAudioSource::OnGet() 
{
    if (GetVideoIn())
    {
        StuToDlg();
	}
}
