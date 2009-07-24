
#include "stdafx.h"

#ifndef _Q4_HEADER_
#define _Q4_HEADER_

struct Q4DATA
{
	char leadData[10]; //ÿÿservers
	char data[1];
} ;
struct Q4DATA_SERVER_INFO
{
	char leadData[23]; //ÿÿservers
	char data[1];
	
};
struct ETQWDATA_RESPONSE_SERVERINFO
{
	char leadData[31]; //ÿÿservers
	// 1  2  3  4  5  6  7 8  9  10 11 12 13                   20 21 22 23 24 25          29 30
	//ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 ff ff ff ff 0d 00 0d 00 1c 03 00
	char data[1];
	
};


void Q4_SetCallbacks(long (*UpdateServerListView)(DWORD index),long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo), long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI));

DWORD Q4_ConnectToMasterServer(GAME_INFO *pGI, int nMasterIdx=0);
DWORD Q4_ParseServers(char * p, DWORD length, GAME_INFO *pGI,long (*InsertServerListView)(GAME_INFO *pGI,SERVER_INFO q4server));
DWORD Q4_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *q4players),long (*UpdateRulesListView)(SERVER_RULES *pServRules));
PLAYERDATA *Q4_ParsePlayers(SERVER_INFO *pSI, char *pointer,char *end, DWORD *numPlayers);

#endif