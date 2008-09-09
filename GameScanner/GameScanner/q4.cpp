

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
extern CLanguage lang;
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
	if(pSI->cGAMEINDEX==Q4_SERVERLIST)
		pointer =Q4SI->data;
	else
		pointer = ETQWResponse->data;

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
//	if(g_bQ4)

	pointer+=2; //move to playerlist data (Q4)


	return pointer;
}

#define iMAX_PACKETS 50



DWORD Q4_ConnectToMasterServer(GAME_INFO *pGI)
{
	Q4_bScanningInProgress = TRUE;
	
	size_t packetlen=0;

	//WSADATA wsaData;
	SOCKET ConnectSocket;
	char sendbuf[30];// = {"\xFF\xFFgetServers\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"}; 
	ZeroMemory(sendbuf,sizeof(sendbuf));
	
	int len = 0;//(int)strlen(sendbuf);
	len = UTILZ_ConvertEscapeCodes(pGI->szMasterQueryString,sendbuf,sizeof(sendbuf));

	dbg_print("Master server %s:%d",pGI->szMasterServerIP,(unsigned short)pGI->dwMasterServerPORT);


	ConnectSocket = getsockudp(pGI->szMasterServerIP,(unsigned short)pGI->dwMasterServerPORT);
   
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

	for(i=0; i<iMAX_PACKETS;i++)
	{
		packet[i] = NULL;

		packet[i]=(unsigned char*)getpacket(ConnectSocket, &packetlen);


		if(packet[i]==NULL) 
		{

			dbg_print("Could NOT receive all packets!\n");
			break;  	
		} 
		SetStatusText(pGI->iIconIndex,lang.GetString("StatusReceivingMaster"),Q4_dwNewTotalServers,pGI->szGAME_NAME);		
		Q4_parseServers((char*)packet[i],packetlen,pGI,Q4_InsertServerItem);
		free(packet[i]);
	
	}

	closesocket(ConnectSocket);

	//Global Q4 serverlist to start
	//Q4_pSI = pSI = Q4_pSIServerListStart;	

	pGI->dwTotalServers = pGI->vSI.size();

	//Q4_InitilizeRescan(pGI);
		
	Q4_bScanningInProgress = FALSE;

	return 	0;
}



SERVER_INFO* Q4_parseServers(char * p, DWORD length,  GAME_INFO *pGI,long (*InsertServerListView)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q4DATA *q4d;
	q4d = (Q4DATA*)p;

	SERVER_INFO tempSI;
	DWORD idx = pGI->vSI.size();

	DWORD *dwIP=NULL;

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
//			tempSI.cCountryFlag = 0;
			tempSI.bNeedToUpdateServerInfo = true;
			tempSI.dwIndex = idx++;
			tempSI.pPlayerData = NULL;
			strcpy(tempSI.szShortCountryName,"zz");
			tempSI.pServerRules = NULL;
			pGI->vSI.push_back(tempSI);

			/*if(InsertServerListView!=NULL)
				InsertServerListView(pGI,tempSI);
			*/

			Q4_dwNewTotalServers++;
		} //end serverexsist

		
		Q4_dwTotalServers++;

	} //end while

	return NULL;
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
	    char country[60],szShortName[8];
		//ZeroMemory(szShortName,sizeof(szShortName));
		strncpy_s(pSI->szCountry,sizeof(pSI->szCountry),fnIPtoCountry2(pSI->dwIP,country,szShortName),49);  //Update country info only when adding a new server						
		strcpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName);
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
				pSI->dwMod = Get_ModByName(pSI->cGAMEINDEX, pSI->szMod);
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
			
			if(Q4_Get_RuleValue("si_maxPlayers",pServRules)!=NULL)
				pSI->nMaxPlayers = atoi(Q4_Get_RuleValue("si_maxPlayers",pServRules));
			
			
			if(Q4_Get_RuleValue("si_privatePlayers",pServRules)!=NULL)
				pSI->nPrivateClients = atoi(Q4_Get_RuleValue("si_privatePlayers",pServRules));
			else if(Q4_Get_RuleValue("si_privateClients",pServRules)!=NULL) 			//ETQW
			{
				pSI->nPrivateClients = atoi(Q4_Get_RuleValue("si_privateClients",pServRules));
				pSI->nMaxPlayers -=	pSI->nPrivateClients;
			}

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



void Q4_OnServerSelection(SERVER_INFO* pServerInfo,long (*UpdatePlayerListView)(PLAYERDATA *pPlayers),long (*UpdateRulesList)(SERVER_RULES*pServer_Rules) )
{
	if(pServerInfo==NULL)
		return;

	Q4_Get_ServerStatus(pServerInfo,UpdatePlayerListView,UpdateRulesList);

	//if(Q4_UpdateServerListView!=NULL)
 //	Q4_UpdateServerListView(pServerInfo->dwIndex);
}
/*
0x035437F2  32 3a 35 38 3a 31 37 00 73 69 5f 70 75 72 65 00 31 00 67 61 6d 65 6e 61 6d 65 00 62 61 73 65 44 4f 4f 4d 2d 31 00 00 00  2:58:17.si_pure.1.gamename.baseDOOM-1...
            id 
0x0354381A  00 a2 00 80 3e 00 00 5e 34 28 28 28 5e 32 47 49 5e 37 4f 20 4b 5e 31 41 4e 5e 34 29 29 29 00 01 72 00 80 3e 00 00 5e 37  .¢.€>..^4(((^2GI^7O K^1AN^4)))..r.€>..^7
0x03543842  46 5e 31 74 5e 37 50 7c 5e 31 46 40 53 54 5e 37 42 45 4e 4e 00 02 21 00 80 3e 00 00 5e 32 4d 6f 67 77 61 69 20 5e 33 7b  F^1t^7P|^1F@ST^7BENN..!.€>..^2Mogwai ^3{
0x0354386A  5e 31 4b 41 4e 5e 33 7d 00 03 72 00 80 3e 00 00 5e 39 4f 4c 44 5e 31 4d 49 43 48 4c 5e 33 7b 4b 41 4e 7d 00 04 72 00 b0  ^1KAN^3}..r.€>..^9OLD^1MICHL^3{KAN}..r.°
0x03543892  36 00 00 5e 33 54 5e 31 6f 5e 33 6d 20 5e 31 6f 5e 33 6e 20 5e 31 46 5e 33 69 5e 31 72 5e 33 65 5e 38 20 7b 4b 41 4e 7d  6..^3T^1o^3m ^1o^3n ^1F^3i^1r^3e^8 {KAN}
0x035438BA  00 05 3f 00 80 3e 00 00 5e 30 4a 77 61 79 6e 65 38 39 5e 39 7b 4b 41 4e 7d 00 20 07 00 00 00 00 fd fd fd fd ab ab ab ab  ..?.€>..^0Jwayne89^9{KAN}. .....ý
*/


PLAYERDATA *Q4_ParsePlayers(SERVER_INFO *pSI,char *packet,char *end, DWORD *numPlayers)
{
	
	PLAYERDATA *pQ4Players=NULL;
//	if(pSI->cGAMEINDEX==ETQW_SERVERLIST)
//		packet+=2;

	pSI->cBots = 0;
	if(packet[0]==0x20)
	{
		if(packet[5]==0x01)
		{
			pSI->cRanked = 1;   //ETQW
		}
	}

	if(packet[0]==0 || packet[0]==1)
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
			
			if(pSI->cGAMEINDEX==Q4_SERVERLIST) //Is it Quake 4
			{	
				if(strlen(packet)>0)
					player->szClanTag = _strdup(packet);
				packet+=strlen(packet)+1;
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
		
		if(pPL->szPlayerName!=NULL)
			free(pPL->szPlayerName);
		if(pPL->szClanTag!=NULL)
			free(pPL->szClanTag);
		if(pPL->szTeam!=NULL)
			free(pPL->szTeam);
		
		pPL->szPlayerName = NULL;
		pPL->szClanTag = NULL;
		pPL->szTeam = NULL;
		pPL->pNext = NULL;
		free(pPL);
		pPL = NULL;
	}
}
