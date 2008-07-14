// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the HTTPFILEDOWNLOAD_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// HTTPFILEDOWNLOAD_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef HTTPFILEDOWNLOAD_EXPORTS
#define HTTPFILEDOWNLOAD_API __declspec(dllexport)
#else
#define HTTPFILEDOWNLOAD_API __declspec(dllimport)
#endif




HTTPFILEDOWNLOAD_API int  HttpFileDownload(char* lpszServer,char *lpszDest,HWND hWndProgress=NULL,HWND hWndMsg = NULL);
HTTPFILEDOWNLOAD_API bool HFD_SetPath(char *szPath);