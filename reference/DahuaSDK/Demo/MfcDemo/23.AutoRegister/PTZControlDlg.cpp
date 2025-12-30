// PTZControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveLoginDemo.h"
#include "PTZControlDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPTZControlDlg dialog


CPTZControlDlg::CPTZControlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPTZControlDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPTZControlDlg)
	m_presetData = 0;
	m_posX = 0;
	m_posY = 0;
	m_posZoom = 0;
	//}}AFX_DATA_INIT
    m_lLoginID = 0;
    m_nChannel = 0;
}


void CPTZControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPTZControlDlg)
	DDX_Control(pDX, IDC_COMBO_PTZDATA, m_comboPTZData);
	DDX_Text(pDX, IDC_PRESET_DATA, m_presetData);
    DDX_Control(pDX, IDC_IRIS_OPEN, m_iris_open);
    DDX_Control(pDX, IDC_IRIS_CLOSE, m_iris_close);
    DDX_Control(pDX, IDC_FOCUS_FAR, m_focus_far);
    DDX_Control(pDX, IDC_FOCUS_NEAR, m_focus_near);
    DDX_Control(pDX, IDC_ZOOM_TELE, m_zoom_tele);
    DDX_Control(pDX, IDC_ZOOM_WIDE, m_zoom_wide);
    DDX_Control(pDX, IDC_PTZ_RIGHTDOWN, m_ptz_rightdown);
    DDX_Control(pDX, IDC_PTZ_RIGHTUP, m_ptz_rightup);
    DDX_Control(pDX, IDC_PTZ_LEFTDOWN, m_ptz_leftdown);
    DDX_Control(pDX, IDC_PTZ_LEFTUP, m_ptz_leftup);
    DDX_Control(pDX, IDC_PTZ_RIGHT, m_ptz_right);
    DDX_Control(pDX, IDC_PTZ_LEFT, m_ptz_left);
    DDX_Control(pDX, IDC_PTZ_DOWN, m_ptz_down);
	DDX_Control(pDX, IDC_PTZ_UP, m_ptz_up);
	DDX_Text(pDX, IDC_POS_X, m_posX);
	DDX_Text(pDX, IDC_POS_Y, m_posY);
	DDX_Text(pDX, IDC_POS_ZOOM, m_posZoom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPTZControlDlg, CDialog)
	//{{AFX_MSG_MAP(CPTZControlDlg)
	ON_BN_CLICKED(IDC_PRESET_SET, OnPresetSet)
	ON_BN_CLICKED(IDC_PRESET_ADD, OnPresetAdd)
	ON_BN_CLICKED(IDC_PRESET_DELE, OnPresetDele)
	ON_BN_CLICKED(IDC_FAST_GO, OnFastGo)
	ON_BN_CLICKED(IDC_EXACT_GO, OnExactGo)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPTZControlDlg message handlers

BOOL CPTZControlDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	g_SetWndStaticText(this,DLG_PTZ);
    m_ptz_up.SetButtonCommand(DH_PTZ_UP_CONTROL);
    m_ptz_down.SetButtonCommand(DH_PTZ_DOWN_CONTROL);
    m_ptz_left.SetButtonCommand(DH_PTZ_LEFT_CONTROL);
    m_ptz_right.SetButtonCommand(DH_PTZ_RIGHT_CONTROL);
    m_zoom_wide.SetButtonCommand(DH_PTZ_ZOOM_DEC_CONTROL);
    m_zoom_tele.SetButtonCommand(DH_PTZ_ZOOM_ADD_CONTROL);
    m_focus_near.SetButtonCommand(DH_PTZ_FOCUS_DEC_CONTROL);
    m_focus_far.SetButtonCommand(DH_PTZ_FOCUS_ADD_CONTROL);
    m_iris_open.SetButtonCommand(DH_PTZ_APERTURE_ADD_CONTROL);
    m_iris_close.SetButtonCommand(DH_PTZ_APERTURE_DEC_CONTROL);
    
    m_ptz_rightup.SetButtonCommand(DH_EXTPTZ_RIGHTTOP);
    m_ptz_rightdown.SetButtonCommand(DH_EXTPTZ_RIGHTDOWN);
    m_ptz_leftup.SetButtonCommand(DH_EXTPTZ_LEFTTOP);
	m_ptz_leftdown.SetButtonCommand(DH_EXTPTZ_LEFTDOWN);
	m_comboPTZData.SetCurSel(3);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPTZControlDlg::OnPresetSet() 
{
    BOOL bValid = UpdateData(TRUE);
    if(bValid)
    {
        PtzExtControl(DH_PTZ_POINT_MOVE_CONTROL);
	}
}

//PTZ control extensive function 
void CPTZControlDlg::PtzExtControl(DWORD dwCommand, DWORD dwParam)
{
	if(0 != m_lLoginID)
	{
		//Get channel number 
		CString strDispNum;
		CString strAuxNo;
		if(-1 == m_nChannel)
		{
			return;
		}
		long param1=0,param2=0,param3 =0;
		switch(dwCommand)
        {
		case DH_PTZ_POINT_MOVE_CONTROL:
			//Go to preset 
			param1=0;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_PTZ_POINT_SET_CONTROL:
			//Add preset 
			param1=0;
			param2=(long)m_presetData;
			param3=0;
			break;
		case DH_PTZ_POINT_DEL_CONTROL:
			//Delete preset 
			param1=0;
			param2=(long)m_presetData;
			param3=0;
			break;
// 		case DH_PTZ_POINT_LOOP_CONTROL:
// 			//Begin tour/stop tour 
// 			if (0 == dwParam) 
// 			{
// 				param1=(long)m_crviseGroup;
// 				param2=0;
// 				param3=76;
// 			}
// 			else
// 			{
// 				param1=(long)m_crviseGroup;
// 				param2=0;
// 				param3=96;
// 			}
// 			break;
// 		case DH_EXTPTZ_ADDTOLOOP:
// 			//Add tour 
// 			param1=(long)m_crviseGroup;
// 			param2=(long)m_presetData;
// 			param3=0;
// 			break;
// 		case DH_EXTPTZ_DELFROMLOOP:
// 			//Delete tour 
// 			param1=(long)m_crviseGroup;
// 			param2=(long)m_presetData;
// 			param3=0;
// 			break;
// 		case DH_EXTPTZ_CLOSELOOP:
// 			//Delete tour group 
// 			param1=(long)m_crviseGroup;
// 			param2=0;//(long)m_presetData;
// 			param3=0;
// 			break;
// 		case DH_EXTPTZ_SETMODESTART:
// 			//begin record 
// 			param1=(long)m_moveNo;
// 			param2=0;
// 			param3=0;
// 			break;
// 		case DH_EXTPTZ_SETMODESTOP:
// 			//Stop record 
// 			param1=(long)m_moveNo;
// 			param2=0;
// 			param3=0;
// 			break;
		case DH_EXTPTZ_SETLEFTBORDER:
			//set left limit
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_SETRIGHTBORDER:
			//set right limit
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STARTLINESCAN:
			//begin scan 
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_CLOSELINESCAN:
			//Stop scan 
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_FASTGOTO:
		case DH_EXTPTZ_EXACTGOTO:
			//3D intelligent position 
			param1=m_posX;
			param2=m_posY;
			param3=m_posZoom;
			break;
		case DH_EXTPTZ_RESETZERO:
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STARTPANCRUISE:
			//Begin rotation
			param1=0;
			param2=0;
			param3=0;
			break;
		case DH_EXTPTZ_STOPPANCRUISE:
			//Stop rotation 
			param1=0;
			param2=0;
			param3=0;
			break;
		default:
			break;
		}
		BOOL bRet=CLIENT_DHPTZControlEx(m_lLoginID,m_nChannel,dwCommand,param1,param2,param3,FALSE);
		if(bRet)
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Success",DLG_PTZ));
		}
		else
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Fail",DLG_PTZ));
		}
	}
}

//PTZ control 
void CPTZControlDlg::PtzControl(int type, BOOL stop)
{
	SetDlgItemText(IDC_PTZSTATUS,"");
	if(0 != m_lLoginID)
	{
		//Get channel number
		if(-1 == m_nChannel)
		{
			return;
		}
		BYTE param1=0,param2=0;
		BYTE bPTZData=(unsigned char)GetDlgItemInt(IDC_COMBO_PTZDATA);
		switch(type) {
		case DH_PTZ_UP_CONTROL:
			//Up
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_DOWN_CONTROL:
			//Down
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_LEFT_CONTROL:
			//Left 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_RIGHT_CONTROL:
			//Right 
			param1=0;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_LEFTTOP:
			//Up left
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_LEFTDOWN:
			//Up down 
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_RIGHTTOP:
			//Up right 
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_EXTPTZ_RIGHTDOWN:
			//Up down 
			param1=bPTZData;
			param2=bPTZData;
			break;
		case DH_PTZ_ZOOM_DEC_CONTROL:
			//Zoom out 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_ZOOM_ADD_CONTROL:
			//Zoom in 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_FOCUS_DEC_CONTROL:
			//Focus zoom in 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_FOCUS_ADD_CONTROL:
			//Focus zoom out 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_APERTURE_DEC_CONTROL:
			//Aperture zoom out 
			param1=0;
			param2=bPTZData;
			break;
		case DH_PTZ_APERTURE_ADD_CONTROL:
			//Aperture zoom in 
			param1=0;
			param2=bPTZData;
			break;
		default:
			break;
		}
		BOOL bRet=CLIENT_DHPTZControl(m_lLoginID,m_nChannel,type,param1,param2,0,stop);
		if(bRet)
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Success",DLG_PTZ));
		}
		else
		{
			SetDlgItemText(IDC_PTZSTATUS, ConvertString("Fail",DLG_PTZ));
		}
	}
}

void CPTZControlDlg::OnPresetAdd() 
{
    BOOL bValid = UpdateData(TRUE);
    if(bValid)
    {
        PtzExtControl(DH_PTZ_POINT_SET_CONTROL);
	}	
}

void CPTZControlDlg::OnPresetDele() 
{
    BOOL bValid = UpdateData(TRUE);
    if(bValid)
    {
        PtzExtControl(DH_PTZ_POINT_DEL_CONTROL);
	}
}

void CPTZControlDlg::OnFastGo() 
{
    BOOL bValid = UpdateData(TRUE);
    if(bValid)
    {
        PtzExtControl(DH_EXTPTZ_FASTGOTO);
	}
}

void CPTZControlDlg::OnExactGo() 
{
    BOOL bValid = UpdateData(TRUE);
    if(bValid)
    {
        PtzExtControl(DH_EXTPTZ_EXACTGOTO);
	}
}

void CPTZControlDlg::OnReset() 
{
    BOOL bValid = UpdateData(TRUE);
    if(bValid)
    {
        PtzExtControl(DH_EXTPTZ_RESETZERO);
	}
}
