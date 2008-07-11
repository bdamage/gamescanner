
#include "stdafx.h"
#include "utilz.h"
#include "structs_defines.h"
#include <time.h>
#include <Mswsock.h>

extern APP_SETTINGS_NEW AppCFG;
extern HWND g_hWnd;
extern int g_statusIcon;
extern HWND g_hwndLogger;
extern HINSTANCE g_hInst;
extern GAME_INFO GI[MAX_SERVERLIST+1];

string UTILZ_sLogger;

//Debug OFF!!!
#ifndef _DEBUG

#define dbg_print(exp) ((void)0)

#else

void dbg_print(char *szMsg, ...)
{
	char *szBuffer;
	va_list argList;
	size_t len;
	va_start(argList, szMsg);

	len = _vscprintf( szMsg, argList ) + 1; 
	szBuffer = (char*)malloc( len * sizeof(char));

	vsprintf_s(szBuffer,len,szMsg, argList);

	OutputDebugString(szBuffer);
	OutputDebugString("\n");


	UTILZ_sLogger.append(szBuffer);
	UTILZ_sLogger.append("\r\n");
	
	if(UTILZ_sLogger.length()>100)
		UTILZ_sLogger.erase(UTILZ_sLogger.begin(),UTILZ_sLogger.begin()+strlen(szBuffer));

	
	SetWindowText(g_hwndLogger,UTILZ_sLogger.c_str());
	SendMessage(g_hwndLogger,WM_VSCROLL,LOWORD(SB_BOTTOM),NULL);


	va_end(argList);
	free(szBuffer);	


}

#endif

// DoLockDlgRes - loads and locks a dialog template resource. 
// Returns the address of the locked resource. 
// lpszResName - name of the resource 
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCSTR lpszResName) 
{ 
    HRSRC hrsrc = FindResource(g_hInst, lpszResName, RT_DIALOG); 
    HGLOBAL hglb = LoadResource(g_hInst, hrsrc); 
    return (DLGTEMPLATE *) LockResource(hglb); 
} 


//trans range 0-100
void SetDlgTrans(HWND hwnd,int trans)
{
	if(trans<MIN_TRANSPARANCY)
		trans = AppCFG.g_cTransparancy = MIN_TRANSPARANCY;
	SLWA pSetLayeredWindowAttributes = NULL;  
	HINSTANCE hmodUSER32 = LoadLibrary("USER32.DLL"); 
	pSetLayeredWindowAttributes = (SLWA)GetProcAddress(hmodUSER32,"SetLayeredWindowAttributes");
	SetWindowLong(hwnd, GWL_EXSTYLE,GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	pSetLayeredWindowAttributes(hwnd, 0, (255 * (BYTE)trans) / 100, LWA_ALPHA);
}


BOOL CenterWindow(HWND hwnd)
{
    RECT rect;
    int width, height;      
    int screenX, screenY;
    GetWindowRect(hwnd, &rect);

	screenX  = GetSystemMetrics(SM_CXSCREEN)/2;
    screenY = GetSystemMetrics(SM_CYSCREEN)/2;

    width  = (rect.right  - rect.left);
    height = (rect.bottom - rect.top);
    
    MoveWindow(hwnd, screenX-(width/2), screenY-(height/2), width, height, FALSE);
	return TRUE;
}

void ClickMouse() 
{
 //mouse_event(          MOUSEEVENTF_LEFTDOWN,0,0,0,0);
 //mouse_event(          MOUSEEVENTF_LEFTUP,0,0,0,0);

  INPUT pInputs[1];
  MOUSEINPUT pMouseInput;
  
  pMouseInput.dx = 0;
  pMouseInput.dy = 0;
  pMouseInput.mouseData = XBUTTON1;
  pMouseInput.dwFlags = MOUSEEVENTF_LEFTDOWN;
  pMouseInput.time = 0;
  pInputs[0].type = INPUT_MOUSE;
  pInputs[0].mi = pMouseInput;
  SendInput(1, pInputs, sizeof(INPUT));
  pMouseInput.dwFlags = MOUSEEVENTF_LEFTUP;
  pInputs[0].mi = pMouseInput;
  SendInput(1, pInputs, sizeof(INPUT));

 // return;

}


bool IsInstalled(char *version)
{
	bool installed=false;
	LONG ret;
	char dump[10];
	HKEY hkey;
	DWORD dwDisposition;
	ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE , "SOFTWARE\\GameScanner\\", 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
	DWORD cbData = sizeof(dump);
	memset(dump,0,sizeof(dump));
	ret = RegQueryValueEx( hkey,"version",NULL,NULL,(BYTE*)dump,&cbData);
	
	if(strcmp(dump,version)==0)
		installed = true;
	else
	{
		ret = RegSetValueEx( hkey, "version", 0, REG_SZ, (const unsigned char*)version,(DWORD) strlen(version)); 
	}
	RegCloseKey( hkey ); 
	return installed;
}
void GetInstallPath(char *path)
{
	LONG ret;
	HKEY hkey;
	DWORD dwDisposition;
	DWORD cbData;
	ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE , "SOFTWARE\\GameScanner\\", 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
	cbData=_MAX_PATH;
	ret = RegQueryValueEx( hkey,"installpath",NULL,NULL,(BYTE*)path,&cbData);
	if(ret>0)
		ret = RegSetValueEx( hkey, "installpath", 0, REG_SZ, (const unsigned char*)path, (DWORD)strlen(path)); 

	RegCloseKey( hkey ); 
}
void SetInstallPath(char *path)
{
	LONG ret;
	HKEY hkey;
	DWORD dwDisposition;
	ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE , "SOFTWARE\\GameScanner\\", 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
	ret = RegSetValueEx( hkey, "installpath", 0, REG_SZ, (const unsigned char*)path, (DWORD)strlen(path)); 
	RegCloseKey( hkey ); 
}


void SelfInstall(char *path)
{
	LONG ret;
	HKEY hkey;
	DWORD dwDisposition;
    char totpath[_MAX_PATH+_MAX_FNAME],path2[_MAX_PATH+_MAX_FNAME];

	strcpy_s(path2,sizeof(path2),path);
   //check if it is in root only.. C:\?
   if(strlen(path2)>3)	
	   strcat_s(path2,sizeof(path2),"\\GameScanner.exe");
   else
	   strcat_s(path2,sizeof(path2),"GameScanner.exe");

    sprintf_s(totpath,515,"\"%s\" /tasktray",path2);
	strcpy_s(path2,sizeof(path2),totpath);

	ret = RegCreateKeyEx(HKEY_CURRENT_USER , "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
	ret = RegSetValueEx( hkey, "ETServerViewer", 0, REG_SZ, (const unsigned char*)path2, strlen(path2)); 
	RegCloseKey( hkey ); 

}
void UnInstall()
{
	HKEY hkey;
	LONG ret;
	DWORD dwDisposition;
	ret = RegCreateKeyEx(HKEY_CURRENT_USER , "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
	ret = RegDeleteValue(hkey,"ETServerViewer");
	RegCloseKey( hkey ); 
}
void dbg_dumpbuf(const char *file, const void *buf, size_t size) {
	FILE *fp=fopen(file, "wb");
	fwrite(buf, size, 1, fp);
	fclose(fp);
}
void dbg_readbuf(const char *file, char *buf, size_t size) {
	FILE *fp=fopen(file, "rb");
	fread(buf, size, 1, fp);
	fclose(fp);
}
//#define NOLOG

DEVMODE GetScreenResolution(VOID) 
{
	DEVMODE mySettings;
  	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mySettings);
	AddLogInfo(ETSV_INFO,"Screen resolution detected: %d x %d @ %d",mySettings.dmPelsWidth,mySettings.dmPelsHeight,mySettings.dmBitsPerPel );

  return mySettings;
}

BOOL SetScreenResolution(DEVMODE DisplaySettings) {
  int ret;
  
  ret = ChangeDisplaySettings(&DisplaySettings, CDS_TEST);
  if(ret != DISP_CHANGE_SUCCESSFUL) return 0;
	ChangeDisplaySettings(&DisplaySettings, CDS_UPDATEREGISTRY);
  return 1;
}

HFONT  SetFont(HWND hDlg, int nIDDlgItem)
{
	HFONT hf;
	LOGFONT lf;
	memset(&lf,0,sizeof(LOGFONT));
	strcpy_s(lf.lfFaceName,sizeof(lf.lfFaceName),"Arial"); //Courier New, Terminal Tahoma
	
	lf.lfHeight = 12;

	lf.lfWeight = FW_NORMAL; //FW_EXTRABOLD; //FW_NORMAL, FW_EXTRABOLD etc. se LOGFONT struct i msdn

	hf = CreateFontIndirect(&lf);
	HWND target;
	target = GetDlgItem(hDlg,nIDDlgItem);
	if(target!=NULL)
	     SendMessage(target, WM_SETFONT, (WPARAM)hf,(LPARAM) MAKELONG((WORD) TRUE, 0));
	return hf;
}

void SetFontToDlgItem(HWND hDlg,HFONT hf,int nIDDlgItem)
{	
	HWND target;
	target = GetDlgItem(hDlg,nIDDlgItem);
	if(target!=NULL)
	     SendMessage(target, WM_SETFONT, (WPARAM)hf,(LPARAM) MAKELONG((WORD) TRUE, 0));	
}

char szLogPath[MAX_PATH];

void SetLogPath(const char *szPath)
{
	strcpy_s(szLogPath,sizeof(szLogPath),szPath);
}

void AddGetLastErrorIntoLog(char* lpszFunction)
{
	
			LPVOID lpMsgBuf;
		//	LPVOID lpDisplayBuf;
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

			// Display the error message and exit the process

		//	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		//		(lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(char)); 
			
		//	StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpszFunction),"%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf); 
			
			AddLogInfo(1,"GetLastError from func %s Info: %s",lpszFunction,lpMsgBuf);

			LocalFree(lpMsgBuf);
		//	LocalFree(lpDisplayBuf);
			
	
}

void AddLogInfo(int color, char *lpszText, ...)
{

#ifndef NOLOG
	//if(appSettings.bLogging)
	{
		char szColor[10];
		strcpy_s(szColor,sizeof(szColor),"000000");	  //Default

		if(color==ETSV_WARNING)
			strcpy_s(szColor,sizeof(szColor),"99dd00");
		else if(color==ETSV_DEBUG)
			strcpy_s(szColor,sizeof(szColor),"0000FF");
		else if(color==ETSV_ERROR)
			strcpy_s(szColor,sizeof(szColor),"FF0000");

		

		char time[128], date[128];
		// Set time zone from TZ environment variable. If TZ is not set,
		// the operating system is queried to obtain the default value 
		// for the variable. 
		//
		_tzset();

		// Display operating system-style date and time. 
		_strtime_s( time, 128 );
	//  printf( "OS time:\t\t\t\t%s\n", tmpbuf );
		_strdate_s( date, 128 );
		//printf( "OS date:\t\t\t\t%s\n", tmpbuf );


			
			va_list argList;
			FILE *pFile = NULL;
	


			//Initialize variable argument list
			va_start(argList, lpszText);

			if(szLogPath!=NULL)
				SetCurrentDirectory(szLogPath);
			//Open the log file for appending
			pFile = fopen("Log.htm", "a+");

			if(pFile != NULL)
			{
				//Write the error to the log file
				fprintf(pFile, "<font face=\"Arial\" size=\"2\" color=\"#%s\"><b>",szColor);
				fprintf(pFile, "[%s][%s]",date,time);
				
				
			//	fprintf(pFile, lpszText);
				vfprintf(pFile, lpszText, argList);
				fprintf(pFile, "</b></font><br>\n");

				//Close the file
				fclose(pFile);
				char szBuffer[512];
				vsprintf_s(szBuffer,sizeof(szBuffer),lpszText, argList);
			
			//	GetWindowTextLength(g_hwndLogger);

				UTILZ_sLogger.append(szBuffer);
				UTILZ_sLogger.append("\r\n");

				if(UTILZ_sLogger.length()>1000)
				{
					
					UTILZ_sLogger.erase(UTILZ_sLogger.begin(),UTILZ_sLogger.begin()+UTILZ_sLogger.find_first_of("\n")); //strlen(szBuffer));
				}

	
				SetWindowText(g_hwndLogger,UTILZ_sLogger.c_str());
				SendMessage(g_hwndLogger,WM_VSCROLL,LOWORD(SB_BOTTOM),NULL);
#ifdef _DEBUG
				OutputDebugString(szBuffer);
				OutputDebugString("\n");
#endif
					 
			}
//#if defined(_DEBUG) && defined(WIN32)
		//	fprintf(stderr, "%s\n", lpszText);
			
//#endif
			va_end(argList);
	}
	//ReleaseMutex(g_hMutex);
#endif

}
char *colorfilter(const char* name,char *namefilter,int len)
{
	size_t i=0,ii=0;
	if(name!=NULL)
	{
		memset(namefilter,0,len);
		while(i<strlen(name))
		{
			if(name[i]=='^')
			{

				if(name[i+1]!='^')  //for those who are using double ^^
					i+=2;
				else
				{
					namefilter[ii]=name[i];
					i++;
					ii++;
				}
			}
			else
			{
				namefilter[ii]=name[i];
				i++;
				ii++;
			}	
		}
	} else
		return "...";
	return namefilter;
}

BOOL isNumeric(char c)
{
	if((c>='0') && (c<='9'))
		return TRUE;

	return FALSE;
}


char * DWORD_IP_to_szIP(DWORD dwIP)
{
	in_addr inAddr;
	ZeroMemory(&inAddr,sizeof(in_addr));
	inAddr.S_un.S_addr = htonl(dwIP);
	char *szIP = inet_ntoa(inAddr);
	if(szIP!=NULL)
		return szIP;
	return NULL; 
}


//Convert networkname (www.bdamage.se) to an IP address (ie. 10.10.0.1)
DWORD NetworkNameToIP(char *host_name,char *port)
{
	struct addrinfo aiHints;
	struct addrinfo *aiList = NULL;
	int retVal;

	memset(&aiHints, 0, sizeof(aiHints));
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = IPPROTO_TCP;
	
	DWORD test,test2;

	//hostent* remoteHost;
	unsigned int addr;

	if (isalpha(host_name[0])) 
	{   /* host address is a name */
		
		
			//--------------------------------
			// Call getaddrinfo(). If the call succeeds,
			// the aiList variable will hold a linked list
			// of addrinfo structures containing response
			// information about the host
			if ((retVal = getaddrinfo(host_name, port, &aiHints, &aiList)) != 0) 
			{
				AddLogInfo(ETSV_WARNING,"getaddrinfo() failed.\n");
				return 0;
			}

		test = *(DWORD*)&aiList->ai_addr->sa_data[2];  //Ip
		test2 = ntohl(test);
		addr = test2;
		freeaddrinfo(aiList);
			
		// host_name[strlen(host_name)-1] = '\0'; /* NULL TERMINATED */
		//remoteHost = gethostbyname(host_name);
	}
	else  
	{ 
		addr = inet_addr(host_name);
		if(addr== INADDR_NONE)
		{
			AddLogInfo(ETSV_WARNING,"getaddrinfo() failed at %d.\n",__LINE__);
			return 0;
		}
		test2 = ntohl(addr);
		addr = test2;
		// remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);
	}

	return addr;
}



//Check if the server exsist
bool UTILZ_CheckForDuplicateServer(GAME_INFO *pGI, SERVER_INFO pSI)
{
	vSRV_INF::iterator  iResult;	
	iResult = find(pGI->pSC->vSI.begin(), pGI->pSC->vSI.end(),pSI);
	
	if(iResult == pGI->pSC->vSI.end())
		 return false;

	 return true;
}

void SetStatusText( int icon, char *szMsg,...)
{	
	char *szBuffer;
	va_list argList;
	size_t len;
	if(g_hWnd==NULL)
		return;

	//Initialize variable argument list
	va_start(argList, szMsg);

	len = _vscprintf( szMsg, argList ) + 1; 
	szBuffer = (char*)malloc( len * sizeof(char));

	vsprintf_s(szBuffer,len,szMsg, argList);
	
	SetDlgItemText(g_hWnd,IDC_EDIT_STATUS,szBuffer);

	//dbg_print(szBuffer);

	va_end(argList);
	free(szBuffer);	
	g_statusIcon=icon;
}

//linked list clean up
void UTILZ_CleanUp_ServerRules(LPSERVER_RULES &pSR)
{
	//dbg_print("Enter Q3_CleanUp_ServerRules(...)\n");
	if(pSR!=NULL)
	{
		__try
		{
			if(pSR->pNext!=NULL)
				UTILZ_CleanUp_ServerRules(pSR->pNext);

			free(pSR->name);
			free(pSR->value);
			pSR->pNext = NULL;
			free(pSR);
			pSR = NULL;

		} 	
		__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
		{
			// exception handling code
			AddLogInfo(ETSV_ERROR,"Access Violation!!! @ Q3_CleanUp_ServerList(...)\n");
			DebugBreak();
		}
		
	}
}
void UTILZ_CleanUp_PlayerList(LPPLAYERDATA &pPL)
{
//	dbg_print("Enter Q3_CleanUp_PlayerList(...)\n");
	if(pPL!=NULL)
	{
		
		if(pPL->pNext!=NULL)
			UTILZ_CleanUp_PlayerList(pPL->pNext);
		
		pPL->pNext = NULL;
		if(pPL->szPlayerName!=NULL)
		{
		//	dbg_print("Removing: ");
		//	dbg_print(pPL->szPlayerName);
		//	dbg_print("\n");
			free(pPL->szPlayerName);
			pPL->szPlayerName = NULL;
		}
		if(pPL->szClanTag!=NULL)
		{
			free(pPL->szClanTag);
			pPL->szClanTag = NULL;
		}
		
		free(pPL);
		pPL = NULL;
	}
}


char *colorfilterQ4(char* name,char *namefilter, size_t len)
{
	size_t i=0,ii=0;
	if(name!=NULL)
	{
		memset(namefilter,0,len);
		while((i<strlen(name)) && (i<len))
		{
			if(name[i]=='^')
			{
				if((name[i+1]=='i') && (name[i+2]=='d') && (isNumeric(name[i+4]))) //Q4 ^idm0 and ^idm1 icons
					i+=5;
				if((name[i+1]=='c') && (isNumeric(name[i+2])) && (isNumeric(name[i+3])) ) //Q4 color encoded RGB ^c000 ^c2456 etc...				
					i+=5;				
				else if(name[i+1]!='^')  //for those who are using double ^^
					i+=2;
				else
				{
					namefilter[ii]=name[i];
					i++;
					ii++;
				}
			}
			else
			{
				namefilter[ii]=name[i];
				i++;
				ii++;
			}	
		}
	} else
		return "...";
	return namefilter;
}



char *getpacket(SOCKET s, size_t *len) {
	fd_set set;

	FD_ZERO(&set);
	FD_SET(s, &set);
	
	struct timeval socktimeout;
	socktimeout.tv_sec = AppCFG.socktimeout.tv_sec; //3;
	socktimeout.tv_usec  =  AppCFG.socktimeout.tv_usec;

	if(select(FD_SETSIZE, &set, NULL, NULL, &socktimeout)>0) 
	{
		
		char *buf=(char*)calloc(18048, sizeof(char));	
		if(buf==NULL)
			dbg_print("Error allocating memory!\n");
	
		int ret=recv(s, buf, 18048, 0);

	//	if(memcmp("\xFF\xFF\xFF\xFE",buf,4)==0)
	//	{
	//		DebugBreak();
	//	}

		if(ret!=SOCKET_ERROR && ret!=0) {
			*len=ret;
			
			//dbg_print("Bytes recieved %d\n",ret);
			
			return (char*)realloc(buf, ret+1);
		}
		else {
			dbg_print("getpacket err: %d\n", WSAGetLastError());
			free(buf);
			return NULL;
		}
	}
	else 	
	{
		//dbg_print("Socket timed out.");

	}
	return NULL;
}


char *	ReadPacket(SOCKET socket, size_t *len) 
{
	char *buf=(char*)calloc(10048, sizeof(char));	
	if(buf==NULL)
	{
		dbg_print("Error allocating memory!\n");
		DebugBreak();
	}	
	int ret=recv(socket, buf, 10048, 0);

	//	if(memcmp("\xFF\xFF\xFF\xFE",buf,4)==0)
	//	{
	//		DebugBreak();
	//	}

	if(ret!=SOCKET_ERROR && ret!=0) 
	{
		*len=ret;
	
	//	dbg_print("Bytes recieved %d\n",ret);	
		return (char*)buf;
	}
	else 
	{
		dbg_print("getpacket err: %d\n", WSAGetLastError());
		free(buf);		
	}
	return NULL;
}

//g_gametype ETMain Server Sets the type of game being played, 
//2=objective, 3=stopwatch, 4=campaign, 5=LMS 


/*
0. Free For All
1. Tournament 1 on 1
2. Single Player
3. Team Deathmatch
4. Capture the Flag
5. One Flag CTF
6. Overload
7. Harvester 

 0 - Free For All 1 - Tournament 2 - Single Player 
3 - Team Deathmatch 4 - Capture the Flag  
to start a dedicated server in tournament mode, you would use: quake3.exe +set dedicated 2 +set sv_zone tournaments +set g_gametype 1 +map q3tourney2, "Graeme Devine" thanks also to TheKiller 5 - One Flag CTF 6 - Overload 7 - Harvester (Team Arena only) 
 
 
*/
char szGAMETYPEUNKOWN[]={"Unknown"};
struct _GAMETYPENAME
{ 
	char gametype;
	char szName[20];
	WORD cETSVGAMETYPE;
};

_GAMETYPENAME GTWARSOW[16] = {
	0,"Unknown",GAMETYPE_FFA,
	1,"ctf",GAMETYPE_CTF,
	2,"ca",GAMETYPE_CA,
	3,"tdm",GAMETYPE_TDM,
	4,"dm",GAMETYPE_DM,
	5,"duel",GAMETYPE_DUEL,
	6,"race",GAMETYPE_DM,
	7,"Unknown",GAMETYPE_UNKNOWN,
	8,"Unknown",GAMETYPE_UNKNOWN,
	9,"Unknown",GAMETYPE_UNKNOWN,
   10,"Unknown",GAMETYPE_UNKNOWN,
   11,"Unknown",GAMETYPE_UNKNOWN,
   12,"Unknown",GAMETYPE_UNKNOWN,
   13,"Unknown",GAMETYPE_UNKNOWN,
   14,"Unknown",GAMETYPE_UNKNOWN,
};

//This applies to both CoD 2 & CoD 4
_GAMETYPENAME GTCOD2[16] = {
	0,"Unknown",GAMETYPE_FFA,
	1,"ctf",GAMETYPE_CTF,
	2,"ca",GAMETYPE_CA,
	3,"tdm",GAMETYPE_TDM,
	4,"dm",GAMETYPE_DM,
	5,"duel",GAMETYPE_DUEL,
	6,"race",GAMETYPE_DM,
	7,"sd",GAMETYPE_SD, //Search & Destroy
	8,"zom",GAMETYPE_ZOM,
	9,"hq",GAMETYPE_HQ,
   10,"Unknown",GAMETYPE_UNKNOWN,
   11,"Unknown",GAMETYPE_UNKNOWN,
   12,"Unknown",GAMETYPE_UNKNOWN,
   13,"Unknown",GAMETYPE_UNKNOWN,
   14,"Unknown",GAMETYPE_UNKNOWN,
};



DWORD Get_GameTypeByName(int gameIdx, char *szGameType)
{
	if(szGameType==NULL)
		return GAMETYPE_UNKNOWN;

	for(unsigned int i=0;i<GI[gameIdx].pSC->vFilterGameType.size();i++)
	{
		GAMEFILTER gf = GI[gameIdx].pSC->vFilterGameType.at(i);
		if(strstr(szGameType,gf.sStrValue.c_str())!=NULL)
			return gf.dwValue;		
	}
	return GAMETYPE_UNKNOWN;

}

DWORD Get_ModByName(int gameIdx, char *szModName)
{
	if(szModName==NULL)
		return GAMETYPE_UNKNOWN;

	for(unsigned int i=0;i<GI[gameIdx].pSC->vFilterMod.size();i++)
	{
		GAMEFILTER gf = GI[gameIdx].pSC->vFilterMod.at(i);
		if(strstr(szModName,gf.sStrValue.c_str())!=NULL)
			return gf.dwValue;		
	}
	return GAMETYPE_UNKNOWN;
}
DWORD Get_MapByName(int gameIdx, char *szMapName)
{
	if(szMapName==NULL)
		return GAMETYPE_UNKNOWN;

	for(unsigned int i=0;i<GI[gameIdx].pSC->vFilterMap.size();i++)
	{
		GAMEFILTER gf = GI[gameIdx].pSC->vFilterMap.at(i);
		if(strstr(szMapName,gf.sStrValue.c_str())!=NULL)
			return gf.dwValue;		
	}
	return GAMETYPE_UNKNOWN;
}

DWORD Get_FilterVersionByVersionString(int gameIdx, char *szVersion)
{
	if(szVersion==NULL)
		return VERSION_UNKNOWN;

	for(unsigned int i=0;i<GI[gameIdx].pSC->vFilterVersion.size();i++)
	{
		GAMEFILTER gf = GI[gameIdx].pSC->vFilterVersion.at(i);

		if(strstr(szVersion,gf.sStrValue.c_str())!=NULL)
			return gf.dwValue;
	}

	return VERSION_UNKNOWN;
}

//This funciton is for visual, when the listview updates
//A better readable name is returned
const char * Get_GameTypeNameByGameType(int gameIdx, WORD cGameType)
{
	for(unsigned int i=0;i<GI[gameIdx].pSC->vFilterGameType.size();i++)
	{
		GAMEFILTER gf = GI[gameIdx].pSC->vFilterGameType.at(i);
		
		if(cGameType == gf.dwValue)
			return GI[gameIdx].pSC->vFilterGameType.at(i).sFriendlyName.c_str(); //gf.sFriendlyName.c_str();		
	}
	return szGAMETYPEUNKOWN;
}


#define MS_VC_EXCEPTION 0x406D1388

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName( DWORD dwThreadID, char* threadName)
{
   Sleep(10);
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}

void Show_ErrorDetails(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
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

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

}

