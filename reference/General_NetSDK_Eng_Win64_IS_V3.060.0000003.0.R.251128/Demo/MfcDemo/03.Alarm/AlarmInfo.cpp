// AlarmInfo.cpp: implementation of the CAlarmInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlarmInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CAlarmInfoEx::CAlarmInfoEx()
{
	m_nAlarmType = 0;		
	m_nChannel = 0;
	m_emAlarmStatus = ALARM_STOP;
	m_strTime = "";
}

CAlarmInfoEx::~CAlarmInfoEx()
{
	
}