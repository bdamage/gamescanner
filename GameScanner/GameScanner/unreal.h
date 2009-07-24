


DWORD UT_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *pPlayers),long (*UpdateRulesListView)(SERVER_RULES *pServerRules));
char *GS_ParseServerRules(SERVER_RULES* &pLinkedListStart,char *packet,DWORD packetlen);
PLAYERDATA *GS_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers);