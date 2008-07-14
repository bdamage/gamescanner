
#include "structs_defines.h"
#include "utilz.h"



struct Q3DATA
{
	char leadData[10]; //ÿÿservers
	char data[1];
} ;
struct Q3DATA_SERVER_INFO
{
	char lead[4]; //ÿÿÿÿ
	char leadData[14]; //statusResponse or infoResponse
	char seperators[2];
	char data[1];  //first server var
};

struct Q2DATA_SERVER_INFO
{
	char lead[4]; //ÿÿÿÿ
	char leadData[5]; //print
	char seperators[2];
	char data[1];  //first server var starts here
};

struct WARSOWDATA_SERVER_INFO //
{
	char lead[4]; //ÿÿÿÿ
	char leadData[12]; //statusResponse or infoResponse
	char seperators[2];
	char data[1];  //first server var
};

struct QWDATA_SERVER_SHORTINFO
{
	char lead[4]; //ÿÿÿÿ
	char leadData[1]; //n
	char seperator[1];
	char data[1];  //first server var
};

void Q3_SetCallbacks(long (*UpdateServerListView)(DWORD index), long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),long (*Q3_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI));


void Q3_InitilizeRescan(GAME_INFO *pGI,bool (*filterServerItem)(LPARAM *lp,DWORD dwFilterFlags));
//SERVER_INFO * Q3_ConnectToMasterServer(DWORD gameID, char *szMasterServerIP,DWORD port,DWORD protocol,long (*UpdateServerListView)(LPARAM *Q3server),long (*InstertServerListView)(SERVER_INFO  *Q3server),HWND hwndProgressBar);
DWORD Q3_ConnectToMasterServer(GAME_INFO *pGI);
SERVER_INFO *Q3_parseServers(char * p, DWORD length, GAME_INFO *CV);
DWORD Q3_GetTotalServers();



char *Q3_ParseServerRules(SERVER_RULES* &pLinkedListStart,char *p,DWORD packetlen);
DWORD WINAPI  Q3_Get_ServerStatusThread(LPVOID lpParam);
DWORD Q3_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules));
//char *Q3_ParseServerRules(Q3DATA_SERVER_RULES* pLinkedListStart,char *p,DWORD packetlen);
PLAYERDATA *Q3_ParsePlayers(SERVER_INFO *pSI, char *pointer,char *end, DWORD *numPlayers,char *szP=NULL);
PLAYERDATA *QW_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers);
void Q3_OnServerSelection(SERVER_INFO* pServerInfo,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules ));
char *Q3_Get_RuleValue(char *szRuleName,SERVER_RULES *pSR);
void Q3_UpdateItem(LPARAM * lp);
