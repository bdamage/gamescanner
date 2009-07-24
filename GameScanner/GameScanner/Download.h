#pragma once

class CDownload
{
	char HFD_g_szPath[MAX_PATH+_MAX_FNAME];
	char HFD_g_szCommonMapPath[MAX_PATH+_MAX_FNAME];  //More thread safe
public:
	CDownload(void);
	~CDownload(void);
	bool SetPath(char *szPath);
	bool SetCommonMapPath(char *szPath);
	int  HttpFileDownload(char* lpszServer, char* lpszDest, HWND hWndProgress, HWND hWndMsg);

};
