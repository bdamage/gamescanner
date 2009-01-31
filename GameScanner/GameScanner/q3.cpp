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

extern CLanguage g_lang;
extern bool g_bCancel;
extern GamesMap GamesInfo;
extern APP_SETTINGS_NEW AppCFG;
extern HWND g_hWnd;
extern SERVER_INFO *g_CurrentSRV;
bool Q3_bCloseApp=false;


long (*Q3_UpdateServerListView)(DWORD index);
long (*Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);
long (*Q3_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);

DWORD Q3_dwThreadCounter=0;
DWORD Q3_dwTotalServers=0;
DWORD Q3_dwNewTotalServers=0;
BOOL Q3_bScanningInProgress = FALSE;


char szPlyType[5][12] = {TEXT("Connecting"), TEXT("Axis"), TEXT("Allies"), TEXT("Spectator"),TEXT("Unknown")};


void Q3_SetCallbacks(long (*UpdateServerListView)(DWORD index), 
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	Q3_UpdateServerListView = UpdateServerListView;
	Callback_CheckForBuddy = _Callback_CheckForBuddy;
	Q3_InsertServerItem = InsertServerItem;
}




DWORD Q3_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick=0;

	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return (DWORD)0x000001;
	}

	if(pSI->bLocked)
	{
		dbg_print("Server locked @Get_ServerStatus!\n");
		return 3;
	}

	pSI->bLocked = TRUE;


	if(pSI->pPlayerData!=NULL)
		CleanUp_PlayerList(pSI->pPlayerData);
	pSI->pPlayerData = NULL;

	if(pSI->pServerRules!=NULL)
		CleanUp_ServerRules(pSI->pServerRules);
	pSI->pServerRules = NULL;


	pSocket =  getsockudp(pSI->szIPaddress ,(unsigned short)pSI->usPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  pSI->bLocked = FALSE;
	  return 0x000002;
	}

	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
	{
	
		char szShortName[4];			
		fnIPtoCountry(pSI->dwIP,szShortName);//optimized since v1.31 
		strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
	}
	DWORD dwRetries=0;
	int len = 0; //(int)strlen(sendbuf);
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	len = UTILZ_ConvertEscapeCodes(GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo,sendbuf,sizeof(sendbuf));
retry:
	if(GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo!=NULL)
		packetlen = send(pSocket, sendbuf, len+1, 0);
	else
		packetlen=SOCKET_ERROR;

		
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		pSI->bLocked = FALSE;
		return -1;
	}

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

		pCurrPointer = Q3_ParseServerRules(pServRules,(char*)packet,packetlen);
		pSI->pServerRules = pServRules;
		if(pServRules!=NULL)
		{		
		
			char szP_ET[150];
			if(pSI->cGAMEINDEX == ET_SERVERLIST)
			{			
				ZeroMemory(&szP_ET,sizeof(szP_ET));			
				char *szPVarValue=NULL;
				szPVarValue = Get_RuleValue("P",pServRules);
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
			if(UpdatePlayerListView!=NULL) 
				UpdatePlayerListView(pQ3Players);
				
			if(UpdateRulesListView!=NULL)
				UpdateRulesListView(pServRules);
			//-----------------------------------
			//Update server info from rule values
			//-----------------------------------
			pSI->bNeedToUpdateServerInfo = 0;
			pSI->bUpdated = true;
			pSI->cPurge = 0;
			pSI->nCurrentPlayers = nPlayers;
			char *szVarValue=NULL;
			char *pVarValue = NULL;
			pVarValue = Get_RuleValue("sv_hostname",pServRules);
			if(pVarValue!=NULL)
			{
				strncpy(pSI->szServerName,pVarValue ,99);
			}
			else  //QW
			{
				pVarValue = Get_RuleValue("hostname",pServRules);
				if(pVarValue!=NULL)
					strncpy(pSI->szServerName,pVarValue ,99);
			}
			//getting status value
			switch(pSI->cGAMEINDEX)
			{	
				case QW_SERVERLIST:
					{
						Set_RuleStr(pSI->pServerRules,"status", pSI->szSTATUS,sizeof(pSI->szSTATUS)-1);

					}
				break;
				case Q2_SERVERLIST:
					{
						//Lets enumerate through vars for the best match...
						if(Set_RuleStr(pSI->pServerRules,"time_remaining", pSI->szSTATUS,sizeof(pSI->szSTATUS)-1)==FALSE)						
						{
							if(Set_RuleStr(pSI->pServerRules,"#time_left", pSI->szSTATUS,sizeof(pSI->szSTATUS)-1)==FALSE)
							{
								Set_RuleStr(pSI->pServerRules,"gamestats", pSI->szSTATUS,sizeof(pSI->szSTATUS)-1);	

							}
						}
							szVarValue = Get_RuleValue("game",pServRules);
							if(szVarValue!=NULL)
								strcpy(pSI->szGameTypeName,szVarValue);
							pSI->dwGameType = Get_GameTypeByName(pSI->cGAMEINDEX, szVarValue);
						
					}
				break;
			}

			if(Set_RuleStr(pSI->pServerRules,"mapname", pSI->szMap,sizeof(pSI->szMap)-1)==FALSE)
			{
				Set_RuleStr(pSI->pServerRules,"map", pSI->szMap,sizeof(pSI->szMap)-1);	//for Quake World
			}
			
			pSI->dwMap = Get_MapByName(pSI->cGAMEINDEX, pSI->szMap); //get quick lookup DWORD value for filtering


			switch(pSI->cGAMEINDEX)	 //MODS
			{

				case COD4_SERVERLIST :
					{
						pVarValue = Get_RuleValue("mod",pServRules);			
						if(pVarValue!=NULL)
						{
							pSI->dwMod = 1;
							if(strcmp(pVarValue,"1")==0)
							{	
								char *mod;
								mod = Get_RuleValue("fs_game",pServRules);
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
						pVarValue = Get_RuleValue("gamename",pServRules);			
						if(pVarValue==NULL)
						{
							pVarValue = Get_RuleValue("*gamedir",pServRules); //Normal QW
							if(pVarValue==NULL)							
							{
								pVarValue = Get_RuleValue("gamename",pServRules); //Normal Q2
								if(pVarValue==NULL)
									pVarValue = Get_RuleValue("*progs",pServRules); //Is it QW with Qizmo proxy
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
			pSI->dwMod = Get_ModByName(pSI->cGAMEINDEX,pSI->szMod);
			
			switch(pSI->cGAMEINDEX)
			{			
				case COD_SERVERLIST :
				case COD2_SERVERLIST :
				case COD4_SERVERLIST :
					{
						szVarValue = Get_RuleValue("pswrd",pServRules);  //CoD & Cod2
						if(szVarValue!=NULL)
							pSI->bPrivate = atoi(szVarValue);

						szVarValue = Get_RuleValue("shortversion",pServRules);
						if(szVarValue!=NULL)
						{
							ZeroMemory(pSI->szVersion,sizeof(pSI->szVersion));
							strncpy(pSI->szVersion,szVarValue,49);
						}
					}		
				break;
				case NEXUIZ_SERVERLIST:
				case WARSOW_SERVERLIST:
					{
						szVarValue = Get_RuleValue("bots",pServRules); //Warsow specific
						if(szVarValue!=NULL)
							pSI->cBots = atoi(szVarValue);
					}
				case Q3_SERVERLIST:
					{
						if(pSI->szMod,"cpma")
						{
							szVarValue = Get_RuleValue("mode_current",pServRules);
							if(szVarValue!=NULL)
							{					
								strncpy(pSI->szGameTypeName,szVarValue,sizeof(pSI->szGameTypeName)-1);
								strncpy(pSI->szMode,szVarValue,sizeof(pSI->szMode)-1);
								pSI->dwMode = Get_ModeByName(pSI->cGAMEINDEX, pSI->szGameTypeName);
							}
						}

					}
				case ET_SERVERLIST:
					{//Below code removed since v1.24
					//	szVarValue = Get_RuleValue("omnibot_enable",pServRules); //ET specific
					//	if(szVarValue!=NULL)
					//		pSI->cBots = atoi(szVarValue);
					} //Fall through and continue on default...
				default:
					{						
						szVarValue = Get_RuleValue("g_needpass",pServRules);
						if(szVarValue!=NULL)
							pSI->bPrivate = (char)atoi(szVarValue);
						else
						{
							szVarValue = Get_RuleValue("needpass",pServRules);
							if(szVarValue!=NULL)
							{
								pSI->bPrivate = (char)atoi(szVarValue);
								
								if(pSI->bPrivate==4) //Quake World fix
									pSI->bPrivate = 0;

							}
						}
						ZeroMemory(pSI->szVersion,sizeof(pSI->szVersion));
						szVarValue = Get_RuleValue("version",pServRules);
						if(szVarValue==NULL)
							szVarValue = Get_RuleValue("*version",pServRules); // QuakeWorld
								if(szVarValue==NULL)
									szVarValue = Get_RuleValue("gameversion",pServRules); // Nexuiz
						
						if(szVarValue!=NULL)
							strncpy(pSI->szVersion,szVarValue,MAX_VERSION_LEN-1);
					}
					break;
			}
			if(pSI->szVersion!=NULL)
				pSI->dwVersion =  Get_FilterVersionByVersionString(pSI->cGAMEINDEX,pSI->szVersion);

			szVarValue = Get_RuleValue("sv_pure",pServRules);
			if(szVarValue!=NULL)
				pSI->cPure = atoi(szVarValue);
			
			szVarValue = Get_RuleValue("g_gametype",pServRules);
			pSI->dwGameType = Get_GameTypeByName(pSI->cGAMEINDEX, szVarValue);

			szVarValue = Get_RuleValue("sv_punkbuster",pServRules);
			if(szVarValue==NULL)
				szVarValue = Get_RuleValue("sv_battleye",pServRules); //Warsow
			
			if(szVarValue!=NULL)
				pSI->bPunkbuster = (char)atoi(szVarValue);


			szVarValue = Get_RuleValue("sv_privateClients",pServRules);
			if(szVarValue!=NULL)
				pSI->nPrivateClients = atoi(szVarValue);

			szVarValue = Get_RuleValue("sv_maxclients",pServRules);
			if(szVarValue!=NULL)
			{
				int maxClient = atoi(szVarValue);
				if(maxClient>pSI->nPrivateClients)
					pSI->nMaxPlayers = maxClient-pSI->nPrivateClients;
				else
					pSI->nMaxPlayers = pSI->nPrivateClients-maxClient;

			}
			else
			{ //for QW
				szVarValue = Get_RuleValue("maxclients",pServRules);
				if(szVarValue!=NULL)
					pSI->nMaxPlayers = atoi(szVarValue)-pSI->nPrivateClients;
			}


			if(Callback_CheckForBuddy!=NULL)
				Callback_CheckForBuddy(pQ3Players,pSI);
			//Debug purpose
			if(pServRules!=pSI->pServerRules)
			{
				AddLogInfo(GS_LOG_ERROR,"Error at pServRules!=pSI->pServerRules");
				DebugBreak();
			} 
			else
			{
				CleanUp_ServerRules(pSI->pServerRules);
				pSI->pServerRules = NULL;	
			}

			CleanUp_PlayerList(pQ3Players);
			pSI->pPlayerData = NULL;
			

		} //end if(pServRules!=NULL)

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;   //increase purge counter when the server is not responding

	closesocket(pSocket);
	pSI->bLocked = FALSE;
	return 0;
}




SERVER_INFO* Q3_ParseServers(char * p, DWORD length, GAME_INFO *pGI)
{
	Q3DATA *Q3d;
	Q3d = (Q3DATA*)p;

	SERVER_INFO ptempSI;
	DWORD idx = pGI->vSI.size();	
	DWORD *dwIP=NULL;
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
	int pbytes=0;
	//Scan to start
	while((p[0]!=0x5c) && (p[0]!=0x20))
	{
		pbytes++;
		p++;
		if(p>end)
			break;
	}
	end = p+(length-pbytes);
	p++;
	
	ZeroMemory(&ptempSI,sizeof(SERVER_INFO));
	ptempSI.dwPing = 9999;
	ptempSI.cGAMEINDEX = (char) pGI->cGAMEINDEX;
	strcpy(ptempSI.szShortCountryName,"zz");
	ptempSI.bNeedToUpdateServerInfo = 1;

	int hash = 0;
	while(p<end) 
	{	
		if((p[0]=='E') && (p[1]=='O') && (p[2]=='T') && (p[3]==0x00))
			break;
		else if((p[0]=='E') && (p[1]=='O') && (p[2]=='F') && (p[3]==0x00))
			break;

		//Parse and initialize server info
		dwIP = (DWORD*)&p[0];
		ptempSI.dwIP = ntohl((DWORD)*dwIP); 
	
		p+=4;
		ptempSI.usPort  = ((p[0])<<8);
		ptempSI.usPort |=(unsigned char)(p[1]);
		//ptempSI.usPort &= 0x0000FFFF;	//safe, ensure max port value
		
		if(pGI->cGAMEINDEX == Q2_SERVERLIST)
			p+=2; //q2
		else
			p+=3; //q3
		
		hash = ptempSI.dwIP + ptempSI.usPort;

		if(UTILZ_checkforduplicates(pGI,  hash,ptempSI.dwIP, ptempSI.usPort)==FALSE)
		{	
			strcpy_s(ptempSI.szIPaddress,sizeof(ptempSI.szIPaddress),DWORD_IP_to_szIP(ptempSI.dwIP));
			ptempSI.dwIndex = idx++;

			SERVER_INFO *pNewSrv = (SERVER_INFO*)calloc(1,sizeof(SERVER_INFO));
			memcpy(pNewSrv,&ptempSI,sizeof(SERVER_INFO));
		

			pGI->shash.insert(Int_Pair(hash,ptempSI.dwIndex) );
			pGI->vSI.push_back(pNewSrv);
			Q3_dwNewTotalServers++;
		}
		//end serverexsist

	} //end while
	return NULL;
}


PLAYERDATA *QW_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers)
{
	PLAYERDATA *pPlayers=NULL;

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
			player->pServerInfo = pSI;
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
					if(pointer[0]!='S')
					{
						player->rate = atoi(pointer);				
					} else
					{
						player->szTeam = strdup("Spectator");
					}
					pointer+=strlen(pointer)+1;
				}
				endOfString = strchr(pointer,' ');
				if(endOfString!=NULL)
				{
					endOfString[0] = 0;
					player->time = atoi(pointer);
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
			player->dwServerIndex = pSI->dwIndex;

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
int ParseNum(char* pData)
{
	char *endOfString = strchr(pData,' ');
	if(endOfString!=NULL)
	{
		endOfString[0] = 0;
		int val = atoi(pData);		
		pData+=strlen(pData)+1;
		return val;
	}
	return 0;
}
//Don't forget to free up the new allocated string
char * ParseString(char* pData)
{
	char *StartOfString= NULL;
	StartOfString = strchr(pData,'\"');				
	if(StartOfString!=NULL)
	{
		pData++; //skip initial byte which is "
		
		char *EndOfString = strrchr(pData,'\"');
		if(EndOfString!=NULL)
		{
			EndOfString[0]=0; //Add a termination NULL 			
			char *NewString= _strdup(pData);
			pData+=strlen(pData)+1;			
			return NewString;
		}
	}
	return NULL;
}

PLAYERDATA *Q3_ParsePlayers(SERVER_INFO *pSI,char *pointer,char *end, DWORD *numPlayers,char *szP)
{
	int Pindex =0;
	PLAYERDATA *pQ3Players=NULL;
	BOOL bGTVBug=FALSE;
	pSI->cBots = 0;
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
			player->szClanTag = NULL;	
			player->szTeam = NULL;	
			player->cGAMEINDEX = pSI->cGAMEINDEX;
			player->pServerInfo = pSI;
			player->dwServerIndex = pSI->dwIndex;

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
				endString = strchr(pointer,0x0a);  //is this really needed???
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
					
					case Q3_SERVERLIST:
						{
							if(player->ping == 0)
								pSI->cBots ++;

						}
						break;
					case ET_SERVERLIST: //ETpro for retrieving player status (connecting, spectating, allies & axis)
					{
						if(player->ping == 0)
							pSI->cBots ++;

						if(szP!=NULL)
						{
							int l = strlen(szP);
							while(Pindex<l)
							{
								if(szP[Pindex]!='-')
								{
									int _idx = (szP[Pindex]-48);
									player->szTeam = _strdup(&szPlyType[_idx][0]);
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
							player->szTeam = _strdup("Spectator");
						else if(strcmp(pointer,"2")==0)
							player->szTeam = _strdup("Red");
						else if(strcmp(pointer,"3")==0)
							player->szTeam= _strdup("Blue");
							
						pointer+=2;						
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
	

	if(strncmp((char*)&Q3SI->leadData,"statusResponse",14)!=0) //start detecting most common first
	{
		if(strncmp((char*)&WSI->leadData,"infoResponse",12)!=0)
		{
			if (QWSI->leadData[0]=='n') //could it be a Quake world or
			{
				pointer=QWSI->data;
				if(strncmp((char*)&QWSI->data,"banned",6)==0)
					return NULL;
			}
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
	return pointer;
}


DWORD Q3_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx)
{
	size_t packetlen=0;
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	SOCKET ConnectSocket;

//	UINT_PTR timerProgressWait = SetTimer(g_hWnd,IDT_TIMER_1SECOND,100,NULL);
	
	int i = 0;
	unsigned char *packet[MAX_PACKETS];
	size_t packet_len[MAX_PACKETS];

	Q3_dwTotalServers=0;
	Q3_dwNewTotalServers = 0;
	for(i=0; i<MAX_PACKETS;i++)
		packet[i] = NULL;

	i = 0;

	char szIP[260];
	strcpy(szIP,pGI->szMasterServerIP[iMasterIdx]);
	
	SplitIPandPORT(szIP,pGI->dwMasterServerPORT);

	int len = 0;//(int)strlen(sendbuf);
	len = UTILZ_ConvertEscapeCodes(pGI->szMasterQueryString,sendbuf,sizeof(sendbuf));
	ConnectSocket = getsockudp(szIP,(unsigned short)pGI->dwMasterServerPORT); // etmaster.idsoftware.com"27950 master server
  
	if(INVALID_SOCKET==ConnectSocket)
	{

		KillTimer(g_hWnd,IDT_1SECOND);
		AddLogInfo(GS_LOG_ERROR,"Error connecting to socket!");
		return 1;
	}

	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		KillTimer(g_hWnd,IDT_1SECOND);
		AddLogInfo(GS_LOG_ERROR,"WSACreateEvent() = WSA_INVALID_EVENT");
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
		KillTimer(g_hWnd,IDT_1SECOND);
		AddLogInfo(GS_LOG_ERROR,"EventSelect() = SOCKET_ERROR");
		closesocket(ConnectSocket);
		WSACloseEvent(hEvent);
		return 5;
	}



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
			AddLogInfo(GS_LOG_ERROR,"WSAWaitForMultipleEvents = WSA_WAIT_TIMEOUT");
			break;
		}

		//
		// Figure out what happened
		//
		//AddLogInfo(0,"\nWSAEnumNetworkEvents()");
		nRet = WSAEnumNetworkEvents(ConnectSocket, hEvent, &events);
		if (nRet == SOCKET_ERROR)
		{
			AddLogInfo(GS_LOG_ERROR,"WSAEnumNetworkEvents() = SOCKET_ERROR");
			break;
		}

		//				 //
		// Handle events //
		//				 //

		// Connect event?
		if (events.lNetworkEvents & FD_CONNECT)
		{
			//AddLogInfo(0,"\nFD_CONNECT: %d", events.iErrorCode[FD_CONNECT_BIT]);
			AddLogInfo(0,"Master server %s   (%d)",pGI->szMasterServerIP[0],(unsigned short)pGI->dwMasterServerPORT);
			AddLogInfo(0,"Sending command [%s] (%s) Len: %d",sendbuf,pGI->szMasterQueryString,len);
			if(send(ConnectSocket, sendbuf, len , 0)==SOCKET_ERROR) 
			{
				KillTimer(g_hWnd,IDT_1SECOND);
				Q3_bScanningInProgress = FALSE;
				WSACloseEvent(hEvent);
				closesocket(ConnectSocket);		
				AddLogInfo(GS_LOG_ERROR,"Error sending master query packet!");
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
		if (events.lNetworkEvents & FD_CLOSE)
		{
			AddLogInfo(0,"\nFD_CLOSE: %d",events.iErrorCode[FD_CLOSE_BIT]);
			break;
		}
/*
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
		    Q3_ParseServers((char*)packet[i],packet_len[i],pGI);
			free(packet[i]);			
			packet[i]=NULL;
			SetStatusText(pGI->iIconIndex,g_lang.GetString("StatusReceivingMaster"),Q3_dwNewTotalServers,pGI->szGAME_NAME);
			if(g_bCancel)
			{
				//ensure to clean up
				for(i=0; i<MAX_PACKETS;i++)
				{
					if(packet[i] != NULL)
					{
						free(packet[i]);			
						packet[i]=NULL;
					}
				}
			
				break;
			}
			Sleep(50); //give cpu sometime to breath
		}
	}

//	KillTimer(g_hWnd,IDT_TIMER_1SECOND);
	pGI->dwTotalServers += Q3_dwNewTotalServers;
	Q3_bScanningInProgress = FALSE;

	return 0;
}





