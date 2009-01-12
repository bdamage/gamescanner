
#include "stdafx.h"
#include "structs_defines.h"
#include "utilz.h"

#ifndef _Q3_HEADER_

#define  _Q3_HEADER_

struct Q3DATA
{
	TCHAR leadData[10]; //ÿÿservers
	TCHAR data[1];
} ;
struct Q3DATA_SERVER_INFO
{
	TCHAR lead[4]; //ÿÿÿÿ
	TCHAR leadData[14]; //statusResponse or infoResponse
	TCHAR seperators[2];
	TCHAR data[1];  //first server var
};

struct Q2DATA_SERVER_INFO
{
	TCHAR lead[4]; //ÿÿÿÿ
	TCHAR leadData[5]; //print
	TCHAR seperators[2];
	TCHAR data[1];  //first server var starts here
};

struct WARSOWDATA_SERVER_INFO //
{
	TCHAR lead[4]; //ÿÿÿÿ
	TCHAR leadData[12]; //statusResponse or infoResponse
	TCHAR seperators[2];
	TCHAR data[1];  //first server var
};

struct QWDATA_SERVER_SHORTINFO
{
	TCHAR lead[4]; //ÿÿÿÿ
	TCHAR leadData[1]; //n
	TCHAR seperator[1];
	TCHAR data[1];  //first server var
};

void Q3_SetCallbacks(long (*UpdateServerListView)(DWORD index), long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),long (*Q3_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI));


void Q3_InitilizeRescan(GAME_INFO *pGI,bool (*filterServerItem)(LPARAM *lp,DWORD dwFilterFlags));
DWORD Q3_ConnectToMasterServer(GAME_INFO *pGI, int nMasterIdx=0);
SERVER_INFO *Q3_ParseServers(TCHAR * p, DWORD length, GAME_INFO *CV);
TCHAR *Q3_ParseServerRules(SERVER_RULES* &pLinkedListStart,TCHAR *p,DWORD packetlen);
DWORD WINAPI  Q3_Get_ServerStatusThread(LPVOID lpParam);
DWORD Q3_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules));
PLAYERDATA *Q3_ParsePlayers(SERVER_INFO *pSI, TCHAR *pointer,TCHAR *end, DWORD *numPlayers,TCHAR *szP=NULL);
PLAYERDATA *QW_ParsePlayers(SERVER_INFO *pSI,TCHAR *pointer,TCHAR *end, DWORD *numPlayers);


#endif