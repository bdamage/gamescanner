#include "StdAfx.h"
#include "Internet.h"

CInternet::CInternet(void)
{
	m_hOpen = NULL;
	m_hOpen = InternetOpen (TEXT("HttpFileDownload"), INTERNET_OPEN_TYPE_PRECONFIG , NULL, 0, 0);  

	if (!m_hOpen)
	{
		TCHAR szErrMsg[512];
		_stprintf_s(szErrMsg, TEXT("%s: %x\n"), TEXT("InternetOpen Error"),  GetLastError());
		OutputDebugString(szErrMsg);
	}
}

CInternet::~CInternet(void)
{
	// Close the Internet handles.
	if (m_hOpen)
	{
		if (!InternetCloseHandle (m_hOpen))
		{
			TCHAR szErrMsg[512];
			_stprintf_s (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), GetLastError());
			OutputDebugString(szErrMsg);
		}
	}
}

DWORD CInternet::GetFileLength(HINTERNET hInt)
{
   TCHAR szErrMsg[512];
   char szLength[32];
   DWORD dwLength=0,dwBuffLen=sizeof(szLength);

   if(HttpQueryInfo (hInt, HTTP_QUERY_CONTENT_LENGTH, &szLength,&dwBuffLen, NULL)==FALSE)
   {
		_stprintf_s(szErrMsg, TEXT("%s: %d\n"), TEXT("HttpQueryInfo Error"),GetLastError());
		OutputDebugString(szErrMsg);
		DisplayErrMsg();
		return 0;
   }
   dwLength = (DWORD)atol(szLength);
#ifdef _DEBUG
   wsprintf (szErrMsg, TEXT("File Size %d\n"),dwLength);
   OutputDebugString(szErrMsg);
#endif
   return dwLength;
}

int CInternet::URLPost(const TCHAR *lpszServer, const char *lpszDest)
{
	int bReturn = 0;
//	HINTERNET  hConnect = NULL, hRequest = NULL;
	
	TCHAR szErrMsg[512];
  	char *lpBufferA=NULL;

	LPTSTR AcceptTypes[10] = {TEXT("*/*"), NULL}; 

    DWORD dwRequestFlag = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;

	if(strstr(lpszServer,"https://")!=NULL) //check if it is a HTTPS server
		dwRequestFlag = INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

	TCHAR szHeaders[] = _T("Content-Type: application/x-www-form-urlencoded"); //content type for post...
	
	TCHAR *HostName = _tcsdup(uc.lpszHostName);
	HostName[uc.dwHostNameLength] = '\0';
	TCHAR *FileName = _tcsdup(uc.lpszUrlPath);
	FileName[uc.dwUrlPathLength] = '\0';
	HINTERNET hCO = InternetConnect(m_hOpen, HostName, uc.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	HINTERNET hIS = HttpOpenRequest(hCO, _T("POST"), FileName, NULL, NULL, (LPCTSTR*)AcceptTypes, dwRequestFlag, 0);

again:
	HINTERNET hOU = InternetOpenUrl (m_hOpen, lpszServer, NULL, 0,dwRequestFlag, 0);
	//DWORD dwLength = GetFileLength(hOU);
	if (!HttpSendRequest(hIS, szHeaders, _tcslen(szHeaders), (TCHAR*)&uc.lpszUrlPath[1], _tcslen(&uc.lpszUrlPath[1])))
	{
		DWORD LastError = GetLastError();
		if(LastError == ERROR_INTERNET_INVALID_CA)
		{
			DWORD dwFlags;
			DWORD dwBuffLen = sizeof(dwFlags);

			InternetQueryOption (hIS, INTERNET_OPTION_SECURITY_FLAGS,
			(LPVOID)&dwFlags, &dwBuffLen);

			dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
			InternetSetOption (hIS, INTERNET_OPTION_SECURITY_FLAGS,
			&dwFlags, sizeof (dwFlags) );
			goto again;

		}
	}
	
	FILE *stream = NULL;
	errno_t err = _set_fmode(_O_BINARY);
	if (err == EINVAL)
	{ 
	  OutputDebugString("Invalid mode.\n");
	  return 5;
	}
	
	DWORD dynamicByte = 32000;
	DWORD downloadBytes = 0;
	DWORD dwSize = 0; 
	DWORD availableSize=0;
	do
	{
		lpBufferA = new CHAR [dynamicByte];
		InternetQueryDataAvailable(hIS,&availableSize,0,0);
		sprintf_s(szErrMsg,sizeof(szErrMsg),"Downloaded %d of %d KBytes\n",downloadBytes/1000,availableSize/1000);
		OutputDebugString(szErrMsg);	
	
		if (!InternetReadFile (hIS, (LPVOID)lpBufferA, dynamicByte, &dwSize))
		{
			_stprintf_s(szErrMsg, TEXT("%s: %x\n"), TEXT("InternetReadFile Error"),GetLastError());
			OutputDebugString(szErrMsg);
			delete[] lpBufferA;
			goto exit;
		}

		if (dwSize != 0)    
		{	
			downloadBytes+=dwSize;
			if((stream==NULL) && (bReturn==0))
		   		fopen_s(&stream, lpszDest, "w+" );

			if(stream!=NULL)
				 fwrite( lpBufferA, sizeof( CHAR ), dwSize, stream );		
		}    
		if(lpBufferA)
			delete[] lpBufferA; 
		dynamicByte+=1024;
		if(dynamicByte>128000)
			dynamicByte = 128000;

	} while (dwSize);

	if(stream!=NULL)
		fclose(stream);

  goto exitWithNoErr;
exit:
	bReturn = 1;
exitWithNoErr:
	free(HostName);
	free(FileName);

	if (hIS)
	{
		if (!InternetCloseHandle (hIS))
		{
			_stprintf_s(szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), GetLastError());
			OutputDebugString(szErrMsg);
		}
	}
	if (hCO)
	{
		if (!InternetCloseHandle (hCO))
		{
			_stprintf_s(szErrMsg, TEXT("%s: %x"), TEXT("ConnectOpen close Error"), GetLastError());
			OutputDebugString(szErrMsg);
		}
	}
    _stprintf_s(szErrMsg, TEXT("Return %d"), bReturn);
	OutputDebugString(szErrMsg);
	return bReturn;
}

BOOL CInternet::CrackURL(const TCHAR *url)
{
	//let's split the url...
	uc.dwStructSize = sizeof(uc);
	uc.lpszScheme = NULL;
	uc.dwSchemeLength = 0;
	uc.lpszHostName = NULL;
	uc.dwHostNameLength = 1;
	uc.nPort = 0;
	uc.lpszUserName = NULL;
	uc.dwUserNameLength = 0;
	uc.lpszPassword = NULL;
	uc.dwPasswordLength = 0;
	uc.lpszUrlPath = NULL;
	uc.dwUrlPathLength = 1;
	uc.lpszExtraInfo = NULL;
	uc.dwExtraInfoLength = 0;
	return InternetCrackUrl(url, _tcslen(url), 0, &uc);
}


void CInternet::DisplayErrMsg()
{

	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
		GetModuleHandle(_T("WinINet.dll")),
		GetLastError(),
		0, // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	// Process any inserts in lpMsgBuf.
	// ...
	// Display the string.

	MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
	// Free the buffer.
	LocalFree( lpMsgBuf );
}


bool CInternet::SetPath(char *szPath)
{
	memset(HFD_g_szPath,0,sizeof(HFD_g_szPath));
	int len = strlen(szPath);
	if(len>=sizeof(HFD_g_szPath))
	{
		OutputDebugString("Error setting path in IP country dll!\n");
		return false;
		
	}

	strncpy_s(HFD_g_szPath,sizeof(HFD_g_szPath),szPath,len);
	SetCurrentDirectory(HFD_g_szPath);
	OutputDebugString("IP Country Path: ");
	OutputDebugString(HFD_g_szPath);
	OutputDebugString("\n\n");
	return true;

}