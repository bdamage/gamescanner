
#include "structs_defines.h"

#pragma once
#ifndef __UTILZ__
#define __UTILZ__


//Debug OFF!!!
#ifndef _DEBUG

#define dbg_print(exp) ((void)0)

#else

void dbg_print(TCHAR *szMsg, ...);

#endif



#define ETSV_WARNING 10
#define ETSV_INFO 20
#define ETSV_DEBUG 30
#define ETSV_ERROR 40


TCHAR *SplitIPandPORT(TCHAR *szIPport,DWORD &port);

TCHAR *Get_RuleValue(TCHAR *szRuleName,SERVER_RULES *pSR);
int UTILZ_ConvertEscapeCodes(TCHAR*pszInput,TCHAR*pszOutput,DWORD dwMaxBuffer);
void LOGGER_DeInit();
void LOGGER_Init();
BOOL UTILZ_checkforduplicates(GAME_INFO *pGI, int hash,DWORD dwIP, DWORD dwPort);
SOCKET getsock(const TCHAR *host, unsigned short port, int family, int socktype, int protocol);
void AddGetLastErrorIntoLog(LPTSTR lpszFunction);
#define getsockudp(host,port) getsock(host, port, AF_INET, SOCK_DGRAM, IPPROTO_UDP)
#define getsocktcp(host,port) getsock(host, port, AF_INET, SOCK_STREAM, IPPROTO_TCP)
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCSTR lpszResName) ;
void SetDlgTrans(HWND hwnd,int trans);
DWORD Get_GameTypeByName(int gametype, TCHAR *szGameType);
DWORD Get_GameTypeByGameType(int gametype, WORD szGameOldType);
const TCHAR * Get_GameTypeNameByGameType(int gametype, WORD cGameType);
DWORD Get_ModByName(int gametype, TCHAR *szModName);
DWORD Get_MapByName(int gameIdx, TCHAR *szMapName);
DWORD Get_FilterVersionByVersionString(int gametype, TCHAR *szVersion);
DWORD Get_ModeByName(int gameIdx, char *szMode);
//bool UTILZ_CheckForDuplicateServer(GAME_INFO *pGI, SERVER_INFO pSI);
TCHAR *colorfilter(const TCHAR* name,TCHAR *namefilter, int len);
TCHAR *colorfilterQ4(const TCHAR* name,TCHAR *namefilter, int len);
TCHAR *colorfilterQW(const TCHAR *szInText,TCHAR *namefilter, int len);
TCHAR *colorfilterNEXUIZ(const TCHAR* name,TCHAR *namefilter,int len);
BOOL CenterWindow(HWND hwnd);
bool IsInstalled(TCHAR *version);
void GetInstallPath(TCHAR *path);
void SetInstallPath(TCHAR *path);
void RegisterProtocol(TCHAR *path);
void SelfInstall(TCHAR *path);
void UnInstall();
void dbg_readbuf(const TCHAR *file, TCHAR *buf, size_t size);
void dbg_dumpbuf(const TCHAR *file, const void *buf, size_t size);
void AddLogInfo(int color, TCHAR *lpszText, ...);
void SetLogPath(const TCHAR *szPath);
TCHAR *getpacket(SOCKET s, size_t *len);
DEVMODE GetScreenResolution(VOID);
BOOL	SetScreenResolution(DEVMODE DisplaySettings);
void SetThreadName( DWORD dwThreadID, TCHAR* threadName);
//HFONT SetFont(HWND hDlg, int nIDDlgItem);
HFONT  MyCreateFont(HWND hDlg,LONG height = 14, LONG weight = FW_NORMAL, TCHAR* pszFontFace=_T("Arial") );
void SetFontToDlgItem(HWND hDlg,HFONT hf,int nIDDlgItem);
void Show_ErrorDetails(LPTSTR lpszFunction);
void CleanUp_PlayerList(LPPLAYERDATA &pPL);
void UTILZ_SetStatusText( int icon, TCHAR *szMsg,...);
void CleanUp_ServerRules(LPSERVER_RULES &pSR);
void SetStatusText(int icon, const TCHAR *szMsg,...);
TCHAR *	ReadPacket(SOCKET socket, size_t *len);
TCHAR * DWORD_IP_to_szIP(DWORD dwIP);
DWORD NetworkNameToIP(TCHAR *host_name,TCHAR *port);
BOOL Set_RuleStr(SERVER_RULES *pServerRules, TCHAR *szRuleName, TCHAR *StrOut, size_t OutbufferSize);
BOOL Set_RuleInt(SERVER_RULES *pServerRules, TCHAR *szRuleName, int *IntOut);

#endif