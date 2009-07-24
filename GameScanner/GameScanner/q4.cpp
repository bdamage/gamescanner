

#include "stdafx.h"
#include "q4.h"
#include "utilz.h"
#include "gamemanager.h"

#define _SECURE_SCL 0
#pragma warning(disable : 4995)



long (*Q4_UpdateServerListView)(DWORD index);
long (*Q4_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);
long (*Q4_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);



extern HWND g_hwndProgressBar;
extern APP_SETTINGS_NEW AppCFG;
extern CLanguage g_lang;
extern CIPtoCountry g_IPtoCountry;
extern CGameManager gm;

//extern GamesMap gm.GamesInfo;



void Q4_SetCallbacks(long (*UpdateServerListView)(DWORD index),
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q4_UpdateServerListView = UpdateServerListView;
	Q4_Callback_CheckForBuddy = _Callback_CheckForBuddy;
	Q4_InsertServerItem = InsertServerItem;
}



char *Q4_ParseServerRules(SERVER_INFO* pSI,SERVER_RULES* &pLinkedListStart,char *packet,DWORD packetlen)
{
	SERVER_RULES *pSR=NULL;
	SERVER_RULES *pCurrentSR=NULL;
	Q4DATA_SERVER_INFO *Q4SI; 
	ETQWDATA_RESPONSE_SERVERINFO *ETQWResponse;
	
	Q4SI = (Q4DATA_SERVER_INFO *)packet;
	ETQWResponse  =(ETQWDATA_RESPONSE_SERVERINFO *)packet;
	pSR=NULL;

	char *pointer = NULL;
	if(pSI->cGAMEINDEX==ETQW_SERVERLIST)
		pointer = ETQWResponse->data;
	else
		pointer =Q4SI->data;

	if(strcmp(&packet[4],"disconnect")==0)
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
	pointer+=2; //move to playerlist data (Q4)


	return pointer;
}

#define iMAX_PACKETS 50



DWORD Q4_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx)
{
	size_t packetlen=0;
	char sendbuf[40];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	
	int len = 0;
	len = UTILZ_ConvertEscapeCodes(pGI->szMasterQueryString,sendbuf,sizeof(sendbuf));
	char szIP[260];
	strcpy(szIP,pGI->szMasterServerIP[iMasterIdx]);
	
	SplitIPandPORT(szIP,pGI->dwMasterServerPORT);

	dbg_print("Master server %s:%d",szIP,(unsigned short)pGI->dwMasterServerPORT);

	SOCKET ConnectSocket = getsockudp(szIP,(unsigned short)pGI->dwMasterServerPORT);
   
	if(INVALID_SOCKET==ConnectSocket)
	{
		dbg_print("Error connecting to socket!");
		return 1001;
	}
	dbg_print("Sending command %s Len: %d",sendbuf,len);
	if(send(ConnectSocket, sendbuf, len, 0)==SOCKET_ERROR) 
	{
		closesocket(ConnectSocket);		
		dbg_print("Error sending packet!");
		return 1002;
	}

	int i = 0;
	unsigned char *packet[iMAX_PACKETS];
	size_t packlen[iMAX_PACKETS];

	DWORD Q4_dwNewTotalServers=0;
	// Let's retrieve all packets as fast as possible, that's the reason for splitting up int two for loop.
	for(i=0; i<iMAX_PACKETS;i++)
	{
		packet[i] = NULL;
		packet[i]=(unsigned char*)getpacket(ConnectSocket, &packlen[i]);
		if(packet[i]==NULL) //End of transfer
			break;  	
	}
	//Now parse all server IP's from recieved packets
	for(i=0; i<iMAX_PACKETS;i++)
	{
		if(packet[i]==NULL) //End of packets
			break;  	
		Q4_dwNewTotalServers += Q4_ParseServers((char*)packet[i],packlen[i],pGI,Q4_InsertServerItem);
		SetStatusText(pGI->iIconIndex,g_lang.GetString("StatusReceivingMaster"),Q4_dwNewTotalServers,pGI->szGAME_NAME);		
		free(packet[i]);
	}
	
	closesocket(ConnectSocket);
	pGI->dwTotalServers = pGI->vSI.size();
	return 	0;
}



DWORD Q4_ParseServers(char * p, DWORD length,  GAME_INFO *pGI,long (*InsertServerListView)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	DWORD dwNewServers = 0;
	Q4DATA *q4d;
	SERVER_INFO tempSI;
	DWORD *dwIP=NULL;
	DWORD idx = pGI->vSI.size();

	if (p==NULL)
		return 0;

	q4d = (Q4DATA*)p;
	char *end = p+length;

	p = q4d->data;

	ZeroMemory(&tempSI,sizeof(SERVER_INFO));		
	tempSI.pPlayerData = NULL;
	tempSI.pServerRules = NULL;
	strcpy(tempSI.szShortCountryName,"zz");
	tempSI.cGAMEINDEX =  pGI->cGAMEINDEX;
	tempSI.bUpdated = 0;
	tempSI.dwPing = 9999;

	int hash = 0;
	while(p<end) 
	{	
		dwIP = (DWORD*)&p[0];
		tempSI.dwIP = ntohl((DWORD)*dwIP); 
		tempSI.usPort  = ((p[5])<<8);
		tempSI.usPort |=(unsigned char)(p[4]);
		tempSI.usQueryPort = tempSI.usPort;
		hash = tempSI.dwIP + tempSI.usPort;
		
		if(UTILZ_checkforduplicates(pGI,  hash,tempSI.dwIP, tempSI.usPort)==FALSE)	
		{		
			sprintf_s(tempSI.szIPaddress,sizeof(tempSI.szIPaddress),"%d.%d.%d.%d",(unsigned char)p[0],(unsigned char)p[1],(unsigned char)p[2],(unsigned char)p[3]);
			tempSI.dwIndex = idx++;
			SERVER_INFO *pNewSrv = (SERVER_INFO*)calloc(1,sizeof(SERVER_INFO));
			memcpy(pNewSrv,&tempSI,sizeof(SERVER_INFO));
			InitializeCriticalSection(&pNewSrv->csLock);
			pGI->shash.insert(Int_Pair(hash,tempSI.dwIndex));
			pGI->vSI.push_back(pNewSrv);
			dwNewServers++;
		} //end serverexsist
		p+=6;			
	} //end while

	return dwNewServers;
}




DWORD Q4_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *q4players),long (*UpdateRulesListView)(SERVER_RULES *pServRules))
{

	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return -1;
	}
/*	if(pSI->bLocked)
	{
		dbg_print("Server locked @Get_ServerStatus!\n");
		return 3;
	}

	pSI->bLocked = TRUE;
*/
	SOCKET pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	//  pSI->bLocked = FALSE;
	  return -1;
	}
  
	char sendbuf[]={"\xFF\xFFgetInfo\x00\x01\x00\x00\x00"};
	

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	//If country shortname is EU or zz (Unknown) try to find a country based on the IP address.
	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{
		char szShortName[4];			
		g_IPtoCountry.IPtoCountry(pSI->dwIP,szShortName);
		strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
	}
	DWORD dwRetries=0;
retry:
	packetlen = send(pSocket, sendbuf, 14, 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);
		pSI->cPurge++;
	//	pSI->bLocked = FALSE;
		return -1;
	}

	unsigned char *packet=NULL;
	DWORD dwStartTick=0;

	
	dwStartTick = GetTickCount();

	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if(packet==NULL)
	{
		if(dwRetries<AppCFG.dwRetries)
		{
			dwRetries++;
			goto retry;
		}
	}

	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);
		//pSI->cPure=0;

		//dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		GetServerLock(pSI);

		if(pSI->pPlayerData!=NULL)
			CleanUp_PlayerList(pSI->pPlayerData);
		pSI->pPlayerData = NULL;

		if(pSI->pServerRules!=NULL)
			CleanUp_ServerRules(pSI->pServerRules);
		pSI->pServerRules = NULL;

		char *pCurrPointer=NULL; //will contain the start address for the player data
		pCurrPointer = Q4_ParseServerRules(pSI,pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		
			char *szVarValue = NULL;

			pSI->szServerName = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_SERVERNAME).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szMap = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MAP).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szMod = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_MOD).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szGameTypeName = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_GAMETYPE).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szVersion = Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_VERSION).sRuleValue.c_str(),pSI->pServerRules);
			pSI->szFS_GAME = Get_RuleValue("fs_game",pSI->pServerRules);


			szVarValue= Get_RuleValue((TCHAR*)gm.GamesInfo[pSI->cGAMEINDEX].vGAME_SPEC_COL.at(COL_PRIVATE).sRuleValue.c_str(),pSI->pServerRules);
			if(szVarValue!=NULL)
				pSI->bPrivate = (char)atoi(szVarValue);

			if(Get_RuleValue("net_serverPunkbusterEnabled",pServRules)!=NULL)
				pSI->bPunkbuster = (char)atoi(Get_RuleValue("net_serverPunkbusterEnabled",pServRules));
			else if(Get_RuleValue("sv_punkbuster",pServRules)!=NULL)
				pSI->bPunkbuster = (char)atoi(Get_RuleValue("sv_punkbuster",pServRules));


			PLAYERDATA *pQ4Players=NULL;
			DWORD nPlayers=0;
			//---------------------------------
			//Retrieve players if any exsist...
			//---------------------------------
			pQ4Players = Q4_ParsePlayers(pSI,pCurrPointer,end,&nPlayers);
			pSI->pPlayerData = pQ4Players;


			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------
			time(&pSI->timeLastScan);
			pSI->bUpdated = 1;
			pSI->nPlayers = nPlayers;
			pSI->cPurge = 0;

			
			if(Get_RuleValue("si_maxPlayers",pServRules)!=NULL)
				pSI->nMaxPlayers = atoi(Get_RuleValue("si_maxPlayers",pServRules));

	//		if(Get_RuleValue("si_tv",pServRules)!=NULL)
	//			pSI->bTV = atoi(Get_RuleValue("si_tv",pServRules));		
			
			if(Get_RuleValue("si_privatePlayers",pServRules)!=NULL)
				pSI->nPrivateClients = atoi(Get_RuleValue("si_privatePlayers",pServRules));
			else if(Get_RuleValue("si_privateClients",pServRules)!=NULL) 			//ETQW
			{
				pSI->nPrivateClients = atoi(Get_RuleValue("si_privateClients",pServRules));
				pSI->nMaxPlayers -=	pSI->nPrivateClients;
			}

		} //end if(pServRules!=NULL)

		ReleaseServerLock(pSI);
		if(Q4_Callback_CheckForBuddy!=NULL)
			Q4_Callback_CheckForBuddy(pSI->pPlayerData,pSI);

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;

	if(UpdatePlayerListView!=NULL) 
		UpdatePlayerListView(pSI->pPlayerData);
		
	if(UpdateRulesListView!=NULL)
		UpdateRulesListView(pSI->pServerRules);

	//pSI->bLocked = FALSE;
	closesocket(pSocket);
	return 0;
}


PLAYERDATA *Q4_ParsePlayers(SERVER_INFO *pSI,char *packet,char *end, DWORD *numPlayers)
{
	PLAYERDATA *pQ4Players=NULL;

	pSI->cBots = 0;
	if(packet[0]==0x20)
	{
		if(packet[5]==0x01)
		{
			pSI->cRanked = 1;   //ETQW
		
		}
	}
/*
bot_minplayers 
*/
	if(packet[0]!=32)// || packet[0]==1)
	{
		//Parseplayers
	
		PLAYERDATA *pQ4CurrentPlayer=NULL;
		
		while(packet<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			player->pNext = NULL;
			player->cGAMEINDEX = pSI->cGAMEINDEX;
			player->dwServerIndex = pSI->dwIndex;

			player->iPlayer = packet[0]; //Player Index
			packet++; 

			WORD *ping,*rate;
			ping = (WORD*)&packet[0];
			player->ping  = *ping; 
			packet+=2;
			if((pSI->cGAMEINDEX==Q4_SERVERLIST) ||(pSI->cGAMEINDEX==DOOM3_SERVERLIST) )
			{
				rate = (WORD*)&packet[0];
				player->rate  = *rate; 
				packet+=4;//Unknown bytes
			}

			player->szPlayerName = _strdup(packet);
			packet+=strlen(packet)+1;
			
			if(pSI->cGAMEINDEX==Q4_SERVERLIST) //Is it Quake 4?
			{	
				if(strlen(packet)>0)
					player->szClanTag = _strdup(packet);
				packet+=strlen(packet)+1;

				if(player->szClanTag!=NULL)
					if((player->rate==50000) && (strcmp("[BOT]",player->szClanTag)==0))
						pSI->cBots++;

			} 
			else if(pSI->cGAMEINDEX==ETQW_SERVERLIST)//Otherwise go for ETQW
			{

				packet++; //Skip Clan tag position info
				if(strlen(packet)>0)
					player->szClanTag = _strdup(packet);						
				packet+=strlen(packet)+1;

				if((char)packet[0] == 0x01)
				{
					player->szTeam = _strdup("BOT");
					pSI->cBots++;
				}
				else
					player->szTeam = _strdup("PLAYER");

				packet++;			
			}
			player->pServerInfo = pSI;
			if(pQ4Players==NULL)
				pQ4Players = pQ4CurrentPlayer = player;
			else 
				pQ4CurrentPlayer = pQ4CurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;

			if(pSI->cGAMEINDEX==ETQW_SERVERLIST) //ETQW specific
			{	
				if(packet[5]==0x01)
				{
					pSI->cRanked = 1;
					ServerRule_Add(pSI->pServerRules,"ranked","1");

				//	packet++; 4+2
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

			if (packet[0]==0x20)  //ETQW & Q4
				break;
		}
		
	}
	return pQ4Players;
}
