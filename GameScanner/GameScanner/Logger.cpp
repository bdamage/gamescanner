#include "StdAfx.h"
#include "Logger.h"
#include <string>

CLogger::CLogger(void)
{
	remove("Log_previous_start.htm");
	rename("g_log.htm","Log_previous_start.htm");
	hwndLogger = NULL;
	strcpy(szLogPath,".\\");
	InitializeCriticalSection(&CS_Logger);
}

CLogger::~CLogger(void)
{
	DeleteCriticalSection(&CS_Logger);
}




void CLogger::SetLogPath(const char *szPath)
{
	strcpy_s(szLogPath,sizeof(szLogPath),szPath);
}


/*
CAUTION: Try to avoid use AddLogInfo from the main application thread. Otherwise deadlock can occur!!
TODO : make this function into a new thread on every call...
*/
void CLogger::AddLogInfo(int color, char *lpszText, ...)
{

		char szColor[10];
		strcpy_s(szColor,sizeof(szColor),"000000");	  //Default

		if(color==GS_LOG_WARNING)
			strcpy_s(szColor,sizeof(szColor),"99dd00");
		else if(color==GS_LOG_DEBUG)
			strcpy_s(szColor,sizeof(szColor),"0000FF");
		else if(color==GS_LOG_ERROR)
			strcpy_s(szColor,sizeof(szColor),"FF0000");

		char time[64], date[64];
		// Set time zone from TZ environment variable. If TZ is not set,
		// the operating system is queried to obtain the default value 
		// for the variable. 
		//
		_tzset();

		// Display operating system-style date and time. 
		_strtime_s( time, 64 );
		_strdate_s( date, 64 );
		
		if(TryEnterCriticalSection(&CS_Logger)==FALSE)
			return;


			va_list argList;
			FILE *pFile = NULL;
	

			//Initialize variable argument list
			va_start(argList, lpszText);

			if(szLogPath!=NULL)
				SetCurrentDirectory(szLogPath);
			//Open the g_log.file for appending
			pFile = fopen("g_log.htm", "a+");

			if(pFile != NULL)
			{
				//Write the error to the g_log.file
				fprintf(pFile, "<font face=\"Arial\" size=\"2\" color=\"#%s\"><b>",szColor);
				fprintf(pFile, "[%s][%s]",date,time);
				
				vfprintf(pFile, lpszText, argList);
				fprintf(pFile, "</b></font><br>\n");

				//Close the file
				fclose(pFile);
				char szBuffer[512];
				vsprintf_s(szBuffer,sizeof(szBuffer),lpszText, argList);
				

				UTILZ_sLogger.append(szBuffer);
				UTILZ_sLogger.append("\r\n");

				if(UTILZ_sLogger.length()>1000)
					UTILZ_sLogger.erase(UTILZ_sLogger.begin(),UTILZ_sLogger.begin()+UTILZ_sLogger.find_first_of("\n")); //strlen(szBuffer));
				
				SetWindowText(hwndLogger,UTILZ_sLogger.c_str());
			

				SendMessage(hwndLogger,WM_VSCROLL,LOWORD(SB_BOTTOM),NULL);
#ifdef _DEBUG
				OutputDebugString(szBuffer);
				OutputDebugString("\n");
#endif
					 
			}
			va_end(argList);

	LeaveCriticalSection(&CS_Logger);

}

void CLogger::AddGetLastErrorIntoLog(char* lpszFunction)
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	AddLogInfo(1,"GetLastError from func %s Info: %s",lpszFunction,lpMsgBuf);

	LocalFree(lpMsgBuf);	
}