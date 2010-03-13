
#include "stdafx.h"
#include "utilz.h"
#include "structs_defines.h"
#include <time.h>
#include <Mswsock.h>
#include "logger.h"

extern APP_SETTINGS_NEW AppCFG;
extern HWND g_hWnd;
extern int g_statusIcon;
extern HWND g_hwndLogger;
extern HINSTANCE g_hInst;
extern GamesMap GamesInfo;
extern RECT g_INFOIconRect;
extern _WINDOW_CONTAINER WNDCONT[15];

extern char NexuizASCII[];
extern char QuakeWorldASCII[];

extern CLogger g_log;



//Debug OFF!!!
#ifndef _DEBUG

#else

void dbg_print(char *szMsg, ...)
{	
	char *szBuffer=NULL;
	va_list argList;
	size_t len;
	va_start(argList, szMsg);

	len = _vscprintf( szMsg, argList ) + 1; 
	szBuffer = (char*)malloc( len * sizeof(char));

	vsprintf_s(szBuffer,len,szMsg, argList);

	OutputDebugString(szBuffer);
	OutputDebugString("\n");

	va_end(argList);
	free(szBuffer);	
}


void dbg_printNoLF(char *szMsg, ...)
{	
	char *szBuffer=NULL;
	va_list argList;
	size_t len;
	va_start(argList, szMsg);

	len = _vscprintf( szMsg, argList ) + 1; 
	szBuffer = (char*)malloc( len * sizeof(char));

	vsprintf_s(szBuffer,len,szMsg, argList);

	OutputDebugString(szBuffer);

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

void GetServerLock(SERVER_INFO *pSrv)
{

	while(pSrv!=NULL)
	{
		if(TryEnterCriticalSection(&pSrv->csLock)==FALSE)
		{
			dbg_print("csLock: Waiting for server to be released %s.\n",pSrv->szIPaddress); 
			Sleep(100);
			
		}else
			break;
	}
}
void ReleaseServerLock(SERVER_INFO *pSrv)
{

	if(pSrv!=NULL)
		LeaveCriticalSection(&pSrv->csLock);
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

char *UTF8toMB(const char* inUtf8, char* outStr,int maxLen)
{
	if(inUtf8==NULL)
		return NULL;
	
	int len = strlen((char*)inUtf8);
	
	if(len>=maxLen)
		len = maxLen-1;

	WCHAR* pwcbuff = (WCHAR*)calloc(len+1,sizeof(WCHAR));

    MultiByteToWideChar(CP_UTF8,0, inUtf8, len,  pwcbuff,  len*2);
	WideCharToMultiByte(CP_ACP,0, pwcbuff, len, outStr, len, NULL, NULL);
	free(pwcbuff);
	return outStr;
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

//this is used by the minimzer code
void ClickMouse() 
{
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
}


void AddAutoRun(char *path)
{
	LONG ret;
	HKEY hkey;
	DWORD dwDisposition;
	if(path==NULL)
		return;
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
	ret = RegSetValueEx( hkey, "GameScanner", 0, REG_SZ, (const unsigned char*)path2, strlen(path2)); 
	RegCloseKey( hkey ); 

}

void RemoveAutoRun()
{
	HKEY hkey;
	LONG ret;
	DWORD dwDisposition;
	ret = RegCreateKeyEx(HKEY_CURRENT_USER , "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition ); 
	ret = RegDeleteValue(hkey,"GameScanner");
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
	g_log.AddLogInfo(GS_LOG_INFO,"Screen resolution detected: %d x %d @ %d",mySettings.dmPelsWidth,mySettings.dmPelsHeight,mySettings.dmBitsPerPel );

  return mySettings;
}

BOOL SetScreenResolution(DEVMODE DisplaySettings) {
  int ret;
  
  ret = ChangeDisplaySettings(&DisplaySettings, CDS_TEST);
  if(ret != DISP_CHANGE_SUCCESSFUL) return 0;
	ChangeDisplaySettings(&DisplaySettings, CDS_UPDATEREGISTRY);
  return 1;
}

//FW_EXTRABOLD;
//FW_NORMAL, FW_EXTRABOLD etc. se LOGFONT struct i msdn
//Courier New, Terminal Tahoma
HFONT  MyCreateFont(HWND hDlg,LONG height, LONG weight, char *pszFontFace)
{
	HFONT hf;
	LOGFONT lf;

	memset(&lf,0,sizeof(LOGFONT));
	strcpy_s(lf.lfFaceName,sizeof(lf.lfFaceName),pszFontFace); 
	lf.lfHeight = height;
	lf.lfWeight = weight; 
	hf = CreateFontIndirect(&lf);

	return hf;
}


void SetFontToWindowHandler(HWND hTargetWnd,HFONT hf)
{	
	if(hTargetWnd!=NULL)
	     SendMessage(hTargetWnd, WM_SETFONT, (WPARAM)hf,(LPARAM) MAKELONG((WORD) TRUE, 0));	
}

void SetFontToDlgItem(HWND hDlg,HFONT hf,int nIDDlgItem)
{	
	HWND target;
	target = GetDlgItem(hDlg,nIDDlgItem);
	if(target!=NULL)
	     SendMessage(target, WM_SETFONT, (WPARAM)hf,(LPARAM) MAKELONG((WORD) TRUE, 0));	
}







BOOL isNumeric(char c)
{
	if((c>='0') && (c<='9'))
		return TRUE;
	return FALSE;
}

char *colorfilter(const char* name,char *namefilter,int len)
{
	int n=0;
	memset(namefilter,0,len);
	if(name!=NULL)
	{
		
		for(int i=0;i<strlen(name);i++)
		{
			if(i>=len)
				break;
			if(name[i]=='^')
			{
				i++;
				if(name[i]!='^') // this fixes these kind of names with double ^^
					continue;
			}
			
			namefilter[n] = name[i];		
			n++;
		}
	} 	
	return namefilter;
}

//Jedi Knight 3 filter 
char *colorfilterJK3(const char* name,char *namefilter,int len)
{
	int n=0;
	memset(namefilter,0,len);
	if(name!=NULL)
	{
		
		for(int i=0;i<strlen(name);i++)
		{
			if(i>=len)
				break;
			if(name[i]==-128)
			{
				i++;
				continue;
			}
			if(name[i]=='^')
			{
				i++;
				if(name[i]!='^') // this fixes these kind of names with double ^^
					continue;
			}
			
			namefilter[n] = name[i];		
			n++;
		}
	} 	
	return namefilter;
}

char *colorfilterNEXUIZ(const char* name,char *namefilter,int len)
{
	int n=0;
	memset(namefilter,0,len);
	if(name!=NULL)
	{
	
		for(int i=0;i<strlen(name);i++)
		{		
			if(i>=len)
				break;
			if(name[i]=='^')
			{
				if(name[i+1]=='x')//^xFFF or 
				{
					i+=4;
					continue;
				}
				i++;
				if(name[i]!='^') // this fixes these kind of names with double ^^
					continue;
			}			
			namefilter[n] = NexuizASCII[(unsigned char)name[i]];		
			n++;
		}
	}
	return namefilter;
}
/*
http://www.truecarnage.com/quake-4-color-code-chart-r24.htm
http://www.truecarnage.com/coloring-team-chat-and-name-tricks-in-q4-r2.htm
^iw00: Gauntlet
^iw01: Machine Gun
^iw02: Shotgun
^iw03: Hyperblaster
^iw04: Grenade Launcher
^iw05: Nailgun
^iw06: Rocket Launcher
^iw07: Railgun
^iw08: Lightning Gun
^iw09: Dark Matter Gun

^ifls: Strogg Flag
^iflm: Marine Flag

^irgn: Regeneration
^idbl: Doubler
^igrd: Guard
^isct: Scout

^ivce: Voice Enabled
^ivcd: Voice Disabled
^ifdd: Player Muted
^ifde: Player Unmuted
^ipbe: Punkbuster g_log.
^idse: Armor Shard
^ipse: Padlock
^ifve: Star
^idm0: Skull 'n' Crossbones
^idm1: Green X
^iarr: Right Arrow

*/
char *colorfilterQ4(const char* name,char *namefilter, int len)
{
	size_t i=0,ii=0;
	memset(namefilter,0,len);
	if(name!=NULL)
	{
		
		while((i<strlen(name)) && (i<len))
		{
			if(name[i]=='^')
			{
				if((name[i+1]=='i')) //Q4 ^idm0 and ^idm1 icons
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

char * colorfilterQW(const char *szInText,char *namefilter, int len)
{
	int i=0;
	memset(namefilter,0,len);
	if(szInText==NULL)
		return NULL;
	while(szInText[i]!=0)
	{
		if(i>=len)
			break;
		namefilter[i] = (unsigned char)QuakeWorldASCII[szInText[i]];
		i++;
	}
	return namefilter;
}

//This does only convert UTF-8 to ANSI
char * colorfilterUTF8(const char *szInText,char *namefilter, int len)
{
	int i=0;
	memset(namefilter,0,len);	
	UTF8toMB(szInText,namefilter,len);
	return namefilter;
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
				g_log.AddLogInfo(GS_LOG_WARNING,"getaddrinfo() failed. IP: %s\n",host_name);
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
			g_log.AddLogInfo(GS_LOG_WARNING,"getaddrinfo() failed at %d.\n",__LINE__);
			return 0;
		}
		test2 = ntohl(addr);
		addr = test2;
		// remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);
	}

	return addr;
}

//split/parse ip & port string indata=1.1.1.1:27960
char *SplitIPandPORT(char *szIPport,DWORD &port)
{
	port = 0;
	if(strlen(szIPport)>0)
	{
		char *p=NULL,*r=NULL;
	
		p = strchr(szIPport,':');

		char *space = strchr(szIPport,' '); //Added since v1.26
		if(space!=NULL)//Added since v1.26
			space[0]=0;

		//Added since <1.0
		r = strrchr(szIPport,'/');  //reverse find
		if(r!=NULL)
			r[0]=0;

		if(p!=NULL)
		{
			p[0]=0;
			p++;
			port = atoi(p);		
		}
		return szIPport;
	}
	return NULL;
}

//custom str compare
int CustomStrCmp(char *a, char *b)
{
	int i=0,i2=0;
	char upperA;
	char upperB;

	if(a==NULL) 
		return 0;
	if(b==NULL)
		return 1;

	while(a[i]==32) //trim spaces
		i++;

	while(b[i2]==32)  //trim spaces
		i2++;


	while(a[i]!=0)
	{
		if(a[i]==0 && b[i2]==0)
			return 0;
		else if(a[i]==0)
			return -1;
		else if(b[i2]==0)
			return 1;


		upperA = a[i];

		if(__isascii(a[i]))
			if(islower(a[i]))
				upperA =_toupper(a[i]);
		
		upperB = b[i2];

		if(__isascii(b[i2]))
			if(islower(b[i2]))
				upperB = _toupper(b[i2]);

		if(upperA> upperB)
			return 1;
		else if(upperA < upperB)
			return -1;
		
		i++;
		i2++;
	}

	return 0;
}

char * ServerRule_Add(SERVER_RULES* &pLinkedListStart,char *szRuleName,char*szRuleValue)
{
	SERVER_RULES *pSR = pLinkedListStart;
	if(pSR!=NULL)
		while(pSR->pNext!=NULL)
			pSR = pSR->pNext;

	if(pLinkedListStart==NULL)
	{
		pSR = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));
		if(pSR==NULL) //out of memory
			return NULL;
		pLinkedListStart = pSR;
	}else
	{
		pSR->pNext = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));
		pSR = pSR->pNext;
		if(pSR==NULL) //out of memory
			return NULL;
	}

	pSR->name = _strdup(szRuleName);
	pSR->value = _strdup(szRuleValue);
	return pSR->value;
}



BOOL UTILZ_checkforduplicates(GAME_INFO *pGI, int hash,DWORD dwIP, DWORD dwPort)
{
	hash_multimap <int, int>::iterator hmp_Iter;
	hmp_Iter = pGI->shash.find(hash);
	while(hmp_Iter!= pGI->shash.end())
	{
		Int_Pair idx = *hmp_Iter;		
		SERVER_INFO  *pSI = (SERVER_INFO*) pGI->vSI.at(idx.second);
		if((dwIP == pSI->dwIP) && (dwPort == pSI->usQueryPort))
			return TRUE;
		hmp_Iter++;
	}
	return FALSE;
}


void SetStatusText( int icon,const char *szMsg,...)
{	
	char *szBuffer;
	va_list argList;
	size_t len;
	if(g_hWnd==NULL)
		return;

	if(icon==ICO_WARNING)
		PlaySound("error.wav", 0,SND_ASYNC | SND_FILENAME);
//	if(icon==ICO_INFO)
//		PlaySound("notification.wav", 0,SND_ASYNC | SND_FILENAME);

	//Initialize variable argument list
	va_start(argList, szMsg);
	len = _vscprintf( szMsg, argList ) + 1; 
	szBuffer = (char*)malloc( len * sizeof(char));

	vsprintf_s(szBuffer,len,szMsg, argList);
	
	SetDlgItemText(g_hWnd,IDC_EDIT_STATUS,szBuffer);
	va_end(argList);
	free(szBuffer);	
	if(icon!=g_statusIcon)
	{
		g_statusIcon=icon;
		RECT rc;
		GetClientRect(g_hWnd,&rc);
		rc.top = WNDCONT[WIN_STATUS].rSize.top; 
		rc.right = 25;
		InvalidateRect(g_hWnd,&rc,TRUE);
	}
}

int UTILZ_ConvertEscapeCodes(char*pszInput,char*pszOutput,DWORD dwMaxBuffer)
{
	int i=0;
	int len=0;
	char CR = 13;
	char backslash='\\';
	while(pszInput[i]!=0)
	{
		char *c = &pszInput[i];
		char val;
		if(c[0]=='\\')
		{
			if((c[1]=='x') )
			{
				char hex[4];
				hex[0] = c[2];
				hex[1] = c[3];
				hex[2] = 0;
				char *End = &hex[2];
				val = (char)strtol(hex,&End,16);
				c = &val;			
				i+=3;
			}
			if((c[1]=='n'))
			{
				i++;
				c = &CR;  //carriege return
			}

			else if((c[1]=='\\'))
			{
				i++;
				c = &backslash;
			}
		}

		pszOutput[len] = c[0];
		i++;
		len++;
		if(len==dwMaxBuffer)
			break;
	}
	pszOutput[dwMaxBuffer-1]=0;
	return len;
}
/***************************************

Usage: ReplaceStrInStr(myString,"%IP%","127.0.0.1");
Return: TRUE if successfull.

****************************************/
BOOL ReplaceStrInStr(string &strToReplace,const char *szReplace,const char *szReplaceWith)
{
	string::size_type offset;
	offset = strToReplace.find(szReplace);
	if(offset!=-1)
	{
		strToReplace.insert(offset,szReplaceWith);
		offset = strToReplace.find(szReplace);
		strToReplace.erase(offset,strlen(szReplace));
		return TRUE;
	}
	return FALSE;
}



char *Get_RuleValue(const TCHAR *szRuleName,SERVER_RULES *pSR,int iCompareMode)
{

	while(pSR!=NULL)
	{
		if(pSR->name!=NULL)
		{
			switch(iCompareMode)
			{
				case 0:
				default:
				{
					__try{

						if(_stricmp(pSR->name,szRuleName)==0)
							return pSR->value;
					}
					__except(EXCEPTION_ACCESS_VIOLATION == GetExceptionCode())
					{						
						return NULL;
					}

					break;
				}
				case 1:
				{
					if(strstr(pSR->name,szRuleName)!=NULL)
						return pSR->value;
					break;
				}
			}
		}
		pSR = pSR->pNext;

	}
	return NULL;
}

void CleanUp_PlayerList(LPPLAYERDATA &pPL)
{
	if(pPL!=NULL)
	{
		
		if(pPL->pNext!=NULL)
			CleanUp_PlayerList(pPL->pNext);
		
		pPL->pNext = NULL;
		if(pPL->szPlayerName!=NULL)
		{
			free(pPL->szPlayerName);
			pPL->szPlayerName = NULL;
		}
		if(pPL->szClanTag!=NULL)
		{
			free(pPL->szClanTag);
			pPL->szClanTag = NULL;
		}
		if(pPL->szTeam!=NULL)
		{
			free(pPL->szTeam);
			pPL->szTeam = NULL;
		}		
		free(pPL);
		pPL = NULL;
	}
}


//linked list clean up
void CleanUp_ServerRules(LPSERVER_RULES &pSR)
{
	if(pSR!=NULL)
	{
		__try
		{
			if(pSR->pNext!=NULL)
				CleanUp_ServerRules(pSR->pNext);

			free(pSR->name);
			free(pSR->value);
			pSR->value = NULL;
			pSR->name = NULL;
			pSR->pNext = NULL;
			free(pSR);
			pSR = NULL;

		} 	
		__except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION)
		{
			// exception handling code
			g_log.AddLogInfo(GS_LOG_ERROR,"Access Violation!!! @ CleanUp_ServerList(...)\n");
			DebugBreak();
		}
		
	}
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
		
		char *buf=(char*)calloc(2048, sizeof(char));	
		if(buf==NULL)
			dbg_print("Error allocating memory!\n");
	
		int ret=recv(s, buf, 2048, 0);

		if(ret!=SOCKET_ERROR && ret!=0) {
			*len=ret;
			
		//	dbg_print("Bytes recieved %d\n",ret);
			
			return (char*)realloc(buf, ret+1);
		}
		else {
		//	dbg_print("getpacket err: %d\n", WSAGetLastError());
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
	if(ret!=SOCKET_ERROR && ret!=0) 
	{
		*len=ret;
	
		dbg_print("Bytes recieved %d\n",ret);	
		return (char*)buf;
	}
	else 
	{
		//dbg_print("getpacket err: %d\n", WSAGetLastError());
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
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(char)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);

}



/*

BOOL Set_RuleStr(SERVER_RULES *pServerRules, TCHAR *szRuleName, TCHAR *StrOut, size_t OutbufferSize)
{
	TCHAR *pValue = Get_RuleValue(szRuleName,pServerRules); 
	if(pValue!=NULL)
	{
		strncpy(StrOut,pValue ,OutbufferSize);
		return TRUE;
	}
	return FALSE;
}
*/
BOOL Set_RuleInt(SERVER_RULES *pServerRules, TCHAR *szRuleName, int *IntOut)
{
	TCHAR *pValue = Get_RuleValue(szRuleName,pServerRules); 
	if(pValue!=NULL)
	{
		*IntOut = atoi(pValue);	
		return TRUE;
	}
	return FALSE;
}

/**************************************************************
Used for reading following XML example:
            <MatchOnColorEncoded>0</MatchOnColorEncoded>
            <LastSeenIP>69.90.19.3:27733</LastSeenIP>
            <LastSeenGameIdx>0</LastSeenGameIdx>

Usage:
	ReadCfgInt2(pBud , "LastSeenGameIdx",(int&)ptempBI->cGAMEINDEX);

*****************************************************************/
int ReadCfgInt2(TiXmlElement* pNode, char *szParamName, int& intVal)
{	
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{		
		const char *pName=pNode->Value();
		if(pName==NULL)
			continue;
	
		if(strcmp(szParamName,pName)==0)
		{
			const char *pValue = pNode->FirstChild()->Value();
			
			intVal = atoi(pValue);
			
			return XML_READ_OK	;					
		}
	}
	g_log.AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return XML_READ_ERROR;
}


const char * ReadCfgStr2(TiXmlElement* pNode, char *szParamName,char *szOutputBuffer,int iBuffSize)
{
	if(szOutputBuffer==NULL)
		return NULL;
	ZeroMemory(szOutputBuffer,iBuffSize);
	for( pNode; pNode; pNode=pNode->NextSiblingElement())
	{
		
		const char *pName=pNode->Value();
		if(pName==NULL)
			continue;
		
		
		if(strcmp(szParamName,pName)==0)
		{
			if(pNode->FirstChild()!=NULL)
			{
				const char *pValue = pNode->FirstChild()->Value();
			//int len = strlen(pValue);
			//strcpy(szOutputBuffer,pValue);
				strncpy(szOutputBuffer,pValue,iBuffSize);
				return szOutputBuffer;					
			}
			return NULL;
		}
	
	}
	g_log.AddLogInfo(0,"Error reading XML tag %s",szParamName);
	return NULL;
}


//Case sensitive
int wildicmp(const char *string, const char *wild) {
  // Written by Jack Handy - jakkhandy@hotmail.com

  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) 
  {
    if ((toupper(*wild) != toupper(*string)) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }


  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((toupper(*wild) == toupper(*string)) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}
 

int wildcmp(const char *string, const char *wild) 
{
  // Written by Jack Handy - jakkhandy@hotmail.com

  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }


  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}


//case insensitive strstr compare
const TCHAR *stristr(TCHAR *szString, const TCHAR *szSubstring)
{
   TCHAR   *  pPos = NULL;
   TCHAR   *  szCopy1 = NULL;
   TCHAR   *  szCopy2 = NULL;

   // verify parameters
   if ( szString == NULL || szSubstring == NULL )
   {
      return szString;
   }

   // empty substring - return input (consistent with strstr)
   if ( _tcslen(szSubstring) == 0 ) {
      return szString;
   }

   szCopy1 = _tcslwr(_tcsdup(szString));
   szCopy2 = _tcslwr(_tcsdup(szSubstring));

   if ( szCopy1 == NULL || szCopy2 == NULL  ) {
      // another option is to raise an exception here
      free((void*)szCopy1);
      free((void*)szCopy2);
      return NULL;
   }

   pPos = strstr(szCopy1, szCopy2);

   if ( pPos != NULL ) {
      // map to the original string
      pPos = szString + (pPos - szCopy1);
   }

   free((void*)szCopy1);
   free((void*)szCopy2);

   return pPos;
} // stristr(...)

