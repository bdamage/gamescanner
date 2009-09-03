

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

struct WOLF_RESPONSE_SERVERINFO
{
	char leadData[31]; //ÿÿservers
	// 1  2  3  4  5  6  7 8  9  10 11 12 13                   20 21 22 23 24 25          29 30 31
	//ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 ff ff ff ff 12 00 1e 00 8c 03 00 00  //WOLF
	char data[1];
	
};


void Q4_SetCallbacks(long (*UpdateServerListView)(DWORD index),
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q4_UpdateServerListView = UpdateServerListView;
	Q4_Callback_CheckForBuddy = _Callback_CheckForBuddy;
	Q4_InsertServerItem = InsertServerItem;
}


/*
Wolf beta 
-----------
0x0A2DE878  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 ff ff ff ff 12 00 1e 00 cb 03 00 00 73 69 5f 6e 61 6d 65 00 46 6c 61 6d 65 47 75 61 72 64  ÿÿinfoResponse.....ÿÿÿÿ....Ë...si_name.FlameGuard

0x030AD010  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 ff ff ff ff 12 00 1e 00 2d 04 00 00 73 69 5f 6e 61 6d 65 00 46 6c 61 6d 65 47 75 61 72 64  ÿÿinfoResponse.....ÿÿÿÿ....-...si_name.FlameGuard

0x0A2E0188  ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 00 01 00 00 00 ff ff ff ff 12 00 1e 00 8c 03 00 00 73 69 5f 6e 61 6d 65 00 59 43 4e 20 48 6f 73 74 69 6e  ÿÿinfoResponse.....ÿÿÿÿ....Œ...si_name.YCN Hostin


*/
char *Q4_ParseServerRules(SERVER_INFO* pSI,SERVER_RULES* &pLinkedListStart,char *packet,DWORD packetlen)
{
	SERVER_RULES *pSR=NULL;
	SERVER_RULES *pCurrentSR=NULL;
	Q4DATA_SERVER_INFO *Q4SI; 
	ETQWDATA_RESPONSE_SERVERINFO *ETQWResponse;
	WOLF_RESPONSE_SERVERINFO *WOLFResponse;
	
	Q4SI = (Q4DATA_SERVER_INFO *)packet;
	ETQWResponse  =(ETQWDATA_RESPONSE_SERVERINFO *)packet;
	WOLFResponse  =(WOLF_RESPONSE_SERVERINFO *)packet;
	pSR=NULL;

	char *pointer = NULL;
	if(pSI->cGAMEINDEX==ETQW_SERVERLIST)
		pointer = ETQWResponse->data;
	else if(pSI->cGAMEINDEX==WOLF_SERVERLIST)
		pointer = WOLFResponse->data;
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

	SOCKET pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usPort); 
 
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
/*
Wolf beta

0x034331B8  5f 61 6e 74 69 4c 61 67 4f 6e 6c 79 00 30 00 73 69 5f 61 6e 74 69 4c 61 67 00 31 00 62 6f 74 5f 65 6e 61 62 6c 65 00 30 00 67 61 6d 65 6e 61 6d 65  _antiLagOnly.0.si_antiLag.1.bot_enable.0.gamename
0x034331E9  00 62 61 73 65 57 6f 6c 66 65 6e 73 74 65 69 6e 2d 31 00 73 69 5f 6d 61 70 00 6d 61 70 73 2f 6d 70 5f 6d 61 6e 6f 72 2e 65 6e 74 69 74 69 65 73 00  .baseWolfenstein-1.si_map.maps/mp_manor.entities.
                  #  PING  RATE   ? ?  Ply name    ?   ? ?  
0x0343321A  00 00 00 2b 00 80 3e 00 00 54 73 75 00 00 00 00 01 40 00 80 3e 00 00 47 61 72 79 42 6f 75 72 62 69 65 72 00 00 00 00 02 35 00 80 3e 00 00 62 69 6c  ...+.€>..Tsu.....@.€>..GaryBourbier.....5.€>..bil
0x0343324B  62 6f 65 65 65 00 00 00 00 03 32 00 80 3e 00 00 62 6f 6f 79 61 68 00 00 00 00 04 36 00 80 3e 00 00 4d 75 63 6b 61 21 00 00 00 00 05 28 00 80 3e 00  boeee.....2.€>..booyah.....6.€>..Mucka!.....(.€>.
0x0343327C  00 63 33 70 65 67 34 00 00 00 00 06 48 00 80 3e 00 00 61 7a 69 72 00 00 00 00 08 07 00 80 3e 00 00 4a 6f 6e 6e 79 00 00 00 00 09 6b 00 80 3e 00 00  .c3peg4.....H.€>..azir.......€>..Jonny.....k.€>..
0x034332AD  41 6e 75 62 69 53 00 00 00 00 0a 50 00 80 3e 00 00 4a 75 6d 2d 4a 75 6d 00 00 00 00 0b 4c 00 80 3e 00 00 5e 37 49 6e 66 61 6e 5e 33 74 5e 37 6f 72  AnubiS.....P.€>..Jum-Jum.....L.€>..^7Infan^3t^7or
0x034332DE  00 00 5e 33 65 6e 69 67 6d 61 5e 39 2e 5e 30 00 00 0c 50 00 80 3e 00 00 76 69 70 65 72 65 6b 00 00 00 00 0d 3a 00 80 3e 00 00 5e 32 73 74 61 6d 69  ..^3enigma^9.^0...P.€>..viperek.....:.€>..^2stami
0x0343330F  6e 61 5e 37 62 6f 79 00 00 00 00 0e 20 01 80 3e 00 00 62 6f 74 61 00 00 00 00 0f 3b 00 80 3e 00 00 7a 69 47 45 52 00 00 00 00 10 01 00 00 00 00 77  na^7boy..... .€>..bota.....;.€>..ziGER..........w
0x03433340  f1 06 00 02 00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 f3 54 31 69 fb 3a 1d 18 78 13 28 03 b0 33 43 03 10  ñ....ýýýý««««««««.

Wolf beta
 # PING  RATE  ?   ? Player name                            ?  Clan tag                         ?
03 1e 00 80 3e 00 00 57 61 6b 69 7a 61 73 68 69 5e 33 21 00 00 5e 30 61 72 74 5e 34 2e 5e 30 00 00 ...€>..Wakizashi^3!..^0art^4.^0..



1 BYTE Ply index
2 BYTE Ping
2 BYTE Rate
2 BYTE Unknown
n BYTES String name
1 BYTE NULL string terminator
1 BYTE Unknown
n BYTES Clan tag
1 BYTE NULL string terminator
1 BYTE Unknown

*/

struct serverInfo_t {
  int osMask;
  byte isRanked;
  int timeLeft;
  byte gameState;
 };


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
	} else 	if ((packet[0]==16) && (pSI->cGAMEINDEX == WOLF_SERVERLIST)) 
	{
		*numPlayers = 0;
		return pQ4Players;
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
			if((pSI->cGAMEINDEX==Q4_SERVERLIST) || (pSI->cGAMEINDEX==DOOM3_SERVERLIST)  || (pSI->cGAMEINDEX==WOLF_SERVERLIST) )
			{
				rate = (WORD*)&packet[0];
				player->rate  = *rate; 
				packet+=2; //rate
				packet+=2; //unknown bytes
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
			} else if(pSI->cGAMEINDEX==WOLF_SERVERLIST)
			{

				packet++; //unknown byte

				if(strlen(packet)>0)
					player->szClanTag = _strdup(packet);

				packet+=strlen(packet)+1;

				packet++; //unknown byte
			}
			player->pServerInfo = pSI;
			if(pQ4Players==NULL)
				pQ4Players = pQ4CurrentPlayer = player;
			else 
				pQ4CurrentPlayer = pQ4CurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;

			if(pSI->cGAMEINDEX==WOLF_SERVERLIST || (pSI->cGAMEINDEX==WOLF_SERVERLIST)) //ETQW specific
			{	
				serverInfo_t *serverInfo = (serverInfo_t *)packet;

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
				switch(serverInfo->gameState)
				{
					case 1 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","WARMUP");	break;
					case 2 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","Game in progress");	break;
					case 4 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","Reviewing Score");	break;
					case 8 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","STOPWATCH SECOND PLAYOFF");	break;

				}

			}

			if (packet[0]==0x20)  //ETQW & Q4
				break;
			if ((packet[0]==16) && packet[2]==0 && packet[3]==0 && (pSI->cGAMEINDEX == WOLF_SERVERLIST)) 
			{
				serverInfo_t *serverInfo = (serverInfo_t *)&packet[1];
				switch(serverInfo->gameState)
				{
					case 1 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","WARMUP");	break;
					case 2 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","Game in progress");	break;
					case 4 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","Reviewing Score");	break;
					case 8 :	pSI->szSTATUS = ServerRule_Add(pSI->pServerRules,"gs.status","STOPWATCH SECOND PLAYOFF");	break;
				}

				
				break;
			}
		}
		
	}
	return pQ4Players;
}
/*  OS MASK---| R |timeleft--| 
 10 01 00 00 00 00 23 f7 06 00 02 00
 10 01 00 00 00 00 5a d6 02 00 02 00 
 10 01 00 00 00 00 5a d6 02 00 02 00  1:36
 10 01 00 00 00 00 5d 36 00 00 02 00 0:21
 10 01 00 00 00 00 00 00 00 00 04 00 Review score
 10 01 00 00 00 00 c4 14 00 00 01 00 14:41
 
 10 01 00 00 00 00 00 00 00 00 01 00 Review Score

 10 01 00 00 00 00 a8 83 0a 00 02 00 11:47

 10 01 00 00 00 00 a8 83 0a 00 02 00
*/