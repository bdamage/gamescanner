#include "StdAfx.h"
#include "Timer.h"

CTimer::CTimer(void)
{
	m_dwStartTick = 0;
}

CTimer::~CTimer(void)
{
}

void CTimer::Start()
{
		m_dwStartTick = GetTickCount();
	
}
void CTimer::Stop()
{
	m_dwStopTick = (GetTickCount() - m_dwStartTick);
}
void CTimer::Print()
{
	
	dbg_print("Timer is %d ms",m_dwStopTick);
}