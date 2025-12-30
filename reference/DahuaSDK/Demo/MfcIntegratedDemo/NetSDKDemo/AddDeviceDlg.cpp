// AddDeviceDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "NetSDKDemo.h"
#include "AddDeviceDlg.h"
#include "NetSDKDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//添加设备页面的处理
/////////////////////////////////////////////////////////////////////////////
// AddDeviceDlg dialog

UserInfo *g_userinfo[10];
CString strPath;

inline void dbg_print_ex(int level, const char *msg, ...)
{
	char buf[256];
	
	va_list ap;
	va_start(ap, msg); // use variable arg list
	vsprintf(buf, msg, ap);
	va_end( ap );
}

AddDeviceDlg::AddDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AddDeviceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AddDeviceDlg)
	m_devicename = _T("test");
	m_deviceusername = _T("1");
	m_deviceuserpsw = _T("");
	m_deviceport = 37777;
	m_IPstr = _T("");
	//}}AFX_DATA_INIT

	memset(&stuParam,0,sizeof(stuParam));
}


void AddDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AddDeviceDlg)
	DDX_Control(pDX, IDC_IPSELECTION, m_ipsel);
	DDX_Text(pDX, IDC_DEVICE_NAME, m_devicename);
	DDV_MaxChars(pDX, m_devicename, 20);
	DDX_Text(pDX, IDC_DEVICE_USERNAME, m_deviceusername);
	DDV_MaxChars(pDX, m_deviceusername, 40);
	DDX_Text(pDX, IDC_DEVICE_USERPSW, m_deviceuserpsw);
	DDV_MaxChars(pDX, m_deviceuserpsw, 40);
	DDX_Text(pDX, IDC_DEVICE_PORT, m_deviceport);
	DDX_CBString(pDX, IDC_IPSELECTION, m_IPstr);
	DDV_MaxChars(pDX, m_IPstr, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AddDeviceDlg, CDialog)
	//{{AFX_MSG_MAP(AddDeviceDlg)
	ON_BN_CLICKED(IDC_CONNECT_DEVICE, OnConnectDevice)
	ON_BN_CLICKED(IDC_CANCEL_ADDDEVICE, OnCancelAdddevice)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_IPSELECTION, OnSelchangeIpselection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AddDeviceDlg message handlers

//连接失败的类型
void AddDeviceDlg::GetConnectError(int error)
{	
	//根据返回值不同判断错误类型
	switch(error) {
	case 1://密码不正确
		MessageBox(ConvertString(MSG_ERROR_PASSWORD));
		break;
	case 2://用户名不存在
		MessageBox(ConvertString(MSG_ERROR_USER));
		break;
	case 3://登录超时
		MessageBox(ConvertString(MSG_ERROR_TIMEOUT));
		break;
	case 4://重复登录
		MessageBox(ConvertString(MSG_ERROR_RELOGGIN));
		break;
	case 5://帐号被锁定
		MessageBox(ConvertString(MSG_ERROR_LOCKED));
		break;
	case 6://帐号被列入禁止名单
		MessageBox(ConvertString(MSG_ERROR_BLOCKLIST));
		break;
	case 7://系统忙，资源不足
		MessageBox(ConvertString(MSG_ERROR_BUSY));
		break;
	case 8://子连接失败
		MessageBox(ConvertString(MSG_ERROR_CONNECT));
		break;
	case 9://找不到主机
		MessageBox(ConvertString(MSG_ERROR_CONNECT));
		break;
	case 10://超过最大连接数
		MessageBox(ConvertString(MSG_ERROR_MAXCONNECT));
		break;
	case 11://只支持3代协议
		MessageBox(ConvertString(MSG_ERROR_PROTOCOL3_ONLY));
		break;
	case 12://设备未插入U盾或U盾信息错误
		MessageBox(ConvertString(MSG_ERROR_UKEY_LOST));
		break;
	case 13://客户端IP地址没有登录权限
		MessageBox(ConvertString(MSG_ERROR_ERROR_NO_AUTHORIZED));
		break;
	default:
		MessageBox(ConvertString(MSG_ERROR_NETWORK));
		break;
	}
}


//增加连接设备,连接成功增加到g_ptrdevicelist中
void AddDeviceDlg::OnConnectDevice() 
{
	//CString
	DeviceNode *inode;//临时设备信息节点

	// 判断端口号是否为空，空即提示输入
	CString str;
	GetDlgItem(IDC_DEVICE_PORT)->GetWindowText(str);
	if (str.IsEmpty())
	{
		MessageBox(ConvertString("Input port first!"), ConvertString("Prompt"));
		return ;
	}

	UpdateData(TRUE);	//获取界面输入	
	CLIENT_SetNetworkParam(&stuParam);
	inode = new DeviceNode;
	memset((char *)inode, 0, sizeof(DeviceNode));
	
	//将页面信息写入节点
	_snprintf(inode->IP,sizeof(inode->IP) ,"%s", m_IPstr);
	memcpy(inode->Name,m_devicename.GetBuffer(0),min(m_devicename.GetLength(), 20));
	memcpy(inode->UserNanme, m_deviceusername.GetBuffer(0), m_deviceusername.GetLength()+1);
	//调用登录接口

	NET_IN_LOGIN_WITH_HIGHLEVEL_SECURITY stInparam;
	memset(&stInparam, 0, sizeof(stInparam));
	stInparam.dwSize = sizeof(stInparam);
	strncpy(stInparam.szIP, inode->IP, sizeof(stInparam.szIP) - 1);
	strncpy(stInparam.szPassword, m_deviceuserpsw.GetBuffer(0), sizeof(stInparam.szPassword) - 1);
	strncpy(stInparam.szUserName, m_deviceusername.GetBuffer(0), sizeof(stInparam.szUserName) - 1);
	stInparam.nPort = (int)m_deviceport;
	stInparam.emSpecCap = EM_LOGIN_SPEC_CAP_TCP;

	NET_OUT_LOGIN_WITH_HIGHLEVEL_SECURITY stOutparam;
	memset(&stOutparam, 0, sizeof(stOutparam));
	stOutparam.dwSize = sizeof(stOutparam);

	inode->LoginID = CLIENT_LoginWithHighLevelSecurity(&stInparam, &stOutparam); 
	inode->Info = stOutparam.stuDeviceInfo;

  	//inode->LoginID = CLIENT_Login(inode->IP, (WORD)m_deviceport, 
   //               m_deviceusername.GetBuffer(0), m_deviceuserpsw.GetBuffer(0), &inode->Info,&err);	
	if(0 != inode->LoginID)
	{
		inode->bIsOnline = TRUE;
		int nRetLen = 0;
		NET_DEV_CHN_COUNT_INFO stuChn = { sizeof(NET_DEV_CHN_COUNT_INFO) };
		stuChn.stuVideoIn.dwSize = sizeof(stuChn.stuVideoIn);
		stuChn.stuVideoOut.dwSize = sizeof(stuChn.stuVideoOut);
		if (CLIENT_QueryDevState(inode->LoginID, DH_DEVSTATE_DEV_CHN_COUNT, (char*)&stuChn, stuChn.dwSize, &nRetLen))
		{
            inode->nChnNum = stuChn.stuVideoIn.nMaxTotal;
		}
        else
        {
            inode->nChnNum = inode->Info.nChanNum;
        }
		//登录成功，临时节点生效		
		int r = CDevMgr::GetDevMgr().PushBack(inode);
		if (r < 0)
		{
			AfxMessageBox(ConvertString("device's node add to list failed"));
		}

		//开始侦听设备回调信息		
		BOOL ret = CLIENT_StartListenEx(inode->LoginID);
		if(!ret)
		{
			((CNetSDKDemoDlg *)GetParent())->LastError();
			MessageBox(ConvertString(MSG_ERROR_LISTEN));
		}

		//刷新内存和配置文件里的列表
		bool bIsnew = true;
		int counter;
		CString strKey;
		for (counter = 0; counter < m_ipsel.GetCount(); counter++)
		{
			CString strtemp;
			m_ipsel.GetLBText(counter, strtemp);
			if (strcmp(m_IPstr, g_userinfo[counter]->ip) == 0)
			{
				bIsnew = false;
				UserInfo tmpnode;
				strncpy (tmpnode.devicename, m_devicename.GetBuffer(0), sizeof(tmpnode.devicename) - 1);
				strncpy (tmpnode.ip, m_IPstr.GetBuffer(0), sizeof(tmpnode.ip) -1 );
				tmpnode.port = m_deviceport;
				strncpy (tmpnode.username, m_deviceusername.GetBuffer(0), sizeof(tmpnode.username) - 1);
				strncpy (tmpnode.userpsw, m_deviceuserpsw.GetBuffer(0), sizeof(tmpnode.userpsw) - 1);
				strKey.Format("%d", counter);
				BOOL ret = WritePrivateProfileStruct("UserInfo" , strKey , (LPVOID)&tmpnode, sizeof(UserInfo), strPath);
			}
		}
		//新的节点
		if (bIsnew) 
		{
			if (counter >= 10) 
			{
				counter = 9;
			}
			UserInfo tmpnode;
			strncpy (tmpnode.devicename, m_devicename.GetBuffer(0), sizeof(tmpnode.devicename) - 1);
			strncpy (tmpnode.ip, m_IPstr.GetBuffer(0), sizeof(tmpnode.ip) - 1);
			tmpnode.port = m_deviceport;
			strncpy (tmpnode.username, m_deviceusername.GetBuffer(0), sizeof(tmpnode.username) - 1);
			strncpy (tmpnode.userpsw, m_deviceuserpsw.GetBuffer(0), sizeof(tmpnode.userpsw) - 1);
			strKey.Format("%d", counter);
			BOOL ret = WritePrivateProfileStruct("UserInfo" , strKey , (LPVOID)&tmpnode, sizeof(UserInfo), strPath);
		}
		for (int j = 0; j < 10; j++)
		{
			if (g_userinfo[j]) 
			{
				delete g_userinfo[j];
			}
		}

		CDialog ::OnOK();
	}
	else
	{
		//登录失败的情况
		((CNetSDKDemoDlg *)GetParent())->LastError();
		GetConnectError(stOutparam.nError);
		delete inode;
	}
}

//取消
void AddDeviceDlg::OnCancelAdddevice() 
{
	for (int j = 0; j < 10; j++)
	{
		if (g_userinfo[j]) 
		{
			delete g_userinfo[j];
		}
	}
	CDialog::OnCancel();
}

//初始化页面
BOOL AddDeviceDlg::OnInitDialog() 
{
	g_SetWndStaticText(this);
	CDialog::OnInitDialog();

	UserInfo tmpinfo;
	for (int i = 0; i < 10; i++)
	{
		g_userinfo[i] = NULL;
		CString strKey;
		strKey.Format("%d", i);
		strPath = g_strWorkDir + "\\userinfo.ini";
		if (GetPrivateProfileStruct("UserInfo", strKey, (LPVOID)&tmpinfo, sizeof(UserInfo), strPath))
		{
			g_userinfo[i] = new UserInfo;
			memcpy(g_userinfo[i], &tmpinfo, sizeof(UserInfo));
			m_ipsel.InsertString(i,tmpinfo.ip);
		}
	}
	
	if (m_ipsel.GetCount() > 0) 
	{
		m_devicename.Format("%s", g_userinfo[0]->devicename);
		m_deviceport = g_userinfo[0]->port;
		m_deviceusername.Format("%s", g_userinfo[0]->username);
		m_deviceuserpsw.Format("%s", g_userinfo[0]->userpsw);
		UpdateData(FALSE);
		m_ipsel.SetCurSel(0);
	}
	
	memset(&stuParam, 0 ,sizeof(NET_PARAM));
	stuParam.nWaittime = 8000;
	stuParam.nConnectTime =3000;
	stuParam.nConnectTryNum = 3;
	stuParam.nSubConnectSpaceTime = 100;
    stuParam.nGetConnInfoTime = 3000;
	
	return TRUE;  
}

void AddDeviceDlg::OnClose() 
{
	for (int j = 0; j < 10; j++)
	{
		if (g_userinfo[j]) 
		{
			delete g_userinfo[j];
		}
	}
	CDialog::OnClose();
}

void AddDeviceDlg::OnSelchangeIpselection() 
{
	m_IPstr.Format("%s", g_userinfo[m_ipsel.GetCurSel()]->ip);
	m_devicename.Format("%s", g_userinfo[m_ipsel.GetCurSel()]->devicename);
	m_deviceport = g_userinfo[m_ipsel.GetCurSel()]->port;
	m_deviceusername.Format("%s", g_userinfo[m_ipsel.GetCurSel()]->username);
	m_deviceuserpsw.Format("%s", g_userinfo[m_ipsel.GetCurSel()]->userpsw);
	UpdateData(FALSE);
}