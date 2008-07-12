#define _SECURE_SCL 0



#include "stdafx.h"

#include "steam.h"
#include "utilz.h"
#include "scanner.h"
#include "..\..\iptocountry\iptocountry.h"

#define MAX_PACKETS 500
extern bool g_bCancel;
extern HWND g_hWnd;


#pragma pack(1)



struct A2S_PLAYER_RESPONSE_DATA
{
	char leadData[4]; //FF FF FF FF D
	char response;
	char numPlayers;
	char playerData;
} ;

struct A2S_CHALLANGE_RESPONSE_DATA
{
	char leadData[5]; //FF FF FF FF 41
	DWORD dwChallenge;
} ;


struct STEAM_MASTER_LEADDATA
{
	char leadData[6]; //FF FF FF FF 66 0A
	char data[1];
} ;

struct STEAM_SERVER_RESPONSE
{
	char leadData[4]; //FF FF FF FF 
	char type[1];
	char data[1];
} ;

char A2S_MASTER_REPLY[] =  {"\xFF\xFF\xFF\xFF\x66\x0A"};

#define A2S_GETCHALLENGE	  "\xFF\xFF\xFF\xFF\x57"
#define A2S_CHALLENGERESPONSE 0x41
#define A2S_INFO			  "\xFF\xFF\xFF\xFF\x54Source Engine Query"
#define A2S_INFORESPONSE_HL1  0x6D
#define A2S_INFORESPONSE_HL2  0x49
#define A2S_PLAYER			"\xFF\xFF\xFF\xFF\x55"
#define A2S_PLAYERRESPONSE	0x44
#define A2S_RULES			 "\xFF\xFF\xFF\xFF\x56"
#define A2S_RULESRESPONSE	 0x45

long (*CALLBACK_UpdateServerListView)(DWORD index);
long (*CALLBACK_InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI);
long (*CALLBACK_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo);

struct _regions{
	BYTE cCode;
	char szName[40];
} REGIONS[] = {
0x00,  "US East coast",  
0x01,  "US West coast",  
0x02,  "South America"  ,
0x03,  "Europe",
0x04,  "Asia",  
0x05,  "Australia",  
0x06,  "Middle East",
0x07,  "Africa",  
0xFF,  "Rest of the world"
};
char cRegionCodeIndex=0;

/**********************************************************************

Details:
http://developer.valvesoftware.com/wiki/Master_Server_Query_Protocol

http://developer.valvesoftware.com/wiki/Server_Queries

***********************************************************************/
DWORD STEAM_GetChallenge(SERVER_INFO *pSI, DWORD &dwChallenge)
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	char sendbuf[80];
	size_t packetlen = 0;

	dwChallenge = 0;
	
	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return 1;
	}

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->dwPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 1;
	}

	strcpy_s(sendbuf,sizeof(sendbuf),A2S_GETCHALLENGE);
	
	packetlen = send(pSocket, sendbuf, strlen(A2S_GETCHALLENGE), 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		
		return 1;
	}
		packet=(unsigned char*)getpacket(pSocket, &packetlen);

	if(packet) 
	{
		A2S_CHALLANGE_RESPONSE_DATA	*data;
		data = (A2S_CHALLANGE_RESPONSE_DATA	*)packet;
		dwChallenge = data->dwChallenge;
	}
	closesocket(pSocket);
	
	return 0;
}

char *STEAM_GetString(char *pointer, DWORD &dwLen)
{
	char *l = pointer;
	DWORD i=0;
	while(l[i]!=0)
		i++;
	dwLen = i+1;
	char *str = (char*)calloc(1,i+1);

	memcpy(str,pointer,i);
	return str;
}
DWORD STEAM_GetPlayers(SERVER_INFO *pSI, DWORD dwChallenge)
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	char sendbuf[80];
	size_t packetlen = 0;
	PLAYERDATA *pQ3Players=NULL;
	PLAYERDATA *pQ3CurrentPlayer=NULL;
	
	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return 1;
	}

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->dwPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 1;
	}

	ZeroMemory(sendbuf,sizeof(sendbuf));
	strcpy_s(sendbuf,sizeof(sendbuf),A2S_PLAYER);
	DWORD dwtmp = dwChallenge;
	memcpy(&sendbuf[5],(DWORD*)&dwtmp,sizeof(DWORD));

	packetlen = send(pSocket, sendbuf, 9, 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		
		return 1;
	}
	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	char *pEndAddress = ( char*)packet + packetlen;
	if(packet) 
	{
		A2S_PLAYER_RESPONSE_DATA	*data = (A2S_PLAYER_RESPONSE_DATA*)packet;
		char *p = (char*)&data->playerData;

		int i=0;
		if(data->response != A2S_PLAYERRESPONSE)
			return 1;

		while(p<pEndAddress)
		{
		//	if(pSI->nCurrentPlayers!=data->numPlayers)
		//		DebugBreak();

			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory
				break;
			player->pNext = NULL;
			player->szClanTag = NULL;

			p++; //player index

			
			DWORD dwLen =0;
			
 			player->szPlayerName = STEAM_GetString(p, dwLen); //_strdup(p);
			p+=dwLen; //strlen(p)+1;
			//dbg_print(player->szPlayerName);
			//dbg_print("\n");

			DWORD *dwP;
			dwP = (DWORD*)p;
			player->rate = (DWORD)*dwP;  //kills
			p+=4;
			p+=4; //time

			if(pQ3Players==NULL)
				pQ3Players = pQ3CurrentPlayer = player;
			else 
				pQ3CurrentPlayer = pQ3CurrentPlayer->pNext = player;
			i++;
			if((p[0]==0x00) || (p[1]==0xFD))
				break;
			if(p>pEndAddress)
				DebugBreak();
				
		}
		pSI->pPlayerData = pQ3Players;	
		free(packet);
	}

	closesocket(pSocket);
	
	return 0;
}

DWORD STEAM_ConnectToMasterServer(GAME_INFO *pGI)
{
	size_t packetlen=0;
	char sendbuf[100];
	SOCKET ConnectSocket;
	ZeroMemory(sendbuf,sizeof(sendbuf));

	
	ConnectSocket = getsockudp(pGI->szMasterServerIP,(unsigned short)pGI->dwMasterServerPORT); 
   
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
	nRet = WSAEventSelect(ConnectSocket,hEvent,FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		dbg_print("EventSelect()");
		closesocket(ConnectSocket);
		WSACloseEvent(hEvent);
		return 5;
	}


	int i = 0;
	unsigned char *packet = NULL;
	size_t packet_len = 0;
	DWORD dwNewServers = 0;
	cRegionCodeIndex = 0;
	
	//
	// Handle async network events
	//
	WSANETWORKEVENTS events;
	while(1)
	{
		//
		// Wait for something to happen
		//

		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
									 &hEvent,
									 FALSE,
									 1000,
									 FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			dbg_print("\nWSAWaitForMultipleEvents timed out\n");
			
			if(cRegionCodeIndex<8)
			{
				cRegionCodeIndex++;
				//continue on the next region				
				ZeroMemory(sendbuf,sizeof(sendbuf));
				sprintf_s(sendbuf,sizeof(sendbuf), "1%c0.0.0.0:0\x00\x00",REGIONS[cRegionCodeIndex].cCode);
				memcpy(&sendbuf[12],pGI->szQueryString,strlen(pGI->szQueryString));
				int len = 12;
				int len2 = (int)strlen(pGI->szQueryString)+1;
				len +=len2;
				sendbuf[len-1]=0; //for debug purpose ensure to fill out with zeros
				sendbuf[len]=0;
				if(send(ConnectSocket, sendbuf, len , 0)==SOCKET_ERROR) 
				{

					WSACloseEvent(hEvent);
					closesocket(ConnectSocket);		
					 dbg_print("Error sending packet!");
					return 2;
				}
			} else
			{
				break;
			}

		}

		//
		// Figure out what happened
		//
		//dbg_print("\nWSAEnumNetworkEvents()");
		nRet = WSAEnumNetworkEvents(ConnectSocket,
								 hEvent,
								 &events);
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
			dbg_print("\nFD_CONNECT: %d", events.iErrorCode[FD_CONNECT_BIT]);
			ZeroMemory(sendbuf,sizeof(sendbuf));
			sprintf_s(sendbuf,sizeof(sendbuf), "1%c0.0.0.0:0\x00\x00",REGIONS[cRegionCodeIndex].cCode);
			memcpy(&sendbuf[12],pGI->szQueryString,strlen(pGI->szQueryString));
			int len = 12;
			int len2 = (int)strlen(pGI->szQueryString)+1;
			len +=len2;
			sendbuf[len-1]=0;  //for debug purpose ensure to fill out with zeros
			sendbuf[len]=0;
			if(send(ConnectSocket, sendbuf, len , 0)==SOCKET_ERROR) 
			{

				WSACloseEvent(hEvent);
				closesocket(ConnectSocket);		
				 dbg_print("Error sending packet!");
				return 2;
			}
		}

		// Read event?
		if (events.lNetworkEvents & FD_READ)
		{
			dbg_print("\nFD_READ: %d, %d",events.iErrorCode[FD_READ_BIT],i);
			char szLastIP[50];
			DWORD dwLastPort=0;
			ZeroMemory(szLastIP,sizeof(szLastIP));

			// Read the data and write it to stdout
			packet=(unsigned char*)ReadPacket(ConnectSocket, &packetlen);
			//packet=(unsigned char*)getpacket(ConnectSocket, &packetlen);
			packet_len = packetlen;

			DWORD dwNewSrv = 0;
		    dwNewSrv = STEAM_parseServers((char*)packet,packet_len,pGI,szLastIP,dwLastPort);
			if(dwNewSrv==99999) //Did we get any new servers?
				dwNewSrv = 0;  //Nope...

			pGI->dwTotalServers += dwNewSrv;
			dwNewServers += dwNewSrv;
			free(packet);
			SetStatusText(0,"Recieved %d new %s servers from master server (Region %s).",dwNewServers,pGI->szGAME_NAME,REGIONS[cRegionCodeIndex].szName);
			packet=NULL;
			i++;
			if(dwLastPort==0) //End of server list
			{
				
				if(cRegionCodeIndex<8)
				{
					cRegionCodeIndex++;
					strcpy_s(szLastIP,sizeof(szLastIP),"0.0.0.0:0");					
					
				} else
				{
					break;
				}
				
			}
			if(i>=MAX_PACKETS)
			{
				DebugBreak();
				break;
			}
			if(g_bCancel)
				break;

			
			char szPort[8];
			ZeroMemory(sendbuf,sizeof(sendbuf));
			_itoa(dwLastPort,szPort,10);
			sprintf_s(sendbuf,sizeof(sendbuf), "1%c%s:%d\x00\x00",REGIONS[cRegionCodeIndex].cCode,szLastIP,dwLastPort);
			int len = 2;
			len += strlen(szLastIP);
			len += strlen(szPort)+2;
			int len2 = (int)strlen(pGI->szQueryString)+1;
			
			memcpy(&sendbuf[len],pGI->szQueryString,strlen(pGI->szQueryString));
			len +=len2;
			sendbuf[len-1]=0;  //for debug purpose ensure to fill out with zeros
			sendbuf[len]=0;			

			if(send(ConnectSocket, sendbuf, len , 0)==SOCKET_ERROR) 
			{

				WSACloseEvent(hEvent);
				closesocket(ConnectSocket);		
				 dbg_print("Error sending packet!");
				return 2;
			}
		
		}

		// Close event?
		if (events.lNetworkEvents & FD_CLOSE)
		{
			dbg_print("\nFD_CLOSE: %d",events.iErrorCode[FD_CLOSE_BIT]);
			break;
		}

		// Write event?
	/*	if (events.lNetworkEvents & FD_WRITE)
		{
			AddLogInfo(0,"\nFD_WRITE: %d",events.iErrorCode[FD_WRITE_BIT]);
		}
		*/

	}
	
	closesocket(ConnectSocket);
	WSACloseEvent(hEvent);
	return 0;
}







DWORD STEAM_parseServers(char * packet, DWORD length, GAME_INFO *pGI,char *szLastIP,DWORD &dwLastPort)
{
	STEAM_MASTER_LEADDATA *leaddata;
	SERVER_INFO ptempSI;
	DWORD *dwIP=NULL;
	DWORD dwNewTotalServers = 0;

	if (packet==NULL)
		return 0;

	if(memcmp(A2S_MASTER_REPLY,packet,5)!=0)
		return 99999;

	leaddata = (STEAM_MASTER_LEADDATA*)packet;

	DWORD idx = pGI->pSC->vSI.size();

	char *endAddress;
	char *p = leaddata->data;
	endAddress = packet+length; 
	
	ptempSI.dwIP = 0;
	ptempSI.dwPort = 0;

	int i=1;
	while(p<(endAddress)) 
	{	
		ZeroMemory(&ptempSI,sizeof(SERVER_INFO));
		//Parse and initialize server info
		dwIP = (DWORD*)&p[0];
		ptempSI.dwIP = 0;
		ptempSI.dwIP = ntohl((DWORD)*dwIP); 
		
		if((p[0]==0) && (p[1]==0) && (p[2]==0) && (p[3]==0))
		{
			dwLastPort = 0;
			break;
		}
		p+=4;
		ptempSI.dwPort = 0;
		ptempSI.dwPort  = ((p[0])<<8);
		ptempSI.dwPort |=(unsigned char)(p[1]);
		ptempSI.dwPort &= 0x0000FFFF;	//safe, ensure max port value

		p+=2;
		//AddLogInfo(0,"Got   >%d %s:%d",i,ptempSI.szIPaddress,ptempSI.dwPort);
		if(UTILZ_CheckForDuplicateServer(pGI,ptempSI)==false)
		{					
			strcpy_s(ptempSI.szIPaddress,sizeof(ptempSI.szIPaddress),DWORD_IP_to_szIP(ptempSI.dwIP));
		//	AddLogInfo(0,"New   >%d %s:%d",i,ptempSI.szIPaddress,ptempSI.dwPort);
			ptempSI.dwPing = 9999;
			ptempSI.cGAMEINDEX = (char) pGI->cGAMEINDEX;
			ptempSI.cCountryFlag = 0;
			ptempSI.bNeedToUpdateServerInfo = true;
			ptempSI.dwIndex = idx++;
		
			pGI->pSC->vSI.push_back(ptempSI);

			if(CALLBACK_InsertServerItem!=NULL)
				CALLBACK_InsertServerItem(pGI,ptempSI);

			dwNewTotalServers++;
		} //end serverexsist
		i++;
	} //end while

	char *szNewIP = DWORD_IP_to_szIP(ptempSI.dwIP);
	if(szNewIP!=NULL)
		strcpy(szLastIP,szNewIP); 
	dwLastPort = ptempSI.dwPort;

	AddLogInfo(0,"Parsed %d servers Last IP seen is %s:%d",i,szLastIP,dwLastPort);

	return dwNewTotalServers;
}

DWORD STEAM_Get_ServerStatus(SERVER_INFO *pSI,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesListView)(SERVER_RULES *pServerRules))
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	DWORD dwStartTick;

	dwStartTick=0;	
	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return 1;
	}

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->dwPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 1;
	}
	char sendbuf[80];

	strcpy_s(sendbuf,sizeof(sendbuf),A2S_INFO);
	
	size_t packetlen = 0;

	//Some default values
	pSI->dwPing = 9999;

	if((pSI->cCountryFlag==0))
		{
			DWORD dwSHORTNAME;
			char country[60],szShortName[4];
			
			ZeroMemory(szShortName,sizeof(szShortName));
			//dwStartTick = GetTickCount();
			char *szNewCountryName = fnIPtoCountry2(pSI->dwIP,&dwSHORTNAME,country,szShortName);
			strncpy_s(pSI->szCountry,sizeof(pSI->szCountry),szNewCountryName,_TRUNCATE);  //Update country info only when adding a new server		
			strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
			//sprintf(country,"IPGeo %d ms %s\n",(GetTickCount() - dwStartTick),pSI->szCountry);
			//dbg_print(country);				

			pSI->cCountryFlag = 1;
		}

	packetlen = send(pSocket, sendbuf, strlen(A2S_INFO), 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return 1;
	}

	dwStartTick = GetTickCount();
	packet=(unsigned char*)getpacket(pSocket, &packetlen);


	if(packet) 
	{
		STEAM_SERVER_RESPONSE *resp;
		resp = (STEAM_SERVER_RESPONSE *)packet;

		pSI->dwPing = (GetTickCount() - dwStartTick);
		pSI->bNeedToUpdateServerInfo = false;
		pSI->bUpdated = true;
		
		//dbg_dumpbuf("dump.bin", packet, packetlen);

		char *end = (char*)((packet)+packetlen);
		char *p = (char*)resp->type;
		if(*resp->type ==  A2S_INFORESPONSE_HL1)
		{
			while(p<end)
			{
				if(p[0]==0)
					break;
				p++;
			}
			p++;
		} else if(*resp->type == A2S_INFORESPONSE_HL2)
		{
			pSI->dwVersion = (DWORD)p[0];  //network version, steam ver.
			p++;
			p++;
		}
		strncpy_s(pSI->szServerName,sizeof(pSI->szServerName),p,_TRUNCATE);
		p+=strlen(p)+1;

		strncpy_s(pSI->szMap,sizeof(pSI->szMap),p,_TRUNCATE);
		p+=strlen(p)+1;

		strncpy_s(pSI->szMod,sizeof(pSI->szMap),p,_TRUNCATE);
		p+=strlen(p)+1;

		strncpy_s(pSI->szGameTypeName,sizeof(pSI->szGameTypeName),p,_TRUNCATE);
		p+=strlen(p)+1;  //Game name
		
		if(*resp->type == A2S_INFORESPONSE_HL2)  
			p+=2;  //Skip AppID

		pSI->nCurrentPlayers = p[0];
		p++;

		pSI->nMaxPlayers = p[0];
		p++;

		if(*resp->type == A2S_INFORESPONSE_HL1)  
		{
			pSI->dwVersion = (DWORD)p[0];  //network version, steam ver.
				
		} else
		{
			pSI->cBots = p[0];   //bots data for HL2 packets
			
		}
		p++;
		pSI->bDedicated = p[0];  //dedicated
		p++;
		p++; //OS
		pSI->bPrivate = p[0];  
		p++; 
		if(*resp->type == A2S_INFORESPONSE_HL1)
		{
			if(p[0]==1)  //modinfo?
			{
				p+=strlen(p)+1;
				p+=strlen(p)+1;
				p+=11;	
			}
			pSI->bPunkbuster = p[0]; //VAC
			p++;
			pSI->cBots = p[0];
		} else //A2S_INFORESPONSE_HL2
		{
			pSI->bPunkbuster = p[0]; //VAC
			p++;
			strncpy_s(pSI->szVersion,sizeof(pSI->szVersion),p,_TRUNCATE);
		}
		pSI->dwVersion =  Get_FilterVersionByVersionString(pSI->cGAMEINDEX,pSI->szVersion);
		pSI->dwMap = Get_MapByName(pSI->cGAMEINDEX, pSI->szMap);
		pSI->wMod = Get_MapByName(pSI->cGAMEINDEX, pSI->szMod);

	
		DWORD dwChallenge;
		if(STEAM_GetChallenge(pSI,dwChallenge)==0)
		{
			pSI->pPlayerData = NULL;
			STEAM_GetPlayers(pSI,dwChallenge);

			if(pSI->pPlayerData!=NULL)
			{
				//AddLogInfo(0,"%d %s",pSI->dwIndex,pSI->pPlayerData->szPlayerName);			

				if(UpdatePlayerListView!=NULL)
					UpdatePlayerListView(pSI->pPlayerData);
			
				if(CALLBACK_CheckForBuddy!=NULL)
				{
					CALLBACK_CheckForBuddy(pSI->pPlayerData,pSI);
				}
				
				UTILZ_CleanUp_PlayerList(pSI->pPlayerData);
				pSI->pPlayerData = NULL;
			}
		}
		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;

	closesocket(pSocket);
	return 0;
}

void STEAM_OnServerSelection(SERVER_INFO* pServerInfo,long (*UpdatePlayerListView)(PLAYERDATA *Q3players),long (*UpdateRulesList)(SERVER_RULES*pServer_Rules) )
{
	if(pServerInfo==NULL)
		return;

	
	STEAM_Get_ServerStatus(pServerInfo,UpdatePlayerListView,UpdateRulesList);

	if(CALLBACK_UpdateServerListView!=NULL)
		CALLBACK_UpdateServerListView(pServerInfo->dwIndex);
}

void STEAM_SetCallbacks(long (*UpdateServerListView)(DWORD index), 
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	CALLBACK_UpdateServerListView = UpdateServerListView;
	CALLBACK_CheckForBuddy = _Callback_CheckForBuddy;
	CALLBACK_InsertServerItem = InsertServerItem;
}

