#define _SECURE_SCL 0

#include "stdafx.h"
#include "q3.h"
#include "utilz.h"
#include "scanner.h"
#include "..\..\iptocountry\iptocountry.h"

#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

#define MAX_PACKETS 250


HWND hwndLVserverlist=NULL;

char Q3_unkown[]={"????"};

extern bool g_bCancel;
extern GAME_INFO GI[MAX_SERVERLIST+1];

bool Q3_bCloseApp=false;

long (*Q3_UpdateServerListView)(DWORD index);
//bool (*Q3_FilterServerItem)(LPARAM *lp,DWORD dwFilterFlags);
long (*Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);
long (*Q3_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);

DWORD Q3_dwThreadCounter=0;
DWORD Q3_dwTotalServers=0;
DWORD Q3_dwNewTotalServers=0;
BOOL Q3_bScanningInProgress = FALSE;


TCHAR szPlyType[5][12] = {TEXT("Connecting"), TEXT("Axis"), TEXT("Allies"), TEXT("Spectator"),TEXT("Unknown")};


void Q3_SetCallbacks(long (*UpdateServerListView)(DWORD index), 
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q3_UpdateServerListView = UpdateServerListView;
	Callback_CheckForBuddy = _Callback_CheckForBuddy;
	Q3_InsertServerItem = InsertServerItem;
}


char *Q3_Get_RuleValue(char *szRuleName,SERVER_RULES *pSR)
{
	while(pSR!=NULL)
	{
		if(_stricmp(pSR->name,szRuleName)==0)
			return pSR->value;
		pSR = pSR->pNext;

	}
	return NULL;//Q3_unkown;
}


DWORD Q3_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick;

	dwStartTick=0;	
	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return -1;
	}

	if(pSI->pPlayerData!=NULL)
		UTILZ_CleanUp_PlayerList(pSI->pPlayerData);
	pSI->pPlayerData = NULL;

	if(pSI->pServerRules!=NULL)
		UTILZ_CleanUp_ServerRules(pSI->pServerRules);
	pSI->pServerRules = NULL;


	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->dwPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return -1;
	}

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
		{
			DWORD dwIPSHORT;
			char country[60],szShortName[4];
			ZeroMemory(szShortName,sizeof(szShortName));
		//	dwStartTick = GetTickCount();
			strncpy(pSI->szCountry,fnIPtoCountry2(pSI->dwIP,&dwIPSHORT,country,szShortName),49);  //Update country info only when adding a new server		
			strcpy(pSI->szShortCountryName,szShortName);
		//	sprintf(country,"IPGeo %d ms %s\n",(GetTickCount() - dwStartTick),pSI->szCountry);
		//	dbg_print(country);				
			pSI->cCountryFlag = 1;

		}
	
	if(GI[pSI->cGAMEINDEX].szServerRequestInfo!=NULL)
		packetlen = send(pSocket, GI[pSI->cGAMEINDEX].szServerRequestInfo, strlen(GI[pSI->cGAMEINDEX].szServerRequestInfo)+1, 0);
	else
		packetlen=SOCKET_ERROR;

		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return -1;
	}
	dwStartTick = GetTickCount();
	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);
		//dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data

		pCurrPointer = Q3_ParseServerRules(pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		
		
			char szP_ET[150];
			if(pSI->cGAMEINDEX == ET_SERVERLIST)
			{			
				ZeroMemory(&szP_ET,sizeof(szP_ET));			
				char *szPVarValue=NULL;
				szPVarValue = Q3_Get_RuleValue("P",pServRules);
				if(szPVarValue!=NULL)
					strcpy(szP_ET,szPVarValue);
			}


			PLAYERDATA *pQ3Players=NULL;
			DWORD nPlayers=0;
			//---------------------------------
			//Retrieve players if any exsist...
			//---------------------------------
			switch(pSI->cGAMEINDEX)
			{
				case QW_SERVERLIST:
				case Q2_SERVERLIST:
					pQ3Players = QW_ParsePlayers(pSI,pCurrPointer,end,&nPlayers);
					break;
				default:
					pQ3Players = Q3_ParsePlayers(pSI,pCurrPointer,end,&nPlayers,szP_ET);
				break;
			}
			
	
			pSI->pPlayerData = pQ3Players;
			if(pQ3Players!=NULL)
			{
				if(UpdatePlayerListView!=NULL)
					UpdatePlayerListView(pQ3Players);
				
			//	Callback_CheckForBuddy(pQ3Players,pSI);
			}
			if(UpdateRulesListView!=NULL)
				UpdateRulesListView(pServRules);
			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------
			pSI->bNeedToUpdateServerInfo = false;
			pSI->bUpdated = true;
			pSI->nCurrentPlayers = nPlayers;
			char *szVarValue=NULL;
			char *pVarValue = NULL;
			pVarValue = Q3_Get_RuleValue("sv_hostname",pServRules);
			if(pVarValue!=NULL)
			{
				strncpy(pSI->szServerName,pVarValue ,99);
			}
			else  //QW
			{
				pVarValue = Q3_Get_RuleValue("hostname",pServRules);
				if(pVarValue!=NULL)
					strncpy(pSI->szServerName,pVarValue ,99);


			}

	

			//getting status value
			switch(pSI->cGAMEINDEX)
			{	
				case QW_SERVERLIST:
					{
						pVarValue = Q3_Get_RuleValue("status",pServRules); //QW
						if(pVarValue!=NULL)
								strncpy(pSI->szSTATUS,pVarValue ,39);
					}
				break;
				case Q2_SERVERLIST:
					{
						pVarValue = Q3_Get_RuleValue("time_remaining",pServRules); //Q2
						if(pVarValue!=NULL)
								strncpy(pSI->szSTATUS,pVarValue ,39);
						else
						{
							pVarValue = Q3_Get_RuleValue("#time_left",pServRules); //Q2
							if(pVarValue!=NULL)
									strncpy(pSI->szSTATUS,pVarValue ,39);
							else
							{
								pVarValue = Q3_Get_RuleValue("gamestats",pServRules); //Q2
								if(pVarValue!=NULL)
									strncpy(pSI->szSTATUS,pVarValue ,39);

							}
						}
					}
				break;
			}

			pVarValue = Q3_Get_RuleValue("mapname",pServRules);
			if(pVarValue!=NULL)
				strncpy(pSI->szMap,pVarValue ,39);
			else
			{ //for QW
				pVarValue = Q3_Get_RuleValue("map",pServRules);
				if(pVarValue!=NULL)
					strncpy(pSI->szMap,pVarValue ,39);

			}
			
			pSI->dwMap = Get_MapByName(pSI->cGAMEINDEX, pVarValue);


			switch(pSI->cGAMEINDEX)	 //MODS
			{

				case COD4_SERVERLIST :
					{
						pVarValue = Q3_Get_RuleValue("mod",pServRules);			
						if(pVarValue!=NULL)
						{
							pSI->wMod = 1;
							if(strcmp(pVarValue,"1")==0)
							{	
								char *mod;
								mod = Q3_Get_RuleValue("fs_game",pServRules);
								if(mod!=NULL)
								{
									strncpy(pSI->szMod, mod,MAX_MODNAME_LEN-1);									
								}
							}
						}
						break;
					}
					default:
					{
						//Fall through and do some guessing...
						pVarValue = Q3_Get_RuleValue("gamename",pServRules);			
						if(pVarValue==NULL)
						{
							pVarValue = Q3_Get_RuleValue("*gamedir",pServRules); //Normal QW
							if(pVarValue==NULL)							
							{
								pVarValue = Q3_Get_RuleValue("gamename",pServRules); //Normal Q2
								if(pVarValue==NULL)
									pVarValue = Q3_Get_RuleValue("*progs",pServRules); //Is it QW with Qizmo proxy
								if(pVarValue!=NULL)
									if(strcmp(pVarValue,"666")==0)
										pVarValue="Qizmo";
									else
										pVarValue=NULL;
							}
						}
						
						if(pVarValue!=NULL)                   
							strncpy(pSI->szMod, pVarValue,MAX_MODNAME_LEN-1);
						
					}
			}
			pSI->wMod = Get_ModByName(pSI->cGAMEINDEX,pSI->szMod);
			
			switch(pSI->cGAMEINDEX)
			{			
				case COD_SERVERLIST :
				case COD2_SERVERLIST :
				case COD4_SERVERLIST :
					{
						szVarValue = Q3_Get_RuleValue("pswrd",pServRules);  //CoD & Cod2
						if(szVarValue!=NULL)
							pSI->bPrivate = atoi(szVarValue);

						szVarValue = Q3_Get_RuleValue("shortversion",pServRules);
						if(szVarValue!=NULL)
						{
							ZeroMemory(pSI->szVersion,sizeof(pSI->szVersion));
							strncpy(pSI->szVersion,szVarValue,49);
						}
					}		
				break;
				case WARSOW_SERVERLIST:
					{
						szVarValue = Q3_Get_RuleValue("bots",pServRules); //Warsow specific
						if(szVarValue!=NULL)
							pSI->cBots = atoi(szVarValue);
					}
				case ET_SERVERLIST:
					{
						szVarValue = Q3_Get_RuleValue("omnibot_enable",pServRules); //ET specific
						if(szVarValue!=NULL)
							pSI->cBots = atoi(szVarValue);
					} //Fall through and continue on default...
				default:
					{						
						szVarValue = Q3_Get_RuleValue("g_needpass",pServRules);
						if(szVarValue!=NULL)
							pSI->bPrivate = (char)atoi(szVarValue);
						else
						{
							szVarValue = Q3_Get_RuleValue("needpass",pServRules);
							if(szVarValue!=NULL)
							{
								pSI->bPrivate = (char)atoi(szVarValue);
								
								if(pSI->bPrivate==4) //Quake World fix
									pSI->bPrivate = 0;

							}
						}
						ZeroMemory(pSI->szVersion,sizeof(pSI->szVersion));
						szVarValue = Q3_Get_RuleValue("version",pServRules);
						if(szVarValue==NULL)
							szVarValue = Q3_Get_RuleValue("*version",pServRules); // QuakeWorld
						
						if(szVarValue!=NULL)
							strncpy(pSI->szVersion,szVarValue,MAX_VERSION_LEN-1);
					}
					break;
			}
			if(pSI->szVersion!=NULL)
				pSI->dwVersion =  Get_FilterVersionByVersionString(pSI->cGAMEINDEX,pSI->szVersion);

			szVarValue = Q3_Get_RuleValue("sv_pure",pServRules);
			if(szVarValue!=NULL)
				pSI->cPure = atoi(szVarValue);
			
			szVarValue = Q3_Get_RuleValue("g_gametype",pServRules);
			pSI->dwGameType = Get_GameTypeByName(pSI->cGAMEINDEX, szVarValue);

			szVarValue = Q3_Get_RuleValue("sv_punkbuster",pServRules);
			if(szVarValue==NULL)
				szVarValue = Q3_Get_RuleValue("sv_battleye",pServRules); //Warsow
			
			if(szVarValue!=NULL)
				pSI->bPunkbuster = (char)atoi(szVarValue);


			szVarValue = Q3_Get_RuleValue("sv_privateClients",pServRules);
			if(szVarValue!=NULL)
				pSI->nPrivateClients = atoi(szVarValue);

			szVarValue = Q3_Get_RuleValue("sv_maxclients",pServRules);
			if(szVarValue!=NULL)
				pSI->nMaxPlayers = atoi(szVarValue)-pSI->nPrivateClients;
			else
			{ //for QW
				szVarValue = Q3_Get_RuleValue("maxclients",pServRules);
				if(szVarValue!=NULL)
					pSI->nMaxPlayers = atoi(szVarValue)-pSI->nPrivateClients;
			}


			if(Callback_CheckForBuddy!=NULL)
				Callback_CheckForBuddy(pQ3Players,pSI);
			//Debug purpose
			if(pServRules!=pSI->pServerRules)
			{
				AddLogInfo(ETSV_ERROR,"Error at pServRules!=pSI->pServerRules");
				DebugBreak();
			} 
			else
			{
				UTILZ_CleanUp_ServerRules(pSI->pServerRules);
				pSI->pServerRules = NULL;	
			}

			UTILZ_CleanUp_PlayerList(pQ3Players);
			pSI->pPlayerData = NULL;
			

		} //end if(pServRules!=NULL)

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;   //increase purge counter when the server is not responding

	closesocket(pSocket);
	return 0;
}


void Q3_OnServerSelection(SERVER_INFO* pServerInfo,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesList)(SERVER_RULES*pServer_Rules) )
{
	if(pServerInfo==NULL)
		return;

	Q3_Get_ServerStatus(pServerInfo,UpdatePlayerListView,UpdateRulesList);

	if(Q3_UpdateServerListView!=NULL)
		Q3_UpdateServerListView(pServerInfo->dwIndex);
}



SERVER_INFO* Q3_parseServers(char * p, DWORD length, GAME_INFO *pGI)
{
	Q3DATA *Q3d;
	Q3d = (Q3DATA*)p;

	int i=0;
	SERVER_INFO ptempSI;
	DWORD idx = pGI->pSC->vSI.size();	
	DWORD *dwIP=NULL;
	DWORD dwResult=0;
	if (p==NULL)
		return NULL;

	char *end;
	end = p+length-10;
/*
Quake 2 master server response
0x011F0BBB  01 00 00 00 00 00 00 00 00 00 00 00 00 40 27 00 00 01 00 00 00 39 e7 00 00 fd fd fd fd ff ff ff ff 73 65 72 76 65 72 73 20  .............@'......9ç..ýýýýÿÿÿÿservers 
0x011F0BE4  45 09 a8 04 6d 19 d0 2b 0f c7 6d 06 26 67 08 62 6d 0c 26 67 08 62 6d 0b c1 6e 7a d7 6d 06 48 e8 e4 ba 6d 1c d9 aa 42 53 6d  E.¨.m.Ð+.Çm.&g.bm.&g.bm.Ánz×m.Hèäºm.ÙªBSm
            1  2   3  4  5 6  1  2                                                                                                      23456123456123456

0x01ECB9B8  ff ff ff ff 73 65 72 76 65 72 73 20 45 09 a8 04 6d 19 d0 2b 0f c7 6d 06 26 67 08 62 6d 0c 26 67 08 62 6d 0b c1 6e 7a d7 6d  ÿÿÿÿservers E.¨.m.Ð+.Çm.&g.bm.&g.bm.Ánz×m
0x01ECB9E1  06 48 e8 e4 ba 6d 1c d9 aa 42 53 6d 06 d9 aa 42 53 6d 10 c3 7a d9 13 6d 24 cb ce 5f 01 6d 01 cb ce 5f 01 6d 0c cb ce 5f 01  .Hèäºm.ÙªBSm.ÙªBSm.ÃzÙ.m$ËÎ_.m.ËÎ_.m.ËÎ_.
0x01ECBA0A  6d 10 cb ce 5f 01 6d 24 cb ce 5f 01 6d 0d cb ce 5f 01 6d 0b cb ce 5f 01 6d 0e cb ce 5f 01 6d 02 cb ce 5f 01 6d 07 cb ce 5f  m.ËÎ_.m$ËÎ_.m.ËÎ_.m.ËÎ_.m.ËÎ_.m.ËÎ_.m.ËÎ_
0x01ECBA33  01 6d 0a cb ce 5f 01 6d 08 cb ce 5f 01 6d 09 cb ce 5f 01 6d 1a c3 7a d9 13 6d 2e c1 bd a0 22 6d 06 45 71 70 d0 6d 06 42 80  .m.ËÎ_.m.ËÎ_.m.ËÎ_.m.ÃzÙ.m.Á. "m.EqpÐm.B€
0x01ECBA5C  32 de 6d 06 26 67 08 62 6d 09 26 67 08 62 6d 10 c3 bd 53 23 6d 06 c3 88 fa dc 6d 07 c3 88 fa dc 6d 0a c8 b1 e5 f8 6d 07 c8  2Þm.&g.bm.&g.bm.Ã.S#m.ÃˆúÜm.ÃˆúÜm.È±åøm.È
0x01ECBA85  62 fa 0f 6d 06 d4 48 73 f8 6d 10 4c 5d 98 47 6d 06 45 40 24 f2 6d 06 c8 b1 e5 f8 6d 06 62 e3 c1 73 6d 06 18 16 a8 99 6d 06  bú.m.ÔHsøm.L]˜Gm.E@$òm.È±åøm.bãÁsm...¨™m.
0x01ECBAAE  d9 73 9b 78 6d 08 d9 73 9b 78 6d 06 d9 73 9b 78 6d 07 d9 73 9b 78 6d 09 d9 73 9b 78 6d 0a 47 6d 78 f3 6d 07 4a 36 ba e2 6d  Ùs.xm.Ùs.xm.Ùs.xm.Ùs.xm.Ùs.xm.Gmxóm.J6ºâm
0x01ECBAD7  0a 4a 36 ba e2 6d 1a 4a 36 ba e2 6c 12 4a 36 ba e2 6d 1b 4a 36 ba e2 6d 1c 4a 36 ba e2 6d 0e 4a 36 ba e2 6d 0c 4a 36 ba e2  .J6ºâm.J6ºâl.J6ºâm.J6ºâm.J6ºâm.J6ºâm.J6ºâ
0x01ECBB00  6d 06 4a 36 ba e2 6d 0d 4a 36 ba e2 6d 56 46 56 2c 9a 6d 07 4a 36 ba e2 6d 11 4a 36 ba e2 6d 10 46 56 2c 9a 6d 06 c2 f7 c0  m.J6ºâm.J6ºâmVFV,šm.J6ºâm.J6ºâm.FV,šm.Â÷À
0x01ECBB29  2b 6d 06 c2 f7 c0 2d 6d 06 c8 48 cc 9b 6d 10 c8 48 cc 9b 6d 17 c8 48 cc 9b 6d 11 c8 62 fa 0f 6d 07 d9 99 3b 6a 6d 0b d9 99  +m.Â÷À-m.ÈHÌ.m.ÈHÌ.m.ÈHÌ.m.Èbú.m.Ù™;jm.Ù™


0x02021C23    0   0   0   0   0  64  39   0   0   1   0   0   0 250 171   0   0 253 253 253 253 255 255 255 255 115 101 114 118 101 114 115  32  .....@'......ú«..ýýýýÿÿÿÿservers 
0x02021C44   69   9 168   4 109  25 208  43  15 199 109   6  38 103   8  98 109  12  38 103   8  98 109  11 193 110 122 215 109   6  72 232 228  E.¨.m.Ð+.Çm.&g.bm.&g.bm.Ánz×m.Hèä
0x02021C65  186 109  28 217 170  66  83 109   6 217 170  66  83 109  16 195 122 217  19 109  36 203 206  95   1 109   1 203 206  95   1 109  12  ºm.ÙªBSm.ÙªBSm.ÃzÙ.m$ËÎ_.m.ËÎ_.m.
0x02021C86  203 206  95   1 109  16 203 206  95   1 109  36 203 206  95   1 109  13 203 206  95   1 109  11 203 206  95   1 109  14 203 206  95  ËÎ_.m.ËÎ_.m$ËÎ_.m.ËÎ_.m.ËÎ_.m.ËÎ_
0x02021CA7    1 109   2 203 206  95   1 109   7 203 206  95   1 109  10 203 206  95   1 109   8 203 206  95   1 109   9 203 206  95   1 109  26  .m.ËÎ_.m.ËÎ_.m.ËÎ_.m.ËÎ_.m.ËÎ_.m.
0x02021CC8  195 122 217  19 109  46 193 189 160  34 109   6  69 113 112 208 109   6  66 128  50 222 109   6  38 103   8  98 109   9  38 103   8  ÃzÙ.m.Á. "m.EqpÐm.B€2Þm.&g.bm.&g.
0x02021CE9   98 109  16 195 189  83  35 109   6 195 136 250 220 109   7 195 136 250 220 109  10 200 177 229 248 109   7 200  98 250  15 109   6  bm.Ã.S#m.ÃˆúÜm.ÃˆúÜm.È±åøm.Èbú.m.
0x02021D0A  212  72 115 248 109  16  76  93 152  71 109   6  69  64  36 242 109   6 200 177 229 248 109   6  98 227 193 115 109   6  24  22 168  ÔHsøm.L]˜Gm.E@$òm.È±åøm.bãÁsm...¨
0x02021D2B  153 109   6 217 115 155 120 109   8 217 115 155 120 109   6 217 115 155 120 109   7 217 115 155 120 109   9 217 115 155 120 109  10  ™m.Ùs.xm.Ùs.xm.Ùs.xm.Ùs.xm.Ùs.xm.
0x02021D4C   71 109 120 243 109   7  74  54 186 226 109  10  74  54 186 226 109  26  74  54 186 226 108  18  74  54 186 226 109  27  74  54 186  Gmxóm.J6ºâm.J6ºâm.J6ºâl.J6ºâm.J6º
0x02021D6D  226 109  28  74  54 186 226 109  14  74  54 186 226 109  12  74  54 186 226 109   6  74  54 186 226 109  13  74  54 186 226 109  86  âm.J6ºâm.J6ºâm.J6ºâm.J6ºâm.J6ºâmV
0x02021D8E   70  86  44 154 109   7  74  54 186 226 109  17  74  54 186 226 109  16  70  86  44 154 109   6 194 247 192  43 109   6 194 247 192  FV,šm.J6ºâm.J6ºâm.FV,šm.Â÷À+m.Â÷À
0x02021DAF   45 109   6 200  72 204 155 109  16 200  72 204 155 109  23 200  72 204 155 109  17 200  98 250  15 109   7 217 153  59 106 109  11  -m.ÈHÌ.m.ÈHÌ.m.ÈHÌ.m.Èbú.m.Ù™;jm.
0x02021DD0  217 153  59 106 109  10 217 153  59 106 109   6 217 153  59 106 109  15 217 153  59 106 109   7 217   8 180  91 109   7 195   2 123  Ù™;jm.Ù™;jm.Ù™;jm.Ù™;jm.Ù.´[m.Ã.{
0x02021DF1  135 109   6 217 170  66  83 109   7 212  59  18  22 109   6  81 161 190  83 109   6 203 206  95   1 109   6  88 198  14 107 109   6  .m.ÙªBSm.Ô;..m.Q¡.Sm.ËÎ_.m.XÆ.km.
0x02021E12  200  72 204 155 109  15  69  90  34  38 109  56 195 148  48 249 109   6 212  59  18  25 109   6  89  31 129  98 109   6  89  31 129  ÈHÌ.m.EZ"&m8Ã”0ùm.Ô;..m.Y..bm.Y..
0x02021E33   98 109   8  74  54 186 226 109   7  74  54 186 226 109  15  74  54 186 226 109  95  74  54 186 226 109  29  74  54 186 226 109   8  bm.J6ºâm.J6ºâm.J6ºâm_J6ºâm.J6ºâm.
0x02021E54   74  54 186 226 109   9  70  85 119 164 109  66  70  84 100 178 109   7  74  54 186 227 109   6  70  84 100 178 109   6  70  84 100  J6ºâm.FUw¤mBFTd.m.J6ºãm.FTd.m.FTd
0x02021E75  178 109  10  70  84 100 178 109   8  70  86  44 154 109   8  70  84 100 178 109   9  70  84 100 178 109  12  69  93  91 146 109   6  .m.FTd.m.FV,šm.FTd.m.FTd.m.E][’m.
0x02021E96   84  32  48  65 109   6  84  32  48  65 109  16  91 102 160  49 109   6  81 198 187  69 109   7  81 198 187  69 109   6  91 102 160  T 0Am.T 0Am.[f 1m.QÆ»Em.QÆ»Em.[f 
0x02021EB7   49 109  16  75 169 121   1 109   7  24  21  15  84 245   4 200 104 255  82  48 142 200  72 204 155 109  18 217 112 240  61 109   6  1m.K©y.m....Tõ.ÈhÿR0ŽÈHÌ.m.Ùpð=m.
0x02021ED8   83 227 160 147 109   6 200 104  15 171 109   7  84  20  12   2 109   6 217   8 180  90 109   7 217   8 180  90 109   9 217   8 180  Sã “m.Èh.«m.T...m.Ù.´Zm.Ù.´Zm.Ù.´
0x02021EF9   90 109   6 200 155 160   6 109  14  82 117 194 235 109   6 194 241 250 103 109   6  89 166  33 224 109   6 216   6 227  94 109   8  Zm.È. .m.RuÂëm.Âñúgm.Y¦!àm.Ø.ã^m.
0x02021F1A  216   6 227  94 109   6 216   6 227  94 109   9 213 215 194 158 109   6 216   6 227  94 109   7 213 215 194 158 109   9 208 104 192  Ø.ã^m.Ø.ã^m.Õ×Âžm.Ø.ã^m.Õ×Âžm.ÐhÀ
0x02021F3B  243 109   6 213 215 194 158 109   7 213 215 194 158 109   8  84  32  48  65 109  26 195 246 243 169 109   6 201  95 155 251 109   6  óm.Õ×Âžm.Õ×Âžm.T 0Am.Ãöó©m.É_.ûm.
0x02021F5C  194  65  14 114 109   6 195 122 217  19 109  41 213 168  36 138 109   6   0   0   0   0   0   0   0   0   0   0   0   0   0   0   0  ÂA.rm.ÃzÙ.m)Õ¨$Šm...

Quake 3
0x01EF1F80  ff ff ff ff 67 65 74 73 65 72 76 65 72 73 52 65 73 70 6f 6e 73 65 5c d8 b4 ed 0a a1 9b 5c d9 4f b6 f7 82 19 5c d8 b4 ed 0e  ÿÿÿÿgetserversResponse\Ø´í.¡.\ÙO¶÷..\Ø´í.
0x01EF1FA9  d7 90 5c d8 b4 ed 0e 6d 42 5c 4e 3b 70 80 6d 38 5c d8 b4 ed 0d f4 b4 5c d8 b4 ed 0e 6d 38 5c 57 f9 b7 f2 f4 f3 5c 59 95 c2  ×.\Ø´í.mB\N;p€m8\Ø´í.ô´\Ø´í.m8\Wù·òôó\Y.Â

CoD 2                                                                                                     \
0x01249028  ff ff ff ff 67 65 74 73 65 72 76 65 72 73 52 65 73 70 6f 6e 73 65 0a 00 5c 4e 8f 19 5d d9 03 5c d5 ef d0 16 71 21 5c 48 33  ÿÿÿÿgetserversResponse..\N..]Ù.\ÕïÐ.q!\H3
0x01249051  3c 15 71 20 5c 43 e4 0b 45 71 20 5c 51 13 db d1 71 34 5c 54 c8 fc e9 69 91 5c 51 00 d9 b1 6a 3e 5c d8 06 e1 69 71 20 5c 51  <.q \Cä.Eq \Q.ÛÑq4\TÈüéi‘\Q.Ù±j>\Ø.áiq \Q

CoD 4                                                                                                    \
0x01249028  ff ff ff ff 67 65 74 73 65 72 76 65 72 73 52 65 73 70 6f 6e 73 65 0a 00 5c 41 63 f6 4f 71 20 5c 5b 79 88 c3 71 25 5c c1 11  ÿÿÿÿgetserversResponse..\AcöOq \[yˆÃq%\Á.
0x01249051  db 28 71 20 5c 55 be 0b 13 71 20 5c 52 62 e1 b6 71 20 5c 57 76 44 9a 71 20 5c d1 61 55 71 71 20 5c 43 a7 ad f2 d7 20 5c d8  Û(q \U...q \Rbá¶q \WvDšq \ÑaUqq \C§­ò× \Ø


*/
	//Scan to start
	while((p[0]!=0x5c) && (p[0]!=0x20))
	{
		p++;
		if(p>end)
			break;
	}
	p++;

	while(p<end) 
	{	
		if((p[0]=='E') && (p[1]=='O') && (p[2]=='T') && (p[3]==0x00))
			break;
		else if((p[0]=='E') && (p[1]=='O') && (p[2]=='F') && (p[3]==0x00))
			break;

		ZeroMemory(&ptempSI,sizeof(SERVER_INFO));

		//Parse and initialize server info
		dwIP = (DWORD*)&p[0];
		ptempSI.dwIP = ntohl((DWORD)*dwIP); 
	
		p+=4;
		ptempSI.dwPort  = ((p[0])<<8);
		ptempSI.dwPort |=(unsigned char)(p[1]);
		ptempSI.dwPort &= 0x0000FFFF;	//safe, ensure max port value
		
		if(pGI->cGAMEINDEX == Q2_SERVERLIST)
			p+=2; //q2
		else
			p+=3; //q3
		
		int hash = ptempSI.dwIP + ptempSI.dwPort;

		if(UTILZ_checkforduplicates(pGI,  hash,ptempSI.dwIP, ptempSI.dwPort)==FALSE)
		{	
			strcpy_s(ptempSI.szIPaddress,sizeof(ptempSI.szIPaddress),DWORD_IP_to_szIP(ptempSI.dwIP));
			ptempSI.dwPing = 9999;
			ptempSI.cGAMEINDEX = (char) pGI->cGAMEINDEX;
			ptempSI.cCountryFlag = 0;
			ptempSI.bNeedToUpdateServerInfo = true;
			ptempSI.dwIndex = idx++;
			strcpy(ptempSI.szShortCountryName,"zz");
			pGI->pSC->shash.insert(Int_Pair(hash,ptempSI.dwIndex) );
			pGI->pSC->vSI.push_back(ptempSI);

			if(Q3_InsertServerItem!=NULL)
				Q3_InsertServerItem(pGI,ptempSI);

			//InsertServerListView(pCurrentSI);

			Q3_dwNewTotalServers++;
		} //end serverexsist

	} //end while
	//dbg_print("Parsing servers DONE!\n");
	return NULL;
}



/*
0x02845178  63 74 00 31 00 73 76 5f 61 6c 6c 6f 77 41 6e 6f 6e 79 6d 6f 75 73 00 30 00  ct.1.sv_allowAnonymous.0.

0x02845191  30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61  0 0"n/a".0 0"n/a".0 0"n/a
0x028451AA  22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e  ".0 0"n/a".0 0"n/a".0 0"n
0x028451C3  2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30  /a".0 0"n/a".0 0"n/a".0 0
0x028451DC  22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30  "n/a".0 0"n/a".0 0"n/a".0
0x028451F5  20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22   0"n/a".0 0"n/a".0 0"n/a"
0x0284520E  0a 30 20 30 22 6e 2f 61 22 0a 30 20 30 22 6e 2f 61 22 0a 00                 .0 0"n/a".0 0"n/a"..

0x02A466C5  00 62 61 73 65 71 33 00 61 73 5f 76 65 72 73 69 6f 6e 00 31 2e 39 39 71 00  .baseq3.as_version.1.99q.

0x02A466DE  35 32 20 30 20 22 50 61 74 72 69 6f 74 22 0a 33 35 20 30 20 22 52 61 6e 67  52 0 "Patriot".35 0 "Rang
0x02A466F7  65 72 22 0a 33 36 20 30 20 22 48 75 6e 74 65 72 22 0a 37 20 30 20 22 42 69  er".36 0 "Hunter".7 0 "Bi
0x02A46710  6b 65 72 22 0a 00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00  ker"..
*/

/* QW
0x00EB4E55  37 37 20 30 20 31 20 35 39 20 22 70 6c 61 79 65 72 22 20  77 0 1 59 "player" 
0x00EB4E68  22 62 61 73 65 22 20 31 31 20 32 0a 00 00 fd fd fd fd ab  "base" 11 2...ýýýý«
0x00EB4E7B  ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00  «««««««............

0x001B4E59  35 35 20 31 31 20 38 20 36 33 20 22 50 6f 77 64 65 72 20  55 11 8 63 "Powder 
0x001B4E6C  4a 72 2e 22 20 22 22 20 34 20 34 0a 35 36 20 31 39 20 37  Jr." "" 4 4.56 19 7


0x001B4E7F  20 36 36 20 22 4a 75 64 67 65 22 20 22 22 20 34 20 31 33   66 "Judge" "" 4 13
0x001B4E92  0a 35 37 20 31 34 20 37 20 33 35 20 22 7a 61 70 69 61 74  .57 14 7 35 "zapiat
0x001B4EA5  61 63 68 6b 61 21 22 20 22 22 20 34 20 31 31 0a 35 38 20  achka!" "" 4 11.58 
0x001B4EB8  36 38 20 36 20 34 30 20 22 7a 6f 7a 6f 22 20 22 22 20 31  68 6 40 "zozo" "" 1
0x001B4ECB  33 20 31 31 0a 35 39 20 32 34 20 35 20 31 36 20 22 44 72  3 11.59 24 5 16 "Dr
0x001B4EDE  2e 50 61 76 6c 6f 66 66 22 20 22 62 61 73 65 22 20 33 20  .Pavloff" "base" 3 
0x001B4EF1  33 0a 36 30 20 34 30 20 35 20 31 32 20 22 4d 6f 72 73 22  3.60 40 5 12 "Mors"
0x001B4F04  20 22 22 20 31 33 20 31 33 0a 00 00 fd fd fd fd ab ab ab   "" 13 13...ýýýý«««
0x001B4F17  ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 6f ba  «««««............oº

player number, score, time, ping, name, skin, color1, color2
55 11 8 63 "Powder Jr." "" 4 4
56 19 7 66 "Judge" "" 4 13
60 40 5 12 "Mors" "" 13 13    [0a]

0x0392B2E4  20 30 20 30 20 31 32 34 20 22 50 61 7a 75 22 20 22 62 61 73 65 22 20 31 33 20 31 33 0a 35 35   0 0 124 "Pazu" "base" 13 13.55
0x0392B303  20 31 20 30 20 37 37 20 22 28 31 29 70 6c 61 79 65 72 22 20 22 22 20 30 20 30 0a 35 34 20 30   1 0 77 "(1)player" "" 0 0.54 0
0x0392B322  20 30 20 35 38 20 22 5a 65 65 44 22 20 22 62 61 73 65 22 20 34 20 34 0a 35 36 20 32 20 30 20   0 58 "ZeeD" "base" 4 4.56 2 0 
0x0392B341  32 32 20 22 78 2b 79 3d 7a 22 20 22 22 20 31 33 20 39 0a 35 37 20 30 20 30 20 32 37 20 22 28  22 "x+y=z" "" 13 9.57 0 0 27 "(
0x0392B360  32 29 70 6c 61 79 65 72 22 20 22 22 20 30 20 30 0a 00 00 fd fd fd fd ab ab ab ab ab ab ab ab  2)player" "" 0 0...ýýýý««««««««
0x0392B37F  00 00 00 00 00 00 00 00 00 58 89 95 23 23 51 06 00 f8 62 90 03 58 9a 08 05


0x0392B297  
32 35 20             25
34 20                4
31 30 35 33 20       1053
32 37 20             27
22 f2 e5 e4 f2 f5 ed 22 20  "òåäòõí" == REDRUM
22 32 39 22 20 34 20 34 0a   4 1053 27  "29" 4 4.
0x0392B2B6  00 00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 73 89 95 08 36  ..


0x0496B340  39 35 20 30 20 31 31 20 32 31 20 
22 c6 72 e1 67 bc 4b be 22 20  "Ærág.K." Frag(K)
22 24 22 20 31 31 20 33 0a 00  95 0 11 21  "$" 11 3..
0x0496B35F  00 fd fd fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 30 1a d7 6d bb f7  .ýýýý««««««««.
0x0496B329  34 35 20 30 20 32 20 34 31 20 22 49 6e 65 72 74 69 61 22 20 22 62 61 73 65 22 20 31 31 20 32  45 0 2 41 "Inertia" "base" 11 2
0x0496B348  0a 
34 36 20 32 20 
32 20 33 38 20 
22 e6 e9 e5 f2 f5 f3 22 20  "æéåòõó" =FIERUS
22 22 20 34 20 34 0a 00 00 fd fd  .46 2 2 38 "" 4 4...ýý
0x0496B367  fd fd ab ab ab ab ab ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0f 1a d7 52 b8 f7  ýý««««««««

0x0496B2F9  36 31 20 32 35 20 34 20 31 32 20 22 4d 6f 72 73 22 20 22 22 20 31 33 20 31 33 0a 36 32 20 38  61 25 4 12 "Mors" "" 13 13.62 8
0x0496B318  20 34 20 33 38 20 
22 2e 2f d2 92 c7 65 d2 2e cd 75 d2 22 20  "./Ò’ÇeÒ.ÍuÒ"  = ./RoGeR.MuR
22 22 20 34 20 36 0a 35 34 20 38   4 38  "" 4 6.54 8
0x0496B337  20 34 20 31 32 20 22 5a 65 65 44 22 20 22 62 61 73 65 22 20 34 20 34 0a 00 00 fd fd fd fd ab   4 12 "ZeeD" "base" 4 4..
*/



PLAYERDATA *QW_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers)
{

	int Pindex =0;
	PLAYERDATA *pPlayers=NULL;
	BOOL bGTVBug=FALSE;

	if(pointer[0]!=0)
	{
		//Parseplayers
	
		PLAYERDATA *pCurrentPlayer=NULL;
		
		while(pointer<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory ?
				return pPlayers;
			player->pNext = NULL;
			player->cGAMEINDEX = pSI->cGAMEINDEX;
				
			char *endOfString = strchr(pointer,' ');
			if(endOfString!=NULL)
			{
				endOfString[0] = 0;
				player->iPlayer =  atoi(pointer);
				pointer+=strlen(pointer)+1;

				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					player->rate = atoi(pointer);				
					pointer+=strlen(pointer)+1;
				}
				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					//skip time				
					pointer+=strlen(pointer)+1;
				}
				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)                       
				{
					endOfString[0] = 0;
					player->ping = atoi(pointer);
					pointer+=strlen(pointer)+1;
					
				}				
					char *name= NULL;
					 name = strchr(pointer,'\"');				
					if(name!=NULL)
						pointer = ++name; //skip initial byte


				endOfString = strchr(pointer,'\"');
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					player->szPlayerName= _strdup(pointer);  //got the name
				
					pointer+=strlen(pointer)+1;
				}

				//jump to end
				endOfString = strchr(pointer,0x0a);
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					pointer+=strlen(pointer)+1;
				}							
			}

			if(pPlayers==NULL)
				pPlayers = pCurrentPlayer = player;
			else 
				pCurrentPlayer = pCurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;	
			if(pointer[0]==0) 
				break;
		}
	}
	return pPlayers;
}
/* CoD 4
34 30 20 32 34 35 20 22 63 68 65 6e 22 0a                                             40 245 "chen".
31 36 38 20 31 31 39 20 22 4d 69 4e 64 46 72 45 61 4b 22 0a                          168 119 "MiNdFrEaK".
34 31 20 33 39 20 22 48 61 6e 64 53 6f 6d 65 22 0a                                    41 39 "HandSome".
30 20 39 39 39 20 22 4b 75 6e 67 46 75 44 75 63 6b 69 65 22 0a                         0 999 "KungFuDuckie".
32 20 35 37 20 22 48 45 52 42 45 52 54 22 0a 00 fd fd fd fd ab ab ab ab ab ab ab ab    2 57 "HERBERT"..ýýýý««««««««	

*/
PLAYERDATA *Q3_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers,char *szP)
{

	int Pindex =0;
	PLAYERDATA *pQ3Players=NULL;
	BOOL bGTVBug=FALSE;

	if(pointer[0]!=0)
	{
		//Parseplayers
	
		PLAYERDATA *pQ3CurrentPlayer=NULL;
		
		while(pointer<end)
		{
			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory
				return pQ3Players;
			player->pNext = NULL;
				
			char *endString = strchr(pointer,' ');
			if(endString!=NULL)
			{
				endString[0] = 0;
				player->rate = atoi(pointer);
				pointer+=strlen(pointer)+1;

				endString = strchr(pointer,' ');
				if(endString==NULL)                        //fix for Quake 3 GTV bug
				{
					endString =	strchr(pointer,'\"');	
					if(endString==NULL)  //temp fix on Cod2
					{
						free(player);
						break;
					}
					endString[0] = 0;

					bGTVBug = TRUE;
				}
				else
					endString[0] = 0;

				player->ping = atoi(pointer);
				pointer+=strlen(pointer)+1;

				//trim
				endString = strchr(pointer,0x0a);
				if(endString!=NULL)
					endString[0] = 0;

				char *ch= NULL;
				if(bGTVBug==FALSE)
				{
					ch = strchr(pointer,'\"');				
					if(ch!=NULL)
						pointer++; //skip initial byte
					
				} else
					pointer++; //skip initial byte

				bGTVBug = FALSE;  //reset

				ch = strrchr(pointer,'\"');
				if(ch!=NULL)
					ch[0]=0; //remove last " 

				player->szPlayerName= _strdup(pointer);
				pointer+=strlen(pointer)+2;
				
				switch(pSI->cGAMEINDEX)
				{
					case ET_SERVERLIST: //ETpro for retrieving player status (connecting, spectating, allies & axis)
					{
						if(szP!=NULL)
						{
							int l = strlen(szP);
							while(Pindex<l)
							{
								if(szP[Pindex]!='-')
								{
									int _idx = (szP[Pindex]-48);
									player->szClanTag = _strdup(&szPlyType[_idx][0]);
									Pindex++;					
									break;
								}
								Pindex++;						
							}
						}
						break;
					}
					case WARSOW_SERVERLIST:
					{
						if(strcmp(pointer,"0")==0)
							player->szClanTag = _strdup("Spectator");
						else if(strcmp(pointer,"2")==0)
							player->szClanTag = _strdup("Red");
						else if(strcmp(pointer,"3")==0)
							player->szClanTag= _strdup("Blue");
							
						pointer++;
						pointer++;
						break;
					}
				}
			}
			if(pQ3Players==NULL)
				pQ3Players = pQ3CurrentPlayer = player;
			else 
				pQ3CurrentPlayer = pQ3CurrentPlayer->pNext = player;

			*numPlayers= *numPlayers+1;	
			if(pointer[0]==0) //Warsow fix
				break;
					
		}
		
	}
	return pQ3Players;
}



/*
QW
0x01049D28  ff ff ff ff 69 6e 66 6f 52 65 73 70 6f 6e 73 65 0a 5c 67 61 6d 65 6e 61 6d  ÿÿÿÿinfoResponse.\gamenam
0x01049D41  65 00 44 61 72 6b 70 6c 61 63 65 73 2d 51 75 61 6b 65 00 70 72 6f 74 6f 63  e.Darkplaces-Quake.protoc
0x01049D5A  6f 6c 00 33 00 63 6c 69 65 6e 74 73 00 30 00 73 76 5f 6d 61 78 63 6c 69 65  ol.3.clients.0.sv_maxclie
0x01049D73  6e 74 73 00 32 00 6d 61 70 6e 61 6d 65 00 64 6d 34 00 6d 61 78 73 70 65 63  nts.2.mapname.dm4.maxspec
0x01049D8C  74 61 74 6f 72 73 00 38 00 61 6c 6c 6f 77 5f 6c 75 6d 61 00 31 00 61 6c 6c  tators.8.allow_luma.1.all
0x01049DA5  6f 77 5f 62 75 6d 70 00 31 00 66 62 73 6b 69 6e 73 00 31 00 70 6d 5f 62 75  ow_bump.1.fbskins.1.pm_bu
0x01049DBE  6e 6e 79 73 70 65 65 64 63 61 70 00 30 00 70 6d 5f 73 6c 69 64 65 66 69 78  nnyspeedcap.0.pm_slidefix
0x01049DD7  00 30 00 70 6d 5f 73 6c 69 64 79 73 6c 6f 70 65 73 00 30 00 70 6d 5f 61 69  .0.pm_slidyslopes.0.pm_ai
0x01049DF0  72 73 74 65 70 00 30 00 70 6d 5f 77 61 6c 6c 6a 75 6d 70 00 30 00 70 6d 5f  rstep.0.pm_walljump.0.pm_
0x01049E09  73 74 65 70 68 65 69 67 68 74 00 31 38 00 2a 76 65 72 73 69 6f 6e 00 46 54  stepheight.18.*version.FT
0x01049E22  45 20 33 31 33 34 00 2a 7a 5f 65 78 74 00 36 33 00 70 6d 5f 6b 74 6a 75 6d  E 3134.*z_ext.63.pm_ktjum
0x01049E3B  70 00 31 00 2a 67 61 6d 65 64 69 72 00 71 77 00 6d 61 78 66 70 73 00 37 37  p.1.*gamedir.qw.maxfps.77
0x01049E54  00 68 6f 73 74 6e 61 6d 65 00 41 73 67 61 61 72 64 20 31 6f 6e 31 2f 32 6f  .hostname.Asgaard 1on1/2o
0x01049E6D  6e 32 2f 34 6f 6e 34 20 2d 20 4e 75 65 72 6e 62 65 72 67 2c 20 47 65 72 6d  n2/4on4 - Nuernberg, Germ
0x01049E86  61 6e 79 00 64 65 61 74 68 6d 61 74 63 68 00 33 00 74 65 61 6d 70 6c 61 79  any.deathmatch.3.teamplay
0x01049E9F  00 30 00 74 69 6d 65 6c 69 6d 69 74 00 31 30 00 fd fd fd fd ab ab ab ab ab  .0.timelimit.10.ýýýý«««««
0x01049EB8  ab ab ab 00 00 00 00 00 00 00 00 00 00 00 00 00 7c 20 9d 3d 88 79 0a 00 18  «««.............| .=ˆy...


*/

char *Q3_ParseServerRules(SERVER_RULES* &pLinkedListStart,char *p,DWORD packetlen)
{
	SERVER_RULES *pSR=NULL;
	SERVER_RULES *pCurrentSR=NULL;
	Q3DATA_SERVER_INFO *Q3SI; 
	Q2DATA_SERVER_INFO *Q2SI;
	WARSOWDATA_SERVER_INFO *WSI;
	QWDATA_SERVER_SHORTINFO *QWSI;

	pLinkedListStart = NULL;
	Q3SI = (Q3DATA_SERVER_INFO *)p;
	Q2SI = (Q2DATA_SERVER_INFO *)p;
	QWSI = (QWDATA_SERVER_SHORTINFO *)p;
	WSI =  (WARSOWDATA_SERVER_INFO*)p;

	pSR=NULL;

	char *pointer=NULL;
	if(strncmp((char*)&Q3SI->leadData,"statusResponse",14)!=0)
	{
		if(strncmp((char*)&WSI->leadData,"infoResponse",12)!=0)
		{
			if (QWSI->leadData[0]=='n') //could it be a Quake world or
				pointer=QWSI->data;
			else
			{
				if(strncmp((char*)&Q2SI->leadData,"print",5)==0) // a  Quake 2 server?
					pointer=Q2SI->data;
				else
				{
					dbg_print("Unrecognized response from server!");
					return NULL;
				}
			}
		}
		else
		{
			pointer=WSI->data;  //it is Warsow
		}

	} else
	{
		pointer=Q3SI->data;
	}

	char *end = &p[packetlen];
	while(pointer<end)
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
		
		char *endString = strchr(pointer,'\\');
		if(endString!=NULL)
		{
			endString[0] = 0;
			pCurrentSR->name = _strdup(pointer);
			pointer+=(int)strlen(pointer)+1;
		}
		endString = strchr(pointer,'\\');

		if(endString!=NULL)
		{
			bool finish=false;
			endString[0] = 0;
			char *pend = strchr(pointer,0x0a);
			if(pend!=NULL) //Warsow fix
			{
				pend[0] = 0;
				finish=true;
			}

			pCurrentSR->value = _strdup(pointer);
			pointer+=strlen(pointer)+1;
			
			if(finish)
				break;

		} else
		{
			//Last server rules ends with 0x0a
			endString = strchr(pointer,0x0a);
			if(endString!=NULL)
			{
				endString[0] = 0;
				pCurrentSR->value = _strdup(pointer);
				pointer+=strlen(pointer)+1;
			//	pCurrentSR->pNext = NULL;
				break;
			} else  //QW and Q2 servers..
			{
				pCurrentSR->value = _strdup(pointer);
				pointer+=strlen(pointer)+1;
				pointer++;
			}
		}
	
		if(pointer[0]==0x0a)
			break;

	}

//	pointer++; //move to playerlist data (Q3)

	return pointer;
}

DWORD Q3_ConnectToMasterServer(GAME_INFO *pGI)
{
	size_t packetlen=0;
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	//WSADATA wsaData;
	SOCKET ConnectSocket;

	if(strlen(pGI->szQueryString)==0)
	{
		if(pGI->dwProtocol==0)
			sprintf(sendbuf, "\xFF\xFF\xFF\xFFgetservers empty full");
		else
			sprintf(sendbuf, "\xFF\xFF\xFF\xFFgetservers %hu empty full",pGI->dwProtocol);
	}
	else
	{
		if(pGI->dwProtocol==0)
			sprintf(sendbuf, "\xFF\xFF\xFF\xFFgetservers %s empty full",pGI->szQueryString);
		else
		{
			if(pGI->cGAMEINDEX == WARSOW_SERVERLIST)
				sprintf(sendbuf, "\xFF\xFF\xFF\xFFgetservers %s %hu empty full",pGI->szQueryString,pGI->dwProtocol);
			else
				sprintf(sendbuf, "\xFF\xFF\xFF\xFFgetservers %hu %s empty full",pGI->dwProtocol,pGI->szQueryString);  //open arena needs another order
		}
	}

	int len = (int)strlen(sendbuf);

	if(pGI->cGAMEINDEX==Q2_SERVERLIST)
	{
		sprintf(sendbuf, "query\x0a\x00");
		len = (int)strlen(sendbuf)+1;
	}


	ConnectSocket = getsockudp(pGI->szMasterServerIP,(unsigned short)pGI->dwMasterServerPORT); // etmaster.idsoftware.com"27950 master server
   
	if(INVALID_SOCKET==ConnectSocket)
	{

		dbg_print("Error connecting to socket!");
		return 1;
	}

	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		dbg_print("WSACreateEvent()");
		closesocket(ConnectSocket);
		return 1;
	}

	//
	// Make the socket non-blocking and 
	// associate it with network events
	//
	int nRet;
	nRet = WSAEventSelect(ConnectSocket, hEvent,FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		dbg_print("EventSelect()");
		closesocket(ConnectSocket);
		WSACloseEvent(hEvent);
		return 5;
	}


	int i = 0;
	unsigned char *packet[MAX_PACKETS];
	size_t packet_len[MAX_PACKETS];

	Q3_dwTotalServers=0;
	Q3_dwNewTotalServers = 0;
	for(i=0; i<MAX_PACKETS;i++)
		packet[i] = NULL;

	i = 0;
	//
	// Handle async network events
	//

	WSANETWORKEVENTS events;
	while(1)
	{
		//
		// Wait for something to happen
		//
		//dbg_print("\nWaitForMultipleEvents()");
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1, &hEvent, FALSE,4000,FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			dbg_print("\nWait timed out");
			break;
		}

		//
		// Figure out what happened
		//
		//AddLogInfo(0,"\nWSAEnumNetworkEvents()");
		nRet = WSAEnumNetworkEvents(ConnectSocket, hEvent, &events);
		if (nRet == SOCKET_ERROR)
		{
			dbg_print("WSAEnumNetworkEvents()");
			break;
		}

		//				 //
		// Handle events //
		//				 //

		// Connect event?
		if (events.lNetworkEvents & FD_CONNECT)
		{
			AddLogInfo(0,"\nFD_CONNECT: %d", events.iErrorCode[FD_CONNECT_BIT]);
			dbg_print("Sending command %s Len: %d",sendbuf,len);
			if(send(ConnectSocket, sendbuf, len , 0)==SOCKET_ERROR) 
			{
				Q3_bScanningInProgress = FALSE;
				WSACloseEvent(hEvent);
				closesocket(ConnectSocket);		
				dbg_print("Error sending packet!");
				return 2;
			}
		}

		// Read event?
		if (events.lNetworkEvents & FD_READ)
		{
			//AddLogInfo(0,"\nFD_READ: %d, %d",events.iErrorCode[FD_READ_BIT],i);
			// Read the data and write it to stdout
			packet[i]=(unsigned char*)ReadPacket(ConnectSocket, &packetlen);
			packet_len[i] = packetlen;
			i++;
			if(i>=MAX_PACKETS)
				break;
	
		}

		// Close event?
/*		if (events.lNetworkEvents & FD_CLOSE)
		{
			AddLogInfo(0,"\nFD_CLOSE: %d",events.iErrorCode[FD_CLOSE_BIT]);
			break;
		}

		// Write event?
		if (events.lNetworkEvents & FD_WRITE)
		{
			AddLogInfo(0,"\nFD_WRITE: %d",events.iErrorCode[FD_WRITE_BIT]);
		}
*/
	}
	
	closesocket(ConnectSocket);
	WSACloseEvent(hEvent);

	for(i=0; i<MAX_PACKETS;i++)
	{
		if(packet[i] != NULL)
		{
		    Q3_parseServers((char*)packet[i],packet_len[i],pGI);
			free(packet[i]);			
			packet[i]=NULL;
			SetStatusText(pGI->iIconIndex,"Recieved %d new %s servers from master server.",Q3_dwNewTotalServers,pGI->szGAME_NAME);
			if(g_bCancel)
				break;
			Sleep(50); //give cpu sometime to breath
		}
	}

	pGI->dwTotalServers += Q3_dwNewTotalServers;
	Q3_bScanningInProgress = FALSE;

	return 0;
}





