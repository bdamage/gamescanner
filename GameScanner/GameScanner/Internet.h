#pragma once

class CInternet
{
	char HFD_g_szPath[MAX_PATH+_MAX_FNAME];
	HINTERNET m_hOpen;
	URL_COMPONENTS uc;
public:
	CInternet(void);
	~CInternet(void);
	DWORD GetFileLength(HINTERNET hInt);
	BOOL CrackURL(const TCHAR *url);
	int URLPost(const TCHAR *lpszServer,const char *lpszDest);
	void DisplayErrMsg();
	bool SetPath(char *szPath);
};
