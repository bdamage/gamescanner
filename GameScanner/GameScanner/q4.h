
#include "stdafx.h"

#ifndef _Q4_HEADER_
#define _Q4_HEADER_


void Q4_SetCallbacks(long (*UpdateServerListView)(DWORD index),long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo), long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI));

DWORD Q4_ConnectToMasterServer(GAME_INFO *pGI, int nMasterIdx=0);
DWORD Q4_ParseServers(char * p, DWORD length, GAME_INFO *pGI,long (*InsertServerListView)(GAME_INFO *pGI,SERVER_INFO q4server));
DWORD Q4_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *q4players),long (*UpdateRulesListView)(SERVER_RULES *pServRules));
PLAYERDATA *Q4_ParsePlayers(SERVER_INFO *pSI, char *pointer,char *end, DWORD *numPlayers);

#endif