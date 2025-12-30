#pragma once
#include <set>

class CNetCardInfoImpl;

class CNetCardInfo
{
public:
	CNetCardInfo();
	~CNetCardInfo();

	void Init();
	void GetNetCardIp(std::set<std::string>& IPVector);

private:
	CNetCardInfoImpl* m_pImpl;
};
