// PreviewParmsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include <math.h>
#include "netsdkdemo.h"
#include "PreviewParmsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewParmsDlg dialog


CPreviewParmsDlg::CPreviewParmsDlg(CWnd* pParent /* = NULL */, DeviceNode* pstuDevInfo /* = NULL */)
	: CDialog(CPreviewParmsDlg::IDD, pParent)
    , m_emPreviewMode(CFG_EM_PREVIEW_MODE_UNKNOWN)
    , m_emRealPlayType((DH_RealPlayType)-1)
{
	//{{AFX_DATA_INIT(CPreviewParmsDlg)
	//}}AFX_DATA_INIT
    memset(&m_stuDevInfo, 0, sizeof(DeviceNode));
    if (NULL != pstuDevInfo)
    {
        memcpy(&m_stuDevInfo, pstuDevInfo, sizeof(DeviceNode));
    }
	
	m_channelId = 0;	
}


void CPreviewParmsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreviewParmsDlg)
	DDX_Control(pDX, IDC_COMBO_PREVIEWTYPE, m_cmbSplitNum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPreviewParmsDlg, CDialog)
	//{{AFX_MSG_MAP(CPreviewParmsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CPreviewParmsDlg private method

void CPreviewParmsDlg::InitDlg()
{
    m_cmbSplitNum.ResetContent();
}

void CPreviewParmsDlg::InitCommbo(UINT uiVideoIn, const CFG_ENCODECAP& stuEncodeCap)
{
    unsigned int i  = 0;
    unsigned int j  = 0;
    int nIndex      = 0;

    // split num
    m_cmbSplitNum.ResetContent();
    if (CFG_EM_PREVIEW_MODE_SNAPSHOT == stuEncodeCap.emPreviewMode)
    {
        nIndex = m_cmbSplitNum.InsertString(-1, ConvertString(stuRealPlayType[0].szType));
        m_cmbSplitNum.SetItemData(nIndex, (DWORD)stuRealPlayType[0].emType);
    }
    else if (CFG_EM_PREVIEW_MODE_SPLITENCODE == stuEncodeCap.emPreviewMode)
    {
        for (i = 0; i < sizeof(stuRealPlayType)/sizeof(stuRealPlayType[0]) && i <= sqrt((double)uiVideoIn); i++)
        {
            if ((UINT)stuRealPlayType[i].emSplitMode <= uiVideoIn)
            {
                nIndex = m_cmbSplitNum.InsertString(-1, ConvertString(stuRealPlayType[i].szType));
                m_cmbSplitNum.SetItemData(nIndex, (DWORD)stuRealPlayType[i].emType);
            }
        }
    }
    else if (CFG_EM_PREVIEW_MODE_SPLITSNAP == stuEncodeCap.emPreviewMode)
    {
        if (1 == stuEncodeCap.nSplitModeNum
            && -1 == stuEncodeCap.anSplitMode[0])
        {
            for (i = 1; i <= m_stuDevInfo.nChnNum && i <= sqrt((double)uiVideoIn); i++)
            {
                if (3 == i)
                {
                    nIndex = m_cmbSplitNum.InsertString(-1, ConvertString(stuRealPlayType[4].szType));
                    m_cmbSplitNum.SetItemData(nIndex, (DWORD)stuRealPlayType[4].emType);
                }

                for (j = 0; j < sizeof(stuRealPlayType)/sizeof(stuRealPlayType[0]); j++)
                {
                    if (stuRealPlayType[j].emSplitMode == (DH_SPLIT_MODE)(i * i))
                    {
                        break;
                    }
                }
				
				//修复一下崩溃问题，如果j最终等于9，则设置j=1,防止越界访问，
				if (j == 9)
				{
					j=8;//默认16画面分割
				}
                nIndex = m_cmbSplitNum.InsertString(-1, ConvertString(stuRealPlayType[j].szType));
                m_cmbSplitNum.SetItemData(nIndex, (DWORD)stuRealPlayType[j].emType);
            }
        }
        else
        {
            for (i = 0; i < stuEncodeCap.nSplitModeNum; i++)
            {
                BOOL bFound = FALSE;
                for (j = 0; j < sizeof(stuRealPlayType)/sizeof(stuRealPlayType[0]); j++)
                {
                    if (stuRealPlayType[j].emSplitMode == (DH_SPLIT_MODE)(stuEncodeCap.anSplitMode[i]))
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                if (bFound)
                {
                    nIndex = m_cmbSplitNum.InsertString(-1, ConvertString(stuRealPlayType[j].szType));
                    m_cmbSplitNum.SetItemData(nIndex, (DWORD)stuRealPlayType[j].emType);
                }
            }
        }
    }
    else
    {
        // 默认只有一种原始多画面预览
        nIndex = m_cmbSplitNum.InsertString(-1, ConvertString(stuRealPlayType[0].szType));
        m_cmbSplitNum.SetItemData(nIndex, (DWORD)stuRealPlayType[0].emType);
    }

    m_cmbSplitNum.SetCurSel(0);
}

BOOL CPreviewParmsDlg::GetEncodeCaps(CFG_ENCODECAP& stuEncodeCap)
{
    char szBuf[1024 * 32] = {0};
    int nErr = 0;
    BOOL bRet = CLIENT_QueryNewSystemInfo(m_stuDevInfo.LoginID, CFG_CMD_ENCODE_GETCAPS, -1, szBuf, sizeof(szBuf), &nErr, 5000);
    if (bRet)
    {
        int nRetLen = 0;
        bRet = CLIENT_ParseData(CFG_CMD_ENCODE_GETCAPS, szBuf, &stuEncodeCap, sizeof(stuEncodeCap), &nRetLen);
        if (bRet)
        {
            m_emPreviewMode = stuEncodeCap.emPreviewMode;
        }
    }
    return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CPreviewParmsDlg message handlers

BOOL CPreviewParmsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	g_SetWndStaticText(this);
	
    CFG_ENCODECAP stuEncodeCap = {0};
    if (GetEncodeCaps(stuEncodeCap))
    {
        InitCommbo(m_stuDevInfo.nChnNum, stuEncodeCap);
    }
	SetDlgItemInt(IDC_EDIT_STCHANNEL, 1);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreviewParmsDlg::OnOK() 
{
    m_channelId = GetDlgItemInt(IDC_EDIT_STCHANNEL);
    if (m_channelId < 0 || m_channelId > m_stuDevInfo.nChnNum)
    {
        m_channelId = 0;
	}

    int nIndex = m_cmbSplitNum.GetCurSel();
    if (-1 == nIndex)
    {
        return;
    }
    m_emRealPlayType = (DH_RealPlayType)m_cmbSplitNum.GetItemData(nIndex);
    
    if (CFG_EM_PREVIEW_MODE_SPLITSNAP == m_emPreviewMode)
    {
        int i = 0;
        DH_SPLIT_MODE emMode = (DH_SPLIT_MODE)-1;
        for (; i < sizeof(stuRealPlayType)/sizeof(stuRealPlayType[0]); i++)
        {
            if (m_emRealPlayType == stuRealPlayType[i].emType)
            {
                emMode = stuRealPlayType[i].emSplitMode;
                break;
            }
        }
        
        int nGroupId = m_channelId/stuRealPlayType[i].emSplitMode;
        if (0 > nGroupId)
        {
            nGroupId = 0;
        }
        
        DH_SPLIT_MODE_INFO stuSplitModeInfo = {sizeof(stuSplitModeInfo)};
        stuSplitModeInfo.emSplitMode = emMode;
        stuSplitModeInfo.dwDisplayType = DH_SPLIT_DISPLAY_TYPE_GENERAL;
        stuSplitModeInfo.nGroupID = nGroupId;
        BOOL bRet = CLIENT_SetSplitMode(m_stuDevInfo.LoginID, 0, &stuSplitModeInfo, 5000);
        if (!bRet)
        {
            MessageBox(ConvertString("Set split mode failed"), ConvertString("Prompt"));
            return;
        } 
    }

	CDialog::OnOK();
}

void CPreviewParmsDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
