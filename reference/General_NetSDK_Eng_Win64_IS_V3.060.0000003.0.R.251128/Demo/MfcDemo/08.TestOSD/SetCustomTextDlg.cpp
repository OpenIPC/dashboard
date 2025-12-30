// SetCustomTextDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TestOSD.h"
#include "SetCustomTextDlg.h"
#include "TestOSDDlg.h"

extern LLONG g_lLogin;
extern unsigned int g_iChannelIndex;
extern ComboItemInfo arComboItemInfo[6];
extern ComboItemInfo arComboItemInfo2[9];

NET_OSD_CUSTOM_TITLE g_stCustomTitle = {sizeof(g_stCustomTitle)};
NET_OSD_CUSTOM_TITLE_TEXT_ALIGN g_stTextAlign = {sizeof(g_stTextAlign)};


int arEditID[] = {IDC_EDIT_COVER_TEXT1,IDC_EDIT_COVER_TEXT2,IDC_EDIT_COVER_TEXT3,IDC_EDIT_COVER_TEXT4,IDC_EDIT_COVER_TEXT5,IDC_EDIT_COVER_TEXT6};
int arCheckID[] = {IDC_CHECK_COVER_TEXT1,IDC_CHECK_COVER_TEXT2,IDC_CHECK_COVER_TEXT3,IDC_CHECK_COVER_TEXT4,IDC_CHECK_COVER_TEXT5,IDC_CHECK_COVER_TEXT6};
int arFColorID[] = {IDC_TEXT_FCOLOR1,IDC_TEXT_FCOLOR2,IDC_TEXT_FCOLOR3,IDC_TEXT_FCOLOR4,IDC_TEXT_FCOLOR5,IDC_TEXT_FCOLOR6};
int arBColorID[] = {IDC_TEXT_BCOLOR1,IDC_TEXT_BCOLOR2,IDC_TEXT_BCOLOR3,IDC_TEXT_BCOLOR4,IDC_TEXT_BCOLOR5,IDC_TEXT_BCOLOR6};
int arTextPosXID[] = {IDC_EDIT_TIME_TITLE_X1,IDC_EDIT_TIME_TITLE_X2,IDC_EDIT_TIME_TITLE_X3,IDC_EDIT_TIME_TITLE_X4,IDC_EDIT_TIME_TITLE_X5,IDC_EDIT_TIME_TITLE_X6};
int arTextPosYID[] = {IDC_EDIT_TIME_TITLE_Y1,IDC_EDIT_TIME_TITLE_Y2,IDC_EDIT_TIME_TITLE_Y3,IDC_EDIT_TIME_TITLE_Y4,IDC_EDIT_TIME_TITLE_Y5,IDC_EDIT_TIME_TITLE_Y6};
int arAlignID[] = {IDC_COMBO_TYPE_TIME1,IDC_COMBO_TYPE_TIME2,IDC_COMBO_TYPE_TIME3,IDC_COMBO_TYPE_TIME4,IDC_COMBO_TYPE_TIME5,IDC_COMBO_TYPE_TIME6};


// CSetCustomTextDlg 对话框

IMPLEMENT_DYNAMIC(CSetCustomTextDlg, CDialog)

CSetCustomTextDlg::CSetCustomTextDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetCustomTextDlg::IDD, pParent)
{

}

CSetCustomTextDlg::~CSetCustomTextDlg()
{
}

void CSetCustomTextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetCustomTextDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_TEXT, OnBnClickedButtonSaveText)
    ON_BN_CLICKED(IDC_BUTTON_GET, OnBnClickedButtonGet)
    ON_CBN_SELCHANGE(IDC_COMBO_COVER, OnCbnSelchangeComboCover)
END_MESSAGE_MAP()


// CSetCustomTextDlg 消息处理程序

void CSetCustomTextDlg::OnBnClickedButtonSaveText()
{
    // TODO: 在此添加控件通知处理程序代码

    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_COVER);
    g_stCustomTitle.emOsdBlendType = (NET_EM_OSD_BLEND_TYPE)(pkCombo->GetItemData(pkCombo->GetCurSel()));
    int iNum = 0;
    for (int i = 0;i < _countof(arEditID) && i < g_stCustomTitle.nCustomTitleNum;i++)
    {
        CString szText = "";       
        GetDlgItemText(arEditID[i],szText);     

        strncpy(g_stCustomTitle.stuCustomTitle[iNum].szText,szText,sizeof(g_stCustomTitle.stuCustomTitle[iNum].szText) - 1);

        g_stCustomTitle.stuCustomTitle[iNum].bEncodeBlend = ((CButton*)(GetDlgItem(arCheckID[i])))->GetCheck();
        BYTE byR = 0;
        BYTE byG = 0;
        BYTE byB = 0;
        BYTE byA = 0;
        ((CIPAddressCtrl*)(GetDlgItem(arFColorID[i])))->GetAddress(byR,byG,byB,byA);
        g_stCustomTitle.stuCustomTitle[iNum].stuFrontColor.nRed = byR;
        g_stCustomTitle.stuCustomTitle[iNum].stuFrontColor.nGreen = byG;
        g_stCustomTitle.stuCustomTitle[iNum].stuFrontColor.nBlue = byB;
        g_stCustomTitle.stuCustomTitle[iNum].stuFrontColor.nAlpha = byA;

        ((CIPAddressCtrl*)(GetDlgItem(arBColorID[i])))->GetAddress(byR,byG,byB,byA);
        g_stCustomTitle.stuCustomTitle[iNum].stuBackColor.nRed = byR;
        g_stCustomTitle.stuCustomTitle[iNum].stuBackColor.nGreen = byG;
        g_stCustomTitle.stuCustomTitle[iNum].stuBackColor.nBlue = byB;
        g_stCustomTitle.stuCustomTitle[iNum].stuBackColor.nAlpha = byA;

        g_stCustomTitle.stuCustomTitle[iNum].stuRect.nLeft = GetDlgItemInt(arTextPosXID[i]);
        g_stCustomTitle.stuCustomTitle[iNum].stuRect.nTop = GetDlgItemInt(arTextPosYID[i]);
        g_stCustomTitle.stuCustomTitle[iNum].stuRect.nRight = GetDlgItemInt(arTextPosXID[i]);
        g_stCustomTitle.stuCustomTitle[iNum].stuRect.nBottom = GetDlgItemInt(arTextPosYID[i]);

        g_stTextAlign.emTextAlign[iNum] = (EM_TITLE_TEXT_ALIGNTYPE)((CComboBox*)(GetDlgItem(arAlignID[i])))->GetItemData(((CComboBox*)(GetDlgItem(arAlignID[i])))->GetCurSel());

        iNum++;
    }

    g_stCustomTitle.nCustomTitleNum = iNum;    

    g_stCustomTitle.dwSize = sizeof(g_stCustomTitle);
    BOOL bRet = CLIENT_SetConfig(g_lLogin,NET_EM_CFG_CUSTOMTITLE,g_iChannelIndex,&g_stCustomTitle,sizeof(g_stCustomTitle));
    if (bRet)
    {
        MessageBox(ConvertString("set custom title text success!"),ConvertString("Prompt"));
    }
    else
    {
        CString szText = "";
        szText.Format(ConvertString("set custom title text failed! error code(%d)"),CLIENT_GetLastError());
        MessageBox(szText,ConvertString("Prompt"));
    }

    g_stTextAlign.dwSize = sizeof(g_stTextAlign);
    g_stTextAlign.nCustomTitleNum = iNum;
    bRet = CLIENT_SetConfig(g_lLogin,NET_EM_CFG_CUSTOMTITLETEXTALIGN,g_iChannelIndex,&g_stTextAlign,sizeof(g_stTextAlign));
    if (bRet)
    {
        MessageBox(ConvertString("set custom title TextAlgin success"),ConvertString("Prompt"));
    }
    else
    {
        CString szText = "";
        szText.Format(ConvertString("set custom title TextAlgin failed! error code(%d)"),CLIENT_GetLastError());
        MessageBox(szText,ConvertString("Prompt"));
    }
}

BOOL CSetCustomTextDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
	g_SetWndStaticText(this);

    // TODO:  在此添加额外的初始化    

    InitCombo(this,IDC_COMBO_COVER,arComboItemInfo,_countof(arComboItemInfo));

    InitCombo(this,IDC_COMBO_TYPE_TIME1,arComboItemInfo2,_countof(arComboItemInfo2));
    InitCombo(this,IDC_COMBO_TYPE_TIME2,arComboItemInfo2,_countof(arComboItemInfo2));
    InitCombo(this,IDC_COMBO_TYPE_TIME3,arComboItemInfo2,_countof(arComboItemInfo2));
    InitCombo(this,IDC_COMBO_TYPE_TIME4,arComboItemInfo2,_countof(arComboItemInfo2));
    InitCombo(this,IDC_COMBO_TYPE_TIME5,arComboItemInfo2,_countof(arComboItemInfo2));
    InitCombo(this,IDC_COMBO_TYPE_TIME6,arComboItemInfo2,_countof(arComboItemInfo2));

    //UpdateCustomText();
    //UpdateCustomTextAlign();   

	OnBnClickedButtonGet();

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}




void CSetCustomTextDlg::UpdateCustomText()
{  
    //memset(&g_stCustomTitle,0,sizeof(g_stCustomTitle));

    CComboBox* pkCombo = (CComboBox*)GetDlgItem(IDC_COMBO_COVER);
    g_stCustomTitle.dwSize = sizeof(g_stCustomTitle);
    g_stCustomTitle.emOsdBlendType = (NET_EM_OSD_BLEND_TYPE)(pkCombo->GetItemData(pkCombo->GetCurSel()));

    BOOL bRet = CLIENT_GetConfig(g_lLogin,NET_EM_CFG_CUSTOMTITLE,g_iChannelIndex,&g_stCustomTitle,sizeof(g_stCustomTitle));
    if (!bRet)
    {
        CString szText = "";
        szText.Format(ConvertString("get custom title failed!error code(%d)"),CLIENT_GetLastError());
        AfxMessageBox(szText,TRUE);
    }
    else
    {
		int i = 0;
        for (i = 0;i < g_stCustomTitle.nCustomTitleNum && i < _countof(arEditID);i++)
        {
            SetDlgItemText(arEditID[i],g_stCustomTitle.stuCustomTitle[i].szText);
            ((CButton*)(GetDlgItem(arCheckID[i])))->SetCheck(g_stCustomTitle.stuCustomTitle[i].bEncodeBlend);
            ((CIPAddressCtrl*)(GetDlgItem(arFColorID[i])))->SetAddress(
                g_stCustomTitle.stuCustomTitle[i].stuFrontColor.nRed,
                g_stCustomTitle.stuCustomTitle[i].stuFrontColor.nGreen,
                g_stCustomTitle.stuCustomTitle[i].stuFrontColor.nBlue,
                g_stCustomTitle.stuCustomTitle[i].stuFrontColor.nAlpha);

            ((CIPAddressCtrl*)(GetDlgItem(arBColorID[i])))->SetAddress(
                g_stCustomTitle.stuCustomTitle[i].stuBackColor.nRed,
                g_stCustomTitle.stuCustomTitle[i].stuBackColor.nGreen,
                g_stCustomTitle.stuCustomTitle[i].stuBackColor.nBlue,
                g_stCustomTitle.stuCustomTitle[i].stuBackColor.nAlpha);

            SetDlgItemInt(arTextPosXID[i],g_stCustomTitle.stuCustomTitle[i].stuRect.nLeft);
            SetDlgItemInt(arTextPosYID[i],g_stCustomTitle.stuCustomTitle[i].stuRect.nTop);
        }

        EnableCtrls(arEditID,_countof(arEditID),FALSE);
        EnableCtrls(arCheckID,_countof(arCheckID),FALSE);
        EnableCtrls(arFColorID,_countof(arFColorID),FALSE);
        EnableCtrls(arBColorID,_countof(arBColorID),FALSE);
        EnableCtrls(arTextPosXID,_countof(arTextPosXID),FALSE);
        EnableCtrls(arTextPosYID,_countof(arTextPosYID),FALSE);

        for (i = 0;i < g_stCustomTitle.nCustomTitleNum && i < _countof(arEditID);i++)
        {
            EnableCtrl(arEditID[i]);
            EnableCtrl(arCheckID[i]);
            EnableCtrl(arFColorID[i]);
            EnableCtrl(arBColorID[i]);
            EnableCtrl(arTextPosXID[i]);
            EnableCtrl(arTextPosYID[i]);
        }

        MessageBox(ConvertString("get custom title config success!"),ConvertString("Prompt"));
    }    

    return;
}

void CSetCustomTextDlg::UpdateCustomTextAlign()
{
    g_stTextAlign.dwSize = sizeof(g_stTextAlign);
	int i = 0;
    BOOL bRet = CLIENT_GetConfig(g_lLogin,NET_EM_CFG_CUSTOMTITLETEXTALIGN,g_iChannelIndex,&g_stTextAlign,sizeof(g_stTextAlign));
    if (!bRet)
    {
        CString szText = "";
        szText.Format(ConvertString("get custom title TextAlgin config failed!error code(%d)"),CLIENT_GetLastError());
        AfxMessageBox(szText,TRUE);       
    }
    else
    {
        for (i = 0;i < g_stTextAlign.nCustomTitleNum && i < _countof(arAlignID);i++)
        {
            SetComboWithValue(this,arAlignID[i],g_stTextAlign.emTextAlign[i]);
        }

        EnableCtrls(arAlignID,_countof(arAlignID),FALSE);

        for (i = 0;i < g_stTextAlign.nCustomTitleNum && i < _countof(arAlignID);i++)
        {
            EnableCtrl(arAlignID[i]);
        }

        MessageBox(ConvertString("get custom title TextAlgin config success!"),ConvertString("Prompt"));
    }    

    return;
}

void CSetCustomTextDlg::EnableCtrl( UINT uID,BOOL bEnable /*= TRUE*/ )
{
    if (GetDlgItem(uID))
    {
        GetDlgItem(uID)->EnableWindow(bEnable);
    }
}

void CSetCustomTextDlg::EnableCtrls( int arIDs[],int iNum,BOOL bEnable /*= TRUE*/ )
{
    for (int i = 0;i < iNum;i++)
    {
        EnableCtrl(arIDs[i],bEnable);
    }
}

void CSetCustomTextDlg::OnBnClickedButtonGet()
{
    // TODO: 在此添加控件通知处理程序代码

    UpdateCustomText();
    UpdateCustomTextAlign();

    EnableCtrl(IDC_BUTTON_SAVE_TEXT,TRUE);    
}

void CSetCustomTextDlg::OnCbnSelchangeComboCover()
{
    // TODO: 在此添加控件通知处理程序代码
    OnBnClickedButtonGet();
}
