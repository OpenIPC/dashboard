#include "StdAfx.h"
#include "Observable.h"
#include "Utils/Lock.h"
#include "Observer.h"

class CObservableImpl
{
public:
	CObservableImpl(void);
	virtual ~CObservableImpl(void);

	void			AddObserver(CObserver* pObserver);
	void			DeleteObserver(CObserver* pObserver);
	void			Notify(int nType, void* pMsg);	

private:
	Mutex					m_mutex;
	std::list<CObserver*>	m_lstObserver;
};

CObservableImpl::CObservableImpl()
{

}

CObservableImpl::~CObservableImpl()
{
	CLock lck(m_mutex);
	m_lstObserver.clear();
}

void CObservableImpl::AddObserver(CObserver* pObserver)
{
	CLock lck(m_mutex);
	m_lstObserver.push_back(pObserver);
}

void CObservableImpl::DeleteObserver(CObserver* pObserver)
{
	CLock lck(m_mutex);
	for (std::list<CObserver*>::iterator it = m_lstObserver.begin(); it != m_lstObserver.end(); )
	{
		CObserver* pTemp = (*it);
		if ((LLONG)pTemp == (LLONG)pObserver)
		{
			m_lstObserver.erase(it++);
			return;
		}
		else
		{
			++it;
		}
	}
}

void CObservableImpl::Notify(int nType, void* pMsg)
{
	CLock lck(m_mutex);
	for (std::list<CObserver*>::iterator it = m_lstObserver.begin(); it != m_lstObserver.end(); ++it)
	{
		CObserver* pTemp = (*it);
		pTemp->Update(nType, pMsg);
	}
}


CObservable::CObservable()
{
	m_pObserableImpl = NEW CObservableImpl;
}

CObservable::~CObservable()
{
	if (m_pObserableImpl)
	{
		delete m_pObserableImpl;
		m_pObserableImpl = NULL;
	}
}

void CObservable::AddObserver(CObserver* pObserver)
{
	if (m_pObserableImpl)
	{
		m_pObserableImpl->AddObserver(pObserver);
	}
}


void CObservable::DeleteObserver(CObserver* pObserver)
{
	if (m_pObserableImpl)
	{
		m_pObserableImpl->DeleteObserver(pObserver);
	}
}

void CObservable::Notify(int nType, void* pMsg)
{
	if (m_pObserableImpl)
	{
		m_pObserableImpl->Notify(nType, pMsg);
	}
}