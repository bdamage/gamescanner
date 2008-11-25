#define _SECURE_SCL 0

#include "stdafx.h"
#include "unreal.h"
#include "utilz.h"
#include "scanner.h"
#include "..\..\iptocountry\iptocountry.h"


extern CLanguage g_lang;
extern bool g_bCancel;
extern GamesMap GamesInfo;
extern APP_SETTINGS_NEW AppCFG;

long (*UT_UpdateServerListView)(DWORD index);
long (*UT_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo)=NULL;
long (*UT_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);

//comments is expected static data
struct init_request
{
	BYTE req_string[3]; //FE FD 09
	DWORD dwSequence;
};

struct first_response
{
	BYTE reply_identification; //0x09	
	DWORD dwSequence;	
	BYTE data[12];
};

struct second_request
{
	BYTE init[2]; //FE FD	
	BYTE request; //00 = server info
	DWORD dwSequence;
	BYTE bChallenge[4];
	BYTE end[4]; //FF FF FF 01
};


struct server_info_response
{
	BYTE reply_identification; //0x00	
	DWORD dwSequence;	
	BYTE junk[11];
	BYTE data;
};

DWORD UT_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *pPlayers),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick;

	dwStartTick=0;	
	if(pSI==NULL)
	{
		dbg_print("Invalid SERVER_INFO argument @ UT_Get_ServerStatus!\n");
		return (DWORD)0xFFFFFFF;
	}

	if(pSI->pPlayerData!=NULL)
		CleanUp_PlayerList(pSI->pPlayerData);
	pSI->pPlayerData = NULL;

	if(pSI->pServerRules!=NULL)
		CleanUp_ServerRules(pSI->pServerRules);
	pSI->pServerRules = NULL;

		size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{
		char country[60],szShortName[4];
		strncpy(pSI->szCountry,fnIPtoCountry2(pSI->dwIP,country,szShortName),49);  //Update country info only when adding a new server		
		strcpy(pSI->szShortCountryName,szShortName);
	}

	DWORD dwRetries=0;



	int idxPortStep=0;
	unsigned short port = pSI->usQueryPort;//pSI->usPort;//6500;
port_Step:
	

	pSocket =  getsockudp(pSI->szIPaddress ,port); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  idxPortStep++;
	  port+=100;
	  dbg_print("Error at getsockudp()\n");
	  if(idxPortStep>5)
		return 0xFFFFFF;
	  else
		  goto port_Step;

	}

//http://wiki.unrealadmin.org/index.php?title=UT3_query_protocol
	init_request iReq;
	first_response *fr;

	iReq.req_string[0] = 0xFE;
	iReq.req_string[1] = 0xFD;
	iReq.req_string[2] = 0x09;
	iReq.dwSequence = GetTickCount();
retry:
	packetlen = send(pSocket, (const char*)&iReq,sizeof(iReq), 0);
		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return -1;
	}
	dwStartTick = GetTickCount();

	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if((packet==NULL) || packetlen<5)
	{
		if(dwRetries<AppCFG.dwRetries)
		{
			dwRetries++;
			goto port_Step;
		}
	}


	if(packet)
	{
		fr = (first_response *)packet;
		second_request sr;
		sr.init[0] = 0xFE;
		sr.init[1] = 0xFD;
		sr.request = 0;
		sr.dwSequence = GetTickCount();
		
		dbg_print("Response challenge %d \n",fr->dwSequence);

		long challenge = atol((const char*)&fr->data);
		if(challenge<0)
			challenge = challenge - 4294967296;

		DWORD chal = challenge;

		sr.bChallenge[0] = (challenge >> 24);
		sr.bChallenge[1] = (challenge >> 16);
		sr.bChallenge[2] = (challenge >> 8);
		sr.bChallenge[3] = (challenge >> 0);


		sr.end[0] = 0xFF;
		sr.end[1] = 0xFF;
		sr.end[2] = 0xFF;
		sr.end[3] = 0x01;
	
		free(packet);

		dwStartTick = GetTickCount();

		packetlen = send(pSocket, (const char*)&sr,sizeof(second_request), 0);
			
		if(packetlen==SOCKET_ERROR) 
		{
			dbg_print("Error at send() second_request\n");
			closesocket(pSocket);		
			return -1;
		}
		
		packetlen = 0;
		packet=(unsigned char*)getpacket(pSocket, &packetlen);
		if(packet!=NULL)
		{
			pSI->dwPing = (GetTickCount() - dwStartTick);

			server_info_response *sir = (server_info_response *)packet;
			sir->data;
			UT_ParseServerRules(pSI->pServerRules,(char*)&sir->data,packetlen);

			char *pValue = Get_RuleValue("hostname",pSI->pServerRules); 
			if(pValue!=NULL)
				strncpy(pSI->szServerName,pValue ,sizeof(pSI->szServerName)-1);

			pValue = Get_RuleValue("hostport",pSI->pServerRules); 
			if(pValue!=NULL)
				pSI->usPort = atoi(pValue);

			pValue = Get_RuleValue("maxplayers",pSI->pServerRules); 
			if(pValue!=NULL)
				pSI->nMaxPlayers = atoi(pValue);

			pValue = Get_RuleValue("numplayers",pSI->pServerRules); 
			if(pValue!=NULL)
				pSI->nCurrentPlayers = atoi(pValue);

			int npub = 0;

			pValue = Get_RuleValue("NumPublicConnections",pSI->pServerRules); 
			if(pValue!=NULL)
				npub = atoi(pValue);

			pSI->nPrivateClients = pSI->nMaxPlayers - npub;
			pSI->nMaxPlayers-= pSI->nPrivateClients;

			pValue = Get_RuleValue("gamemode",pSI->pServerRules); 
			if(pValue!=NULL)
				strncpy(pSI->szGameTypeName,pValue ,sizeof(pSI->szGameTypeName)-1);

			pValue = Get_RuleValue("p1073741825",pSI->pServerRules); 
			if(pValue!=NULL)
				strncpy(pSI->szMap,pValue ,sizeof(pSI->szMap)-1);

			pValue = Get_RuleValue("p268435703",pSI->pServerRules); 
			if(pValue!=NULL)
				pSI->cBots = atoi(pValue);			
	
			pSI->bNeedToUpdateServerInfo = false;
			pSI->bUpdated = true;

			free(packet);
		}
	
		
	}


	closesocket(pSocket);
	return 0;

}
/*
0x02DBBDC8  00 20 8f 3e 02 73 70 6c 69 74 6e 75 6d 00 80 00 68 6f 73 74 6e 61 6d 65 00 45 70 69 63 20 4d 69 64 77 61 79 20 57 61 72  . .>.splitnum.€.hostname.Epic Midway War
0x02DBBDF0  66 61 72 65 20 23 31 35 20 28 31 36 20 50 6c 61 79 65 72 29 00 68 6f 73 74 70 6f 72 74 00 37 38 37 37 00 6e 75 6d 70 6c  fare #15 (16 Player).hostport.7877.numpl
0x02DBBE18  61 79 65 72 73 00 30 00 6d 61 78 70 6c 61 79 65 72 73 00 31 36 00 67 61 6d 65 6d 6f 64 65 00 6f 70 65 6e 70 6c 61 79 69  ayers.0.maxplayers.16.gamemode.openplayi
0x02DBBE40  6e 67 00 6d 61 70 6e 61 6d 65 00 4f 77 6e 69 6e 67 50 6c 61 79 65 72 49 64 3d 31 31 35 39 36 34 34 33 38 2c 4e 75 6d 50  ng.mapname.OwningPlayerId=115964438,NumP
0x02DBBE68  75 62 6c 69 63 43 6f 6e 6e 65 63 74 69 6f 6e 73 3d 31 36 2c 62 55 73 65 73 53 74 61 74 73 3d 54 72 75 65 2c 62 49 73 44  ublicConnections=16,bUsesStats=True,bIsD
0x02DBBE90  65 64 69 63 61 74 65 64 3d 54 72 75 65 2c 4f 77 6e 69 6e 67 50 6c 61 79 65 72 4e 61 6d 65 3d 45 70 69 63 20 4d 69 64 77  edicated=True,OwningPlayerName=Epic Midw
0x02DBBEB8  61 79 20 57 61 72 66 61 72 65 20 23 31 35 20 28 31 36 20 50 6c 61 79 65 72 29 2c 41 76 65 72 61 67 65 53 6b 69 6c 6c 52  ay Warfare #15 (16 Player),AverageSkillR
0x02DBBEE0  61 74 69 6e 67 3d 31 30 30 30 2e 30 30 30 30 30 30 2c 45 6e 67 69 6e 65 56 65 72 73 69 6f 6e 3d 33 36 31 34 2c 4d 69 6e  ating=1000.000000,EngineVersion=3614,Min
0x02DBBF08  4e 65 74 56 65 72 73 69 6f 6e 3d 33 34 36 37 2c 73 33 32 37 37 39 3d 32 2c 73 30 3d 35 2c 73 31 3d 30 2c 73 36 3d 31 2c  NetVersion=3467,s32779=2,s0=5,s1=0,s6=1,
0x02DBBF30  73 37 3d 30 2c 73 38 3d 30 2c 73 39 3d 30 2c 73 31 30 3d 30 2c 73 31 31 3d 30 2c 73 31 32 3d 30 2c 73 31 33 3d 31 2c 73  s7=0,s8=0,s9=0,s10=0,s11=0,s12=0,s13=1,s
0x02DBBF58  31 34 3d 31 2c 70 31 30 37 33 37 34 31 38 32 35 3d 57 41 52 2d 44 6f 77 6e 74 6f 77 6e 2c 70 31 30 37 33 37 34 31 38 32  14=1,p1073741825=WAR-Downtown,p107374182
0x02DBBF80  36 3d 55 54 47 61 6d 65 43 6f 6e 74 65 6e 74 2e 55 54 4f 6e 73 6c 61 75 67 68 74 47 61 6d 65 5f 43 6f 6e 74 65 6e 74 2c  6=UTGameContent.UTOnslaughtGame_Content,
0x02DBBFA8  70 32 36 38 34 33 35 37 30 34 3d 35 2c 70 32 36 38 34 33 35 37 30 35 3d 32 30 2c 70 32 36 38 34 33 35 37 30 33 3d 33 2c  p268435704=5,p268435705=20,p268435703=3,
0x02DBBFD0  70 31 30 37 33 37 34 31 38 32 37 3d 2c 70 32 36 38 34 33 35 37 31 37 3d 30 2c 70 31 30 37 33 37 34 31 38 32 38 3d 52 33  p1073741827=,p268435717=0,p1073741828=R3
0x02DBBFF8  50 4f 52 54 45 52 1c 53 65 72 76 65 72 20 41 64 76 65 72 74 69 73 65 6d 65 6e 74 73 20 28 50 72 6f 67 72 65 73 73 69 76  PORTER.Server Advertisements (Progressiv
0x02DBC020  65 29 00 4f 77 6e 69 6e 67 50 6c 61 79 65 72 49 64 00 31 31 35 39 36 34 34 33 38 00 4e 75 6d 50 75 62 6c 69 63 43 6f 6e  e).OwningPlayerId.115964438.NumPublicCon
0x02DBC048  6e 65 63 74 69 6f 6e 73 00 31 36 00 62 55 73 65 73 53 74 61 74 73 00 54 72 75 65 00 62 49 73 44 65 64 69 63 61 74 65 64  nections.16.bUsesStats.True.bIsDedicated
0x02DBC070  00 54 72 75 65 00 4f 77 6e 69 6e 67 50 6c 61 79 65 72 4e 61 6d 65 00 45 70 69 63 20 4d 69 64 77 61 79 20 57 61 72 66 61  .True.OwningPlayerName.Epic Midway Warfa
0x02DBC098  72 65 20 23 31 35 20 28 31 36 20 50 6c 61 79 65 72 29 00 41 76 65 72 61 67 65 53 6b 69 6c 6c 52 61 74 69 6e 67 00 31 30  re #15 (16 Player).AverageSkillRating.10
0x02DBC0C0  30 30 2e 30 30 30 30 30 30 00 45 6e 67 69 6e 65 56 65 72 73 69 6f 6e 00 33 36 31 34 00 4d 69 6e 4e 65 74 56 65 72 73 69  00.000000.EngineVersion.3614.MinNetVersi
0x02DBC0E8  6f 6e 00 33 34 36 37 00 73 33 32 37 37 39 00 32 00 73 30 00 35 00 73 31 00 30 00 73 36 00 31 00 73 37 00 30 00 73 38 00  on.3467.s32779.2.s0.5.s1.0.s6.1.s7.0.s8.
0x02DBC110  30 00 73 39 00 30 00 73 31 30 00 30 00 73 31 31 00 30 00 73 31 32 00 30 00 73 31 33 00 31 00 73 31 34 00 31 00 70 31 30  0.s9.0.s10.0.s11.0.s12.0.s13.1.s14.1.p10
0x02DBC138  37 33 37 34 31 38 32 35 00 57 41 52 2d 44 6f 77 6e 74 6f 77 6e 00 70 31 30 37 33 37 34 31 38 32 36 00 55 54 47 61 6d 65  73741825.WAR-Downtown.p1073741826.UTGame
0x02DBC160  43 6f 6e 74 65 6e 74 2e 55 54 4f 6e 73 6c 61 75 67 68 74 47 61 6d 65 5f 43 6f 6e 74 65 6e 74 00 70 32 36 38 34 33 35 37  Content.UTOnslaughtGame_Content.p2684357
0x02DBC188  30 34 00 35 00 70 32 36 38 34 33 35 37 30 35 00 32 30 00 70 32 36 38 34 33 35 37 30 33 00 33 00 70 31 30 37 33 37 34 31  04.5.p268435705.20.p268435703.3.p1073741
0x02DBC1B0  38 32 37 00 00 70 32 36 38 34 33 35 37 31 37 00 30 00 70 31 30 37 33 37 34 31 38 32 38 00 52 33 50 4f 52 54 45 52 1c 53  827..p268435717.0.p1073741828.R3PORTER.S
0x02DBC1D8  65 72 76 65 72 20 41 64 76 65 72 74 69 73 65 6d 65 6e 74 73 20 28 50 72 6f 67 72 65 73 73 69 76 65 29 00 00 01 70 6c 61  erver Advertisements (Progressive)...pla
0x02DBC200  79 65 72 5f 00 00 00 73 63 6f 72 65 5f 00 00 00 70 69 6e 67 5f 00 00 00 74 65 61 6d 5f 00 00 00 64 65 61 74 68 73 5f 00  yer_...score_...ping_...team_...deaths_.
0x02DBC228  00 00 70 69 64 5f 00 00 00 00 02 74 65 61 6d 5f 74 00 00 54 65 61 6d 00 54 65 61 6d 00 00 73 63 6f 72 65 5f 74 00 00 32  ..pid_.....team_t..Team.Team..score_t..2
0x02DBC250  00 34 00 00 00 00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 00 00 4a 81 f0 84 d9 e2 1c 00  .4....
*/


char *UT_ParseServerRules(SERVER_RULES* &pLinkedListStart,char *packet,DWORD packetlen)
{
	SERVER_RULES *pSR=NULL;
	SERVER_RULES *pCurrentSR=NULL;
	pLinkedListStart = NULL;

	char *end = &packet[packetlen];
	while(packet<end)
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
		
	
		pCurrentSR->name = _strdup(packet);
		packet+=(int)strlen(packet)+1;

		pCurrentSR->value = _strdup(packet);
		packet+=strlen(packet)+1;			

	}
	return packet;
}
