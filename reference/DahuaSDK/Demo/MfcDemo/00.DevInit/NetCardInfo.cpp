#include "NetCardInfo.h"

#include <WinSock2.h>
#include <Iphlpapi.h>
#include <iostream>
#include <string>

using namespace std;

#if defined(_WIN64)
	#pragma comment(lib,"Iphlplib/win64/Iphlpapi.lib") 
#else
	#pragma comment(lib,"Iphlplib/win32/Iphlpapi.lib") 
#endif

class CNetCardInfoImpl
{
public:
	CNetCardInfoImpl();
	~CNetCardInfoImpl();

	void Init();

	void GetNetCardIp(std::set<std::string>& IPVector);

private:
	PIP_ADAPTER_INFO	m_pIpAdapterInfo;
	bool				m_bSuccess;
};

CNetCardInfoImpl::CNetCardInfoImpl():m_pIpAdapterInfo(NULL), m_bSuccess(false)
{
}

CNetCardInfoImpl::~CNetCardInfoImpl()
{
	if (m_pIpAdapterInfo)
	{
		delete m_pIpAdapterInfo;
		m_pIpAdapterInfo = NULL;
	}
	m_bSuccess = false;
}

void CNetCardInfoImpl::Init()
{
	if (m_pIpAdapterInfo)
	{
		delete m_pIpAdapterInfo;
		m_pIpAdapterInfo = NULL;
	}
	m_bSuccess = false;

	m_pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	int nRet = GetAdaptersInfo(m_pIpAdapterInfo, &stSize);
	if (ERROR_BUFFER_OVERFLOW == nRet)
	{
		delete m_pIpAdapterInfo;
		m_pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		nRet = GetAdaptersInfo(m_pIpAdapterInfo, &stSize);
	}
	m_bSuccess = (ERROR_SUCCESS == nRet)?true:false;
}

void CNetCardInfoImpl::GetNetCardIp(std::set<std::string>& IPVector)
{
	if (m_bSuccess)
	{
		IP_ADAPTER_INFO* pIpAdapterInfo = m_pIpAdapterInfo;

		while (pIpAdapterInfo)
		{
			IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
			while(pIpAddrString)
			{
				string strIp(pIpAddrString->IpAddress.String);
				if (strIp != "0.0.0.0")
				{
					IPVector.insert(strIp);
				}
				
				pIpAddrString = pIpAddrString->Next;
			}
			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}
}


CNetCardInfo::CNetCardInfo()
{
	m_pImpl = new CNetCardInfoImpl;
}

CNetCardInfo::~CNetCardInfo()
{
	if (m_pImpl)
	{
		delete m_pImpl;
		m_pImpl = NULL;
	}
}

void CNetCardInfo::Init()
{
	if (m_pImpl)
	{
		m_pImpl->Init();
	}
}

void CNetCardInfo::GetNetCardIp(std::set<std::string>& IPVector)
{
	if (m_pImpl)
	{
		m_pImpl->GetNetCardIp(IPVector);
	}
}