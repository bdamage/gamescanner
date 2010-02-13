#pragma once

class CLogger
{
	CRITICAL_SECTION	CS_Logger;
	string UTILZ_sLogger;
	char szLogPath[MAX_PATH];
	

public:
	HWND hwndLogger;
	CLogger(void);
	~CLogger(void);
	void SetLogPath(const char *szPath);
	void AddLogInfo(int color, char *lpszText, ...);
	void AddGetLastErrorIntoLog(char* lpszFunction);
};
