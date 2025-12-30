#ifndef _OBSERVER_H_
#define _OBSERVER_H_

/************************************************************************/
/*        Observer Class£¬provide Update interface£¬
/*		  Interface class inherits this class and is used to update the interface after receiving the alarm notice                                                              */
/************************************************************************/
class CObserver
{
public:
	virtual void Update(int nType, void* pMsg) = 0;
};

#endif