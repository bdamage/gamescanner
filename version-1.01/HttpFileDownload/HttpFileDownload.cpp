// HttpFileDownload.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "HttpFileDownload.h"
#include <fcntl.h>     /* for _O_TEXT and _O_BINARY */
#include <errno.h>     /* for EINVAL */
#include <commctrl.h>
#include <stdlib.h>


char HFD_g_szPath[MAX_PATH+_MAX_FNAME];
char HFD_g_szCommonMapPath[MAX_PATH+_MAX_FNAME];  //More thread safe

//void DisplayErrMsg();

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#define IDC_STATIC_TEXT                 1005


HTTPFILEDOWNLOAD_API  bool HFD_SetPath(char *szPath)
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

HTTPFILEDOWNLOAD_API  bool HFD_SetCommonMapPath(char *szPath)
{
	memset(HFD_g_szCommonMapPath,0,sizeof(HFD_g_szCommonMapPath));
	int len = strlen(szPath);
	if(len>=sizeof(HFD_g_szCommonMapPath))
	{
		OutputDebugString("Error setting path in IP country dll!\n");
		return false;
		
	}

	strncpy_s(HFD_g_szCommonMapPath,sizeof(HFD_g_szCommonMapPath),szPath,len);
	SetCurrentDirectory(HFD_g_szCommonMapPath);
	OutputDebugString("IP Country Path: ");
	OutputDebugString(HFD_g_szCommonMapPath);
	OutputDebugString("\n\n");
	return true;

}

// This is an example of an exported function.
HTTPFILEDOWNLOAD_API int  HttpFileDownload(char* lpszServer, char* lpszDest, HWND hWndProgress, HWND hWndMsg)
{

  int bReturn = 0;
  
  HINTERNET hOpen = NULL, 
            hConnect = NULL, 
            hRequest = NULL;

  DWORD dwSize = 0, 
        dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE; 

  TCHAR szErrMsg[512];
  
  char *lpBufferA;

  LPTSTR AcceptTypes[10] = {TEXT("*/*"), NULL}; 
//BOOL retBool = InternetCheckConnection(TEXT("etsv.cludden.se"),FLAG_ICC_FORCE_CONNECTION,0);
  

  hOpen = InternetOpen (TEXT("HttpFileDownload"), INTERNET_OPEN_TYPE_PRECONFIG , NULL, 0, 0);  //INTERNET_OPEN_TYPE_PRECONFIG

  if (!hOpen)
  {
    wsprintf (szErrMsg, TEXT("%s: %x\n"), TEXT("InternetOpen Error"),  GetLastError());
	OutputDebugString(szErrMsg);
    return 1;
  }
  
	hRequest = InternetOpenUrl (hOpen, lpszServer, NULL, 0,INTERNET_FLAG_RELOAD, 0);
    if (hRequest==NULL)
    {
		wsprintf (szErrMsg, TEXT("%s: %x\n"), TEXT("InternetOpenUrl Error"),GetLastError());
		OutputDebugString(szErrMsg);
		bReturn = 2;
		goto exit;
    }
 
    char szLength[32];
	
	DWORD dwLength=0,dwBuffLen=sizeof(szLength);

   if(HttpQueryInfo (hRequest, HTTP_QUERY_CONTENT_LENGTH, &szLength,&dwBuffLen, NULL)==FALSE)
   {
		wsprintf (szErrMsg, TEXT("%s: %x\n"), TEXT("HttpQueryInfo Error"),GetLastError());
		OutputDebugString(szErrMsg);
   }
	wsprintf (szErrMsg, TEXT("Size to download %d\n"),dwLength);
	OutputDebugString(szErrMsg);

   dwLength = (DWORD)atol(szLength) ;

   FILE *stream;
   errno_t err;
   int  pmode = -1;
	err = _set_fmode(_O_BINARY);
   if (err == EINVAL)
   { 
	  OutputDebugString("Invalid mode.\n");
      return 5;
   }
   	fopen_s(&stream, lpszDest, "w+" );

	if(hWndProgress!=NULL)
	 SendMessage(hWndProgress, PBM_SETRANGE, (WPARAM) 0,MAKELPARAM(0, 100));

	
  DWORD dynamicByte = 32000;
  DWORD downloadBytes = 0;
  do
  {
	 lpBufferA = new CHAR [dynamicByte];

	 sprintf_s(szErrMsg,sizeof(szErrMsg),"Downloaded %d of %d KBytes\n",downloadBytes/1000,dwLength/1000);
	 OutputDebugString(szErrMsg);

	 if(hWndProgress!=NULL)
	 {
		float percentage =  ((float)downloadBytes /  (float)dwLength) * 100.0f ;
		if(percentage>100.0f)
			percentage = 100;
		HWND hwnMsg = GetDlgItem(hWndProgress,IDC_STATIC_TEXT);
		SendMessage(hWndMsg,WM_SETTEXT,0,(LPARAM)szErrMsg);		
		SendMessage(hWndProgress, PBM_SETPOS, (WPARAM) percentage, 0);
	 }
	
    if (!InternetReadFile (hRequest, (LPVOID)lpBufferA, dynamicByte, &dwSize))
    {
	    wsprintf(szErrMsg, TEXT("%s: %x\n"), TEXT("InternetReadFile Error"),GetLastError());
		OutputDebugString(szErrMsg);
	  delete[] lpBufferA;
      goto exit;
    }

    if (dwSize != 0)    
    {	
		downloadBytes+=dwSize;
		//Quick check if HTML error has occured
		if(strncmp("<!DOCTYPE",lpBufferA,9)==0)
				bReturn = 100;
		if(strncmp("<HTM",lpBufferA,4)==0)
				bReturn = 100;
		if(strncmp("<htm",lpBufferA,4)==0)
				bReturn = 100;	

		if(stream!=NULL)
			 fwrite( lpBufferA, sizeof( CHAR ), dwSize, stream );		
    }    
	delete[] lpBufferA; 
	dynamicByte+=1024;
	if(dynamicByte>128000)
		dynamicByte = 128000;
		

  } while (dwSize);


  if(stream!=NULL)
  	  fclose( stream );

  goto exitWithNoErr;
exit:
	bReturn = 1;
exitWithNoErr:
    if (hRequest)
  {
    if (!InternetCloseHandle (hRequest))
	{
		wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), GetLastError());
		OutputDebugString(szErrMsg);

	}
  }

  // Close the Internet handles.
  if (hOpen)
  {
    if (!InternetCloseHandle (hOpen))
	{
		wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), GetLastError());
		OutputDebugString(szErrMsg);

	}
  }
  
  if (hConnect)
  {
    if (!InternetCloseHandle (hConnect))
	{
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), GetLastError());
	  OutputDebugString(szErrMsg);

	}
  }

  return bReturn;
}
/*
void DisplayErrMsg()
{

LPVOID lpMsgBuf;
FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
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
*/
