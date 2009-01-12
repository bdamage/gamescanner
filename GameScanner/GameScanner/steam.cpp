#define _SECURE_SCL 0



#include "stdafx.h"
#define _DEFINE_DEPRECATED_HASH_CLASSES 0


#include "steam.h"
#include "utilz.h"
#include "scanner.h"
#include "..\..\iptocountry\iptocountry.h"

#define MAX_PACKETS 1000
extern bool g_bCancel;
extern HWND g_hWnd;
extern APP_SETTINGS_NEW AppCFG;
extern CLanguage g_lang;
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
	DWORD dwInit;
	char leadData[2]; //FF FF FF FF 66 0A
	char data[1];
} ;

struct STEAM_SERVER_RESPONSE
{
	char leadData[4]; //FF FF FF FF 
	char type[1];
	char data[1];
} ;

char A2S_MASTER_REPLY[] =  {"\xFF\xFF\xFF\xFF\x66\x0A"};

#define S2C_CHALLENGE 0x41
#define A2S_GETCHALLENGE_GOLDSRC	  "\xFF\xFF\xFF\xFF"  //bug fix for protocol 48 since 2 Nov 2008, more details see http://developer.valvesoftware.com/wiki/Source_Server_Query_Protocol
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
	BOOL bActive;
} REGIONS[] = {
0x00,  "US East coast", FALSE, 
0x01,  "US West coast", FALSE, 
0x02,  "South America"  ,FALSE,
0x03,  "Europe",FALSE,
0x04,  "Asia",  FALSE,
0x05,  "Australia",  FALSE,
0x06,  "Middle East",FALSE,
0x07,  "Africa",  FALSE,
0xFF,  "Rest of the world",FALSE
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

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->usPort); 
 
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
		free(packet);
		closesocket(pSocket);
		return 0;
	}
	closesocket(pSocket);
	
	return 1;
}

DWORD STEAM_GetChallengeGoldSrc(SERVER_INFO *pSI, DWORD &dwChallenge)
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	char sendbuf[20];
	size_t packetlen = 0;

	dwChallenge = 0;
	
	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return 1;
	}

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->usPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 1;
	}

	ZeroMemory(sendbuf,sizeof(sendbuf));
	strcpy_s(sendbuf,sizeof(sendbuf),A2S_PLAYER);
	DWORD dwDefaultChallenge = 0xFFFFFFFF;
	memcpy(&sendbuf[5],(DWORD*)&dwDefaultChallenge,sizeof(DWORD));


	packetlen = send(pSocket, sendbuf, 9, 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		return 1;
	}
	packet=(unsigned char*)getpacket(pSocket, &packetlen);

	if(packet) 
	{
		DWORD dwReturn = 1;
		if(packet[4]==S2C_CHALLENGE)
		{
			A2S_CHALLANGE_RESPONSE_DATA	*data;
			data = (A2S_CHALLANGE_RESPONSE_DATA	*)packet;
			dwChallenge = data->dwChallenge;
			dwReturn =  0;
		}else if(packet[4]==A2S_PLAYERRESPONSE)
		{
			STEAM_ParsePlayers(pSI, (char*) packet,packetlen);
			dwReturn =  A2S_PLAYERRESPONSE;
		}

		free(packet);
		closesocket(pSocket);
		return dwReturn;
	}
	closesocket(pSocket);
	
	return 1;
}



char *STEAM_GetString(char *pointer, DWORD &dwLen)
{
	char *l = pointer;
	DWORD i=0;
	while(l[i]!=0)
		i++;
	dwLen = i+1;
	char *str = (char*)calloc(1,i+1);
//	UTF8toMB((LPCWSTR)pointer,str);
	memcpy(str,pointer,i);
	return str;
}
DWORD STEAM_ParsePlayers(SERVER_INFO *pSI, char *packet,DWORD dwLength)
{
	PLAYERDATA *pPlayers=NULL;
	PLAYERDATA *pCurrentPlayer=NULL;
	char *pEndAddress = ( char*)packet + dwLength;
	if(packet) 
	{
		A2S_PLAYER_RESPONSE_DATA	*data = (A2S_PLAYER_RESPONSE_DATA*)packet;
		char *p = (char*)&data->playerData;

		int i=0;
		if(data->response != A2S_PLAYERRESPONSE)
			return (DWORD)data->response;

		while(p<pEndAddress)
		{
		//	if(pSI->nCurrentPlayers!=data->numPlayers)
		//		DebugBreak();

			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory
				break;
			player->pNext = NULL;
			player->szClanTag = NULL;
			player->cGAMEINDEX = pSI->cGAMEINDEX;
			player->dwServerIndex = pSI->dwIndex;

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
			player->time = (DWORD)(*((float*)p));
			p+=4; //time
			

			if(pPlayers==NULL)
				pPlayers = pCurrentPlayer = player;
			else 
				pCurrentPlayer = pCurrentPlayer->pNext = player;
			i++;
			if((p[0]==0x00) && (p[1]==0xFD))
				break;

				
		}
	}
	pSI->pPlayerData = pPlayers;	
	return 0;
}

DWORD STEAM_GetPlayers(SERVER_INFO *pSI, DWORD dwChallenge)
{
	SOCKET pSocket = NULL;
	unsigned char *packet=NULL;
	char sendbuf[80];
	size_t packetlen = 0;

	
	if(pSI==NULL)
	{
		dbg_print("Invalid pointer argument @Get_ServerStatus!\n");
		return 1;
	}

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->usPort); 
 
	if(pSocket==INVALID_SOCKET)
	{
	  dbg_print("Error at getsockudp()\n");
	  return 1;
	}

	ZeroMemory(sendbuf,sizeof(sendbuf));
	strcpy_s(sendbuf,sizeof(sendbuf),A2S_PLAYER);

	memcpy(&sendbuf[5],(DWORD*)&dwChallenge,sizeof(DWORD));


	packetlen = send(pSocket, sendbuf, 9, 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		return 1;
	}
	packet=(unsigned char*)getpacket(pSocket, &packetlen);
	if(packet)
	{
		STEAM_ParsePlayers(pSI, (char*) packet,packetlen);
		free(packet);
	}
/*	char *pEndAddress = ( char*)packet + packetlen;
	if(packet) 
	{
		A2S_PLAYER_RESPONSE_DATA	*data = (A2S_PLAYER_RESPONSE_DATA*)packet;
		char *p = (char*)&data->playerData;

		int i=0;
		if(data->response != A2S_PLAYERRESPONSE)
		{	
			free(packet);
			return 1;
		}

		while(p<pEndAddress)
		{
		//	if(pSI->nCurrentPlayers!=data->numPlayers)
		//		DebugBreak();

			PLAYERDATA *player = (PLAYERDATA *)calloc(1,sizeof(PLAYERDATA));
			if(player==NULL) //Out of memory
				break;
			player->pNext = NULL;
			player->szClanTag = NULL;
			player->cGAMEINDEX = pSI->cGAMEINDEX;
			player->dwServerIndex = pSI->dwIndex;

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


			if(pPlayers==NULL)
				pPlayers = pCurrentPlayer = player;
			else 
				pCurrentPlayer = pCurrentPlayer->pNext = player;
			i++;
			if((p[0]==0x00) || (p[1]==0xFD))
				break;
			if(p>pEndAddress)
				DebugBreak();
				
		}
		pSI->pPlayerData = pPlayers;	
		free(packet);
	}
*/
	closesocket(pSocket);
	
	return 0;
}

DWORD STEAM_ConnectToMasterServer(GAME_INFO *pGI, int iMasterIdx)
{
	size_t packetlen=0;
	char sendbuf[100];
	SOCKET ConnectSocket;
	ZeroMemory(sendbuf,sizeof(sendbuf));

	DWORD val=1;
	for(int i=0;i<9;i++)
	{			
		REGIONS[i].bActive = FALSE;
		if(pGI->filter.dwRegion & val)
			REGIONS[i].bActive = TRUE;

		val = val *2;
	}

	char appid[]={"\\napp\\500"};
	char szIP[256];
	strcpy(szIP,pGI->szMasterServerIP[iMasterIdx]);
	SplitIPandPORT(szIP,pGI->dwMasterServerPORT);
	ConnectSocket = getsockudp(szIP,(unsigned short)pGI->dwMasterServerPORT); 
   
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
		dwRet = WSAWaitForMultipleEvents(1, &hEvent, FALSE,1000, FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			dbg_print("\nWSAWaitForMultipleEvents timed out\n");
nextRegion:
			if(cRegionCodeIndex<8)
			{

				
				cRegionCodeIndex++;
				if(pGI->filter.dwRegion!=0)
					if(REGIONS[cRegionCodeIndex].bActive==FALSE)
						goto nextRegion;

				//continue on the next region				
				ZeroMemory(sendbuf,sizeof(sendbuf));
				sprintf_s(sendbuf,sizeof(sendbuf), "1%c0.0.0.0:0\x00",REGIONS[cRegionCodeIndex].cCode);

				memcpy(&sendbuf[12],pGI->szMasterQueryString,strlen(pGI->szMasterQueryString));
				int len = 11;
				int len2 = (int)strlen(pGI->szMasterQueryString)+1;
				len +=len2;
				sendbuf[len]=0; //for debug purpose ensure to fill out with zeros
				sendbuf[len+1]=0;
				
				if(send(ConnectSocket, sendbuf, len+1 , 0)==SOCKET_ERROR) 
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
nextRegion3:
			if(cRegionCodeIndex<8)
			{				
				
				if(pGI->filter.dwRegion!=0)
					if(REGIONS[cRegionCodeIndex].bActive==FALSE)
					{
						cRegionCodeIndex++;
						goto nextRegion3;
					}
			}
			sprintf_s(sendbuf,sizeof(sendbuf), "1%c0.0.0.0:0\x00\x00",REGIONS[cRegionCodeIndex].cCode);
			memcpy(&sendbuf[12],pGI->szMasterQueryString,strlen(pGI->szMasterQueryString));
			int len = 12;
			int len2 = (int)strlen(pGI->szMasterQueryString)+1;
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
			SetStatusText(pGI->iIconIndex,g_lang.GetString("ValveMasterStatus"),dwNewServers,pGI->szGAME_NAME,REGIONS[cRegionCodeIndex].szName);
			packet=NULL;
			i++;
			if(dwLastPort==0) //End of server list
			{
nextRegion2:				
				if(cRegionCodeIndex<8)
				{
					cRegionCodeIndex++;
					if(pGI->filter.dwRegion!=0)
						if(REGIONS[cRegionCodeIndex].bActive==FALSE)
							goto nextRegion2;
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
			AddLogInfo(0,"Sending %s (1REGION%s:%d) to master server %s.",sendbuf,szLastIP,dwLastPort,&pGI->szMasterServerIP[iMasterIdx]);

			int len = 2;
			len += strlen(szLastIP);
			len += strlen(szPort)+2;
			int len2 = (int)strlen(pGI->szMasterQueryString)+1;
			
			memcpy(&sendbuf[len],pGI->szMasterQueryString,strlen(pGI->szMasterQueryString));
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
/*		if (events.lNetworkEvents & FD_CLOSE)
		{
			dbg_print("\nFD_CLOSE: %d",events.iErrorCode[FD_CLOSE_BIT]);
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

	DWORD idx = pGI->vSI.size();

	char *endAddress;
	char *p = leaddata->data;
	endAddress = packet+length; 
	
	AddLogInfo(0,"First 4 bytes %X",leaddata->dwInit);

	int i=1;
	int hash=0;
	ZeroMemory(&ptempSI,sizeof(SERVER_INFO));

	ptempSI.dwPing = 9999;
	ptempSI.cGAMEINDEX = pGI->cGAMEINDEX;
	ptempSI.bNeedToUpdateServerInfo = 1;			
	strcpy(ptempSI.szShortCountryName,"zz");

	while(p<(endAddress)) 
	{	
		//Parse and initialize server info
		dwIP = (DWORD*)&p[0];
		ptempSI.dwIP = 0;
		ptempSI.usPort = 0;
		ptempSI.dwIP = ntohl((DWORD)*dwIP); 
		
		if(ptempSI.dwIP==0) 
		{
			dwLastPort = 0;
			break;
		}
		p+=4;
		
		ptempSI.usPort  = ((p[0])<<8);
		ptempSI.usPort |=(unsigned char)(p[1]);
		p+=2;

		hash = ptempSI.dwIP + ptempSI.usPort;
		if(UTILZ_checkforduplicates(pGI,hash,ptempSI.dwIP, ptempSI.usPort)==FALSE)
		{					
			strcpy_s(ptempSI.szIPaddress,sizeof(ptempSI.szIPaddress),DWORD_IP_to_szIP(ptempSI.dwIP));

			ptempSI.dwIndex = idx++;
			pGI->shash.insert(Int_Pair(hash,ptempSI.dwIndex) );		
			pGI->vSI.push_back(ptempSI);

			dwNewTotalServers++;
		} //end serverexsist
		i++;
	} //end while

	char *szNewIP = DWORD_IP_to_szIP(ptempSI.dwIP);
	if(szNewIP!=NULL)
		strcpy(szLastIP,szNewIP); 
	dwLastPort = ptempSI.usPort;

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

	pSocket =  getsockudp(pSI->szIPaddress , (unsigned short)pSI->usPort); 
 
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

	if( ((pSI->szShortCountryName[0]=='E') && (pSI->szShortCountryName[1]=='U')) || ((pSI->szShortCountryName[0]=='z') && (pSI->szShortCountryName[1]=='z')))
		{
			//DWORD dwSHORTNAME;
			char country[60],szShortName[4];
			
			//ZeroMemory(szShortName,sizeof(szShortName));
			//dwStartTick = GetTickCount();
			char *szNewCountryName = fnIPtoCountry2(pSI->dwIP,country,szShortName);
			strncpy_s(pSI->szCountry,sizeof(pSI->szCountry),szNewCountryName,_TRUNCATE);  //Update country info only when adding a new server		
			strncpy_s(pSI->szShortCountryName,sizeof(pSI->szShortCountryName),szShortName,_TRUNCATE);
			//sprintf(country,"IPGeo %d ms %s\n",(GetTickCount() - dwStartTick),pSI->szCountry);
			//dbg_print(country);
		}
	DWORD dwRetries=0;
retry:
	packetlen = send(pSocket, sendbuf, strlen(A2S_INFO)+1, 0);
	if(packetlen==SOCKET_ERROR) 
	{
		dbg_print("Error at send()\n");
		closesocket(pSocket);		
		pSI->cPurge++;
		return 1;
	}

	packetlen = 0;
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
		STEAM_SERVER_RESPONSE *resp;
		resp = (STEAM_SERVER_RESPONSE *)packet;

		pSI->dwPing = (GetTickCount() - dwStartTick);
		pSI->bNeedToUpdateServerInfo = 0;
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

	//	WCHAR wOutBuff[512];
	//	MultiByteToWideChar(CP_UTF8, MB_COMPOSITE, pSI->szServerName, -1, wOutBuff,  511 );

		strncpy_s(pSI->szMap,sizeof(pSI->szMap),p,_TRUNCATE);
		p+=strlen(p)+1;

		strncpy_s(pSI->szMod,sizeof(pSI->szMod),p,_TRUNCATE);
		p+=strlen(p)+1;

		strncpy_s(pSI->szGameTypeName,sizeof(pSI->szGameTypeName),p,_TRUNCATE);
		p+=strlen(p)+1;  //Game name
		
		if(*resp->type == A2S_INFORESPONSE_HL2)  
		{
			pSI->STEAM_AppId = *(short*)p;
			p+=2;  // AppID
		}

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
		if((*resp->type == A2S_INFORESPONSE_HL1))
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
			pSI->dwVersion =  Get_FilterVersionByVersionString(pSI->cGAMEINDEX,pSI->szVersion);
		}
		
		pSI->dwMap = Get_MapByName(pSI->cGAMEINDEX, pSI->szMap);
		pSI->dwMod = Get_ModByName(pSI->cGAMEINDEX, pSI->szMod);
		pSI->dwGameType = pSI->dwMod ; //For sorting
	
		DWORD dwChallenge=0;
		DWORD dwRet=1;
		
		if((*resp->type == A2S_INFORESPONSE_HL1) || (pSI->STEAM_AppId<200)) //Added to handle the Gold Source games reporting HL2 protocol
			dwRet = STEAM_GetChallengeGoldSrc(pSI,dwChallenge);
		else
			dwRet = STEAM_GetChallenge(pSI,dwChallenge);
		
		if(dwRet==0)
		{
			pSI->pPlayerData = NULL;
			STEAM_GetPlayers(pSI,dwChallenge);
		} 
		if(pSI->pPlayerData!=NULL)
		{
			//AddLogInfo(0,"%d %s",pSI->dwIndex,pSI->pPlayerData->szPlayerName);			

			if(UpdatePlayerListView!=NULL)
				UpdatePlayerListView(pSI->pPlayerData);
		
			if(CALLBACK_CheckForBuddy!=NULL)
			{
				CALLBACK_CheckForBuddy(pSI->pPlayerData,pSI);
			}
			
			CleanUp_PlayerList(pSI->pPlayerData);
			pSI->pPlayerData = NULL;
		}

		free(packet);

	} //end if(packet)
	else
		pSI->cPurge++;

	closesocket(pSocket);
	return 0;
}

void STEAM_SetCallbacks(long (*UpdateServerListView)(DWORD index), 
					 long (*_Callback_CheckForBuddy)(PLAYERDATA *pPlayers, SERVER_INFO* pServerInfo),
					 long (*InsertServerItem)(GAME_INFO *pGI,SERVER_INFO pSI))
{
	CALLBACK_UpdateServerListView = UpdateServerListView;
	CALLBACK_CheckForBuddy = _Callback_CheckForBuddy;
	CALLBACK_InsertServerItem = InsertServerItem;
}

