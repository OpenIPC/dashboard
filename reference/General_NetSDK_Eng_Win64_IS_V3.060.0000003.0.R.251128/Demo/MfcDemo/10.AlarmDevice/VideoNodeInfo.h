// VideoNodeInfo.h: interface for the VideoNodeInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEONODEINFO_H__11A333F2_E40F_4443_B6EA_FAB5FEB810F3__INCLUDED_)
#define AFX_VIDEONODEINFO_H__11A333F2_E40F_4443_B6EA_FAB5FEB810F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVideoNodeInfo  
{
	//constructor and destructor
public:
	CVideoNodeInfo();
	virtual ~CVideoNodeInfo();
	
public:
	void SetVideoInfo(int iDisplayNum,CString strDvrIP,WORD wPort,int iChannel,CString strUserName,CString strPwd,enum RealPlayMode ePlayMode);
	//DVR log in password 
	void SetDvrPwd(CString strDvrPwd);
	CString GetDvrPwd();
	//DVR log in user name 
	void SetDvrUserName(CString strDvrUserName);
	CString GetDvrUserName();
	//Channel number 
	void SetDvrChannel(int iChannel);
	int GetDvrChannel();
	//DVR port number 
	void SetDvrPort(WORD wPort);
	WORD GetDvrPort();
	//DVR IP address 
	void SetDvrIP(CString strDvrIP);
	CString GetDvrIP();
	//Video number 
	void SetDisplayNum(int iDisplayNum);
	int GetDisplayNum();
	//Video play mode 
	enum RealPlayMode GetPlayMode();
	void SetPlayMode(enum RealPlayMode ePlayMode);

private:
	enum RealPlayMode m_playMode;
	CString m_strDvrPwd;
	CString m_strDvrUserName;
	int m_iDvrChannel;
	WORD m_wDvrPort;
	CString m_strDvrIP;
	int m_iDisplayNum;
};

//Play mode 
//Direct mode, server mode and multiple-window preview mode 
enum RealPlayMode{
	DirectMode,
	ServerMode,
	MultiMode,
	MultiServerMode,
};

#endif // !defined(AFX_VIDEONODEINFO_H__11A333F2_E40F_4443_B6EA_FAB5FEB810F3__INCLUDED_)
