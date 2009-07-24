#pragma once

class CTimer
{
	DWORD m_dwStartTick,m_dwStopTick;
public:
	CTimer(void);
	~CTimer(void);
	void Start();
	void Stop();
	void Print();
};
