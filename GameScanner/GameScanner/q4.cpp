

#include "stdafx.h"
#include "q4.h"
#include "utilz.h"
#include "..\..\iptocountry\iptocountry.h"

#define _SECURE_SCL 0
#pragma warning(disable : 4995)

char Q4_unkown[]={"????"};
bool Q4_bCloseApp = false;

long (*Q4_UpdateServerListView)(DWORD index);
long (*Q4_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);
long (*Q4_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);

DWORD Q4_dwTotalServers=0;
DWORD Q4_dwNewTotalServers=0;
extern HWND g_hwndProgressBar;
extern APP_SETTINGS_NEW AppCFG;
extern CLanguage g_lang;
BOOL Q4_bScanningInProgress = FALSE;

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



DWORD Q4_ConnectToMasterServer(GAME_INFO *pGI, int nMasterIdx)
{
	Q4_bScanningInProgress = TRUE;	
	size_t packetlen=0;
	SOCKET ConnectSocket;
	char sendbuf[40];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	
	int len = 0;
	len = UTILZ_ConvertEscapeCodes(pGI->szMasterQueryString,sendbuf,sizeof(sendbuf));

	dbg_print("Master server %s:%d",pGI->szMasterServerIP[0],(unsigned short)pGI->dwMasterServerPORT);


	ConnectSocket = getsockudp(pGI->szMasterServerIP[0],(unsigned short)pGI->dwMasterServerPORT);
   
	if(INVALID_SOCKET==ConnectSocket)
	{
		Q4_bScanningInProgress = FALSE;
		dbg_print("Error connecting to socket!");
		return 1;
	}
	dbg_print("Sending command %s Len: %d",sendbuf,len);
	if(send(ConnectSocket, sendbuf, len, 0)==SOCKET_ERROR) 
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

	// Let's retrieve all packets as fast as possible, that's the reason for splitting up the for loop.
	for(i=0; i<iMAX_PACKETS;i++)
	{
		packet[i] = NULL;
		packet[i]=(unsigned char*)getpacket(ConnectSocket, &packetlen);
		if(packet[i]==NULL) //End of transfer
			break;  	
	}
	//Now parse all server IP's from recieved packets
	for(i=0; i<iMAX_PACKETS;i++)
	{
		if(packet[i]==NULL) //End of packets
			break;  	
		Q4_ParseServers((char*)packet[i],packetlen,pGI,Q4_InsertServerItem);
		SetStatusText(pGI->iIconIndex,g_lang.GetString("StatusReceivingMaster"),Q4_dwNewTotalServers,pGI->szGAME_NAME);		
		free(packet[i]);
	}
	
	closesocket(ConnectSocket);
	pGI->dwTotalServers = pGI->vSI.size();
	Q4_bScanningInProgress = FALSE;

	return 	0;
}



SERVER_INFO* Q4_ParseServers(char * p, DWORD length,  GAME_INFO *pGI,long (*InsertServerListView)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q4DATA *q4d;
	SERVER_INFO tempSI;
	DWORD *dwIP=NULL;
	DWORD idx = pGI->vSI.size();

	if (p==NULL)
		return NULL;

	q4d = (Q4DATA*)p;
	char *end = p+length;

	p = q4d->data;

	ZeroMemory(&tempSI,sizeof(SERVER_INFO));		
	tempSI.pPlayerData = NULL;
	tempSI.pServerRules = NULL;
	strcpy(tempSI.szShortCountryName,"zz");
	tempSI.cGAMEINDEX =  pGI->cGAMEINDEX;
	tempSI.bNeedToUpdateServerInfo = 1;
	tempSI.dwPing = 9999;

	int hash = 0;
	while(p<end) 
	{	
		dwIP = (DWORD*)&p[0];
		tempSI.dwIP = ntohl((DWORD)*dwIP); 
		tempSI.usPort  = ((p[5])<<8);
		tempSI.usPort |=(unsigned char)(p[4]);
	
		 hash = tempSI.dwIP + tempSI.usPort;
		
		if(UTILZ_checkforduplicates(pGI,  hash,tempSI.dwIP, tempSI.usPort)==FALSE)	
		{		
			sprintf_s(tempSI.szIPaddress,sizeof(tempSI.szIPaddress),"%d.%d.%d.%d",(unsigned char)p[0],(unsigned char)p[1],(unsigned char)p[2],(unsigned char)p[3]);
			tempSI.dwIndex = idx++;
			pGI->shash.insert(Int_Pair(hash,tempSI.dwIndex));
			pGI->vSI.push_back(tempSI);
			Q4_dwNewTotalServers++;
		} //end serverexsist
		p+=6;			
		Q4_dwTotalServers++;

	} //end while

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
		CleanUp_PlayerList(pSI->pPlayerData);
	pSI->pPlayerData = NULL;

	if(pSI->pServerRules!=NULL)
		CleanUp_ServerRules(pSI->pServerRules);
	pSI->pServerRules = NULL;

	SOCKET pSocket;
	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
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
		fnIPtoCountry(pSI->dwIP,szShortName);
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
/*
Doom 3 Responeses
0x02E7AB30  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 29 00 01 00 73 76 5f 70 75 6e 6b 62 75 73 74 65 72 00 30 00 73  ÿÿinfoResponse.....)...sv_punkbuster.0.s
0x02E7AB58  69 5f 76 65 72 73 69 6f 6e 00 44 4f 4f 4d 20 31 2e 33 2e 31 2e 31 33 30 34 20 6c 69 6e 75 78 2d 78 38 36 20 4a 61 6e 20  i_version.DOOM 1.3.1.1304 linux-x86 Jan 
0x02E7AB80  31 36 20 32 30 30 37 20 32 31 3a 35 33 3a 32 39 00 66 73 5f 67 61 6d 65 5f 62 61 73 65 00 00 66 73 5f 67 61 6d 65 00 00  16 2007 21:53:29.fs_game_base..fs_game..
0x02E7ABA8  73 69 5f 69 64 6c 65 53 65 72 76 65 72 00 30 00 6e 65 74 5f 73 65 72 76 65 72 44 65 64 69 63 61 74 65 64 00 31 00 73 69  si_idleServer.0.net_serverDedicated.1.si

0x02ECC6C8  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 55 00 02 00 6e 65 74 5f 73 65 72 76 65 72 4d 61 78 43 6c 69 65  ÿÿinfoResponse.....U...net_serverMaxClie
0x02ECC6F0  6e 74 52 61 74 65 00 32 35 36 30 30 00 6e 65 74 5f 73 65 72 76 65 72 53 6e 61 70 73 68 6f 74 44 65 6c 61 79 00 33 30 00  ntRate.25600.net_serverSnapshotDelay.30.
0x02ECC718  6e 65 74 5f 73 65 72 76 65 72 44 65 64 69 63 61 74 65 64 00 31 00 66 73 5f 67 61 6d 65 5f 62 61 73 65 00 00 66 73 5f 67  net_serverDedicated.1.fs_game_base..fs_g
0x02ECC740  61 6d 65 00 71 34 6d 61 78 00 73 76 5f 70 75 6e 6b 62 75 73 74 65 72 00 30 00 73 69 5f 76 65 72 73 69 6f 6e 00 51 75 61  ame.q4max.sv_punkbuster.0.s

Quake 4 Responses
0x03772A98  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 55 00 02 00 6e 65 74 5f 73 65 72 76 65 72 4d 61 78 43 6c 69 65  ÿÿinfoResponse.....U...net_serverMaxClie
0x03772AC0  6e 74 52 61 74 65 00 32 35 36 30 30 00 6e 65 74 5f 73 65 72 76 65 72 53 6e 61 70 73 68 6f 74 44 65 6c 61 79 00 33 30 00  ntRate.25600.net_serverSnapshotDelay.30.
0x03772AE8  6e 65 74 5f 73 65 72 76 65 72 44 65 64 69 63 61 74 65 64 00 31 00 66 73 5f 67 61 6d 65 5f 62 61 73 65 00 00 66 73 5f 67  net_serverDedicated.1.fs_game_base..fs_g
0x03772B10  61 6d 65 00 71 34 6d 70 00 73 76 5f 70 75 6e 6b 62 75 73 74 65 72 00 30 00 73 69 5f 76 65 72 73 69 6f 6e 00 51 75 61 6b  ame.q4mp.sv_punkbuster.0.si_version.Quak
0x03772B38  65 34 20 20 56 31 2e 34 2e 32 20 77 69 6e 2d 78 38 36 20 4a 75 6e 20 31 35 20 32 30 30 37 00 73 69 5f 6d 61 78 50 6c 61  e4  V1.4.2 

0x03772A98  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 55 00 02 00 6e 65 74 5f 73 65 72 76 65 72 4d 61 78 43 6c 69 65  ÿÿinfoResponse.....U...net_serverMaxClie
0x03772AC0  6e 74 52 61 74 65 00 32 35 36 30 30 00 6e 65 74 5f 73 65 72 76 65 72 53 6e 61 70 73 68 6f 74 44 65 6c 61 79 00 33 30 00  ntRate.25600.net_serverSnapshotDelay.30.
0x03772AE8  6e 65 74 5f 73 65 72 76 65 72 44 65 64 69 63 61 74 65 64 00 31 00 66 73 5f 67 61 6d 65 5f 62 61 73 65 00 00 66 73 5f 67  net_serverDedicated.1.fs_game_base..fs_g
0x03772B10  61 6d 65 00 71 34 6d 70 00 73 76 5f 70 75 6e 6b 62 75 73 74 65 72 00 30 00 73 69 5f 76 65 72 73 69 6f 6e 00 51 75 61 6b  ame.q4mp.sv_punkbuster.0.si_version.Quak
0x03772B38  65 34 20 20 56 31 2e 34 2e 32 20 77 69 6e 2d 78 38 36 20 4a 75 6e 20 31 35 20 32 30 30 37 00 73 69 5f 6d 61 78 50 6c 61  e4  V1.4.2 

*/
	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);

		//dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data
		pCurrPointer = Q4_ParseServerRules(pSI,pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		
			char *szVarValue = NULL;
			szVarValue = Get_RuleValue("si_version",pServRules);
			if(szVarValue!=NULL)
				strncpy(pSI->szVersion, szVarValue,49);	

			if(Get_RuleValue("si_name",pServRules)!=NULL)
				strncpy(pSI->szServerName, Get_RuleValue("si_name",pServRules),99);

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
			pSI->bNeedToUpdateServerInfo = 0;
			pSI->bUpdated = true;
			pSI->nCurrentPlayers = nPlayers;
			
			
			if(Get_RuleValue("si_name",pServRules)!=NULL)
				strncpy(pSI->szServerName, Get_RuleValue("si_name",pServRules),99);
			if(Get_RuleValue("si_map",pServRules)!=NULL)
				strncpy(pSI->szMap, Get_RuleValue("si_map",pServRules),39);

			if(Get_RuleValue("fs_game",pServRules)!=NULL)
				strncpy(pSI->szFS_GAME, Get_RuleValue("fs_game",pServRules),39);

			if(Get_RuleValue("gamename",pServRules)!=NULL)
			{
				strncpy(pSI->szMod, Get_RuleValue("gamename",pServRules),24);
				pSI->dwMod = Get_ModByName(pSI->cGAMEINDEX, pSI->szMod);
			}

		
			szVarValue = Get_RuleValue("si_gametype",pServRules);
			if(szVarValue!=NULL)
			{	
				strncpy(pSI->szGameTypeName, szVarValue,29);		
			}
			else
			{
				szVarValue = Get_RuleValue("si_rules",pServRules);
				if(szVarValue!=NULL)
				{		
					if(strlen(szVarValue)>11)
						strncpy_s(pSI->szGameTypeName,sizeof(pSI->szGameTypeName), &szVarValue[strlen("sdGameRules")],14);	//ETQW
					else
						strcpy_s(pSI->szGameTypeName,sizeof(pSI->szGameTypeName),"Unknown");					
				}

			}
			
			pSI->dwGameType = Get_GameTypeByName(pSI->cGAMEINDEX, pSI->szGameTypeName);

			szVarValue = Get_RuleValue("si_pure",pServRules);
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

			if(pSI->szVersion!=NULL)
				pSI->dwVersion =  Get_FilterVersionByVersionString(pSI->cGAMEINDEX,pSI->szVersion);

			if(Get_RuleValue("net_serverPunkbusterEnabled",pServRules)!=NULL)
				pSI->bPunkbuster = (char)atoi(Get_RuleValue("net_serverPunkbusterEnabled",pServRules));
			else if(Get_RuleValue("sv_punkbuster",pServRules)!=NULL)
				pSI->bPunkbuster = (char)atoi(Get_RuleValue("sv_punkbuster",pServRules));
			
			if(Get_RuleValue("si_usepass",pServRules)!=NULL)
				pSI->bPrivate = (char)atoi(Get_RuleValue("si_usepass",pServRules));				
			else if(Get_RuleValue("si_needPass",pServRules)!=NULL)  //ETQW
				pSI->bPrivate = (char)atoi(Get_RuleValue("si_needPass",pServRules));
			
			if(Get_RuleValue("si_maxPlayers",pServRules)!=NULL)
				pSI->nMaxPlayers = atoi(Get_RuleValue("si_maxPlayers",pServRules));

			if(Get_RuleValue("si_tv",pServRules)!=NULL)
				pSI->bTV = atoi(Get_RuleValue("si_tv",pServRules));		
			
			if(Get_RuleValue("si_privatePlayers",pServRules)!=NULL)
				pSI->nPrivateClients = atoi(Get_RuleValue("si_privatePlayers",pServRules));
			else if(Get_RuleValue("si_privateClients",pServRules)!=NULL) 			//ETQW
			{
				pSI->nPrivateClients = atoi(Get_RuleValue("si_privateClients",pServRules));
				pSI->nMaxPlayers -=	pSI->nPrivateClients;
			}

			CleanUp_PlayerList(pQ4Players);
			pSI->pPlayerData = NULL;

			CleanUp_ServerRules(pServRules);
			pSI->pServerRules = NULL;

		} //end if(pServRules!=NULL)

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;

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
	//if(packet[0]==0 || packet[0]==1)
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

			if(pQ4Players==NULL)
				pQ4Players = pQ4CurrentPlayer = player;
			else 
				pQ4CurrentPlayer = pQ4CurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;

			if(pSI->cGAMEINDEX==ETQW_SERVERLIST) //ETQW specific
			{	
				if(packet[5]==0x01)
				{
					/*dbg_print("Found ranked server!\n");
					dbg_print(pSI->szServerName);
					dbg_print("\n");*/
					pSI->cRanked = 1;
				
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
