#include "GameScanner.h"
#include "structs_defines.h"
#include "utilz.h"

void STEAM_OnServerSelection(SERVER_INFO* pServerInfo,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesList)(SERVER_RULES*pServer_Rules) );
DWORD STEAM_parseServers(char * packet, DWORD length, GAME_INFO *pGI,char *szLastIP,DWORD &dwLastPort);
DWORD STEAM_ConnectToMasterServer(GAME_INFO *pGI);
DWORD STEAM_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules));
void STEAM_SetCallbacks(long (*UpdateServerListView)(DWORD index), long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI));
