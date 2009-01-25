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



DWORD UT_Get_ServerStatus(SERVER_INFO *pSI,long (*Callback_UpdatePlayerListView)(PLAYERDATA *pPlayers),long (*Callback_UpdateRulesListView)(SERVER_RULES *pServerRules))
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
		char szShortName[4];
		fnIPtoCountry(pSI->dwIP,szShortName);
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
	dbg_print("1. Request challenge %X",iReq.dwSequence );
	packetlen = send(pSocket, (const char*)&iReq,sizeof(iReq), 0);
		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return -1;
	}

	packet=(unsigned char*)getpacket(pSocket, &packetlen);

	if((packet==NULL))
	{

		dbg_print("Error at getpacket()");
		if(dwRetries<AppCFG.dwRetries)
		{
			dwRetries++;
			if(packet!=NULL)
				free(packet);
			goto port_Step;
		}
	}
	else
	{
		OutputDebugString((LPCSTR)packet);
		OutputDebugString("\n");
	}


	if(packet)
	{
		fr = (first_response *)packet;
		second_request sr;
		sr.init[0] = 0xFE;
		sr.init[1] = 0xFD;
		sr.request = 0;
		sr.dwSequence = GetTickCount()+1000;
		
		
		dbg_print("1b Response challenge %X",fr->dwSequence);

		dbg_print("2a. Requist new challenge %d",fr->data);
		dbg_print("2a. request new timestamp %d",sr.dwSequence);

		long challenge = atol((const char*)&fr->data);
		

	/*	if(challenge<0)
		{
			dbg_print("- negative -");
			challenge = challenge - 4294967296;
		}
*/
		DWORD chal = challenge;

		sr.bChallenge[0] = (BYTE)(challenge >> 24);
		sr.bChallenge[1] = (BYTE)(challenge >> 16);
		sr.bChallenge[2] = (BYTE)(challenge >> 8);
		sr.bChallenge[3] = (BYTE)(challenge >> 0);

		dbg_print("2b. Response new final converted challenge %X%X%X%X",sr.bChallenge[0],sr.bChallenge[2],sr.bChallenge[2],sr.bChallenge[3]);
		dbg_print("2c. Response new converted challenge %X",challenge);

		sr.end[0] = 0xFF;
		sr.end[1] = 0xFF;
		sr.end[2] = 0xFF;
		sr.end[3] = 0x01;
	
		free(packet);
		packet = NULL;

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
			DWORD numPlayers=0;
			pSI->dwPing = (GetTickCount() - dwStartTick);

			server_info_response *sir = (server_info_response *)packet;
			dbg_print("3a.Response challenge %2.2X %2.2X %2.2X %2.2X %2.2X",packet[0],packet[1],packet[2],packet[3],packet[4]);
			dbg_print("3b.Response challenge %X",ntohl(sir->dwSequence));
			char *currentData = (char*)&sir->data;
			char *endOfData = &currentData[packetlen];

			currentData = UT_ParseServerRules(pSI->pServerRules,(char*)&sir->data,packetlen);
			pSI->pPlayerData = UT_ParsePlayers(pSI,currentData,endOfData, &numPlayers);

			Set_RuleStr(pSI->pServerRules,"hostname", pSI->szServerName,sizeof(pSI->szServerName)-1);
			int port;
			Set_RuleInt(pSI->pServerRules,"hostport", (int*)&port);
			pSI->usPort = (unsigned short)port;
			Set_RuleStr(pSI->pServerRules,"gamemode", pSI->szGameTypeName,sizeof(pSI->szGameTypeName)-1);
			Set_RuleStr(pSI->pServerRules,"p1073741825", pSI->szMap,sizeof(pSI->szMap)-1);
			Set_RuleInt(pSI->pServerRules,"p268435703", (int*)&pSI->cBots);	
			Set_RuleInt(pSI->pServerRules,"numplayers", (int*)&pSI->nCurrentPlayers);
			Set_RuleInt(pSI->pServerRules,"maxplayers", (int*)&pSI->nMaxPlayers);

			int npub = 0;
			Set_RuleInt(pSI->pServerRules,"NumPublicConnections", (int*)&npub);

			pSI->nPrivateClients = pSI->nMaxPlayers - npub;
			pSI->nMaxPlayers-= pSI->nPrivateClients;

			pSI->bNeedToUpdateServerInfo = false;
			pSI->bUpdated = true;

			free(packet);
			packet=NULL;
			
			if(Callback_UpdatePlayerListView!=NULL)
				Callback_UpdatePlayerListView(pSI->pPlayerData);

			if(Callback_UpdateRulesListView!=NULL)
				Callback_UpdateRulesListView(pSI->pServerRules);

			if(UT_Callback_CheckForBuddy!=NULL)
				UT_Callback_CheckForBuddy(pSI->pPlayerData,pSI);

			if(pSI->pServerRules!=NULL)
				CleanUp_ServerRules(pSI->pServerRules);
			pSI->pServerRules = NULL;
		}
			
	}
	closesocket(pSocket);
	return 0;

}
/*


0x046DCCD8  70 32 36 38 34 33 35 37 30 33 00 31 31 00 70 31 30 37 33 37 34 31 38 32 37 00 00 70 32 36 38 34 33 35 37 31 37 00 30 00  p268435703.11.p1073741827..p268435717.0.
0x046DCD00  70 31 30 37 33 37 34 31 38 32 38 70 6c 61 79 65 72 5f 00 00 6c 31 67 68 74 6d 79 66 31 72 65 00 75 74 33 5f  p1073741828
00 00 00 01 ....
player_..l1ghtmyf1re.ut3_
0x046DCD28  6b 6f 73 74 61 73 00 62 61 7a 69 6c 69 63 31 33 00 6b 6f 6c 74 37 38 00 42 69 4f 7c 53 77 65 65 70 65 72 00 4b 79 6c 72  kostas.bazilic13.kolt78.BiO|Sweeper.Kylr
0x046DCD50  69 61 00 69 4e 46 41 55 53 54 00 5b 4d 42 4d 5d 5e 50 49 54 00 4d 2e 44 2e 41 6d 61 6e 6f 00 6e 69 6e 6a 61 6b 69 64 39  ia.iNFAUST.[MBM]^PIT.M.D.Amano.ninjakid9
0x046DCD78  00 54 68 61 64 6f 00 62 6c 61 63 6b 30 31 34 00 43 6f 6d 70 65 78 31 37 37 00 6a 6f 6e 74 65 6d 61 6e 00 62 65 6c 65 78  .Thado.black014.Compex177.jonteman.belex
0x046DCDA0  65 73 00 51 52 5a 42 52 41 61 00 41 73 73 61 73 73 69 6e 64 75 63 6b 00 4d 61 74 68 65 6f 54 58 00 00 73 63 6f 72 65 5f  es.QRZBRAa.Assassinduck.MatheoTX..score_
0x046DCDC8  00 00 34 33 00 32 31 00 38 00 32 39 00 39 00 31 32 00 32 39 00 31 36 00 37 39 00 32 00 30 00 31 37 00 32 39 00 35 30 00  ..43.21.8.29.9.12.29.16.79.2.0.17.29.50.
0x046DCDF0  34 00 33 37 00 32 00 37 00 00 70 69 6e 67 5f 00 00 36 34 00 31 33 36 00 37 36 00 34 38 00 31 32 30 00 34 30 00 36 38 00  4.37.2.7..ping_..64.136.76.48.120.40.68.
0x046DCE18  37 32 00 39 32 00 31 31 32 00 36 34 00 32 32 30 00 37 36 00 37 32 00 31 33 32 00 31 32 38 00 36 34 00 36 38 00 00 74 65  72.92.112.64.220.76.72.132.128.64.68..te
0x046DCE40  61 6d 5f 00 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00  am_..Team.Team.Team.Team.Team.Team.Team.
0x046DCE68  54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00  Team.Team.Team.Team.Team.Team.Team.Team.
0x046DCE90  54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 00 64 65 61 74 68 73 5f 00 00 32 31 00 31 37 00 32 37 00 32 35 00 32 38 00  Team.Team.Team..deaths_..21.17.27.25.28.
0x046DCEB8  00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 3d 56 75 2a 08 6c 0a 00 30 57 28 02 38 65 6d 04  .¤¤¤¤лллллллл...........=Vu*.l..0W(.8em.


0x02DBBDC8  00 20 8f 3e 02 73 70 6c 69 74 6e 75 6d 00 80 00 68 6f 73 74 6e 61 6d 65 00 45 70 69 63 20 4d 69 64 77 61 79 20 57 61 72  . .>.splitnum.А.hostname.Epic Midway War
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

0x046DDF98  6f 6e 74 65 6e 74 00 70 32 36 38 34 33 35 37 30 34 00 31 30 00 70 32 36 38 34 33 35 37 30 35 00 32 35 00 70 32 36 38 34  ontent.p268435704.10.p268435705.25.p2684
0x046DDFC0  33 35 37 30 33 00 37 00 70 31 30 37 33 37 34 31 38 32 37 00 00 70 32 36 38 34 33 35 37 31 37 00 30 00 70 31 30 37 33 37  35703.7.p1073741827..p268435717.0.p10737
0x046DDFE8  34 31 38 32 38 00 00 00 01 70 6c 61 79 65 72 5f 00 00 67 65 6b 74 6f 72 31 33 30 38 00 77 70 65 63 6b 65 72 00 6c 75 63  41828....player_..gektor1308.wpecker.luc
0x046DE010  6b 79 6c 75 6b 65 32 30 00 62 61 72 6e 79 2e 63 72 79 00 78 6f 6f 70 73 78 00 73 74 61 6c 6b 65 72 33 32 33 33 73 74 61  kyluke20.barny.cry.xoopsx.stalker3233sta
0x046DE038  6c 00 6d 6f 72 61 6e 5f 39 33 00 57 40 52 46 52 45 45 4b 00 50 69 6e 6b 6c 65 00 56 49 4b 49 4e 47 2d 53 4c 00 53 79 6c  l.moran_93.W@RFREEK.Pinkle.VIKING-SL.Syl
0x046DE060  61 72 39 30 00 6d 65 6e 6b 39 39 00 58 42 6c 6f 6f 64 72 61 7a 6f 72 00 41 74 69 6e 71 00 76 6f 6c 74 61 67 65 35 32 36  ar90.menk99.XBloodrazor.Atinq.voltage526
0x046DE088  00 57 6f 6c 66 73 62 6c 75 74 34 33 00 62 6c 61 63 6b 73 74 61 72 62 67 00 59 61 69 73 6d 61 74 5b 47 45 52 5d 00 5a 65  .Wolfsblut43.blackstarbg.Yaismat[GER].Ze
0x046DE0B0  72 67 6c 69 6e 67 65 72 00 43 79 72 69 6c 6c 75 73 00 00 73 63 6f 72 65 5f 00 00 36 32 00 31 39 00 31 35 00 32 30 00 34  rglinger.Cyrillus..score_..62.19.15.20.4
0x046DE0D8  36 00 35 36 00 31 33 00 36 00 30 00 30 00 34 00 37 00 36 00 37 00 38 00 32 33 00 34 00 39 00 31 00 30 00 00 70 69 6e 67  6.56.13.6.0.0.4.7.6.7.8.23.4.9.1.0..ping
0x046DE100  5f 00 00 31 31 32 00 33 37 32 00 31 33 36 00 31 33 32 00 31 33 32 00 32 34 34 00 32 35 32 00 31 34 30 00 32 34 38 00 31  _..112.372.136.132.132.244.252.140.248.1
0x046DE128  37 32 00 39 38 38 00 31 31 32 00 32 30 34 00 31 36 30 00 32 34 34 00 31 32 34 00 31 36 30 00 32 30 34 00 31 36 30 00 30  72.988.112.204.160.244.124.160.204.160.0
0x046DE150  00 00 74 65 61 6d 5f 00 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 54 65 61 6d 00 00  ..team_..Team.Team.Team.Team.Team.Team..
0x046DE178  00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 74 ff fe 51 1a 14 0a 19 e0 db 6d 04 00 00 00 00  .¤¤¤¤лллллллл...........t ■Q....р█m.....
0x046DE1A0  ec d4 cf 00 7b 03 00 00 2b 01 00 00 01 00 00 00 12 37 01 00 fd fd fd fd 00 4e 11 09 00 73 70 6c 69 74 6e 75 6d 00 81 01  ь╘╧.{...+.......

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

		if((packet[0]==0) && (packet[1]==1)) // && (packet[2]==0) && (packet[4]==0x01))
			break;

	}
	packet+=2;
	return packet;
}

PLAYERDATA *UT_ParsePlayers(SERVER_INFO *pSI,char *packet,char *end, DWORD *numPlayers)
{
	PLAYERDATA *pPlayers=NULL;
	PLAYERDATA *pCurrentPlayer=NULL;
	if(memcmp(packet,"player_",strlen("player_"))==0)
	{


		packet+=strlen("player_")+2;
		if(packet[0]==0)
			return NULL;

		while(packet<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory
				return pPlayers;
			player->pNext = NULL;							
			player->szClanTag = NULL;	
			player->szTeam = NULL;	
			player->cGAMEINDEX = pSI->cGAMEINDEX;
			player->dwServerIndex = pSI->dwIndex;


			player->szPlayerName = _strdup(packet);
			packet+=(int)strlen(packet)+1;

			if(pPlayers==NULL)
				pPlayers = pCurrentPlayer = player;
			else 
				pCurrentPlayer = pCurrentPlayer->pNext = player;


			*numPlayers = *numPlayers + 1;			
			if(packet[0]==0)
				break;
		}
	}
	return pPlayers;
}