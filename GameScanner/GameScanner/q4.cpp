#include "stdafx.h"
#include "q4.h"
#include "utilz.h"
#include "..\..\iptocountry\iptocountry.h"


char Q4_unkown[]={"????"};
BOOL g_bQ4=FALSE;
bool Q4_bCloseApp = false;

long (*Q4_UpdateServerListView)(DWORD index);
long (*Q4_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);
long (*Q4_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);

DWORD Q4_dwTotalServers=0;
DWORD Q4_dwNewTotalServers=0;
extern HWND g_hwndProgressBar;
BOOL Q4_bScanningInProgress = FALSE;

void Q4_SetCallbacks(long (*UpdateServerListView)(DWORD index),
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q4_UpdateServerListView = UpdateServerListView;
	Q4_Callback_CheckForBuddy = _Callback_CheckForBuddy;
	Q4_InsertServerItem = InsertServerItem;
}



char *Q4_ParseServerRules(SERVER_RULES* &pLinkedListStart,char *p,DWORD packetlen)
{
	SERVER_RULES *pSR=NULL;
	SERVER_RULES *pCurrentSR=NULL;
	Q4DATA_SERVER_INFO *Q4SI; 
	ETQWDATA_RESPONSE_SERVERINFO *ETQWResponse;
	
	Q4SI = (Q4DATA_SERVER_INFO *)p;
	ETQWResponse  =(ETQWDATA_RESPONSE_SERVERINFO *)p;
	pSR=NULL;

	char *pointer = NULL;
	if(g_bQ4)
		pointer =Q4SI->data;
	else
		pointer = ETQWResponse->data;

	if(strcmp(&p[4],"disconnect")==0)
	{		
		dbg_print("Server response Disconnect!\n");
		return NULL;
	}

	while(1)
	{
		if(pSR==NULL)
		{
			pSR = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));
			
			pLinkedListStart = (SERVER_RULES *) pCurrentSR = pSR;			
		}else
		{
			pCurrentSR->pNext = (SERVER_RULES *)calloc(1,sizeof(SERVER_RULES));
			pCurrentSR = pCurrentSR->pNext;					
		}

		pCurrentSR->name = _strdup(pointer);
		pointer+=(int)strlen(pointer)+1;
		pCurrentSR->value = _strdup(pointer);
		pointer+=strlen(pointer)+1;
	
		if(pointer[0]==0 && pointer[1]==0)
			break;
	}
	if(g_bQ4)
		pointer+=2; //move to playerlist data (Q4)


	return pointer;
}

#define iMAX_PACKETS 50



DWORD Q4_ConnectToMasterServer(GAME_INFO *pGI)
{
	Q4_bScanningInProgress = TRUE;

	SERVER_INFO * pSI = NULL;
	
	size_t packetlen=0;

	//WSADATA wsaData;
	SOCKET ConnectSocket;

	ConnectSocket = getsockudp(pGI->szMasterServerIP,(unsigned short)pGI->dwMasterServerPORT);
   
	if(INVALID_SOCKET==ConnectSocket)
	{
		Q4_bScanningInProgress = FALSE;
		dbg_print("Error connecting to socket!");
		return 1;
	}

	//char sendbuf[80];
	//ZeroMemory(sendbuf,sizeof(sendbuf));
	char sendbuf[] = {"\xFF\xFFgetServers\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"}; 

	if(send(ConnectSocket, sendbuf, sizeof(sendbuf), 0)==SOCKET_ERROR) 
	{
		Q4_bScanningInProgress = FALSE;
		closesocket(ConnectSocket);		
		 dbg_print("Error sending packet!");
		return 2;
	}

	int i = 0;
	unsigned char *packet[iMAX_PACKETS];

	Q4_dwTotalServers=0;
	Q4_dwNewTotalServers=0;

	for(i=0; i<iMAX_PACKETS;i++)
	{
		packet[i] = NULL;

		packet[i]=(unsigned char*)getpacket(ConnectSocket, &packetlen);

/*		int iBytesInBuffer = 0;
		ioctlsocket(ConnectSocket, FIONREAD , (u_long FAR*) &iBytesInBuffer);

		if(iBytesInBuffer==0)
			DebugBreak();
*/
		//dbg_dumpbuf("serverlist.bin", packet[i], packetlen);

		if(packet[i]==NULL) 
		{

			dbg_print("Could NOT receive all packets!\n");
			break;  	
		} 
		
		//if(Q4_pSIServerListStart==NULL)
		Q4_parseServers((char*)packet[i],packetlen,pGI,Q4_InsertServerItem);
	//	else
	//		Q4_parseServers((char*)packet[i],packetlen,Q4_pSIServerListStart,InsertServerListView);

		free(packet[i]);
	
	}

	closesocket(ConnectSocket);

	//Global Q4 serverlist to start
	//Q4_pSI = pSI = Q4_pSIServerListStart;	

	pGI->dwTotalServers = pGI->pSC->vSI.size();

	//Q4_InitilizeRescan(pGI);
		
	Q4_bScanningInProgress = FALSE;

	return 	0;
}


SERVER_INFO *LastAddedServerToFavorites=NULL;

SERVER_INFO *Q4_LastAddedServerToFavorites()
{
	return LastAddedServerToFavorites;
}




SERVER_INFO* Q4_parseServers(char * p, DWORD length,  GAME_INFO *pGI,long (*InsertServerListView)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q4DATA *q4d;
	q4d = (Q4DATA*)p;

	int i=0;
	SERVER_INFO tempSI;
	DWORD idx = pGI->pSC->vSI.size();

	DWORD *dwIP=NULL;
	DWORD dwResult=0;


	p = q4d->data;

	if (p==NULL)
		return NULL;


	char *end;
	end = p+length-10;

	while(p<end) 
	{	

		ZeroMemory(&tempSI,sizeof(SERVER_INFO));

		//Parse and initialize server info
	
		dwIP = (DWORD*)&p[0];
		tempSI.dwIP = ntohl((DWORD)*dwIP); 
		
		sprintf_s(tempSI.szIPaddress,sizeof(tempSI.szIPaddress),"%d.%d.%d.%d",(unsigned char)p[0],(unsigned char)p[1],(unsigned char)p[2],(unsigned char)p[3]);
		tempSI.dwPort  = ((p[5])<<8);
		tempSI.dwPort |=(unsigned char)(p[4]);


		p+=6;	
		if(UTILZ_CheckForDuplicateServer(pGI,tempSI)==false)
		{		

			tempSI.cGAMEINDEX = (char) pGI->cGAMEINDEX;
			tempSI.cCountryFlag = 0;
			tempSI.bNeedToUpdateServerInfo = true;
			tempSI.dwIndex = idx++;
			tempSI.pPlayerData = NULL;
			strcpy(tempSI.szShortCountryName,"zz");
			tempSI.pServerRules = NULL;
			pGI->pSC->vSI.push_back(tempSI);

			if(InsertServerListView!=NULL)
				InsertServerListView(pGI,tempSI);

			Q4_dwNewTotalServers++;
		} //end serverexsist

		
		Q4_dwTotalServers++;

	} //end while

	return NULL;
}

DWORD Q4_GetTotalServers()
{
	return Q4_dwNewTotalServers;
}

char *Q4_Get_RuleValue(char *szRuleName,SERVER_RULES *pSR)
{
	while(pSR!=NULL)
	{
		if(_stricmp(pSR->name,szRuleName)==0)
			return pSR->value;
		pSR = pSR->pNext;

	}
	return NULL;
}



DWORD Q4_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *q4players),long (*UpdateRulesListView)(SERVER_RULES *pServRules))
{

	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return -1;
	}
	if(pSI->pPlayerData!=NULL)
		Q4_CleanUp_PlayerList(pSI->pPlayerData);
	pSI->pPlayerData = NULL;

	if(pSI->pServerRules!=NULL)
		Q4_CleanUp_ServerRules(pSI->pServerRules);
	pSI->pServerRules = NULL;

	SOCKET pSocket;
	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->dwPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return -1;
	}
  
	char sendbuf[]={"\xFF\xFFgetInfo\x00\x01\x00\x00\x00"};
	

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{
		DWORD dwIPSHORT;
	    char country[60],szShortName[8];
		ZeroMemory(szShortName,sizeof(szShortName));
		strncpy_s(pSI->szCountry,sizeof(pSI->szCountry),fnIPtoCountry2(pSI->dwIP,&dwIPSHORT,country,szShortName),49);  //Update country info only when adding a new server						
		strcpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName);
	}

	packetlen = send(pSocket, sendbuf, 14, 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);
		pSI->cPurge++;
		return -1;
	}

	unsigned char *packet=NULL;
	DWORD dwStartTick=0;

	
	dwStartTick = GetTickCount();

	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);

		

		//dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data
		pCurrPointer = Q4_ParseServerRules(pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		
			char *szVarValue = NULL;
			szVarValue = Q4_Get_RuleValue("si_version",pServRules);
			if(szVarValue!=NULL)
				strncpy(pSI->szVersion, szVarValue,49);	

			if(Q4_Get_RuleValue("si_name",pServRules)!=NULL)
				strncpy(pSI->szServerName, Q4_Get_RuleValue("si_name",pServRules),99);

			PLAYERDATA *pQ4Players=NULL;
			DWORD nPlayers=0;
			//---------------------------------
			//Retrieve players if any exsist...
			//---------------------------------
			pQ4Players = Q4_ParsePlayers(pSI,pCurrPointer,end,&nPlayers);
			pSI->pPlayerData = pQ4Players;
			if(pQ4Players!=NULL)
			{
				if(UpdatePlayerListView!=NULL)
					UpdatePlayerListView(pQ4Players);
				
				Q4_Callback_CheckForBuddy(pQ4Players,pSI);
			//	dbg_print("Sweet we got some players!\n");
			}
			if(UpdateRulesListView!=NULL)
				UpdateRulesListView(pServRules);
			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------
			pSI->bNeedToUpdateServerInfo = false;
			pSI->bUpdated = true;
			pSI->nCurrentPlayers = nPlayers;
			
			
			if(Q4_Get_RuleValue("si_name",pServRules)!=NULL)
				strncpy(pSI->szServerName, Q4_Get_RuleValue("si_name",pServRules),99);
			if(Q4_Get_RuleValue("si_map",pServRules)!=NULL)
				strncpy(pSI->szMap, Q4_Get_RuleValue("si_map",pServRules),39);
			if(Q4_Get_RuleValue("gamename",pServRules)!=NULL)
			{
				strncpy(pSI->szMod, Q4_Get_RuleValue("gamename",pServRules),19);
				pSI->wMod = Get_ModByName(pSI->cGAMEINDEX, pSI->szMod);
			}

		
			szVarValue = Q4_Get_RuleValue("si_gametype",pServRules);
			if(szVarValue!=NULL)
			{	
				strncpy(pSI->szGameTypeName, szVarValue,29);		
			}
			else
			{
				szVarValue = Q4_Get_RuleValue("si_rules",pServRules);
				if(szVarValue!=NULL)
				{		
					if(strlen(szVarValue)>11)
						strncpy_s(pSI->szGameTypeName,sizeof(pSI->szGameTypeName), &szVarValue[strlen("sdGameRules")],14);	
					else
						strcpy_s(pSI->szGameTypeName,sizeof(pSI->szGameTypeName),"Unknown");

					
				}

			}
			
			pSI->dwGameType = Get_GameTypeByName(pSI->cGAMEINDEX, pSI->szGameTypeName);

			szVarValue = Q4_Get_RuleValue("si_pure",pServRules);
			if(szVarValue!=NULL)
				pSI->cPure = atoi(szVarValue);

			//Trim Mapname
			char *start = strchr(pSI->szMap,'/');
			if(start!=NULL)
			{
				//Let's do the trim
				start++;
			
				char *end = strchr(pSI->szMap,'.');
				if(end!=NULL)
					end[0]=0;

				strcpy_s(pSI->szMap,sizeof(pSI->szMap),start);
			}
			pSI->dwMap = Get_MapByName(pSI->cGAMEINDEX, pSI->szMap);

			
			if(Q4_Get_RuleValue("net_serverPunkbusterEnabled",pServRules)!=NULL)
				pSI->bPunkbuster = (char)atoi(Q4_Get_RuleValue("net_serverPunkbusterEnabled",pServRules));
			else if(Q4_Get_RuleValue("sv_punkbuster",pServRules)!=NULL)
				pSI->bPunkbuster = (char)atoi(Q4_Get_RuleValue("sv_punkbuster",pServRules));
			
			if(Q4_Get_RuleValue("si_usepass",pServRules)!=NULL)
				pSI->bPrivate = (char)atoi(Q4_Get_RuleValue("si_usepass",pServRules));				
			else if(Q4_Get_RuleValue("si_needPass",pServRules)!=NULL)  //ETQW
				pSI->bPrivate = (char)atoi(Q4_Get_RuleValue("si_needPass",pServRules));
			
			//si_numPrivatePlayers
			if(Q4_Get_RuleValue("si_privatePlayers",pServRules)!=NULL)
				pSI->nPrivateClients = atoi(Q4_Get_RuleValue("si_privatePlayers",pServRules));
			else if(Q4_Get_RuleValue("si_privateClients",pServRules)!=NULL) 			//ETQW
				pSI->nPrivateClients = atoi(Q4_Get_RuleValue("si_privateClients",pServRules));

			if(Q4_Get_RuleValue("si_maxPlayers",pServRules)!=NULL)
				pSI->nMaxPlayers = atoi(Q4_Get_RuleValue("si_maxPlayers",pServRules));
	

			
			//Debug purpose
			if(pServRules!=pSI->pServerRules)
			{
				AddLogInfo(ETSV_DEBUG,"Error at pServRules!=pSI->pServerRules");
				DebugBreak();
			}

		//	if(UpdatePlayerListView==NULL)
			{
				Q4_CleanUp_PlayerList(pQ4Players);
				pSI->pPlayerData = NULL;
				Q4_CleanUp_ServerRules(pServRules);
				pSI->pServerRules = NULL;

			}

		} //end if(pServRules!=NULL)

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;

	closesocket(pSocket);
	return 0;
}

void Q4_Get_ServerUpdateByLPARAM(LPARAM *param)
{
	SERVER_INFO *pSI = (SERVER_INFO*)param;
}


void Q4_OnServerSelection(SERVER_INFO* pServerInfo,long (*UpdatePlayerListView)(PLAYERDATA *pPlayers),long (*UpdateRulesList)(SERVER_RULES*pServer_Rules) )
{
	if(pServerInfo==NULL)
		return;

	Q4_Get_ServerStatus(pServerInfo,UpdatePlayerListView,UpdateRulesList);

	if(Q4_UpdateServerListView!=NULL)
		Q4_UpdateServerListView(pServerInfo->dwIndex);
}



PLAYERDATA *Q4_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers)
{
	
	PLAYERDATA *pQ4Players=NULL;
	if(g_bQ4==FALSE)
		pointer+=2;

	pSI->cBots = 0;
	if(pointer[0]==0x20)
	{
		if(pointer[5]==0x01)
		{
		//	dbg_print("Found ranked server!\n");
		//	dbg_print(pSI->szServerName);
		//	dbg_print("\n");
			pSI->cRanked = 1;
		}
	}

	if(pointer[0]==0 || pointer[0]==1)
	{
		//Parseplayers
	
		PLAYERDATA *pQ4CurrentPlayer=NULL;
		
		while(pointer<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			player->pNext = NULL;
			
			player->iPlayer = pointer[0]; //Player Index
			pointer++; 

			WORD *ping,*rate;
			ping = (WORD*)&pointer[0];
			player->ping  = *ping; 
			pointer+=2;
			if(g_bQ4)
			{
				rate = (WORD*)&pointer[0];
				player->rate  = *rate; 
				pointer+=4;//Unknown bytes
			}
		

			player->szPlayerName = _strdup(pointer);
			pointer+=strlen(pointer)+1;
			
			//	player->pCurrentServer = pSI;
			if(g_bQ4) //Is it Quake 4
			{				
				player->szClanTag = _strdup(pointer);
				pointer+=strlen(pointer)+1;
			} else //Otherwise go for ETQW
			{

			//	if((strstr(pSI->szVersion,"1.2")!=NULL) || (strstr(pSI->szVersion,"1.4")!=NULL)) //Since ETQW v1.2
				{
					pointer++; //Skip Clan tag position info
					player->szClanTag = _strdup(pointer);						
					pointer+=strlen(pointer)+1;
				}

				//player->ping = pointer[0];
				if((char)pointer[0] == 0x01)
				{
					player->szClanTag = _strdup("BOT");
					pSI->cBots++;
				}
				else
					player->szClanTag = _strdup("PLAYER");

				pointer++;

			
			}

			if(pQ4Players==NULL)
				pQ4Players = pQ4CurrentPlayer = player;
			else 
				pQ4CurrentPlayer = pQ4CurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;
			//char szdebug[200];
			//sprintf(szdebug,"ply (%d) #%d %s\n",g_bQ4,*numPlayers,player->szPlayerName);
			//dbg_print(szdebug);
		//	if(*numPlayers>32)
		//		DebugBreak();

			if(!g_bQ4) //ETQW specific
			{	
				if(pointer[5]==0x01)
				{
					/*dbg_print("Found ranked server!\n");
					dbg_print(pSI->szServerName);
					dbg_print("\n");*/
					pSI->cRanked = 1;
				
				//	pointer++; 4+2
					 /* 
					  int osMask;
					  byte isRanked;
					  int timeLeft;
					  byte gameState;
					  byte serverType; // 0 for regular server, 1 for tv server
					  //After this, the next block depends on the server type.

					  byte numInterestedClients; // number of clients considering joining this server
					  
					  //For tv servers:

					  int numConnectedClients; // number of clients on the tv server
					  int maxClients; // max clients that the tv server supports
						*/


				}

			}

			if (pointer[0]==0x20)  //ETQW & Q4
				break;
				
		}
		
	}
	return pQ4Players;
}

//linked list clean up
void Q4_CleanUp_ServerRules(LPSERVER_RULES &pSR)
{
	if(pSR!=NULL)
	{
		if(pSR->pNext!=NULL)
			Q4_CleanUp_ServerRules(pSR->pNext);
		
		free(pSR->name);
		free(pSR->value);
		pSR->pNext = NULL;
		free(pSR);
		pSR = NULL;

	}
}

void Q4_CleanUp_PlayerList(LPPLAYERDATA &pPL)
{
	if(pPL!=NULL)
	{
		if(pPL->pNext!=NULL)
			Q4_CleanUp_PlayerList(pPL->pNext);
		
		free(pPL->szPlayerName);
		free(pPL->szClanTag);
		pPL->pNext = NULL;
		free(pPL);
		pPL = NULL;
	}
}
