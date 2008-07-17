
#include "structs_defines.h"

#pragma once
#ifndef __UTILZ__
#define __UTILZ__


//Debug OFF!!!
#ifndef _DEBUG

#define dbg_print(exp) ((void)0)

#else

void dbg_print(char *szMsg, ...);

#endif



#define ETSV_WARNING 10
#define ETSV_INFO 20
#define ETSV_DEBUG 30
#define ETSV_ERROR 40

void LOGGER_DeInit();
void LOGGER_Init();
BOOL UTILZ_checkforduplicates(GAME_INFO *pGI, int hash,DWORD dwIP, DWORD dwPort);
SOCKET getsock(const char *host, unsigned short port, int family, int socktype, int protocol);
void AddGetLastErrorIntoLog(LPTSTR lpszFunction);
#define getsockudp(host,port) getsock(host, port, AF_INET, SOCK_DGRAM, IPPROTO_UDP)
#define getsocktcp(host,port) getsock(host, port, AF_INET, SOCK_STREAM, IPPROTO_TCP)
DLGTEMPLATE * WINAPI DoLockDlgRes(LPCSTR lpszResName) ;
void SetDlgTrans(HWND hwnd,int trans);
DWORD Get_GameTypeByName(int gametype, char *szGameType);
DWORD Get_GameTypeByGameType(int gametype, WORD szGameOldType);
const char * Get_GameTypeNameByGameType(int gametype, WORD cGameType);
DWORD Get_ModByName(int gametype, char *szModName);
DWORD Get_MapByName(int gameIdx, char *szMapName);
DWORD Get_FilterVersionByVersionString(int gametype, char *szVersion);
bool UTILZ_CheckForDuplicateServer(GAME_INFO *pGI, SERVER_INFO pSI);
char *colorfilter(const char* name,char *namefilter, int len);
char *colorfilterQ4(char* name,char *namefilter, size_t len=0);
BOOL CenterWindow(HWND hwnd);
bool IsInstalled(char *version);
void GetInstallPath(char *path);
void SetInstallPath(char *path);
void RegisterProtocol(char *path);
void SelfInstall(char *path);
void UnInstall();
void dbg_readbuf(const char *file, char *buf, size_t size);
void dbg_dumpbuf(const char *file, const void *buf, size_t size);
void AddLogInfo(int color, char *lpszText, ...);
void SetLogPath(const char *szPath);
char *getpacket(SOCKET s, size_t *len);
DEVMODE GetScreenResolution(VOID);
BOOL	SetScreenResolution(DEVMODE DisplaySettings);
void SetThreadName( DWORD dwThreadID, char* threadName);
//HFONT SetFont(HWND hDlg, int nIDDlgItem);
HFONT  MyCreateFont(HWND hDlg,LONG height = 12, LONG weight = FW_NORMAL, char* pszFontFace="Arial" );
void SetFontToDlgItem(HWND hDlg,HFONT hf,int nIDDlgItem);
void Show_ErrorDetails(LPTSTR lpszFunction);
void UTILZ_CleanUp_PlayerList(LPPLAYERDATA &pPL);
void UTILZ_SetStatusText( int icon, char *szMsg,...);
void UTILZ_CleanUp_ServerRules(LPSERVER_RULES &pSR);
void SetStatusText(int icon, char *szMsg,...);
char *	ReadPacket(SOCKET socket, size_t *len);
char * DWORD_IP_to_szIP(DWORD dwIP);
DWORD NetworkNameToIP(char *host_name,char *port);

#endif