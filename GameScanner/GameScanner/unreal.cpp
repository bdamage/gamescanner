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
	BYTE *data;
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

	int idxPortStep=0;
	unsigned short port = pSI->usPort+10;//pSI->usPort;//6500;
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
	int len = 0; //(int)strlen(sendbuf);
	char sendbuf[80];
	ZeroMemory(sendbuf,sizeof(sendbuf));
	//len = UTILZ_ConvertEscapeCodes(GamesInfo[pSI->cGAMEINDEX].szServerRequestInfo,sendbuf,sizeof(sendbuf));

	
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
		packetlen = send(pSocket, (const char*)&sr,sizeof(second_request), 0);
			
		if(packetlen==SOCKET_ERROR) 
		{
			dbg_print("Error at send()\n");
			closesocket(pSocket);		
			return -1;
		}
		
		packetlen = 0;
		packet=(unsigned char*)getpacket(pSocket, &packetlen);
		if(packet==NULL)
		{
		}
		else
		{
			server_info_response *sir = (server_info_response *)packet;
			sir->data;

			free(packet);

		}
	
		
	}


	closesocket(pSocket);
	return 0;

	if(packet) 
	{
		pSI->dwPing = (GetTickCount() - dwStartTick);
		dbg_dumpbuf("dump.bin", packet, packetlen);
		SERVER_RULES *pServRules=NULL;
		char *end = (char*)((packet)+packetlen);
		
		char *pCurrPointer=NULL; //will contain the start address for the player data

		//pCurrPointer = UT_ParseServerRules(pServRules,(char*)packet,packetlen);
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
				//	pPlayers = QW_ParsePlayers(pSI,pCurrPointer,end,&nPlayers);
					break;
				default:
				//	pPlayers = Q3_ParsePlayers(pSI,pCurrPointer,end,&nPlayers,szP_ET);
				break;
			}
			
	
			pSI->pPlayerData = pQ3Players;
			//if(pQ3Players!=NULL) //removed since ver 1.08
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
			pSI->bNeedToUpdateServerInfo = 0;
			pSI->bUpdated = true;
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
	

			pVarValue = Get_RuleValue("mapname",pServRules);
			if(pVarValue!=NULL)
				strncpy(pSI->szMap,pVarValue ,39);
			else
			{ //for QW
				pVarValue = Get_RuleValue("map",pServRules);
				if(pVarValue!=NULL)
					strncpy(pSI->szMap,pVarValue ,39);

			}
			
			pSI->dwMap = Get_MapByName(pSI->cGAMEINDEX, pVarValue);


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
			/*	case Q3_SERVERLIST:
					{
						szVarValue = Get_RuleValue("bot_minplayers",pServRules); //Warsow specific
						if(szVarValue!=NULL)
							pSI->cBots = atoi(szVarValue);

					}*/
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


			if(UT_Callback_CheckForBuddy!=NULL)
				UT_Callback_CheckForBuddy(pQ3Players,pSI);
			//Debug purpose
			if(pServRules!=pSI->pServerRules)
			{
				AddLogInfo(ETSV_ERROR,"Error at pServRules!=pSI->pServerRules");
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
	return 0;
}
