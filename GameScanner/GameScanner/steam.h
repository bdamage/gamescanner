#include "GameScanner.h"
#include "structs_defines.h"
#include "utilz.h"

#ifndef _STEAM_HEADER_

#define  _STEAM_HEADER_

DWORD STEAM_parseServers(TCHAR * packet, DWORD length, GAME_INFO *pGI,TCHAR *szLastIP,DWORD &dwLastPort);
DWORD STEAM_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx=0);
DWORD STEAM_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules));
void STEAM_SetCallbacks(long (*UpdateServerListView)(DWORD index), long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI));
DWORD STEAM_ParsePlayers(SERVER_INFO *pSI, char *packet,DWORD dwLength);

#endif
