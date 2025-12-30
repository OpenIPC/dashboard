#ifndef _OBSERVERABLE_H_
#define _OBSERVERABLE_H_

/************************************************************************/
/*        Observable class, an observable object, can register observers                        */
/************************************************************************/
class CObservableImpl;
class CObserver;
class CObservable
{
public:
	CObservable(void);
	virtual ~CObservable(void);

	void			AddObserver(CObserver* pObserver);
	void			DeleteObserver(CObserver* pObserver);

	void			Notify(int nType, void* pMsg);	

private:
	CObservableImpl*	m_pObserableImpl;
};

#endif