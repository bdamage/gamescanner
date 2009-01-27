//#include "ETServerViewer.h"
#include "structs_defines.h"
#include "utilz.h"

#ifndef __GS_LOG_SCANNER__
#define __GS_LOG_SCANNER__


#endif


void SCAN_Set_CALLBACKS(DWORD (*Get_ServerStatus)(SERVER_INFO *pSI,
				   long (*UpdatePlayerListView)(LPPLAYERDATA pPlayers),
				   long (*UpdateRulesListView)(LPSERVER_RULES pServerRules)),
				   long (*SCANNER_UpdateServerListView)(DWORD idx)
				   );
//Functions declaration
/*void Initilize_Rescan(GAME_INFO *pGI, bool (*filterServerItem)(LPARAM *lp,DWORD dwFilterFlags));
DWORD WINAPI  Get_ServerStatusThread(LPVOID lpParam);
*/

void Initialize_Rescan2(GAME_INFO *pGI, bool (*filterServerItem)(SERVER_INFO *lp,GAME_INFO *pGI));
DWORD WINAPI  Get_ServerStatusThread2(LPVOID lpParam);