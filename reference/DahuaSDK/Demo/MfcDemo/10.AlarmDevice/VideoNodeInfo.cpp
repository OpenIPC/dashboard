// VideoNodeInfo.cpp: implementation of the VideoNodeInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlarmDevice.h"
#include "VideoNodeInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoNodeInfo::CVideoNodeInfo()
{
	m_iDisplayNum=-1;
	m_strDvrIP="";
	m_wDvrPort=0;
	m_iDvrChannel=-1;
	m_strDvrUserName="";
	m_strDvrPwd="";
	m_playMode=DirectMode;
}

CVideoNodeInfo::~CVideoNodeInfo()
{

}

//To get video signal 
int CVideoNodeInfo::GetDisplayNum()
{
	return m_iDisplayNum;
}

//Set video number 
void CVideoNodeInfo::SetDisplayNum(int iDisplayNum)
{
	m_iDisplayNum=iDisplayNum;
}

//Get DVR IP address 
CString CVideoNodeInfo::GetDvrIP()
{
	return m_strDvrIP;
}

//Set DVR IP address 
void CVideoNodeInfo::SetDvrIP(CString strDvrIP)
{
	m_strDvrIP=strDvrIP;
}

//Get DVR port number 
WORD CVideoNodeInfo::GetDvrPort()
{
	return m_wDvrPort;
}

//Set DVR port number 
void CVideoNodeInfo::SetDvrPort(WORD wPort)
{
	m_wDvrPort=wPort;
}

//Get DVR channel number 
int CVideoNodeInfo::GetDvrChannel()
{
	return m_iDvrChannel;
}

//Set DVR channel number 
void CVideoNodeInfo::SetDvrChannel(int iChannel)
{
	m_iDvrChannel=iChannel;
}

//Get DVR user name 
CString CVideoNodeInfo::GetDvrUserName()
{
	return m_strDvrUserName;
}

//Set DVR user name 
void CVideoNodeInfo::SetDvrUserName(CString strDvrUserName)
{
	m_strDvrUserName=strDvrUserName;
}

//Get DVR password 
CString CVideoNodeInfo::GetDvrPwd()
{
	return m_strDvrPwd;
}

//Set DVR password 
void CVideoNodeInfo::SetDvrPwd(CString strDvrPwd)
{
	m_strDvrPwd=strDvrPwd;
}

//Get video play mode 
enum RealPlayMode CVideoNodeInfo::GetPlayMode()
{
	return m_playMode;
}

//Set video play mode 
void CVideoNodeInfo::SetPlayMode(enum RealPlayMode ePlayMode)
{
	m_playMode=ePlayMode;
}

// Set VideoInfo
void CVideoNodeInfo::SetVideoInfo(int iDisplayNum, CString strDvrIP, WORD wPort, int iChannel, CString strUserName, CString strPwd, enum RealPlayMode ePlayMode)
{
	m_iDisplayNum=iDisplayNum;
	m_strDvrIP=strDvrIP;
	m_wDvrPort=wPort;
	m_iDvrChannel=iChannel;
	m_strDvrUserName=strUserName;
	m_strDvrPwd=strPwd;
	m_playMode=ePlayMode;
}




















































































































































































































































































































































